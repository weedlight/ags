/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2005-2011 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "ags_combo_box_text.h"

/**
 * SECTION:ags_combo_box_text
 * @short_description: Complete GtkComboBox
 * @title: AgsComboBoxText
 * @section_id:
 * @include: ags/lib/ags_combo_box_text.h
 *
 * Functions completing #GtkComboBoxText API.
 */

/**
 * ags_combo_box_text_remove_all:
 * @combo_box: the #GtkComboBoxText
 * 
 * Emptys #GtkTreeModel of @combo_box.
 *
 * Since: 0.4
 */
void
ags_combo_box_text_remove_all(GtkComboBoxText *combo_box){
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box));
  
  if(GTK_IS_LIST_STORE(model) && gtk_tree_model_get_iter_first(model, &iter)){   
    do{
      gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    }while(gtk_tree_model_iter_next(model, &iter));
  }
}
