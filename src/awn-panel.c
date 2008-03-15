/*
 *  Copyright (C) 2008 Neil Jagdish Patel <njpatel@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 *
 *  Author : Neil Jagdish Patel <njpatel@gmail.com>
 *
 */

#include "config.h"

#include <gdk/gdkx.h>

#include <X11/Xlib.h>
#include <X11/extensions/shape.h>

#include "awn-panel.h"

#if 0
#include "awn-background.h"
#include "awn-monitor.h"
#include "awn-x.h"
#endif

G_DEFINE_TYPE (AwnPanel, awn_panel, GTK_TYPE_WINDOW) 

#define AWN_PANEL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE (obj, \
  AWN_TYPE_PANEL, AwnPanelPrivate))

struct _AwnPanelPrivate
{
  AwnConfigClient *client;

  GdkScreen *screen;

  gboolean composited;

  GtkWidget *eventbox;

  gint old_width;
  gint old_height;
};

enum 
{
  PROP_0,

  PROP_COMPOSITED
};

static const GtkTargetEntry drop_types[] = 
{
	{ "STRING", 0, 0 }
};
static const gint n_drop_types = G_N_ELEMENTS (drop_types);

/* Forwards */
static void on_composite_changed (GdkScreen *screen, AwnPanel *window);

/* Public */

/*
 * Prepare the window for the current desktop composited setting
 */
void
awn_panel_set_composited (AwnPanel *window, gboolean composited)
{
  AwnPanelPrivate *priv;

  g_return_if_fail (AWN_IS_PANEL (window));
  priv = window->priv;

  if (priv->composited == composited)
    return;
  priv->composited = composited;
  
  if (GTK_IS_WIDGET (priv->eventbox))
    gdk_window_set_composited (priv->eventbox->window, composited);

  gtk_widget_queue_draw (GTK_WIDGET (window));
}

static void
awn_panel_add (GtkContainer *window, GtkWidget *widget)
{
  AwnPanelPrivate *priv;

  g_return_if_fail (AWN_IS_PANEL (window));
  priv = AWN_PANEL_GET_PRIVATE (window);

  priv->eventbox = gtk_event_box_new ();
  GTK_CONTAINER_CLASS (awn_panel_parent_class)->add (GTK_CONTAINER (window),
                                                      priv->eventbox);

  gtk_container_add (GTK_CONTAINER (priv->eventbox), widget);

  gtk_widget_show (priv->eventbox);
  gdk_window_set_composited (priv->eventbox->window, priv->composited);
}
/* Window stuff */
static gboolean
awn_panel_expose (GtkWidget *widget, GdkEventExpose *event)
{
  AwnPanelPrivate *priv;
  gint width = 0, height = 0;
  cairo_t *cr;
  GtkWidget *child;

  g_return_val_if_fail (AWN_IS_PANEL (widget), FALSE);
  priv = AWN_PANEL (widget)->priv;

  if (!GDK_IS_DRAWABLE (widget->window))
  {
    g_debug ("!GDK_IS_DRAWABLE (widget->window) failed");
    return FALSE;
  }

  /* Get our ctx */
  cr = gdk_cairo_create (widget->window);
  if (!cr)
  {
    g_debug ("Unable to create cairo context\n");
    return FALSE;
  }
  gtk_window_get_size (GTK_WINDOW (widget), &width, &height);

  /*awn_background_render (priv->bg, GTK_WIDGET (widget),
                         cr, width, height, priv->composited);
                         */
  
  child = gtk_bin_get_child (GTK_BIN (widget));
  if (priv->composited)
  {
    GdkRegion *region;
    
    gdk_cairo_set_source_pixmap (cr, child->window,
                                 child->allocation.x, 
                                 child->allocation.y);

    region = gdk_region_rectangle (&child->allocation);
    gdk_region_intersect (region, event->region);
    gdk_cairo_region (cr, region);
    cairo_clip (cr);
    
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_paint (cr);
  }

  cairo_destroy (cr);

  gtk_container_propagate_expose (GTK_CONTAINER (widget),
                                  child,
                                  event);

  if (priv->old_width != width || priv->old_height != height)
    ;//update_position (AWN_PANEL (widget));

  priv->old_width = width;
  priv->old_height = height;

  return TRUE;
}
/*
 * When the screen changes, its usually due to the fact that
 */
static void
on_window_screen_changed (GtkWidget *widget,
                          GdkScreen *old_screen,
                          AwnPanel *window)
{
  AwnPanelPrivate *priv;
  GdkScreen *screen;
  GdkColormap *colormap = NULL;
  gboolean composited = TRUE;

  g_return_if_fail (AWN_IS_PANEL (window));
  priv = window->priv;

  screen = priv->screen = gtk_widget_get_screen (GTK_WIDGET (window));
  g_signal_connect (screen, "composited-changed",
                    G_CALLBACK (on_composite_changed), window);

  if (gdk_screen_is_composited (screen))
  {
    colormap = gdk_screen_get_rgba_colormap (screen);
  }
  else
  {
    colormap = gdk_screen_get_rgb_colormap (screen);
    composited = FALSE;
  }
  gtk_widget_set_colormap (GTK_WIDGET (window), colormap);
  g_debug ("Screen is composited: %s\n", 
            gdk_screen_is_composited (screen) ? "true": "false");
}

