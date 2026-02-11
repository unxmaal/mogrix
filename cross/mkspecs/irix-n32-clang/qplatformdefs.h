/****************************************************************************
**
** IRIX N32 platform definitions for Qt5 cross-compilation
** Based on linux-clang/qplatformdefs.h, adapted for SGI IRIX 6.5
**
****************************************************************************/

#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

// Get Qt defines/settings
#include "qglobal.h"

// IRIX uses _SGI_SOURCE to enable all APIs (equivalent to _GNU_SOURCE on Linux)
#ifndef _SGI_SOURCE
#  define _SGI_SOURCE
#endif

#include <unistd.h>

// IRIX does not have <features.h> - skip it
#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define QT_USE_XOPEN_LFS_EXTENSIONS
#include "../common/posix/qplatformdefs.h"

#undef QT_SOCKLEN_T
#define QT_SOCKLEN_T            socklen_t

#define QT_SNPRINTF             ::snprintf
#define QT_VSNPRINTF            ::vsnprintf

#endif // QPLATFORMDEFS_H
