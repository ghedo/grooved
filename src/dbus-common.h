/*
 * Generated by gdbus-codegen 2.40.0. DO NOT EDIT.
 *
 * The license of this code is the same as for the source it was derived from.
 */

#ifndef __DBUS_COMMON_H__
#define __DBUS_COMMON_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for io.github.ghedo.grooved.Player */

#define GROOVED_TYPE_PLAYER (grooved_player_get_type ())
#define GROOVED_PLAYER(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GROOVED_TYPE_PLAYER, GroovedPlayer))
#define GROOVED_IS_PLAYER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GROOVED_TYPE_PLAYER))
#define GROOVED_PLAYER_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), GROOVED_TYPE_PLAYER, GroovedPlayerIface))

struct _GroovedPlayer;
typedef struct _GroovedPlayer GroovedPlayer;
typedef struct _GroovedPlayerIface GroovedPlayerIface;

struct _GroovedPlayerIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_add_list) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_path);

  gboolean (*handle_add_track) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_path);

  gboolean (*handle_list) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_loop) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_mode);

  gboolean (*handle_next) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_pause) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_play) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_prev) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_quit) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_remove_track) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation,
    gint64 arg_index);

  gboolean (*handle_replaygain) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_mode);

  gboolean (*handle_seek) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation,
    gint64 arg_seconds);

  gboolean (*handle_status) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_stop) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_toggle) (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

  void (*option_changed) (
    GroovedPlayer *object);

  void (*status_changed) (
    GroovedPlayer *object);

  void (*track_added) (
    GroovedPlayer *object);

  void (*track_changed) (
    GroovedPlayer *object);

};

GType grooved_player_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *grooved_player_interface_info (void);
guint grooved_player_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void grooved_player_complete_status (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation,
    const gchar *state,
    const gchar *path,
    gdouble length,
    gdouble position,
    gdouble percent,
    GVariant *metadata,
    const gchar *replaygain,
    const gchar *loop);

