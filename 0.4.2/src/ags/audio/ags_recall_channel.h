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

#ifndef __AGS_RECALL_CHANNEL_H__
#define __AGS_RECALL_CHANNEL_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_channel.h>

#define AGS_TYPE_RECALL_CHANNEL                (ags_recall_channel_get_type())
#define AGS_RECALL_CHANNEL(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_RECALL_CHANNEL, AgsRecallChannel))
#define AGS_RECALL_CHANNEL_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_RECALL_CHANNEL, AgsRecallChannelClass))
#define AGS_IS_RECALL_CHANNEL(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_RECALL_CHANNEL))
#define AGS_IS_RECALL_CHANNEL_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_RECALL_CHANNEL))
#define AGS_RECALL_CHANNEL_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_RECALL_CHANNEL, AgsRecallChannelClass))

typedef struct _AgsRecallChannel AgsRecallChannel;
typedef struct _AgsRecallChannelClass AgsRecallChannelClass;

struct _AgsRecallChannel
{
  AgsRecall recall;

  AgsChannel *destination;
  AgsChannel *source;
};

struct _AgsRecallChannelClass
{
  AgsRecallClass recall;
};

GType ags_recall_channel_get_type();

GList* ags_recall_channel_find_channel(GList *recall_channel, AgsChannel *channel);

AgsRecallChannel* ags_recall_channel_new();

#endif /*__AGS_RECALL_CHANNEL_H__*/
