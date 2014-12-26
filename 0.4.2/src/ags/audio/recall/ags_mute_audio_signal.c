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

#include <ags/audio/recall/ags_mute_audio_signal.h>
#include <ags/audio/recall/ags_mute_audio.h>
#include <ags/audio/recall/ags_mute_channel.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/main.h>

#include <ags/object/ags_dynamic_connectable.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_container.h>
#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_recall_channel.h>
#include <ags/audio/ags_recall_channel_run.h>

#include <stdlib.h>

void ags_mute_audio_signal_class_init(AgsMuteAudioSignalClass *mute_audio_signal);
void ags_mute_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_mute_audio_signal_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_mute_audio_signal_init(AgsMuteAudioSignal *mute_audio_signal);
void ags_mute_audio_signal_connect(AgsConnectable *connectable);
void ags_mute_audio_signal_disconnect(AgsConnectable *connectable);
void ags_mute_audio_signal_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_mute_audio_signal_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_mute_audio_signal_finalize(GObject *gobject);

void ags_mute_audio_signal_run_inter(AgsRecall *recall);
AgsRecall* ags_mute_audio_signal_duplicate(AgsRecall *recall,
					   AgsRecallID *recall_id,
					   guint *n_params, GParameter *parameter);

/**
 * SECTION:ags_mute_audio_signal
 * @short_description: mutes audio signal
 * @title: AgsMuteAudioSignal
 * @section_id:
 * @include: ags/audio/recall/ags_mute_audio_signal.h
 *
 * The #AgsMuteAudioSignal class mutes the audio signal.
 */

static gpointer ags_mute_audio_signal_parent_class = NULL;
static AgsConnectableInterface *ags_mute_audio_signal_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_mute_audio_signal_parent_dynamic_connectable_interface;

GType
ags_mute_audio_signal_get_type()
{
  static GType ags_type_mute_audio_signal = 0;

  if(!ags_type_mute_audio_signal){
    static const GTypeInfo ags_mute_audio_signal_info = {
      sizeof (AgsMuteAudioSignalClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_mute_audio_signal_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsMuteAudioSignal),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_mute_audio_signal_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_mute_audio_signal_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_mute_audio_signal_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_mute_audio_signal = g_type_register_static(AGS_TYPE_RECALL_AUDIO_SIGNAL,
							"AgsMuteAudioSignal\0",
							&ags_mute_audio_signal_info,
							0);

    g_type_add_interface_static(ags_type_mute_audio_signal,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_mute_audio_signal,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);
  }

  return(ags_type_mute_audio_signal);
}

void
ags_mute_audio_signal_class_init(AgsMuteAudioSignalClass *mute_audio_signal)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;

  ags_mute_audio_signal_parent_class = g_type_class_peek_parent(mute_audio_signal);

  /* GObjectClass */
  gobject = (GObjectClass *) mute_audio_signal;

  gobject->finalize = ags_mute_audio_signal_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) mute_audio_signal;

  recall->run_inter = ags_mute_audio_signal_run_inter;  
  recall->duplicate = ags_mute_audio_signal_duplicate;
}

void
ags_mute_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_mute_audio_signal_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_mute_audio_signal_connect;
  connectable->disconnect = ags_mute_audio_signal_disconnect;
}

void
ags_mute_audio_signal_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_mute_audio_signal_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_mute_audio_signal_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_mute_audio_signal_disconnect_dynamic;
}

void
ags_mute_audio_signal_init(AgsMuteAudioSignal *mute_audio_signal)
{
  AGS_RECALL(mute_audio_signal)->name = "ags-mute\0";
  AGS_RECALL(mute_audio_signal)->version = AGS_EFFECTS_DEFAULT_VERSION;
  AGS_RECALL(mute_audio_signal)->build_id = AGS_BUILD_ID;
  AGS_RECALL(mute_audio_signal)->xml_type = "ags-mute-audio-signal\0";
  AGS_RECALL(mute_audio_signal)->port = NULL;

  AGS_RECALL(mute_audio_signal)->child_type = G_TYPE_NONE;
}

