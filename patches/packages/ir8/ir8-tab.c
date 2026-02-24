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

#if defined(HAVE_CONFIG_H) && HAVE_CONFIG_H && defined(BUILDING_WITH_CMAKE)
#endif
#include "ir8-tab.h"

#include "ir8-searchbox.h"
#include "ir8-window.h"
#include <string.h>

enum {
    PROP_0,

    PROP_VIEW
};

struct _Ir8Tab {
    GtkBox parent;

    WebKitWebView *webView;
    GtkWidget *searchBar;
    GtkWidget *statusLabel;
    gboolean wasSearchingWhenEnteredFullscreen;
    gboolean inspectorIsVisible;
    GtkWidget *fullScreenMessageLabel;
    guint fullScreenMessageLabelId;
    GtkWidget *pointerLockMessageLabel;
    guint pointerLockMessageLabelId;

    /* Tab Title */
    GtkWidget *titleBox;
    GtkWidget *titleLabel;
    GtkWidget *titleSpinner;
    GtkWidget *titleAudioButton;
    GtkWidget *titleCloseButton;
};

static GHashTable *userMediaPermissionGrantedOrigins;
static GHashTable *mediaKeySystemPermissionGrantedOrigins;
static GHashTable *clipboardPermissionGrantedOrigins;
struct _Ir8TabClass {
    GtkBoxClass parent;
};

G_DEFINE_TYPE(Ir8Tab, ir8_tab, GTK_TYPE_BOX)

typedef struct {
    WebKitPermissionRequest *request;
    gchar *origin;
} PermissionRequestData;

static PermissionRequestData *permissionRequestDataNew(WebKitPermissionRequest *request, gchar *origin)
{
    PermissionRequestData *data = g_malloc0(sizeof(PermissionRequestData));

    data->request = g_object_ref(request);
    data->origin = origin;

    return data;
}

static void permissionRequestDataFree(PermissionRequestData *data)
{
    g_clear_object(&data->request);
    g_clear_pointer(&data->origin, g_free);
    g_free(data);
}

static gchar *getWebViewOrigin(WebKitWebView *webView)
{
    WebKitSecurityOrigin *origin = webkit_security_origin_new_for_uri(webkit_web_view_get_uri(webView));
    gchar *originStr = webkit_security_origin_to_string(origin);
    webkit_security_origin_unref(origin);

    return originStr;
}

static void titleChanged(WebKitWebView *webView, GParamSpec *pspec, Ir8Tab *tab)
{
    const char *title = webkit_web_view_get_title(webView);
    if (title && *title)
        gtk_label_set_text(GTK_LABEL(tab->titleLabel), title);
}

static void isLoadingChanged(WebKitWebView *webView, GParamSpec *paramSpec, Ir8Tab *tab)
{
    if (webkit_web_view_is_loading(webView)) {
        gtk_spinner_start(GTK_SPINNER(tab->titleSpinner));
        gtk_widget_show(tab->titleSpinner);
    } else {
        gtk_spinner_stop(GTK_SPINNER(tab->titleSpinner));
        gtk_widget_hide(tab->titleSpinner);
    }
}

static gboolean decidePolicy(WebKitWebView *webView, WebKitPolicyDecision *decision, WebKitPolicyDecisionType decisionType, Ir8Tab *tab)
{
    if (decisionType != WEBKIT_POLICY_DECISION_TYPE_RESPONSE)
        return FALSE;

    WebKitResponsePolicyDecision *responseDecision = WEBKIT_RESPONSE_POLICY_DECISION(decision);
    if (webkit_response_policy_decision_is_mime_type_supported(responseDecision))
        return FALSE;

    if (!webkit_response_policy_decision_is_main_frame_main_resource(responseDecision))
        return FALSE;

    webkit_policy_decision_download(decision);
    return TRUE;
}

static void removeChildIfInfoBar(GtkWidget *child, GtkContainer *tab)
{
    if (GTK_IS_INFO_BAR(child))
        gtk_container_remove(tab, child);
}

static void loadChanged(WebKitWebView *webView, WebKitLoadEvent loadEvent, Ir8Tab *tab)
{
    if (loadEvent != WEBKIT_LOAD_STARTED)
        return;

    gtk_container_foreach(GTK_CONTAINER(tab), (GtkCallback)removeChildIfInfoBar, tab);
}

