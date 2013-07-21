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

#include <ags/audio/ags_audio_signal.h>

#include <ags/object/ags_marshal.h>
#include <ags/object/ags_connectable.h>

#include <ags/audio/ags_devout.h>

#include <stdlib.h>
#include <math.h>
#include <string.h>

void ags_audio_signal_class_init(AgsAudioSignalClass *audio_signal_class);
void ags_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_audio_signal_init(AgsAudioSignal *audio_signal);
void ags_audio_signal_set_property(GObject *gobject,
				   guint prop_id,
				   const GValue *value,
				   GParamSpec *param_spec);
void ags_audio_signal_get_property(GObject *gobject,
				   guint prop_id,
				   GValue *value,
				   GParamSpec *param_spec);
void ags_audio_signal_finalize(GObject *gobject);
void ags_audio_signal_connect(AgsConnectable *connectable);
void ags_audio_signal_disconnect(AgsConnectable *connectable);

void ags_audio_signal_real_realloc_buffer_size(AgsAudioSignal *audio_signal, guint buffer_size);
void ags_audio_signal_real_morph_samplerate(AgsAudioSignal *audio_signal, guint samplerate, double k_morph);

enum{
  PROP_0,
  PROP_DEVOUT,
  PROP_RECYCLING,
  PROP_RECALL_ID,
};

enum{
  REALLOC_BUFFER_SIZE,
  MORPH_SAMPLERATE,
  LAST_SIGNAL,
};

static gpointer ags_audio_signal_parent_class = NULL;
static guint audio_signal_signals[LAST_SIGNAL];

GType
ags_audio_signal_get_type(void)
{
  static GType ags_type_audio_signal = 0;

  if(!ags_type_audio_signal){
    static const GTypeInfo ags_audio_signal_info = {
      sizeof (AgsAudioSignalClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_audio_signal_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAudioSignal),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_audio_signal_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_audio_signal_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_audio_signal = g_type_register_static(G_TYPE_OBJECT,
						   "AgsAudioSignal\0",
						   &ags_audio_signal_info,
						   0);

    g_type_add_interface_static(ags_type_audio_signal,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_audio_signal);
}

void
ags_audio_signal_class_init(AgsAudioSignalClass *audio_signal)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_audio_signal_parent_class = g_type_class_peek_parent(audio_signal);

  /* GObjectClass */
  gobject = (GObjectClass *) audio_signal;

  gobject->set_property = ags_audio_signal_set_property;
  gobject->get_property = ags_audio_signal_get_property;

  gobject->finalize = ags_audio_signal_finalize;

  /* properties */
  param_spec = g_param_spec_object("devout\0",
				   "assigned devout\0",
				   "The devout it is assigned with\0",
				   AGS_TYPE_DEVOUT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DEVOUT,
				  param_spec);

  param_spec = g_param_spec_object("recycling\0",
				   "assigned recycling\0",
				   "The devout it is assigned with\0",
				   AGS_TYPE_RECYCLING,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECYCLING,
				  param_spec);

  param_spec = g_param_spec_object("recall-id\0",
				   "assigned recall id\0",
				   "The recall id it is assigned with\0",
				   AGS_TYPE_RECALL_ID,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECALL_ID,
				  param_spec);

  /*  */
  audio_signal->realloc_buffer_size = ags_audio_signal_real_realloc_buffer_size;
  audio_signal->morph_samplerate = ags_audio_signal_real_morph_samplerate;

  audio_signal_signals[REALLOC_BUFFER_SIZE] =
    g_signal_new("realloc_buffer_size\0",
		 G_TYPE_FROM_CLASS (audio_signal),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsAudioSignalClass, realloc_buffer_size),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__UINT,
		 G_TYPE_NONE, 1,
		 G_TYPE_UINT);

  audio_signal_signals[MORPH_SAMPLERATE] =
    g_signal_new("morph_samplerate\0",
		 G_TYPE_FROM_CLASS (audio_signal),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsAudioSignalClass, morph_samplerate),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__UINT_DOUBLE,
		 G_TYPE_NONE, 2,
		 G_TYPE_UINT, G_TYPE_DOUBLE);
}

void
ags_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_audio_signal_connect;
  connectable->disconnect = ags_audio_signal_disconnect;
}

