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

#ifndef __AGS_GUI_TASK_THREAD_H__
#define __AGS_GUI_TASK_THREAD_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/thread/ags_task_thread.h>

#define AGS_TYPE_GUI_TASK_THREAD                (ags_gui_task_thread_get_type())
#define AGS_GUI_TASK_THREAD(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_GUI_TASK_THREAD, AgsGuiTaskThread))
#define AGS_GUI_TASK_THREAD_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_GUI_TASK_THREAD, AgsGuiTaskThread))
#define AGS_IS_GUI_TASK_THREAD(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_GUI_TASK_THREAD))
#define AGS_IS_GUI_TASK_THREAD_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_GUI_TASK_THREAD))
#define AGS_GUI_TASK_THREAD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_GUI_TASK_THREAD, AgsGuiTaskThreadClass))

typedef struct _AgsGuiTaskThread AgsGuiTaskThread;
typedef struct _AgsGuiTaskThreadClass AgsGuiTaskThreadClass;

struct _AgsGuiTaskThread
{
  AgsTaskThread task_thread;
};

struct _AgsGuiTaskThreadClass
{
  AgsTaskThreadClass task_thread;
};


GType ags_gui_task_thread_get_type();

AgsGuiTaskThread* ags_gui_task_thread_new(GObject *devout);

#endif /*__AGS_GUI_TASK_THREAD_H__*/
