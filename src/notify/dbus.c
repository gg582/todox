#include <notify/notify.h>
#include <dbus/dbus.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>

/** @brief set freedesktop notification mesage. */
static inline DBusMessage * set_dbus_message() {
    DBusMessage *msg = dbus_message_new_method_call("org.freedesktop.Notifications",
                                       "/org/freedesktop/Notifications",
                                       "org.freedesktop.Notifications", "Notify");
    if(msg == nullptr) {
        syslog(LOG_ERR, "todox: failed to create dbus Notify message");
        return nullptr;
    }

    return msg;
}

/** @brief append message to DBus. */
static inline DBusMessage * append_message(DBusConnection *conn, DBusMessage *msg, DBusError *err, const char *title, const char *body) {

    const char *app_name = "todox";
    dbus_uint32_t replaces_id = 0;
    const char *app_icon = "dialog-information";
    const char *summary = title ? title : "todox";
    const char *notification_body = body ? body : "";
    dbus_int32_t expire_timeout = 0;

    DBusMessageIter args;
    DBusMessageIter sub;

    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &app_name);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &replaces_id);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &app_icon);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &summary);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &notification_body);

    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &sub);
    dbus_message_iter_close_container(&args, &sub);

    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &sub);
    dbus_message_iter_close_container(&args, &sub);

    dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &expire_timeout);
    return dbus_connection_send_with_reply_and_block(conn, msg, 5000, err);
}

int todox_send_desktop_notification(const char *title, const char *body) {
    DBusError err;
    DBusConnection *conn = nullptr;
    DBusMessage *reply = nullptr;
    DBusMessage *msg = set_dbus_message();
    int ret = -1;


    // initialize DBusError.
    dbus_error_init(&err);

    if(!msg) goto cleanup;


    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

    /* If the session bus address was stripped by sudo or another privilege
     * boundary, derive it from the standard systemd user runtime directory.
     * This keeps the daemon working when launched as a systemd --user service
     * or from an otherwise clean environment.
     */
    if(conn == nullptr && getenv("DBUS_SESSION_BUS_ADDRESS") == nullptr) {
        const char *runtime = getenv("XDG_RUNTIME_DIR");
        if(runtime != nullptr) {
            char addr[512];
            int n = snprintf(addr, sizeof(addr), "unix:path=%s/bus", runtime);
            if(n > 0 && (size_t)n < sizeof(addr)) {
                setenv("DBUS_SESSION_BUS_ADDRESS", addr, 1);
                dbus_error_free(&err);
                dbus_error_init(&err);
                conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
            }
        }
    }

    if(dbus_error_is_set(&err)) {
        syslog(LOG_ERR, "todox: failed to get dbus session bus: %s", err.message);
        goto cleanup;
    }
    if(conn == nullptr) {
        syslog(
            LOG_ERR,
            "todox: dbus connection is null (DBUS_SESSION_BUS_ADDRESS may be missing or invalid)");
        goto cleanup;
    }

    reply = append_message(conn, msg, &err, title, body);
    // unreference a message after appending.
    dbus_message_unref(msg);
    msg = nullptr;

    if(dbus_error_is_set(&err)) {
        syslog(LOG_ERR, "todox: dbus Notify call failed: %s", err.message);
        goto cleanup;
    }

    ret = 0;

/// clean up remaining messages and destroy a connection.
cleanup:
    if(reply != nullptr) dbus_message_unref(reply);
    if(msg != nullptr) dbus_message_unref(msg);
    if(conn != nullptr) dbus_connection_unref(conn);
    dbus_error_free(&err);

    return ret;
}
