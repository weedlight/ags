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

#include <ags/X/machine/ags_ffplayer_callbacks.h>
#include <ags/X/ags_machine_callbacks.h>

#include <ags/main.h>

#include <ags/object/ags_playable.h>
#include <ags/object/ags_main_loop.h>

#include <ags/thread/ags_audio_loop.h>

#include <ags/audio/ags_channel.h>

#include <ags/audio/task/ags_link_channel.h>
#include <ags/audio/task/ags_add_audio_signal.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_editor.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_output.h>

#include <ags/audio/file/ags_audio_file.h>
#include <ags/audio/file/ags_ipatch_sf2_reader.h>

#include <math.h>

void ags_ffplayer_open_dialog_response_callback(GtkWidget *widget, gint response,
						AgsMachine *machine);

void
ags_ffplayer_parent_set_callback(GtkWidget *widget, GtkObject *old_parent, AgsFFPlayer *ffplayer)
{
  AgsWindow *window;
  AgsAudio *audio;

  if(old_parent != NULL)
    return;

  window = (AgsWindow *) gtk_widget_get_toplevel(widget);
  audio = ffplayer->machine.audio;
  audio->devout = (GObject *) window->devout;
  
  AGS_MACHINE(ffplayer)->name = g_strdup_printf("Default %d\0",
						ags_window_find_machine_counter(window, AGS_TYPE_FFPLAYER)->counter);
  ags_window_increment_machine_counter(window,
				       AGS_TYPE_FFPLAYER);
}

void
ags_ffplayer_open_clicked_callback(GtkWidget *widget, AgsFFPlayer *ffplayer)
{
  GtkFileChooserDialog *file_chooser;

  file_chooser = ags_machine_file_chooser_dialog_new(AGS_MACHINE(ffplayer));
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser),
				       FALSE);

  g_signal_connect((GObject *) file_chooser, "response\0",
		   G_CALLBACK(ags_ffplayer_open_dialog_response_callback), AGS_MACHINE(ffplayer));

  gtk_widget_show_all((GtkWidget *) file_chooser);
}

void
ags_ffplayer_open_dialog_response_callback(GtkWidget *widget, gint response,
					   AgsMachine *machine)
{
  AgsWindow *window;
  AgsFFPlayer *ffplayer;
  GtkFileChooserDialog *file_chooser;
  AgsDevout *devout;

  window = AGS_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(machine)));
  ffplayer = AGS_FFPLAYER(machine);
  file_chooser = GTK_FILE_CHOOSER_DIALOG(widget);

  if(response == GTK_RESPONSE_ACCEPT){
    gchar *filename;

    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));

    if(g_str_has_suffix(filename, ".sf2\0")){
      AgsIpatch *ipatch;
      AgsPlayable *playable;
      gchar **preset;
      GError *error;

      /* clear preset and instrument */
      ags_combo_box_text_remove_all(ffplayer->preset);
      ags_combo_box_text_remove_all(ffplayer->instrument);

      /* Ipatch related */
      ipatch = g_object_new(AGS_TYPE_IPATCH,
			    "mode\0", AGS_IPATCH_READ,
			    "filename\0", filename,
			    NULL);
      ffplayer->ipatch = ipatch;
      ipatch->devout = window->devout;

      playable = AGS_PLAYABLE(ipatch);

      error = NULL;
      ags_playable_level_select(playable,
				0, filename,
				&error);

      /* select first preset */
      ipatch->nth_level = 1;
      preset = ags_playable_sublevel_names(playable);

      error = NULL;
      ags_playable_level_select(playable,
				1, *preset,
				&error);

      /* fill ffplayer->preset */
      while(preset != NULL && preset[0] != NULL){
	gtk_combo_box_text_append_text(ffplayer->preset,
				       *preset);

	preset++;
      }
    }
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(ffplayer->preset),
			   0);
  
  gtk_widget_destroy(widget);
}

void
ags_ffplayer_preset_changed_callback(GtkComboBox *preset, AgsFFPlayer *ffplayer)
{
  AgsPlayable *playable;
  AgsIpatch *ipatch;
  gchar *preset_name;
  gchar **instrument;
  GError *error;

  playable = AGS_PLAYABLE(ffplayer->ipatch);
  ipatch = ffplayer->ipatch;

  preset_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(preset));

  /* load presets */
  error = NULL;
  ags_playable_level_select(playable,
			    1, preset_name,
			    &error);

  /* select first instrument */
  ags_combo_box_text_remove_all(ffplayer->instrument);

  ipatch->nth_level = 2;
  instrument = ags_playable_sublevel_names(playable);
  
  error = NULL;
  ags_playable_level_select(playable,
			    2, *instrument,
			    &error);
  
  /* fill ffplayer->instrument */
  while(instrument != NULL && instrument[0] != NULL){
    gtk_combo_box_text_append_text(ffplayer->instrument,
				   *instrument);

    instrument++;
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(ffplayer->instrument),
			   0);
}

