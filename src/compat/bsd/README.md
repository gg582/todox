# BSD Compatibility Layer

This directory contains workarounds and helper code for building and running
`todox` on BSD systems (FreeBSD, OpenBSD, NetBSD, DragonFly BSD).

Windows is not a target for this project.

## Contents

- `time.c` / `include/compat/bsd/time.h`  
  Provides `bsd_timegm()`, a BSD-compatible replacement for the GNU extension
  `timegm()`. The main code uses conditional compilation to call `timegm()` on
  Linux and `bsd_timegm()` on BSD, so no extra object file is compiled on Linux.

- `rc.d/todox`  
  A generic BSD `rc.d` service script for the notification daemon. It is
  installed by `install.sh bsd` to `/etc/rc.d/todox` and enabled via
  `/etc/rc.conf`.

## Adding New Workarounds

When a GNU/Linux-specific API is introduced, provide a portable fallback here:

1. Declare the fallback in `include/compat/bsd/<area>.h`.
2. Implement it in `src/compat/bsd/<area>.c`.
3. Add the `.c` file to `TODOX_SOURCES` in `CMakeLists.txt` under the BSD
   system-name check.
4. Use `#ifdef __linux__` at the call site to choose the Linux API or the BSD
   fallback explicitly.
