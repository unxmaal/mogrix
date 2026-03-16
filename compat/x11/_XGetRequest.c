/* _XGetRequest.c - provide _XGetRequest() for IRIX native X11
 *
 * IRIX native libX11.so.1 (X11R6.4) uses GetReq as an inline macro
 * that directly manipulates dpy->bufptr. Modern Xlib (X11R7+) replaced
 * it with a function call to _XGetRequest(). Libraries compiled with
 * modern X11 headers (libXrender, libXft, etc.) need this function
 * at runtime when linking against IRIX native libX11.so.1.
 *
 * Compiled into libmogrix_compat.so (preloaded via _RLDN32_LIST).
 * The Display struct fields accessed here (bufptr, bufmax, last_req,
 * request) have been stable since X11R4 and are at compatible offsets
 * between IRIX native and modern Xlib.
 */

#include <X11/Xlib.h>
#include <X11/Xlibint.h>

void *
_XGetRequest(Display *dpy, CARD8 type, size_t len)
{
    xReq *req;

    if ((dpy->bufptr + len) > dpy->bufmax)
        _XFlush(dpy);

    req = (xReq *)(dpy->last_req = dpy->bufptr);
    req->reqType = type;
    req->length = len >> 2;
    dpy->bufptr += len;
    dpy->request++;

    return (void *)req;
}
