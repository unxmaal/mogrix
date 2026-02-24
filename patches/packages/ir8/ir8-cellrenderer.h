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

#ifndef Ir8CellRendererVariant_h
#define Ir8CellRendererVariant_h

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define IR8_TYPE_CELL_RENDERER_VARIANT            (ir8_cell_renderer_variant_get_type())
#define IR8_CELL_RENDERER_VARIANT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), IR8_TYPE_CELL_RENDERER_VARIANT, Ir8CellRendererVariant))
#define IR8_CELL_RENDERER_VARIANT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  IR8_TYPE_CELL_RENDERER_VARIANT, Ir8CellRendererVariantClass))
#define IR8_IS_CELL_RENDERER_VARIANT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), IR8_TYPE_CELL_RENDERER_VARIANT))
#define IR8_IS_CELL_RENDERER_VARIANT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  IR8_TYPE_CELL_RENDERER_VARIANT))
#define IR8_CELL_RENDERER_VARIANT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  IR8_TYPE_CELL_RENDERER_VARIANT, Ir8CellRendererVariantClass))

typedef struct _Ir8CellRendererVariant        Ir8CellRendererVariant;
typedef struct _Ir8CellRendererVariantClass   Ir8CellRendererVariantClass;

GType ir8_cell_renderer_variant_get_type(void);

GtkCellRenderer* ir8_cell_renderer_variant_new(void);

G_END_DECLS

#endif
