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

#include <ags/X/machine/ags_drum_input_pad_callbacks.h>
#include <ags/X/machine/ags_drum_input_line_callbacks.h>
#include <ags/X/machine/ags_drum.h>

#include <ags/main.h>

#include <ags/thread/ags_audio_loop.h>
#include <ags/thread/ags_task_thread.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_pattern.h>
#include <ags/audio/ags_recall.h>

#include <ags/audio/task/ags_start_devout.h>
#include <ags/audio/task/ags_init_channel.h>
#include <ags/audio/task/ags_append_channel.h>
#include <ags/audio/task/ags_append_recall.h>
#include <ags/audio/task/ags_add_audio_signal.h>
#include <ags/audio/task/ags_open_single_file.h>
#include <ags/audio/task/ags_cancel_channel.h>

#include <ags/audio/recall/ags_play_channel_run.h>
#include <ags/audio/recall/ags_play_audio_signal.h>
#include <ags/audio/recall/ags_stream_audio_signal.h>

#include <ags/audio/file/ags_audio_file.h>

#include <ags/X/ags_line_callbacks.h>

#include <math.h>

void ags_drum_input_pad_open_play_callback(GtkToggleButton *toggle_button, AgsDrumInputPad *pad);
void ags_drum_input_pad_open_play_done(AgsRecall *recall, AgsDrumInputPad *drum_input_pad);
void ags_drum_input_pad_open_response_callback(GtkWidget *widget, gint response, AgsDrumInputPad *pad);

void ags_drum_input_pad_init_channel_launch_callback(AgsTask *task, AgsDrumInputPad *drum_input_pad);

#define AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME "AgsDrumInputPadOpenAudioFileName\0"
#define AGS_DRUM_INPUT_PAD_OPEN_SPIN_BUTTON "AgsDrumInputPadOpenSpinButton\0"

void
ags_drum_input_pad_open_callback(GtkWidget *widget, AgsDrumInputPad *drum_input_pad)
{
  GtkFileChooserDialog *file_chooser;
  GtkHBox *hbox;
  GtkLabel *label;
  GtkSpinButton *spin_button;
  GtkToggleButton *play;

  if(drum_input_pad->file_chooser != NULL)
    return;

  drum_input_pad->file_chooser =
    file_chooser = (GtkFileChooserDialog *) gtk_file_chooser_dialog_new (g_strdup("Open File\0"),
									 (GtkWindow *) gtk_widget_get_toplevel((GtkWidget *) drum_input_pad),
									 GTK_FILE_CHOOSER_ACTION_OPEN,
									 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
									 NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser), FALSE);
  g_object_set_data((GObject *) file_chooser, (char *) g_type_name(AGS_TYPE_AUDIO_FILE), NULL);
  g_object_set_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME, NULL);

  hbox = (GtkHBox *) gtk_hbox_new(FALSE, 0);
  gtk_file_chooser_set_extra_widget((GtkFileChooser *) file_chooser, (GtkWidget *) hbox);
  
  label = (GtkLabel *) gtk_label_new(g_strdup("channel: \0"));
  gtk_box_pack_start((GtkBox *) hbox, (GtkWidget *) label, FALSE, FALSE, 0);

  spin_button = (GtkSpinButton *) gtk_spin_button_new_with_range(0.0, 16.0, 1.0);
  g_object_set_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_SPIN_BUTTON, spin_button);
  gtk_box_pack_start((GtkBox *) hbox, (GtkWidget *) spin_button, FALSE, FALSE, 0);

  if(drum_input_pad->pad.group->active)
    gtk_widget_set_sensitive((GtkWidget *) spin_button, FALSE);

  play = (GtkToggleButton *) g_object_new(GTK_TYPE_TOGGLE_BUTTON,
					  "label\0", GTK_STOCK_MEDIA_PLAY,
					  "use-stock\0", TRUE,
					  "use-underline\0", TRUE,
					  NULL);
  gtk_box_pack_start((GtkBox *) GTK_DIALOG(file_chooser)->action_area, (GtkWidget *) play, FALSE, FALSE, 0);
  gtk_box_reorder_child((GtkBox *) GTK_DIALOG(file_chooser)->action_area, (GtkWidget *) play, 0);

  gtk_widget_show_all((GtkWidget *) file_chooser);

  g_signal_connect((GObject *) file_chooser, "response\0",
		   G_CALLBACK(ags_drum_input_pad_open_response_callback), (gpointer) drum_input_pad);
  g_signal_connect((GObject *) play, "toggled\0",
		   G_CALLBACK(ags_drum_input_pad_open_play_callback), (gpointer) drum_input_pad);
}

