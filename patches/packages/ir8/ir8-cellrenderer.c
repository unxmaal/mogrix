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

#include "ir8-cellrenderer.h"
#include "ir8-marshal.h"
#include <errno.h>

enum {
    PROP_0,

    PROP_VALUE,
    PROP_ADJUSTMENT
};

enum {
    CHANGED,

    LAST_SIGNAL
};

struct _Ir8CellRendererVariant {
    GtkCellRenderer parent;

    GValue *value;

    GtkCellRenderer *textRenderer;
    GtkCellRenderer *toggleRenderer;
    GtkCellRenderer *spinRenderer;
};

struct _Ir8CellRendererVariantClass {
    GtkCellRendererClass parent;
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(Ir8CellRendererVariant, ir8_cell_renderer_variant, GTK_TYPE_CELL_RENDERER)

static void ir8CellRendererVariantFinalize(GObject *object)
{
    Ir8CellRendererVariant *renderer = IR8_CELL_RENDERER_VARIANT(object);

    g_object_unref(renderer->toggleRenderer);
    g_object_unref(renderer->spinRenderer);
    g_object_unref(renderer->textRenderer);
    if (renderer->value)
        g_boxed_free(G_TYPE_VALUE, renderer->value);

    G_OBJECT_CLASS(ir8_cell_renderer_variant_parent_class)->finalize(object);
}

static void ir8CellRendererVariantGetProperty(GObject *object, guint propId, GValue *value, GParamSpec *pspec)
{
    Ir8CellRendererVariant *renderer = IR8_CELL_RENDERER_VARIANT(object);

    switch (propId) {
    case PROP_VALUE:
        g_value_set_boxed(value, renderer->value);
        break;
    case PROP_ADJUSTMENT: {
        GtkAdjustment *adjustment = NULL;
        g_object_get(G_OBJECT(renderer->spinRenderer), "adjustment", &adjustment, NULL);
        if (adjustment) {
            g_value_set_object(value, adjustment);
            g_object_unref(adjustment);
        }
        break;
    }
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propId, pspec);
    }
}

static void ir8CellRendererVariantSetModeForValue(Ir8CellRendererVariant *renderer)
{
    if (!renderer->value)
        return;

    GtkCellRendererMode mode;
    if (G_VALUE_HOLDS_BOOLEAN(renderer->value))
        mode = GTK_CELL_RENDERER_MODE_ACTIVATABLE;
    else if (G_VALUE_HOLDS_STRING(renderer->value) || G_VALUE_HOLDS_UINT(renderer->value))
        mode = GTK_CELL_RENDERER_MODE_EDITABLE;
    else
        return;

    g_object_set(G_OBJECT(renderer), "mode", mode, NULL);
}

static void ir8CellRendererVariantSetProperty(GObject *object, guint propId, const GValue *value, GParamSpec *pspec)
{
    Ir8CellRendererVariant *renderer = IR8_CELL_RENDERER_VARIANT(object);

    switch (propId) {
    case PROP_VALUE:
        if (renderer->value)
            g_boxed_free(G_TYPE_VALUE, renderer->value);
        renderer->value = g_value_dup_boxed(value);
        ir8CellRendererVariantSetModeForValue(renderer);
        break;
    case PROP_ADJUSTMENT:
        g_object_set(G_OBJECT(renderer->spinRenderer), "adjustment", g_value_get_object(value), NULL);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propId, pspec);
    }
}

static GtkCellRenderer *ir8CellRendererVariantGetRendererForValue(Ir8CellRendererVariant *renderer)
{
    if (!renderer->value)
        return NULL;

    if (G_VALUE_HOLDS_BOOLEAN(renderer->value)) {
        g_object_set(G_OBJECT(renderer->toggleRenderer),
                     "active", g_value_get_boolean(renderer->value),
                     NULL);
        return renderer->toggleRenderer;
    }

    if (G_VALUE_HOLDS_STRING(renderer->value)) {
        g_object_set(G_OBJECT(renderer->textRenderer),
                     "text", g_value_get_string(renderer->value),
                     NULL);
        return renderer->textRenderer;
    }

    if (G_VALUE_HOLDS_UINT(renderer->value)) {
        gchar *text = g_strdup_printf("%u", g_value_get_uint(renderer->value));
        g_object_set(G_OBJECT(renderer->spinRenderer), "text", text, NULL);
        g_free(text);
        return renderer->spinRenderer;
    }

    return NULL;
}

static void ir8CellRendererVariantCellRendererTextEdited(Ir8CellRendererVariant *renderer, const gchar *path, const gchar *newText)
{
    if (!renderer->value)
        return;

    if (!G_VALUE_HOLDS_STRING(renderer->value))
        return;

    g_value_set_string(renderer->value, newText);
    g_signal_emit(renderer, signals[CHANGED], 0, path, renderer->value);
}

