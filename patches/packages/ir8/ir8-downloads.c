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

#include "ir8-downloads.h"


#include <glib/gi18n.h>

#define IR8_TYPE_DOWNLOAD (ir8_download_get_type())
#define IR8_DOWNLOAD(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), IR8_TYPE_DOWNLOAD, Ir8Download))

typedef struct _Ir8Download Ir8Download;
typedef struct _Ir8DownloadClass Ir8DownloadClass;

static GType ir8_download_get_type();

struct _Ir8DownloadsBar {
    GtkInfoBar parent;
};

struct _Ir8DownloadsBarClass {
    GtkInfoBarClass parentClass;
};

G_DEFINE_TYPE(Ir8DownloadsBar, ir8_downloads_bar, GTK_TYPE_INFO_BAR)

static void
ir8DownloadsBarChildRemoved(GtkContainer *infoBar,  GtkWidget *widget, Ir8DownloadsBar *downloadsBar)
{
    GList *children = gtk_container_get_children(infoBar);
    if (g_list_length(children) == 1)
        gtk_info_bar_response(GTK_INFO_BAR(downloadsBar), GTK_RESPONSE_CLOSE);
    g_list_free(children);
}

static void ir8DownloadsBarResponse(GtkInfoBar *infoBar, gint responseId)
{
    gtk_widget_destroy(GTK_WIDGET(infoBar));
}

static void ir8_downloads_bar_init(Ir8DownloadsBar *downloadsBar)
{
    GtkWidget *contentBox = gtk_info_bar_get_content_area(GTK_INFO_BAR(downloadsBar));
    g_signal_connect_after(contentBox, "remove", G_CALLBACK(ir8DownloadsBarChildRemoved), downloadsBar);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(contentBox), GTK_ORIENTATION_VERTICAL);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>Downloads</span>");
    gtk_label_set_xalign(GTK_LABEL(title), 0.);
    gtk_label_set_yalign(GTK_LABEL(title), 0.5);
    gtk_box_pack_start(GTK_BOX(contentBox), title, FALSE, FALSE, 12);
    gtk_widget_show(title);
}

static void ir8_downloads_bar_class_init(Ir8DownloadsBarClass *klass)
{
    GtkInfoBarClass *infoBarClass = GTK_INFO_BAR_CLASS(klass);
    infoBarClass->response = ir8DownloadsBarResponse;
}

GtkWidget *ir8_downloads_bar_new()
{
    GtkInfoBar *downloadsBar = GTK_INFO_BAR(g_object_new(IR8_TYPE_DOWNLOADS_BAR, NULL));
    gtk_info_bar_add_buttons(downloadsBar, "_Close", GTK_RESPONSE_CLOSE, NULL);
    return GTK_WIDGET(downloadsBar);
}

struct _Ir8Download {
    GtkBox parent;

    WebKitDownload *download;
    guint64 contentLength;
    guint64 downloadedSize;
    gboolean finished;

    GtkWidget *statusLabel;
    GtkWidget *remainingLabel;
    GtkWidget *progressBar;
    GtkWidget *actionButton;
};

struct _Ir8DownloadClass {
    GtkBoxClass parentClass;
};

G_DEFINE_TYPE(Ir8Download, ir8_download, GTK_TYPE_BOX)

static void actionButtonClicked(GtkButton *button, Ir8Download *ir8Download)
{
    if (!ir8Download->finished) {
        webkit_download_cancel(ir8Download->download);
        return;
    }

    gtk_show_uri_on_window(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(ir8Download))),
                 webkit_download_get_destination(ir8Download->download),
                 gtk_get_current_event_time(), NULL);
    gtk_widget_destroy(GTK_WIDGET(ir8Download));
}

static void ir8DownloadFinalize(GObject *object)
{
    Ir8Download *ir8Download = IR8_DOWNLOAD(object);

    if (ir8Download->download) {
        g_signal_handlers_disconnect_by_data(ir8Download->download, ir8Download);
        g_object_unref(ir8Download->download);
        ir8Download->download = NULL;
    }

    G_OBJECT_CLASS(ir8_download_parent_class)->finalize(object);
}

static void ir8_download_init(Ir8Download *download)
{
    GtkWidget *mainBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(download), mainBox, FALSE, FALSE, 0);
    gtk_widget_show(mainBox);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(mainBox), vbox, TRUE, TRUE, 0);
    gtk_widget_show(vbox);

    GtkWidget *statusBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), statusBox, TRUE, TRUE, 0);
    gtk_widget_show(statusBox);

    download->statusLabel = gtk_label_new("Starting Download");
    gtk_label_set_ellipsize(GTK_LABEL(download->statusLabel), PANGO_ELLIPSIZE_END);
    gtk_label_set_xalign(GTK_LABEL(download->statusLabel), 0.);
    gtk_label_set_yalign(GTK_LABEL(download->statusLabel), 0.5);
    gtk_box_pack_start(GTK_BOX(statusBox), download->statusLabel, TRUE, TRUE, 0);
    gtk_widget_show(download->statusLabel);

    download->remainingLabel = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(download->remainingLabel), 1.);
    gtk_label_set_yalign(GTK_LABEL(download->remainingLabel), 0.5);
    gtk_box_pack_end(GTK_BOX(statusBox), download->remainingLabel, TRUE, TRUE, 0);
    gtk_widget_show(download->remainingLabel);

    download->progressBar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), download->progressBar, FALSE, FALSE, 0);
    gtk_widget_show(download->progressBar);

    download->actionButton = gtk_button_new_with_mnemonic("_Cancel");
    g_signal_connect(download->actionButton, "clicked", G_CALLBACK(actionButtonClicked), download);
    gtk_box_pack_end(GTK_BOX(mainBox), download->actionButton, FALSE, FALSE, 0);
    gtk_widget_show(download->actionButton);
}