void
ags_ffplayer_instrument_changed_callback(GtkComboBox *instrument, AgsFFPlayer *ffplayer)
{
  AgsChannel *channel;
  AgsAudioSignal *audio_signal_source_old;
  AgsPlayable *playable;
  AgsLinkChannel *link_channel;
  AgsAddAudioSignal *add_audio_signal;
  gchar *instrument_name;
  gchar **preset;
  gchar **sample;
  GList *task;
  GList *list;
  guint count;
  int i;
  gboolean has_more;
  GError *error;

  playable = AGS_PLAYABLE(ffplayer->ipatch);

  instrument_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(instrument));

  /* select instrument */
  error = NULL;

  ags_playable_level_select(playable,
			    2, instrument_name,
			    &error);

  if(error != NULL){
    g_error("%s\0", error->message);
  }

  /* select first sample */
  sample = NULL;

  AGS_IPATCH(ffplayer->ipatch)->nth_level = 3;
  sample = ags_playable_sublevel_names(playable);

  error = NULL;
  ags_playable_level_select(playable,
			    3, *sample,
			    &error);

  if(error != NULL){
    g_error("%s\0", error->message);
  }

  count = 0;
  
  while(*sample != NULL){
    sample++;
    count++;
  }

  /* read all samples */
  ags_audio_set_audio_channels(AGS_MACHINE(ffplayer)->audio,
			       AGS_IPATCH_DEFAULT_CHANNELS);

  AGS_IPATCH(ffplayer->ipatch)->nth_level = 3;

  ags_playable_iter_start(playable);

  ags_audio_set_pads(AGS_MACHINE(ffplayer)->audio, AGS_TYPE_INPUT,
		     count);
  
  channel = AGS_MACHINE(ffplayer)->audio->input;
  task = NULL;
  has_more = TRUE;

  while(channel != NULL && has_more){
    list = ags_playable_read_audio_signal(playable,
					  AGS_MACHINE(ffplayer)->audio->devout,
					  channel->audio_channel, AGS_IPATCH_DEFAULT_CHANNELS);

    for(i = 0; i < AGS_IPATCH_DEFAULT_CHANNELS && list != NULL; i++){
      /* create task */
      link_channel = ags_link_channel_new(channel, NULL);
      task = g_list_prepend(task,
			    link_channel);

      AGS_AUDIO_SIGNAL(list->data)->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
      ags_recycling_add_audio_signal(channel->first_recycling,
				     AGS_AUDIO_SIGNAL(list->data));
      //      add_audio_signal = ags_add_audio_signal_new(channel->first_recycling,
      //					  AGS_AUDIO_SIGNAL(list->data),
      //					  AGS_MACHINE(ffplayer)->audio->devout,
      //					  NULL,
      //					  AGS_AUDIO_SIGNAL_TEMPLATE);
      //      task = g_list_prepend(task,
      //		    add_audio_signal);

      /* iterate */	
      channel = channel->next;
      list = list->next;
    }

    has_more = ags_playable_iter_next(playable);
  }
      
  /* append tasks */
  //  task = g_list_reverse(task);
  //  ags_task_thread_append_tasks(AGS_AUDIO_LOOP(AGS_MAIN(AGS_DEVOUT(AGS_MACHINE(ffplayer)->audio->devout)->ags_main)->main_loop)->task_thread,
  //			       task);
}

gboolean
ags_ffplayer_drawing_area_expose_callback(GtkWidget *widget, GdkEventExpose *event, AgsFFPlayer *ffplayer)
{
  ags_ffplayer_paint(ffplayer);

  return(FALSE);
}

gboolean
ags_ffplayer_drawing_area_configure_callback(GtkWidget *widget, GdkEventConfigure *event, AgsFFPlayer *ffplayer)
{
  ags_ffplayer_paint(ffplayer);

  return(FALSE);
}

gboolean
ags_ffplayer_drawing_area_button_press_callback(GtkWidget *widget, AgsFFPlayer *ffplayer)
{

  return(FALSE);
}

void
ags_ffplayer_hscrollbar_value_changed(GtkAdjustment *adjustment, AgsFFPlayer *ffplayer)
{
  ags_ffplayer_paint(ffplayer);
}