void grooved_player_complete_play (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_pause (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_toggle (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_next (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_prev (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_stop (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_seek (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_list (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *files,
    gint64 count,
    gint64 position);

void grooved_player_complete_add_track (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_add_list (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_remove_track (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_replaygain (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_loop (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);

void grooved_player_complete_quit (
    GroovedPlayer *object,
    GDBusMethodInvocation *invocation);



/* D-Bus signal emissions functions: */
void grooved_player_emit_status_changed (
    GroovedPlayer *object);

void grooved_player_emit_track_changed (
    GroovedPlayer *object);

void grooved_player_emit_track_added (
    GroovedPlayer *object);

void grooved_player_emit_option_changed (
    GroovedPlayer *object);



/* D-Bus method calls: */
void grooved_player_call_status (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_status_finish (
    GroovedPlayer *proxy,
    gchar **out_state,
    gchar **out_path,
    gdouble *out_length,
    gdouble *out_position,
    gdouble *out_percent,
    GVariant **out_metadata,
    gchar **out_replaygain,
    gchar **out_loop,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_status_sync (
    GroovedPlayer *proxy,
    gchar **out_state,
    gchar **out_path,
    gdouble *out_length,
    gdouble *out_position,
    gdouble *out_percent,
    GVariant **out_metadata,
    gchar **out_replaygain,
    gchar **out_loop,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_play (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_play_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_play_sync (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_pause (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_pause_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_pause_sync (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_toggle (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_toggle_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_toggle_sync (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_next (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_next_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_next_sync (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_prev (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_prev_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_prev_sync (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_stop (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_stop_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_stop_sync (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_seek (
    GroovedPlayer *proxy,
    gint64 arg_seconds,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_seek_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_seek_sync (
    GroovedPlayer *proxy,
    gint64 arg_seconds,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_list (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_list_finish (
    GroovedPlayer *proxy,
    gchar ***out_files,
    gint64 *out_count,
    gint64 *out_position,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_list_sync (
    GroovedPlayer *proxy,
    gchar ***out_files,
    gint64 *out_count,
    gint64 *out_position,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_add_track (
    GroovedPlayer *proxy,
    const gchar *arg_path,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_add_track_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_add_track_sync (
    GroovedPlayer *proxy,
    const gchar *arg_path,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_add_list (
    GroovedPlayer *proxy,
    const gchar *arg_path,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_add_list_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_add_list_sync (
    GroovedPlayer *proxy,
    const gchar *arg_path,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_remove_track (
    GroovedPlayer *proxy,
    gint64 arg_index,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_remove_track_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_remove_track_sync (
    GroovedPlayer *proxy,
    gint64 arg_index,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_replaygain (
    GroovedPlayer *proxy,
    const gchar *arg_mode,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_replaygain_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_replaygain_sync (
    GroovedPlayer *proxy,
    const gchar *arg_mode,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_loop (
    GroovedPlayer *proxy,
    const gchar *arg_mode,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_loop_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_loop_sync (
    GroovedPlayer *proxy,
    const gchar *arg_mode,
    GCancellable *cancellable,
    GError **error);

void grooved_player_call_quit (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean grooved_player_call_quit_finish (
    GroovedPlayer *proxy,
    GAsyncResult *res,
    GError **error);

gboolean grooved_player_call_quit_sync (
    GroovedPlayer *proxy,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define GROOVED_TYPE_PLAYER_PROXY (grooved_player_proxy_get_type ())
#define GROOVED_PLAYER_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GROOVED_TYPE_PLAYER_PROXY, GroovedPlayerProxy))
#define GROOVED_PLAYER_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GROOVED_TYPE_PLAYER_PROXY, GroovedPlayerProxyClass))
#define GROOVED_PLAYER_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GROOVED_TYPE_PLAYER_PROXY, GroovedPlayerProxyClass))
#define GROOVED_IS_PLAYER_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GROOVED_TYPE_PLAYER_PROXY))
#define GROOVED_IS_PLAYER_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GROOVED_TYPE_PLAYER_PROXY))

typedef struct _GroovedPlayerProxy GroovedPlayerProxy;
typedef struct _GroovedPlayerProxyClass GroovedPlayerProxyClass;
typedef struct _GroovedPlayerProxyPrivate GroovedPlayerProxyPrivate;

struct _GroovedPlayerProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  GroovedPlayerProxyPrivate *priv;
};

struct _GroovedPlayerProxyClass
{
  GDBusProxyClass parent_class;
};

GType grooved_player_proxy_get_type (void) G_GNUC_CONST;

void grooved_player_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
GroovedPlayer *grooved_player_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
GroovedPlayer *grooved_player_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void grooved_player_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
GroovedPlayer *grooved_player_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
GroovedPlayer *grooved_player_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define GROOVED_TYPE_PLAYER_SKELETON (grooved_player_skeleton_get_type ())
#define GROOVED_PLAYER_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GROOVED_TYPE_PLAYER_SKELETON, GroovedPlayerSkeleton))
#define GROOVED_PLAYER_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GROOVED_TYPE_PLAYER_SKELETON, GroovedPlayerSkeletonClass))
#define GROOVED_PLAYER_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GROOVED_TYPE_PLAYER_SKELETON, GroovedPlayerSkeletonClass))
#define GROOVED_IS_PLAYER_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GROOVED_TYPE_PLAYER_SKELETON))
#define GROOVED_IS_PLAYER_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GROOVED_TYPE_PLAYER_SKELETON))

typedef struct _GroovedPlayerSkeleton GroovedPlayerSkeleton;
typedef struct _GroovedPlayerSkeletonClass GroovedPlayerSkeletonClass;
typedef struct _GroovedPlayerSkeletonPrivate GroovedPlayerSkeletonPrivate;

struct _GroovedPlayerSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  GroovedPlayerSkeletonPrivate *priv;
};

struct _GroovedPlayerSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType grooved_player_skeleton_get_type (void) G_GNUC_CONST;

GroovedPlayer *grooved_player_skeleton_new (void);


G_END_DECLS

#endif /* __DBUS_COMMON_H__ */
