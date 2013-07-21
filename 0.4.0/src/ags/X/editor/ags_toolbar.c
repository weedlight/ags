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

#include <ags/X/editor/ags_toolbar.h>
#include <ags/X/editor/ags_toolbar_callbacks.h>
#include <ags/X/editor/ags_toolbar_mode_stock.h>

#include <ags/X/ags_menu_bar.h>

#include <gtk/gtkhbox.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkstock.h>

void ags_toolbar_class_init(AgsToolbarClass *toolbar);
void ags_toolbar_init(AgsToolbar *toolbar);
void ags_toolbar_connect(AgsToolbar *toolbar);
void ags_toolbar_destroy(GtkObject *object);
void ags_toolbar_show(GtkWidget *widget);

GType
ags_toolbar_get_type(void)
{
  static GType toolbar_type = 0;

  if (!toolbar_type){
    static const GtkTypeInfo toolbar_info = {
      "AgsToolbar\0",
      sizeof(AgsToolbar), /* base_init */
      sizeof(AgsToolbarClass), /* base_finalize */
      (GtkClassInitFunc) ags_toolbar_class_init,
      (GtkObjectInitFunc) ags_toolbar_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      (GtkClassInitFunc) NULL,
    };

    toolbar_type = gtk_type_unique (GTK_TYPE_TOOLBAR, &toolbar_info);
  }

  return (toolbar_type);
}

void
ags_toolbar_class_init(AgsToolbarClass *toolbar)
{
}

void
ags_toolbar_init(AgsToolbar *toolbar)
{
  GtkLabel *label;
  GtkMenu *menu;
  GtkMenuItem *item;

  toolbar->position = (GtkToggleButton *) g_object_new(GTK_TYPE_TOGGLE_BUTTON,
						       "image\0", (GtkWidget *) gtk_image_new_from_stock(GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_LARGE_TOOLBAR),
						       "relief\0", GTK_RELIEF_NONE,
						       NULL);
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) toolbar->position, "position cursor\0", NULL);

  toolbar->edit = (GtkToggleButton *) g_object_new(GTK_TYPE_TOGGLE_BUTTON,
						   "image\0", (GtkWidget *) gtk_image_new_from_stock(GTK_STOCK_EDIT, GTK_ICON_SIZE_LARGE_TOOLBAR),
						   "relief\0", GTK_RELIEF_NONE,
						   "active\0", TRUE,
						   NULL);
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) toolbar->edit, "edit notes\0", NULL);
  toolbar->selected_edit_mode = toolbar->edit;

  toolbar->clear = (GtkToggleButton *) g_object_new(GTK_TYPE_TOGGLE_BUTTON,
						    "image\0", (GtkWidget *) gtk_image_new_from_stock(GTK_STOCK_CLEAR, GTK_ICON_SIZE_LARGE_TOOLBAR),
						    "relief\0", GTK_RELIEF_NONE,
						    NULL);
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) toolbar->clear, "clear notes\0", NULL);

  toolbar->select = (GtkToggleButton *) g_object_new(GTK_TYPE_TOGGLE_BUTTON,
						     "image\0", (GtkWidget *) gtk_image_new_from_stock(GTK_STOCK_SELECT_ALL, GTK_ICON_SIZE_LARGE_TOOLBAR),
						     "relief\0", GTK_RELIEF_NONE,
						     NULL);
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) toolbar->select, "select notes\0", NULL);

  toolbar->copy = (GtkButton *) g_object_new(GTK_TYPE_BUTTON,
					     "image\0", (GtkWidget *) gtk_image_new_from_stock(GTK_STOCK_COPY, GTK_ICON_SIZE_LARGE_TOOLBAR),
					     "relief\0", GTK_RELIEF_NONE,
					     NULL);
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) toolbar->copy, "copy notes\0", NULL);

  toolbar->cut = (GtkButton *) g_object_new(GTK_TYPE_BUTTON,
					    "image\0", (GtkWidget *) gtk_image_new_from_stock(GTK_STOCK_CUT, GTK_ICON_SIZE_LARGE_TOOLBAR),
					    "relief\0", GTK_RELIEF_NONE,
					    NULL);
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) toolbar->cut, "cut notes\0", NULL);

  toolbar->paste = (GtkButton *) g_object_new(GTK_TYPE_BUTTON,
					      "image\0", (GtkWidget *) gtk_image_new_from_stock(GTK_STOCK_PASTE, GTK_ICON_SIZE_LARGE_TOOLBAR),
					      "relief\0", GTK_RELIEF_NONE,
					      NULL);
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) toolbar->paste, "paste notes\0", NULL);

  label = (GtkLabel *) gtk_label_new("zoom\0");
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) label, NULL, NULL);

  toolbar->zoom_history = 6;
  toolbar->zoom = (GtkOptionMenu *) gtk_option_menu_new();
  gtk_option_menu_set_menu(toolbar->zoom, (GtkWidget *) ags_zoom_menu_new());
  gtk_option_menu_set_history(toolbar->zoom, 6);
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) toolbar->zoom, NULL , NULL);

  label = (GtkLabel *) gtk_label_new("tact\0");
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) label, NULL, NULL);

  toolbar->tact_history = 4;
  toolbar->tact = (GtkOptionMenu *) gtk_option_menu_new();
  gtk_option_menu_set_menu(toolbar->tact, (GtkWidget *) ags_tact_menu_new());
  gtk_option_menu_set_history(toolbar->tact, 4);
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) toolbar->tact, NULL, NULL);

  label = (GtkLabel *) gtk_label_new("mode\0");
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) label, NULL, NULL);

  menu = (GtkMenu *) gtk_menu_new();

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(AGS_TOOLBAR_MODE_SINGLE_CHANNEL);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(AGS_TOOLBAR_MODE_GROUP_CHANNELS);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  toolbar->mode = (GtkOptionMenu *) gtk_option_menu_new();
  gtk_option_menu_set_menu(toolbar->mode, (GtkWidget *) menu);
  gtk_toolbar_append_widget((GtkToolbar *) toolbar, (GtkWidget *) toolbar->mode, NULL, NULL);
}

