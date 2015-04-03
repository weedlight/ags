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

#include <ags/audio/task/ags_remove_point_from_selection.h>

#include <ags-lib/object/ags_connectable.h>

void ags_remove_point_from_selection_class_init(AgsRemovePointFromSelectionClass *remove_point_from_selection);
void ags_remove_point_from_selection_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_remove_point_from_selection_init(AgsRemovePointFromSelection *remove_point_from_selection);
void ags_remove_point_from_selection_connect(AgsConnectable *connectable);
void ags_remove_point_from_selection_disconnect(AgsConnectable *connectable);
void ags_remove_point_from_selection_finalize(GObject *gobject);

void ags_remove_point_from_selection_launch(AgsTask *task);

static gpointer ags_remove_point_from_selection_parent_class = NULL;
static AgsConnectableInterface *ags_remove_point_from_selection_parent_connectable_interface;

GType
ags_remove_point_from_selection_get_type()
{
  static GType ags_type_remove_point_from_selection = 0;

  if(!ags_type_remove_point_from_selection){
    static const GTypeInfo ags_remove_point_from_selection_info = {
      sizeof (AgsRemovePointFromSelectionClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_remove_point_from_selection_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRemovePointFromSelection),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_remove_point_from_selection_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_remove_point_from_selection_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_remove_point_from_selection = g_type_register_static(AGS_TYPE_TASK,
								  "AgsRemovePointFromSelection\0",
								  &ags_remove_point_from_selection_info,
								  0);
    
    g_type_add_interface_static(ags_type_remove_point_from_selection,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_remove_point_from_selection);
}

void
ags_remove_point_from_selection_class_init(AgsRemovePointFromSelectionClass *remove_point_from_selection)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_remove_point_from_selection_parent_class = g_type_class_peek_parent(remove_point_from_selection);

  /* gobject */
  gobject = (GObjectClass *) remove_point_from_selection;

  gobject->finalize = ags_remove_point_from_selection_finalize;

  /* task */
  task = (AgsTaskClass *) remove_point_from_selection;

  task->launch = ags_remove_point_from_selection_launch;
}

void
ags_remove_point_from_selection_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_remove_point_from_selection_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_remove_point_from_selection_connect;
  connectable->disconnect = ags_remove_point_from_selection_disconnect;
}

void
ags_remove_point_from_selection_init(AgsRemovePointFromSelection *remove_point_from_selection)
{
  remove_point_from_selection->notation = NULL;
  remove_point_from_selection->x = 0;
  remove_point_from_selection->y = 0;
}

void
ags_remove_point_from_selection_connect(AgsConnectable *connectable)
{
  ags_remove_point_from_selection_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_remove_point_from_selection_disconnect(AgsConnectable *connectable)
{
  ags_remove_point_from_selection_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_remove_point_from_selection_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_remove_point_from_selection_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_remove_point_from_selection_launch(AgsTask *task)
{
  AgsRemovePointFromSelection *remove_point_from_selection;

  remove_point_from_selection = AGS_REMOVE_POINT_FROM_SELECTION(task);

  /* add note */
  ags_notation_remove_point_from_selection(remove_point_from_selection->notation,
					   remove_point_from_selection->x, remove_point_from_selection->y);
}

AgsRemovePointFromSelection*
ags_remove_point_from_selection_new(AgsNotation *notation,
				  guint x, guint y)
{
  AgsRemovePointFromSelection *remove_point_from_selection;

  remove_point_from_selection = (AgsRemovePointFromSelection *) g_object_new(AGS_TYPE_REMOVE_POINT_FROM_SELECTION,
									     NULL);

  remove_point_from_selection->notation = notation;
  remove_point_from_selection->x = x;
  remove_point_from_selection->y = y;

  return(remove_point_from_selection);
}