void
ags_audio_signal_init(AgsAudioSignal *audio_signal)
{
  audio_signal->flags = 0;

  audio_signal->devout = NULL;

  audio_signal->recycling = NULL;
  audio_signal->recall_id = NULL;

  audio_signal->samplerate = AGS_DEVOUT_DEFAULT_SAMPLERATE;
  audio_signal->buffer_size = AGS_DEVOUT_DEFAULT_BUFFER_SIZE;

  audio_signal->length = 0;
  audio_signal->last_frame = 0;
  audio_signal->loop_start = 0;
  audio_signal->loop_end = 0;

  audio_signal->stream_beginning = NULL;
  audio_signal->stream_current = NULL;
  audio_signal->stream_end = NULL;
}

void
ags_audio_signal_set_property(GObject *gobject,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *param_spec)
{
  AgsAudioSignal *audio_signal;

  audio_signal = AGS_AUDIO_SIGNAL(gobject);

  switch(prop_id){
  case PROP_DEVOUT:
    {
      GObject *devout;

      devout = g_value_get_object(value);

      if(audio_signal->devout == devout)
	return;

      if(audio_signal->devout != NULL)
	g_object_unref(audio_signal->devout);

      if(devout != NULL)
	g_object_ref(devout);

      audio_signal->devout = devout;
    }
    break;
  case PROP_RECYCLING:
    {
      GObject *recycling;

      recycling = g_value_get_object(value);

      if(audio_signal->recycling == recycling)
	return;

      if(audio_signal->recycling != NULL)
	g_object_unref(audio_signal->recycling);

      if(recycling != NULL)
	g_object_ref(recycling);

      audio_signal->recycling = recycling;
    }
    break;
  case PROP_RECALL_ID:
    {
      GObject *recall_id;

      recall_id = g_value_get_object(value);
      
      if(audio_signal->recall_id == recall_id)
	return;
      
      if(audio_signal->recall_id != NULL)
	g_object_unref(audio_signal->recall_id);

      if(recall_id != NULL)
	g_object_ref(recall_id);

      audio_signal->recall_id = recall_id;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_audio_signal_get_property(GObject *gobject,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *param_spec)
{
  AgsAudioSignal *audio_signal;

  audio_signal = AGS_AUDIO_SIGNAL(gobject);

  switch(prop_id){
  case PROP_DEVOUT:
    g_value_set_object(value, audio_signal->devout);
    break;
  case PROP_RECYCLING:
    g_value_set_object(value, audio_signal->recycling);
    break;
  case PROP_RECALL_ID:
    g_value_set_object(value, audio_signal->recall_id);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_audio_signal_finalize(GObject *gobject)
{
  AgsAudioSignal *audio_signal;

  audio_signal = AGS_AUDIO_SIGNAL(gobject);

  g_message("finalize AgsAudioSignal\0");

  if((AGS_AUDIO_SIGNAL_TEMPLATE & (audio_signal->flags)) != 0)
    g_warning("AGS_AUDIO_SIGNAL_TEMPLATE: destroying\n\0");

  if(audio_signal->devout != NULL)
    g_object_unref(audio_signal->devout);

  if(audio_signal->recycling != NULL)
    g_object_unref(audio_signal->recycling);

  if(audio_signal->recall_id != NULL)
    g_object_unref(audio_signal->recall_id);

  ags_list_free_and_free_link(audio_signal->stream_beginning);

  g_message("post: finalize AgsAudioSignal\0");

  /* call parent */
  G_OBJECT_CLASS(ags_audio_signal_parent_class)->finalize(gobject);
}

signed short*
ags_stream_alloc(guint buffer_size)
{
  signed short *buffer;

  buffer = (signed short *) malloc(buffer_size * sizeof(signed short));
  memset(buffer, 0, buffer_size * sizeof(signed short));

  return(buffer);
}

void
ags_audio_signal_connect(AgsConnectable *connectable)
{
  // empty
}

void
ags_audio_signal_disconnect(AgsConnectable *connectable)
{
  // empty
}

/**
 * ags_attack_alloc:
 * @first_start a guint indicating the very first frame of the stream.
 * @first_length a guint telling how many frames remain from @first_start
 * to the end of a buffer.
 * @second_start a guint generally just used by #AgsDevout.
 * @second_length a guint generally just used by #AgsDevout.
 * Returns: the allocated #AgsAttack.
 *
 * Allocs an #AgsAttack you probably never may use this function directly
 * you rather want to duplicate an existing #AgsAttack.
 */
AgsAttack*
ags_attack_alloc(guint first_start, guint first_length,
		 guint second_start, guint second_length)
{
  AgsAttack *attack;

  attack = (AgsAttack *) malloc(sizeof(AgsAttack));

  attack->flags = 0;

  attack->first_start = first_start;
  attack->first_length = first_length;
  attack->second_start = second_start;
  attack->second_length = second_length;

  return(attack);
}

/**
 * ags_attack_duplicate:
 * @attack the #AgsAttack to duplicate.
 * Returns: the duplicated #AgsAttack.
 *
 * Duplicates an #AgsAttack.
 */
AgsAttack*
ags_attack_duplicate(AgsAttack *attack)
{
  AgsAttack *copy;

  if(attack == NULL)
    return(NULL);

  copy = (AgsAttack *) malloc(sizeof(AgsAttack));

  copy->flags = 0;

  copy->first_start = attack->first_start;
  copy->first_length = attack->first_length;
  copy->second_start = attack->second_start;
  copy->second_length = attack->second_length;

  return(copy);
}

/**
 * ags_attack_duplicate_from_devout:
 * @gobject an #AgsDevout.
 * Returns: the duplicated #AgsAttack.
 *
 * Retrieves streaming offsets of the current stream position
 * and bpm. 
 */
AgsAttack*
ags_attack_duplicate_from_devout(GObject *gobject)
{
  AgsDevout *devout;
  AgsAttack *copy, *attack;
  GValue attack_value = {0,};

  devout = AGS_DEVOUT(gobject);

  g_value_init(&attack_value, G_TYPE_POINTER);
  g_object_get_property(G_OBJECT(devout),
			"attack\0",
			&attack_value);
  attack = g_value_get_pointer(&attack_value);
  g_value_unset(&attack_value);

  if((AGS_DEVOUT_ATTACK_FIRST & (devout->flags)) != 0)
    copy = ags_attack_alloc(attack->first_start, attack->first_length,
			    0, 0);
  else
    copy = ags_attack_alloc(attack->second_start, attack->second_length,
			    0, 0);

  return(copy);
}

/**
 * ags_audio_signal_get_length_till_current:
 * @audio_signal an #AgsAudioSignal
 * Returns: the counted length.
 *
 * Counts the buffers from stream_beginning upto stream_current.
 */
guint
ags_audio_signal_get_length_till_current(AgsAudioSignal *audio_signal)
{
  GList *list, *stop;
  guint length;

  if(audio_signal == NULL){
    return(0);
  }

  list = audio_signal->stream_beginning;
  length = 0;

  if(audio_signal->stream_current != NULL){
    stop = audio_signal->stream_current->next;
  }else{
    return(0);
  }

  while(list != stop){
    length++;
    list = list->next;
  }

  return(length);
}

/**
 * ags_audio_signal_add_stream:
 * @audio_signal an #AgsAudioSignal
 *
 * Adds a buffer at the end of the stream.
 */
void
ags_audio_signal_add_stream(AgsAudioSignal *audio_signal)
{
  GList *stream, *end_old;

  stream = g_list_alloc();
  stream->data = (gpointer) ags_stream_alloc(AGS_DEVOUT(audio_signal->devout)->buffer_size);

  end_old = audio_signal->stream_end;

  if(end_old != NULL){
    stream->prev = end_old;
    end_old->next = stream;
  }else{
    audio_signal->stream_beginning = stream;
    audio_signal->stream_current = stream;
  }

  audio_signal->stream_end = stream;

  audio_signal->length++;
}

/**
 * ags_audio_signal_stream_resize:
 * @audio_signal an #AgsAudioSignal to resize.
 * @length a guint as the new length.
 *
 * Resizes an #AgsAudioSignal's stream but be carefull with shrinking.
 * This function may crash the application.
 */
void
ags_audio_signal_stream_resize(AgsAudioSignal *audio_signal, guint length)
{
  guint i;

  if(audio_signal->length < length){
    GList *start, *stream, *end_old;

    start =
      stream = g_list_alloc();
    stream->data = (gpointer) ags_stream_alloc(AGS_DEVOUT(audio_signal->devout)->buffer_size);

    for(i = audio_signal->length; i < length; i++){
      stream->next = g_list_alloc();
      stream->next->prev = stream;
      stream = stream->next;
      stream->data = (gpointer) ags_stream_alloc(AGS_DEVOUT(audio_signal->devout)->buffer_size);

    }

    end_old = audio_signal->stream_end;

    if(end_old != NULL){
      start->prev = end_old;
      end_old->next = stream;
    }else{
      audio_signal->stream_beginning = start;
      //      audio_signal->stream_current = start;
    }
    
    audio_signal->stream_end = stream;
  }else if(audio_signal->length > length){
    GList *stream, *stream_end, *stream_next;
    gboolean check_current;

    stream = audio_signal->stream_beginning;
    check_current = TRUE;

    for(i = 0; i < length; i++){
      if(check_current && stream == audio_signal->stream_current){
	audio_signal->stream_current = NULL;
	check_current = FALSE;
      }

      stream = stream->next;
    }


    if(length != 0){
      stream_end = stream->prev;
      stream_end->next = NULL;
    }else{
      audio_signal->stream_beginning = NULL;
      audio_signal->stream_current = NULL;
      audio_signal->stream_end = NULL;
    }

    while(stream != NULL){
      stream_next = stream->next;
      free((signed short *) stream->data);
      g_list_free1(stream);
      stream = stream_next;
    }
  }

  audio_signal->length = length;
}

/**
 * ags_audio_signal_stream_safe_resize:
 * @audio_signal an #AgsAudioSignal
 * @length a guint
 *
 * Resizes an #AgsAudioSignal's stream but doesn't shrink more than the
 * current stream position.
 */
void
ags_audio_signal_stream_safe_resize(AgsAudioSignal *audio_signal, guint length)
{
  guint length_till_current;

  length_till_current = ags_audio_signal_get_length_till_current(audio_signal);

  if(length_till_current < length){
    ags_audio_signal_stream_resize(audio_signal,
				   length);
  }else{
    ags_audio_signal_stream_resize(audio_signal,
				   length_till_current);
  }
}

void
ags_audio_signal_real_realloc_buffer_size(AgsAudioSignal *audio_signal, guint buffer_size)
{
  GList *current, *old; 
  guint old_buffer_size;
  guint counter;
  guint i_old, i_current;

  old = audio_signal->stream_beginning;
  old_buffer_size = audio_signal->buffer_size;

  current = NULL;
  current = g_list_prepend(current,
			   ags_stream_alloc(buffer_size));
  counter = 0;

  i_old = 0;
  i_current = 0;
  
  while(old != NULL){
    ((signed short *) current->data)[i_current] = ((signed short *) old->data)[i_old];
    
    if(i_current == buffer_size){
      current = g_list_prepend(current,
			       ags_stream_alloc(buffer_size));
      
      i_current = 0;
      
      counter++;
    }
    
    if(i_old == old_buffer_size){
      old = old->next;
      
      i_old = 0;
    }
    
    i_current++;
    i_old++;
  }
  
  memset(((signed short *) current->data), 0, (buffer_size - i_current) * sizeof(signed short));
  
  /*  */
  audio_signal->length = counter;
  audio_signal->last_frame = i_current;
  
  audio_signal->stream_end = current;
  audio_signal->stream_beginning = g_list_reverse(current);
}

/**
 * ags_audio_signal_realloc_buffer_size:
 * @audio_signal
 * @buffer_size
 *
 * Realloc the stream to the new buffer size. 
 */
void
ags_audio_signal_realloc_buffer_size(AgsAudioSignal *audio_signal, guint buffer_size)
{
  g_return_if_fail(AGS_IS_AUDIO_SIGNAL(audio_signal));
  g_object_ref(G_OBJECT(audio_signal));
  g_signal_emit(G_OBJECT(audio_signal),
		audio_signal_signals[REALLOC_BUFFER_SIZE], 0,
		buffer_size);
  g_object_unref(G_OBJECT(audio_signal));
}

void
ags_audio_signal_real_morph_samplerate(AgsAudioSignal *audio_signal, guint samplerate, double k_morph)
{
  GList *current, *old, *tmp; 
  guint old_samplerate;
  guint counter;
  guint i_old, i_current;
  guint j_old, j_current;
  double k_old, k_current;
  guint frame;
  double factor, value, preview;

  old = audio_signal->stream_beginning;
  old_samplerate = audio_signal->samplerate;

  current = NULL;
  current = g_list_prepend(current,
			   ags_stream_alloc(audio_signal->buffer_size));
  counter = 0;

  i_old = 0;
  i_current = 0;

  factor = samplerate / old_samplerate;

  if(k_morph == 0.0){
    k_morph = (samplerate < old_samplerate) ? 2.0 * factor: 0.5 * factor;
  }

  while(old != NULL){
    value = (double) (((signed short *) old->data)[i_old]);

    for(j_old = 0, j_current = 0; (((samplerate < old_samplerate) && (j_old < ceil(1.0 / factor))) ||
				   (j_current < ceil(factor))); j_old++, j_current++){
      preview = 0.0;

      for(frame = 0; frame < ceil(factor); frame++){
	//	preview += ;
      }

      for(frame = 0; frame < ceil(factor); frame++){
	//	preview += ;
      }

      for(k_old = 0.0, k_current = 0.0; (((k_morph < 1.0) && (k_old < ceil(1.0 / k_morph))) ||
					 (k_current < ceil(k_morph))); k_old++, k_current++){
	//	value = value * ( / );
      }
    }

    ((signed short *) current->data)[i_current] = value;

    if(i_current == samplerate){
      current = g_list_prepend(current,
			       ags_stream_alloc(audio_signal->buffer_size));

      i_current = 0;

      counter++;
    }

    if(i_old == old_samplerate){
      old = old->next;

      i_old = 0;
    }

    i_current++;
    i_old++;
  }

  /*  */
  audio_signal->length = counter;
  audio_signal->last_frame = i_current;

  audio_signal->stream_end = current;
  audio_signal->stream_beginning = g_list_reverse(current);
}

/**
 * ags_audio_signal_morph_samplerate:
 * @audio_signal
 * @samplerate
 *
 * Morph audio quality to new samplerate.
 */
void
ags_audio_signal_morph_samplerate(AgsAudioSignal *audio_signal, guint samplerate, double k_morph)
{
  g_return_if_fail(AGS_IS_AUDIO_SIGNAL(audio_signal));
  g_object_ref(G_OBJECT(audio_signal));
  g_signal_emit(G_OBJECT(audio_signal),
		audio_signal_signals[MORPH_SAMPLERATE], 0,
		samplerate, k_morph);
  g_object_unref(G_OBJECT(audio_signal));
}

/**
 * ags_audio_signal_copy_buffer_to_buffer:
 * @destination
 * @dchannels
 * @source
 * @schannels
 * @size
 *
 * Copy a buffer to an other buffer.
 */
void
ags_audio_signal_copy_buffer_to_buffer(signed short *destination, guint dchannels, signed short *source, guint schannels, guint size)
{
  guint i;

  for(i = 0; i < size; i++){
    destination[0] = (signed short) ((0xffff) & ((int)destination[0] + (int)source[0]));

    destination += dchannels;
    source += schannels;
  }
}

void
ags_audio_signal_duplicate_stream(AgsAudioSignal *audio_signal,
				  AgsAudioSignal *template,
				  guint attack)
{
  GList *template_stream, *stream, *start;
  signed short *buffer;
  guint size;
  guint k, template_k;

  if(audio_signal->stream_beginning != NULL)
    ags_audio_signal_stream_resize(audio_signal, 0);

  if(template->stream_beginning == NULL){
    audio_signal->stream_beginning = NULL;
    audio_signal->stream_current = NULL;
    audio_signal->stream_end = NULL;
  }else{
    AgsDevout *devout;

    devout = AGS_DEVOUT(audio_signal->devout);

    size = devout->buffer_size * sizeof(signed short);

    ags_audio_signal_stream_resize(audio_signal, template->length);
    
    if(attack + template->last_frame > devout->buffer_size){
      ags_audio_signal_add_stream(audio_signal);
    }

    stream =
      start = audio_signal->stream_beginning;
    template_stream = template->stream_beginning;

    k = attack;
    template_k = 0;

    while(template_stream != NULL){
      if(k == devout->buffer_size){
	k = 0;
	stream = stream->next;
      }

      if(template_k == devout->buffer_size){
	template_k = 0;
	template_stream = template_stream->next;
      }

      for(;
	  template_stream != NULL && k < devout->buffer_size && template_k < devout->buffer_size;
	  k++, template_k++){
	/* copy audio data from template to new AgsAudioSignal */
	((signed short*) stream->data)[k] = ((signed short*) template_stream->data)[template_k];
      }
    }

    // audio_signal->stream_beginning = start;
    // audio_signal->stream_end = stream;

    audio_signal->length = template->length;
    audio_signal->last_frame = template->last_frame;
  }
}

AgsAudioSignal*
ags_audio_signal_get_template(GList *audio_signal)
{
  GList *list;

  list = audio_signal;

  while(list != NULL){
    if((AGS_AUDIO_SIGNAL_TEMPLATE & (AGS_AUDIO_SIGNAL(list->data)->flags)) != 0)
      return((AgsAudioSignal *) list->data);

    list = list->next;
  }

  return(NULL);
}

GList*
ags_audio_signal_get_stream_current(GList *list_audio_signal,
				    GObject *recall_id)
{
  AgsAudioSignal *audio_signal;
  GList *list;

  list = list_audio_signal;

  while(list != NULL){
    audio_signal = AGS_AUDIO_SIGNAL(list->data);

    if((AGS_AUDIO_SIGNAL_TEMPLATE & (audio_signal->flags)) != 0){
      list = list->next;
      continue;
    }

    if(audio_signal->stream_current != NULL && audio_signal->recall_id == recall_id)
      return(list);

    list = list->next;
  }

  return(NULL);
}

GList*
ags_audio_signal_get_by_recall_id(GList *list_audio_signal,
				  GObject *recall_id)
{
  AgsAudioSignal *audio_signal;
  GList *list;

  list = list_audio_signal;

  while(list != NULL){
    audio_signal = AGS_AUDIO_SIGNAL(list->data);

    if((AGS_AUDIO_SIGNAL_TEMPLATE & (audio_signal->flags)) != 0){
      list = list->next;
      continue;
    }

    if(audio_signal->recall_id == recall_id)
      return(list);

    list = list->next;
  }

  return(NULL);
}

void
ags_audio_signal_tile(AgsAudioSignal *audio_signal,
		      AgsAudioSignal *template,
		      guint length)
{
  AgsDevout *devout;
  GList *template_stream, *audio_signal_stream, *audio_signal_stream_end;
  signed short *template_buffer, *audio_signal_buffer;
  guint buffer_size;
  guint template_length, mod_template_length, mod_template_length_odd;
  guint remaining_length;
  gboolean mod_template_even;
  guint i, j, j_offcut, j_offcut_odd, k;

  devout = AGS_DEVOUT(audio_signal->devout);

  audio_signal_stream = NULL;
  template_stream = template->stream_beginning;

  if(template_stream == NULL){
    return;
  }

  buffer_size = devout->buffer_size;

  template_length = template->length;
  mod_template_length = template_length % buffer_size;
  mod_template_length_odd = buffer_size - mod_template_length;
  mod_template_even = TRUE;

  j_offcut = 0;
  k = 1;

  /* write buffers */
  for(i = 0; i < length - buffer_size; i += buffer_size){
    audio_signal_buffer = (signed short *) malloc(buffer_size * sizeof(signed short));
    audio_signal_stream = g_list_prepend(audio_signal_stream,
					 audio_signal_buffer);

    template_buffer = (signed short *) template_stream->data;

    if(template_length < buffer_size){
      ags_audio_signal_copy_buffer_to_buffer(audio_signal_buffer, 1,
					     &(template_buffer[j_offcut]), 1, template_length - j_offcut);

      for(j = template_length - j_offcut; j < buffer_size - template_length; j += template_length){
	ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
					       template_buffer, 1, template_length);
      }

      j_offcut = buffer_size - j;
      ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
					     template_buffer, 1, j_offcut);
    }else{
      ags_audio_signal_copy_buffer_to_buffer(audio_signal_buffer, 1,
					     &(template_buffer[j_offcut]), 1, j_offcut_odd);

      if((guint) floor((double) (i + j_offcut_odd + buffer_size) / (double) template_length) < k){
	template_stream = template_stream->next;
      }else{
	template_stream = template->stream_beginning;
	k++;
      }
      
      ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j_offcut_odd]), 1,
					     template_buffer, 1, j_offcut);

      if(mod_template_even){
	j_offcut = mod_template_length;
	j_offcut_odd = mod_template_length_odd;
	
	mod_template_even = FALSE;
      }else{
	j_offcut = mod_template_length_odd;
	j_offcut_odd = mod_template_length;
	
	mod_template_even = TRUE;
      }
    }
  }
  
  /* write remaining buffer */
  remaining_length = length - (i * buffer_size);

  if(template_length < buffer_size){
    guint remaining_length_odd, remaining_length_overflow;

    if(remaining_length < template_length - j_offcut){
      ags_audio_signal_copy_buffer_to_buffer(audio_signal_buffer, 1,
					     &(template_buffer[j_offcut]), 1, remaining_length);
    }else{
      ags_audio_signal_copy_buffer_to_buffer(audio_signal_buffer, 1,
					     &(template_buffer[j_offcut]), 1, template_length - j_offcut);
    
      if(remaining_length >= template_length){
	for(j = template_length - j_offcut; j < remaining_length - template_length; j += template_length){
	  ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
						 template_buffer, 1, template_length);
	}
	
	j_offcut = remaining_length - j;
	ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
					       template_buffer, 1, j_offcut);
      }else{
	j_offcut = remaining_length + j_offcut - template_length;
	ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
					       template_buffer, 1, j_offcut);
      }
    }
  }else{
    guint remaining_length_odd, remaining_length_overflow;

    remaining_length_overflow = j_offcut + remaining_length;

    if(remaining_length_overflow > buffer_size){
      remaining_length = j_offcut_odd;
      remaining_length_odd = buffer_size - remaining_length_overflow;
    }else{
      remaining_length_odd = 0;
    }

    ags_audio_signal_copy_buffer_to_buffer(audio_signal_buffer, 1,
					   &(template_buffer[j_offcut]), 1, remaining_length);

    if(remaining_length_odd != 0){
      if((i + j_offcut + buffer_size) % template_length < template_length){
	template_stream = template_stream->next;
      }else{
	template_stream = template->stream_beginning;
      }
      
      ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j_offcut_odd]), 1,
					     template_buffer, 1, j_offcut);
    }
  }
  
  /* reverse list */
  audio_signal_stream_end = audio_signal_stream;
  audio_signal_stream = g_list_reverse(audio_signal_stream);

  if(audio_signal->stream_beginning != NULL){
    ags_list_free_and_free_link(audio_signal->stream_beginning);
  }

  audio_signal->stream_beginning = audio_signal_stream;
  //  audio_signal->stream_current = audio_signal_stream;
  audio_signal->stream_end = audio_signal_stream_end;
}

