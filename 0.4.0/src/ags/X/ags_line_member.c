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

#include <ags/X/ags_line_member.h>
#include <ags/X/ags_line_member_callbacks.h>

#include <ags/main.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/thread/ags_audio_loop.h>
#include <ags/thread/ags_task_thread.h>

#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_devout.h>

#include <ags/widget/ags_dial.h>

#include <ags/X/ags_pad.h>

void ags_line_member_class_init(AgsLineMemberClass *line_member);
void ags_line_member_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_line_member_init(AgsLineMember *line_member);
void ags_line_member_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec);
void ags_line_member_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec);
void ags_line_member_connect(AgsConnectable *connectable);
void ags_line_member_disconnect(AgsConnectable *connectable);
void ags_line_member_finalize(GObject *gobject);

void ags_line_member_real_change_port(AgsLineMember *line_member,
				      gpointer port_data);

enum{
  CHANGE_PORT,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_WIDGET_TYPE,
  PROP_PLUGIN_NAME,
  PROP_SPECIFIER,
  PROP_CONTROL_PORT,
  PROP_PORT,
  PROP_PORT_DATA,
  PROP_PORT_DATA_LENGTH,
  PROP_TASK_TYPE,
};

static gpointer ags_line_member_parent_class = NULL;
static guint line_member_signals[LAST_SIGNAL];

GType
ags_line_member_get_type(void)
{
  static GType ags_type_line_member = 0;

  if(!ags_type_line_member){
    static const GTypeInfo ags_line_member_info = {
      sizeof(AgsLineMemberClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_line_member_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsLineMember),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_line_member_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_line_member_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_line_member = g_type_register_static(GTK_TYPE_FRAME,
						  "AgsLineMember\0", &ags_line_member_info,
						  0);

    g_type_add_interface_static(ags_type_line_member,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_line_member);
}

void
ags_line_member_class_init(AgsLineMemberClass *line_member)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_line_member_parent_class = g_type_class_peek_parent(line_member);

  /* GObjectClass */
  gobject = G_OBJECT_CLASS(line_member);

  gobject->set_property = ags_line_member_set_property;
  gobject->get_property = ags_line_member_get_property;

  gobject->finalize = ags_line_member_finalize;

  /* properties */
  param_spec = g_param_spec_ulong("widget-type\0",
				  "widget type of line member\0",
				  "The widget_type this line member packs\0",
				  0, G_MAXULONG, 
				  G_TYPE_NONE,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_WIDGET_TYPE,
				  param_spec);

  param_spec = g_param_spec_string("plugin-name\0",
				   "plugin name to control\0",
				   "The plugin's name to control\0",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PLUGIN_NAME,
				  param_spec);

  param_spec = g_param_spec_string("control-port\0",
				   "control port index\0",
				   "The index of the port to control\0",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CONTROL_PORT,
				  param_spec);

  /* AgsLineMember */
  line_member->change_port = ags_line_member_change_port;
}

void
ags_line_member_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_line_member_connect;
  connectable->disconnect = ags_line_member_disconnect;
}

void
ags_line_member_init(AgsLineMember *line_member)
{
  g_signal_connect_after((GObject *) line_member, "parent_set\0",
			 G_CALLBACK(ags_line_member_parent_set_callback), (gpointer) line_member);

  line_member->flags = AGS_LINE_MEMBER_RESET_BY_ATOMIC;

  line_member->widget_type = AGS_TYPE_DIAL;

  line_member->plugin_name = NULL;
  line_member->specifier = NULL;

  line_member->control_port = NULL;

  line_member->port = NULL;
  line_member->port_data = NULL;

  line_member->task_type = G_TYPE_NONE;
}

void
ags_line_member_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec)
{
  AgsLineMember *line_member;

  line_member = AGS_LINE_MEMBER(gobject);

  switch(prop_id){
  case PROP_WIDGET_TYPE:
    {
      GtkWidget *child, *new_child;
      GType widget_type;

      widget_type = g_value_get_ulong(value);

      if(widget_type == line_member->widget_type){
	return;
      }

      child = gtk_bin_get_child(GTK_BIN(line_member));

      if(child != NULL){
	gtk_widget_destroy(child);
      }

      new_child = (GtkWidget *) g_object_new(widget_type,
					     NULL);
      gtk_container_add(GTK_CONTAINER(line_member),
			new_child);
			
    }
    break;
  case PROP_PLUGIN_NAME:
    {
    }
    break;
  case PROP_SPECIFIER:
    {
    }
    break;
  case PROP_CONTROL_PORT:
    {
    }
    break;
  case PROP_PORT_DATA:
    {
    }
    break;
  case PROP_PORT_DATA_LENGTH:
    {
    }
    break;
  case PROP_TASK_TYPE:
    {
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_line_member_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec)
{
  /* empty */
}

void
ags_line_member_connect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_line_member_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_line_member_finalize(GObject *gobject)
{
  /* empty */
}

void
ags_line_member_real_change_port(AgsLineMember *line_member,
				 gpointer port_data)
{
  if((AGS_LINE_MEMBER_RESET_BY_ATOMIC & (line_member->flags)) != 0){
    g_atomic_pointer_set(&line_member->port,
			 port_data);
  }

  if((AGS_LINE_MEMBER_RESET_BY_TASK & (line_member->flags)) != 0){
    AgsLine *line;
    AgsTaskThread *task_thread;
    AgsTask *task;

    line = (AgsLine *) gtk_widget_get_ancestor(GTK_WIDGET(line_member),
					       AGS_TYPE_LINE);
    
    task_thread = AGS_TASK_THREAD(AGS_AUDIO_LOOP(AGS_MAIN(AGS_DEVOUT(line->channel->devout)->ags_main)->main_loop)->task_thread);

    task = (AgsTask *) g_object_new(line_member->task_type,
				    line_member->control_port, port_data,
				    NULL);

    ags_task_thread_append_task(task_thread,
				task);
  }

  line_member->port_data = port_data;
}

void
ags_line_member_change_port(AgsLineMember *line_member,
			    gpointer port_data)
{
  //TODO:JK: implement me
}

AgsLineMember*
ags_line_member_new()
{
  AgsLineMember *line_member;

  line_member = (AgsLineMember *) g_object_new(AGS_TYPE_LINE_MEMBER,
					       NULL);

  return(line_member);
}
