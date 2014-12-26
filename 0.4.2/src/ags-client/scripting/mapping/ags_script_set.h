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

#ifndef __AGS_SCRIPT_SET_H__
#define __AGS_SCRIPT_SET_H__

#include <glib.h>
#include <glib-object.h>

#include <ags-client/scripting/mapping/ags_script_controller.h>
#include <ags-client/scripting/mapping/ags_script_array.h>

#define AGS_TYPE_SCRIPT_SET                (ags_script_set_get_type())
#define AGS_SCRIPT_SET(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SCRIPT_SET, AgsScriptSet))
#define AGS_SCRIPT_SET_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_SCRIPT_SET, AgsScriptSetClass))
#define AGS_IS_SCRIPT_SET(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_SCRIPT_SET))
#define AGS_IS_SCRIPT_SET_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_SCRIPT_SET))
#define AGS_SCRIPT_SET_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_SCRIPT_SET, AgsScriptSetClass))

typedef struct _AgsScriptSet AgsScriptSet;
typedef struct _AgsScriptSetClass AgsScriptSetClass;

typedef enum{
  AGS_SCRIPT_SET_EQUATION_TRUE        = 1,
  AGS_SCRIPT_SET_EQUATION_OPEN        = 1 << 1,
}AgsScriptSetFlags;

struct _AgsScriptSet
{
  AgsScriptController controller;

  guint flags;
  
  gchar *start;
  gchar *current;
};

struct _AgsScriptSetClass
{
  AgsScriptControllerClass controller;
};

GType ags_script_set_get_type();

void ags_script_set_from_string(AgsScriptSet *script_set, gchar *string);

gchar* ags_script_set_matrix_to_string(AgsScriptSet *script_set,
				       AgsScriptArray *array);
AgsScriptArray* ags_script_set_matrix_from_string(AgsScriptSet *script_set,
						  gchar *string);

gboolean ags_script_set_boolean_term(AgsScriptSet *script_set);
gint ags_script_set_int_term(AgsScriptSet *script_set);
guint ags_script_set_uint_term(AgsScriptSet *script_set);
gdouble ags_script_set_double_term(AgsScriptSet *script_set);

AgsScriptSet* ags_script_set_new();

#endif /*__AGS_SCRIPT_SET_H__*/