void
ags_drum_input_pad_open_play_callback(GtkToggleButton *toggle_button, AgsDrumInputPad *drum_input_pad)
{
  GtkFileChooserDialog *file_chooser;
  AgsDevout *devout;
  GList *list, *tasks;
  gchar *name0, *name1;

  file_chooser = drum_input_pad->file_chooser;
  name0 = (gchar *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
  name1 = g_object_get_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME);

  devout = AGS_DEVOUT(AGS_AUDIO(AGS_PAD(drum_input_pad)->channel->audio)->devout);

  if(toggle_button->active){
    AgsDevoutPlay *devout_play;
    AgsPlayAudioSignal *play_audio_signal;
    AgsStreamAudioSignal *stream_audio_signal;
    AgsStartDevout *start_devout;
    AgsAppendRecall *append_recall;
    AgsAudioFile *audio_file;
    GList *audio_signal;
    guint i;

    drum_input_pad->flags &= (~AGS_DRUM_INPUT_PAD_OPEN_PLAY_DONE);

    /* AgsAudioFile */
    if(!g_strcmp0(name0, name1)){
      audio_file = (AgsAudioFile *) g_object_get_data((GObject *) file_chooser, g_type_name(AGS_TYPE_AUDIO_FILE));
    }else{
      if(name1 != NULL){
	audio_file = g_object_get_data((GObject *) file_chooser, (char *) g_type_name(AGS_TYPE_AUDIO_FILE));
	g_object_unref(G_OBJECT(audio_file));
      }

      audio_file = ags_audio_file_new(name0,
				      devout,
				      0, devout->pcm_channels);
      g_object_set_data((GObject *) file_chooser, (char *) g_type_name(AGS_TYPE_AUDIO_FILE), audio_file);

      ags_audio_file_open(audio_file);
      ags_audio_file_read_audio_signal(audio_file);
      g_message("ags_drum_input_pad_open_play:\0");
    }

    /* task */
    audio_signal = audio_file->audio_signal;

    tasks = NULL;
    i = 0;

    while(audio_signal != NULL){
      /* AgsPlayAudioSignal recall */
      play_audio_signal = ags_play_audio_signal_new(AGS_AUDIO_SIGNAL(audio_signal->data),
						    devout,
						    i);
      AGS_AUDIO_SIGNAL(audio_signal->data)->stream_current = AGS_AUDIO_SIGNAL(audio_signal->data)->stream_beginning;
      drum_input_pad->pad_open_play_ref++;

      drum_input_pad->pad_open_recalls = g_list_prepend(drum_input_pad->pad_open_recalls,
							play_audio_signal);
      g_signal_connect(G_OBJECT(play_audio_signal), "done\0",
		       G_CALLBACK(ags_drum_input_pad_open_play_done), drum_input_pad);

      /* AgsAppendRecall */
      devout_play = ags_devout_play_alloc();

      devout_play->flags = AGS_DEVOUT_PLAY_PLAYBACK;
      devout_play->source = G_OBJECT(play_audio_signal);

      append_recall = ags_append_recall_new(G_OBJECT(AGS_MAIN(devout->ags_main)->main_loop),
					    devout_play);
    
      tasks = g_list_prepend(tasks, append_recall);
      
      /* AgsStreamAudioSignal recall */
      stream_audio_signal = ags_stream_audio_signal_new(AGS_AUDIO_SIGNAL(audio_signal->data));
      drum_input_pad->pad_open_play_ref++;

      drum_input_pad->pad_open_recalls = g_list_prepend(drum_input_pad->pad_open_recalls,
							stream_audio_signal);
      g_signal_connect(G_OBJECT(stream_audio_signal), "done\0",
		       G_CALLBACK(ags_drum_input_pad_open_play_done), drum_input_pad);

      /* AgsAppendRecall */
      devout_play = ags_devout_play_alloc();

      devout_play->flags = AGS_DEVOUT_PLAY_PLAYBACK;
      devout_play->source = G_OBJECT(stream_audio_signal);

      append_recall = ags_append_recall_new(G_OBJECT(AGS_MAIN(devout->ags_main)->main_loop),
					    devout_play);
    
      tasks = g_list_prepend(tasks, append_recall);

      /* next */
      audio_signal = audio_signal->next;
      i++;
    }

    /*  */
    tasks = g_list_reverse(tasks);

    /* create start task */
    start_devout = ags_start_devout_new(devout);
    tasks = g_list_prepend(tasks, start_devout);

    ags_task_thread_append_tasks(AGS_TASK_THREAD(AGS_AUDIO_LOOP(AGS_MAIN(devout->ags_main)->main_loop)->task_thread),
				 tasks);
  }else{
    if((AGS_DRUM_INPUT_PAD_OPEN_PLAY_DONE & (drum_input_pad->flags)) == 0){
      GList *list;

      list = drum_input_pad->pad_open_recalls;

      while(list != NULL){
	ags_recall_cancel(AGS_RECALL(list->data));

	list = list->next;
      }
    }else{
      drum_input_pad->flags &= (~AGS_DRUM_INPUT_PAD_OPEN_PLAY_DONE);
    }
  }
}

