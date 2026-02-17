/*
 * Stub implementations for X Generic Event (XGE) extension functions.
 *
 * IRIX ships X11R6 which predates the XGE extension (X11R7.1+).
 * libXi.so.6 (XInput2) calls these to register event cookie handlers.
 * Returning NULL (no previous handler) is safe — it just means XInput2
 * cookie events won't be processed, which is fine since IRIX's X server
 * doesn't support XInput2 anyway.
 *
 * These must be visible in .dynsym so rld can resolve them when loading
 * libXi.so.6. Link into executables or export from a shared library.
 */

/* Opaque types — we only need the function signatures to match */
typedef struct _XDisplay Display;
typedef int (*XExtensionHooks)(void);

/*
 * XESetWireToEventCookie - Register a wire-to-event cookie conversion handler.
 * Returns: previous handler (NULL if none).
 */
XExtensionHooks XESetWireToEventCookie(Display *dpy, int extension_id,
                                        XExtensionHooks proc)
{
    (void)dpy;
    (void)extension_id;
    (void)proc;
    return (XExtensionHooks)0;
}

/*
 * XESetCopyEventCookie - Register an event cookie copy handler.
 * Returns: previous handler (NULL if none).
 */
XExtensionHooks XESetCopyEventCookie(Display *dpy, int extension_id,
                                      XExtensionHooks proc)
{
    (void)dpy;
    (void)extension_id;
    (void)proc;
    return (XExtensionHooks)0;
}
