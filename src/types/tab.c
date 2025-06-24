#include "tab.h"

#include "db/db.h"

#include <globals.h>

void tab_list_update_positions(TabList_ptr tabs) {
    i32 i = -1;
    for each(Tab_ptr, tab, TabList, tabs) {
        if(tab->position != ++i) {
            tab->position = i;
            db_save_tab(db, tab, TabsColumn_position);
        }
    }
}