void
ags_drum_input_pad_open_play_done(AgsRecall *recall, AgsDrumInputPad *drum_input_pad)
{
  drum_input_pad->pad_open_recalls = g_list_remove(drum_input_pad->pad_open_recalls,
						   recall);
  recall->flags |= AGS_RECALL_REMOVE;

  drum_input_pad->pad_open_play_ref--;

  if(drum_input_pad->pad_open_play_ref == 0){
    GtkToggleButton *toggle_button;
    GList *list;

    list = gtk_container_get_children((GtkContainer *) GTK_DIALOG(drum_input_pad->file_chooser)->action_area);
    toggle_button = (GtkToggleButton *) list->data;

    drum_input_pad->flags |= AGS_DRUM_INPUT_PAD_OPEN_PLAY_DONE;
    gtk_toggle_button_set_active(toggle_button, FALSE);

    g_list_free(list);
  }
}

void
ags_drum_input_pad_open_response_callback(GtkWidget *widget, gint response, AgsDrumInputPad *drum_input_pad)
{
  AgsDrum *drum;
  AgsAudioFile *audio_file;
  GtkFileChooserDialog *file_chooser;
  GtkSpinButton *spin_button;
  AgsOpenSingleFile *open_single_file;
  char *name0, *name1;

  drum = (AgsDrum *) gtk_widget_get_ancestor(GTK_WIDGET(drum_input_pad), AGS_TYPE_DRUM);

  file_chooser = drum_input_pad->file_chooser;

  if(response == GTK_RESPONSE_ACCEPT){
    name0 = gtk_file_chooser_get_filename((GtkFileChooser *) file_chooser);
    name1 = (char *) g_object_get_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME);

    spin_button = (GtkSpinButton *) g_object_get_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_SPIN_BUTTON);

    /* open audio file and read audio signal */
    if(!g_strcmp0(name0, name1)){
      audio_file = (AgsAudioFile *) g_object_get_data((GObject *) file_chooser, g_type_name(AGS_TYPE_AUDIO_FILE));
    }else{
      if(name1 != NULL){
	audio_file = (AgsAudioFile *) g_object_get_data((GObject *) file_chooser, g_type_name(AGS_TYPE_AUDIO_FILE));
	g_object_unref(G_OBJECT(audio_file));
      }
    }

    /* task */
    if(AGS_PAD(drum_input_pad)->group->active){
      open_single_file = ags_open_single_file_new(AGS_PAD(drum_input_pad)->channel,
						  AGS_DEVOUT(AGS_AUDIO(AGS_MACHINE(drum)->audio)->devout),
						  name0,
						  0, AGS_AUDIO(AGS_MACHINE(drum)->audio)->audio_channels);
    }else{
      AgsLine *line;
      GList *list;
      
      list = gtk_container_get_children(GTK_CONTAINER(AGS_PAD(drum_input_pad)->expander_set));
      line = AGS_LINE(ags_line_find_next_grouped(list)->data);

      open_single_file = ags_open_single_file_new(line->channel,
						  AGS_DEVOUT(AGS_AUDIO(AGS_MACHINE(drum)->audio)->devout),
						  name0,
						  (guint) spin_button->adjustment->value, 1);

      g_list_free(list);
    }

    ags_task_thread_append_task(AGS_TASK_THREAD(AGS_AUDIO_LOOP(AGS_MAIN(AGS_DEVOUT(AGS_AUDIO(AGS_MACHINE(drum)->audio)->devout)->ags_main)->main_loop)->task_thread),
									AGS_TASK(open_single_file));

    gtk_widget_destroy((GtkWidget *) file_chooser);
  }else if(response == GTK_RESPONSE_CANCEL){
    audio_file = (AgsAudioFile *) g_object_get_data((GObject *) file_chooser, g_type_name(AGS_TYPE_AUDIO_FILE));

    if(audio_file != NULL){
      g_object_unref(G_OBJECT(audio_file));
    }

    gtk_widget_destroy((GtkWidget *) file_chooser);
  }

  drum_input_pad->file_chooser = NULL;
}

