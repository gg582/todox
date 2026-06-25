#!/bin/bash
set -e

TARGET=$1
BIN_DIR=/usr/local/bin
CONFIG_DIR=/usr/local/etc/todox
CONFIG_FILE=${TODOX_ALARM_FILE:-$CONFIG_DIR/alarm.txt}
SERVICE_NAME=todox

# Determine the user that should own the daemon. Desktop notifications are sent
# over the user's D-Bus session bus, so the daemon must run as that user rather
# than root.
detect_daemon_user() {
    if [ -n "${SUDO_USER:-}" ]; then
        echo "$SUDO_USER"
    else
        local login_user
        login_user=$(logname 2>/dev/null || true)
        if [ -n "$login_user" ]; then
            echo "$login_user"
        else
            whoami
        fi
    fi
}

# Run a command as the target user. If already running as that user, run directly.
run_as_user() {
    local user=$1
    shift
    if [ "$(id -un)" = "$user" ]; then
        "$@"
    else
        sudo -u "$user" "$@"
    fi
}

install_bin() {
    mkdir -p "$BIN_DIR"
    if [ ! -f build/todox ]; then
        echo "build/todox not found. run 'cmake -S . -B build && cmake --build build' first."
        exit 1
    fi
    cp build/todox "$BIN_DIR/todox"
    chmod +x "$BIN_DIR/todox"
}

install_config() {
    local daemon_user=$1
    mkdir -p "$CONFIG_DIR"
    if [ ! -f "$CONFIG_FILE" ]; then
        touch "$CONFIG_FILE"
    fi
    # The daemon runs as the desktop user, so the alarm file should be writable
    # by that user.
    if [ -n "${SUDO_USER:-}" ]; then
        chown "$SUDO_USER:$SUDO_USER" "$CONFIG_FILE"
        chmod 644 "$CONFIG_FILE"
    elif [ "$daemon_user" != "root" ]; then
        chown "$daemon_user:$daemon_user" "$CONFIG_FILE"
        chmod 644 "$CONFIG_FILE"
    else
        chmod 666 "$CONFIG_FILE"
    fi
}

install_env() {
    cat > /etc/profile.d/${SERVICE_NAME}.sh <<EOF
export TODOX_ALARM_FILE="$CONFIG_FILE"
EOF
    chmod 644 /etc/profile.d/${SERVICE_NAME}.sh
}

DAEMON_USER=$(detect_daemon_user)
if [ "$DAEMON_USER" = "root" ]; then
    echo "warning: daemon will run as root. desktop notifications require the"
    echo "         user's D-Bus session bus and will likely fail. install with"
    echo "         sudo from the desktop user account, or run 'todox --daemonize'"
    echo "         directly from your user session."
fi

case "$TARGET" in
    systemd)
        install_bin
        install_config "$DAEMON_USER"
        install_env
        # Install as a user service so it runs inside the desktop user's session
        # and can reach the D-Bus session bus used by the notification daemon.
        USER_SYSTEMD_DIR=$(getent passwd "$DAEMON_USER" | cut -d: -f6)/.config/systemd/user
        mkdir -p "$USER_SYSTEMD_DIR"
        cat > "$USER_SYSTEMD_DIR/${SERVICE_NAME}.service" <<EOF
[Unit]
Description=todox alarm daemon

[Service]
Type=forking
Environment="TODOX_ALARM_FILE=$CONFIG_FILE"
ExecStart=$BIN_DIR/todox --daemonize
Restart=on-failure

[Install]
WantedBy=default.target
EOF
        chown "$DAEMON_USER:$DAEMON_USER" "$USER_SYSTEMD_DIR"
        chown "$DAEMON_USER:$DAEMON_USER" "$USER_SYSTEMD_DIR/${SERVICE_NAME}.service"
        run_as_user "$DAEMON_USER" systemctl --user daemon-reload
        run_as_user "$DAEMON_USER" systemctl --user enable ${SERVICE_NAME}
        run_as_user "$DAEMON_USER" systemctl --user start ${SERVICE_NAME}
        echo "systemd user service installed and started for user '$DAEMON_USER'."
        echo "use: systemctl --user status ${SERVICE_NAME}"
        ;;

    openrc)
        install_bin
        install_config "$DAEMON_USER"
        install_env
        cat > /etc/init.d/${SERVICE_NAME} <<EOF
