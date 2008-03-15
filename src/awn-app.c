/*
 * Copyright (C) 2008 Neil J. Patel
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authored by Neil Jagdish Patel <njpatel@gmail.com>
 *
 */

/*
 * AwnApp is the 'controller' for the panel. It will launch the panel with the
 * correct settings.
 *
 * In the future, AwnApp will be responsible for launching each Awn panel 
 * separately, and making dealing with the configuration backend for each
 * panel.
 */

#include <glib.h>

#include <stdio.h>
#include <string.h>

#include <libawn/awn-config-client.h>

#include "awn-app.h"

G_DEFINE_TYPE (AwnApp, awn_app, G_TYPE_OBJECT)

#define AWN_APP_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        AWN_TYPE_APP, AwnAppPrivate))

struct _AwnAppPrivate
{
  gint null;
};

/* GObject functions */
static void
awn_app_dispose (GObject *object)
{
  G_OBJECT_CLASS (awn_app_parent_class)->dispose (object);
}

static void
awn_app_finalize (GObject *app)
{
  AwnAppPrivate *priv;
  
  g_return_if_fail (AWN_IS_APP (app));
  priv = AWN_APP (app)->priv;

  G_OBJECT_CLASS (awn_app_parent_class)->finalize (app);
}


static void
awn_app_class_init (AwnAppClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = awn_app_finalize;
  obj_class->dispose = awn_app_dispose;

  g_type_class_add_private (obj_class, sizeof (AwnAppPrivate)); 
}

static void
awn_app_init (AwnApp *app)
{
  AwnAppPrivate *priv;
    
  priv = app->priv = AWN_APP_GET_PRIVATE (app);
}

AwnApp*
awn_app_get_default (void)
{
  static AwnApp *app = NULL;
  
  if (app == NULL)
    app = g_object_new (AWN_TYPE_APP, 
                         NULL);

  return app;
}
