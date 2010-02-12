/*
 * Copyright (C) 2010 Michal Hruby <michal.mhr@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 *
 * Authored by Michal Hruby <michal.mhr@gmail.com>
 */

using DBus;

[DBus (name="org.freedesktop.DockManager")]
public interface DockManagerDBusInterface: GLib.Object
{
  public abstract string[] get_capabilities () throws DBus.Error;
  public abstract ObjectPath[] get_items () throws DBus.Error;
  public abstract ObjectPath[] get_items_by_name (string name) throws DBus.Error;
  public abstract ObjectPath[] get_items_by_desktop_file (string desktop_file) throws DBus.Error;
  public abstract ObjectPath[] get_items_by_pid (int pid) throws DBus.Error;
  public abstract ObjectPath get_item_by_xid (int64 xid) throws DBus.Error;

  public signal void item_added (ObjectPath path);
  public signal void item_removed (ObjectPath path);
}

[DBus (name="org.freedesktop.DockItem")]
public interface DockItemDBusInterface: GLib.Object
{
  public abstract int add_menu_item (HashTable<string, Value?> menu_hints) throws DBus.Error;
  public abstract void remove_menu_item (int id) throws DBus.Error;
  public abstract void update_dock_item (HashTable<string, Value?> hints) throws DBus.Error;

  public abstract string desktop_file { owned get; }

  public signal void menu_item_activated (int id);
}

public class TaskManagerDispatcher: GLib.Object, DockManagerDBusInterface
{
  public Task.Manager manager { get; construct; }

  public TaskManagerDispatcher (Task.Manager manager)
  {
    GLib.Object (manager: manager);

    /*
    manager.some_signal.connect ( (m, i) =>
    {
      this.item_added (i);
    });
    */

    var conn = Bus.get (BusType.SESSION);
    string obj_path = "/org/freedesktop/DockManager";
    conn.register_object (obj_path, this);
  }

  private static ObjectPath[] list_to_object_path_array (SList<Task.Icon> list)
  {
    ObjectPath[] result = new ObjectPath[list.length ()];
    int i = 0;
    foreach (unowned Task.Icon icon in list)
    {
      unowned TaskIconDispatcher dispatcher;
      dispatcher = icon.get_dbus_dispatcher () as TaskIconDispatcher;
      result[i++] = new ObjectPath (dispatcher.object_path);
    }

    return result;
  }

  public string[] get_capabilities () throws DBus.Error
  {
    string[] capabilities = {"x-awn-0.4"};
    return capabilities;
  }

  public ObjectPath[] get_items () throws DBus.Error
  {
    return list_to_object_path_array (this.manager.get_icons ());
  }

  public ObjectPath[] get_items_by_name (string name) throws DBus.Error
  {
    return null;
  }

  public ObjectPath[] get_items_by_desktop_file (string desktop_file) throws DBus.Error
  {
    unowned SList<unowned Task.Icon> icons = this.manager.get_icons ();
    SList<unowned Task.Icon> matches = new SList<unowned Task.Icon> ();

    foreach (unowned Task.Icon icon in icons)
    {
      unowned Task.Launcher? launcher = icon.get_launcher () as Task.Launcher;
      if (launcher != null)
      {
        if (launcher.get_desktop_path ().has_suffix (desktop_file))
        {
          matches.append (icon);
        }
      }
    }

    return list_to_object_path_array (matches);
  }

  public ObjectPath[] get_items_by_pid (int pid) throws DBus.Error
  {
    unowned SList<unowned Task.Icon> icons = this.manager.get_icons ();
    SList<unowned Task.Icon> matches = new SList<unowned Task.Icon> ();

    foreach (unowned Task.Icon icon in icons)
    {
      foreach (unowned Task.Item item in icon.get_items ())
      {
        if (item is Task.Window)
        {
          unowned Task.Window window = item as Task.Window;
          if (window.get_pid () == pid)
          {
            matches.append (icon);
            break;
          }
        }
      }
    }

    return list_to_object_path_array (matches);
  }

  public ObjectPath get_item_by_xid (int64 xid) throws DBus.Error
  {
    unowned Task.Icon? icon = this.manager.get_icon_by_xid (xid);

    if (icon != null)
    {
      unowned TaskIconDispatcher dispatcher;
      dispatcher = icon.get_dbus_dispatcher () as TaskIconDispatcher;

      return new ObjectPath (dispatcher.object_path);
    }

    return null;
  }
}

