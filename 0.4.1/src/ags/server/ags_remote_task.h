#ifndef __AGS_REMOTE_TASK_H__
#define __AGS_REMOTE_TASK_H__

#include <glib.h>
#include <glib-object.h>

#ifdef AGS_WITH_XMLRPC_C
#include <xmlrpc.h>
#include <xmlrpc_server.h>
#endif

#define AGS_TYPE_REMOTE_TASK                (ags_remote_task_get_type())
#define AGS_REMOTE_TASK(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_REMOTE_TASK, AgsRemoteTask))
#define AGS_REMOTE_TASK_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_REMOTE_TASK, AgsRemoteTaskClass))
#define AGS_IS_REMOTE_TASK(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_REMOTE_TASK))
#define AGS_IS_REMOTE_TASK_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_REMOTE_TASK))
#define AGS_REMOTE_TASK_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_REMOTE_TASK, AgsRemoteTaskClass))

typedef struct _AgsRemoteTask AgsRemoteTask;
typedef struct _AgsRemoteTaskClass AgsRemoteTaskClass;

struct _AgsRemoteTask
{
  GObject object;

  GObject *server;
};

struct _AgsRemoteTaskClass
{
  GObjectClass object;
};

GType ags_remote_task_get_type();


#ifdef AGS_WITH_XMLRPC_C
xmlrpc_value* ags_remote_task_launch(xmlrpc_env *env,
				     xmlrpc_value *param_array,
				     void *server_info);

xmlrpc_value* ags_remote_task_launch_timed(xmlrpc_env *env,
					   xmlrpc_value *param_array,
					   void *server_info);
#endif

AgsRemoteTask* ags_remote_task_new();

#endif /*__AGS_REMOTE_TASK_H__*/
