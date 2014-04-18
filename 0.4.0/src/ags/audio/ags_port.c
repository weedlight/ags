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

#include <ags/audio/ags_port.h>

#include <ags-lib/object/ags_connectable.h>

void ags_port_class_init(AgsPortClass *port_class);
void ags_port_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_port_init(AgsPort *port);
void ags_port_set_property(GObject *gobject,
			   guint prop_id,
			   const GValue *value,
			   GParamSpec *param_spec);
void ags_port_get_property(GObject *gobject,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *param_spec);
void ags_port_connect(AgsConnectable *connectable);
void ags_port_disconnect(AgsConnectable *connectable);

enum{
  PROP_0,
  PROP_PLUGIN_NAME,
  PROP_SPECIFIER,
  PROP_CONTROL_PORT,
  PROP_PORT_VALUE_IS_POINTER,
  PROP_PORT_VALUE_TYPE,
  PROP_PORT_VALUE_SIZE,
  PROP_PORT_VALUE_LENGTH,
  PROP_PORT_VALUE,
};

static gpointer ags_port_parent_class = NULL;

#include <stdlib.h>
#include <string.h>

GType
ags_port_get_type (void)
{
  static GType ags_type_port = 0;

  if(!ags_type_port){
    static const GTypeInfo ags_port_info = {
      sizeof (AgsPortClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_port_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPort),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_port_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_port_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_port = g_type_register_static(G_TYPE_OBJECT,
					   "AgsPort\0",
					   &ags_port_info,
					   0);

    g_type_add_interface_static(ags_type_port,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_port);
}

void
ags_port_class_init(AgsPortClass *port)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_port_parent_class = g_type_class_peek_parent(port);

  /* GObjectClass */
  gobject = (GObjectClass *) port;

  gobject->set_property = ags_port_set_property;
  gobject->get_property = ags_port_get_property;

  /* properties */
  param_spec = g_param_spec_string("plugin-name\0",
				   "plugin-name of port\0",
				   "The plugin-name this port belongs to\0",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PLUGIN_NAME,
				  param_spec);

  param_spec = g_param_spec_string("specifier\0",
				   "specifier of port\0",
				   "The specifier this port is identified by\0",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SPECIFIER,
				  param_spec);

  param_spec = g_param_spec_string("control-port\0",
				   "control-port of port\0",
				   "The control-port this port is numbered\0",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CONTROL_PORT,
				  param_spec);

  param_spec = g_param_spec_boolean("port-value-is-pointer\0",
				    "port-value-is-pointer indicates if value is a pointer\0",
				    "The port-value-is-pointer indicates if value is a pointer\0",
				    FALSE,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PORT_VALUE_IS_POINTER,
				  param_spec);

  param_spec = g_param_spec_gtype("port-value-type\0",
				  "port-value-type tells you the type of the values\0",
				  "The port-value-type tells you the type of the values\0",
				  G_TYPE_NONE,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PORT_VALUE_TYPE,
				  param_spec);

  param_spec = g_param_spec_uint("port-value-size\0",
				 "port-value-size is the size of a single entry\0",
				 "The port-value-size is the size of a single entry\0",
				 1, 8,
				 sizeof(gdouble),
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PORT_VALUE_SIZE,
				  param_spec);

  param_spec = g_param_spec_uint("port-value-length\0",
				 "port-value-length is the array size\0",
				 "The port-value-length is the array size\0",
				 0, 65535,
				 1,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PORT_VALUE_LENGTH,
				  param_spec);
}

void
ags_port_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->add_to_registry = NULL;
  connectable->remove_from_registry = NULL;

  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_port_connect;
  connectable->disconnect = ags_port_disconnect;
}

