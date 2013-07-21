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

#include <ags/audio/ags_recall_channel_run.h>

#include <ags/lib/ags_parameter.h>

#include <ags/object/ags_marshal.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_packable.h>
#include <ags/object/ags_run_connectable.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_recall_audio_run.h>
#include <ags/audio/ags_recall_channel.h>
#include <ags/audio/ags_recall_container.h>
#include <ags/audio/ags_recall_recycling.h>

#include <ags/audio/task/ags_cancel_recall.h>

void ags_recall_channel_run_class_init(AgsRecallChannelRunClass *recall_channel_run);
void ags_recall_channel_runconnectable_interface_init(AgsConnectableInterface *connectable);
void ags_recall_channel_run_packable_interface_init(AgsPackableInterface *packable);
void ags_recall_channel_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable);
void ags_recall_channel_run_init(AgsRecallChannelRun *recall_channel_run);
void ags_recall_channel_run_set_property(GObject *gobject,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *param_spec);
void ags_recall_channel_run_get_property(GObject *gobject,
					 guint prop_id,
					 GValue *value,
					 GParamSpec *param_spec);
void ags_recall_channel_run_connect(AgsConnectable *connectable);
gboolean ags_recall_channel_run_pack(AgsPackable *packable, GObject *container);
gboolean ags_recall_channel_run_unpack(AgsPackable *packable);
void ags_recall_channel_run_disconnect(AgsConnectable *connectable);
void ags_recall_channel_run_run_connect(AgsRunConnectable *run_connectable);
void ags_recall_channel_run_run_disconnect(AgsRunConnectable *run_connectable);
void ags_recall_channel_run_finalize(GObject *gobject);

AgsRecall* ags_recall_channel_run_duplicate(AgsRecall *recall,
					    AgsRecallID *recall_id,
					    guint *n_params, GParameter *parameter);

void ags_recall_channel_run_map_recall_recycling(AgsRecallChannelRun *recall_channel_run);

void ags_recall_channel_run_remap_child_source(AgsRecallChannelRun *recall_channel_run,
					       AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
					       AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region);
void ags_recall_channel_run_remap_child_destination(AgsRecallChannelRun *recall_channel_run,
						    AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
						    AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region);
void ags_recall_channel_run_refresh_child_source(AgsRecallChannelRun *recall_channel_run,
						 AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
						 AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region);
void ags_recall_channel_run_refresh_child_destination(AgsRecallChannelRun *recall_channel_run,
						      AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
						      AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region);
void ags_recall_channel_run_source_recycling_changed_callback(AgsChannel *channel,
							      AgsRecycling *old_start_region, AgsRecycling *old_end_region,
							      AgsRecycling *new_start_region, AgsRecycling *new_end_region,
							      AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
							      AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region,
							      AgsRecallChannelRun *recall_channel_run);

void ags_recall_channel_run_destination_recycling_changed_callback(AgsChannel *channel,
								   AgsRecycling *old_start_region, AgsRecycling *old_end_region,
								   AgsRecycling *new_start_region, AgsRecycling *new_end_region,
								   AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
								   AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region,
								   AgsRecallChannelRun *recall_channel_run);


AgsGroupId ags_recall_channel_run_real_get_audio_run_group_id(AgsRecallChannelRun *recall_channel_run);
void ags_recall_channel_run_real_run_order_changed(AgsRecallChannelRun *recall_channel_run,
						   guint run_order);

enum{
  GET_AUDIO_RUN_GROUP_ID,
  RUN_ORDER_CHANGED,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_AUDIO_CHANNEL,
  PROP_DESTINATION,
  PROP_SOURCE,
  PROP_RECALL_AUDIO_RUN,
  PROP_RECALL_CHANNEL,
};

static gpointer ags_recall_channel_run_parent_class = NULL;
static AgsConnectableInterface* ags_recall_channel_run_parent_connectable_interface;
static AgsPackableInterface* ags_recall_channel_run_parent_packable_interface;
static AgsRunConnectableInterface *ags_recall_channel_run_parent_run_connectable_interface;

static guint recall_channel_run_signals[LAST_SIGNAL];

