/*
 * Copyright (C) 2016 Igalia S.L.
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

#ifndef Ir8Tab_h
#define Ir8Tab_h

#include <gtk/gtk.h>

#include <webkit2/webkit2.h>

G_BEGIN_DECLS

#define IR8_TYPE_TAB            (ir8_tab_get_type())
#define IR8_TAB(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), IR8_TYPE_TAB, Ir8Tab))
#define IR8_TAB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  IR8_TYPE_TAB, Ir8TabClass))
#define IR8_IS_TAB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), IR8_TYPE_TAB))
#define IR8_IS_TAB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  IR8_TYPE_TAB))
#define IR8_TAB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  IR8_TYPE_TAB, Ir8TabClass))

typedef struct _Ir8Tab        Ir8Tab;
typedef struct _Ir8TabClass   Ir8TabClass;

GType ir8_tab_get_type(void);

GtkWidget* ir8_tab_new(WebKitWebView*);
WebKitWebView* ir8_tab_get_web_view(Ir8Tab*);
void ir8_tab_load_uri(Ir8Tab*, const char* uri);
GtkWidget *ir8_tab_get_title_widget(Ir8Tab*);
void ir8_tab_set_status_text(Ir8Tab*, const char* text);
void ir8_tab_toggle_inspector(Ir8Tab*);
void ir8_tab_start_search(Ir8Tab*);
void ir8_tab_stop_search(Ir8Tab*);
void ir8_tab_enter_fullscreen(Ir8Tab*);
void ir8_tab_leave_fullscreen(Ir8Tab*);
void ir8_tab_set_background_color(Ir8Tab*, GdkRGBA*);

G_END_DECLS

#endif
