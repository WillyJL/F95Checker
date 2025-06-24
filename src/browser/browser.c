#include "browser.h"

#include "path/path.h"

#include <shlex/shlex.h>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/hash.h>

static BrowserHash browser_name_hash(const char* name) {
    // Why did I think this was a good idea 3 years ago?
    u8 md5[WC_MD5_DIGEST_SIZE];
    wc_Md5Hash((void*)name, strlen(name), md5);
    BrowserHash hash = 0;
    for(size_t i = 0; i < 6; i++) {
        hash = (hash << 8) | md5[10 + i];
    }
    assert(hash != BrowserReservedHashIntegrated && hash != BrowserReservedHashCustom);
    return hash;
}

static void browser_parse_desktop_file(BrowserList_ptr browsers, Path* desktop_file) {
    m_string_list_t lines;
    m_string_list_init(lines);

    if(path_read_lines(desktop_file, lines)) {
        Browser browser;
        browser_init(browser);

        bool is_desktop_entry = false;
        bool is_private_action = false;
        for each(m_string_ptr, line, m_string_list_t, lines) {
            if(m_string_start_with_str_p(line, "[") && m_string_end_with_str_p(line, "]")) {
                is_desktop_entry = m_string_equal_p(line, "[Desktop Entry]");
                is_private_action = m_string_equal_p(line, "[Desktop Action new-private-window]");
                continue;
            }
            if(is_desktop_entry && m_string_start_with_str_p(line, "Name=")) {
                m_string_right(line, strlen("Name="));
                m_string_set(browser->name, line);
            }
            if(m_string_start_with_str_p(line, "Exec=")) {
                m_string_list_ptr args = NULL;
                if(is_desktop_entry && m_string_list_empty_p(browser->args_regular)) {
                    args = browser->args_regular;
                } else if(is_private_action && m_string_list_empty_p(browser->args_private)) {
                    args = browser->args_private;
                }
                if(args == NULL) continue;
                m_string_right(line, strlen("Exec="));
                shlex_split(m_string_get_cstr(line), args);
                for each(m_string_ptr, arg, m_string_list_t, args) {
                    if(m_string_start_with_str_p(arg, "%")) {
                        m_string_list_remove(args, arg_it);
                    }
                }
            }
        }

        if(!m_string_empty_p(browser->name) && !m_string_list_empty_p(browser->args_regular)) {
            browser->hash = browser_name_hash(m_string_get_cstr(browser->name));
            if(m_string_list_empty_p(browser->args_private)) {
                m_string_list_set(browser->args_private, browser->args_regular);
                struct {
                    const char* search;
                    const char* arg;
                } private_args[] = {
                    {"Opera", "-private"},
                    {"Chrom", "-incognito"},
                    {"Brave", "-incognito"},
                    {"Edge", "-inprivate"},
                    {"fox", "-private-window"},
                    {"Zen", "-private-window"},
                    {"LibreWolf", "-private-window"},
                };
                for(size_t i = 0; i < COUNT_OF(private_args); i++) {
                    if(m_string_search(browser->name, private_args[i].search) !=
                       M_STRING_FAILURE) {
                        m_string_t private_arg;
                        m_string_init_set(private_arg, private_args[i].arg);
                        m_string_list_push_back_move(browser->args_private, &private_arg);
                        break;
                    }
                }
            }
            browser_list_push_back_move(browsers, &browser);
        } else {
            browser_clear(browser);
        }
    }

    m_string_list_clear(lines);
}

static void browser_discover_installed_xdg(BrowserList_ptr browsers, const char* xdg_data_dir) {
    UNUSED(browsers);
    if(xdg_data_dir[0] != '/') return;

    Path* app_dir = path_init(xdg_data_dir);
    path_join(app_dir, "applications");

    Path* apps_file = path_dup(app_dir);
    path_join(apps_file, "mimeinfo.cache");
    m_string_list_t lines;
    m_string_list_init(lines);

    if(path_is_file(apps_file) && path_read_lines(apps_file, lines)) {
        m_string_list_t apps;
        m_string_list_init(apps);

        for each(m_string_ptr, line, m_string_list_t, lines) {
            // x-scheme-handler/https=app1.desktop;app2.desktop;
            if(!m_string_start_with_str_p(line, "x-scheme-handler/http=") &&
               !m_string_start_with_str_p(line, "x-scheme-handler/https=")) {
                continue;
            }
            m_string_right(line, m_string_search_char(line, '=') + 1);

            m_string_t app;
            m_string_init(app);
            while(!m_string_empty_p(line)) {
                size_t separator = m_string_search_char(line, ';');
                if(separator != M_STRING_FAILURE) {
                    m_string_set_n(app, line, 0, separator);
                    m_string_right(line, separator + 1);
                } else {
                    m_string_set(app, line);
                    m_string_reset(line);
                }
                if(!m_string_empty_p(app)) {
                    bool is_duplicate = false;
                    for each(m_string_ptr, found_app, m_string_list_t, apps) {
                        if(!m_string_equal_p(app, found_app)) continue;
                        is_duplicate = true;
                        break;
                    }
                    if(!is_duplicate) {
                        m_string_list_push_back(apps, app);
                    }
                }
            }
            m_string_clear(app);
        }

        for each(m_string_ptr, app, m_string_list_t, apps) {
            Path* app_file = path_dup(app_dir);
            path_join(app_file, m_string_get_cstr(app));
            browser_parse_desktop_file(browsers, app_file);
            path_free(app_file);
        }

        m_string_list_clear(apps);
    }

    m_string_list_clear(lines);
    path_free(apps_file);

    path_free(app_dir);
}

void browser_discover_installed(BrowserList_ptr browsers) {
    Browser browser;
    browser_init(browser);

    m_string_set(browser->name, "Integrated");
    browser->hash = BrowserReservedHashIntegrated;
    browser_list_push_back(browsers, browser);

    m_string_set(browser->name, "Custom");
    browser->hash = BrowserReservedHashCustom;
    browser_list_push_back(browsers, browser);

    browser_clear(browser);

#if OS == OS_WINDOWS
#error Not implemented // FIXME: browsers on windows
#elif OS == OS_LINUX

    m_string_t xdg_data_dirs;
    m_string_init(xdg_data_dirs);
    const char* xdg_data_dirs_env = getenv("XDG_DATA_DIRS");
    if(xdg_data_dirs_env != NULL) {
        m_string_set(xdg_data_dirs, xdg_data_dirs_env);
    } else {
        m_string_set(xdg_data_dirs, "/usr/share/");
    }

    m_string_t xdg_data_dir;
    m_string_init(xdg_data_dir);
    while(true) {
        size_t separator = m_string_search_char(xdg_data_dirs, ':');
        if(separator == M_STRING_FAILURE) {
            browser_discover_installed_xdg(browsers, m_string_get_cstr(xdg_data_dirs));
            break;
        }

        m_string_set_n(xdg_data_dir, xdg_data_dirs, 0, separator);
        browser_discover_installed_xdg(browsers, m_string_get_cstr(xdg_data_dir));
        m_string_right(xdg_data_dirs, separator + 1);
    }
    m_string_clear(xdg_data_dir);

    m_string_clear(xdg_data_dirs);

#elif OS == OS_MACOS
#error Not implemented // FIXME: browsers on macos
#endif
}
