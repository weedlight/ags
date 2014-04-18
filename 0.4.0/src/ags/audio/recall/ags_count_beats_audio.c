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

#include <ags/audio/recall/ags_count_beats_audio.h>
#include <ags/audio/recall/ags_count_beats_audio_run.h>

#include <ags/main.h>

#include <ags/object/ags_plugin.h>

void ags_count_beats_audio_class_init(AgsCountBeatsAudioClass *count_beats_audio);
void ags_count_beats_audio_init(AgsCountBeatsAudio *count_beats_audio);
void ags_count_beats_audio_plugin_interface_init(AgsPluginInterface *plugin);
void ags_count_beats_audio_set_property(GObject *gobject,
					guint prop_id,
					const GValue *value,
					GParamSpec *param_spec);
void ags_count_beats_audio_get_property(GObject *gobject,
					guint prop_id,
					GValue *value,
					GParamSpec *param_spec);
void ags_count_beats_audio_set_ports(AgsPlugin *plugin, GList *port);
void ags_count_beats_audio_finalize(GObject *gobject);

enum{
  PROP_0,
  PROP_LOOP,
  PROP_NOTATION_LOOP_START,
  PROP_NOTATION_LOOP_END,
  PROP_SEQUENCER_LOOP_START,
  PROP_SEQUENCER_LOOP_END,
};

static gpointer ags_count_beats_audio_parent_class = NULL;

static const gchar *ags_count_beats_audio_plugin_name = "ags-count-beats\0";
static const gchar *ags_count_beats_audio_specifier[] = {
  "./loop[0]\0",
  "./sequencer_loop_start[0]\0",
  "./notation_loop_start[0]\0",
  "./sequencer_loop_end[0]\0",
  "./notation_loop_end[0]\0"
};
static const gchar *ags_count_beats_audio_control_port[] = {
  "1/5\0",
  "2/5\0",
  "3/5\0",
  "4/5\0",
  "5/5\0",
};

