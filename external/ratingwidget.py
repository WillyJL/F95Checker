# https://gist.github.com/WillyJL/e9e9dac70b7970b6ee12fcf52b9b8f11
import imgui

from modules import icons

filled_icon = icons.star
half_icon = icons.star_half
empty_icon  = icons.star_outline


def ratingwidget(id: str, current: float, num_stars=5):
    value = current
    accent_col = imgui.style.colors[imgui.COLOR_BUTTON_HOVERED]
    imgui.push_style_color(imgui.COLOR_BUTTON, 0, 0, 0, 0)
    imgui.push_style_color(imgui.COLOR_BUTTON_ACTIVE, 0, 0, 0, 0)
    imgui.push_style_color(imgui.COLOR_BUTTON_HOVERED, 0, 0, 0, 0)
    imgui.push_style_var(imgui.STYLE_FRAME_PADDING, (0, 0))
    imgui.push_style_var(imgui.STYLE_ITEM_SPACING, (0, 0))
    imgui.push_style_var(imgui.STYLE_FRAME_BORDERSIZE, 0)
    star_size = imgui.calc_text_size(filled_icon)
    pos = imgui.get_cursor_pos()
    imgui.invisible_button(id, star_size.x * num_stars, star_size.y)
    imgui.same_line()
    imgui.set_cursor_pos(pos)
    is_active = imgui.is_item_active()
    is_hovered = imgui.is_item_hovered()
    show = current
    if is_active or is_hovered:
        mouse_pos = imgui.get_mouse_pos()
        star_pos_x = imgui.get_item_rect_min().x
        for i in range(num_stars):
            if mouse_pos.x < star_pos_x:
                show = float(i)
                break
            elif mouse_pos.x < star_pos_x + star_size.x / 2:
                show = i + 0.5
                break
            else:
                show = float(i + 1)
            star_pos_x += star_size.x
        show = min(max(show, 0.0), num_stars)
        if is_active:
            if imgui.is_item_clicked():
                if show == current:
                    value = 0.0  # Clicking the current value resets the rating to 0
                else:
                    value = show
            elif imgui.is_mouse_dragging(imgui.MOUSE_BUTTON_LEFT):
                value = show
        elif is_hovered:
            accent_col = accent_col._replace(w=accent_col.w * 0.69)
    for i in range(1, num_stars + 1):
        if i > show and show > i - 1:
            pos = imgui.get_cursor_pos()
            imgui.text(empty_icon)
            imgui.same_line()
            imgui.set_cursor_pos(pos)
            label = half_icon
        elif i <= show:
            label = filled_icon
        else:
            label = empty_icon
        if label != empty_icon:
            imgui.push_style_color(imgui.COLOR_TEXT, *accent_col)
        imgui.text(label)
        if label != empty_icon:
            imgui.pop_style_color()
        imgui.same_line()
    imgui.pop_style_color(3)
    imgui.pop_style_var(3)
    imgui.dummy(0, 0)
    return value != current, value


# Example usage
if __name__ == "__main__":
    rating_5 = 0
    rating_10 = 0

    # Note: you will need material design icons or another icon font for this:
    imgui.get_io().fonts.add_font_from_file_ttf(
        "materialdesignicons-webfont.ttf", 16,
        font_config=imgui.core.FontConfig(merge_mode=True),
        glyph_ranges=imgui.core.GlyphRanges([0xf0000, 0xf2000, 0])
    )
    impl.refresh_font_texture()

    while True:  # Your main window draw loop
        with imgui.begin("Example rating"):

            imgui.text("With 5 stars:")
            imgui.same_line()
            changed, rating_5 = ratingwidget("5_stars", rating_5)  # Default star count is 5
            if changed:
                imgui.same_line()
                imgui.text(f"You set me to {rating_5} stars!")

            imgui.text("With 10 stars:")
            imgui.same_line()
            _, rating_10 = ratingwidget("10_stars", rating_10, num_stars=10)
