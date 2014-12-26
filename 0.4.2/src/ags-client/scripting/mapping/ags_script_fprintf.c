/* AGS Client - Advanced GTK Sequencer Client
 * Copyright (C) 2013 Joël Krähemann
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

#include <ags-client/scripting/mapping/ags_script_fprintf.h>

#include <ags-lib/object/ags_connectable.h>

void ags_script_fprintf_class_init(AgsScriptFPrintfClass *script_fprintf);
void ags_script_fprintf_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_script_fprintf_init(AgsScriptFPrintf *script_fprintf);
void ags_script_fprintf_connect(AgsConnectable *connectable);
void ags_script_fprintf_disconnect(AgsConnectable *connectable);
void ags_script_fprintf_finalize(GObject *gobject);

AgsScriptObject* ags_script_fprintf_launch(AgsScriptObject *script_object, GError **error);

static gpointer ags_script_fprintf_parent_class = NULL;

GType
ags_script_fprintf_get_type()
{
  static GType ags_type_script_fprintf = 0;

  if(!ags_type_script_fprintf){
    static const GTypeInfo ags_script_fprintf_info = {
      sizeof (AgsScriptFPrintfClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_script_fprintf_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsScriptFPrintf),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_script_fprintf_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_script_fprintf_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_script_fprintf = g_type_register_static(AGS_TYPE_SCRIPT_CONTROLLER,
						     "AgsScriptFPrintf\0",
						     &ags_script_fprintf_info,
						     0);
    
    g_type_add_interface_static(ags_type_script_fprintf,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_script_fprintf);
}

void
ags_script_fprintf_class_init(AgsScriptFPrintfClass *script_fprintf)
{
  AgsScriptObjectClass *script_object;
  GObjectClass *gobject;

  ags_script_fprintf_parent_class = g_type_class_peek_parent(script_fprintf);

  /* GObjectClass */
  gobject = (GObjectClass *) script_fprintf;

  gobject->finalize = ags_script_fprintf_finalize;
  
  /* AgsScriptObjectClass */
  script_object = (AgsScriptObjectClass *) script_fprintf;

  script_object->launch = ags_script_fprintf_launch;
}

void
ags_script_fprintf_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_script_fprintf_connect;
  connectable->disconnect = ags_script_fprintf_disconnect;
}

void
ags_script_fprintf_init(AgsScriptFPrintf *script_fprintf)
{
  script_fprintf->args = NULL;
}

void
ags_script_fprintf_connect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_script_fprintf_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_script_fprintf_finalize(GObject *gobject)
{
  AgsScriptFPrintf *script_fprintf;

  script_fprintf = AGS_SCRIPT_FPRINTF(gobject);

  g_object_unref(script_fprintf->args);

  G_OBJECT_CLASS(ags_script_fprintf_parent_class)->finalize(gobject);
}

AgsScriptObject*
ags_script_fprintf_launch(AgsScriptObject *script_object, GError **error)
{
  //TODO:JK: implement me

  return(script_object);
}

AgsScriptFPrintf*
ags_script_fprintf_new()
{
  AgsScriptFPrintf *script_fprintf;

  script_fprintf = (AgsScriptFPrintf *) g_object_new(AGS_TYPE_SCRIPT_FPRINTF,
						     NULL);

  return(script_fprintf);
}
