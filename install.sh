#!/bin/bash
set -e

TARGET=$1
BIN_DIR=/usr/local/bin
CONFIG_DIR=/usr/local/etc/todox
CONFIG_FILE=${TODOX_ALARM_FILE:-$CONFIG_DIR/alarm.txt}
SERVICE_NAME=todox

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
    mkdir -p "$CONFIG_DIR"
    if [ ! -f "$CONFIG_FILE" ]; then
        touch "$CONFIG_FILE"
    fi
}

case "$TARGET" in
    systemd)
        install_bin
        install_config
        cat > /etc/systemd/system/${SERVICE_NAME}.service <<EOF
[Unit]
Description=todox alarm daemon
After=network.target

[Service]
Type=forking
Environment="TODOX_ALARM_FILE=$CONFIG_FILE"
ExecStart=$BIN_DIR/todox --daemonize
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF
        systemctl daemon-reload
        systemctl enable ${SERVICE_NAME}
        systemctl start ${SERVICE_NAME}
        echo "systemd service installed and started."
        ;;

    openrc)
        install_bin
        install_config
        cat > /etc/init.d/${SERVICE_NAME} <<'EOF'
#!/sbin/openrc-run
name="todox"
description="todox alarm daemon"
command="/usr/local/bin/todox"
command_args="--daemonize"
command_background="yes"
pidfile="/run/${RC_SVCNAME}.pid"
export TODOX_ALARM_FILE="CONFIG_FILE_PLACEHOLDER"
depend() {
    need net
    after firewall
}
EOF
        sed -i "s|CONFIG_FILE_PLACEHOLDER|$CONFIG_FILE|g" /etc/init.d/${SERVICE_NAME}
        chmod +x /etc/init.d/${SERVICE_NAME}
        rc-update add ${SERVICE_NAME} default
        rc-service ${SERVICE_NAME} start
        echo "openrc service installed and started."
        ;;

    slackware)
        install_bin
        install_config
        cat > /etc/rc.d/rc.${SERVICE_NAME} <<'EOF'
#!/bin/sh
# Start/stop/restart todox alarm daemon
DAEMON=/usr/local/bin/todox
PIDFILE=/var/run/todox.pid
CONFIG_FILE=CONFIG_FILE_PLACEHOLDER
export TODOX_ALARM_FILE="$CONFIG_FILE"
todox_start() {
    if [ -f "$PIDFILE" ] && kill -0 $(cat "$PIDFILE") 2>/dev/null; then
        echo "todox is already running"
        return
    fi
    echo "Starting todox..."
    $DAEMON --daemonize
    sleep 1
    pgrep -f "$DAEMON --daemonize" > "$PIDFILE" 2>/dev/null || true
}
todox_stop() {
    if [ -f "$PIDFILE" ]; then
        kill $(cat "$PIDFILE") 2>/dev/null || true
        rm -f "$PIDFILE"
        echo "todox stopped"
    fi
}
todox_restart() {
    todox_stop
    sleep 1
    todox_start
}
case "$1" in
    start) todox_start ;;
    stop) todox_stop ;;
    restart) todox_restart ;;
    *) echo "usage: $0 {start|stop|restart}" ;;
esac
EOF
        sed -i "s|CONFIG_FILE_PLACEHOLDER|$CONFIG_FILE|g" /etc/rc.d/rc.${SERVICE_NAME}
        chmod +x /etc/rc.d/rc.${SERVICE_NAME}
        if [ -f /etc/rc.d/rc.local ] && ! grep -q "/etc/rc.d/rc.${SERVICE_NAME} start" /etc/rc.d/rc.local; then
            echo "/etc/rc.d/rc.${SERVICE_NAME} start" >> /etc/rc.d/rc.local
        fi
        /etc/rc.d/rc.${SERVICE_NAME} start
        echo "slackware rc script installed and started."
        ;;

    *)
        echo "usage: $0 {systemd|openrc|slackware}"
        exit 1
        ;;
esac
