/*
 * glib-n32-fixup.h — Fix GLib 2.80 g_once_init_enter/leave for MIPS n32.
 *
 * On n32, sizeof(gsize) == 8 but sizeof(gpointer) == 4. GLib 2.80 added
 * G_STATIC_ASSERT(sizeof *(location) == sizeof(gpointer)) to all versions
 * of g_once_init_enter/leave macros, making G_DEFINE_TYPE fail on n32.
 *
 * Fix: include glib.h, then undef the macro wrappers so the underlying
 * gsize-based functions are called directly (no pointer-size assertion).
 *
 * Usage: CFLAGS += -include glib-n32-fixup.h
 */
#include <glib.h>

#ifdef g_once_init_enter
#undef g_once_init_enter
#endif
#ifdef g_once_init_leave
#undef g_once_init_leave
#endif
