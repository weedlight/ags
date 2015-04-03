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

#include <ags/audio/ags_devout.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/main.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <errno.h>

#include <string.h>
#include <math.h>
#include <time.h>

#include <ags/audio/ags_notation.h>

void ags_devout_class_init(AgsDevoutClass *devout);
void ags_devout_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_devout_init(AgsDevout *devout);
void ags_devout_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec);
void ags_devout_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec);
void ags_devout_disconnect(AgsConnectable *connectable);
void ags_devout_connect(AgsConnectable *connectable);
void ags_devout_finalize(GObject *gobject);

void ags_devout_real_change_bpm(AgsDevout *devout, double bpm);

void ags_devout_switch_buffer_flag(AgsDevout *devout);

void ags_devout_play_functions(AgsDevout *devout);

void ags_devout_alsa_init(AgsDevout *devout,
			  GError **error);
void ags_devout_alsa_play(AgsDevout *devout,
			  GError **error);
void ags_devout_alsa_free(AgsDevout *devout);

enum{
  PROP_0,
  PROP_MAIN,
  PROP_DEVICE,
  PROP_DSP_CHANNELS,
  PROP_PCM_CHANNELS,
  PROP_BITS,
  PROP_BUFFER_SIZE,
  PROP_FREQUENCY,
  PROP_BUFFER,
  PROP_BPM,
  PROP_ATTACK,
  PROP_TASK,
};

enum{
  RUN,
  STOP,
  TIC,
  LAST_SIGNAL,
};

static gpointer ags_devout_parent_class = NULL;
static guint devout_signals[LAST_SIGNAL];

/* dangerous - produces a lot of output */
static gboolean DEBUG_DEVOUT = FALSE;

GType
ags_devout_get_type (void)
{
  static GType ags_type_devout = 0;

  if(!ags_type_devout){
    static const GTypeInfo ags_devout_info = {
      sizeof (AgsDevoutClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_devout_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsDevout),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_devout_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_devout_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_devout = g_type_register_static(G_TYPE_OBJECT,
					     "AgsDevout\0",
					     &ags_devout_info,
					     0);

    g_type_add_interface_static(ags_type_devout,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_devout);
}

void
ags_devout_class_init(AgsDevoutClass *devout)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_devout_parent_class = g_type_class_peek_parent(devout);

  /* GObjectClass */
  gobject = (GObjectClass *) devout;

  gobject->set_property = ags_devout_set_property;
  gobject->get_property = ags_devout_get_property;

  gobject->finalize = ags_devout_finalize;

  /* properties */
  param_spec = g_param_spec_object("main\0",
				   "the main object\0",
				   "The main object\0",
				   AGS_TYPE_MAIN,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_MAIN,
				  param_spec);

  param_spec = g_param_spec_string("device\0",
				   "the device identifier\0",
				   "The device to perform output to\0",
				   "hw:0\0",
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DEVICE,
				  param_spec);
  
  param_spec = g_param_spec_uint("dsp_channels\0",
				 "count of DSP channels\0",
				 "The count of DSP channels to use\0",
				 1,
				 64,
				 2,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DSP_CHANNELS,
				  param_spec);

  param_spec = g_param_spec_uint("pcm_channels\0",
				 "count of PCM channels\0",
				 "The count of PCM channels to use\0",
				 1,
				 64,
				 2,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PCM_CHANNELS,
				  param_spec);

  /*
   * TODO:JK: add support for other quality than 16 bit
   */
  param_spec = g_param_spec_uint("bits\0",
				 "precision of buffer\0",
				 "The precision to use for a frame\0",
				 1,
				 64,
				 16,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_BITS,
				  param_spec);

  param_spec = g_param_spec_uint("buffer_size\0",
				 "frame count of a buffer\0",
				 "The count of frames a buffer contains\0",
				 1,
				 44100,
				 940,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_BUFFER_SIZE,
				  param_spec);

  param_spec = g_param_spec_uint("frequency\0",
				 "frames per second\0",
				 "The frames count played during a second\0",
				 8000,
				 96000,
				 44100,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FREQUENCY,
				  param_spec);

  param_spec = g_param_spec_pointer("buffer\0",
				    "the buffer\0",
				    "The buffer to play\0",
				    G_PARAM_READABLE);
  g_object_class_install_property(gobject,
				  PROP_BUFFER,
				  param_spec);

  param_spec = g_param_spec_double("bpm\0",
				   "beats per minute\0",
				   "Beats per minute to use\0",
				   1.0,
				   240.0,
				   120.0,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_BPM,
				  param_spec);

  param_spec = g_param_spec_pointer("attack\0",
				    "attack of buffer\0",
				    "The attack to use for the buffer\0",
				    G_PARAM_READABLE);
  g_object_class_install_property(gobject,
				  PROP_ATTACK,
				  param_spec);

  param_spec = g_param_spec_object("task\0",
				   "task to launch\0",
				   "A task to launch\0",
				   AGS_TYPE_TASK,
				   G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_TASK,
				  param_spec);

  /* AgsDevoutClass */
  devout->play_init = ags_devout_alsa_init;
  devout->play = ags_devout_alsa_play;
  devout->stop = ags_devout_alsa_free;

  devout->tic = NULL;
  devout->note_offset_changed = NULL;

  devout_signals[TIC] =
    g_signal_new("tic\0",
		 G_TYPE_FROM_CLASS (devout),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsDevoutClass, tic),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);
}

