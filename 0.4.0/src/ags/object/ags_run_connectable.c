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

#include <ags/object/ags_run_connectable.h>

#include <stdio.h>

void ags_run_connectable_base_init(AgsRunConnectableInterface *interface);

GType
ags_run_connectable_get_type()
{
  static GType ags_type_run_connectable = 0;

  if(!ags_type_run_connectable){
    static const GTypeInfo ags_run_connectable_info = {
      sizeof(AgsRunConnectableInterface),
      (GBaseInitFunc) ags_run_connectable_base_init,
      NULL, /* base_finalize */
    };

    ags_type_run_connectable = g_type_register_static(G_TYPE_INTERFACE,
						      "AgsRunConnectable\0", &ags_run_connectable_info,
						      0);
  }

  return(ags_type_run_connectable);
}

void
ags_run_connectable_base_init(AgsRunConnectableInterface *interface)
{
  /* empty */
}

void
ags_run_connectable_connect(AgsRunConnectable *connectable)
{
  AgsRunConnectableInterface *connectable_interface;

  g_return_if_fail(AGS_IS_RUN_CONNECTABLE(connectable));
  connectable_interface = AGS_RUN_CONNECTABLE_GET_INTERFACE(connectable);
  g_return_if_fail(connectable_interface->connect);
  connectable_interface->connect(connectable);
}

void
ags_run_connectable_disconnect(AgsRunConnectable *connectable)
{
  AgsRunConnectableInterface *connectable_interface;

  g_return_if_fail(AGS_IS_RUN_CONNECTABLE(connectable));
  connectable_interface = AGS_RUN_CONNECTABLE_GET_INTERFACE(connectable);
  g_return_if_fail(connectable_interface->disconnect);
  connectable_interface->connect(connectable);
}
