/*
 * Copyright (C) 2013, 2020 Igalia S.L.
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

#ifndef Ir8SearchBox_h
#define Ir8SearchBox_h

#include <gtk/gtk.h>

#include <webkit2/webkit2.h>

G_BEGIN_DECLS

#define IR8_TYPE_SEARCH_BOX            (ir8_search_box_get_type())
#define IR8_SEARCH_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), IR8_TYPE_SEARCH_BOX, Ir8SearchBox))
#define IR8_IS_SEARCH_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), IR8_TYPE_SEARCH_BOX))
#define IR8_SEARCH_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  IR8_TYPE_SEARCH_BOX, Ir8SearchBoxClass))
#define IR8_IS_SEARCH_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  IR8_TYPE_SEARCH_BOX))
#define IR8_SEARCH_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  IR8_TYPE_SEARCH_BOX, Ir8SearchBoxClass))

typedef struct _Ir8SearchBox       Ir8SearchBox;
typedef struct _Ir8SearchBoxClass  Ir8SearchBoxClass;

struct _Ir8SearchBoxClass {
    GtkBoxClass parent_class;
};

GType ir8_search_box_get_type(void);

GtkWidget *ir8_search_box_new(WebKitWebView *);
GtkEntry *ir8_search_box_get_entry(Ir8SearchBox *);

G_END_DECLS

#endif
