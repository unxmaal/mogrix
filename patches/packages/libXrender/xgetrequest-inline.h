/*
 * IRIX native libX11.so.1 (X11R6.4) does not provide _XGetRequest().
 * Modern Xlibint.h defines GetReq/GetReqSized/GetResReq/GetEmptyReq
 * as calls to _XGetRequest(). Redefine them to use the old inline
 * macro form that directly manipulates dpy->bufptr.
 *
 * Include this AFTER <X11/Xlibint.h>.
 */

#undef GetReqSized
#define GetReqSized(name, sz, req) \
    if ((dpy->bufptr + (sz)) > dpy->bufmax) \
        _XFlush(dpy); \
    req = (x##name##Req *)(dpy->last_req = dpy->bufptr); \
    req->reqType = X_##name; \
    req->length = (sz)>>2; \
    dpy->bufptr += (sz); \
    dpy->request++

#undef GetReq
#define GetReq(name, req) \
    GetReqSized(name, SIZEOF(x##name##Req), req)

#undef GetReqExtra
#define GetReqExtra(name, n, req) \
    GetReqSized(name, SIZEOF(x##name##Req) + n, req)

#undef GetResReq
#define GetResReq(name, rid, req) \
    GetReqSized(name, SIZEOF(xResourceReq), req); \
    req->id = (rid)

#undef GetEmptyReq
#define GetEmptyReq(name, req) \
    if ((dpy->bufptr + SIZEOF(xReq)) > dpy->bufmax) \
        _XFlush(dpy); \
    req = (xReq *)(dpy->last_req = dpy->bufptr); \
    req->reqType = X_##name; \
    req->length = 1; \
    dpy->bufptr += SIZEOF(xReq); \
    dpy->request++
