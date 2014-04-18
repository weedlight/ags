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

#include <ags/X/machine/ags_drum_output_line.h>
#include <ags/X/machine/ags_drum_output_line_callbacks.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/audio/ags_recall_factory.h>

#include <ags/audio/recall/ags_delay_audio.h>
#include <ags/audio/recall/ags_delay_audio_run.h>
#include <ags/audio/recall/ags_copy_pattern_audio_run.h>
#include <ags/audio/recall/ags_loop_channel.h>
#include <ags/audio/recall/ags_loop_channel_run.h>
#include <ags/audio/recall/ags_stream_channel.h>
#include <ags/audio/recall/ags_stream_channel_run.h>

#include <ags/X/ags_window.h>

#include <ags/X/machine/ags_drum.h>

#include <math.h>

void ags_drum_output_line_class_init(AgsDrumOutputLineClass *drum_output_line);
void ags_drum_output_line_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_drum_output_line_init(AgsDrumOutputLine *drum_output_line);
void ags_drum_output_line_destroy(GtkObject *object);
void ags_drum_output_line_connect(AgsConnectable *connectable);
void ags_drum_output_line_disconnect(AgsConnectable *connectable);

void ags_drum_output_line_set_channel(AgsLine *line, AgsChannel *channel);

static gpointer ags_drum_output_line_parent_class = NULL;
static AgsConnectableInterface *ags_drum_output_line_parent_connectable_interface;

