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

#include <ags/X/machine/ags_ffplayer.h>
#include <ags/X/machine/ags_ffplayer_callbacks.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/object/ags_playable.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_recall_factory.h>
#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_recall_container.h>

#include <ags/audio/recall/ags_delay_audio.h>
#include <ags/audio/recall/ags_delay_audio_run.h>
#include <ags/audio/recall/ags_count_beats_audio.h>
#include <ags/audio/recall/ags_count_beats_audio_run.h>
#include <ags/audio/recall/ags_copy_channel.h>
#include <ags/audio/recall/ags_copy_channel_run.h>
#include <ags/audio/recall/ags_stream_channel.h>
#include <ags/audio/recall/ags_stream_channel_run.h>
#include <ags/audio/recall/ags_buffer_channel.h>
#include <ags/audio/recall/ags_buffer_channel_run.h>
#include <ags/audio/recall/ags_play_notation_audio.h>
#include <ags/audio/recall/ags_play_notation_audio_run.h>

#include <ags/X/ags_editor.h>

#include <math.h>

void ags_ffplayer_class_init(AgsFFPlayerClass *ffplayer);
void ags_ffplayer_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_ffplayer_init(AgsFFPlayer *ffplayer);
void ags_ffplayer_connect(AgsConnectable *connectable);
void ags_ffplayer_disconnect(AgsConnectable *connectable);
void ags_ffplayer_finalize(GObject *gobject);
void ags_ffplayer_show(GtkWidget *widget);
void ags_ffplayer_add_default_recalls(AgsMachine *machine);

void ags_ffplayer_set_audio_channels(AgsAudio *audio,
				     guint audio_channels, guint audio_channels_old,
				     gpointer data);
void ags_ffplayer_set_pads(AgsAudio *audio, GType type,
			   guint pads, guint pads_old,
			   gpointer data);

void ags_ffplayer_output_map_recall(AgsFFPlayer *ffplayer, guint output_pad_start);
void ags_ffplayer_input_map_recall(AgsFFPlayer *ffplayer, guint input_pad_start);
void ags_ffplayer_paint(AgsFFPlayer *ffplayer);

static gpointer ags_ffplayer_parent_class = NULL;
static AgsConnectableInterface *ags_ffplayer_parent_connectable_interface;

GtkStyle *ffplayer_style;

