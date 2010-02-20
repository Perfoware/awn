/*
 * Copyright (C) 2008 Neil Jagdish Patel <njpatel@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 *
 * Authored by Neil Jagdish Patel <njpatel@gmail.com>
 *
 */

#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include <libawn/libawn.h>
#include "task-icon.h"

#define TASK_TYPE_MANAGER (task_manager_get_type ())

#define TASK_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
	TASK_TYPE_MANAGER, TaskManager))

#define TASK_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	TASK_TYPE_MANAGER, TaskManagerClass))

#define TASK_IS_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
	TASK_TYPE_MANAGER))

#define TASK_IS_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
	TASK_TYPE_MANAGER))

#define TASK_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	TASK_TYPE_MANAGER, TaskManagerClass))

typedef struct _TaskManager        TaskManager;
typedef struct _TaskManagerClass   TaskManagerClass;
typedef struct _TaskManagerPrivate TaskManagerPrivate;
 
struct _TaskManager
{
  AwnApplet        parent;	

  TaskManagerPrivate *priv;
};

struct _TaskManagerClass
{
  AwnAppletClass   parent_class;

  void (*grouping_changed) (TaskManager *manager,gboolean grouping);
};

GType       task_manager_get_type (void) G_GNUC_CONST;

AwnApplet * task_manager_new      (const gchar *name,
                                   const gchar *uid, 
                                   gint         panel_id);

gboolean task_manager_get_capabilities (TaskManager *manager,
                                        GStrv *supported_keys,
                                        GError **error);

gboolean task_manager_update (TaskManager *manager,
                              GValue *window,
                              GHashTable *hints, /* mappings from string to GValue */
                              GError **error);

GObject* task_manager_get_dbus_dispatcher (TaskManager *manager);

void task_manager_append_launcher     (TaskManager  *manager, 
                                      const gchar * launcher_path);

void task_manager_remove_task_icon    (TaskManager  *manager, GtkWidget *icon);

void task_manager_add_icon            (TaskManager *manager, TaskIcon * icon);

const GSList * task_manager_get_icons       (TaskManager * manager);
GSList * task_manager_get_icons_by_wmclass  (TaskManager * manager, const gchar * name);
GSList * task_manager_get_icons_by_desktop  (TaskManager * manager,const gchar * desktop);
GSList * task_manager_get_icons_by_pid      (TaskManager * manager, int pid);
const TaskIcon * task_manager_get_icon_by_xid (TaskManager * manager, gint64 xid);
#endif /* _TASK_MANAGER_H_ */

