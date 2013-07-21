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

#include <ags/audio/recall/ags_count_beats_audio_run.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_run_connectable.h>
#include <ags/object/ags_countable.h>
#include <ags/object/ags_seekable.h>

#include <ags/audio/ags_recall_container.h>

#include <ags/audio/recall/ags_count_beats_audio.h>

void ags_count_beats_audio_run_class_init(AgsCountBeatsAudioRunClass *count_beats_audio_run);
void ags_count_beats_audio_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_count_beats_audio_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable);
void ags_count_beats_audio_run_seekable_interface_init(AgsSeekableInterface *seekable);
void ags_count_beats_audio_run_countable_interface_init(AgsCountableInterface *countable);
void ags_count_beats_audio_run_init(AgsCountBeatsAudioRun *count_beats_audio_run);
void ags_count_beats_audio_run_set_property(GObject *gobject,
					    guint prop_id,
					    const GValue *value,
					    GParamSpec *param_spec);
void ags_count_beats_audio_run_get_property(GObject *gobject,
					    guint prop_id,
					    GValue *value,
					    GParamSpec *param_spec);
void ags_count_beats_audio_run_finalize(GObject *gobject);
void ags_count_beats_audio_run_connect(AgsConnectable *connectable);
void ags_count_beats_audio_run_disconnect(AgsConnectable *connectable);
void ags_count_beats_audio_run_run_connect(AgsRunConnectable *run_connectable);
void ags_count_beats_audio_run_run_disconnect(AgsRunConnectable *run_connectable);
void ags_count_beats_audio_run_seek(AgsSeekable *seekable,
				    guint steps,
				    gboolean forward);
guint ags_count_beats_audio_run_get_notation_counter(AgsCountable *countable);
guint ags_count_beats_audio_run_get_sequencer_counter(AgsCountable *countable);

void ags_count_beats_audio_run_resolve_dependencies(AgsRecall *recall);
AgsRecall* ags_count_beats_audio_run_duplicate(AgsRecall *recall,
					       AgsRecallID *recall_id,
					       guint *n_params, GParameter *parameter);
void ags_count_beats_audio_run_notify_dependency(AgsRecall *recall,
						 guint notify_mode,
						 gint count);
void ags_count_beats_audio_run_run_init_pre(AgsRecall *recall);

void ags_count_beats_audio_run_sequencer_alloc_output_callback(AgsDelayAudioRun *delay_audio_run,
							       guint nth_run, guint attack,
							       AgsCountBeatsAudioRun *count_beats_audio_run);
void ags_count_beats_audio_run_sequencer_count_callback(AgsDelayAudioRun *delay_audio_run,
							guint nth_run, guint attack,
							AgsCountBeatsAudioRun *count_beats_audio_run);

enum{
  NOTATION_START,
  NOTATION_LOOP,
  NOTATION_STOP,
  SEQUENCER_START,
  SEQUENCER_LOOP,
  SEQUENCER_STOP,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_DELAY_AUDIO_RUN,
  PROP_NOTATION_COUNTER,
  PROP_SEQUENCER_COUNTER,
};

static gpointer ags_count_beats_audio_run_parent_class = NULL;
static AgsConnectableInterface* ags_count_beats_audio_run_parent_connectable_interface;
static AgsRunConnectableInterface *ags_count_beats_audio_run_parent_run_connectable_interface;

static guint count_beats_audio_run_signals[LAST_SIGNAL];