GType
ags_drum_output_line_get_type()
{
  static GType ags_type_drum_output_line = 0;

  if(!ags_type_drum_output_line){
    static const GTypeInfo ags_drum_output_line_info = {
      sizeof(AgsDrumOutputLineClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_drum_output_line_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsDrumOutputLine),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_drum_output_line_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_drum_output_line_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_drum_output_line = g_type_register_static(AGS_TYPE_LINE,
						       "AgsDrumOutputLine\0", &ags_drum_output_line_info,
						       0);

    g_type_add_interface_static(ags_type_drum_output_line,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_drum_output_line);
}

void
ags_drum_output_line_class_init(AgsDrumOutputLineClass *drum_output_line)
{
  AgsLineClass *line;

  ags_drum_output_line_parent_class = g_type_class_peek_parent(drum_output_line);

  /* AgsLineClass */
  line = AGS_LINE_CLASS(drum_output_line);

  line->set_channel = ags_drum_output_line_set_channel;
}

void
ags_drum_output_line_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_drum_output_line_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_drum_output_line_connect;
  connectable->disconnect = ags_drum_output_line_disconnect;
}

void
ags_drum_output_line_init(AgsDrumOutputLine *drum_output_line)
{
  g_signal_connect_after((GObject *) drum_output_line, "parent_set\0",
			 G_CALLBACK(ags_drum_output_line_parent_set_callback), NULL);
}

void
ags_drum_output_line_destroy(GtkObject *object)
{
}

void
ags_drum_output_line_connect(AgsConnectable *connectable)
{
  AgsDrumOutputLine *drum_output_line;

  drum_output_line = AGS_DRUM_OUTPUT_LINE(connectable);

  if((AGS_LINE_CONNECTED & (AGS_LINE(drum_output_line)->flags)) != 0){
    return;
  }
  
  ags_drum_output_line_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_drum_output_line_disconnect(AgsConnectable *connectable)
{
  ags_drum_output_line_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_drum_output_line_set_channel(AgsLine *line, AgsChannel *channel)
{
  AgsDrumOutputLine *drum_output_line;

  AGS_LINE_CLASS(ags_drum_output_line_parent_class)->set_channel(line, channel);

  drum_output_line = AGS_DRUM_OUTPUT_LINE(line);

  if(line->channel != NULL){
    line->flags &= (~AGS_LINE_MAPPED_RECALL);
  }

  if(channel != NULL){
    AgsDevout *devout;
    AgsAudioSignal *audio_signal;
    guint stop;

    devout = AGS_DEVOUT(AGS_AUDIO(channel->audio)->devout);

    stop = (guint) ceil(16.0 * AGS_DEVOUT_DEFAULT_DELAY * exp2(8.0 - 4.0) + 1.0);

    audio_signal = ags_audio_signal_new(devout,
					channel->first_recycling,
					NULL);
    audio_signal->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
    ags_audio_signal_stream_resize(audio_signal,
				   stop);
    ags_recycling_add_audio_signal(channel->first_recycling,
				   audio_signal);

    //    audio_signal = ags_audio_signal_get_template(channel->first_recycling->audio_signal);
    //    ags_audio_signal_stream_resize(audio_signal, stop);
    if((AGS_LINE_PREMAPPED_RECALL & (line->flags)) == 0){
      ags_drum_output_line_add_default_recall(AGS_DRUM_OUTPUT_LINE(line));
    }
  }
}

void
ags_drum_output_line_add_default_recall(AgsDrumOutputLine *drum_output_line)
{
  AgsDrum *drum;
  AgsLine *line;

  AgsAudio *audio;

  AgsChannel *output;
  AgsDelayAudio *recall_delay_audio;
  AgsCountBeatsAudioRun *recall_count_beats_audio_run;
  AgsLoopChannel *recall_loop_channel;
  AgsLoopChannelRun *recall_loop_channel_run;

  GList *list;

  printf("ags_drum_output_line_map_recall\n\0");

  line = AGS_LINE(drum_output_line);
  line->flags |= AGS_LINE_MAPPED_RECALL;

  output = AGS_LINE(drum_output_line)->channel;
  audio = AGS_AUDIO(output->audio);

  drum = AGS_DRUM(audio->machine);

  /* get some recalls */
  list = ags_recall_find_type(audio->play, AGS_TYPE_DELAY_AUDIO);

  if(list != NULL){
    recall_delay_audio = AGS_DELAY_AUDIO(list->data);
  }else{
    recall_delay_audio = NULL;
  }

  list = ags_recall_find_type(audio->play, AGS_TYPE_COUNT_BEATS_AUDIO_RUN);

  if(list != NULL){
    recall_count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(list->data);
  }else{
    recall_count_beats_audio_run = NULL;
  }

  /* ags-loop */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-loop\0",
			    output->audio_channel, output->audio_channel + 1,
			    output->pad, output->pad + 1,
			    (AGS_RECALL_FACTORY_OUTPUT |
			     AGS_RECALL_FACTORY_PLAY | 
			     AGS_RECALL_FACTORY_ADD),
			    0);

  list = ags_recall_find_type(output->play, AGS_TYPE_LOOP_CHANNEL);

  if(list != NULL){
    recall_loop_channel = AGS_LOOP_CHANNEL(list->data);

    /* set dependency */
    g_object_set(G_OBJECT(recall_loop_channel),
		 "delay-audio\0", recall_delay_audio,
		 NULL);
  }else{
    recall_loop_channel = NULL;
  }

  list = ags_recall_find_type(output->play, AGS_TYPE_LOOP_CHANNEL_RUN);

  if(list != NULL){
    recall_loop_channel_run = AGS_LOOP_CHANNEL_RUN(list->data);

    /* set dependency */
    g_object_set(G_OBJECT(recall_loop_channel_run),
		 "count-beats-audio-run\0", recall_count_beats_audio_run,
		 NULL);
  }else{
    recall_loop_channel_run = NULL;
  }

  /* ags-stream */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-stream\0",
			    output->audio_channel, output->audio_channel + 1,
			    output->pad, output->pad + 1,
			    (AGS_RECALL_FACTORY_OUTPUT |
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_RECALL | 
			     AGS_RECALL_FACTORY_ADD),
			    0);
}

AgsDrumOutputLine*
ags_drum_output_line_new(AgsChannel *channel)
{
  AgsDrumOutputLine *drum_output_line;

  drum_output_line = (AgsDrumOutputLine *) g_object_new(AGS_TYPE_DRUM_OUTPUT_LINE,
							"channel\0", channel,
							NULL);

  return(drum_output_line);
}