void
ags_drum_input_pad_play_callback(GtkToggleButton *toggle_button, AgsDrumInputPad *drum_input_pad)
{
  AgsDrum *drum;
  AgsDevout *devout;
  AgsAudioLoop *audio_loop;
  AgsTaskThread *task_thread;
  AgsDevoutThread *devout_thread;
  AgsChannel *channel;
  GList *tasks;

  drum = (AgsDrum *) gtk_widget_get_ancestor((GtkWidget *) drum_input_pad, AGS_TYPE_DRUM);

  devout = AGS_DEVOUT(AGS_MACHINE(drum)->audio->devout);

  audio_loop = AGS_AUDIO_LOOP(AGS_MAIN(devout->ags_main)->main_loop);
  task_thread = AGS_TASK_THREAD(audio_loop->task_thread);
  devout_thread = AGS_DEVOUT_THREAD(audio_loop->devout_thread);

  tasks = NULL;

  if(toggle_button->active){
    AgsStartDevout *start_devout;
    AgsInitChannel *init_channel;
    AgsAppendChannel *append_channel;
    gboolean play_all;
    guint flags;

    play_all = AGS_PAD(drum_input_pad)->group->active;

    flags = AGS_AUDIO_SIGNAL_STANDALONE;

    channel = AGS_PAD(drum_input_pad)->channel;

    /* init channel for playback */
    init_channel = ags_init_channel_new(channel, play_all,
					TRUE, FALSE, FALSE);
    g_signal_connect_after(G_OBJECT(init_channel), "launch\0",
			   G_CALLBACK(ags_drum_input_pad_init_channel_launch_callback), drum_input_pad);
    tasks = g_list_prepend(tasks, init_channel);

    if(play_all){
      AgsChannel *next_pad;

      next_pad = channel->next_pad;

      while(channel != next_pad){
	/* append channel for playback */
	append_channel = ags_append_channel_new(G_OBJECT(audio_loop),
						channel);
	tasks = g_list_prepend(tasks, append_channel);

	//	drum_input_pad->pad_play_ref++;
	channel = channel->next;
      }
    }else{
      AgsLine *line;
      GList *list;

      list = gtk_container_get_children(GTK_CONTAINER(AGS_PAD(drum_input_pad)->expander_set));
      line = AGS_LINE(ags_line_find_next_grouped(list)->data);

      /* append channel for playback */
      append_channel = ags_append_channel_new(G_OBJECT(audio_loop),
					      line->channel);
      tasks = g_list_prepend(tasks, append_channel);

      g_list_free(list);
      //      drum_input_pad->pad_play_ref++;
    }

    /* create start task */
    start_devout = ags_start_devout_new(devout);
    tasks = g_list_prepend(tasks,
			   start_devout);

    /* perform playback */
    tasks = g_list_reverse(tasks);
    ags_task_thread_append_tasks(task_thread, tasks);
  }else{
    AgsCancelChannel *cancel_channel;

    channel = AGS_PAD(drum_input_pad)->channel;

    if(AGS_DEVOUT_PLAY(channel->devout_play)->recall_id[0] == NULL ||
       (AGS_DEVOUT_PLAY_DONE & (AGS_DEVOUT_PLAY(channel->devout_play)->recall_id[0]->flags)) != 0){
      return;
    }

    if((AGS_DEVOUT_PLAY_PAD & (AGS_DEVOUT_PLAY(channel->devout_play)->flags)) != 0){
      AgsChannel *next_pad;

      next_pad = channel->next_pad;

      if((AGS_DEVOUT_PLAY_DONE & (AGS_DEVOUT_PLAY(channel->devout_play)->flags)) == 0){
	/* cancel request */
	while(channel != next_pad){
	  cancel_channel = ags_cancel_channel_new(channel, AGS_DEVOUT_PLAY(channel->devout_play)->recall_id[0],
						  AGS_DEVOUT_PLAY(channel->devout_play));

	  ags_task_thread_append_task(task_thread, (AgsTask *) cancel_channel);

	  channel = channel->next;
	}
      }else{
	/* done */
	while(channel != next_pad){
	  AGS_DEVOUT_PLAY(channel->devout_play)->flags |= AGS_DEVOUT_PLAY_REMOVE;
	  AGS_DEVOUT_PLAY(channel->devout_play)->flags &= (~AGS_DEVOUT_PLAY_DONE);

	  channel = channel->next;
	}
      }
    }else{
      AgsLine *line;
      GList *list;

      list = gtk_container_get_children(GTK_CONTAINER(AGS_PAD(drum_input_pad)->expander_set));
      line = AGS_LINE(ags_line_find_next_grouped(list)->data);

      g_list_free(list);

      /*  */
      channel = line->channel;

      if((AGS_DEVOUT_PLAY_DONE & (AGS_DEVOUT_PLAY(channel->devout_play)->flags)) == 0){
	/* cancel request */
	cancel_channel = ags_cancel_channel_new(channel, AGS_DEVOUT_PLAY(channel->devout_play)->recall_id[0],
						AGS_DEVOUT_PLAY(channel->devout_play));

	ags_task_thread_append_task(task_thread, (AgsTask *) cancel_channel);
      }else{
	/* done */
	AGS_DEVOUT_PLAY(channel->devout_play)->flags |= AGS_DEVOUT_PLAY_REMOVE;
	AGS_DEVOUT_PLAY(channel->devout_play)->flags &= (~AGS_DEVOUT_PLAY_DONE);
      }
    }
  }
}