public class TaskIconDispatcher: GLib.Object, DockItemDBusInterface
{
  private unowned Task.Icon icon;

  public string object_path { get; set; }

  public string desktop_file
  {
    owned get
    {
      string path = "";
      unowned Task.Launcher? launcher;
      launcher = this.icon.get_launcher () as Task.Launcher;
      if (launcher != null)
      {
        path = launcher.get_desktop_path ();
      }
      return path;
    }
  }

  // FIXME: if TaskManagerDispatcher had only get_default constructor without
  //   parameters, we could just emit its item_added signal in our constructor
  public TaskIconDispatcher (Task.Icon icon)
  {
    this.icon = icon;

    var conn = Bus.get (BusType.SESSION);
    this.object_path = "/org/freedesktop/DockItem/%p".printf (icon);
    conn.register_object (this.object_path, this);
  }

  public int add_menu_item (HashTable<string, Value?> menu_hints) throws DBus.Error
  {
    debug ("called add_menu_item method");
    HashTableIter<string, Value?> iter =
      HashTableIter<string, Value?>(menu_hints);
    unowned string key;
    unowned Value? value;
    while (iter.next (out key, out value))
    {
      debug ( "%s: %s", key, value.strdup_contents ());
    }

    int id = 12345;
    return id;
  }

  public void remove_menu_item (int id) throws DBus.Error
  {
  }

  public void update_dock_item (HashTable<string, Value?> hints) throws DBus.Error
  {
    HashTableIter<string, Value?> iter = HashTableIter<string, Value?> (hints);
    unowned string key;
    unowned Value? value;
    while (iter.next (out key, out value))
    {
      if (key == "message")
      {
        if (!value.holds (typeof (string)))
        {
          warning ("Invalid type for property \"%s\"", key);
          continue;
        }

        unowned SList<unowned Task.Item> items = this.icon.get_items ();
        foreach (unowned Task.Item item in items)
        {
          if (item is Task.Launcher) continue;

          if (item.text_overlay == null)
          {
            item.text_overlay = new Awn.OverlayText ();
            item.text_overlay.font_sizing = 15;

            unowned Awn.Overlayable over;
            over = item.get_image_widget () as Awn.Overlayable;
            over.add_overlay (item.text_overlay);
          }

          unowned string text = (string)value;
          item.text_overlay.active = text.size () > 0;
          item.text_overlay.text = text;

          // this refreshes the overlays on TaskIcon
          item.set_task_icon (item.get_task_icon ());
        }
      }
      else if (key == "progress")
      {
        if (!value.holds (typeof (int)))
        {
          warning ("Invalid type for property \"%s\"", key);
          continue;
        }

        unowned SList<unowned Task.Item> items = this.icon.get_items ();
        foreach (unowned Task.Item item in items)
        {
          if (item is Task.Launcher) continue;

          if (item.progress_overlay == null)
          {
            item.progress_overlay = new Awn.OverlayProgressCircle ();
            
            unowned Awn.Overlayable over;
            over = item.get_image_widget () as Awn.Overlayable;
            over.add_overlay (item.progress_overlay);
          }

          int percent = (int)value;
          item.progress_overlay.active = percent != -1;

          if (percent != -1)
          {
            item.progress_overlay.percent_complete = percent;
          }

          // this refreshes the overlays on TaskIcon
          item.set_task_icon (item.get_task_icon ());
        }
      }
      else if (key == "icon-file")
      {
        if (!value.holds (typeof (string)))
        {
          warning ("Invalid type for property \"%s\"", key);
          continue;
        }

        unowned SList<unowned Task.Item> items = this.icon.get_items ();
        foreach (unowned Task.Item item in items)
        {
          if (item is Task.Launcher) continue;

          if (item.icon_overlay == null)
          {
            item.icon_overlay = new Awn.OverlayPixbufFile (null);
            item.icon_overlay.use_source_op = true;
            item.icon_overlay.scale = 1.0;

            unowned Awn.Overlayable over;
            over = item.get_image_widget () as Awn.Overlayable;
            over.add_overlay (item.progress_overlay);
          }

          unowned string filename = (string)value;
          item.icon_overlay.active = filename.size () > 0;

          if (filename.size () > 0)
          {
            item.icon_overlay.file_name = filename;
          }
        }
      }
      else
      {
        debug ("Unsupported key: \"%s\"", key);
      }
      /*else if (key == "attention")
      {
      }
      else if (key == "waiting")
      {
      }*/
    }
  }
}