static void
on_composite_changed (GdkScreen *screen, AwnPanel *window)
{
  g_object_set (window, "composited", gdk_screen_is_composited (screen), NULL);
  g_debug ("Screen is composited: %s, respawning\n", 
            gdk_screen_is_composited (screen) ? "true": "false");
}

/*
 * We set the shape of the window, so when in composited mode, we dont't 
 * receive events in the blank space above the main window
 * FIXME: We need to include the icon_offset
 */
static void
awn_panel_update_input_shape (AwnPanel *window, gint width, gint height)
{
  AwnPanelPrivate *priv;
  GdkBitmap *shaped_bitmap;
  cairo_t *cr;

  g_return_if_fail (AWN_IS_PANEL (window));
  priv = window->priv;
  
  shaped_bitmap = (GdkBitmap*)gdk_pixmap_new (NULL, width, height, 1);

  if (!shaped_bitmap)
    return;

  cr = gdk_cairo_create (shaped_bitmap);
  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  cairo_set_source_rgb (cr, 1.0f, 1.0f, 1.0f);

  if (priv->composited)
    cairo_rectangle (cr, 0, height/2, width, height/2);
  else
    cairo_rectangle (cr, 0, 0, width, height);
  
  cairo_fill (cr);
  cairo_destroy (cr);

  gtk_widget_input_shape_combine_mask (GTK_WIDGET (window), NULL, 0, 0);
  gtk_widget_input_shape_combine_mask (GTK_WIDGET (window), shaped_bitmap,0,0);

  if (shaped_bitmap)
    g_object_unref (shaped_bitmap);
}

/*
 * We get a configure event when the windows size changes. This is a good as
 * a time as any to update the input shape of the window.
 */
static gboolean
on_window_configure (GtkWidget         *widget,
                      GdkEventConfigure *event,
                      AwnPanel         *window)
{
  awn_panel_update_input_shape (window, event->width, event->height);
  return FALSE;
}

/* GObject stuff */
static void
awn_panel_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AwnPanelPrivate *priv;

  g_return_if_fail (AWN_IS_PANEL (object));
  priv = AWN_PANEL (object)->priv;

  switch (prop_id)
  {
    case PROP_COMPOSITED:
      g_value_set_boolean (value, priv->composited);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
awn_panel_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AwnPanelPrivate *priv;

  g_return_if_fail (AWN_IS_PANEL (object));
  priv = AWN_PANEL (object)->priv;

  switch (prop_id)
  {
    case PROP_COMPOSITED:
      awn_panel_set_composited (AWN_PANEL (object), 
                                 g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}


static void
awn_panel_dispose (GObject *object)
{
  G_OBJECT_CLASS (awn_panel_parent_class)->dispose (object);
}

static void
awn_panel_finalize (GObject *object)
{
  G_OBJECT_CLASS (awn_panel_parent_class)->finalize (object);
}

static void
awn_panel_class_init (AwnPanelClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *wid_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *cont_class = GTK_CONTAINER_CLASS (klass);

  obj_class->finalize = awn_panel_finalize;
  obj_class->dispose = awn_panel_dispose;
  obj_class->get_property = awn_panel_get_property;
  obj_class->set_property = awn_panel_set_property;

  cont_class->add = awn_panel_add;
  wid_class->expose_event = awn_panel_expose;

  /* Add properties to the class */
  g_object_class_install_property (obj_class,
    PROP_COMPOSITED,
    g_param_spec_boolean ("composited",
                          "Composited",
                          "The window is composited",
                          TRUE,
                          G_PARAM_CONSTRUCT|G_PARAM_READWRITE));

  g_type_class_add_private (obj_class, sizeof (AwnPanelPrivate));
}

static void
awn_panel_init (AwnPanel *window)
{
  AwnPanelPrivate *priv;

  priv = window->priv = AWN_PANEL_GET_PRIVATE (window);

  priv->screen = gtk_widget_get_screen (GTK_WIDGET (window));
  priv->composited = gdk_screen_is_composited (priv->screen);

  /* Make sure we react to screen-changed & configure events */
  g_signal_connect (window, "configure-event",
                    G_CALLBACK (on_window_configure), window);
  g_signal_connect (window, "screen-changed",
                    G_CALLBACK (on_window_screen_changed), window);
  
  /* Set up the drag destination */
  gtk_widget_add_events (GTK_WIDGET (window), GDK_ALL_EVENTS_MASK);
  gtk_drag_dest_set (GTK_WIDGET (window), 
                     GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP,
                     drop_types, n_drop_types,
                     GDK_ACTION_MOVE | GDK_ACTION_COPY);
}

GtkWidget *
awn_panel_new (void)
{
  GtkWidget *window;

  window = g_object_new (AWN_TYPE_PANEL,
                         "type", GTK_WINDOW_TOPLEVEL,
                         "type-hint", GDK_WINDOW_TYPE_HINT_DOCK,
                         NULL);
  
  //gtk_widget_realize (window);
  on_window_screen_changed (window, NULL, AWN_PANEL (window));

  return window;
}

GtkWidget *
awn_panel_new_from_config (AwnConfigClient *client)
{
  GtkWidget *window = awn_panel_new ();

  return window;
}


