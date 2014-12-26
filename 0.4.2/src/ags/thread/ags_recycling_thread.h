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

#ifndef __AGS_RECYCLING_THREAD_H__
#define __AGS_RECYCLING_THREAD_H__

#include <glib.h>
#include <glib-object.h>

#ifdef AGS_USE_LINUX_THREADS
#include <ags/thread/ags_thread-kthreads.h>
#else
#include <ags/thread/ags_thread-posix.h>
#endif 

#include <ags/audio/ags_recall_id.h>

#define AGS_TYPE_RECYCLING_THREAD                (ags_recycling_thread_get_type())
#define AGS_RECYCLING_THREAD(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_RECYCLING_THREAD, AgsRecyclingThread))
#define AGS_RECYCLING_THREAD_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_RECYCLING_THREAD, AgsRecyclingThread))
#define AGS_IS_RECYCLING_THREAD(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_RECYCLING_THREAD))
#define AGS_IS_RECYCLING_THREAD_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_RECYCLING_THREAD))
#define AGS_RECYCLING_THREAD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_RECYCLING_THREAD, AgsRecyclingThreadClass))

#define AGS_RECYCLING_THREAD_DEFAULT_JIFFIE (100)

typedef struct _AgsRecyclingThread AgsRecyclingThread;
typedef struct _AgsRecyclingThreadClass AgsRecyclingThreadClass;

typedef enum{
  AGS_RECYCLING_THREAD_RUNNING   = 1,
  AGS_RECYCLING_THREAD_WAIT      = 1 << 1,
  AGS_RECYCLING_THREAD_DONE      = 1 << 2,
}AgsRecyclingThreadFlags;

struct _AgsRecyclingThread
{
  AgsThread thread;

  guint flags;

  AgsThread *iterator_thread;

  pthread_mutex_t iteration_mutex;
  pthread_cond_t iteration_cond;
};

struct _AgsRecyclingThreadClass
{
  AgsThreadClass thread;

  void (*play_channel)(AgsRecyclingThread *recycling_thread,
		       GObject *channel,
		       AgsRecallID *recall_id,
		       gint stage);

  void (*play_audio)(AgsRecyclingThread *recycling_thread,
		     GObject *output, GObject *audio,
		     AgsRecallID *recall_id,
		     gint stage);
};

GType ags_recycling_thread_get_type();

void ags_recycling_thread_play_channel(AgsRecyclingThread *recycling_thread,
				       GObject *channel,
				       AgsRecallID *recall_id,
				       gint stage);

void ags_recycling_thread_play_audio(AgsRecyclingThread *recycling_thread,
				     GObject *output, GObject *audio,
				     AgsRecallID *recall_id,
				     gint stage);

AgsRecyclingThread* ags_recycling_thread_new();

#endif /*__AGS_RECYCLING_THREAD_H__*/