static GtkWidget *createInfoBarQuestionMessage(const char *title, const char *text)
{
    GtkWidget *dialog = gtk_info_bar_new_with_buttons("No", GTK_RESPONSE_NO, "Yes", GTK_RESPONSE_YES, NULL);
    gtk_info_bar_set_message_type(GTK_INFO_BAR(dialog), GTK_MESSAGE_QUESTION);

    GtkWidget *contentBox = gtk_info_bar_get_content_area(GTK_INFO_BAR(dialog));
    gtk_orientable_set_orientation(GTK_ORIENTABLE(contentBox), GTK_ORIENTATION_VERTICAL);
    gtk_box_set_spacing(GTK_BOX(contentBox), 0);

    GtkLabel *label = GTK_LABEL(gtk_label_new(NULL));
    gchar *markup = g_strdup_printf("<span size='xx-large' weight='bold'>%s</span>", title);
    gtk_label_set_markup(label, markup);
    g_free(markup);
    gtk_label_set_line_wrap(label, TRUE);
    gtk_label_set_selectable(label, TRUE);
    gtk_label_set_xalign(label, 0.);
    gtk_label_set_yalign(label, 0.5);
    gtk_box_pack_start(GTK_BOX(contentBox), GTK_WIDGET(label), FALSE, FALSE, 2);
    gtk_widget_show(GTK_WIDGET(label));

    label = GTK_LABEL(gtk_label_new(text));
    gtk_label_set_line_wrap(label, TRUE);
    gtk_label_set_selectable(label, TRUE);
    gtk_label_set_xalign(label, 0.);
    gtk_label_set_yalign(label, 0.5);
    gtk_box_pack_start(GTK_BOX(contentBox), GTK_WIDGET(label), FALSE, FALSE, 0);
    gtk_widget_show(GTK_WIDGET(label));

    return dialog;
}

static void tlsErrorsDialogResponse(GtkWidget *dialog, gint response, Ir8Tab *tab)
{
    if (response == GTK_RESPONSE_YES) {
        const char *failingURI = (const char *)g_object_get_data(G_OBJECT(dialog), "failingURI");
        GTlsCertificate *certificate = (GTlsCertificate *)g_object_get_data(G_OBJECT(dialog), "certificate");
        SoupURI *uri = soup_uri_new(failingURI);
        webkit_web_context_allow_tls_certificate_for_host(webkit_web_view_get_context(tab->webView), certificate, uri->host);
        soup_uri_free(uri);
        webkit_web_view_load_uri(tab->webView, failingURI);
    }
    gtk_widget_destroy(dialog);
}

static gboolean loadFailedWithTLSerrors(WebKitWebView *webView, const char *failingURI, GTlsCertificate *certificate, GTlsCertificateFlags errors, Ir8Tab *tab)
{
    gchar *text = g_strdup_printf("Failed to load %s: Do you want to continue ignoring the TLS errors?", failingURI);
    GtkWidget *dialog = createInfoBarQuestionMessage("Invalid TLS Certificate", text);
    g_free(text);
    g_object_set_data_full(G_OBJECT(dialog), "failingURI", g_strdup(failingURI), g_free);
    g_object_set_data_full(G_OBJECT(dialog), "certificate", g_object_ref(certificate), g_object_unref);

    g_signal_connect(dialog, "response", G_CALLBACK(tlsErrorsDialogResponse), tab);

    gtk_box_pack_start(GTK_BOX(tab), dialog, FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(tab), dialog, 0);
    gtk_widget_show(dialog);

    return TRUE;
}

static gboolean pointerLockMessageTimeoutCallback(Ir8Tab *tab)
{
    gtk_widget_hide(tab->pointerLockMessageLabel);
    tab->pointerLockMessageLabelId = 0;
    return FALSE;
}