GType
ags_recall_channel_run_get_type()
{
  static GType ags_type_recall_channel_run = 0;

  if(!ags_type_recall_channel_run){
    static const GTypeInfo ags_recall_channel_run_info = {
      sizeof (AgsRecallChannelRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recall_channel_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecallChannelRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recall_channel_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recall_channel_runconnectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_packable_interface_info = {
      (GInterfaceInitFunc) ags_recall_channel_run_packable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_run_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recall_channel_run_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_recall_channel_run = g_type_register_static(AGS_TYPE_RECALL,
							 "AgsRecallChannelRun\0",
							 &ags_recall_channel_run_info,
							 0);
    
    g_type_add_interface_static(ags_type_recall_channel_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_recall_channel_run,
				AGS_TYPE_PACKABLE,
				&ags_packable_interface_info);
    
    g_type_add_interface_static(ags_type_recall_channel_run,
				AGS_TYPE_RUN_CONNECTABLE,
				&ags_run_connectable_interface_info);
  }

  return(ags_type_recall_channel_run);
}

void
ags_recall_channel_run_class_init(AgsRecallChannelRunClass *recall_channel_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_recall_channel_run_parent_class = g_type_class_peek_parent(recall_channel_run);

  /* GObjectClass */
  gobject = (GObjectClass *) recall_channel_run;

  gobject->set_property = ags_recall_channel_run_set_property;
  gobject->get_property = ags_recall_channel_run_get_property;

  gobject->finalize = ags_recall_channel_run_finalize;

  /* properties */
  param_spec = g_param_spec_uint("audio_channel\0",
				 "assigned audio channel\0",
				 "The audio channel this recall is assigned to\0",
				 0, 65535,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO_CHANNEL,
				  param_spec);


  param_spec = g_param_spec_object("recall_audio_run\0",
				   "AgsRecallAudioRun of this recall\0",
				   "The AgsRecallAudioRun which this recall needs\0",
				   AGS_TYPE_RECALL_AUDIO_RUN,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECALL_AUDIO_RUN,
				  param_spec);

  param_spec = g_param_spec_object("recall_channel\0",
				   "AsgRecallChannel of this recall\0",
				   "The AgsRecallChannel which this recall needs\0",
				   AGS_TYPE_RECALL_CHANNEL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECALL_CHANNEL,
				  param_spec);

  param_spec = g_param_spec_object("destination\0",
				   "destination of output\0",
				   "The destination AgsChannel where it will output to\0",
				   AGS_TYPE_CHANNEL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DESTINATION,
				  param_spec);

  param_spec = g_param_spec_object("source\0",
				   "source of input\0",
				   "The source AgsChannel where it will take the input from\0",
				   AGS_TYPE_CHANNEL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOURCE,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) recall_channel_run;

  recall->duplicate = ags_recall_channel_run_duplicate;

  /* AgsRecallChannelRunClass */
  recall_channel_run->get_audio_run_group_id = ags_recall_channel_run_real_get_audio_run_group_id;
  recall_channel_run->run_order_changed = ags_recall_channel_run_real_run_order_changed;

  /* signals */
  recall_channel_run_signals[RUN_ORDER_CHANGED] =
    g_signal_new("run_order_changed\0",
		 G_TYPE_FROM_CLASS (recall_channel_run),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsRecallChannelRunClass, run_order_changed),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__UINT,
		 G_TYPE_NONE, 1,
		 G_TYPE_UINT);

  recall_channel_run_signals[GET_AUDIO_RUN_GROUP_ID] =
    g_signal_new("get_audio_run_group_id\0",
		 G_TYPE_FROM_CLASS (recall_channel_run),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsRecallChannelRunClass, get_audio_run_group_id),
		 NULL, NULL,
		 g_cclosure_user_marshal_ULONG__VOID,
		 G_TYPE_ULONG, 0);
}

void
ags_recall_channel_runconnectable_interface_init(AgsConnectableInterface *connectable)
{
  AgsConnectableInterface *ags_recall_channel_run_connectable_parent_interface;

  ags_recall_channel_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_recall_channel_run_connect;
  connectable->disconnect = ags_recall_channel_run_disconnect;
}

void
ags_recall_channel_run_packable_interface_init(AgsPackableInterface *packable)
{
  ags_recall_channel_run_parent_packable_interface = g_type_interface_peek_parent(packable);

  packable->pack = ags_recall_channel_run_pack;
  packable->unpack = ags_recall_channel_run_unpack;
}

void
ags_recall_channel_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable)
{
  ags_recall_channel_run_parent_run_connectable_interface = g_type_interface_peek_parent(run_connectable);

  run_connectable->connect = ags_recall_channel_run_run_connect;
  run_connectable->disconnect = ags_recall_channel_run_run_disconnect;
}

