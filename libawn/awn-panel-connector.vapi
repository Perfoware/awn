/* awn-panel-connector.vapi generated by valac, do not modify. */

[CCode (cprefix = "Awn", lower_case_cprefix = "awn_")]
namespace Awn {
	[CCode (cheader_filename = "awn-panel-connector.h")]
	public interface PanelConnector : GLib.Object {
		public int64 bus_docklet_request (int min_size, bool shrink, bool expand);
		public void connect (DBus.Connection con, out DBus.Object proxy) throws DBus.Error;
		public uint inhibit_autohide (string reason);
		public virtual void on_proxy_destroyed ();
		public abstract void property_changed (string prop_name, GLib.Value value);
		public void uninhibit_autohide (uint cookie);
		public abstract int max_size { get; set; }
		public abstract int offset { get; set; }
		public abstract float offset_modifier { get; set; }
		public abstract int panel_id { get; set construct; }
		public abstract DBus.Object panel_proxy { get; }
		public abstract int64 panel_xid { get; }
		public abstract Awn.PathType path_type { get; set; }
		public abstract Gtk.PositionType position { get; set; }
		public abstract int size { get; set; }
		public abstract string uid { get; set construct; }
		public signal void applet_deleted ();
	}
}