GQuark
ags_devout_error_quark()
{
  return(g_quark_from_static_string("ags-devout-error-quark\0"));
}

void
ags_devout_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_devout_connect;
  connectable->disconnect = ags_devout_disconnect;
}

void
ags_devout_init(AgsDevout *devout)
{
  guint default_tact_frames;
  guint default_tic_frames;
  guint i;
  
  /* flags */
  devout->flags = (AGS_DEVOUT_ALSA);

  /* quality */
  devout->dsp_channels = 2;
  devout->pcm_channels = 2;
  devout->bits = 16;
  devout->buffer_size = AGS_DEVOUT_DEFAULT_BUFFER_SIZE;
  devout->frequency = AGS_DEVOUT_DEFAULT_SAMPLERATE;

  //  devout->out.oss.device = NULL;
  devout->out.alsa.handle = NULL;
  devout->out.alsa.device = g_strdup("hw:0\0");

  /* buffer */
  devout->buffer = (signed short **) malloc(4 * sizeof(signed short*));
  devout->buffer[0] = (signed short *) malloc(devout->dsp_channels * devout->buffer_size * sizeof(signed short));
  devout->buffer[1] = (signed short *) malloc(devout->dsp_channels * devout->buffer_size * sizeof(signed short));
  devout->buffer[2] = (signed short *) malloc(devout->dsp_channels * devout->buffer_size * sizeof(signed short));
  devout->buffer[3] = (signed short *) malloc(devout->dsp_channels * devout->buffer_size * sizeof(signed short));

  /* bpm */
  devout->bpm = AGS_DEVOUT_DEFAULT_BPM;

  /* delay and attack */
  devout->delay = (guint *) malloc((int) ceil(2.0 * AGS_NOTATION_TICS_PER_BEAT) *
				   sizeof(gdouble));

  devout->attack = (guint *) malloc((int) ceil(2.0 * AGS_NOTATION_TICS_PER_BEAT) *
				   sizeof(guint));

  default_tact_frames = (guint) (AGS_DEVOUT_DEFAULT_DELAY * AGS_DEVOUT_DEFAULT_BUFFER_SIZE);
  default_tic_frames = (guint) (default_tact_frames * AGS_NOTATION_MINIMUM_NOTE_LENGTH);

  memset(devout->delay, 0, (int) (ceil(2.0 * AGS_NOTATION_TICS_PER_BEAT) * sizeof(guint)));
  memset(devout->delay, 0, (int) (ceil(2.0 * AGS_NOTATION_TICS_PER_BEAT) * sizeof(guint)));

  for(i = 0; i < (int) ceil(2.0 * AGS_NOTATION_TICS_PER_BEAT); i++){
    devout->attack[i] = (i * default_tic_frames) % AGS_DEVOUT_DEFAULT_BUFFER_SIZE;
  }

  for(i = 0; i < (int) ceil(2.0 * AGS_NOTATION_TICS_PER_BEAT); i++){
    //    devout->delay[i] = AGS_DEVOUT_DEFAULT_BUFFER_SIZE / (default_tic_frames) / (AGS_DEVOUT_DEFAULT_SAMPLERATE / AGS_NOTATION_DEFAULT_JIFFIE);
  }

  /*  */
  devout->delay_counter = 0;
  devout->tic_counter = 0;

  /* parent */
  devout->ags_main = NULL;

  /* all AgsAudio */
  devout->audio = NULL;
}

