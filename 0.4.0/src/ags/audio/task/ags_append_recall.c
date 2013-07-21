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

#include <ags/audio/task/ags_append_recall.h>

#include <ags/object/ags_connectable.h>

#include <ags/thread/ags_audio_loop.h>

void ags_append_recall_class_init(AgsAppendRecallClass *append_recall);
void ags_append_recall_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_append_recall_init(AgsAppendRecall *append_recall);
void ags_append_recall_connect(AgsConnectable *connectable);
void ags_append_recall_disconnect(AgsConnectable *connectable);
void ags_append_recall_finalize(GObject *gobject);

void ags_append_recall_launch(AgsTask *task);

static gpointer ags_append_recall_parent_class = NULL;
static AgsConnectableInterface *ags_append_recall_parent_connectable_interface;

GType
ags_append_recall_get_type()
{
  static GType ags_type_append_recall = 0;

  if(!ags_type_append_recall){
    static const GTypeInfo ags_append_recall_info = {
      sizeof (AgsAppendRecallClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_append_recall_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAppendRecall),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_append_recall_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_append_recall_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_append_recall = g_type_register_static(AGS_TYPE_TASK,
						  "AgsAppendRecall\0",
						  &ags_append_recall_info,
						  0);

    g_type_add_interface_static(ags_type_append_recall,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_append_recall);
}

void
ags_append_recall_class_init(AgsAppendRecallClass *append_recall)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_append_recall_parent_class = g_type_class_peek_parent(append_recall);

  /* GObjectClass */
  gobject = (GObjectClass *) append_recall;

  gobject->finalize = ags_append_recall_finalize;

  /* AgsTaskClass */
  task = (AgsTaskClass *) append_recall;

  task->launch = ags_append_recall_launch;
}

void
ags_append_recall_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_append_recall_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_append_recall_connect;
  connectable->disconnect = ags_append_recall_disconnect;
}

void
ags_append_recall_init(AgsAppendRecall *append_recall)
{
  append_recall->audio_loop = NULL;
  append_recall->devout_play = NULL;
}

void
ags_append_recall_connect(AgsConnectable *connectable)
{
  ags_append_recall_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_append_recall_disconnect(AgsConnectable *connectable)
{
  ags_append_recall_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_append_recall_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_append_recall_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_append_recall_launch(AgsTask *task)
{
  AgsAppendRecall *append_recall;
  AgsAudioLoop *audio_loop;

  append_recall = AGS_APPEND_RECALL(task);

  audio_loop = AGS_AUDIO_LOOP(append_recall->audio_loop);

  /* append to AgsDevout */
  append_recall->devout_play->flags &= (~AGS_DEVOUT_PLAY_REMOVE);
  audio_loop->play_recall = g_list_append(audio_loop->play_recall,
					  append_recall->devout_play);
  audio_loop->play_recall_ref += 1;
}

AgsAppendRecall*
ags_append_recall_new(GObject *audio_loop,
		      AgsDevoutPlay *devout_play)
{
  AgsAppendRecall *append_recall;

  append_recall = (AgsAppendRecall *) g_object_new(AGS_TYPE_APPEND_RECALL,
						   NULL);
  
  append_recall->audio_loop = audio_loop;
  append_recall->devout_play = devout_play;

  return(append_recall);
}