static void ir8CellRendererVariantCellRendererSpinEdited(Ir8CellRendererVariant *renderer, const gchar *path, const gchar *newText)
{
    if (!renderer->value)
        return;

    if (!G_VALUE_HOLDS_UINT(renderer->value))
        return;

    GtkAdjustment *adjustment;
    g_object_get(G_OBJECT(renderer->spinRenderer), "adjustment", &adjustment, NULL);
    if (!adjustment)
        return;

    errno = 0;
    gchar *endPtr;
    gdouble value = g_strtod(newText, &endPtr);
    if (errno || value > gtk_adjustment_get_upper(adjustment) || value < gtk_adjustment_get_lower(adjustment) || endPtr == newText) {
        g_warning("Invalid input for cell: %s\n", newText);
        return;
    }

    g_value_set_uint(renderer->value, (guint)value);
    g_signal_emit(renderer, signals[CHANGED], 0, path, renderer->value);
}

static gboolean ir8CellRendererVariantCellRendererActivate(GtkCellRenderer *cell, GdkEvent *event, GtkWidget *widget, const gchar *path, const GdkRectangle *bgArea, const GdkRectangle *cellArea, GtkCellRendererState flags)
{
    Ir8CellRendererVariant *renderer = IR8_CELL_RENDERER_VARIANT(cell);

    if (!renderer->value)
        return TRUE;

    if (!G_VALUE_HOLDS_BOOLEAN(renderer->value))
        return TRUE;

    g_value_set_boolean(renderer->value, !g_value_get_boolean(renderer->value));
    g_signal_emit(renderer, signals[CHANGED], 0, path, renderer->value);

    return TRUE;
}

static void ir8CellRendererVariantCellRendererRender(GtkCellRenderer *cell, cairo_t *cr, GtkWidget *widget, const GdkRectangle *bgArea, const GdkRectangle *cellArea, GtkCellRendererState flags)
{
    GtkCellRenderer *renderer = ir8CellRendererVariantGetRendererForValue(IR8_CELL_RENDERER_VARIANT(cell));
    if (!renderer)
        return;

    GTK_CELL_RENDERER_GET_CLASS(renderer)->render(renderer, cr, widget, bgArea, cellArea, flags);
}

static GtkCellEditable *ir8CellRendererVariantCellRendererStartEditing(GtkCellRenderer *cell, GdkEvent *event, GtkWidget *widget, const gchar *path, const GdkRectangle *bgArea, const GdkRectangle *cellArea, GtkCellRendererState flags)
{
    GtkCellRenderer *renderer = ir8CellRendererVariantGetRendererForValue(IR8_CELL_RENDERER_VARIANT(cell));
    if (!renderer)
        return NULL;

    if (!GTK_CELL_RENDERER_GET_CLASS(renderer)->start_editing)
        return NULL;

    return GTK_CELL_RENDERER_GET_CLASS(renderer)->start_editing(renderer, event, widget, path, bgArea, cellArea, flags);
}

static void ir8CellRendererVariantCellRendererGetPreferredWidth(GtkCellRenderer *cell, GtkWidget *widget, gint *minimumWidth, gint *naturalWidth)
{
    GtkCellRenderer *renderer = ir8CellRendererVariantGetRendererForValue(IR8_CELL_RENDERER_VARIANT(cell));
    if (!renderer)
        return;

    GTK_CELL_RENDERER_GET_CLASS(renderer)->get_preferred_width(renderer, widget, minimumWidth, naturalWidth);
}

static void ir8CellRendererVariantCellRendererGetPreferredHeight(GtkCellRenderer *cell, GtkWidget *widget, gint *minimumHeight, gint *naturalHeight)
{
    GtkCellRenderer *renderer = ir8CellRendererVariantGetRendererForValue(IR8_CELL_RENDERER_VARIANT(cell));
    if (!renderer)
        return;

    GTK_CELL_RENDERER_GET_CLASS(renderer)->get_preferred_height(renderer, widget, minimumHeight, naturalHeight);
}

static void ir8CellRendererVariantCellRendererGetPreferredWidthForHeight(GtkCellRenderer *cell, GtkWidget *widget, gint height, gint *minimumWidth, gint *naturalWidth)
{
    GtkCellRenderer *renderer = ir8CellRendererVariantGetRendererForValue(IR8_CELL_RENDERER_VARIANT(cell));
    if (!renderer)
        return;

    GTK_CELL_RENDERER_GET_CLASS(renderer)->get_preferred_width_for_height(renderer, widget, height, minimumWidth, naturalWidth);
}

