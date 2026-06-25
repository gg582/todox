# todox

CLI alarm manager with optional desktop notification daemon.

## Dependencies

- cmake
- pkg-config
- D-Bus development files

### Debian / Ubuntu

```bash
sudo apt-get install cmake pkg-config libdbus-1-dev
```

### Fedora / RHEL

```bash
sudo dnf install cmake pkgconfig dbus-devel
```

### Arch

```bash
sudo pacman -S cmake pkgconf dbus
```

### openSUSE

```bash
sudo zypper install cmake pkg-config dbus-1-devel
```

### Slackware

```bash
sudo slackpkg install cmake pkg-config dbus
```

### FreeBSD

```bash
sudo pkg install cmake pkgconf dbus
```

### OpenBSD

```bash
doas pkg_add cmake pkgconf dbus
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

The binary is produced at `build/todox`.

## Install

Install from the desktop user account (the one that should receive notifications):

```bash
sudo ./install.sh systemd
# or
sudo ./install.sh openrc
# or
sudo ./install.sh slackware
# or
sudo ./install.sh bsd
```

`install.sh` copies `build/todox` to `/usr/local/bin/todox`, creates `/usr/local/etc/todox/alarm.txt`, installs the init script, enables the service, and starts it. The daemon is started as the installing user so it can reach the desktop user's D-Bus session bus for notifications.

The service runs with `TODOX_ALARM_FILE=/usr/local/etc/todox/alarm.txt` set, so after installation most commands do not need a file argument.

**Note:** the notification daemon must run as the desktop user, not as root. Desktop notifications are sent over the user's D-Bus session bus, which is not available to a root-owned system daemon.

## Alarm file format

Each line is:

```text
YYYY-MM-DD HH:MM:SS +HHMM%%TASK%%COMMENT
```

The date may be omitted; a time-only timestamp is interpreted as today:

```text
HH:MM:SS +HHMM%%TASK%%COMMENT
```

Examples:

```text
1970-01-01 00:00:00 +0000%%test title%%test comment
21:00:00 +0900%%test title%%test comment
```

## Usage

### List alarms

```bash
todox alarm.txt
TODOX_ALARM_FILE=alarm.txt todox
```

### Add an alarm

```bash
todox add alarm.txt "1970-01-01 00:00:00 +0000%%test title%%test comment"
TODOX_ALARM_FILE=alarm.txt todox add "1970-01-01 00:00:00 +0000%%test title%%test comment"

# date may be omitted; time-only input uses today's date
todox add "21:00:00 +0900%%test title%%test comment"
```

### Remove an alarm

```bash
todox remove alarm.txt "test title"
TODOX_ALARM_FILE=alarm.txt todox remove "test title"
```

### Run notification daemon

```bash
todox --daemonize alarm.txt
TODOX_ALARM_FILE=alarm.txt todox --daemonize
```

The daemon loads alarms, sleeps until the next future alarm, sends a desktop notification via D-Bus, removes past alarms from the file, and repeats. Run it from the desktop user session so it can access the user's D-Bus session bus.

## Service control

If installed with systemd (user service):

```bash
systemctl --user start|stop|restart|status todox
```

If installed with openrc:

```bash
sudo rc-service todox start|stop|restart
```

If installed with slackware:

```bash
sudo /etc/rc.d/rc.todox start|stop|restart
```

If installed with bsd:

```bash
sudo service todox start|stop|restart
```