#!/sbin/openrc-run
name="todox"
description="todox alarm daemon"
command="/usr/local/bin/todox"
command_args="--daemonize"
command_background="yes"
command_user="$DAEMON_USER"
pidfile="/run/\${RC_SVCNAME}.pid"
export TODOX_ALARM_FILE="$CONFIG_FILE"
depend() {
    need net
    after firewall
}
EOF
        chmod +x /etc/init.d/${SERVICE_NAME}
        rc-update add ${SERVICE_NAME} default
        rc-service ${SERVICE_NAME} start
        echo "openrc service installed and started (user: $DAEMON_USER)."
        ;;

    slackware)
        install_bin
        install_config "$DAEMON_USER"
        install_env
        cat > /etc/rc.d/rc.${SERVICE_NAME} <<EOF
#!/bin/sh
# Start/stop/restart todox alarm daemon
DAEMON=/usr/local/bin/todox
PIDFILE=/var/run/todox.pid
CONFIG_FILE="$CONFIG_FILE"
DAEMON_USER="$DAEMON_USER"
export TODOX_ALARM_FILE="\$CONFIG_FILE"
todox_start() {
    if [ -f "\$PIDFILE" ] && kill -0 \$(cat "\$PIDFILE") 2>/dev/null; then
        echo "todox is already running"
        return
    fi
    echo "Starting todox..."
    su - "\$DAEMON_USER" -c "\$DAEMON --daemonize"
    sleep 1
    pgrep -f "\$DAEMON --daemonize" > "\$PIDFILE" 2>/dev/null || true
}
todox_stop() {
    if [ -f "\$PIDFILE" ]; then
        kill \$(cat "\$PIDFILE") 2>/dev/null || true
        rm -f "\$PIDFILE"
        echo "todox stopped"
    fi
}
todox_restart() {
    todox_stop
    sleep 1
    todox_start
}
case "\$1" in
    start) todox_start ;;
    stop) todox_stop ;;
    restart) todox_restart ;;
    *) echo "usage: \$0 {start|stop|restart}" ;;
esac
EOF
        chmod +x /etc/rc.d/rc.${SERVICE_NAME}
        if [ -f /etc/rc.d/rc.local ] && ! grep -q "/etc/rc.d/rc.${SERVICE_NAME} start" /etc/rc.d/rc.local; then
            echo "/etc/rc.d/rc.${SERVICE_NAME} start" >> /etc/rc.d/rc.local
        fi
        /etc/rc.d/rc.${SERVICE_NAME} start
        echo "slackware rc script installed and started (user: $DAEMON_USER)."
        ;;

    bsd)
        install_bin
        install_config "$DAEMON_USER"
        install_env
        rc_script="/etc/rc.d/${SERVICE_NAME}"
        cp "src/compat/bsd/rc.d/${SERVICE_NAME}" "$rc_script"
        sed -i "s|/usr/local/etc/todox/alarm.txt|$CONFIG_FILE|g" "$rc_script"
        sed -i "s|TODOX_USER_PLACEHOLDER|$DAEMON_USER|g" "$rc_script"
        chmod +x "$rc_script"
        if ! grep -qE "^${SERVICE_NAME}_enable=" /etc/rc.conf; then
            echo "${SERVICE_NAME}_enable=\"YES\"" >> /etc/rc.conf
        fi
        service ${SERVICE_NAME} start 2>/dev/null || "$rc_script" start
        echo "bsd rc.d script installed and started (user: $DAEMON_USER)."
        ;;

    *)
        echo "usage: $0 {systemd|openrc|slackware|bsd}"
        exit 1
        ;;
esac