static void ir8CellRendererVariantCellRendererGetPreferredHeightForWidth(GtkCellRenderer *cell, GtkWidget *widget, gint width, gint *minimumHeight, gint *naturalHeight)
{
    GtkCellRenderer *renderer = ir8CellRendererVariantGetRendererForValue(IR8_CELL_RENDERER_VARIANT(cell));
    if (!renderer)
        return;

    GTK_CELL_RENDERER_GET_CLASS(renderer)->get_preferred_height_for_width(renderer, widget, width, minimumHeight, naturalHeight);
}

static void ir8CellRendererVariantCellRendererGetAlignedArea(GtkCellRenderer *cell, GtkWidget *widget, GtkCellRendererState flags, const GdkRectangle *cellArea, GdkRectangle *alignedArea)
{
    GtkCellRenderer *renderer = ir8CellRendererVariantGetRendererForValue(IR8_CELL_RENDERER_VARIANT(cell));
    if (!renderer)
        return;

    GTK_CELL_RENDERER_GET_CLASS(renderer)->get_aligned_area(renderer, widget, flags, cellArea, alignedArea);
}

static void ir8_cell_renderer_variant_init(Ir8CellRendererVariant *renderer)
{
    g_object_set(renderer, "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE, NULL);

    renderer->toggleRenderer = gtk_cell_renderer_toggle_new();
    g_object_set(G_OBJECT(renderer->toggleRenderer), "xalign", 0.0, NULL);
    renderer->toggleRenderer = g_object_ref_sink(renderer->toggleRenderer);

    renderer->textRenderer = gtk_cell_renderer_text_new();
    g_signal_connect_swapped(renderer->textRenderer, "edited",
                             G_CALLBACK(ir8CellRendererVariantCellRendererTextEdited), renderer);
    g_object_set(G_OBJECT(renderer->textRenderer), "editable", TRUE, NULL);
    renderer->textRenderer = g_object_ref_sink(renderer->textRenderer);

    renderer->spinRenderer = gtk_cell_renderer_spin_new();
    g_signal_connect_swapped(renderer->spinRenderer, "edited",
                             G_CALLBACK(ir8CellRendererVariantCellRendererSpinEdited), renderer);
    g_object_set(G_OBJECT(renderer->spinRenderer), "editable", TRUE, NULL);
}

static void ir8_cell_renderer_variant_class_init(Ir8CellRendererVariantClass *klass)
{
    GObjectClass *gobjectClass = G_OBJECT_CLASS(klass);
    GtkCellRendererClass *cellRendererClass = GTK_CELL_RENDERER_CLASS(klass);

    gobjectClass->get_property = ir8CellRendererVariantGetProperty;
    gobjectClass->set_property = ir8CellRendererVariantSetProperty;
    gobjectClass->finalize = ir8CellRendererVariantFinalize;

    cellRendererClass->activate = ir8CellRendererVariantCellRendererActivate;
    cellRendererClass->render = ir8CellRendererVariantCellRendererRender;
    cellRendererClass->start_editing = ir8CellRendererVariantCellRendererStartEditing;
    cellRendererClass->get_preferred_width = ir8CellRendererVariantCellRendererGetPreferredWidth;
    cellRendererClass->get_preferred_height = ir8CellRendererVariantCellRendererGetPreferredHeight;
    cellRendererClass->get_preferred_width_for_height = ir8CellRendererVariantCellRendererGetPreferredWidthForHeight;
    cellRendererClass->get_preferred_height_for_width = ir8CellRendererVariantCellRendererGetPreferredHeightForWidth;
    cellRendererClass->get_aligned_area = ir8CellRendererVariantCellRendererGetAlignedArea;

    g_object_class_install_property(gobjectClass,
                                    PROP_VALUE,
                                    g_param_spec_boxed("value",
                                                       NULL, NULL,
                                                       G_TYPE_VALUE,
                                                       G_PARAM_READWRITE));
    g_object_class_install_property(gobjectClass,
                                    PROP_ADJUSTMENT,
                                    g_param_spec_object("adjustment",
                                                        NULL, NULL,
                                                        GTK_TYPE_ADJUSTMENT,
                                                        G_PARAM_READWRITE));

    signals[CHANGED] =
        g_signal_new("changed",
                     G_TYPE_FROM_CLASS(gobjectClass),
                     G_SIGNAL_RUN_LAST,
                     0, NULL, NULL,
                     ir8_marshal_VOID__STRING_BOXED,
                     G_TYPE_NONE, 2,
                     G_TYPE_STRING, G_TYPE_VALUE);
}

GtkCellRenderer *ir8_cell_renderer_variant_new(void)
{
    return GTK_CELL_RENDERER(g_object_new(IR8_TYPE_CELL_RENDERER_VARIANT, NULL));
}