GType
ags_count_beats_audio_run_get_type()
{
  static GType ags_type_count_beats_audio_run = 0;

  if(!ags_type_count_beats_audio_run){
    static const GTypeInfo ags_count_beats_audio_run_info = {
      sizeof (AgsCountBeatsAudioRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_count_beats_audio_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCountBeatsAudioRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_count_beats_audio_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_count_beats_audio_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_run_connectable_interface_info = {
      (GInterfaceInitFunc) ags_count_beats_audio_run_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_seekable_interface_info = {
      (GInterfaceInitFunc) ags_count_beats_audio_run_seekable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_countable_interface_info = {
      (GInterfaceInitFunc) ags_count_beats_audio_run_countable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_count_beats_audio_run = g_type_register_static(AGS_TYPE_RECALL_AUDIO_RUN,
							    "AgsCountBeatsAudioRun\0",
							    &ags_count_beats_audio_run_info,
							    0);
    
    g_type_add_interface_static(ags_type_count_beats_audio_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_count_beats_audio_run,
				AGS_TYPE_RUN_CONNECTABLE,
				&ags_run_connectable_interface_info);

    g_type_add_interface_static(ags_type_count_beats_audio_run,
				AGS_TYPE_COUNTABLE,
				&ags_countable_interface_info);
  }

  return(ags_type_count_beats_audio_run);
}

void
ags_count_beats_audio_run_class_init(AgsCountBeatsAudioRunClass *count_beats_audio_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_count_beats_audio_run_parent_class = g_type_class_peek_parent(count_beats_audio_run);

  /* GObjectClass */
  gobject = (GObjectClass *) count_beats_audio_run;

  gobject->set_property = ags_count_beats_audio_run_set_property;
  gobject->get_property = ags_count_beats_audio_run_get_property;

  gobject->finalize = ags_count_beats_audio_run_finalize;

  /* properties */
  param_spec = g_param_spec_object("delay_audio_run\0",
				   "assigned AgsDelayAudioRun\0",
				   "The AgsDelayAudioRun which emits sequencer_alloc_output sequencer_count signal\0",
				   AGS_TYPE_DELAY_AUDIO_RUN,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DELAY_AUDIO_RUN,
				  param_spec);
  
  param_spec = g_param_spec_uint("notation_counter\0",
				 "notation counter indicates offset\0",
				 "The notation counter indicates the offset in the notation\0",
				 0,
				 65535, //FIXME:JK: figure out how many beats this can really have
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_NOTATION_COUNTER,
				  param_spec);

  param_spec = g_param_spec_uint("sequencer_counter\0",
				 "sequencer counter indicates offset\0",
				 "The sequenecer counter indicates the offset in the sequencer\0",
				 0,
				 65535, //FIXME:JK: figure out how many beats this can really have
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SEQUENCER_COUNTER,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) count_beats_audio_run;

  recall->resolve_dependencies = ags_count_beats_audio_run_resolve_dependencies;
  recall->duplicate = ags_count_beats_audio_run_duplicate;
  recall->notify_dependency = ags_count_beats_audio_run_notify_dependency;
  recall->run_init_pre = ags_count_beats_audio_run_run_init_pre;

  /* AgsCountBeatsAudioRunClass */
  count_beats_audio_run->notation_start = NULL;
  count_beats_audio_run->notation_loop = NULL;
  count_beats_audio_run->notation_stop = NULL;
  count_beats_audio_run->sequencer_start = NULL;
  count_beats_audio_run->sequencer_loop = NULL;
  count_beats_audio_run->sequencer_stop = NULL;

  /* signals */
  count_beats_audio_run_signals[NOTATION_START] =
    g_signal_new("notation_start\0",
		 G_TYPE_FROM_CLASS(count_beats_audio_run),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsCountBeatsAudioRunClass, notation_start),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__UINT,
		 G_TYPE_NONE, 1,
		 G_TYPE_UINT);

  count_beats_audio_run_signals[NOTATION_LOOP] =
    g_signal_new("notation_loop\0",
		 G_TYPE_FROM_CLASS(count_beats_audio_run),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsCountBeatsAudioRunClass, notation_loop),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__UINT,
		 G_TYPE_NONE, 1,
		 G_TYPE_UINT);

  count_beats_audio_run_signals[NOTATION_STOP] =
    g_signal_new("notation_stop\0",
		 G_TYPE_FROM_CLASS(count_beats_audio_run),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsCountBeatsAudioRunClass, notation_stop),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__UINT,
		 G_TYPE_NONE, 1,
		 G_TYPE_UINT);

  count_beats_audio_run_signals[SEQUENCER_START] =
    g_signal_new("sequencer_start\0",
		 G_TYPE_FROM_CLASS(count_beats_audio_run),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsCountBeatsAudioRunClass, sequencer_start),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__UINT,
		 G_TYPE_NONE, 1,
		 G_TYPE_UINT);

  count_beats_audio_run_signals[SEQUENCER_LOOP] =
    g_signal_new("sequencer_loop\0",
		 G_TYPE_FROM_CLASS(count_beats_audio_run),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsCountBeatsAudioRunClass, sequencer_loop),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__UINT,
		 G_TYPE_NONE, 1,
		 G_TYPE_UINT);

  count_beats_audio_run_signals[SEQUENCER_STOP] =
    g_signal_new("sequencer_stop\0",
		 G_TYPE_FROM_CLASS(count_beats_audio_run),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsCountBeatsAudioRunClass, sequencer_stop),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__UINT,
		 G_TYPE_NONE, 1,
		 G_TYPE_UINT);
}