void
ags_drum_input_pad_init_channel_launch_callback(AgsTask *task,
						AgsDrumInputPad *drum_input_pad)
{
  AgsChannel *channel, *next_pad;
  AgsRecycling *recycling;
  AgsAddAudioSignal *add_audio_signal;
  GList *recall, *tmp;
  GList *list, *list_start;

  list_start = 
    list = gtk_container_get_children(AGS_PAD(drum_input_pad)->expander_set);
  
  channel = AGS_PAD(drum_input_pad)->channel;
  next_pad = channel->next_pad;

  while(channel != next_pad){
    if(AGS_DEVOUT_PLAY(channel->devout_play) == NULL ||
       AGS_DEVOUT_PLAY(channel->devout_play)->recall_id[0] == NULL){
      channel = channel->next;
      list = list->next;

      continue;
    }

    /* connect done */
    recall = ags_recall_find_provider_with_recycling_container(channel->play,
							       channel,
							       AGS_DEVOUT_PLAY(channel->devout_play)->recall_id[0]->recycling_container);

    tmp = recall;
    recall = ags_recall_find_type(recall,
				  AGS_TYPE_PLAY_CHANNEL_RUN);
    //TODO:JK: fix me
    //    g_list_free(tmp);
    
    if(recall != NULL){
      g_signal_connect_after(channel, "done\0",
			     G_CALLBACK(ags_line_channel_done_callback), list->data);

      /* add audio signal */
      recycling = channel->first_recycling;

      while(recycling != channel->last_recycling->next){
	add_audio_signal = ags_add_audio_signal_new(recycling,
						    NULL,
						    AGS_AUDIO(channel->audio)->devout,
						    AGS_RECALL(recall->data)->recall_id,
						    0);

	recycling = recycling->next;
      }    
    }

    //TODO:JK: fix me
    //    g_list_free(recall);

    channel = channel->next;
    list = list->next;
  }

  g_list_free(list_start);
}

void
ags_drum_input_pad_edit_callback(GtkWidget *toggle_button, AgsDrumInputPad *drum_input_pad)
{
  GtkToggleButton *toggle;
  AgsDrum *drum;

  drum = (AgsDrum *) gtk_widget_get_ancestor((GtkWidget *) drum_input_pad, AGS_TYPE_DRUM);

  if(drum->selected_edit_button != NULL){
    if(GTK_TOGGLE_BUTTON(toggle_button) != drum->selected_edit_button){
      toggle = drum->selected_edit_button;
      drum->selected_edit_button = NULL;
      gtk_toggle_button_set_active((GtkToggleButton *) toggle, FALSE);
      drum->selected_edit_button = (GtkToggleButton *) toggle_button;
      drum->selected_pad = (AgsDrumInputPad *) gtk_widget_get_ancestor((GtkWidget *) toggle_button, AGS_TYPE_DRUM_INPUT_PAD);
      ags_drum_set_pattern(drum);
    }else{
      toggle = drum->selected_edit_button;
      drum->selected_edit_button = NULL;
      gtk_toggle_button_set_active((GtkToggleButton *) toggle, TRUE);
      drum->selected_edit_button = toggle;
    }
  }
}
