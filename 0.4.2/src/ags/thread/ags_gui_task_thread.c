/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2014 Joël Krähemann
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

#include <ags/thread/ags_gui_task_thread.h>
#include <ags/thread/ags_gui_thread.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/main.h>

#include <math.h>

void ags_gui_task_thread_class_init(AgsGuiTaskThreadClass *gui_task_thread);
void ags_gui_task_thread_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_gui_task_thread_init(AgsGuiTaskThread *gui_task_thread);
void ags_gui_task_thread_connect(AgsConnectable *connectable);
void ags_gui_task_thread_disconnect(AgsConnectable *connectable);
void ags_gui_task_thread_finalize(GObject *gobject);

void ags_gui_task_thread_start(AgsThread *thread);
void ags_gui_task_thread_run(AgsThread *thread);

static gpointer ags_gui_task_thread_parent_class = NULL;
static AgsConnectableInterface *ags_gui_task_thread_parent_connectable_interface;

GType
ags_gui_task_thread_get_type()
{
  static GType ags_type_gui_task_thread = 0;

  if(!ags_type_gui_task_thread){
    static const GTypeInfo ags_gui_task_thread_info = {
      sizeof (AgsGuiTaskThreadClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_gui_task_thread_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsGuiTaskThread),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_gui_task_thread_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_gui_task_thread_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_gui_task_thread = g_type_register_static(AGS_TYPE_TASK_THREAD,
						      "AgsGuiTaskThread\0",
						      &ags_gui_task_thread_info,
						      0);
    
    g_type_add_interface_static(ags_type_gui_task_thread,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_gui_task_thread);
}

void
ags_gui_task_thread_class_init(AgsGuiTaskThreadClass *gui_task_thread)
{
  GObjectClass *gobject;
  AgsThreadClass *thread;

  ags_gui_task_thread_parent_class = g_type_class_peek_parent(gui_task_thread);

  /* GObject */
  gobject = (GObjectClass *) gui_task_thread;

  gobject->finalize = ags_gui_task_thread_finalize;

  /* AgsThread */
  thread = (AgsThreadClass *) gui_task_thread;

  thread->start = ags_gui_task_thread_start;
  thread->run = ags_gui_task_thread_run;
}

void
ags_gui_task_thread_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_gui_task_thread_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_gui_task_thread_connect;
  connectable->disconnect = ags_gui_task_thread_disconnect;
}

void
ags_gui_task_thread_init(AgsGuiTaskThread *gui_task_thread)
{
  /* empty */
}

void
ags_gui_task_thread_connect(AgsConnectable *connectable)
{
  AgsGuiTaskThread *gui_task_thread;

  ags_gui_task_thread_parent_connectable_interface->connect(connectable);
}

void
ags_gui_task_thread_disconnect(AgsConnectable *connectable)
{
  ags_gui_task_thread_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_gui_task_thread_finalize(GObject *gobject)
{
  /*  */
  G_OBJECT_CLASS(ags_gui_task_thread_parent_class)->finalize(gobject);
}

void
ags_gui_task_thread_start(AgsThread *thread)
{
  AgsGuiTaskThread *gui_task_thread;

  gui_task_thread = AGS_GUI_TASK_THREAD(thread);

  if((AGS_THREAD_SINGLE_LOOP & (g_atomic_int_get(&(thread->flags)))) == 0){
    AGS_THREAD_CLASS(ags_gui_task_thread_parent_class)->start(thread);
  }
}

void
ags_gui_task_thread_run(AgsThread *thread)
{
  AgsGuiThread *gui_thread;
  GMainContext *main_context;

  gui_thread = AGS_AUDIO_LOOP(AGS_MAIN(AGS_DEVOUT(thread->devout)->ags_main)->main_loop)->gui_thread;
  main_context = g_main_context_default();

  if(!g_main_context_acquire(main_context)){
    gboolean got_ownership = FALSE;

    while(!got_ownership){
      got_ownership = g_main_context_wait(main_context,
					  &(gui_thread->cond),
					  &(gui_thread->mutex));
    }
  }

  gdk_threads_enter();
  gdk_threads_leave();

  g_main_context_iteration(main_context, FALSE);

  AGS_THREAD_CLASS(ags_gui_task_thread_parent_class)->run(thread);

  g_main_context_release(main_context);
}
 
AgsGuiTaskThread*
ags_gui_task_thread_new(GObject *devout)
{
  AgsGuiTaskThread *gui_task_thread;

  gui_task_thread = (AgsGuiTaskThread *) g_object_new(AGS_TYPE_GUI_TASK_THREAD,
						      "devout\0", devout,
						      NULL);


  return(gui_task_thread);
}
