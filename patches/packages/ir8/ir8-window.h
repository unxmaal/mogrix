/*
 * Copyright (C) 2011 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Ir8Window_h
#define Ir8Window_h

#include <gtk/gtk.h>

#include <webkit2/webkit2.h>

G_BEGIN_DECLS

#define IR8_TYPE_WINDOW            (ir8_window_get_type())
#define IR8_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), IR8_TYPE_WINDOW, Ir8Window))
#define IR8_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  IR8_TYPE_WINDOW, Ir8WindowClass))
#define IR8_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), IR8_TYPE_WINDOW))
#define IR8_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  IR8_TYPE_WINDOW))
#define IR8_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  IR8_TYPE_WINDOW, Ir8WindowClass))
#define IR8_DEFAULT_URL            "ir8-about:home"
#define IR8_ABOUT_SCHEME           "ir8-about"

typedef struct _Ir8Window        Ir8Window;
typedef struct _Ir8WindowClass   Ir8WindowClass;

GType ir8_window_get_type(void);

GtkWidget* ir8_window_new(GtkWindow*, WebKitWebContext*);
WebKitWebContext* ir8_window_get_web_context(Ir8Window*);
void ir8_window_append_view(Ir8Window*, WebKitWebView*);
void ir8_window_load_uri(Ir8Window*, const char *uri);
void ir8_window_load_session(Ir8Window *, const char *sessionFile);
void ir8_window_set_background_color(Ir8Window*, GdkRGBA*);

G_END_DECLS

#endif
