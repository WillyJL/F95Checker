# modules/batch_management.py
"""Batch management support for F95Checker.

Provides multi-item selection with Windows Explorer semantics:
- Single LMB: select game, deselect previous selections.
- Double LMB: open game detailed info popup.
- Shift + LMB: contiguous range selection from last selected game.
- Ctrl + LMB: toggle individual game selection.
- RMB: select clicked item if not already selected, then open context menu.
- Accent border with style corner radius and standard border thickness.
- Streamlined batch context menu when multiple items are selected.
"""

import sys
import glfw
import imgui

_orig_handle_game_hitbox_events = None
_orig_draw_game_context_menu = None
_orig_draw_game_cell = None
_applied = False


def draw_batch_game_context_menu(self):
    """Draw context menu for multiple selected games."""
    from modules import icons

    imgui.text(f"Selected games: {self.selected_games_count}")
    imgui.separator()
    self.draw_game_recheck_button(None, f"{icons.reload_alert} Full Recheck", selectable=True)
    imgui.separator()
    self.draw_game_clear_exes_button(None, f"{icons.folder_remove_outline} Clear Exes", selectable=True)
    imgui.separator()
    self.draw_game_finished_checkbox(None, f"{icons.flag_checkered} Finished")
    self.draw_game_installed_checkbox(None, f"{icons.download} Installed")
    imgui.separator()
    self.draw_game_rating_widget(None)
    if imgui.begin_menu(f"{icons.label_multiple_outline} Labels"):
        draw_batch_game_labels_select_widget(self)
        imgui.end_menu()
    if imgui.begin_menu(f"{icons.tab} Move to Tab"):
        self.draw_game_tab_select_widget(None)
        imgui.end_menu()
    imgui.separator()
    self.draw_game_archive_button(
        None,
        label_off=f"{icons.archive_outline} Archive",
        label_on=f"{icons.archive_off_outline} Unarchive",
        selectable=True,
    )
    self.draw_game_remove_button(None, f"{icons.trash_can_outline} Remove", selectable=True)


def draw_batch_game_labels_select_widget(self):
    """Draw label options for multiple games with scoped IDs."""
    from common.structs import Label
    from modules import globals, icons

    if Label.instances:
        for label in Label.instances:
            imgui.push_id(str(label.id))
            if imgui.small_button(icons.check):
                for game in globals.games.values():
                    if game.selected:
                        game.add_label(label)
            imgui.same_line()
            if imgui.small_button(icons.close):
                for game in globals.games.values():
                    if game.selected:
                        game.remove_label(label)
            imgui.same_line()
            self.draw_label_widget(label)
            imgui.pop_id()
    else:
        imgui.text_disabled("Make some labels first!")


def patched_draw_game_context_menu(self, game=None):
    """Route context menu to batch menu if multiple games are selected."""
    if game is None or self.selected_games_count > 1:
        draw_batch_game_context_menu(self)
    else:
        _orig_draw_game_context_menu(self, game)


