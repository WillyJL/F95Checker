#include "label.h"

#include "db/db.h"

#include <globals.h>

void label_list_update_positions(LabelList_ptr labels) {
    i32 i = -1;
    for each(Label_ptr, label, LabelList, labels) {
        if(label->position != ++i) {
            label->position = i;
            db_save_label(db, label, LabelsColumn_position);
        }
    }
}
