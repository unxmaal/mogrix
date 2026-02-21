/*
 * xge_compat.c - Stub implementations of Generic Event Extension functions
 *
 * IRIX native libX11 predates the Generic Event Extension (X11R7.5).
 * These stubs allow libXi to compile and link, AND export the symbols
 * from libXi.so so that rld can resolve them at runtime. Generic Event
 * (XI2) features will not work, but XInput1 functionality is unaffected.
 *
 * Functions stubbed:
 *   _XUnknownNativeEvent - converts unknown native events to wire format
 *   XESetWireToEventCookie - registers a wire-to-event cookie converter
 *   XESetCopyEventCookie - registers a cookie copy function
 */

#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/extensions/Xge.h>

/* Callback types for XESet*Cookie functions */
typedef Bool (*WireToEventCookieProc)(Display*, XGenericEventCookie*, xEvent*);
typedef Bool (*CopyEventCookieProc)(Display*, XGenericEventCookie*, XGenericEventCookie*);

/* _XUnknownNativeEvent: IRIX libX11 doesn't have this.
 * Return 0 (failure) - the event type is truly unknown. */
Status _XUnknownNativeEvent(Display *dpy, XEvent *re, xEvent *event)
{
    (void)dpy; (void)re; (void)event;
    return 0;
}

/* XESetWireToEventCookie: register a wire-to-cookie event handler.
 * Stub: return NULL (no previous handler). The handler is never called
 * because IRIX X server doesn't generate Generic Events. */
WireToEventCookieProc XESetWireToEventCookie(Display *dpy, int extension,
                                              WireToEventCookieProc proc)
{
    (void)dpy; (void)extension; (void)proc;
    return (WireToEventCookieProc)0;
}

/* XESetCopyEventCookie: register a cookie copy handler.
 * Stub: return NULL (no previous handler). */
CopyEventCookieProc XESetCopyEventCookie(Display *dpy, int extension,
                                          CopyEventCookieProc proc)
{
    (void)dpy; (void)extension; (void)proc;
    return (CopyEventCookieProc)0;
}
