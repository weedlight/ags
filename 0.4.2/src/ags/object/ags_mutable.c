/* AGS - Advanced GTK Sequencer
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

#include <ags/object/ags_mutable.h>

void ags_mutable_base_init(AgsMutableInterface *interface);

GType
ags_mutable_get_type()
{
  static GType ags_type_mutable = 0;

  if(!ags_type_mutable){
    static const GTypeInfo ags_mutable_info = {
      sizeof(AgsMutableInterface),
      (GBaseInitFunc) ags_mutable_base_init,
      NULL, /* base_finalize */
    };

    ags_type_mutable = g_type_register_static(G_TYPE_INTERFACE,
					       "AgsMutable\0", &ags_mutable_info,
					       0);
  }

  return(ags_type_mutable);
}

void
ags_mutable_base_init(AgsMutableInterface *interface)
{
  /* empty */
}

void
ags_mutable_set_muted(AgsMutable *mutable, gboolean muted)
{
  AgsMutableInterface *mutable_interface;

  g_return_if_fail(AGS_IS_MUTABLE(mutable));
  mutable_interface = AGS_MUTABLE_GET_INTERFACE(mutable);
  g_return_if_fail(mutable_interface->set_muted);
  mutable_interface->set_muted(mutable, muted);
}