void
ags_mute_audio_signal_connect(AgsConnectable *connectable)
{
  /* call parent */
  ags_mute_audio_signal_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_mute_audio_signal_disconnect(AgsConnectable *connectable)
{
  /* call parent */
  ags_mute_audio_signal_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_mute_audio_signal_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_mute_audio_signal_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_mute_audio_signal_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_mute_audio_signal_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_mute_audio_signal_finalize(GObject *gobject)
{
  /* empty */

  /* call parent */
  G_OBJECT_CLASS(ags_mute_audio_signal_parent_class)->finalize(gobject);
}

void
ags_mute_audio_signal_run_inter(AgsRecall *recall)
{
  AgsDevout *devout;
  AgsMuteAudio *mute_audio;
  AgsMuteChannel *mute_channel;
  AgsMuteAudioSignal *mute_audio_signal;
  AgsAudioSignal *source;
  GList *stream_source;
  gboolean audio_muted, channel_muted;
  guint i;
  GValue audio_value = {0,};
  GValue channel_value = {0,};

  AGS_RECALL_CLASS(ags_mute_audio_signal_parent_class)->run_inter(recall);

  mute_audio_signal = AGS_MUTE_AUDIO_SIGNAL(recall);

  devout = AGS_DEVOUT(AGS_RECALL(mute_audio_signal)->devout);
  source = AGS_RECALL_AUDIO_SIGNAL(mute_audio_signal)->source;
  stream_source = source->stream_current;

  if(stream_source == NULL){
    ags_recall_done(recall);
    return;
  }

  /* check channel */
  mute_channel = AGS_MUTE_CHANNEL(AGS_RECALL_CHANNEL_RUN(recall->parent->parent)->recall_channel);

  g_value_init(&channel_value, G_TYPE_BOOLEAN);
  ags_port_safe_read(mute_channel->muted,
		     &channel_value);

  channel_muted = g_value_get_boolean(&channel_value);
  g_value_unset(&channel_value);

  /* check audio */
  mute_audio = AGS_MUTE_AUDIO(AGS_RECALL_CONTAINER(AGS_RECALL(mute_channel)->container)->recall_audio);

  g_value_init(&audio_value, G_TYPE_BOOLEAN);
  ags_port_safe_read(mute_audio->muted,
		     &audio_value);

  audio_muted = g_value_get_boolean(&audio_value);
  g_value_unset(&audio_value);

  /* if not muted return */
  if(!channel_muted && !audio_muted){
    return;
  }

  /* mute */
  memset((signed short *) stream_source->data, 0, devout->buffer_size * sizeof(signed short));
}

AgsRecall*
ags_mute_audio_signal_duplicate(AgsRecall *recall,
				AgsRecallID *recall_id,
				guint *n_params, GParameter *parameter)
{
  AgsMuteAudioSignal *mute;

  mute = (AgsMuteAudioSignal *) AGS_RECALL_CLASS(ags_mute_audio_signal_parent_class)->duplicate(recall,
												recall_id,
												n_params, parameter);

  return((AgsRecall *) mute);
}

/**
 * ags_mute_audio_signal_new:
 * @source: the source #AgsAudioSignal
 *
 * Creates an #AgsMuteAudioSignal
 *
 * Returns: a new #AgsMuteAudioSignal
 *
 * Since: 0.4
 */
AgsMuteAudioSignal*
ags_mute_audio_signal_new(AgsAudioSignal *source)
{
  AgsMuteAudioSignal *mute_audio_signal;

  mute_audio_signal = (AgsMuteAudioSignal *) g_object_new(AGS_TYPE_MUTE_AUDIO_SIGNAL,
							  "source\0", source,
							  NULL);

  return(mute_audio_signal);
}