GType
ags_ffplayer_get_type(void)
{
  static GType ags_type_ffplayer = 0;

  if(!ags_type_ffplayer){
    static const GTypeInfo ags_ffplayer_info = {
      sizeof(AgsFFPlayerClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_ffplayer_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsFFPlayer),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_ffplayer_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_ffplayer_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_ffplayer = g_type_register_static(AGS_TYPE_MACHINE,
					    "AgsFFPlayer\0", &ags_ffplayer_info,
					    0);
    
    g_type_add_interface_static(ags_type_ffplayer,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_ffplayer);
}

void
ags_ffplayer_class_init(AgsFFPlayerClass *ffplayer)
{
  GObjectClass *gobject;
  GtkWidgetClass *widget;
  AgsMachineClass *machine;

  ags_ffplayer_parent_class = g_type_class_peek_parent(ffplayer);

  /* GObjectClass */
  gobject = (GObjectClass *) ffplayer;

  gobject->finalize = ags_ffplayer_finalize;

  /* GtkWidgetClass */
  widget = (GtkWidgetClass *) ffplayer;

  widget->show = ags_ffplayer_show;

  /* AgsMachineClass */
  machine = (AgsMachineClass *) ffplayer;

  machine->add_default_recalls = ags_ffplayer_add_default_recalls;
  //  machine->read_file = ags_file_read_ffplayer;
  //  machine->write_file = ags_file_write_ffplayer;
}

void
ags_ffplayer_connectable_interface_init(AgsConnectableInterface *connectable)
{
  AgsConnectableInterface *ags_ffplayer_connectable_parent_interface;

  ags_ffplayer_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_ffplayer_connect;
}

void
ags_ffplayer_init(AgsFFPlayer *ffplayer)
{
  AgsAudio *audio;
  GtkTable *table;
  GtkHScrollbar *hscrollbar;
  GtkVBox *vbox;
  GtkHBox *hbox;
  GtkLabel *label;
  PangoAttrList *attr_list;
  PangoAttribute *attr;

  g_signal_connect_after((GObject *) ffplayer, "parent_set\0",
			 G_CALLBACK(ags_ffplayer_parent_set_callback), (gpointer) ffplayer);

  audio = AGS_MACHINE(ffplayer)->audio;
  audio->flags |= (AGS_AUDIO_OUTPUT_HAS_RECYCLING |
		   AGS_AUDIO_INPUT_HAS_RECYCLING |
		   AGS_AUDIO_INPUT_TAKES_FILE |
		   AGS_AUDIO_ASYNC |
		   AGS_AUDIO_HAS_NOTATION | 
		   AGS_AUDIO_NOTATION_DEFAULT);
  
  AGS_MACHINE(ffplayer)->flags |= AGS_MACHINE_IS_SYNTHESIZER;
  AGS_MACHINE(ffplayer)->file_input_flags |= AGS_MACHINE_ACCEPT_SOUNDFONT2;

  ffplayer->mapped_input_pad = 0;
  ffplayer->mapped_output_pad = 0;

  /* create widgets */
  table = (GtkTable *) gtk_table_new(3, 2, FALSE);
  gtk_container_add((GtkContainer *) (gtk_container_get_children((GtkContainer *) ffplayer))->data, (GtkWidget *) table);

  hbox = (GtkHBox *) gtk_hbox_new(FALSE, 0);
  gtk_table_attach(table,
		   GTK_WIDGET(hbox),
		   0, 1,
		   1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  label = (GtkLabel *) g_object_new(GTK_TYPE_LABEL,
				    "label\0", "instrument\0",
				    "xalign\0", 0.0,
				    NULL);
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(label),
		     FALSE, FALSE,
		     0);

  ffplayer->instrument = (GtkComboBoxText *) gtk_combo_box_text_new();
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(ffplayer->instrument),
		     TRUE, FALSE,
		     0);

  ffplayer->open = (GtkButton *) g_object_new(GTK_TYPE_BUTTON,
					      "label\0", GTK_STOCK_OPEN,
					      "use-stock\0", TRUE,
					      NULL);
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(ffplayer->open),
		     FALSE, FALSE,
		     0);

  vbox = (GtkVBox *) gtk_vbox_new(FALSE, 2);
  gtk_table_attach(table, (GtkWidget *) vbox,
		   1, 2,
		   0, 3,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  ffplayer->control_width = 12;
  ffplayer->control_height = 40;

  ffplayer->drawing_area = (GtkDrawingArea *) gtk_drawing_area_new();
  gtk_widget_set_size_request((GtkWidget *) ffplayer->drawing_area, 16 * ffplayer->control_width, ffplayer->control_width * 8 + ffplayer->control_height);
  gtk_widget_set_style((GtkWidget *) ffplayer->drawing_area, ffplayer_style);
  gtk_widget_set_events ((GtkWidget *) ffplayer->drawing_area,
                         GDK_EXPOSURE_MASK
                         | GDK_LEAVE_NOTIFY_MASK
                         | GDK_BUTTON_PRESS_MASK
                         | GDK_POINTER_MOTION_MASK
                         | GDK_POINTER_MOTION_HINT_MASK);
  gtk_box_pack_start((GtkBox *) vbox, (GtkWidget *) ffplayer->drawing_area, FALSE, FALSE, 0);

  ffplayer->hadjustment = (GtkAdjustment *) gtk_adjustment_new(0.0,
							       0.0,
							       76 * ffplayer->control_width - GTK_WIDGET(ffplayer->drawing_area)->allocation.width,
							       1.0,
							       (double) ffplayer->control_width,
							       (double) (16 * ffplayer->control_width));
  hscrollbar = (GtkHScrollbar *) gtk_hscrollbar_new(ffplayer->hadjustment);
  gtk_widget_set_style((GtkWidget *) hscrollbar, ffplayer_style);
  gtk_box_pack_start((GtkBox *) vbox, (GtkWidget *) hscrollbar, FALSE, FALSE, 0);
}

void
ags_ffplayer_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_ffplayer_parent_class)->finalize(gobject);
}