void
ags_count_beats_audio_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_count_beats_audio_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_count_beats_audio_run_connect;
  connectable->disconnect = ags_count_beats_audio_run_disconnect;
}

void
ags_count_beats_audio_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable)
{
  ags_count_beats_audio_run_parent_run_connectable_interface = g_type_interface_peek_parent(run_connectable);

  run_connectable->connect = ags_count_beats_audio_run_run_connect;
  run_connectable->disconnect = ags_count_beats_audio_run_run_disconnect;
}

void
ags_count_beats_audio_run_seekable_interface_init(AgsSeekableInterface *seekable)
{
  seekable->seek = ags_count_beats_audio_run_seek;
}

void
ags_count_beats_audio_run_countable_interface_init(AgsCountableInterface *countable)
{
  countable->get_notation_counter = ags_count_beats_audio_run_get_notation_counter;
  countable->get_sequencer_counter = ags_count_beats_audio_run_get_sequencer_counter;
}

void
ags_count_beats_audio_run_init(AgsCountBeatsAudioRun *count_beats_audio_run)
{
  count_beats_audio_run->first_run_counter = 0;

  count_beats_audio_run->notation_counter = 0;
  count_beats_audio_run->sequencer_counter = 0;

  count_beats_audio_run->recall_ref = 0;

  count_beats_audio_run->hide_ref = 0;
  count_beats_audio_run->notation_hide_ref_counter = 0;
  count_beats_audio_run->sequencer_hide_ref_counter = 0;

  count_beats_audio_run->delay_audio_run = NULL;
}

