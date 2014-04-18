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

#ifndef __AGS_FILE_CLUSTER_H__
#define __AGS_FILE_CLUSTER_H__

#include <glib.h>
#include <glib-object.h>

#include <libxml/tree.h>

/* cluster */
void ags_file_cluster_read_cluster(AgsFile *file, xmlNode *node, AgsCluster **cluster);
xmlNode* ags_file_cluster_write_cluster(AgsFile *file, xmlNode *parent, AgsCluster *cluster);

#endif /*__AGS_FILE_CLUSTER_H__*/