static void permissionRequestDialogResponse(GtkWidget *dialog, gint response, PermissionRequestData *requestData)
{
    switch (response) {
    case GTK_RESPONSE_YES:
        if (WEBKIT_IS_USER_MEDIA_PERMISSION_REQUEST(requestData->request))
            g_hash_table_add(userMediaPermissionGrantedOrigins, g_strdup(requestData->origin));
        if (WEBKIT_IS_MEDIA_KEY_SYSTEM_PERMISSION_REQUEST(requestData->request))
            g_hash_table_add(mediaKeySystemPermissionGrantedOrigins, g_strdup(requestData->origin));
        if (WEBKIT_IS_CLIPBOARD_PERMISSION_REQUEST(requestData->request))
            g_hash_table_add(clipboardPermissionGrantedOrigins, g_strdup(requestData->origin));

        webkit_permission_request_allow(requestData->request);
        break;
    default:
        if (WEBKIT_IS_USER_MEDIA_PERMISSION_REQUEST(requestData->request))
            g_hash_table_remove(userMediaPermissionGrantedOrigins, requestData->origin);
        if (WEBKIT_IS_MEDIA_KEY_SYSTEM_PERMISSION_REQUEST(requestData->request))
            g_hash_table_remove(mediaKeySystemPermissionGrantedOrigins, requestData->origin);
        if (WEBKIT_IS_CLIPBOARD_PERMISSION_REQUEST(requestData->request))
            g_hash_table_remove(clipboardPermissionGrantedOrigins, requestData->origin);

        webkit_permission_request_deny(requestData->request);
        break;
    }

    gtk_widget_destroy(dialog);
    g_clear_pointer(&requestData, permissionRequestDataFree);
}

static gboolean decidePermissionRequest(WebKitWebView *webView, WebKitPermissionRequest *request, Ir8Tab *tab)
{
    const gchar *title = NULL;
    gchar *text = NULL;

    if (WEBKIT_IS_GEOLOCATION_PERMISSION_REQUEST(request)) {
        title = "Geolocation request";
        text = g_strdup("Allow geolocation request?");
    } else if (WEBKIT_IS_NOTIFICATION_PERMISSION_REQUEST(request)) {
        title = "Notification request";
        text = g_strdup("Allow notifications request?");
    } else if (WEBKIT_IS_USER_MEDIA_PERMISSION_REQUEST(request)) {
        title = "UserMedia request";
        gboolean isForAudioDevice = webkit_user_media_permission_is_for_audio_device(WEBKIT_USER_MEDIA_PERMISSION_REQUEST(request));
        gboolean isForVideoDevice = webkit_user_media_permission_is_for_video_device(WEBKIT_USER_MEDIA_PERMISSION_REQUEST(request));
        gboolean isForDisplayDevice = webkit_user_media_permission_is_for_display_device(WEBKIT_USER_MEDIA_PERMISSION_REQUEST(request));

        const char *mediaType = NULL;
        if (isForAudioDevice) {
            if (isForVideoDevice)
                mediaType = "audio/video";
            else
                mediaType = "audio";
        } else if (isForVideoDevice)
            mediaType = "video";
        else if (isForDisplayDevice)
            mediaType = "display";
        text = g_strdup_printf("Allow access to %s device?", mediaType);
    } else if (WEBKIT_IS_DEVICE_INFO_PERMISSION_REQUEST(request)) {
        char* origin = getWebViewOrigin(webView);
        if (g_hash_table_contains(userMediaPermissionGrantedOrigins, origin)) {
            webkit_permission_request_allow(request);
            g_free(origin);
            return TRUE;
        }
        g_free(origin);
        return FALSE;
    } else if (WEBKIT_IS_POINTER_LOCK_PERMISSION_REQUEST(request)) {
        const gchar *titleOrURI = webkit_web_view_get_title(tab->webView);
        if (!titleOrURI || !titleOrURI[0])
            titleOrURI = webkit_web_view_get_uri(tab->webView);

        char *message = g_strdup_printf("%s wants to lock the pointer. Press ESC to get the pointer back.", titleOrURI);
        gtk_label_set_text(GTK_LABEL(tab->pointerLockMessageLabel), message);
        g_free(message);
        gtk_widget_show(tab->pointerLockMessageLabel);

        tab->pointerLockMessageLabelId = g_timeout_add_seconds(2, (GSourceFunc)pointerLockMessageTimeoutCallback, tab);
        g_source_set_name_by_id(tab->pointerLockMessageLabelId, "[WebKit]pointerLockMessageTimeoutCallback");
        return TRUE;
    } else if (WEBKIT_IS_WEBSITE_DATA_ACCESS_PERMISSION_REQUEST(request)) {
        title = "WebsiteData access request";
        WebKitWebsiteDataAccessPermissionRequest *websiteDataAccessRequest = WEBKIT_WEBSITE_DATA_ACCESS_PERMISSION_REQUEST(request);
        const gchar *requestingDomain = webkit_website_data_access_permission_request_get_requesting_domain(websiteDataAccessRequest);
        const gchar *currentDomain = webkit_website_data_access_permission_request_get_current_domain(websiteDataAccessRequest);
        text = g_strdup_printf("Do you want to allow \"%s\" to use cookies while browsing \"%s\"? This will allow \"%s\" to track your activity",
            requestingDomain, currentDomain, requestingDomain);
    } else if (WEBKIT_IS_MEDIA_KEY_SYSTEM_PERMISSION_REQUEST(request)) {
        char *origin = getWebViewOrigin(webView);
        if (g_hash_table_contains(mediaKeySystemPermissionGrantedOrigins, origin)) {
            webkit_permission_request_allow(request);
            g_free(origin);
            return TRUE;
        }
        g_free(origin);
        title = "DRM system access request";
        text = g_strdup_printf("Allow to use a CDM providing access to %s?", webkit_media_key_system_permission_get_name(WEBKIT_MEDIA_KEY_SYSTEM_PERMISSION_REQUEST(request)));
    } else if (WEBKIT_IS_CLIPBOARD_PERMISSION_REQUEST(request)) {
        char *origin = getWebViewOrigin(webView);
        if (g_hash_table_contains(clipboardPermissionGrantedOrigins, origin)) {
            webkit_permission_request_allow(request);
            g_free(origin);
            return TRUE;
        }
        title = "Clipboard access request";
        text = g_strdup_printf("Do you want to allow \"%s\" to read the contents of the clipboard?", origin);
        g_free(origin);
    } else {
        g_print("%s request not handled\n", G_OBJECT_TYPE_NAME(request));
        return FALSE;
    }

    GtkWidget *dialog = createInfoBarQuestionMessage(title, text);
    g_free(text);
    g_signal_connect(dialog, "response", G_CALLBACK(permissionRequestDialogResponse), permissionRequestDataNew(request,
        getWebViewOrigin(webView)));

    gtk_box_pack_start(GTK_BOX(tab), dialog, FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(tab), dialog, 0);
    gtk_widget_show(dialog);

    return TRUE;
}

