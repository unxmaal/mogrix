/*
 * xge_compat.c - Stub implementations of Generic Event Extension functions
 *
 * IRIX native libX11 predates the Generic Event Extension (X11R7.5).
 * These stubs allow libXi to compile and link, but Generic Event (XI2)
 * features will not work at runtime. XInput1 functionality is unaffected.
 *
 * Functions stubbed:
 *   _XUnknownNativeEvent - converts unknown native events to wire format
 *   XESetWireToEventCookie - registers a wire-to-event cookie converter
 *   XESetCopyEventCookie - registers a cookie copy function
 */

#include <X11/Xlib.h>
#include <X11/Xproto.h>

typedef int Status;
typedef int Bool;

/* Forward declare types we need */
typedef struct {
    int            type;
    unsigned long  serial;
    Bool           send_event;
    Display        *display;
    int            extension;
    int            evtype;
    unsigned int   cookie;
    void           *data;
} XGenericEventCookie;

/* _XUnknownNativeEvent: IRIX libX11 doesn't have this.
 * Return 0 (failure) - the event type is truly unknown. */
Status _XUnknownNativeEvent(Display *dpy, XEvent *re, xEvent *event)
{
    return 0;
}

/* Callback types for XESet*Cookie functions */
typedef Bool (*WireToEventCookieProc)(Display*, XGenericEventCookie*, xEvent*);
typedef Bool (*CopyEventCookieProc)(Display*, XGenericEventCookie*, XGenericEventCookie*);

/* XESetWireToEventCookie: register a wire-to-cookie event handler.
 * Stub: return NULL (no previous handler). The handler is never called
 * because IRIX X server doesn't generate Generic Events. */
WireToEventCookieProc XESetWireToEventCookie(Display *dpy, int extension,
                                              WireToEventCookieProc proc)
{
    return (WireToEventCookieProc)0;
}

/* XESetCopyEventCookie: register a cookie copy handler.
 * Stub: return NULL (no previous handler). */
CopyEventCookieProc XESetCopyEventCookie(Display *dpy, int extension,
                                          CopyEventCookieProc proc)
{
    return (CopyEventCookieProc)0;
}