void
ags_ffplayer_connect(AgsConnectable *connectable)
{
  AgsFFPlayer *ffplayer;

  ags_ffplayer_parent_connectable_interface->connect(connectable);

  /* AgsFFPlayer */
  ffplayer = AGS_FFPLAYER(connectable);

  g_signal_connect((GObject *) ffplayer->open, "clicked\0",
		   G_CALLBACK(ags_ffplayer_open_clicked_callback), (gpointer) ffplayer);

  g_signal_connect_after((GObject *) ffplayer->instrument, "changed\0",
			 G_CALLBACK(ags_ffplayer_instrument_changed_callback), (gpointer) ffplayer);


  g_signal_connect((GObject *) ffplayer->drawing_area, "expose_event\0",
                   G_CALLBACK(ags_ffplayer_drawing_area_expose_callback), (gpointer) ffplayer);

  g_signal_connect((GObject *) ffplayer->drawing_area, "button_press_event\0",
                   G_CALLBACK(ags_ffplayer_drawing_area_button_press_callback), (gpointer) ffplayer);

  g_signal_connect((GObject *) ffplayer->hadjustment, "value_changed\0",
		   G_CALLBACK(ags_ffplayer_hscrollbar_value_changed), (gpointer) ffplayer);

  /* AgsAudio */
  g_signal_connect_after(G_OBJECT(ffplayer->machine.audio), "set_audio_channels\0",
			 G_CALLBACK(ags_ffplayer_set_audio_channels), NULL);

  g_signal_connect_after(G_OBJECT(ffplayer->machine.audio), "set_pads\0",
			 G_CALLBACK(ags_ffplayer_set_pads), NULL);
}

void
ags_ffplayer_disconnect(AgsConnectable *connectable)
{
  ags_ffplayer_parent_connectable_interface->disconnect(connectable);

  //TODO:JK
  /* implement me */
}

void
ags_ffplayer_show(GtkWidget *widget)
{
  GTK_WIDGET_CLASS(ags_ffplayer_parent_class)->show(widget);
}

void
ags_ffplayer_add_default_recalls(AgsMachine *machine)
{
  /* empty */
}

void
ags_ffplayer_set_audio_channels(AgsAudio *audio,
				guint audio_channels, guint audio_channels_old,
				gpointer data)
{
  AgsFFPlayer *ffplayer;
  gboolean grow;

  ffplayer = AGS_FFPLAYER(audio->machine);

  if(audio_channels_old == audio_channels){
    return;
  }

  if(audio_channels_old < audio_channels){
    grow = TRUE;
  }else{
    grow = FALSE;
  }

  if(grow){
  }
}

void
ags_ffplayer_set_pads(AgsAudio *audio, GType type,
		      guint pads, guint pads_old,
		      gpointer data)
{
  AgsFFPlayer *ffplayer;
  gboolean grow;
  
  GValue value = {0,};

  ffplayer = AGS_FFPLAYER(audio->machine);

  if(pads_old == pads){
    return;
  }

  if(pads_old == pads)
    return;
  if(pads_old < pads)
    grow = TRUE;
  else
    grow = FALSE;

  if(type == AGS_TYPE_INPUT){
    AgsPlayNotationAudio  *play_notation;
    GList *list, *notation;

    if(grow){
      /* set notation for AgsPlayNotationAudioRun recall */
      list = audio->recall;

      while((list = ags_recall_find_type(list,
					 AGS_TYPE_PLAY_NOTATION_AUDIO)) != NULL){
	play_notation = AGS_PLAY_NOTATION_AUDIO(list->data);

	ags_port_safe_read(play_notation->notation,
			   &value);

	if(g_value_get_object(&value) == NULL){
	  notation = audio->notation;
	
	  while(notation != NULL){
	    g_object_set(G_OBJECT(play_notation),
			 "notation\0", notation->data,
			 NULL);
	
	    notation = notation->next;
	  }
	}

	list = list->next;
      }

      /* depending on destination */
      ags_ffplayer_input_map_recall(ffplayer, pads_old);
    }else{
      ffplayer->mapped_input_pad = audio->input_pads;
    }
  }else{
    if(grow){
      /* depending on destination */
      ags_ffplayer_output_map_recall(ffplayer, pads_old);
    }else{
      ffplayer->mapped_output_pad = audio->output_pads;
    }
  }
}