static void colorChooserRGBAChanged(GtkColorChooser *colorChooser, GParamSpec *paramSpec, WebKitColorChooserRequest *request)
{
    GdkRGBA rgba;
    gtk_color_chooser_get_rgba(colorChooser, &rgba);
    webkit_color_chooser_request_set_rgba(request, &rgba);
}

static void popoverColorClosed(GtkWidget *popover, WebKitColorChooserRequest *request)
{
    webkit_color_chooser_request_finish(request);
}

static void colorChooserRequestFinished(WebKitColorChooserRequest *request, GtkWidget *popover)
{
    g_object_unref(request);
    gtk_widget_destroy(popover);
}

static gboolean runColorChooserCallback(WebKitWebView *webView, WebKitColorChooserRequest *request, Ir8Tab *tab)
{
    GtkWidget *popover = gtk_popover_new(GTK_WIDGET(webView));

    GdkRectangle rectangle;
    webkit_color_chooser_request_get_element_rectangle(request, &rectangle);
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rectangle);

    GtkWidget *colorChooser = gtk_color_chooser_widget_new();
    GdkRGBA rgba;
    webkit_color_chooser_request_get_rgba(request, &rgba);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colorChooser), &rgba);
    g_signal_connect(colorChooser, "notify::rgba", G_CALLBACK(colorChooserRGBAChanged), request);
    gtk_container_add(GTK_CONTAINER(popover), colorChooser);
    gtk_widget_show(colorChooser);

    g_signal_connect_object(popover, "hide", G_CALLBACK(popoverColorClosed), g_object_ref(request), 0);
    g_signal_connect_object(request, "finished", G_CALLBACK(colorChooserRequestFinished), popover, 0);

    gtk_widget_show(popover);

    return TRUE;
}

