/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2014 Joël Krähemann
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

#ifndef __AGS_LADSPA_BROWSER_CALLBACKS_H__
#define __AGS_LADSPA_BROWSER_CALLBACKS_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/X/ags_ladspa_browser.h>

void ags_ladspa_browser_plugin_filename_callback(GtkComboBoxText *combo_box,
						 AgsLadspaBrowser *ladspa_browser);
void ags_ladspa_browser_plugin_effect_callback(GtkComboBoxText *combo_box,
					       AgsLadspaBrowser *ladspa_browser);

void ags_ladspa_browser_preview_close_callback(GtkWidget *preview,
					       AgsLadspaBrowser *ladspa_browser);

int ags_ladspa_browser_ok_callback(GtkWidget *widget, AgsLadspaBrowser *ladspa_browser);
int ags_ladspa_browser_cancel_callback(GtkWidget *widget, AgsLadspaBrowser *ladspa_browser);

#endif /*__AGS_LADSPA_BROWSER_CALLBACKS_H__*/