def patched_handle_game_hitbox_events(self, game, drag_drop: bool = False):
    """Handle hitbox interactions simulating Windows Explorer selection behavior."""
    from modules import db, globals, utils
    from modules.gui import cols
    from external import async_thread

    manual_sort = cols.manual_sort.enabled
    is_hovered = imgui.is_item_hovered(imgui.HOVERED_ALLOW_WHEN_BLOCKED_BY_ACTIVE_ITEM)

    if is_hovered:
        self.hovered_game = game

        # Double click opens game info popup
        if imgui.is_mouse_double_clicked(0):
            self.game_hitbox_click = False
            utils.push_popup(self.draw_game_info_popup, game, self.show_games_ids[self.current_tab].copy())

        # Track single LMB press
        elif imgui.is_item_clicked(0):
            self.game_hitbox_click = True

        # Process single LMB release
        if self.game_hitbox_click and not imgui.is_mouse_down(0):
            self.game_hitbox_click = False

            # Shift + LMB: Range select from last selected game
            if imgui.is_key_down(glfw.KEY_LEFT_SHIFT) or imgui.is_key_down(glfw.KEY_RIGHT_SHIFT):
                if self.selected_games_count and self.last_selected_game and self.last_selected_game.selected:
                    tab_games_ids = self.show_games_ids[self.current_tab]
                    if self.last_selected_game.id in tab_games_ids and game.id in tab_games_ids:
                        start = tab_games_ids.index(self.last_selected_game.id)
                        end = tab_games_ids.index(game.id)
                        if start > end:
                            start, end = end, start
                        for select_id in tab_games_ids[start:end + 1]:
                            globals.games[select_id].selected = True
                    else:
                        game.selected = True
                        self.last_selected_game = game
                else:
                    for g in globals.games.values():
                        if g is not game and g.selected:
                            g.selected = False
                    game.selected = True
                    self.last_selected_game = game

            # Ctrl + LMB: Toggle clicked game
            elif imgui.is_key_down(glfw.KEY_LEFT_CONTROL) or imgui.is_key_down(glfw.KEY_RIGHT_CONTROL):
                game.selected = not game.selected
                if game.selected:
                    self.last_selected_game = game

            # Standard LMB: Select clicked game, deselect all other games
            else:
                for g in globals.games.values():
                    if g is not game and g.selected:
                        g.selected = False
                game.selected = True
                self.last_selected_game = game

        # RMB: If clicked on an unselected game, deselect others and select this one
        if imgui.is_item_clicked(1):
            if not game.selected:
                for g in globals.games.values():
                    if g is not game and g.selected:
                        g.selected = False
                game.selected = True
                self.last_selected_game = game

    # In list mode (table view), render selection border around the row
    if not getattr(self, "_in_game_cell", False) and game.selected:
        from modules import globals
        rect_min = imgui.get_item_rect_min()
        rect_max = imgui.get_item_rect_max()
        rounding = self.scaled(globals.settings.style_corner_radius)
        imgui.get_window_draw_list().add_rect(
            *rect_min, *rect_max,
            imgui.get_color_u32_rgba(*globals.settings.style_accent),
            rounding=rounding,
            thickness=imgui.style.frame_border_size,
        )

    # Left click drag = swap if in manual sort mode
    if imgui.begin_drag_drop_source(flags=self.game_hitbox_drag_drop_flags):
        self.game_hitbox_click = False
        payload = (globals.settings.manual_sort_list.index(game.id) if manual_sort else 0) + 1
        payload = payload.to_bytes(payload.bit_length(), sys.byteorder)
        imgui.set_drag_drop_payload("manual_swap", payload)
        imgui.end_drag_drop_source()

    if drag_drop and manual_sort:
        if imgui.begin_drag_drop_target():
            if payload := imgui.accept_drag_drop_payload("manual_swap", flags=self.game_hitbox_drag_drop_flags):
                payload = int.from_bytes(payload, sys.byteorder)
                payload = payload - 1
                lst = globals.settings.manual_sort_list
                switch = lst.index(game.id)
                lst[switch], lst[payload] = lst[payload], lst[switch]
                async_thread.run(db.update_settings("manual_sort_list"))
                self.recalculate_ids = True
            imgui.end_drag_drop_target()

    # Context menu popup
    context_id = f"###{game.id}_context"
    if (imgui.is_topmost() or imgui.is_popup_open(context_id)) and imgui.begin_popup_context_item(context_id):
        if self.selected_games_count > 1 and game.selected:
            self.draw_game_context_menu(None)
        else:
            self.draw_game_context_menu(game)
        imgui.end_popup()


def patched_draw_game_cell(
    self,
    game,
    drag_drop: bool,
    draw_list,
    cell_width: float,
    expand: bool,
    img_height: float,
    config: tuple,
):
    """Draw game card with accent border and corner radius when selected."""
    from modules import globals

    self._in_game_cell = True
    try:
        _orig_draw_game_cell(self, game, drag_drop, draw_list, cell_width, expand, img_height, config)
    finally:
        self._in_game_cell = False

    # Draw solid accent border with corner radius over merged card
    if game.selected:
        rect_min = imgui.get_item_rect_min()
        rect_max = imgui.get_item_rect_max()
        rounding = self.scaled(globals.settings.style_corner_radius)
        draw_list.add_rect(
            *rect_min, *rect_max,
            imgui.get_color_u32_rgba(*globals.settings.style_accent),
            rounding=rounding,
            thickness=imgui.style.frame_border_size,
        )


def apply(main_gui_cls=None):
    """Apply batch management monkey-patches to MainGUI."""
    global _orig_handle_game_hitbox_events, _orig_draw_game_context_menu, _orig_draw_game_cell, _applied

    if _applied:
        return

    if main_gui_cls is None:
        if "modules.gui" in sys.modules and hasattr(sys.modules["modules.gui"], "MainGUI"):
            main_gui_cls = sys.modules["modules.gui"].MainGUI
        else:
            # MainGUI not loaded yet, wait for import hook
            install()
            return

    _orig_handle_game_hitbox_events = main_gui_cls.handle_game_hitbox_events
    _orig_draw_game_context_menu = main_gui_cls.draw_game_context_menu
    _orig_draw_game_cell = main_gui_cls.draw_game_cell

    main_gui_cls.handle_game_hitbox_events = patched_handle_game_hitbox_events
    main_gui_cls.draw_game_context_menu = patched_draw_game_context_menu
    main_gui_cls.draw_game_cell = patched_draw_game_cell
    main_gui_cls.draw_batch_game_context_menu = draw_batch_game_context_menu
    _applied = True


def install():
    """Install import hook to apply patches once MainGUI is loaded."""
    import builtins
    real_import = builtins.__import__

    def hooked_import(name, globals=None, locals=None, fromlist=(), level=0):
        module = real_import(name, globals, locals, fromlist, level)
        if name in ("modules.gui", "gui") and hasattr(module, "MainGUI"):
            apply(module.MainGUI)
        return module

    builtins.__import__ = hooked_import