void
ags_port_init(AgsPort *port)
{
  pthread_mutexattr_t mutexattr;

  port->plugin_name = NULL;
  port->specifier = NULL;

  port->control_port = NULL;

  port->port_value_is_pointer = FALSE;
  port->port_value_type = G_TYPE_DOUBLE;

  port->port_value_size = sizeof(gdouble);
  port->port_value_length = 1;

  pthread_mutexattr_init(&mutexattr);
  pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

  pthread_mutex_init(&(port->mutex), &mutexattr);
}

void
ags_port_set_property(GObject *gobject,
		      guint prop_id,
		      const GValue *value,
		      GParamSpec *param_spec)
{
  AgsPort *port;

  port = AGS_PORT(gobject);

  switch(prop_id){
  case PROP_PLUGIN_NAME:
    {
      gchar *plugin_name;

      plugin_name = (gchar *) g_value_get_string(value);

      if(port->plugin_name == plugin_name){
	return;
      }
      
      if(port->plugin_name != NULL){
	g_free(port->plugin_name);
      }

      port->plugin_name = g_strdup(plugin_name);
    }
    break;
  case PROP_SPECIFIER:
    {
      gchar *specifier;

      specifier = (gchar *) g_value_get_string(value);

      if(port->specifier == specifier){
	return;
      }

      if(port->specifier != NULL){
	g_free(port->specifier);
      }

      port->specifier = g_strdup(specifier);
    }
    break;
  case PROP_CONTROL_PORT:
    {
      gchar *control_port;

      control_port = (gchar *) g_value_get_string(value);
      
      if(port->control_port == control_port){
	return;
      }

      if(port->control_port != NULL){
	g_free(port->control_port);
      }

      port->control_port = g_strdup(control_port);
    }
    break;
  case PROP_PORT_VALUE_IS_POINTER:
    {
      port->port_value_is_pointer = g_value_get_boolean(value);
    }
    break;
  case PROP_PORT_VALUE_TYPE:
    {
      port->port_value_type = g_value_get_gtype(value);
    }
    break;
  case PROP_PORT_VALUE_SIZE:
    {
      port->port_value_size = g_value_get_uint(value);
    }
    break;
  case PROP_PORT_VALUE_LENGTH:
    {
      port->port_value_length = g_value_get_uint(value);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_port_get_property(GObject *gobject,
		      guint prop_id,
		      GValue *value,
		      GParamSpec *param_spec)
{
  AgsPort *port;

  port = AGS_PORT(gobject);

  switch(prop_id){
  case PROP_PLUGIN_NAME:
    g_value_set_string(value, port->plugin_name);
    break;
  case PROP_SPECIFIER:
    g_value_set_string(value, port->specifier);
    break;
  case PROP_CONTROL_PORT:
    g_value_set_string(value, port->control_port);
    break;
  case PROP_PORT_VALUE_IS_POINTER:
    g_value_set_boolean(value, port->port_value_is_pointer);
    break;
  case PROP_PORT_VALUE_TYPE:
    g_value_set_gtype(value, port->port_value_type);
    break;
  case PROP_PORT_VALUE_SIZE:
    g_value_set_uint(value, port->port_value_size);
    break;
  case PROP_PORT_VALUE_LENGTH:
    g_value_set_uint(value, port->port_value_length);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_port_connect(AgsConnectable *connectable)
{
  AgsPort *port;

  port = AGS_PORT(connectable);
}

void
ags_port_disconnect(AgsConnectable *connectable)
{
  AgsPort *port;

  port = AGS_PORT(connectable);
}

void
ags_port_safe_read(AgsPort *port, GValue *value)
{
  guint overall_size;
  gpointer data;

  overall_size = port->port_value_length * port->port_value_size;

  pthread_mutex_lock(&(port->mutex));

  if(!port->port_value_is_pointer){
    if(port->port_value_type == G_TYPE_BOOLEAN){
      g_value_set_boolean(value, port->port_value.ags_port_boolean);
    }else if(port->port_value_type == G_TYPE_INT){
      g_value_set_int(value, port->port_value.ags_port_int);
    }else if(port->port_value_type == G_TYPE_UINT){
      g_value_set_uint(value, port->port_value.ags_port_uint);
    }else if(port->port_value_type == G_TYPE_DOUBLE){
      g_value_set_double(value, port->port_value.ags_port_double);
    }
  }else{
    data = (gpointer) malloc(overall_size);

    if(port->port_value_type == G_TYPE_BOOLEAN){
      memcpy(data, port->port_value.ags_port_boolean_ptr, overall_size);
    }else if(port->port_value_type == G_TYPE_INT){
      memcpy(data, port->port_value.ags_port_int_ptr, overall_size);
    }else if(port->port_value_type == G_TYPE_UINT){
      memcpy(data, port->port_value.ags_port_uint_ptr, overall_size);
    }else if(port->port_value_type == G_TYPE_DOUBLE){
      memcpy(data, port->port_value.ags_port_double_ptr, overall_size);
    }else if(port->port_value_type == G_TYPE_POINTER){
      g_warning("ags_port_safe_read: not supported data type %s\0", g_type_name(port->port_value_type));
    }else if(port->port_value_type == G_TYPE_OBJECT){
      g_warning("ags_port_safe_read: not supported data type %s\0", g_type_name(port->port_value_type));
    }

    g_value_set_pointer(value, data);
  }

  pthread_mutex_unlock(&(port->mutex));
}

void
ags_port_safe_write(AgsPort *port, GValue *value)
{
  guint overall_size;
  gpointer data;

  overall_size = port->port_value_length * port->port_value_size;

  pthread_mutex_lock(&(port->mutex));

  if(!port->port_value_is_pointer){
    if(port->port_value_type == G_TYPE_BOOLEAN){
      port->port_value.ags_port_boolean = g_value_get_boolean(value);
    }else if(port->port_value_type == G_TYPE_INT){
      port->port_value.ags_port_int = g_value_get_int(value);
    }else if(port->port_value_type == G_TYPE_UINT){
      port->port_value.ags_port_uint = g_value_get_uint(value);
    }else if(port->port_value_type == G_TYPE_DOUBLE){
      port->port_value.ags_port_double = g_value_get_double(value);
    }
  }else{
    data = g_value_get_pointer(value);

    if(port->port_value_type == G_TYPE_BOOLEAN){
      memcpy(port->port_value.ags_port_boolean_ptr, data, overall_size);
    }else if(port->port_value_type == G_TYPE_INT){
      memcpy(port->port_value.ags_port_int_ptr, data, overall_size);
    }else if(port->port_value_type == G_TYPE_UINT){
      memcpy(port->port_value.ags_port_uint_ptr, data, overall_size);
    }else if(port->port_value_type == G_TYPE_DOUBLE){
      memcpy(port->port_value.ags_port_double_ptr, data, overall_size);
    }else if(port->port_value_type == G_TYPE_POINTER){
      g_warning("ags_port_safe_write: not supported data type %s\0", g_type_name(port->port_value_type));
    }else if(port->port_value_type == G_TYPE_OBJECT){
      g_warning("ags_port_safe_write: not supported data type %s\0", g_type_name(port->port_value_type));
    }
  }

  pthread_mutex_unlock(&(port->mutex));
}

void
ags_port_safe_get_property(AgsPort *port, gchar *property_name, GValue *value)
{
  pthread_mutex_lock(&(port->mutex));

  g_object_get_property(port->port_value.ags_port_object,
			property_name,
			value);

  pthread_mutex_unlock(&(port->mutex));
}

void
ags_port_safe_set_property(AgsPort *port, gchar *property_name, GValue *value)
{
  pthread_mutex_lock(&(port->mutex));

  g_object_set_property(port->port_value.ags_port_object,
			property_name,
			value);

  pthread_mutex_unlock(&(port->mutex));
}

AgsPort*
ags_port_new()
{
  AgsPort *port;

  port = (AgsPort *) g_object_new(AGS_TYPE_PORT,
				  NULL);

  return(port);
}
