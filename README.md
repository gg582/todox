# todox

A small CLI alarm manager with a desktop-notification daemon.

## Overview

`todox` stores timestamped tasks in a plain text file and optionally runs a background daemon that wakes up and sends a desktop notification when an alarm is due. It is built in C with minimal dependencies and supports several init systems.

## Features

- Plain-text alarm storage (`YYYY-MM-DD HH:MM:SS +HHMM%%TASK%%COMMENT`).
- Sorted alarm list kept in a fixed-size array.
- Background notification daemon using D-Bus desktop notifications.
- Install helpers for systemd (user service), OpenRC, Slackware, and BSD rc.d.
- Portable UTC time conversion: uses `timegm()` on Linux and modern BSD systems.

## Project layout

```
include/   Public headers
src/       Implementation
src/app/   CLI command handling and I/O
src/compat/BSD compatibility helpers
src/error/ Error reporting
src/list/  Sorted fixed-array list
src/notify/Notification daemon and D-Bus client
src/time/  Time parsing and conversion
tests/     Unit tests
```

## Dependencies

- cmake
- pkg-config
- D-Bus development files

Install them by platform:

```bash
# Debian / Ubuntu
sudo apt-get install cmake pkg-config libdbus-1-dev

# Fedora / RHEL
sudo dnf install cmake pkgconfig dbus-devel

# Arch
sudo pacman -S cmake pkgconf dbus

# openSUSE
sudo zypper install cmake pkg-config dbus-1-devel

# Slackware
sudo slackpkg install cmake pkg-config dbus

# FreeBSD
sudo pkg install cmake pkgconf dbus

# OpenBSD
doas pkg_add cmake pkgconf dbus
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

The binary is written to `build/todox`.

## Install

Run the installer from the desktop user account that should receive notifications:

```bash
sudo ./install.sh systemd
# or
sudo ./install.sh openrc
# or
sudo ./install.sh slackware
# or
sudo ./install.sh bsd
```

`install.sh` copies `build/todox` to `/usr/local/bin/todox`, creates `/usr/local/etc/todox/alarm.txt`, and installs the service file. After installation the service uses `TODOX_ALARM_FILE=/usr/local/etc/todox/alarm.txt`, so most commands do not need a file argument.

> The notification daemon must run as the desktop user, not as root, because it sends notifications over the user's D-Bus session bus.

### systemd user service

`install.sh systemd` installs a user service. It does not start the daemon automatically as root. Enable and start it as the desktop user:

```bash
systemctl --user daemon-reload
systemctl --user enable --now todox
```

## Usage

### Alarm file format

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

### Weekly alarms

Weekday alarms repeat every week and use four fields:

```text
DAY%%TIME%%TASK%%COMMENT
```

`DAY` is a single weekday (`mon`), a range (`mon-fri`), or a list (`mon:wed:fri`).
`TIME` is `HH:MM:SS` with an optional timezone offset such as `+0900`.

Examples:

```text
mon-fri%%21:00:00 +0900%%gym%%workout
mon:wed:fri%%09:00:00 +0900%%standup%%daily sync
```

### Commands

| Command | Description | Example |
|---|---|---|
| (none) | List alarms. | `todox` |
| `add` | Add an alarm. | `todox add "2026-06-25 21:00:00 +0900%%task%%comment"` |
| `remove` | Remove an alarm by task name. | `todox remove "task"` |
| `--daemonize` | Run the notification daemon. | `todox --daemonize` |

Commands accept an explicit alarm file or fall back to `TODOX_ALARM_FILE`:

```bash
todox alarm.txt
TODOX_ALARM_FILE=alarm.txt todox add "21:00:00 +0900%%task%%comment"
```

## Service control

### systemd (user service)

```bash
systemctl --user start|stop|restart|status todox
```

### OpenRC

```bash
sudo rc-service todox start|stop|restart
```

### Slackware

```bash
sudo /etc/rc.d/rc.todox start|stop|restart
```

### BSD

```bash
sudo service todox start|stop|restart
```

## Technical notes

- **D-Bus session model**: the daemon connects to the user's session bus. The systemd install path uses a non-privileged user service so the daemon inherits the correct D-Bus environment without requiring `sudo` workarounds.
- **Time handling**: Linux uses `timegm()` directly; the BSD compatibility layer delegates to the libc `timegm()` provided by current FreeBSD, NetBSD, OpenBSD, and DragonFly systems instead of reimplementing leap-year arithmetic.
- **Memory model**: error messages use a single-free lifespan; the alarm list is a fixed-size sorted array.
