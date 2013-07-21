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

#ifndef __AGS_NOTATION_H__
#define __AGS_NOTATION_H__

#include <glib-object.h>
#include <libxml/tree.h>

#include <ags/audio/ags_note.h>

#define AGS_TYPE_NOTATION                (ags_notation_get_type())
#define AGS_NOTATION(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_NOTATION, AgsNotation))
#define AGS_NOTATION_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_NOTATION, AgsNotationClass))
#define AGS_IS_NOTATION(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_NOTATION))
#define AGS_IS_NOTATION_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_NOTATION))
#define AGS_NOTATION_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_NOTATION, AgsNotationClass))

typedef struct _AgsNotation AgsNotation;
typedef struct _AgsNotationClass AgsNotationClass;

typedef enum{
  AGS_NOTATION_RASTER            =  1,
  AGS_NOTATION_DEFAULT_LENGTH    =  1 <<  1,
}AgsNotationFlags;

struct _AgsNotation
{
  GObject object;

  guint flags;

  guint audio_channel;
  GObject *audio;

  guint base_frequency;

  char *tact;
  guint bpm;

  guint maximum_note_length;

  GList *notes;
  GList *start_loop;
  GList *end_loop;
  guint offset;

  GList *selection;
};

struct _AgsNotationClass
{
  GObjectClass object;
};

GType ags_notation_get_type();

void ags_notation_add_note(AgsNotation *notation,
			   AgsNote *note,
			   gboolean use_selection_list);

gboolean ags_notation_remove_note_at_position(AgsNotation *notation,
					      guint x, guint y);

GList* ags_notation_get_selection(AgsNotation *notation);

gboolean ags_notation_is_note_selected(AgsNotation *notation, AgsNote *note);

AgsNote* ags_notation_find_point(AgsNotation *notation,
				 guint x, guint y,
				 gboolean use_selection_list);
GList* ags_notation_find_region(AgsNotation *notation,
				guint x0, guint y0,
				guint x1, guint y1,
				gboolean use_selection_list);

void ags_notation_free_selection(AgsNotation *notation);

void ags_notation_add_point_to_selection(AgsNotation *notation,
					 guint x, guint y,
					 gboolean replace_current_selection);
void ags_notation_remove_point_from_selection(AgsNotation *notation,
					      guint x, guint y);

void ags_notation_add_region_to_selection(AgsNotation *notation,
					  guint x0, guint y0,
					  guint x1, guint y1,
					  gboolean replace_current_selection);
void ags_notation_remove_region_from_selection(AgsNotation *notation,
					       guint x0, guint y0,
					       guint x1, guint y1);

xmlNodePtr ags_notation_copy_selection(AgsNotation *notation);
xmlNodePtr ags_notation_cut_selection(AgsNotation *notation);

void ags_notation_insert_from_clipboard(AgsNotation *notation,
					xmlNodePtr content,
					gboolean reset_x_offset, guint x_offset,
					gboolean reset_y_offset, guint y_offset);

AgsNotation* ags_notation_new(guint audio_channel);

#endif /*__AGS_NOTATION_H__*/