static void webProcessTerminatedCallback(WebKitWebView *webView, WebKitWebProcessTerminationReason reason)
{
    if (reason == WEBKIT_WEB_PROCESS_CRASHED)
        g_warning("WebProcess CRASHED");
}

static void certificateDialogResponse(GtkDialog *dialog, int response, WebKitAuthenticationRequest *request)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
        if (file) {
            char *path = g_file_get_path(file);
            GError *error = NULL;
            GTlsCertificate *certificate = g_tls_certificate_new_from_file(path, &error);
            if (certificate) {
                WebKitCredential *credential = webkit_credential_new_for_certificate(certificate, WEBKIT_CREDENTIAL_PERSISTENCE_FOR_SESSION);
                webkit_authentication_request_authenticate(request, credential);
                webkit_credential_free(credential);
                g_object_unref(certificate);
            } else {
                g_warning("Failed to create certificate for %s", path);
                g_error_free(error);
            }
            g_free(path);
            g_object_unref(file);
        }
    } else
        webkit_authentication_request_authenticate(request, NULL);

    g_object_unref(request);

    gtk_widget_destroy(GTK_WIDGET(dialog));
}

static gboolean webViewAuthenticate(WebKitWebView *webView, WebKitAuthenticationRequest *request, Ir8Tab *tab)
{
    if (webkit_authentication_request_get_scheme(request) != WEBKIT_AUTHENTICATION_SCHEME_CLIENT_CERTIFICATE_REQUESTED)
        return FALSE;

    GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(tab)));
    GtkWidget *fileChooser = gtk_file_chooser_dialog_new("Certificate required", window, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "PEM Certificate");
    gtk_file_filter_add_mime_type(filter, "application/x-x509-ca-cert");
    gtk_file_filter_add_pattern(filter, "*.pem");
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(fileChooser), filter);

    g_signal_connect(fileChooser, "response", G_CALLBACK(certificateDialogResponse), g_object_ref(request));
    gtk_widget_show(fileChooser);

    return TRUE;
}

static gboolean inspectorOpenedInWindow(WebKitWebInspector *inspector, Ir8Tab *tab)
{
    tab->inspectorIsVisible = TRUE;
    return FALSE;
}

static gboolean inspectorClosed(WebKitWebInspector *inspector, Ir8Tab *tab)
{
    tab->inspectorIsVisible = FALSE;
    return FALSE;
}

static void audioClicked(GtkButton *button, gpointer userData)
{
    Ir8Tab *tab = IR8_TAB(userData);
    gboolean muted = webkit_web_view_get_is_muted(tab->webView);

    webkit_web_view_set_is_muted(tab->webView, !muted);
}

static void audioMutedChanged(WebKitWebView *webView, GParamSpec *pspec, gpointer userData)
{
    Ir8Tab *tab = IR8_TAB(userData);
    gboolean muted = webkit_web_view_get_is_muted(tab->webView);

    gtk_button_set_image(GTK_BUTTON(tab->titleAudioButton), gtk_image_new_from_icon_name(muted ? "audio-volume-muted-symbolic" : "audio-volume-high-symbolic", GTK_ICON_SIZE_MENU));
}


static void ir8TabSetProperty(GObject *object, guint propId, const GValue *value, GParamSpec *pspec)
{
    Ir8Tab *tab = IR8_TAB(object);

    switch (propId) {
    case PROP_VIEW:
        tab->webView = g_value_get_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propId, pspec);
    }
}

static void ir8TabFinalize(GObject *gObject)
{
    Ir8Tab *tab = IR8_TAB(gObject);

    if (tab->fullScreenMessageLabelId)
        g_source_remove(tab->fullScreenMessageLabelId);
    if (tab->pointerLockMessageLabelId)
        g_source_remove(tab->pointerLockMessageLabelId);

    G_OBJECT_CLASS(ir8_tab_parent_class)->finalize(gObject);
}

static void ir8_tab_init(Ir8Tab *tab)
{
    gtk_orientable_set_orientation(GTK_ORIENTABLE(tab), GTK_ORIENTATION_VERTICAL);
}

