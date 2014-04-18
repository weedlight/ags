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

#include <ags/X/machine/ags_panel_input_line.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_factory.h>
#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_recall_container.h>

#include <ags/audio/recall/ags_play_channel.h>
#include <ags/audio/recall/ags_play_channel_run_master.h>
#include <ags/audio/recall/ags_mute_channel.h>
#include <ags/audio/recall/ags_mute_channel_run.h>

#include <ags/widget/ags_expander_set.h>
#include <ags/widget/ags_expander.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_line_callbacks.h>
#include <ags/X/ags_line_member.h>

#include <ags/X/machine/ags_panel.h>

void ags_panel_input_line_class_init(AgsPanelInputLineClass *panel_input_line);
void ags_panel_input_line_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_panel_input_line_init(AgsPanelInputLine *panel_input_line);
void ags_panel_input_line_connect(AgsConnectable *connectable);
void ags_panel_input_line_disconnect(AgsConnectable *connectable);
void ags_panel_input_line_finalize(GObject *gobject);

void ags_panel_input_line_show(GtkWidget *line);

void ags_panel_input_line_set_channel(AgsLine *line, AgsChannel *channel);
void ags_panel_input_line_group_changed(AgsLine *line);

static gpointer ags_panel_input_line_parent_class = NULL;
static AgsConnectableInterface *ags_panel_input_line_parent_connectable_interface;

GType
ags_panel_input_line_get_type()
{
  static GType ags_type_panel_input_line = 0;

  if(!ags_type_panel_input_line){
    static const GTypeInfo ags_panel_input_line_info = {
      sizeof(AgsPanelInputLineClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_panel_input_line_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsPanelInputLine),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_panel_input_line_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_panel_input_line_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_panel_input_line = g_type_register_static(AGS_TYPE_LINE,
						       "AgsPanelInputLine\0", &ags_panel_input_line_info,
						       0);

    g_type_add_interface_static(ags_type_panel_input_line,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_panel_input_line);
}

void
ags_panel_input_line_class_init(AgsPanelInputLineClass *panel_input_line)
{
  GObjectClass *gobject;
  GtkWidgetClass *widget;
  AgsLineClass *line;

  ags_panel_input_line_parent_class = g_type_class_peek_parent(panel_input_line);

  /* GObjectClass */
  gobject = (GObjectClass *) panel_input_line;

  gobject->finalize = ags_panel_input_line_finalize;

  /* GtkWidgetClass */
  widget = (GtkWidgetClass *) panel_input_line;

  widget->show = ags_panel_input_line_show;

  /* AgsLineClass */
  line = AGS_LINE_CLASS(panel_input_line);

  line->set_channel = ags_panel_input_line_set_channel;
}

void
ags_panel_input_line_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_panel_input_line_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_panel_input_line_connect;
  connectable->disconnect = ags_panel_input_line_disconnect;
}

void
ags_panel_input_line_init(AgsPanelInputLine *panel_input_line)
{
  AgsLineMember *line_member;

  line_member = ags_line_member_new();
  line_member->flags |= AGS_LINE_MEMBER_DEFAULT_TEMPLATE;
  line_member->widget_type = GTK_TYPE_CHECK_BUTTON;
  ags_expander_add(AGS_LINE(panel_input_line)->expander,
		   GTK_WIDGET(line_member),
		   0, 0,
		   1, 1);

  panel_input_line->mute = (GtkCheckButton *) gtk_check_button_new_with_label("mute\0");
  gtk_container_add(GTK_CONTAINER(line_member),
		    GTK_WIDGET(panel_input_line->mute));
}

void
ags_panel_input_line_connect(AgsConnectable *connectable)
{
  AgsPanelInputLine *panel_input_line;

  panel_input_line = AGS_PANEL_INPUT_LINE(connectable);


  if((AGS_LINE_CONNECTED & (AGS_LINE(panel_input_line)->flags)) != 0){
    return;
  }
  
  ags_panel_input_line_parent_connectable_interface->connect(connectable);

  //TODO:JK: implement me
}

void
ags_panel_input_line_disconnect(AgsConnectable *connectable)
{
  ags_panel_input_line_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_panel_input_line_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_panel_input_line_parent_class)->finalize(gobject);
  
  /* empty */
}

void
ags_panel_input_line_show(GtkWidget *line)
{
  GTK_WIDGET_CLASS(ags_panel_input_line_parent_class)->show(line);

  gtk_widget_hide(GTK_WIDGET(AGS_LINE(line)->group));
}

void
ags_panel_input_line_set_channel(AgsLine *line, AgsChannel *channel)
{
  AgsPanelInputLine *panel_input_line;

  AGS_LINE_CLASS(ags_panel_input_line_parent_class)->set_channel(line, channel);

  panel_input_line = AGS_PANEL_INPUT_LINE(line);

  g_message("ags_panel_input_line_set_channel - channel: %u\0",
	    channel->line);

  if(line->channel != NULL){
    line->flags &= (~AGS_LINE_MAPPED_RECALL);
  }

  if(channel != NULL){
    if((AGS_LINE_PREMAPPED_RECALL & (line->flags)) == 0){
      ags_panel_input_line_map_recall(panel_input_line, 0);
    }
  }
}

void
ags_panel_input_line_map_recall(AgsPanelInputLine *panel_input_line,
				guint output_pad_start)
{
  AgsPanel *panel;
  AgsLine *line;

  AgsAudio *audio;
  AgsChannel *source;
  AgsChannel *current;
  AgsPlayChannel *play_channel;
  AgsPlayChannelRunMaster *play_channel_run;

  GList *list;

  line = AGS_LINE(panel_input_line);
  line->flags |= AGS_LINE_MAPPED_RECALL;

  audio = AGS_AUDIO(line->channel->audio);

  panel = AGS_PANEL(audio->machine);

  source = line->channel;

  /* ags-play */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-play-master\0",
			    source->audio_channel, source->audio_channel + 1,
			    source->pad, source->pad + 1,
			    (AGS_RECALL_FACTORY_INPUT,
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_ADD),
			    0);

  /* set audio channel */
  list = source->play;

  while((list = ags_recall_template_find_type(list,
					      AGS_TYPE_PLAY_CHANNEL)) != NULL){

    AGS_PLAY_CHANNEL(list->data)->audio_channel->port_value.ags_port_uint = source->audio_channel;

    list = list->next;
  }
}

AgsPanelInputLine*
ags_panel_input_line_new(AgsChannel *channel)
{
  AgsPanelInputLine *panel_input_line;

  panel_input_line = (AgsPanelInputLine *) g_object_new(AGS_TYPE_PANEL_INPUT_LINE,
							"channel\0", channel,
							NULL);

  return(panel_input_line);
}