void
ags_recall_channel_run_init(AgsRecallChannelRun *recall_channel_run)
{
  recall_channel_run->audio_channel = 0;

  recall_channel_run->recall_audio_run = NULL;
  recall_channel_run->recall_channel = NULL;

  recall_channel_run->source = NULL;
  recall_channel_run->destination = NULL;

  recall_channel_run->run_order = 0;
}


void
ags_recall_channel_run_set_property(GObject *gobject,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *param_spec)
{
  AgsRecallChannelRun *recall_channel_run;

  recall_channel_run = AGS_RECALL_CHANNEL_RUN(gobject);

  switch(prop_id){
  case PROP_AUDIO_CHANNEL:
    {
      recall_channel_run->audio_channel = g_value_get_uint(value);
    }
    break;
  case PROP_RECALL_AUDIO_RUN:
    {
      AgsRecallAudioRun *recall_audio_run;

      recall_audio_run = (AgsRecallAudioRun *) g_value_get_object(value);

      if(recall_channel_run->recall_audio_run == recall_audio_run)
	return;

      if(recall_channel_run->recall_audio_run != NULL){
	g_object_unref(G_OBJECT(recall_channel_run->recall_audio_run));
      }

      if(recall_audio_run != NULL){
	g_object_ref(G_OBJECT(recall_audio_run));
      }

      recall_channel_run->recall_audio_run = recall_audio_run;
    }
    break;
  case PROP_RECALL_CHANNEL:
    {
      AgsRecallChannel *recall_channel;

      recall_channel = (AgsRecallChannel *) g_value_get_object(value);

      if(recall_channel_run->recall_channel == recall_channel)
	return;

      if(recall_channel_run->recall_channel != NULL){
	g_object_unref(G_OBJECT(recall_channel_run->recall_channel));
      }

      if(recall_channel != NULL){
	g_object_ref(G_OBJECT(recall_channel));
      }

      recall_channel_run->recall_channel = recall_channel;
    }
    break;
  case PROP_DESTINATION:
    {
      AgsChannel *destination;
      AgsChannel *old_destination;
      AgsRecycling *new_start_region, *new_end_region;
      AgsRecycling *old_start_region, *old_end_region;

      destination = (AgsChannel *) g_value_get_object(value);

      if(recall_channel_run->destination == destination)
	return;

      old_destination = recall_channel_run->destination;

      if(old_destination != NULL){
	old_start_region = old_destination->first_recycling;
	old_end_region = old_destination->last_recycling;
      }else{
	old_start_region = NULL;
	old_end_region = NULL;
      }

      if(destination != NULL){
	g_object_ref(G_OBJECT(destination));

	new_start_region = destination->first_recycling;
	new_end_region = destination->last_recycling;
      }else{
	new_start_region = NULL;
	new_end_region = NULL;
      }

      recall_channel_run->destination = destination;

      if(destination == recall_channel_run->source)
	g_warning("destination == recall_channel_run->source\0");

      ags_recall_channel_run_remap_child_destination(recall_channel_run,
						     old_start_region, old_end_region,
						     new_start_region, new_end_region);

      if(old_destination != NULL)
	g_object_unref(G_OBJECT(old_destination));
    }
    break;
  case PROP_SOURCE:
    {
      AgsChannel *source;
      AgsChannel *old_source;
      AgsRecycling *new_start_region, *new_end_region;
      AgsRecycling *old_start_region, *old_end_region;

      source = (AgsChannel *) g_value_get_object(value);

      if(recall_channel_run->source == source)
	return;

      old_source = recall_channel_run->source;

      if(old_source != NULL){
	old_start_region = old_source->first_recycling;
	old_end_region = old_source->last_recycling;
      }else{
	old_start_region = NULL;
	old_end_region = NULL;
      }

      if(source != NULL){
	g_object_ref(G_OBJECT(source));

	new_start_region = source->first_recycling;
	new_end_region = source->last_recycling;
      }else{
	new_start_region = NULL;
	new_end_region = NULL;
      }

      recall_channel_run->source = source;

      if(source == recall_channel_run->destination)
	g_warning("destination == recall_channel_run->source\0");

      ags_recall_channel_run_remap_child_source(recall_channel_run,
						old_start_region, old_end_region,
						new_start_region, new_end_region);
      
      if(old_source != NULL)
	g_object_unref(G_OBJECT(old_source));
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_recall_channel_run_get_property(GObject *gobject,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *param_spec)
{
  AgsRecallChannelRun *recall_channel_run;

  recall_channel_run = AGS_RECALL_CHANNEL_RUN(gobject);

  switch(prop_id){
  case PROP_AUDIO_CHANNEL:
    {
      g_value_set_uint(value, recall_channel_run->audio_channel);
    }
    break;
  case PROP_RECALL_AUDIO_RUN:
    {
      g_value_set_object(value, recall_channel_run->recall_audio_run);
    }
    break;
  case PROP_RECALL_CHANNEL:
    {
      g_value_set_object(value, recall_channel_run->recall_channel);
    }
    break;
  case PROP_DESTINATION:
    {
      g_value_set_object(value, recall_channel_run->destination);
    }
    break;
  case PROP_SOURCE:
    {
      g_value_set_object(value, recall_channel_run->source);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_recall_channel_run_finalize(GObject *gobject)
{
  AgsRecallChannelRun *recall_channel_run;

  recall_channel_run = AGS_RECALL_CHANNEL_RUN(gobject);

  if(recall_channel_run->recall_audio_run != NULL)
    g_object_unref(G_OBJECT(recall_channel_run->recall_audio_run));
  
  if(recall_channel_run->recall_channel != NULL)
    g_object_unref(G_OBJECT(recall_channel_run->recall_channel));

  if(recall_channel_run->destination != NULL)
    g_object_unref(recall_channel_run->destination);

  if(recall_channel_run->source != NULL)
    g_object_unref(recall_channel_run->source);

  G_OBJECT_CLASS(ags_recall_channel_run_parent_class)->finalize(gobject);
}

void
ags_recall_channel_run_connect(AgsConnectable *connectable)
{
  AgsRecallChannelRun *recall_channel_run;
  GObject *gobject;

  ags_recall_channel_run_parent_connectable_interface->connect(connectable);

  g_message("ags_recall_channel_run_connect\0");

  /* AgsCopyChannel */
  recall_channel_run = AGS_RECALL_CHANNEL_RUN(connectable);

  /* destination */
  if(recall_channel_run->destination != NULL){
    gobject = G_OBJECT(recall_channel_run->destination);
    
    recall_channel_run->destination_recycling_changed_handler =
      g_signal_connect(gobject, "recycling_changed\0",
		       G_CALLBACK(ags_recall_channel_run_destination_recycling_changed_callback), recall_channel_run);
  }

  /* source */
  gobject = G_OBJECT(recall_channel_run->source);

  recall_channel_run->source_recycling_changed_handler =
    g_signal_connect(gobject, "recycling_changed\0",
		     G_CALLBACK(ags_recall_channel_run_source_recycling_changed_callback), recall_channel_run);

}

void
ags_recall_channel_run_disconnect(AgsConnectable *connectable)
{
  AgsRecallChannelRun *recall_channel_run;
  GObject *gobject;

  ags_recall_channel_run_parent_connectable_interface->disconnect(connectable);

  /* AgsRecallChannelRun */
  recall_channel_run = AGS_RECALL_CHANNEL_RUN(connectable);

  /* destination */
  if(recall_channel_run->destination != NULL){
    gobject = G_OBJECT(recall_channel_run->destination);
  
    g_signal_handler_disconnect(gobject, recall_channel_run->destination_recycling_changed_handler);
  }

  /* source */
  gobject = G_OBJECT(recall_channel_run->source);

  g_signal_handler_disconnect(gobject, recall_channel_run->source_recycling_changed_handler);
}

gboolean
ags_recall_channel_run_pack(AgsPackable *packable, GObject *container)
{
  AgsRecallContainer *recall_container;
  GList *list;
  AgsGroupId group_id;

  if(ags_recall_channel_run_parent_packable_interface->pack(packable, container))
    return(TRUE);

  recall_container = AGS_RECALL_CONTAINER(container);

  recall_container->recall_channel_run = g_list_prepend(recall_container->recall_channel_run,
							AGS_RECALL(packable));

  /* set AgsRecallAudioRun */
  list = recall_container->recall_audio_run;

  if(AGS_RECALL(packable)->recall_id != NULL){
    group_id = ags_recall_channel_run_get_audio_run_group_id(AGS_RECALL_CHANNEL_RUN(packable));
      
    list = ags_recall_find_group_id(list,
				    group_id);

    if(list != NULL){
      g_object_set(G_OBJECT(packable),
		   "recall_audio_run\0", AGS_RECALL_AUDIO_RUN(list->data),
		   NULL);
    }
  }else if((AGS_RECALL_TEMPLATE & (AGS_RECALL(packable)->flags)) != 0){
    list = ags_recall_find_template(list);

    if(list != NULL){
      g_object_set(G_OBJECT(packable),
		   "recall_audio_run\0", AGS_RECALL_AUDIO_RUN(list->data),
		   NULL);
    }
  }

  /* set AgsRecallChannel */
  if(AGS_RECALL_CHANNEL_RUN(packable)->recall_channel->source != NULL){
    list = recall_container->recall_channel;

    if((list = ags_recall_find_provider(list,
					G_OBJECT(AGS_RECALL_CHANNEL_RUN(packable)->recall_channel->source))) != NULL){
      g_object_set(G_OBJECT(packable),
		   "recall_channel\0", AGS_RECALL_CHANNEL(list->data),
		   NULL);
    }
  }

  return(FALSE);
}

gboolean
ags_recall_channel_run_unpack(AgsPackable *packable)
{
  AgsRecall *recall;
  AgsRecallContainer *recall_container;
  GList *list;
  AgsGroupId group_id;

  recall = AGS_RECALL(packable);

  if(recall == NULL)
    return(TRUE);

  recall_container = AGS_RECALL_CONTAINER(recall->container);

  if(recall_container == NULL)
    return(TRUE);

  /* ref */
  g_object_ref(recall);
  g_object_ref(recall_container);

  /* unset AgsRecallAudioRun */
  g_object_set(G_OBJECT(packable),
	       "recall_audio_run\0", NULL,
	       NULL);

  /* unset AgsRecallChannel */
  g_object_set(G_OBJECT(packable),
	       "recall_channel\0", NULL,
	       NULL);

  /* call parent */
  if(ags_recall_channel_run_parent_packable_interface->unpack(packable)){
    g_object_unref(recall);
    g_object_unref(recall_container);

    return(TRUE);
  }

  /* remove from list */
  recall_container->recall_channel_run = g_list_remove(recall_container->recall_channel_run,
						       recall);

  /* unref */
  g_object_unref(recall);
  g_object_unref(recall_container);

  return(FALSE);
}

void
ags_recall_channel_run_run_connect(AgsConnectable *connectable)
{
  ags_recall_channel_run_parent_run_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_recall_channel_run_run_disconnect(AgsConnectable *connectable)
{
  ags_recall_channel_run_parent_run_connectable_interface->disconnect(connectable);

  /* empty */
}

AgsRecall*
ags_recall_channel_run_duplicate(AgsRecall *recall,
				 AgsRecallID *recall_id,
				 guint *n_params, GParameter *parameter)
{
  AgsRecallChannelRun *recall_channel_run, *copy;

  recall_channel_run = AGS_RECALL_CHANNEL_RUN(recall);

  parameter = ags_parameter_grow(G_OBJECT_TYPE(recall),
				 parameter, n_params,
				 "devout\0", AGS_RECALL(recall_channel_run)->devout,
				 "recall_channel\0", recall_channel_run->recall_channel,
				 "audio_channel\0", recall_channel_run->audio_channel,
				 //				 "audio_channel\0", recall_channel_run->audio_channel,
				 "destination\0", recall_channel_run->destination,
				 "source\0", recall_channel_run->source,
				 NULL);
  copy = AGS_RECALL_CHANNEL_RUN(AGS_RECALL_CLASS(ags_recall_channel_run_parent_class)->duplicate(recall,
												 recall_id,
												 n_params, parameter));

  //  ags_recall_channel_run_map_recall_recycling(copy);


  return((AgsRecall *) copy);
}

void
ags_recall_channel_run_map_recall_recycling(AgsRecallChannelRun *recall_channel_run)
{
  AgsRecallChannel *recall_channel;
  AgsRecycling *source_recycling, *destination_recycling;

  if(recall_channel_run->source == NULL ||
     AGS_RECALL(recall_channel_run)->child_type == G_TYPE_NONE ||
     (AGS_RECALL_TEMPLATE & (AGS_RECALL(recall_channel_run)->flags)) != 0)
    return;

  /* AgsRecallChannel */
  recall_channel = AGS_RECALL_CHANNEL(recall_channel_run->recall_channel);

  /* AgsRecycling - source */
  source_recycling = recall_channel->source->first_recycling;

  /* AgsRecycling - destination*/
  if(recall_channel_run->destination != NULL){
    destination_recycling = recall_channel_run->destination->first_recycling;
  }else{
    destination_recycling = NULL;
  }

  if(source_recycling != NULL){
    AgsRecallRecycling *recall_recycling;

    while(source_recycling != recall_channel->source->last_recycling->next){
      g_message("ags_recall_channel_run_map_recall_recycling\n\0");

      do{
	if(AGS_RECALL(recall_channel_run)->child_type != G_TYPE_NONE){
	  recall_recycling = g_object_new(AGS_RECALL(recall_channel_run)->child_type,
					  "devout\0", AGS_RECALL(recall_channel_run)->devout,
					  "recall_id\0", AGS_RECALL(recall_channel_run)->recall_id,
					  "audio_channel\0", recall_channel_run->audio_channel,
					  "destination\0", destination_recycling,
					  "source\0", source_recycling,
					  NULL);

	  ags_recall_add_child(AGS_RECALL(recall_channel_run), AGS_RECALL(recall_recycling));
	}

	if(destination_recycling != NULL){
	  destination_recycling = destination_recycling->next;
	}
      }while(destination_recycling != NULL);

      source_recycling = source_recycling->next;
    }
  }
}

void
ags_recall_channel_run_remap_child_source(AgsRecallChannelRun *recall_channel_run,
					  AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
					  AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region)
{
  AgsRecycling *destination_recycling, *source_recycling;
  AgsRecallRecycling *recall_recycling;
  GList *list;

  if(recall_channel_run->source == NULL ||
     AGS_RECALL(recall_channel_run)->child_type == G_TYPE_NONE ||
     (AGS_RECALL_TEMPLATE & (AGS_RECALL(recall_channel_run)->flags)) != 0)
    return;

  /* remove old */
  if(old_start_changed_region != NULL){
    AgsDevout *devout;
    AgsRecall *recall;
    AgsCancelRecall *cancel_recall;

    devout = AGS_DEVOUT(AGS_AUDIO(recall_channel_run->source->audio)->devout);
    source_recycling = old_start_changed_region;

    while(source_recycling != old_end_changed_region->next){
      list = ags_recall_get_children(AGS_RECALL(recall_channel_run));

      while(list != NULL){
	if(AGS_RECALL_RECYCLING(list->data)->source == source_recycling){
	  recall = AGS_RECALL(list->data);

	  recall->flags |= AGS_RECALL_HIDE;
	  cancel_recall = ags_cancel_recall_new(recall,
						NULL);

	  ags_task_thread_append_task(devout->task_thread,
				      (AgsTask *) cancel_recall);
	}

	list = list->next;
      }

      source_recycling = source_recycling->next;
    }
  }

  /* add new */
  if(new_start_changed_region != NULL){

    if((AGS_RECALL_OUTPUT_ORIENTATED & (AGS_RECALL(recall_channel_run)->flags)) != 0 &&
       (AGS_RECALL_INPUT_ORIENTATED & (AGS_RECALL(recall_channel_run)->flags)) != 0){
      if(recall_channel_run->destination == NULL ||
	 recall_channel_run->destination->first_recycling == NULL)
	return;
      
      destination_recycling = recall_channel_run->destination->first_recycling;
      
      while(destination_recycling != recall_channel_run->destination->last_recycling->next){
	source_recycling = new_start_changed_region;
	
	while(source_recycling != new_end_changed_region->next){
	  if(AGS_RECALL(recall_channel_run)->child_type != G_TYPE_NONE){
	    recall_recycling = g_object_new(AGS_RECALL(recall_channel_run)->child_type,
					    "devout\0", AGS_RECALL(recall_channel_run)->devout,
					    "recall_id\0", AGS_RECALL(recall_channel_run)->recall_id,
					    "audio_channel\0", recall_channel_run->audio_channel,
					    "destination\0", destination_recycling,
					    "source\0", source_recycling,
					    NULL);
	    
	    ags_recall_add_child(AGS_RECALL(recall_channel_run), AGS_RECALL(recall_recycling));
	  }

	  source_recycling = source_recycling->next;
	}
      
	destination_recycling = destination_recycling->next;
      }
    }else{
      source_recycling = new_start_changed_region;
      
      while(source_recycling != new_end_changed_region->next){

	if(AGS_RECALL(recall_channel_run)->child_type != G_TYPE_NONE){
	  recall_recycling = g_object_new(AGS_RECALL(recall_channel_run)->child_type,
					  "devout\0", AGS_RECALL(recall_channel_run)->devout,
					  "recall_id\0", AGS_RECALL(recall_channel_run)->recall_id,
					  "audio_channel\0", recall_channel_run->audio_channel,
					  "destination\0", NULL,
					  "source\0", source_recycling,
					  NULL);
	
	  ags_recall_add_child(AGS_RECALL(recall_channel_run), AGS_RECALL(recall_recycling));
	}

	source_recycling = source_recycling->next;
      }
    }
  }
}

void
ags_recall_channel_run_remap_child_destination(AgsRecallChannelRun *recall_channel_run,
					       AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
					       AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region)
{
  AgsRecycling *destination_recycling, *source_recycling;
  AgsRecallRecycling *recall_recycling;
  GList *list;

  if(recall_channel_run->source == NULL ||
     AGS_RECALL(recall_channel_run)->child_type == G_TYPE_NONE ||
     (AGS_RECALL_TEMPLATE & (AGS_RECALL(recall_channel_run)->flags)) != 0)
    return;

  /* remove old */
  if(old_start_changed_region != NULL){
    AgsDevout *devout;
    AgsRecall *recall;
    AgsCancelRecall *cancel_recall;

    devout = AGS_DEVOUT(AGS_AUDIO(recall_channel_run->source->audio)->devout);
    destination_recycling = old_start_changed_region;
    
    while(destination_recycling != old_end_changed_region->next){
      list = ags_recall_get_children(AGS_RECALL(recall_channel_run));

      while(list != NULL){
	if(AGS_RECALL_RECYCLING(list->data)->destination == destination_recycling){
	  recall = AGS_RECALL(list->data);

	  recall->flags |= AGS_RECALL_HIDE;
	  cancel_recall = ags_cancel_recall_new(recall,
						NULL);

	  ags_task_thread_append_task(devout->task_thread,
				      (AgsTask *) cancel_recall);
	}

	list = list->next;
      }

      destination_recycling = destination_recycling->next;
    }
  }

  /* add new */
  if(new_start_changed_region != NULL){
    if((AGS_RECALL_OUTPUT_ORIENTATED & (AGS_RECALL(recall_channel_run)->flags)) != 0 &&
       (AGS_RECALL_INPUT_ORIENTATED & (AGS_RECALL(recall_channel_run)->flags)) != 0){
      if(recall_channel_run->source->first_recycling == NULL)
	return;
      
      destination_recycling = new_start_changed_region;
      
      while(destination_recycling != new_end_changed_region->next){
	source_recycling = recall_channel_run->source->first_recycling;
	
	while(source_recycling != recall_channel_run->source->last_recycling->next){
	  if(AGS_RECALL(recall_channel_run)->child_type != G_TYPE_NONE){
	    recall_recycling = g_object_new(AGS_RECALL(recall_channel_run)->child_type,
					    "devout\0", AGS_RECALL(recall_channel_run)->devout,
					    "recall_id\0", AGS_RECALL(recall_channel_run)->recall_id,
					    "audio_channel\0", recall_channel_run->audio_channel,
					    "destination\0", destination_recycling,
					    "source\0", source_recycling,
					    NULL);
	  
	    ags_recall_add_child(AGS_RECALL(recall_channel_run), AGS_RECALL(recall_recycling));
	  }

	  source_recycling = source_recycling->next;
	}
	
	destination_recycling = destination_recycling->next;
      }
    }else{
      destination_recycling = new_start_changed_region;
      
      while(destination_recycling != new_end_changed_region->next){

	if(AGS_RECALL(recall_channel_run)->child_type != G_TYPE_NONE){
	  recall_recycling = g_object_new(AGS_RECALL(recall_channel_run)->child_type,
					  "devout\0", AGS_RECALL(recall_channel_run)->devout,
					  "recall_id\0", AGS_RECALL(recall_channel_run)->recall_id,
					  "audio_channel\0", recall_channel_run->audio_channel,
					  "destination\0", NULL,
					  "source\0", destination_recycling,
					  NULL);
	
	  ags_recall_add_child(AGS_RECALL(recall_channel_run), AGS_RECALL(recall_recycling));
	}

	destination_recycling = destination_recycling->next;
      }
    }
  }
}

void
ags_recall_channel_run_refresh_child_source(AgsRecallChannelRun *recall_channel_run,
					    AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
					    AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region)
{
  //TODO:JK: implement me
}

void
ags_recall_channel_run_refresh_child_destination(AgsRecallChannelRun *recall_channel_run,
						 AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
						 AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region)
{
  //TODO:JK: implement me
}

void
ags_recall_channel_run_source_recycling_changed_callback(AgsChannel *channel,
							 AgsRecycling *old_start_region, AgsRecycling *old_end_region,
							 AgsRecycling *new_start_region, AgsRecycling *new_end_region,
							 AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
							 AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region,
							 AgsRecallChannelRun *recall_channel_run)
{
  ags_recall_channel_run_refresh_child_source(recall_channel_run,
					      old_start_changed_region, old_end_changed_region,
					      new_start_changed_region, new_end_changed_region);
}

void
ags_recall_channel_run_destination_recycling_changed_callback(AgsChannel *channel,
							      AgsRecycling *old_start_region, AgsRecycling *old_end_region,
							      AgsRecycling *new_start_region, AgsRecycling *new_end_region,
							      AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
							      AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region,
							      AgsRecallChannelRun *recall_channel_run)
{
  ags_recall_channel_run_refresh_child_destination(recall_channel_run,
						   old_start_changed_region, old_end_changed_region,
						   new_start_changed_region, new_end_changed_region);
}

AgsGroupId
ags_recall_channel_run_real_get_audio_run_group_id(AgsRecallChannelRun *recall_channel_run)
{
  AgsAudio *audio;
  AgsRecall *recall;
  AgsGroupId group_id;

  recall = AGS_RECALL(recall_channel_run);

  if(recall->recall_id == NULL)
    return(G_MAXULONG);

  audio = AGS_AUDIO(recall_channel_run->recall_channel->source->audio);

  if((AGS_RECALL_INPUT_ORIENTATED & (recall->flags)) != 0){
    if((AGS_AUDIO_INPUT_HAS_RECYCLING & (audio->flags)) != 0){
      group_id = recall->recall_id->group_id;
    }else{
      group_id = recall->recall_id->group_id;
    }
  }else if((AGS_RECALL_OUTPUT_ORIENTATED & (recall->flags)) != 0){
    if((AGS_AUDIO_OUTPUT_HAS_RECYCLING & (audio->flags)) != 0){
      group_id = recall->recall_id->group_id;
    }else{
      group_id = recall->recall_id->group_id;
    }
  }else{
    group_id = recall->recall_id->group_id;
  }

  return(group_id);
}

AgsGroupId
ags_recall_channel_run_get_audio_run_group_id(AgsRecallChannelRun *recall_channel_run)
{
  AgsGroupId group_id;

  g_return_val_if_fail(AGS_IS_RECALL_CHANNEL_RUN(recall_channel_run), G_MAXULONG);

  g_object_ref(G_OBJECT(recall_channel_run));
  g_signal_emit(G_OBJECT(recall_channel_run),
		recall_channel_run_signals[GET_AUDIO_RUN_GROUP_ID], 0,
		&group_id);
  g_object_unref(G_OBJECT(recall_channel_run));

  return(group_id);
}

void
ags_recall_channel_run_real_run_order_changed(AgsRecallChannelRun *recall_channel_run,
					      guint run_order)
{
  recall_channel_run->run_order = run_order;
}

void
ags_recall_channel_run_run_order_changed(AgsRecallChannelRun *recall_channel_run,
					 guint run_order)
{
  g_return_if_fail(AGS_IS_RECALL(recall_channel_run));

  g_object_ref(G_OBJECT(recall_channel_run));
  g_signal_emit(G_OBJECT(recall_channel_run),
		recall_channel_run_signals[RUN_ORDER_CHANGED], 0,
		run_order);
  g_object_unref(G_OBJECT(recall_channel_run));
}

/*
guint
ags_recall_channel_run_get_run_order(AgsRecallChannelRun *recall_channel_run)
{
  AgsRunOrder *run_order;
  gint position;

  run_order = ags_run_order_find_group_id(AGS_AUDIO(recall_channel_run->source->audio)->run_order,
					  AGS_RECALL(recall_channel_run)->recall_id->group_id);

  if(run_order == 0)
    return(G_MAXUINT);


  position = g_list_index(run_order->run_order,
			  recall_channel_run->source);

  return((position != -1) ? (guint) position: 0);
}
*/

AgsRecallChannelRun*
ags_recall_channel_run_new()
{
  AgsRecallChannelRun *recall_channel_run;

  recall_channel_run = (AgsRecallChannelRun *) g_object_new(AGS_TYPE_RECALL_CHANNEL_RUN, NULL);

  return(recall_channel_run);
}