static void ir8TabConstructed(GObject *gObject)
{
    Ir8Tab *tab = IR8_TAB(gObject);

    G_OBJECT_CLASS(ir8_tab_parent_class)->constructed(gObject);

    tab->searchBar = gtk_search_bar_new();
    GtkWidget *searchBox = ir8_search_box_new(tab->webView);
    gtk_search_bar_set_show_close_button(GTK_SEARCH_BAR(tab->searchBar), TRUE);
    gtk_container_add(GTK_CONTAINER(tab->searchBar), searchBox);
    gtk_widget_show(searchBox);
    gtk_search_bar_connect_entry(GTK_SEARCH_BAR(tab->searchBar),
        ir8_search_box_get_entry(IR8_SEARCH_BOX(searchBox)));
    gtk_container_add(GTK_CONTAINER(tab), tab->searchBar);
    gtk_widget_show(tab->searchBar);

    GtkWidget *overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(tab), overlay);
    gtk_widget_show(overlay);

    tab->statusLabel = gtk_label_new(NULL);
    gtk_widget_set_halign(tab->statusLabel, GTK_ALIGN_START);
    gtk_widget_set_valign(tab->statusLabel, GTK_ALIGN_END);
    gtk_widget_set_margin_start(tab->statusLabel, 1);
    gtk_widget_set_margin_end(tab->statusLabel, 1);
    gtk_widget_set_margin_top(tab->statusLabel, 1);
    gtk_widget_set_margin_bottom(tab->statusLabel, 1);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), tab->statusLabel);

    tab->fullScreenMessageLabel = gtk_label_new(NULL);
    gtk_widget_set_halign(tab->fullScreenMessageLabel, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(tab->fullScreenMessageLabel, GTK_ALIGN_CENTER);
    gtk_widget_set_no_show_all(tab->fullScreenMessageLabel, TRUE);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), tab->fullScreenMessageLabel);

    tab->pointerLockMessageLabel = gtk_label_new(NULL);
    gtk_widget_set_halign(tab->pointerLockMessageLabel, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(tab->pointerLockMessageLabel, GTK_ALIGN_START);
    gtk_widget_set_no_show_all(tab->pointerLockMessageLabel, TRUE);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), tab->pointerLockMessageLabel);

    gtk_widget_set_vexpand(GTK_WIDGET(tab->webView), TRUE);
    gtk_container_add(GTK_CONTAINER(overlay), GTK_WIDGET(tab->webView));
    gtk_widget_show(GTK_WIDGET(tab->webView));

    tab->titleBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(hbox, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(hbox, TRUE);

    tab->titleSpinner = gtk_spinner_new();
    gtk_box_pack_start(GTK_BOX(hbox), tab->titleSpinner, FALSE, FALSE, 0);

    tab->titleLabel = gtk_label_new(NULL);
    gtk_label_set_ellipsize(GTK_LABEL(tab->titleLabel), PANGO_ELLIPSIZE_END);
    gtk_label_set_single_line_mode(GTK_LABEL(tab->titleLabel), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), tab->titleLabel, FALSE, FALSE, 0);
    gtk_widget_show(tab->titleLabel);

    gtk_box_pack_start(GTK_BOX(tab->titleBox), hbox, TRUE, TRUE, 0);
    gtk_widget_show(hbox);

    tab->titleAudioButton = gtk_button_new_from_icon_name("audio-volume-high-symbolic", GTK_ICON_SIZE_MENU);
    gtk_button_set_relief(GTK_BUTTON(tab->titleAudioButton), GTK_RELIEF_NONE);
    g_signal_connect(tab->titleAudioButton, "clicked", G_CALLBACK(audioClicked), tab);
    gtk_widget_set_focus_on_click(tab->titleAudioButton, FALSE);

    gtk_box_pack_start(GTK_BOX(tab->titleBox), tab->titleAudioButton, FALSE, FALSE, 0);

    tab->titleCloseButton = gtk_button_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_MENU);
    gtk_button_set_relief(GTK_BUTTON(tab->titleCloseButton), GTK_RELIEF_NONE);
    g_signal_connect_swapped(tab->titleCloseButton, "clicked", G_CALLBACK(gtk_widget_destroy), tab);
    gtk_widget_set_focus_on_click(tab->titleCloseButton, FALSE);

    gtk_box_pack_start(GTK_BOX(tab->titleBox), tab->titleCloseButton, FALSE, FALSE, 0);
    gtk_widget_show(tab->titleCloseButton);

    g_signal_connect(tab->webView, "notify::title", G_CALLBACK(titleChanged), tab);
    g_signal_connect(tab->webView, "notify::is-loading", G_CALLBACK(isLoadingChanged), tab);
    g_signal_connect(tab->webView, "decide-policy", G_CALLBACK(decidePolicy), tab);
    g_signal_connect(tab->webView, "load-changed", G_CALLBACK(loadChanged), tab);
    g_signal_connect(tab->webView, "load-failed-with-tls-errors", G_CALLBACK(loadFailedWithTLSerrors), tab);
    g_signal_connect(tab->webView, "permission-request", G_CALLBACK(decidePermissionRequest), tab);
    g_signal_connect(tab->webView, "run-color-chooser", G_CALLBACK(runColorChooserCallback), tab);
    g_signal_connect(tab->webView, "web-process-terminated", G_CALLBACK(webProcessTerminatedCallback), NULL);
    g_signal_connect(tab->webView, "authenticate", G_CALLBACK(webViewAuthenticate), tab);

    g_object_bind_property(tab->webView, "is-playing-audio", tab->titleAudioButton, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_signal_connect(tab->webView, "notify::is-muted", G_CALLBACK(audioMutedChanged), tab);

    WebKitWebInspector *inspector = webkit_web_view_get_inspector(tab->webView);
    g_signal_connect(inspector, "open-window", G_CALLBACK(inspectorOpenedInWindow), tab);
    g_signal_connect(inspector, "closed", G_CALLBACK(inspectorClosed), tab);

    if (webkit_web_view_is_editable(tab->webView))
        webkit_web_view_load_html(tab->webView, "<html></html>", "file:///");
}

