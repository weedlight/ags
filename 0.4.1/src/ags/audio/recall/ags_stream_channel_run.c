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

#include <ags/audio/recall/ags_stream_channel_run.h>
#include <ags/audio/recall/ags_stream_recycling.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/main.h>

#include <ags/object/ags_dynamic_connectable.h>
#include <ags/object/ags_plugin.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_recall_id.h>

#include <ags/audio/task/ags_cancel_recall.h>

void ags_stream_channel_run_class_init(AgsStreamChannelRunClass *stream_channel_run);
void ags_stream_channel_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_stream_channel_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_stream_channel_run_plugin_interface_init(AgsPluginInterface *plugin);
void ags_stream_channel_run_init(AgsStreamChannelRun *stream_channel_run);
void ags_stream_channel_run_connect(AgsConnectable *connectable);
void ags_stream_channel_run_disconnect(AgsConnectable *connectable);
void ags_stream_channel_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_stream_channel_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_stream_channel_run_finalize(GObject *gobject);

AgsRecall* ags_stream_channel_run_duplicate(AgsRecall *recall,
					    AgsRecallID *recall_id,
					    guint *n_params, GParameter *parameter);

static gpointer ags_stream_channel_run_parent_class = NULL;
static AgsConnectableInterface *ags_stream_channel_run_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_stream_channel_run_parent_dynamic_connectable_interface;
static AgsPluginInterface *ags_stream_channel_run_parent_plugin_interface;

GType
ags_stream_channel_run_get_type()
{
  static GType ags_type_stream_channel_run = 0;

  if(!ags_type_stream_channel_run){
    static const GTypeInfo ags_stream_channel_run_info = {
      sizeof (AgsStreamChannelRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_stream_channel_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsStreamChannelRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_stream_channel_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_stream_channel_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_stream_channel_run_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_stream_channel_run_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };    

    ags_type_stream_channel_run = g_type_register_static(AGS_TYPE_RECALL_CHANNEL_RUN,
							 "AgsStreamChannelRun\0",
							 &ags_stream_channel_run_info,
							 0);

    g_type_add_interface_static(ags_type_stream_channel_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_stream_channel_run,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);

    g_type_add_interface_static(ags_type_stream_channel_run,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return (ags_type_stream_channel_run);
}

void
ags_stream_channel_run_class_init(AgsStreamChannelRunClass *stream_channel_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;

  ags_stream_channel_run_parent_class = g_type_class_peek_parent(stream_channel_run);

  /* GObjectClass */
  gobject = (GObjectClass *) stream_channel_run;

  gobject->finalize = ags_stream_channel_run_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) stream_channel_run;

  recall->duplicate = ags_stream_channel_run_duplicate;
}

void
ags_stream_channel_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_stream_channel_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_stream_channel_run_connect;
  connectable->disconnect = ags_stream_channel_run_disconnect;
}

void
ags_stream_channel_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_stream_channel_run_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_stream_channel_run_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_stream_channel_run_disconnect_dynamic;
}

void
ags_stream_channel_run_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_stream_channel_run_parent_plugin_interface = g_type_interface_peek_parent(plugin);
}

void
ags_stream_channel_run_init(AgsStreamChannelRun *stream_channel_run)
{
  AGS_RECALL(stream_channel_run)->name = "ags-stream\0";
  AGS_RECALL(stream_channel_run)->version = AGS_EFFECTS_DEFAULT_VERSION;
  AGS_RECALL(stream_channel_run)->build_id = AGS_BUILD_ID;
  AGS_RECALL(stream_channel_run)->xml_type = "ags-stream-channel-run\0";
  AGS_RECALL(stream_channel_run)->port = NULL;

  AGS_RECALL(stream_channel_run)->flags |= AGS_RECALL_INPUT_ORIENTATED;
  AGS_RECALL(stream_channel_run)->child_type = AGS_TYPE_STREAM_RECYCLING;
}

void
ags_stream_channel_run_finalize(GObject *gobject)
{
  /* empty */

  /* call parent */
  G_OBJECT_CLASS(ags_stream_channel_run_parent_class)->finalize(gobject);
}

void
ags_stream_channel_run_connect(AgsConnectable *connectable)
{
  /* call parent */
  ags_stream_channel_run_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_stream_channel_run_disconnect(AgsConnectable *connectable)
{
  /* call parent */
  ags_stream_channel_run_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_stream_channel_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_stream_channel_run_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_stream_channel_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  AgsChannel *channel;
  AgsStreamChannelRun *stream_channel_run;

  ags_stream_channel_run_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  /* empty */
}

AgsRecall*
ags_stream_channel_run_duplicate(AgsRecall *recall,
				 AgsRecallID *recall_id,
				 guint *n_params, GParameter *parameter)
{
  AgsStreamChannelRun *stream_channel_run, *copy;

  stream_channel_run = (AgsStreamChannelRun *) recall;  
  copy = (AgsStreamChannelRun *) AGS_RECALL_CLASS(ags_stream_channel_run_parent_class)->duplicate(recall,
												  recall_id,
												  n_params, parameter);

  return((AgsRecall *) copy);
}

AgsStreamChannelRun*
ags_stream_channel_run_new()
{
  AgsStreamChannelRun *stream_channel_run;

  stream_channel_run = (AgsStreamChannelRun *) g_object_new(AGS_TYPE_STREAM_CHANNEL_RUN,
							    NULL);
  
  return(stream_channel_run);
}