void
ags_devout_set_property(GObject *gobject,
			guint prop_id,
			const GValue *value,
			GParamSpec *param_spec)
{
  AgsDevout *devout;

  devout = AGS_DEVOUT(gobject);

  //TODO:JK: implement set functionality
  
  switch(prop_id){
  case PROP_MAIN:
    {
      AgsMain *ags_main;

      ags_main = g_value_get_object(value);

      if(devout->ags_main == ags_main){
	return;
      }

      if(devout->ags_main != NULL){
	g_object_unref(G_OBJECT(devout->ags_main));
      }

      if(ags_main != NULL){
	g_object_ref(G_OBJECT(ags_main));
      }

      devout->ags_main = ags_main;
    }
    break;
  case PROP_DEVICE:
    {
      char *device;

      device = (char *) g_value_get_string(value);

      if((AGS_DEVOUT_LIBAO & (devout->flags)) != 0){
	//TODO:JK: implement me
      }else if((AGS_DEVOUT_OSS & (devout->flags)) != 0){
	devout->out.oss.device = device;
      }else if((AGS_DEVOUT_ALSA & (devout->flags)) != 0){
	devout->out.alsa.device = device;
      }
    }
    break;
  case PROP_DSP_CHANNELS:
    {
      guint dsp_channels;

      dsp_channels = g_value_get_uint(value);

      if(dsp_channels == devout->dsp_channels){
	return;
      }

      devout->dsp_channels = dsp_channels;
    }
    break;
  case PROP_PCM_CHANNELS:
    {
      guint pcm_channels;

      pcm_channels = g_value_get_uint(value);

      if(pcm_channels == devout->pcm_channels){
	return;
      }

      devout->pcm_channels = pcm_channels;

      free(devout->buffer[0]);
      free(devout->buffer[1]);
      free(devout->buffer[2]);
      free(devout->buffer[3]);

      devout->buffer[0] = (signed short *) malloc((pcm_channels * devout->buffer_size) * sizeof(signed short));
      devout->buffer[1] = (signed short *) malloc((pcm_channels * devout->buffer_size) * sizeof(signed short));
      devout->buffer[2] = (signed short *) malloc((pcm_channels * devout->buffer_size) * sizeof(signed short));
      devout->buffer[3] = (signed short *) malloc((pcm_channels * devout->buffer_size) * sizeof(signed short));
    }
    break;
  case PROP_BITS:
    {
	//TODO:JK: implement me
    }
    break;
  case PROP_BUFFER_SIZE:
    {
      guint buffer_size;

      buffer_size = g_value_get_uint(value);

      if(buffer_size == devout->buffer_size){
	return;
      }

      devout->buffer_size = buffer_size;

      free(devout->buffer[0]);
      free(devout->buffer[1]);
      free(devout->buffer[2]);
      free(devout->buffer[3]);

      devout->buffer[0] = (signed short *) malloc((devout->pcm_channels * buffer_size) * sizeof(signed short));
      devout->buffer[1] = (signed short *) malloc((devout->pcm_channels * buffer_size) * sizeof(signed short));
      devout->buffer[2] = (signed short *) malloc((devout->pcm_channels * buffer_size) * sizeof(signed short));
      devout->buffer[3] = (signed short *) malloc((devout->pcm_channels * buffer_size) * sizeof(signed short));
    }
    break;
  case PROP_FREQUENCY:
    {
      guint frequency;

      frequency = g_value_get_uint(value);

      if(frequency == devout->frequency){
	return;
      }

      devout->frequency = frequency;
    }
    break;
  case PROP_BUFFER:
    {
	//TODO:JK: implement me
    }
    break;
  case PROP_BPM:
    {
	//TODO:JK: implement me
    }
    break;
  case PROP_TASK:
    {
	//TODO:JK: implement me
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_devout_get_property(GObject *gobject,
			guint prop_id,
			GValue *value,
			GParamSpec *param_spec)
{
  AgsDevout *devout;

  devout = AGS_DEVOUT(gobject);
  
  switch(prop_id){
  case PROP_DEVICE:
    {
      if((AGS_DEVOUT_LIBAO & (devout->flags)) != 0){
	g_value_set_string(value, ao_driver_info(devout->out.ao.driver_ao)->name);
      }else if((AGS_DEVOUT_OSS & (devout->flags)) != 0){
	g_value_set_string(value, devout->out.oss.device);
      }else if((AGS_DEVOUT_ALSA & (devout->flags)) != 0){
	g_value_set_string(value, devout->out.alsa.device);
      }
    }
    break;
  case PROP_DSP_CHANNELS:
    {
      g_value_set_uint(value, devout->dsp_channels);
    }
    break;
  case PROP_PCM_CHANNELS:
    {
      g_value_set_uint(value, devout->pcm_channels);
    }
    break;
  case PROP_BITS:
    {
      g_value_set_uint(value, devout->bits);
    }
    break;
  case PROP_BUFFER_SIZE:
    {
      g_value_set_uint(value, devout->buffer_size);
    }
    break;
  case PROP_FREQUENCY:
    {
      g_value_set_uint(value, devout->frequency);
    }
    break;
  case PROP_BUFFER:
    {
      g_value_set_pointer(value, devout->buffer);
    }
    break;
  case PROP_BPM:
    {
      g_value_set_double(value, devout->bpm);
    }
    break;
  case PROP_ATTACK:
    {
      g_value_set_pointer(value, devout->attack);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_devout_finalize(GObject *gobject)
{
  AgsDevout *devout;
  GList *list, *list_next;

  devout = AGS_DEVOUT(gobject);

  /* free output buffer */
  free(devout->buffer[0]);
  free(devout->buffer[1]);
  free(devout->buffer[2]);
  free(devout->buffer[3]);

  /* free buffer array */
  free(devout->buffer);

  /* free AgsAttack */
  free(devout->attack);

  /* call parent */
  G_OBJECT_CLASS(ags_devout_parent_class)->finalize(gobject);
}

void
ags_devout_connect(AgsConnectable *connectable)
{
  AgsDevout *devout;
  GList *list;

  devout = AGS_DEVOUT(connectable);
  
  list = devout->audio;

  while(list != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(list->data));

    list = list->next;
  }
}

void
ags_devout_disconnect(AgsConnectable *connectable)
{
  //TODO:JK: implement me
}

AgsDevoutPlayDomain*
ags_devout_play_domain_alloc()
{
  AgsDevoutPlayDomain *devout_play_domain;

  devout_play_domain = (AgsDevoutPlayDomain *) malloc(sizeof(AgsDevoutPlayDomain));

  devout_play_domain->domain = NULL;

  devout_play_domain->playback = FALSE;
  devout_play_domain->sequencer = FALSE;
  devout_play_domain->notation = FALSE;

  devout_play_domain->devout_play = NULL;

  return(devout_play_domain);
}

void
ags_devout_play_domain_free(AgsDevoutPlayDomain *devout_play_domain)
{
  g_list_free(devout_play_domain->devout_play);

  free(devout_play_domain);
}

AgsDevoutPlay*
ags_devout_play_alloc()
{
  AgsDevoutPlay *play;

  play = (AgsDevoutPlay *) malloc(sizeof(AgsDevoutPlay));

  play->flags = 0;

  play->iterator_thread = (AgsIteratorThread **) malloc(3 * sizeof(AgsIteratorThread *));

  play->iterator_thread[0] = ags_iterator_thread_new();
  play->iterator_thread[1] = ags_iterator_thread_new();
  play->iterator_thread[2] = ags_iterator_thread_new();

  play->source = NULL;
  play->audio_channel = 0;

  play->recall_id = (AgsDevoutPlay **) malloc(3 * sizeof(AgsDevoutPlay *));

  play->recall_id[0] = NULL;
  play->recall_id[1] = NULL;
  play->recall_id[2] = NULL;

  return(play);
}

void
ags_devout_play_free(AgsDevoutPlay *play)
{
  g_object_unref(G_OBJECT(play->iterator_thread[0]));
  g_object_unref(G_OBJECT(play->iterator_thread[1]));
  g_object_unref(G_OBJECT(play->iterator_thread[2]));

  free(play->iterator_thread);
}

void
ags_devout_list_cards(GList **card_id, GList **card_name)
{
  snd_ctl_t *card_handle;
  snd_ctl_card_info_t *card_info;
  char *name;
  gchar *str;
  int card_num;
  int error;

  *card_id = NULL;
  *card_name = NULL;
  card_num = -1;

  while(TRUE){
    error = snd_card_next(&card_num);

    if(card_num < 0){
      break;
    }

    if(error < 0){
      continue;
    }

    str = g_strdup_printf("hw:%i\0", card_num);
    error = snd_ctl_open(&card_handle, str, 0);

    if(error < 0){
      continue;
    }

    snd_ctl_card_info_alloca(&card_info);
    error = snd_ctl_card_info(card_handle, card_info);

    if(error < 0){
      continue;
    }

    *card_id = g_list_prepend(*card_id, str);
    *card_name = g_list_prepend(*card_name, g_strdup(snd_ctl_card_info_get_name(card_info)));

    snd_ctl_close(card_handle);
  }

  snd_config_update_free_global();

  *card_id = g_list_reverse(*card_id);
  *card_name = g_list_reverse(*card_name);
}

void
ags_devout_pcm_info(char *card_id,
		    guint *channels_min, guint *channels_max,
		    guint *rate_min, guint *rate_max,
		    guint *buffer_size_min, guint *buffer_size_max,
		    GError **error)
{
  int rc;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  int err;

  /* Open PCM device for playback. */
  handle = NULL;

  rc = snd_pcm_open(&handle, card_id, SND_PCM_STREAM_PLAYBACK, 0);

  if(rc < 0) {
    g_message("unable to open pcm device: %s\n\0", snd_strerror(rc));

    g_set_error(error,
		AGS_DEVOUT_ERROR,
		AGS_DEVOUT_ERROR_LOCKED_SOUNDCARD,
		"unable to open pcm device: %s\n\0",
		snd_strerror(rc));

    return;
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* channels */
  snd_pcm_hw_params_get_channels_min(params, &val);
  *channels_min = val;

  snd_pcm_hw_params_get_channels_max(params, &val);
  *channels_max = val;

  /* samplerate */
  dir = 0;
  snd_pcm_hw_params_get_rate_min(params, &val, &dir);
  *rate_min = val;

  dir = 0;
  snd_pcm_hw_params_get_rate_max(params, &val, &dir);
  *rate_max = val;

  /* buffer size */
  dir = 0;
  snd_pcm_hw_params_get_buffer_size_min(params, &frames);
  *buffer_size_min = frames;

  dir = 0;
  snd_pcm_hw_params_get_buffer_size_max(params, &frames);
  *buffer_size_max = frames;

  snd_pcm_close(handle);
}

void
ags_devout_add_audio(AgsDevout *devout, GObject *audio)
{
  g_object_ref(G_OBJECT(audio));
  devout->audio = g_list_prepend(devout->audio,
				 audio);
}

void
ags_devout_remove_audio(AgsDevout *devout, GObject *audio)
{
  devout->audio = g_list_remove(devout->audio,
				audio);
  g_object_unref(G_OBJECT(audio));
}

void
ags_devout_tic(AgsDevout *devout)
{
  g_return_if_fail(AGS_IS_DEVOUT(devout));

  if((AGS_DEVOUT_PLAY & devout->flags) == 0){
    g_message("ags_devout_tic: not playing\0");
    return;
  }

  g_object_ref((GObject *) devout);
  g_signal_emit(G_OBJECT(devout),
		devout_signals[TIC], 0);
  g_object_unref((GObject *) devout);
}

/**
 * ags_devout_switch_buffer_flag:
 * @devout an #AgsDevout
 *
 * The buffer flag indicates the currently played buffer.
 */
void
ags_devout_switch_buffer_flag(AgsDevout *devout)
{
  if((AGS_DEVOUT_BUFFER0 & (devout->flags)) != 0){
    devout->flags &= (~AGS_DEVOUT_BUFFER0);
    devout->flags |= AGS_DEVOUT_BUFFER1;
  }else if((AGS_DEVOUT_BUFFER1 & (devout->flags)) != 0){
    devout->flags &= (~AGS_DEVOUT_BUFFER1);
    devout->flags |= AGS_DEVOUT_BUFFER2;
  }else if((AGS_DEVOUT_BUFFER2 & (devout->flags)) != 0){
    devout->flags &= (~AGS_DEVOUT_BUFFER2);
    devout->flags |= AGS_DEVOUT_BUFFER3;
  }else if((AGS_DEVOUT_BUFFER3 & (devout->flags)) != 0){
    devout->flags &= (~AGS_DEVOUT_BUFFER3);
    devout->flags |= AGS_DEVOUT_BUFFER0;
  }
}

void
ags_devout_alsa_init(AgsDevout *devout,
		     GError **error)
{
  static unsigned int period_time = 100000;
  static snd_pcm_format_t format = SND_PCM_FORMAT_S16;

  int rc;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *hwparams;
  unsigned int val;
  snd_pcm_uframes_t frames;
  unsigned int rate;
  unsigned int rrate;
  unsigned int channels;
  snd_pcm_uframes_t size;
  snd_pcm_sframes_t buffer_size;
  snd_pcm_sframes_t period_size;
  snd_pcm_sw_params_t *swparams;
  int period_event = 0;
  int err, dir;

  /* Open PCM device for playback. */
  if ((err = snd_pcm_open(&handle, devout->out.alsa.device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    printf("Playback open error: %s\n", snd_strerror(err));
    return;
  }

  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);

  /* choose all parameters */
  err = snd_pcm_hw_params_any(handle, hwparams);
  if (err < 0) {
    printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
    return;
  }

  /* set hardware resampling */
  err = snd_pcm_hw_params_set_rate_resample(handle, hwparams, 1);
  if (err < 0) {
    printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
    return;
  }

  /* set the interleaved read/write format */
  err = snd_pcm_hw_params_set_access(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
  if (err < 0) {
    printf("Access type not available for playback: %s\n", snd_strerror(err));
    return;
  }

  /* set the sample format */
  err = snd_pcm_hw_params_set_format(handle, hwparams, format);
  if (err < 0) {
    printf("Sample format not available for playback: %s\n", snd_strerror(err));
    return;
  }

  /* set the count of channels */
  channels = devout->dsp_channels;
  err = snd_pcm_hw_params_set_channels(handle, hwparams, channels);
  if (err < 0) {
    printf("Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
    return;
  }

  /* set the stream rate */
  rate = devout->frequency;
  rrate = rate;
  err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &rrate, 0);
  if (err < 0) {
    printf("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
    return;
  }

  if (rrate != rate) {
    printf("Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
    exit(-EINVAL);
  }

  /* set the buffer size */
  size = devout->buffer_size;
  err = snd_pcm_hw_params_set_buffer_size(handle, hwparams, size);
  if (err < 0) {
    printf("Unable to set buffer size %i for playback: %s\n", size, snd_strerror(err));
    return;
  }

  buffer_size = size;

  /* set the period time */
  err = snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, &dir);
  if (err < 0) {
    printf("Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
    return;
  }

  err = snd_pcm_hw_params_get_period_size(hwparams, &size, &dir);
  if (err < 0) {
    printf("Unable to get period size for playback: %s\n", snd_strerror(err));
    return;
  }
  period_size = size;

  /* write the parameters to device */
  err = snd_pcm_hw_params(handle, hwparams);
  if (err < 0) {
    printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
    return;
  }

  /* get the current swparams */
  err = snd_pcm_sw_params_current(handle, swparams);
  if (err < 0) {
    printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
    return;
  }

  /* start the transfer when the buffer is almost full: */
  /* (buffer_size / avail_min) * avail_min */
  err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
  if (err < 0) {
    printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
    return;
  }

  /* allow the transfer when at least period_size samples can be processed */
  /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
  err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_event ? buffer_size : period_size);
  if (err < 0) {
    printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
    return;
  }

  /* write the parameters to the playback device */
  err = snd_pcm_sw_params(handle, swparams);
  if (err < 0) {
    printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
    return;
  }

  /*  */
  devout->out.alsa.handle = handle;
  devout->delay_counter = 0;
}

void
ags_devout_alsa_play(AgsDevout *devout,
		     GError **error)
{    
  /*  */
  if((AGS_DEVOUT_BUFFER0 & (devout->flags)) != 0){
    memset(devout->buffer[3], 0, (size_t) devout->dsp_channels * devout->buffer_size * sizeof(signed short));
      
    devout->out.alsa.rc = snd_pcm_writei(devout->out.alsa.handle,
					 devout->buffer[0],
					 (snd_pcm_uframes_t) (devout->buffer_size));

    if((AGS_DEVOUT_NONBLOCKING & (devout->flags)) == 0){
      if(devout->out.alsa.rc == -EPIPE){
	/* EPIPE means underrun */
	snd_pcm_prepare(devout->out.alsa.handle);

#ifdef AGS_DEBUG
	g_message("underrun occurred\0");
#endif
      }else if(devout->out.alsa.rc == -ESTRPIPE){
	static const struct timespec idle = {
	  0,
	  4000,
	};

	int err;

	while((err = snd_pcm_resume(devout->out.alsa.handle)) == -EAGAIN)
	  nanosleep(&idle, NULL); /* wait until the suspend flag is released */
	if(err < 0){
	  err = snd_pcm_prepare(devout->out.alsa.handle);
	}
      }else if(devout->out.alsa.rc < 0){
	g_message("error from writei: %s\0", snd_strerror(devout->out.alsa.rc));
      }else if(devout->out.alsa.rc != (int) devout->buffer_size) {
	g_message("short write, write %d frames\0", devout->out.alsa.rc);
      }
    }      
    //      g_message("ags_devout_play 0\0");
  }else if((AGS_DEVOUT_BUFFER1 & (devout->flags)) != 0){
    memset(devout->buffer[0], 0, (size_t) devout->dsp_channels * devout->buffer_size * sizeof(signed short));

    devout->out.alsa.rc = snd_pcm_writei(devout->out.alsa.handle,
					 devout->buffer[1],
					 (snd_pcm_uframes_t) (devout->buffer_size));
     
    if((AGS_DEVOUT_NONBLOCKING & (devout->flags)) == 0){
      if(devout->out.alsa.rc == -EPIPE){
	/* EPIPE means underrun */
	snd_pcm_prepare(devout->out.alsa.handle);

#ifdef AGS_DEBUG
	g_message("underrun occurred\0");
#endif
      }else if(devout->out.alsa.rc == -ESTRPIPE){
	static const struct timespec idle = {
	  0,
	  4000,
	};

	int err;	

	while((err = snd_pcm_resume(devout->out.alsa.handle)) == -EAGAIN)
	  nanosleep(&idle, NULL); /* wait until the suspend flag is released */
	if(err < 0){
	  err = snd_pcm_prepare(devout->out.alsa.handle);
	}
      }else if(devout->out.alsa.rc < 0){
	g_message("error from writei: %s\0", snd_strerror(devout->out.alsa.rc));
      }else if(devout->out.alsa.rc != (int) devout->buffer_size) {
	g_message("short write, write %d frames\0", devout->out.alsa.rc);
      }
    }      
    //      g_message("ags_devout_play 1\0");
  }else if((AGS_DEVOUT_BUFFER2 & (devout->flags)) != 0){
    memset(devout->buffer[1], 0, (size_t) devout->dsp_channels * devout->buffer_size * sizeof(signed short));
      
    devout->out.alsa.rc = snd_pcm_writei(devout->out.alsa.handle,
					 devout->buffer[2],
					 (snd_pcm_uframes_t) (devout->buffer_size));

    if((AGS_DEVOUT_NONBLOCKING & (devout->flags)) == 0){
      if(devout->out.alsa.rc == -EPIPE){
	/* EPIPE means underrun */
	snd_pcm_prepare(devout->out.alsa.handle);

#ifdef AGS_DEBUG
	g_message("underrun occurred\0");
#endif
      }else if(devout->out.alsa.rc == -ESTRPIPE){
	static const struct timespec idle = {
	  0,
	  4000,
	};

	int err;

	while((err = snd_pcm_resume(devout->out.alsa.handle)) == -EAGAIN)
	  nanosleep(&idle, NULL); /* wait until the suspend flag is released */
	if(err < 0){
	  err = snd_pcm_prepare(devout->out.alsa.handle);
	}
      }else if(devout->out.alsa.rc < 0){
	g_message("error from writei: %s\0", snd_strerror(devout->out.alsa.rc));
      }else if(devout->out.alsa.rc != (int) devout->buffer_size) {
	g_message("short write, write %d frames\0", devout->out.alsa.rc);
      }
    }
    //      g_message("ags_devout_play 2\0");
  }else if((AGS_DEVOUT_BUFFER3 & devout->flags) != 0){
    memset(devout->buffer[2], 0, (size_t) devout->dsp_channels * devout->buffer_size * sizeof(signed short));
      
    devout->out.alsa.rc = snd_pcm_writei(devout->out.alsa.handle,
					 devout->buffer[3],
					 (snd_pcm_uframes_t) (devout->buffer_size));

    if((AGS_DEVOUT_NONBLOCKING & (devout->flags)) == 0){
      if(devout->out.alsa.rc == -EPIPE){
	snd_pcm_prepare(devout->out.alsa.handle);

#ifdef AGS_DEBUG
	g_message("underrun occurred\0");
#endif
      }else if(devout->out.alsa.rc == -ESTRPIPE){
	static const struct timespec idle = {
	  0,
	  4000,
	};

	int err;

	while((err = snd_pcm_resume(devout->out.alsa.handle)) == -EAGAIN)
	  nanosleep(&idle, NULL); /* wait until the suspend flag is released */
	if(err < 0){
	  err = snd_pcm_prepare(devout->out.alsa.handle);
	}
      }else if(devout->out.alsa.rc < 0){
	g_message("error from writei: %s\0", snd_strerror(devout->out.alsa.rc));
      }else if(devout->out.alsa.rc != (int) devout->buffer_size) {
	g_message("short write, write %d frames\0", devout->out.alsa.rc);
      }
    }
    //      g_message("ags_devout_play 3\0");
  }

  /* determine if attack should be switched */
  devout->delay_counter += 1; //AGS_DEVOUT_DEFAULT_JIFFIE

  ///TODO:JK: fix me
  if(devout->delay_counter >= AGS_DEVOUT_DEFAULT_DELAY){ //devout->delay[devout->tic_counter]
    /* tic */
    ags_devout_tic(devout);

    devout->tic_counter += 1;

    if(devout->tic_counter == AGS_NOTATION_TICS_PER_BEAT){
      devout->tic_counter = 0;
    }

    /* delay */
    devout->delay_counter = 0;
  } 

  /* switch buffer flags */
  ags_devout_switch_buffer_flag(devout);

  snd_pcm_prepare(devout->out.alsa.handle);
}

void
ags_devout_alsa_free(AgsDevout *devout)
{
  snd_pcm_drain(devout->out.alsa.handle);
  snd_pcm_close(devout->out.alsa.handle);
  devout->out.alsa.handle = NULL;
} 

AgsDevout*
ags_devout_new(GObject *ags_main)
{
  AgsDevout *devout;

  devout = (AgsDevout *) g_object_new(AGS_TYPE_DEVOUT, NULL);
  
  if(ags_main != NULL){
    g_object_ref(G_OBJECT(ags_main));
    devout->ags_main = ags_main;
  }

  return(devout);
}