static void ir8_download_class_init(Ir8DownloadClass *klass)
{
    GObjectClass *objectClass = G_OBJECT_CLASS(klass);

    objectClass->finalize = ir8DownloadFinalize;
}

static void downloadReceivedResponse(WebKitDownload *download, GParamSpec *paramSpec, Ir8Download *ir8Download)
{
    WebKitURIResponse *response = webkit_download_get_response(download);
    ir8Download->contentLength = webkit_uri_response_get_content_length(response);
    char *text = g_strdup_printf("Downloading %s", webkit_uri_response_get_uri(response));
    gtk_label_set_text(GTK_LABEL(ir8Download->statusLabel), text);
    g_free(text);
}

static gchar *remainingTime(Ir8Download *ir8Download)
{
    guint64 total = ir8Download->contentLength;
    guint64 current = ir8Download->downloadedSize;
    gdouble elapsedTime = webkit_download_get_elapsed_time(ir8Download->download);

    if (current <= 0)
        return NULL;

    gdouble perByteTime = elapsedTime / current;
    gdouble interval = perByteTime * (total - current);

    int hours = (int) (interval / 3600);
    interval -= hours * 3600;
    int mins = (int) (interval / 60);
    interval -= mins * 60;
    int secs = (int) interval;

    if (hours > 0) {
        if (mins > 0)
            return g_strdup_printf (ngettext ("%u:%02u hour left", "%u:%02u hours left", hours), hours, mins);
        return g_strdup_printf (ngettext ("%u hour left", "%u hours left", hours), hours);
    }

    if (mins > 0)
        return g_strdup_printf (ngettext ("%u:%02u minute left", "%u:%02u minutes left", mins), mins, secs);
    return g_strdup_printf (ngettext ("%u second left", "%u seconds left", secs), secs);
}

static void downloadProgress(WebKitDownload *download, GParamSpec *paramSpec, Ir8Download *ir8Download)
{
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ir8Download->progressBar),
                                  webkit_download_get_estimated_progress(download));
    char *remaining = remainingTime(ir8Download);
    gtk_label_set_text(GTK_LABEL(ir8Download->remainingLabel), remaining);
    g_free(remaining);
}

static void downloadReceivedData(WebKitDownload *download, guint64 dataLength, Ir8Download *ir8Download)
{
    ir8Download->downloadedSize += dataLength;
}

static void downloadFinished(WebKitDownload *download, Ir8Download *ir8Download)
{
    gchar *text = g_strdup_printf("Download completed: %s", webkit_download_get_destination(download));
    gtk_label_set_text(GTK_LABEL(ir8Download->statusLabel), text);
    g_free(text);
    gtk_label_set_text(GTK_LABEL(ir8Download->remainingLabel), NULL);
    gtk_button_set_image(GTK_BUTTON(ir8Download->actionButton), gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_BUTTON));
    gtk_button_set_label(GTK_BUTTON(ir8Download->actionButton), "Open ...");
    ir8Download->finished = TRUE;
}

static void downloadFailed(WebKitDownload *download, GError *error, Ir8Download *ir8Download)
{
    g_signal_handlers_disconnect_by_func(ir8Download->download, downloadFinished, ir8Download);
    if (g_error_matches(error, WEBKIT_DOWNLOAD_ERROR, WEBKIT_DOWNLOAD_ERROR_CANCELLED_BY_USER)) {
        gtk_widget_destroy(GTK_WIDGET(ir8Download));
        return;
    }

    char *errorMessage = g_strdup_printf("Download failed: %s", error->message);
    gtk_label_set_text(GTK_LABEL(ir8Download->statusLabel), errorMessage);
    g_free(errorMessage);
    gtk_label_set_text(GTK_LABEL(ir8Download->remainingLabel), NULL);
    gtk_widget_set_sensitive(ir8Download->actionButton, FALSE);
}

GtkWidget *ir8DownloadNew(WebKitDownload *download)
{
    Ir8Download *ir8Download = IR8_DOWNLOAD(g_object_new(IR8_TYPE_DOWNLOAD,
                                                                     "orientation", GTK_ORIENTATION_VERTICAL,
                                                                     NULL));

    ir8Download->download = g_object_ref(download);
    g_signal_connect(ir8Download->download, "notify::response", G_CALLBACK(downloadReceivedResponse), ir8Download);
    g_signal_connect(ir8Download->download, "notify::estimated-progress", G_CALLBACK(downloadProgress), ir8Download);
    g_signal_connect(ir8Download->download, "received-data", G_CALLBACK(downloadReceivedData), ir8Download);
    g_signal_connect(ir8Download->download, "finished", G_CALLBACK(downloadFinished), ir8Download);
    g_signal_connect(ir8Download->download, "failed", G_CALLBACK(downloadFailed), ir8Download);

    return GTK_WIDGET(ir8Download);
}

void ir8_downloads_bar_add_download(Ir8DownloadsBar *downloadsBar, WebKitDownload *download)
{
    GtkWidget *ir8Download = ir8DownloadNew(download);
    GtkWidget *contentBox = gtk_info_bar_get_content_area(GTK_INFO_BAR(downloadsBar));
    gtk_box_pack_start(GTK_BOX(contentBox), ir8Download, FALSE, TRUE, 0);
    gtk_widget_show(ir8Download);
}

