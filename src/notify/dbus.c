#include <notify/notify.h>
#include <dbus/dbus.h>
#include <syslog.h>

int todox_send_desktop_notification(const char *title, const char *body) {
    DBusError err;
    DBusConnection *conn;
    DBusMessage *msg;
    DBusMessage *reply;
    DBusMessageIter args;
    DBusMessageIter sub;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if(dbus_error_is_set(&err)) {
        syslog(LOG_ERR, "todox: failed to get dbus session bus: %s", err.message);
        dbus_error_free(&err);
    }
    if(conn == NULL) {
        syslog(LOG_ERR, "todox: dbus connection is null (DBUS_SESSION_BUS_ADDRESS may be missing or invalid)");
        return -1;
    }

    msg = dbus_message_new_method_call(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        "Notify");
    if(msg == NULL) {
        syslog(LOG_ERR, "todox: failed to create dbus Notify message");
        return -1;
    }

    const char *app_name = "todox";
    dbus_uint32_t replaces_id = 0;
    const char *app_icon = "dialog-information";
    const char *summary = title ? title : "todox";
    const char *notification_body = body ? body : "";
    dbus_int32_t expire_timeout = 0;
    const char *urgency_key = "urgency";
    dbus_int32_t urgency_value = 2;

    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &app_name);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &replaces_id);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &app_icon);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &summary);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &notification_body);

    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &sub);
    dbus_message_iter_close_container(&args, &sub);

    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &sub);
    {
        DBusMessageIter dict_entry, variant;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry);
        dbus_message_iter_append_basic(&dict_entry, DBUS_TYPE_STRING, &urgency_key);
        dbus_message_iter_open_container(&dict_entry, DBUS_TYPE_VARIANT, DBUS_TYPE_BYTE_AS_STRING, &variant);
        dbus_message_iter_append_basic(&variant, DBUS_TYPE_BYTE, &urgency_value);
        dbus_message_iter_close_container(&dict_entry, &variant);
        dbus_message_iter_close_container(&sub, &dict_entry);
    }
    dbus_message_iter_close_container(&args, &sub);

    dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &expire_timeout);

    reply = dbus_connection_send_with_reply_and_block(conn, msg, 5000, &err);
    dbus_message_unref(msg);

    if(dbus_error_is_set(&err)) {
        syslog(LOG_ERR, "todox: dbus Notify call failed: %s", err.message);
        dbus_error_free(&err);
        if(reply != NULL) {
            dbus_message_unref(reply);
        }
        return -1;
    }

    if(reply != NULL) {
        dbus_message_unref(reply);
    }
    return 0;
}