void
ags_toolbar_connect(AgsToolbar *toolbar)
{
  GList *list;

  g_signal_connect((GObject *) toolbar, "destroy\0",
		   G_CALLBACK(ags_toolbar_destroy_callback), (gpointer) toolbar);

  g_signal_connect((GObject *) toolbar, "show\0",
		   G_CALLBACK(ags_toolbar_show_callback), (gpointer) toolbar);

  g_signal_connect_after((GObject *) toolbar->position, "toggled\0",
			 G_CALLBACK(ags_toolbar_position_callback), (gpointer) toolbar);

  g_signal_connect_after((GObject *) toolbar->edit, "toggled\0",
			 G_CALLBACK(ags_toolbar_edit_callback), (gpointer) toolbar);

  g_signal_connect_after((GObject *) toolbar->clear, "toggled\0",
			 G_CALLBACK(ags_toolbar_clear_callback), (gpointer) toolbar);

  g_signal_connect_after((GObject *) toolbar->select, "toggled\0",
			 G_CALLBACK(ags_toolbar_select_callback), (gpointer) toolbar);

  g_signal_connect((GObject *) toolbar->copy, "clicked\0",
		   G_CALLBACK(ags_toolbar_copy_or_cut_callback), (gpointer) toolbar);

  g_signal_connect((GObject *) toolbar->cut, "clicked\0",
		   G_CALLBACK(ags_toolbar_copy_or_cut_callback), (gpointer) toolbar);

  g_signal_connect((GObject *) toolbar->paste, "clicked\0",
		   G_CALLBACK(ags_toolbar_paste_callback), (gpointer) toolbar);

  g_signal_connect_after((GObject *) toolbar->zoom, "changed\0",
			 G_CALLBACK(ags_toolbar_zoom_callback), (gpointer) toolbar);

  g_signal_connect_after((GObject *) toolbar->tact, "changed\0",
			 G_CALLBACK(ags_toolbar_tact_callback), (gpointer) toolbar);

  list = gtk_container_get_children((GtkContainer *) gtk_option_menu_get_menu(toolbar->mode));
  g_signal_connect((GObject *) list->data, "activate\0",
		   G_CALLBACK(ags_toolbar_mode_default_callback), (gpointer) toolbar);

  list = list->next;
  g_signal_connect((GObject *) list->data, "activate\0",
		   G_CALLBACK(ags_toolbar_mode_group_channels_callback), (gpointer) toolbar);
}

void
ags_toolbar_destroy(GtkObject *object)
{
}

void
ags_toolbar_show(GtkWidget *widget)
{
  /*
  GList *list;

  list = gtk_container_get_children((GtkContainer *) widget);

  while(list != NULL){
    gtk_widget_show_all((GtkWidget *) widget);

    list = list->next;
  }
  */
}

AgsToolbar*
ags_toolbar_new()
{
  AgsToolbar *toolbar;

  toolbar = (AgsToolbar *) g_object_new(AGS_TYPE_TOOLBAR, NULL);

  return(toolbar);
}