void
ags_ffplayer_input_map_recall(AgsFFPlayer *ffplayer, guint input_pad_start)
{
  AgsAudio *audio;
  AgsChannel *source, *current, *destination;
  AgsCopyChannel *copy_channel;
  AgsCopyChannelRun *copy_channel_run;

  GList *list;

  audio = AGS_MACHINE(ffplayer)->audio;

  if(ffplayer->mapped_input_pad > input_pad_start){
    return;
  }else{
    ffplayer->mapped_input_pad = audio->input_pads;
  }

  source = ags_channel_nth(audio->input,
			   input_pad_start * audio->audio_channels);

  current = source;

  while(current != NULL){
    /* ags-copy */
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-copy\0",
			      current->audio_channel, current->audio_channel + 1, 
			      current->pad, current->pad + 1,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_RECALL |
			       AGS_RECALL_FACTORY_ADD),
			      0);


    destination = ags_channel_nth(audio->output,
				  current->audio_channel);

    while(destination != NULL){
      /* recall */
      list = current->recall;

      while((list = ags_recall_find_type(list, AGS_TYPE_COPY_CHANNEL)) != NULL){
	copy_channel = AGS_COPY_CHANNEL(list->data);

	g_object_set(G_OBJECT(copy_channel),
		     "destination\0", destination,
		     NULL);

	list = list->next;
      }

      list = current->recall;
    
      while((list = ags_recall_find_type(list, AGS_TYPE_COPY_CHANNEL_RUN)) != NULL){
	copy_channel_run = AGS_COPY_CHANNEL_RUN(list->data);

	g_object_set(G_OBJECT(copy_channel_run),
		     "destination\0", destination,
		     NULL);

	list = list->next;
      }

      destination = destination->next_pad;
    }

    /* ags-stream */
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-stream\0",
			      current->audio_channel, current->audio_channel + 1, 
			      current->pad, current->pad + 1,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_PLAY |
			       AGS_RECALL_FACTORY_RECALL | 
			       AGS_RECALL_FACTORY_ADD),
			      0);

    current = current->next_pad;
  }
}

void
ags_ffplayer_output_map_recall(AgsFFPlayer *ffplayer, guint output_pad_start)
{
  AgsAudio *audio;
  AgsChannel *source;

  audio = AGS_MACHINE(ffplayer)->audio;

  if(ffplayer->mapped_output_pad > output_pad_start){
    return;
  }else{
    ffplayer->mapped_output_pad = audio->output_pads;
  }
  
  source = ags_channel_nth(audio->output,
			   output_pad_start * audio->audio_channels);

  /* ags-stream */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-buffer\0",
			    source->audio_channel, source->audio_channel + 1,
			    output_pad_start, audio->output_pads,
			    (AGS_RECALL_FACTORY_OUTPUT |
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_RECALL | 
			     AGS_RECALL_FACTORY_ADD),
			    0);
}