void ags_count_beats_audio_run_set_property(GObject *gobject,
					    guint prop_id,
					    const GValue *value,
					    GParamSpec *param_spec)
{
  AgsCountBeatsAudioRun *count_beats_audio_run;

  count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(gobject);

  switch(prop_id){
  case PROP_DELAY_AUDIO_RUN:
    {
      AgsDelayAudioRun *delay_audio_run;
      gboolean is_template;

      delay_audio_run = (AgsDelayAudioRun *) g_value_get_object(value);

      if(count_beats_audio_run->delay_audio_run == delay_audio_run)
	return;

      if(delay_audio_run != NULL &&
	 (AGS_RECALL_TEMPLATE & (AGS_RECALL(delay_audio_run)->flags)) != 0){
	is_template = TRUE;
      }else{
	is_template = FALSE;
      }

      if(count_beats_audio_run->delay_audio_run != NULL){
	if(is_template){
	  ags_recall_remove_dependency(AGS_RECALL(count_beats_audio_run),
				       (AgsRecall *) count_beats_audio_run->delay_audio_run);
	}else{
	  if((AGS_RECALL_RUN_INITIALIZED & (AGS_RECALL(count_beats_audio_run)->flags)) != 0){
	    g_signal_handler_disconnect(G_OBJECT(count_beats_audio_run),
					count_beats_audio_run->sequencer_alloc_output_handler);

	    g_signal_handler_disconnect(G_OBJECT(count_beats_audio_run),
					count_beats_audio_run->sequencer_count_handler);
	  }
	}
	
	g_object_unref(G_OBJECT(count_beats_audio_run->delay_audio_run));
      }

      if(delay_audio_run != NULL){
	g_object_ref(G_OBJECT(delay_audio_run));

	if(is_template){
	  ags_recall_add_dependency(AGS_RECALL(count_beats_audio_run),
				    ags_recall_dependency_new((GObject *) delay_audio_run));
	}else{
	  if((AGS_RECALL_RUN_INITIALIZED & (AGS_RECALL(count_beats_audio_run)->flags)) != 0){
	    count_beats_audio_run->sequencer_count_handler =
	      g_signal_connect(G_OBJECT(delay_audio_run), "sequencer_alloc_output\0",
			       G_CALLBACK(ags_count_beats_audio_run_sequencer_alloc_output_callback), count_beats_audio_run);

	    count_beats_audio_run->sequencer_count_handler =
	      g_signal_connect(G_OBJECT(delay_audio_run), "sequencer_count\0",
			       G_CALLBACK(ags_count_beats_audio_run_sequencer_count_callback), count_beats_audio_run);
	  }
	}
      }

      count_beats_audio_run->delay_audio_run = delay_audio_run;
    }
    break;
  case PROP_NOTATION_COUNTER:
    {
      guint counter;

      counter = g_value_get_uint(value);

      count_beats_audio_run->notation_counter = counter;
    }
    break;
  case PROP_SEQUENCER_COUNTER:
    {
      guint counter;

      counter = g_value_get_uint(value);

      count_beats_audio_run->sequencer_counter = counter;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void ags_count_beats_audio_run_get_property(GObject *gobject,
					    guint prop_id,
					    GValue *value,
					    GParamSpec *param_spec)
{
  AgsCountBeatsAudioRun *count_beats_audio_run;
  
  count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(gobject);

  switch(prop_id){
  case PROP_DELAY_AUDIO_RUN:
    {
      g_value_set_object(value, count_beats_audio_run->delay_audio_run);
    }
    break;
  case PROP_NOTATION_COUNTER:
    {
      g_value_set_uint(value, count_beats_audio_run->notation_counter);
    }
    break;
  case PROP_SEQUENCER_COUNTER:
    {
      g_value_set_uint(value, count_beats_audio_run->sequencer_counter);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_count_beats_audio_run_finalize(GObject *gobject)
{
  AgsCountBeatsAudioRun *count_beats_audio_run;

  count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(gobject);

  if(count_beats_audio_run->delay_audio_run != NULL)
    g_object_unref(count_beats_audio_run->delay_audio_run);

  G_OBJECT_CLASS(ags_count_beats_audio_run_parent_class)->finalize(gobject);
}

void
ags_count_beats_audio_run_connect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_count_beats_audio_run_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_count_beats_audio_run_run_connect(AgsRunConnectable *run_connectable)
{
  AgsCountBeatsAudioRun *count_beats_audio_run;

  ags_count_beats_audio_run_parent_run_connectable_interface->connect(run_connectable);

  /* AgsCountBeats */
  count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(run_connectable);

  count_beats_audio_run->sequencer_alloc_output_handler =
    g_signal_connect(G_OBJECT(count_beats_audio_run->delay_audio_run), "sequencer_alloc_output\0",
		     G_CALLBACK(ags_count_beats_audio_run_sequencer_alloc_output_callback), count_beats_audio_run);

  count_beats_audio_run->sequencer_count_handler =
    g_signal_connect(G_OBJECT(count_beats_audio_run->delay_audio_run), "sequencer_count\0",
		     G_CALLBACK(ags_count_beats_audio_run_sequencer_count_callback), count_beats_audio_run);
}

void
ags_count_beats_audio_run_run_disconnect(AgsRunConnectable *run_connectable)
{
  AgsCountBeatsAudioRun *count_beats_audio_run;

  ags_count_beats_audio_run_parent_run_connectable_interface->connect(run_connectable);

  /* AgsCountBeats */
  count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(run_connectable);

  g_signal_handler_disconnect(G_OBJECT(count_beats_audio_run), count_beats_audio_run->sequencer_alloc_output_handler);
  g_signal_handler_disconnect(G_OBJECT(count_beats_audio_run), count_beats_audio_run->sequencer_count_handler);
}

void
ags_count_beats_audio_run_seek(AgsSeekable *seekable,
			       guint steps,
			       gboolean forward)
{
  //TODO:JK: implement this function
}

guint
ags_count_beats_audio_run_get_notation_counter(AgsCountable *countable)
{
  return(AGS_COUNT_BEATS_AUDIO_RUN(countable)->notation_counter);
}

guint
ags_count_beats_audio_run_get_sequencer_counter(AgsCountable *countable)
{
  return(AGS_COUNT_BEATS_AUDIO_RUN(countable)->sequencer_counter);
}

void
ags_count_beats_audio_run_resolve_dependencies(AgsRecall *recall)
{
  AgsRecall *template;
  AgsCountBeatsAudioRun *count_beats_audio_run;
  AgsRecallDependency *recall_dependency;
  AgsDelayAudioRun *delay_audio_run;
  GList *list;
  AgsGroupId group_id;
  guint i, i_stop;

  count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(recall);

  template = AGS_RECALL(ags_recall_find_template(AGS_RECALL_CONTAINER(recall->container)->recall_audio_run)->data);

  list = template->dependencies;
  group_id = recall->recall_id->group_id;

  delay_audio_run = NULL;

  i_stop = 1;

  for(i = 0; i < i_stop && list != NULL;){
    recall_dependency = AGS_RECALL_DEPENDENCY(list->data);

    if(AGS_IS_DELAY_AUDIO_RUN(recall_dependency->dependency)){
      delay_audio_run = (AgsDelayAudioRun *) ags_recall_dependency_resolve(recall_dependency, group_id);

      i++;
    }

    list = list->next;
  }

  g_object_set(G_OBJECT(recall),
	       "delay_audio_run\0", delay_audio_run,
	       NULL);
}

AgsRecall*
ags_count_beats_audio_run_duplicate(AgsRecall *recall,
				    AgsRecallID *recall_id,
				    guint *n_params, GParameter *parameter)
{
  AgsCountBeatsAudioRun *copy;

  copy = AGS_COUNT_BEATS_AUDIO_RUN(AGS_RECALL_CLASS(ags_count_beats_audio_run_parent_class)->duplicate(recall,
												       recall_id,
												       n_params, parameter));

  g_message("ags_count_beats_audio_run_duplicate\n\0");

  return((AgsRecall *) copy);
}

void
ags_count_beats_audio_run_notify_dependency(AgsRecall *recall,
					    guint notify_mode,
					    gint count)
{
  AgsCountBeatsAudioRun *count_beats_audio_run;

  count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(recall);

  switch(notify_mode){
  case AGS_RECALL_NOTIFY_RUN:
    count_beats_audio_run->hide_ref += count;
    g_message("count_beats_audio_run->hide_ref: %u\n\0", count_beats_audio_run->hide_ref);
    break;
  case AGS_RECALL_NOTIFY_AUDIO:
    break;
  case AGS_RECALL_NOTIFY_AUDIO_RUN:
    break;
  case AGS_RECALL_NOTIFY_CHANNEL:
    break;
  case AGS_RECALL_NOTIFY_CHANNEL_RUN:
    count_beats_audio_run->recall_ref += count;

    break;
  default:
    g_message("ags_count_beats_audio_run.c - ags_count_beats_audio_run_notify: unknown notify\0");
  }
}

void
ags_count_beats_audio_run_run_init_pre(AgsRecall *recall)
{
  AgsCountBeatsAudioRun *count_beats_audio_run;

  AGS_RECALL_CLASS(ags_count_beats_audio_run_parent_class)->run_init_pre(recall);

  count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(recall);

  count_beats_audio_run->first_run_counter = 0;
}

void
ags_count_beats_audio_run_notation_start(AgsCountBeatsAudioRun *count_beats_audio_run,
					 guint nth_run)
{
  g_return_if_fail(AGS_IS_COUNT_BEATS_AUDIO_RUN(count_beats_audio_run));

  g_object_ref(G_OBJECT(count_beats_audio_run));
  g_signal_emit(G_OBJECT(count_beats_audio_run),
		count_beats_audio_run_signals[NOTATION_START], 0,
		nth_run);
  g_object_unref(G_OBJECT(count_beats_audio_run));
}

void
ags_count_beats_audio_run_notation_loop(AgsCountBeatsAudioRun *count_beats_audio_run,
					guint nth_run)
{
  g_return_if_fail(AGS_IS_COUNT_BEATS_AUDIO_RUN(count_beats_audio_run));

  g_object_ref(G_OBJECT(count_beats_audio_run));
  g_signal_emit(G_OBJECT(count_beats_audio_run),
		count_beats_audio_run_signals[NOTATION_LOOP], 0,
		nth_run);
  g_object_unref(G_OBJECT(count_beats_audio_run));
}

void
ags_count_beats_audio_run_notation_stop(AgsCountBeatsAudioRun *count_beats_audio_run,
					guint nth_run)
{
  g_return_if_fail(AGS_IS_COUNT_BEATS_AUDIO_RUN(count_beats_audio_run));

  g_object_ref(G_OBJECT(count_beats_audio_run));
  g_signal_emit(G_OBJECT(count_beats_audio_run),
		count_beats_audio_run_signals[NOTATION_STOP], 0,
		nth_run);
  g_object_unref(G_OBJECT(count_beats_audio_run));
}

void
ags_count_beats_audio_run_sequencer_start(AgsCountBeatsAudioRun *count_beats_audio_run,
					  guint nth_run)
{
  g_return_if_fail(AGS_IS_COUNT_BEATS_AUDIO_RUN(count_beats_audio_run));

  g_object_ref(G_OBJECT(count_beats_audio_run));
  g_signal_emit(G_OBJECT(count_beats_audio_run),
		count_beats_audio_run_signals[SEQUENCER_START], 0,
		nth_run);
  g_object_unref(G_OBJECT(count_beats_audio_run));
}

void
ags_count_beats_audio_run_sequencer_loop(AgsCountBeatsAudioRun *count_beats_audio_run,
					 guint nth_run)
{
  g_return_if_fail(AGS_IS_COUNT_BEATS_AUDIO_RUN(count_beats_audio_run));

  g_object_ref(G_OBJECT(count_beats_audio_run));
  g_signal_emit(G_OBJECT(count_beats_audio_run),
		count_beats_audio_run_signals[SEQUENCER_LOOP], 0,
		nth_run);
  g_object_unref(G_OBJECT(count_beats_audio_run));
}

void
ags_count_beats_audio_run_sequencer_stop(AgsCountBeatsAudioRun *count_beats_audio_run,
					 guint nth_run)
{
  g_return_if_fail(AGS_IS_COUNT_BEATS_AUDIO_RUN(count_beats_audio_run));

  g_object_ref(G_OBJECT(count_beats_audio_run));
  g_signal_emit(G_OBJECT(count_beats_audio_run),
		count_beats_audio_run_signals[SEQUENCER_STOP], 0,
		nth_run);
  g_object_unref(G_OBJECT(count_beats_audio_run));
}

void
ags_count_beats_audio_run_notation_alloc_output_callback(AgsDelayAudioRun *delay_audio_run,
							 guint nth_run, guint attack,
							 AgsCountBeatsAudioRun *count_beats_audio_run)
{
  AgsCountBeatsAudio *count_beats_audio;

  count_beats_audio = AGS_COUNT_BEATS_AUDIO(count_beats_audio_run->recall_audio_run.recall_audio);

  if(count_beats_audio_run->notation_counter == 0){
    /* emit notation signals */
    if(count_beats_audio_run->first_run_counter != count_beats_audio_run->hide_ref){
      ags_count_beats_audio_run_notation_start(count_beats_audio_run,
					       nth_run);
    }

    if(count_beats_audio->loop){
      if(count_beats_audio_run->first_run_counter == count_beats_audio_run->hide_ref){
	ags_count_beats_audio_run_notation_loop(count_beats_audio_run,
						nth_run);

	g_message("ags_count_beats_audio_run_notation_alloc_output_callback: loop\n\0");
      }
    }else{
      if((AGS_RECALL_PERSISTENT & (AGS_RECALL(count_beats_audio_run)->flags)) == 0){
	g_message("ags_count_beats_audio_run_notation_alloc_output_callback: done\n\0");

	ags_recall_done(AGS_RECALL(count_beats_audio_run));
      }
    }
  }
}

void
ags_count_beats_audio_run_sequencer_alloc_output_callback(AgsDelayAudioRun *delay_audio_run,
							  guint nth_run, guint attack,
							  AgsCountBeatsAudioRun *count_beats_audio_run)
{
  AgsCountBeatsAudio *count_beats_audio;

  count_beats_audio = AGS_COUNT_BEATS_AUDIO(count_beats_audio_run->recall_audio_run.recall_audio);


  if(count_beats_audio_run->sequencer_counter == 0){
    /* emit sequencer signals */
    if(count_beats_audio_run->first_run_counter != count_beats_audio_run->hide_ref){
      g_message("ags_count_beats_audio_run_sequencer_alloc_output_callback: start\n\0");
      ags_count_beats_audio_run_sequencer_start(count_beats_audio_run,
						nth_run);
    }

    if(count_beats_audio->loop){
      if(count_beats_audio_run->first_run_counter == count_beats_audio_run->hide_ref){
	g_message("ags_count_beats_audio_run_sequencer_alloc_output_callback: loop\n\0");
      
	ags_count_beats_audio_run_sequencer_loop(count_beats_audio_run,
						 nth_run);
      }
    }else{
      /* emit stop signals */
      if((AGS_RECALL_PERSISTENT & (AGS_RECALL(count_beats_audio_run)->flags)) == 0){
	g_message("ags_count_beats_audio_run_sequencer_alloc_output_callback: done\n\0");

	ags_count_beats_audio_run_sequencer_stop(count_beats_audio_run,
						 nth_run);
	
	ags_count_beats_audio_run_notation_stop(count_beats_audio_run,
						nth_run);
	
	ags_recall_done(AGS_RECALL(count_beats_audio_run));
      }
    }

    if(count_beats_audio_run->first_run_counter != count_beats_audio_run->hide_ref){
      count_beats_audio_run->first_run_counter += 1;
    }
  }
}

void
ags_count_beats_audio_run_notation_count_callback(AgsDelayAudioRun *delay_audio_run,
						   guint nth_run, guint attack,
						   AgsCountBeatsAudioRun *count_beats_audio_run)
{
  AgsCountBeatsAudio *count_beats_audio;

  count_beats_audio = AGS_COUNT_BEATS_AUDIO(count_beats_audio_run->recall_audio_run.recall_audio);

  /* 
   * Block counter for sequencer and notation counter
   */
  if(count_beats_audio_run->hide_ref != 0){
    count_beats_audio_run->notation_hide_ref_counter += 1;
  }

  if(count_beats_audio_run->notation_hide_ref_counter == count_beats_audio_run->hide_ref){
    if(count_beats_audio->loop){
      if(count_beats_audio_run->notation_counter == count_beats_audio->notation_loop_end - 1){
	count_beats_audio_run->notation_counter = 0;
      }
    }else{
      count_beats_audio_run->notation_counter += 1;
    }

    count_beats_audio_run->notation_hide_ref_counter = 0;
  }
}

void
ags_count_beats_audio_run_sequencer_count_callback(AgsDelayAudioRun *delay_audio_run,
						   guint nth_run, guint attack,
						   AgsCountBeatsAudioRun *count_beats_audio_run)
{
  AgsCountBeatsAudio *count_beats_audio;

  count_beats_audio = AGS_COUNT_BEATS_AUDIO(count_beats_audio_run->recall_audio_run.recall_audio);

  /* 
   * Block counter for sequencer and notation counter
   */
  if(count_beats_audio_run->hide_ref != 0){
    count_beats_audio_run->sequencer_hide_ref_counter += 1;
  }

  if(count_beats_audio_run->sequencer_hide_ref_counter == count_beats_audio_run->hide_ref){
    //    g_message("sequencer: tic\0");
  
    if(count_beats_audio->loop){
      if(count_beats_audio_run->sequencer_counter == count_beats_audio->sequencer_loop_end - 1){
	count_beats_audio_run->sequencer_counter = 0;
      }else{
	count_beats_audio_run->sequencer_counter += 1;
      }
    }else{
      if(count_beats_audio_run->sequencer_counter == count_beats_audio->sequencer_loop_end - 1)
	return;

      count_beats_audio_run->sequencer_counter += 1;
    }

    count_beats_audio_run->sequencer_hide_ref_counter = 0;
  }
}

AgsCountBeatsAudioRun*
ags_count_beats_audio_run_new(AgsDelayAudioRun *delay_audio_run)
{
  AgsCountBeatsAudioRun *count_beats_audio_run;

  count_beats_audio_run = (AgsCountBeatsAudioRun *) g_object_new(AGS_TYPE_COUNT_BEATS_AUDIO_RUN,
								 "delay_audio_run\0", delay_audio_run,
								 NULL);
  
  return(count_beats_audio_run);
}
