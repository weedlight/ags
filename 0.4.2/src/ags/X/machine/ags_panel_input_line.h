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

#ifndef __AGS_PANEL_INPUT_LINE_H__
#define __AGS_PANEL_INPUT_LINE_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/X/ags_line.h>

#define AGS_TYPE_PANEL_INPUT_LINE                (ags_panel_input_line_get_type())
#define AGS_PANEL_INPUT_LINE(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_PANEL_INPUT_LINE, AgsPanelInputLine))
#define AGS_PANEL_INPUT_LINE_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_PANEL_INPUT_LINE, AgsPanelInputLineClass))
#define AGS_IS_PANEL_INPUT_LINE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_PANEL_INPUT_LINE))
#define AGS_IS_PANEL_INPUT_LINE_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_PANEL_INPUT_LINE))
#define AGS_PANEL_INPUT_LINE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_PANEL_INPUT_LINE, AgsPanelInputLineClass))

typedef struct _AgsPanelInputLine AgsPanelInputLine;
typedef struct _AgsPanelInputLineClass AgsPanelInputLineClass;

struct _AgsPanelInputLine
{
  AgsLine line;

  gchar *name;
  gchar *xml_type;
};

struct _AgsPanelInputLineClass
{
  AgsLineClass line;
};

GType ags_panel_input_line_get_type();

void ags_panel_input_line_map_recall(AgsPanelInputLine *panel_input_line,
				    guint output_pad_start);

AgsPanelInputLine* ags_panel_input_line_new(AgsChannel *channel);

#endif /*__AGS_PANEL_INPUT_LINE_H__*/