void
ags_audio_signal_scale(AgsAudioSignal *audio_signal,
		       AgsAudioSignal *template,
		       guint length)
{
}

AgsAudioSignal*
ags_audio_signal_new(GObject *devout,
		     GObject *recycling,
		     GObject *recall_id)
{
  AgsAudioSignal *audio_signal;

  audio_signal = (AgsAudioSignal *) g_object_new(AGS_TYPE_AUDIO_SIGNAL,
						 "devout\0", devout,
						 "recycling\0", recycling,
						 "recall-id\0", recall_id,
						 NULL);

  return(audio_signal);
}

AgsAudioSignal*
ags_audio_signal_new_with_length(GObject *devout,
				 GObject *recycling,
				 GObject *recall_id,
				 guint length)
{
  AgsAudioSignal *audio_signal, *template;

  audio_signal = (AgsAudioSignal *) g_object_new(AGS_TYPE_AUDIO_SIGNAL,
						 "devout\0", devout,
						 "recycling\0", recycling,
						 "recall-id\0", recall_id,
						 NULL);

  template = ags_audio_signal_get_template(AGS_RECYCLING(recycling)->audio_signal);

  if(template != NULL){
    ags_audio_signal_tile(audio_signal,
			  template,
			  length);
  }

  return(audio_signal);
}