void
ags_ffplayer_paint(AgsFFPlayer *ffplayer)
{
  GtkWidget *widget;
  cairo_t *cr;
  double semi_key_height;
  guint bitmap;
  guint x[2];
  guint i, i_stop, j, j0;

  widget = (GtkWidget *) ffplayer->drawing_area;

  semi_key_height = 2.0 / 3.0 * (double) ffplayer->control_height;
  bitmap = 0x52a52a; // description of the keyboard

  j = (guint) ceil(ffplayer->hadjustment->value / (double) ffplayer->control_width);
  j = j % 12;

  x[0] = (guint) round(ffplayer->hadjustment->value) % ffplayer->control_width;

  if(x[0] != 0){
    x[0] = ffplayer->control_width - x[0];
  }

  x[1] = ((guint) widget->allocation.width - x[0]) % ffplayer->control_width;
  i_stop = (widget->allocation.width - x[0] - x[1]) / ffplayer->control_width;

  cr = gdk_cairo_create(widget->window);

  cairo_set_source_rgb(cr, 0.015625, 0.03125, 0.21875);
  cairo_rectangle(cr, 0.0, 0.0, (double) widget->allocation.width, (double) widget->allocation.height);
  cairo_fill(cr);

  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgb(cr, 0.75, 0.75, 0.75);

  if(x[0] != 0){
    j0 = (j != 0) ? j -1: 11;

    if(((1 << j0) & bitmap) != 0){
      cairo_set_source_rgb(cr, 0.5, 0.5, 1.0);

      cairo_rectangle(cr, 0.0, 0.0, x[0], (double) semi_key_height);
      cairo_fill(cr); 	

      cairo_set_source_rgb(cr, 0.75, 0.75, 0.75);

      if(x[0] > ffplayer->control_width / 2){
	cairo_move_to(cr, (double) (x[0] - ffplayer->control_width / 2),  semi_key_height);
	cairo_line_to(cr, (double) (x[0] - ffplayer->control_width / 2), (double) ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, 0.0, ffplayer->control_height);
      cairo_line_to(cr, (double) x[0], ffplayer->control_height);
      cairo_stroke(cr);
    }else{
      if(((1 << (j0 + 1)) & bitmap) == 0){
	cairo_move_to(cr, (double) x[0], 0.0);
	cairo_line_to(cr, (double) x[0], ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, 0.0, ffplayer->control_height);
      cairo_line_to(cr, (double) x[0], ffplayer->control_height);
      cairo_stroke(cr);
    }
  }

  for(i = 0; i < i_stop; i++){
    if(((1 << j) & bitmap) != 0){
      // draw semi tone key
      cairo_set_source_rgb(cr, 0.5, 0.5, 1.0);

      cairo_rectangle(cr, (double) (i * ffplayer->control_width + x[0]), 0.0, (double) ffplayer->control_width, semi_key_height);
      cairo_fill(cr); 	

      cairo_set_source_rgb(cr, 0.75, 0.75, 0.75);

      cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width / 2), semi_key_height);
      cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width / 2), ffplayer->control_height);
      cairo_stroke(cr);

      cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0]), ffplayer->control_height);
      cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), ffplayer->control_height);
      cairo_stroke(cr);
    }else{
      // no semi tone key
      if(((1 << (j + 1)) & bitmap) == 0){
	cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), 0.0);
	cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0]), ffplayer->control_height);
      cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), ffplayer->control_height);
      cairo_stroke(cr);
    }

    if(j == 11)
      j = 0;
    else
      j++;
  }

  if(x[1] != 0){
    j0 = j;

    if(((1 << j0) & bitmap) != 0){
      cairo_set_source_rgb(cr, 0.5, 0.5, 1.0);

      cairo_rectangle(cr, (double) (widget->allocation.width - x[1]), 0.0, (double) x[1], semi_key_height);
      cairo_fill(cr); 	

      cairo_set_source_rgb(cr, 0.75, 0.75, 0.75);

      if(x[1] > ffplayer->control_width / 2){
	cairo_move_to(cr, (double) (widget->allocation.width - x[1] + ffplayer->control_width / 2), semi_key_height);
	cairo_line_to(cr, (double) (widget->allocation.width - x[1] + ffplayer->control_width / 2), ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, (double) (widget->allocation.width - x[1]), ffplayer->control_height);
      cairo_line_to(cr, (double) widget->allocation.width, ffplayer->control_height);
      cairo_stroke(cr);
    }else{
      cairo_move_to(cr, (double) (widget->allocation.width - x[1]), ffplayer->control_height);
      cairo_line_to(cr, (double) widget->allocation.width, ffplayer->control_height);
      cairo_stroke(cr);
    }
  }
}

AgsFFPlayer*
ags_ffplayer_new(GObject *devout)
{
  AgsFFPlayer *ffplayer;
  GValue value = {0,};

  ffplayer = (AgsFFPlayer *) g_object_new(AGS_TYPE_FFPLAYER,
					  NULL);

  if(devout != NULL){
    g_value_init(&value, G_TYPE_OBJECT);
    g_value_set_object(&value, devout);
    g_object_set_property(G_OBJECT(ffplayer->machine.audio),
			  "devout\0", &value);
    g_value_unset(&value);
  }

  return(ffplayer);
}
