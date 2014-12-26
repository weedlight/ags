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

#include <ags/audio/task/ags_set_samplerate.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_audio_signal.h>

void ags_set_samplerate_class_init(AgsSetSamplerateClass *set_samplerate);
void ags_set_samplerate_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_set_samplerate_init(AgsSetSamplerate *set_samplerate);
void ags_set_samplerate_connect(AgsConnectable *connectable);
void ags_set_samplerate_disconnect(AgsConnectable *connectable);
void ags_set_samplerate_finalize(GObject *gobject);

void ags_set_samplerate_launch(AgsTask *task);

void ags_set_samplerate_audio_signal(AgsSetSamplerate *set_samplerate, AgsAudioSignal *audio_signal);
void ags_set_samplerate_recycling(AgsSetSamplerate *set_samplerate, AgsRecycling *recycling);
void ags_set_samplerate_channel(AgsSetSamplerate *set_samplerate, AgsChannel *channel);
void ags_set_samplerate_audio(AgsSetSamplerate *set_samplerate, AgsAudio *audio);
void ags_set_samplerate_devout(AgsSetSamplerate *set_samplerate, AgsDevout *devout);

/**
 * SECTION:ags_set_samplerate
 * @short_description: modifies samplerate
 * @title: AgsSetSamplerate
 * @section_id:
 * @include: ags/audio/task/ags_set_samplerate.h
 *
 * The #AgsSetSamplerate task modifies samplerate of #AgsDevout.
 */

static gpointer ags_set_samplerate_parent_class = NULL;
static AgsConnectableInterface *ags_set_samplerate_parent_connectable_interface;

GType
ags_set_samplerate_get_type()
{
  static GType ags_type_set_samplerate = 0;

  if(!ags_type_set_samplerate){
    static const GTypeInfo ags_set_samplerate_info = {
      sizeof (AgsSetSamplerateClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_set_samplerate_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsSetSamplerate),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_set_samplerate_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_set_samplerate_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_set_samplerate = g_type_register_static(AGS_TYPE_TASK,
						     "AgsSetSamplerate\0",
						     &ags_set_samplerate_info,
						     0);
    
    g_type_add_interface_static(ags_type_set_samplerate,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_set_samplerate);
}

void
ags_set_samplerate_class_init(AgsSetSamplerateClass *set_samplerate)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_set_samplerate_parent_class = g_type_class_peek_parent(set_samplerate);

  /* gobject */
  gobject = (GObjectClass *) set_samplerate;

  gobject->finalize = ags_set_samplerate_finalize;

  /* task */
  task = (AgsTaskClass *) set_samplerate;

  task->launch = ags_set_samplerate_launch;
}

void
ags_set_samplerate_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_set_samplerate_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_set_samplerate_connect;
  connectable->disconnect = ags_set_samplerate_disconnect;
}

void
ags_set_samplerate_init(AgsSetSamplerate *set_samplerate)
{
  set_samplerate->gobject = NULL;
  set_samplerate->samplerate = 44100;
}

void
ags_set_samplerate_connect(AgsConnectable *connectable)
{
  ags_set_samplerate_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_set_samplerate_disconnect(AgsConnectable *connectable)
{
  ags_set_samplerate_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_set_samplerate_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_set_samplerate_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_set_samplerate_launch(AgsTask *task)
{
  AgsSetSamplerate *set_samplerate;
  GObject *gobject;

  set_samplerate = AGS_SET_SAMPLERATE(task);

  gobject = set_samplerate->gobject;

  if(AGS_IS_DEVOUT(gobject)){
    ags_set_samplerate_devout(set_samplerate, AGS_DEVOUT(gobject));
  }else if(AGS_IS_AUDIO(gobject)){
    ags_set_samplerate_audio(set_samplerate, AGS_AUDIO(gobject));
  }else if(AGS_IS_CHANNEL(gobject)){
    ags_set_samplerate_channel(set_samplerate, AGS_CHANNEL(gobject));
  }else if(AGS_IS_RECYCLING(gobject)){
    ags_set_samplerate_recycling(set_samplerate, AGS_RECYCLING(gobject));
  }else if(AGS_IS_AUDIO_SIGNAL(gobject)){
    ags_set_samplerate_audio_signal(set_samplerate, AGS_AUDIO_SIGNAL(gobject));
  }
}

void
ags_set_samplerate_audio_signal(AgsSetSamplerate *set_samplerate, AgsAudioSignal *audio_signal)
{
  ags_audio_signal_morph_samplerate(audio_signal,
				    set_samplerate->samplerate, 0.0);
}

void
ags_set_samplerate_recycling(AgsSetSamplerate *set_samplerate, AgsRecycling *recycling)
{
  GList *list;
  
  list = recycling->audio_signal;

  while(list != NULL){
    ags_set_samplerate_audio_signal(set_samplerate, AGS_AUDIO_SIGNAL(list->data));

    list = list->next;
  }
}

void
ags_set_samplerate_channel(AgsSetSamplerate *set_samplerate, AgsChannel *channel)
{
  AgsRecycling *recycling;

  recycling = channel->first_recycling;
  
  while(recycling != NULL){
    ags_set_samplerate_recycling(set_samplerate, recycling);

    recycling = recycling->next;
  }
}

void
ags_set_samplerate_audio(AgsSetSamplerate *set_samplerate, AgsAudio *audio)
{
  AgsChannel *channel;

  /* AgsOutput */
  channel = audio->output;

  while(channel != NULL){
    ags_set_samplerate_channel(set_samplerate, channel);

    channel = channel->next;
  }

  /* AgsInput */
  channel = audio->input;

  while(channel != NULL){
    ags_set_samplerate_channel(set_samplerate, channel);

    channel = channel->next;
  }
}

void
ags_set_samplerate_devout(AgsSetSamplerate *set_samplerate, AgsDevout *devout)
{
  GList *list;

  g_object_set(G_OBJECT(devout),
	       "frequency\0", (guint) set_samplerate->samplerate,
	       NULL);

  /* AgsAudio */
  list = devout->audio;

  while(list != NULL){
    ags_set_samplerate_audio(set_samplerate, AGS_AUDIO(list->data));

    list = list->next;
  }
}

/**
 * ags_set_samplerate_new:
 * @devout: the #AgsDevout to reset
 * @samplerate: the new samplerate
 *
 * Creates an #AgsSetSamplerate.
 *
 * Returns: an new #AgsSetSamplerate.
 *
 * Since: 0.4
 */
AgsSetSamplerate*
ags_set_samplerate_new(GObject *gobject,
		       guint samplerate)
{
  AgsSetSamplerate *set_samplerate;

  set_samplerate = (AgsSetSamplerate *) g_object_new(AGS_TYPE_SET_SAMPLERATE,
						     NULL);

  set_samplerate->gobject = gobject;
  set_samplerate->samplerate = samplerate;

  return(set_samplerate);
}
