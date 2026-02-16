/*
 * Xge.h - Generic Event Extension client-side header
 *
 * IRIX sysroot Xlib.h and libXext predate the Generic Event Extension
 * (added in X.org X11R7.5 / libX11 1.2). This header provides the
 * XGenericEvent and XGenericEventCookie types needed by libXi 1.7+.
 *
 * These types are normally in Xlib.h (modern) or provided by libXext's
 * Xge.h, but neither is available on IRIX.
 */

#ifndef _XGE_H_
#define _XGE_H_

#include <X11/Xlib.h>

/*
 * XGenericEvent - wire format for GenericEvent (type 35).
 * Added in X11R7.1 / libX11 1.2.
 */
#ifndef _XGenericEvent_defined
#define _XGenericEvent_defined
typedef struct {
    int            type;         /* GenericEvent */
    unsigned long  serial;       /* # of last request processed by server */
    Bool           send_event;   /* true if from SendEvent */
    Display        *display;     /* Display the event was read from */
    int            extension;    /* major opcode of extension */
    int            evtype;       /* event sub-type */
} XGenericEvent;
#endif

/*
 * XGenericEventCookie - client-side representation of a GenericEvent
 * with associated data. Used by XGetEventData/XFreeEventData.
 */
#ifndef _XGenericEventCookie_defined
#define _XGenericEventCookie_defined
typedef struct {
    int            type;         /* GenericEvent */
    unsigned long  serial;       /* # of last request processed by server */
    Bool           send_event;   /* true if from SendEvent */
    Display        *display;     /* Display the event was read from */
    int            extension;    /* major opcode of extension */
    int            evtype;       /* event sub-type */
    unsigned int   cookie;       /* unique cookie for GetEventData */
    void           *data;        /* extension-specific data */
} XGenericEventCookie;
#endif

/*
 * XGetEventData / XFreeEventData - stubs for IRIX.
 * IRIX's X server doesn't generate GenericEvents, so these are no-ops.
 * XGetEventData returns False (no data available).
 */
static __inline__ Bool XGetEventData(Display *dpy, XGenericEventCookie *cookie) {
    (void)dpy; (void)cookie;
    return False;
}
static __inline__ void XFreeEventData(Display *dpy, XGenericEventCookie *cookie) {
    (void)dpy; (void)cookie;
}

/*
 * XSetIMValues - IRIX Xlib.h declares XGetIMValues but not XSetIMValues.
 * Added in X11R6.3 update. Declare it here; the implementation is in libX11.
 */
extern char *XSetIMValues(XIM, ...);

#endif /* _XGE_H_ */
