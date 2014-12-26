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

#ifndef __AGS_LINE_CALLBACKS_H__
#define __AGS_LINE_CALLBACKS_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/X/ags_line.h>

int ags_line_parent_set_callback(GtkWidget *widget, GtkObject *old_parent, AgsLine *line);

void ags_line_remove_recall_callback(AgsRecall *recall, AgsLine *line);

int ags_line_group_clicked_callback(GtkWidget *widget, AgsLine *line);

/* AgsLine */
void ags_line_volume_callback(GtkRange *range,
			      AgsLine *line);

/* AgsRecall - recall */
void ags_line_peak_run_post_callback(AgsRecall *peak_channel,
				     AgsLine *line);

void ags_line_copy_pattern_done(AgsRecall *recall,
				AgsLine *line);
void ags_line_copy_pattern_cancel(AgsRecall *recall,
				  AgsLine *line);
void ags_line_recall_volume_done(AgsRecall *recall,
				 AgsLine *line);
void ags_line_recall_volume_cancel(AgsRecall *recall,
				   AgsLine *line);

/* AgsRecall - play */
void ags_line_channel_done_callback(AgsChannel *channel,
				    AgsLine *line);
void ags_line_play_channel_run_cancel(AgsRecall *recall,
				      AgsLine *line);
void ags_line_play_volume_done(AgsRecall *recall,
			       AgsLine *line);
void ags_line_play_volume_cancel(AgsRecall *recall,
				 AgsLine *line);

#endif /*__AGS_LINE_CALLBACKS_H__*/