static void ir8_tab_class_init(Ir8TabClass *klass)
{
    GObjectClass *gobjectClass = G_OBJECT_CLASS(klass);
    gobjectClass->constructed = ir8TabConstructed;
    gobjectClass->set_property = ir8TabSetProperty;
    gobjectClass->finalize = ir8TabFinalize;

    if (!userMediaPermissionGrantedOrigins)
        userMediaPermissionGrantedOrigins = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (!mediaKeySystemPermissionGrantedOrigins)
        mediaKeySystemPermissionGrantedOrigins = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (!clipboardPermissionGrantedOrigins)
        clipboardPermissionGrantedOrigins = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    g_object_class_install_property(
        gobjectClass,
        PROP_VIEW,
        g_param_spec_object(
            "view",
            NULL, NULL,
            WEBKIT_TYPE_WEB_VIEW,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static char *getInternalURI(const char *uri)
{
    if (g_str_equal(uri, "about:gpu"))
        return g_strdup("webkit://gpu");

    /* Internally we use ir8-about: as about: prefix is ignored by WebKit. */
    if (g_str_has_prefix(uri, "about:") && !g_str_equal(uri, "about:blank"))
        return g_strconcat(IR8_ABOUT_SCHEME, uri + strlen ("about"), NULL);

    return g_strdup(uri);
}

/* Public API. */
GtkWidget *ir8_tab_new(WebKitWebView *view)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_VIEW(view), NULL);

    return GTK_WIDGET(g_object_new(IR8_TYPE_TAB, "view", view, NULL));
}

WebKitWebView *ir8_tab_get_web_view(Ir8Tab *tab)
{
    g_return_val_if_fail(IR8_IS_TAB(tab), NULL);

    return tab->webView;
}

void ir8_tab_load_uri(Ir8Tab *tab, const char *uri)
{
    g_return_if_fail(IR8_IS_TAB(tab));
    g_return_if_fail(uri);

    if (!g_str_has_prefix(uri, "javascript:")) {
        char *internalURI = getInternalURI(uri);
        webkit_web_view_load_uri(tab->webView, internalURI);
        g_free(internalURI);
        return;
    }

    webkit_web_view_evaluate_javascript(tab->webView, strstr(uri, "javascript:"), -1, NULL, NULL, NULL, NULL, NULL);
}

GtkWidget *ir8_tab_get_title_widget(Ir8Tab *tab)
{
    g_return_val_if_fail(IR8_IS_TAB(tab), NULL);

    return tab->titleBox;
}

void ir8_tab_set_status_text(Ir8Tab *tab, const char *text)
{
    g_return_if_fail(IR8_IS_TAB(tab));

    gtk_label_set_text(GTK_LABEL(tab->statusLabel), text);
    gtk_widget_set_visible(tab->statusLabel, !!text);
}

void ir8_tab_toggle_inspector(Ir8Tab *tab)
{
    g_return_if_fail(IR8_IS_TAB(tab));

    WebKitWebInspector *inspector = webkit_web_view_get_inspector(tab->webView);
    if (!tab->inspectorIsVisible) {
        webkit_web_inspector_show(inspector);
        tab->inspectorIsVisible = TRUE;
    } else
        webkit_web_inspector_close(inspector);
}

static gboolean ir8TabIsSearchBarOpen(Ir8Tab *tab)
{
    GtkWidget *revealer = gtk_bin_get_child(GTK_BIN(tab->searchBar));
    return gtk_revealer_get_reveal_child(GTK_REVEALER(revealer));
}

void ir8_tab_start_search(Ir8Tab *tab)
{
    g_return_if_fail(IR8_IS_TAB(tab));
    if (!ir8TabIsSearchBarOpen(tab))
        gtk_search_bar_set_search_mode(GTK_SEARCH_BAR(tab->searchBar), TRUE);
}

void ir8_tab_stop_search(Ir8Tab *tab)
{
    g_return_if_fail(IR8_IS_TAB(tab));
    if (ir8TabIsSearchBarOpen(tab))
        gtk_search_bar_set_search_mode(GTK_SEARCH_BAR(tab->searchBar), FALSE);
}

static gboolean fullScreenMessageTimeoutCallback(Ir8Tab *tab)
{
    gtk_widget_hide(tab->fullScreenMessageLabel);
    tab->fullScreenMessageLabelId = 0;
    return FALSE;
}

void ir8_tab_enter_fullscreen(Ir8Tab *tab)
{
    g_return_if_fail(IR8_IS_TAB(tab));

    const gchar *titleOrURI = webkit_web_view_get_title(tab->webView);
    if (!titleOrURI || !titleOrURI[0])
        titleOrURI = webkit_web_view_get_uri(tab->webView);

    gchar *message = g_strdup_printf("%s is now full screen. Press ESC or f to exit.", titleOrURI);
    gtk_label_set_text(GTK_LABEL(tab->fullScreenMessageLabel), message);
    g_free(message);

    gtk_widget_show(tab->fullScreenMessageLabel);

    tab->fullScreenMessageLabelId = g_timeout_add_seconds(2, (GSourceFunc)fullScreenMessageTimeoutCallback, tab);
    g_source_set_name_by_id(tab->fullScreenMessageLabelId, "[WebKit] fullScreenMessageTimeoutCallback");

    tab->wasSearchingWhenEnteredFullscreen = ir8TabIsSearchBarOpen(tab);
    ir8_tab_stop_search(tab);
}

void ir8_tab_leave_fullscreen(Ir8Tab *tab)
{
    g_return_if_fail(IR8_IS_TAB(tab));

    if (tab->fullScreenMessageLabelId) {
        g_source_remove(tab->fullScreenMessageLabelId);
        tab->fullScreenMessageLabelId = 0;
    }

    gtk_widget_hide(tab->fullScreenMessageLabel);

    if (tab->wasSearchingWhenEnteredFullscreen) {
        /* Opening the search bar steals the focus. Usually, we want
         * this but not when coming back from fullscreen.
         */
        GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(tab)));
        GtkWidget *focusWidget = gtk_window_get_focus(window);
        ir8_tab_start_search(tab);
        gtk_window_set_focus(window, focusWidget);
    }
}

void ir8_tab_set_background_color(Ir8Tab *tab, GdkRGBA *rgba)
{
    g_return_if_fail(IR8_IS_TAB(tab));
    g_return_if_fail(rgba);

    GdkRGBA viewRGBA;
    webkit_web_view_get_background_color(tab->webView, &viewRGBA);
    if (gdk_rgba_equal(rgba, &viewRGBA))
        return;

    webkit_web_view_set_background_color(tab->webView, rgba);
}
