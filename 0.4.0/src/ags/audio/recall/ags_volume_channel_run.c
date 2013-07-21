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

#include <ags/audio/recall/ags_volume_channel_run.h>

#include <ags/lib/ags_list.h>
#include <ags/lib/ags_parameter.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_run_connectable.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_recall_id.h>

#include <ags/audio/task/ags_cancel_recall.h>

#include <ags/audio/recall/ags_volume_recycling.h>

void ags_volume_channel_run_class_init(AgsVolumeChannelRunClass *volume_channel_run);
void ags_volume_channel_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_volume_channel_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable);
void ags_volume_channel_run_init(AgsVolumeChannelRun *volume_channel_run);
void ags_volume_channel_run_set_property(GObject *gobject,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *param_spec);
void ags_volume_channel_run_get_property(GObject *gobject,
					 guint prop_id,
					 GValue *value,
					 GParamSpec *param_spec);
void ags_volume_channel_run_connect(AgsConnectable *connectable);
void ags_volume_channel_run_disconnect(AgsConnectable *connectable);
void ags_volume_channel_run_run_connect(AgsRunConnectable *run_connectable);
void ags_volume_channel_run_run_disconnect(AgsRunConnectable *run_connectable);
void ags_volume_channel_run_finalize(GObject *gobject);

AgsRecall* ags_volume_channel_run_duplicate(AgsRecall *recall,
					    AgsRecallID *recall_id,
					    guint *n_params, GParameter *parameter);

enum{
  PROP_0,
  PROP_VOLUME,
};

static gpointer ags_volume_channel_run_parent_class = NULL;
static AgsConnectableInterface *ags_volume_channel_run_parent_connectable_interface;
static AgsRunConnectableInterface *ags_volume_channel_run_parent_run_connectable_interface;

GType
ags_volume_channel_run_get_type()
{
  static GType ags_type_volume_channel_run = 0;

  if(!ags_type_volume_channel_run){
    static const GTypeInfo ags_volume_channel_run_info = {
      sizeof (AgsVolumeChannelRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_volume_channel_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsVolumeChannelRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_volume_channel_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_volume_channel_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_run_connectable_interface_info = {
      (GInterfaceInitFunc) ags_volume_channel_run_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_volume_channel_run = g_type_register_static(AGS_TYPE_RECALL_CHANNEL_RUN,
							 "AgsVolumeChannelRun\0",
							 &ags_volume_channel_run_info,
							 0);

    g_type_add_interface_static(ags_type_volume_channel_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_volume_channel_run,
				AGS_TYPE_RUN_CONNECTABLE,
				&ags_run_connectable_interface_info);
  }

  return (ags_type_volume_channel_run);
}

void
ags_volume_channel_run_class_init(AgsVolumeChannelRunClass *volume_channel_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_volume_channel_run_parent_class = g_type_class_peek_parent(volume_channel_run);

  /* GObjectClass */
  gobject = (GObjectClass *) volume_channel_run;

  gobject->set_property = ags_volume_channel_run_set_property;
  gobject->get_property = ags_volume_channel_run_get_property;

  gobject->finalize = ags_volume_channel_run_finalize;

  /* properties */
  param_spec = g_param_spec_pointer("volume\0",
				    "volume to apply\0",
				    "The volume to apply on the channel\0",
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_VOLUME,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) volume_channel_run;

  recall->duplicate = ags_volume_channel_run_duplicate;
}

void
ags_volume_channel_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_volume_channel_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_volume_channel_run_connect;
  connectable->disconnect = ags_volume_channel_run_disconnect;
}

void
ags_volume_channel_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable)
{
  ags_volume_channel_run_parent_run_connectable_interface = g_type_interface_peek_parent(run_connectable);

  run_connectable->connect = ags_volume_channel_run_run_connect;
  run_connectable->disconnect = ags_volume_channel_run_run_disconnect;
}

void
ags_volume_channel_run_init(AgsVolumeChannelRun *volume_channel_run)
{
  AGS_RECALL(volume_channel_run)->flags |= AGS_RECALL_INPUT_ORIENTATED;
  AGS_RECALL(volume_channel_run)->child_type = AGS_TYPE_VOLUME_RECYCLING;

  volume_channel_run->volume = NULL;
}


void
ags_volume_channel_run_set_property(GObject *gobject,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *param_spec)
{
  AgsVolumeChannelRun *volume_channel_run;

  volume_channel_run = AGS_VOLUME_CHANNEL_RUN(gobject);

  switch(prop_id){
  case PROP_VOLUME:
    {
      volume_channel_run->volume = g_value_get_pointer(value);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_volume_channel_run_get_property(GObject *gobject,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *param_spec)
{
  AgsVolumeChannelRun *volume_channel_run;

  volume_channel_run = AGS_VOLUME_CHANNEL_RUN(gobject);

  switch(prop_id){
  case PROP_VOLUME:
    {
      g_value_set_pointer(value, volume_channel_run->volume);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_volume_channel_run_finalize(GObject *gobject)
{
  AgsVolumeChannelRun *volume_channel_run;

  volume_channel_run = AGS_VOLUME_CHANNEL_RUN(gobject);

  /* call parent */
  G_OBJECT_CLASS(ags_volume_channel_run_parent_class)->finalize(gobject);
}

void
ags_volume_channel_run_connect(AgsConnectable *connectable)
{
  AgsVolumeChannelRun *volume_channel_run;

  ags_volume_channel_run_parent_connectable_interface->connect(connectable);

  volume_channel_run = AGS_VOLUME_CHANNEL_RUN(connectable);
}

void
ags_volume_channel_run_disconnect(AgsConnectable *connectable)
{
  ags_volume_channel_run_parent_connectable_interface->disconnect(connectable);
}

void
ags_volume_channel_run_run_connect(AgsRunConnectable *run_connectable)
{
  ags_volume_channel_run_parent_run_connectable_interface->connect(run_connectable);
}

void
ags_volume_channel_run_run_disconnect(AgsRunConnectable *run_connectable)
{
  ags_volume_channel_run_parent_run_connectable_interface->disconnect(run_connectable);
}

AgsRecall*
ags_volume_channel_run_duplicate(AgsRecall *recall,
			     AgsRecallID *recall_id,
			     guint *n_params, GParameter *parameter)
{
  AgsVolumeChannelRun *volume_channel_run, *copy;

  volume_channel_run = (AgsVolumeChannelRun *) recall;
  
  parameter = ags_parameter_grow(G_OBJECT_TYPE(recall),
				 parameter, n_params,
				 "volume\0", volume_channel_run->volume,
				 NULL);
  copy = (AgsVolumeChannelRun *) AGS_RECALL_CLASS(ags_volume_channel_run_parent_class)->duplicate(recall,
												  recall_id,
												  n_params, parameter);
  
  return((AgsRecall *) copy);
}

AgsVolumeChannelRun*
ags_volume_channel_run_new(AgsChannel *channel, gdouble *volume)
{
  AgsVolumeChannelRun *volume_channel_run;

  volume_channel_run = (AgsVolumeChannelRun *) g_object_new(AGS_TYPE_VOLUME_CHANNEL_RUN,
							    "source\0", channel,
							    "volume\0", volume,
							    NULL);

  return(volume_channel_run);
}