GType
ags_count_beats_audio_get_type()
{
  static GType ags_type_count_beats_audio = 0;

  if(!ags_type_count_beats_audio){
    static const GTypeInfo ags_count_beats_audio_info = {
      sizeof (AgsCountBeatsAudioClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_count_beats_audio_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCountBeatsAudio),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_count_beats_audio_init,
    };
    
    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_count_beats_audio_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_count_beats_audio = g_type_register_static(AGS_TYPE_RECALL_AUDIO,
							"AgsCountBeatsAudio\0",
							&ags_count_beats_audio_info,
							0);

    g_type_add_interface_static(ags_type_count_beats_audio,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return(ags_type_count_beats_audio);
}

void
ags_count_beats_audio_plugin_interface_init(AgsPluginInterface *plugin)
{
  plugin->set_ports = ags_count_beats_audio_set_ports;
}

void
ags_count_beats_audio_class_init(AgsCountBeatsAudioClass *count_beats_audio)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_count_beats_audio_parent_class = g_type_class_peek_parent(count_beats_audio);

  gobject = (GObjectClass *) count_beats_audio;

  gobject->set_property = ags_count_beats_audio_set_property;
  gobject->get_property = ags_count_beats_audio_get_property;

  gobject->finalize = ags_count_beats_audio_finalize;

  /* properties */
  param_spec = g_param_spec_object("loop\0",
				   "loop playing\0",
				   "Play in a endless loop\0",
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_LOOP,
				  param_spec);

  param_spec = g_param_spec_object("notation_loop_start\0",
				   "start beat of notation loop\0",
				   "The start beat of the notation loop\0",
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_NOTATION_LOOP_START,
				  param_spec);

  param_spec = g_param_spec_object("notation_loop_end\0",
				   "end beat of notation loop\0",
				   "The end beat of the notation loop\0",
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_NOTATION_LOOP_END,
				  param_spec);
  
  param_spec = g_param_spec_object("sequencer_loop_start\0",
				   "start beat of loop\0",
				   "The start beat of the sequencer loop\0",
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SEQUENCER_LOOP_START,
				  param_spec);

  param_spec = g_param_spec_object("sequencer_loop_end\0",
				   "end beat of sequencer loop\0",
				   "The end beat of the sequencer loop\0",
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SEQUENCER_LOOP_END,
				  param_spec);
}

void
ags_count_beats_audio_init(AgsCountBeatsAudio *count_beats_audio)
{
  GList *port;

  AGS_RECALL(count_beats_audio)->name = "ags-count-beats\0";
  AGS_RECALL(count_beats_audio)->version = AGS_EFFECTS_DEFAULT_VERSION;
  AGS_RECALL(count_beats_audio)->build_id = AGS_BUILD_ID;
  AGS_RECALL(count_beats_audio)->xml_type = "ags-count-beats-audio\0";

  port = NULL;

  /* loop */
  count_beats_audio->loop = g_object_new(AGS_TYPE_PORT,
					 "plugin-name\0", ags_count_beats_audio_plugin_name,
					 "specifier\0", ags_count_beats_audio_specifier[0],
					 "control-port\0", ags_count_beats_audio_control_port[0],
					 "port-value-is-pointer\0", FALSE,
					 "port-value-type\0", G_TYPE_BOOLEAN,
					 "port-value-size\0", sizeof(gboolean),
					 "port-value-length", 1,
					 NULL);

  count_beats_audio->loop->port_value.ags_port_boolean = FALSE;

  port = g_list_prepend(port, count_beats_audio->loop);

  /* notation-loop-start */
  count_beats_audio->notation_loop_start = g_object_new(AGS_TYPE_PORT,
							"plugin-name\0", ags_count_beats_audio_plugin_name,
							"specifier\0", ags_count_beats_audio_specifier[1],
							"control-port\0", ags_count_beats_audio_control_port[1],
							"port-value-is-pointer\0", FALSE,
							"port-value-type\0", G_TYPE_DOUBLE,
							"port-value-size\0", sizeof(gdouble),
							"port-value-length", 1,
							NULL);

  count_beats_audio->notation_loop_start->port_value.ags_port_double = 0.0;

  port = g_list_prepend(port, count_beats_audio->notation_loop_start);

  /* notation-loop-end */
  count_beats_audio->notation_loop_end = g_object_new(AGS_TYPE_PORT,
						      "plugin-name\0", ags_count_beats_audio_plugin_name,
						      "specifier\0", ags_count_beats_audio_specifier[2],
						      "control-port\0", ags_count_beats_audio_control_port[2],
						      "port-value-is-pointer\0", FALSE,
						      "port-value-type\0", G_TYPE_DOUBLE,
						      "port-value-size\0", sizeof(gdouble),
						      "port-value-length", 1,
						      NULL);

  count_beats_audio->notation_loop_end->port_value.ags_port_double = 64.0;

  port = g_list_prepend(port, count_beats_audio->notation_loop_end);

  /* sequencer-loop-start  */
  count_beats_audio->sequencer_loop_start = g_object_new(AGS_TYPE_PORT,
							 "plugin-name\0", ags_count_beats_audio_plugin_name,
							 "specifier\0", ags_count_beats_audio_specifier[3],
							 "control-port\0", ags_count_beats_audio_control_port[3],
							 "port-value-is-pointer\0", FALSE,
							 "port-value-type\0", G_TYPE_DOUBLE,
							 "port-value-size\0", sizeof(gdouble),
							 "port-value-length", 1,
							 NULL);

  count_beats_audio->sequencer_loop_start->port_value.ags_port_double = 0.0;

  port = g_list_prepend(port, count_beats_audio->sequencer_loop_start);

  /* sequencer-loop-end */
  count_beats_audio->sequencer_loop_end = g_object_new(AGS_TYPE_PORT,
						       "plugin-name\0", ags_count_beats_audio_plugin_name,
						       "specifier\0", ags_count_beats_audio_specifier[4],
						       "control-port\0", ags_count_beats_audio_control_port[4],
						       "port-value-is-pointer\0", FALSE,
						       "port-value-type\0", G_TYPE_DOUBLE,
						       "port-value-size\0", sizeof(gdouble),
						       "port-value-length", 1,
						       NULL);

  count_beats_audio->sequencer_loop_end->port_value.ags_port_double = 16.0;

  port = g_list_prepend(port, count_beats_audio->sequencer_loop_end);

  /* port */
  AGS_RECALL(count_beats_audio)->port = port;
}

void
ags_count_beats_audio_set_property(GObject *gobject,
				   guint prop_id,
				   const GValue *value,
				   GParamSpec *param_spec)
{
  AgsCountBeatsAudio *count_beats_audio;

  count_beats_audio = AGS_COUNT_BEATS_AUDIO(gobject);

  switch(prop_id){
  case PROP_LOOP:
    {
      AgsPort *port;

      port = (AgsPort *) g_value_get_object(value);

      if(port == count_beats_audio->loop){
	return;
      }

      if(count_beats_audio->loop != NULL){
	g_object_unref(G_OBJECT(count_beats_audio->loop));
      }
      
      if(port != NULL){
	g_object_ref(G_OBJECT(port));
      }

      count_beats_audio->loop = port;
    }
    break;
  case PROP_NOTATION_LOOP_START:
    {
      AgsPort *port;

      port = (AgsPort *) g_value_get_object(value);

      if(port == count_beats_audio->notation_loop_start){
	return;
      }

      if(count_beats_audio->notation_loop_start != NULL){
	g_object_unref(G_OBJECT(count_beats_audio->notation_loop_start));
      }
      
      if(port != NULL){
	g_object_ref(G_OBJECT(port));
      }

      count_beats_audio->notation_loop_start = port;
    }
    break;
  case PROP_NOTATION_LOOP_END:
    {
      AgsPort *port;

      port = (AgsPort *) g_value_get_object(value);

      if(port == count_beats_audio->notation_loop_end){
	return;
      }

      if(count_beats_audio->notation_loop_end != NULL){
	g_object_unref(G_OBJECT(count_beats_audio->notation_loop_end));
      }
      
      if(port != NULL){
	g_object_ref(G_OBJECT(port));
      }

      count_beats_audio->notation_loop_end = port;
    }
    break;
  case PROP_SEQUENCER_LOOP_START:
    {
      AgsPort *port;

      port = (AgsPort *) g_value_get_object(value);

      if(port == count_beats_audio->sequencer_loop_start){
	return;
      }

      if(count_beats_audio->sequencer_loop_start != NULL){
	g_object_unref(G_OBJECT(count_beats_audio->sequencer_loop_start));
      }
      
      if(port != NULL){
	g_object_ref(G_OBJECT(port));
      }

      count_beats_audio->sequencer_loop_start = port;
    }
    break;
  case PROP_SEQUENCER_LOOP_END:
    {
      AgsPort *port;

      port = (AgsPort *) g_value_get_object(value);

      if(port == count_beats_audio->sequencer_loop_end){
	return;
      }

      if(count_beats_audio->sequencer_loop_end != NULL){
	g_object_unref(G_OBJECT(count_beats_audio->sequencer_loop_end));
      }
      
      if(port != NULL){
	g_object_ref(G_OBJECT(port));
      }

      count_beats_audio->sequencer_loop_end = port;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_count_beats_audio_get_property(GObject *gobject,
				   guint prop_id,
				   GValue *value,
				   GParamSpec *param_spec)
{
  AgsCountBeatsAudio *count_beats;
  
  count_beats = AGS_COUNT_BEATS_AUDIO(gobject);

  switch(prop_id){
  case PROP_LOOP:
    {
      g_value_set_object(value, count_beats->loop);
    }
    break;
  case PROP_NOTATION_LOOP_START:
    {
      g_value_set_object(value, count_beats->notation_loop_start);
    }
    break;
  case PROP_NOTATION_LOOP_END:
    {
      g_value_set_object(value, count_beats->notation_loop_end);
    }
    break;
  case PROP_SEQUENCER_LOOP_START:
    {
      g_value_set_object(value, count_beats->sequencer_loop_start);
    }
    break;
  case PROP_SEQUENCER_LOOP_END:
    {
      g_value_set_object(value, count_beats->sequencer_loop_end);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_count_beats_audio_set_ports(AgsPlugin *plugin, GList *port)
{
  while(port != NULL){
    if(!strncmp(AGS_PORT(port->data)->specifier,
		"./loop[0]\0",
		8)){
      g_object_set(G_OBJECT(plugin),
		   "loop\0", AGS_PORT(port->data),
		   NULL);
    }else if(!strncmp(AGS_PORT(port->data)->specifier,
		      "./notation-loop-start[0]\0",
		      23)){
      g_object_set(G_OBJECT(plugin),
		   "notation-loop-start\0", AGS_PORT(port->data),
		   NULL);
    }else if(!strncmp(AGS_PORT(port->data)->specifier,
		      "./notation-loop-end[0]\0",
		      21)){
      g_object_set(G_OBJECT(plugin),
		   "notation-loop-end\0", AGS_PORT(port->data),
		   NULL);
    }else if(!strncmp(AGS_PORT(port->data)->specifier,
		      "./sequencer-loop-start[0]\0",
		      24)){
      g_object_set(G_OBJECT(plugin),
		   "sequencer-loop-start\0", AGS_PORT(port->data),
		   NULL);
    }else if(!strncmp(AGS_PORT(port->data)->specifier,
		      "./sequencer-loop-end[0]\0",
		      22)){
      g_object_set(G_OBJECT(plugin),
		   "sequencer-loop-end\0", AGS_PORT(port->data),
		   NULL);
    }

    port = port->next;
  }
}

void
ags_count_beats_audio_finalize(GObject *gobject)
{
  AgsCountBeatsAudio *count_beats_audio;

  count_beats_audio = AGS_COUNT_BEATS_AUDIO(gobject);

  if(count_beats_audio->loop != NULL){
    g_object_unref(G_OBJECT(count_beats_audio->loop));
  }

  if(count_beats_audio->notation_loop_start != NULL){
    g_object_unref(G_OBJECT(count_beats_audio->notation_loop_start));
  }

  if(count_beats_audio->notation_loop_end != NULL){
    g_object_unref(G_OBJECT(count_beats_audio->notation_loop_end));
  }

  if(count_beats_audio->sequencer_loop_start != NULL){
    g_object_unref(G_OBJECT(count_beats_audio->sequencer_loop_start));
  }

  if(count_beats_audio->sequencer_loop_end != NULL){
    g_object_unref(G_OBJECT(count_beats_audio->sequencer_loop_end));
  }

  /* call parent */
  G_OBJECT_CLASS(ags_count_beats_audio_parent_class)->finalize(gobject);
}

AgsCountBeatsAudio*
ags_count_beats_audio_new(gboolean loop)
{
  AgsCountBeatsAudio *count_beats_audio;

  count_beats_audio = (AgsCountBeatsAudio *) g_object_new(AGS_TYPE_COUNT_BEATS_AUDIO,
							  "loop\0", loop,
							  NULL);
  
  return(count_beats_audio);
}
