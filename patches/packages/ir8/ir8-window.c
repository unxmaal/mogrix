/*
 * Copyright (C) 2006, 2007 Apple Inc.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
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

#if defined(HAVE_CONFIG_H) && HAVE_CONFIG_H && defined(BUILDING_WITH_CMAKE)
#endif
#include "ir8-window.h"

#include "ir8-downloads.h"
#include "ir8-settings.h"
#include "ir8-tab.h"
#include <gdk/gdkkeysyms.h>
#include <string.h>

struct _Ir8Window {
    GtkApplicationWindow parent;

    WebKitWebContext *webContext;

    GtkWidget *mainBox;
    GtkWidget *toolbar;
    GtkWidget *uriEntry;
    GtkWidget *backItem;
    GtkWidget *forwardItem;
    GtkWidget *editToolbar;
    GtkWidget *settingsDialog;
    GtkWidget *notebook;
    GActionGroup *editActionGroup;
    Ir8Tab *activeTab;
    GtkWidget *downloadsBar;
    gboolean searchBarVisible;
    gboolean fullScreenIsEnabled;
    GdkPixbuf *favicon;
    GtkWidget *reloadOrStopButton;
    GtkWindow *parentWindow;
    guint resetEntryProgressTimeoutId;
    gchar *sessionFile;
    GdkRGBA backgroundColor;
};

struct _Ir8WindowClass {
    GtkApplicationWindowClass parent;
};

static const char *defaultWindowTitle = "ir8";
static const gdouble minimumZoomLevel = 0.5;
static const gdouble maximumZoomLevel = 3;
static const gdouble defaultZoomLevel = 1;
static const gdouble zoomStep = 1.2;

G_DEFINE_TYPE(Ir8Window, ir8_window, GTK_TYPE_APPLICATION_WINDOW)

static char *getExternalURI(const char *uri)
{
    /* From the user point of view we support about: prefix. */
    if (uri && g_str_has_prefix(uri, IR8_ABOUT_SCHEME))
        return g_strconcat("about", uri + strlen(IR8_ABOUT_SCHEME), NULL);

    return g_strdup(uri);
}

static void ir8WindowSetStatusText(Ir8Window *window, const char *text)
{
    ir8_tab_set_status_text(window->activeTab, text);
}

static void activateUriEntryCallback(Ir8Window *window)
{
    ir8_window_load_uri(window,
        gtk_entry_get_text(GTK_ENTRY(window->uriEntry))
    );
}

static void reloadOrStopCallback(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    if (webkit_web_view_is_loading(webView))
        webkit_web_view_stop_loading(webView);
    else
        webkit_web_view_reload(webView);
}

static void goBackCallback(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    WebKitWebView *webView = ir8_tab_get_web_view(IR8_WINDOW(userData)->activeTab);
    webkit_web_view_go_back(webView);
}

static void goForwardCallback(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    WebKitWebView *webView = ir8_tab_get_web_view(IR8_WINDOW(userData)->activeTab);
    webkit_web_view_go_forward(webView);
}

static void settingsCallback(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    if (window->settingsDialog) {
        gtk_window_present(GTK_WINDOW(window->settingsDialog));
        return;
    }

    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    window->settingsDialog = ir8_settings_dialog_new(webkit_web_view_get_settings(webView));
    gtk_window_set_transient_for(GTK_WINDOW(window->settingsDialog), GTK_WINDOW(window));
    g_object_add_weak_pointer(G_OBJECT(window->settingsDialog), (gpointer *)&window->settingsDialog);
    gtk_widget_show(window->settingsDialog);
}

static void webViewURIChanged(WebKitWebView *webView, GParamSpec *pspec, Ir8Window *window)
{
    char *externalURI = getExternalURI(webkit_web_view_get_uri(webView));
    gtk_entry_set_text(GTK_ENTRY(window->uriEntry), externalURI ? externalURI : "");
    g_free(externalURI);
}

static void webViewTitleChanged(WebKitWebView *webView, GParamSpec *pspec, Ir8Window *window)
{
    const char *title = webkit_web_view_get_title(webView);
    if (!title || !*title)
        title = defaultWindowTitle;
    char *windowTitle = NULL;
    if (webkit_web_view_is_ephemeral(webView))
        windowTitle = g_strdup_printf("[Private] %s — ir8", title);
    else
        windowTitle = g_strdup_printf("%s — ir8", title);
    gtk_window_set_title(GTK_WINDOW(window), windowTitle);
    g_free(windowTitle);
}

static gboolean resetEntryProgress(Ir8Window *window)
{
    gtk_entry_set_progress_fraction(GTK_ENTRY(window->uriEntry), 0);
    window->resetEntryProgressTimeoutId = 0;
    return FALSE;
}

static void webViewLoadProgressChanged(WebKitWebView *webView, GParamSpec *pspec, Ir8Window *window)
{
    gdouble progress = webkit_web_view_get_estimated_load_progress(webView);
    gtk_entry_set_progress_fraction(GTK_ENTRY(window->uriEntry), progress);
    if (progress == 1.0) {
        window->resetEntryProgressTimeoutId = g_timeout_add(500, (GSourceFunc)resetEntryProgress, window);
        g_source_set_name_by_id(window->resetEntryProgressTimeoutId, "[WebKit] resetEntryProgress");
    } else if (window->resetEntryProgressTimeoutId) {
        g_source_remove(window->resetEntryProgressTimeoutId);
        window->resetEntryProgressTimeoutId = 0;
    }
}

static void downloadStarted(WebKitWebContext *webContext, WebKitDownload *download, Ir8Window *window)
{
    if (!window->downloadsBar) {
        window->downloadsBar = ir8_downloads_bar_new();
        gtk_box_pack_start(GTK_BOX(window->mainBox), window->downloadsBar, FALSE, FALSE, 0);
        gtk_box_reorder_child(GTK_BOX(window->mainBox), window->downloadsBar, 0);
        g_object_add_weak_pointer(G_OBJECT(window->downloadsBar), (gpointer *)&(window->downloadsBar));
        gtk_widget_show(window->downloadsBar);
    }
    ir8_downloads_bar_add_download(IR8_DOWNLOADS_BAR(window->downloadsBar), download);
}

static void ir8WindowHistoryItemActivated(Ir8Window *window, GVariant *parameter, GAction *action)
{
    WebKitBackForwardListItem *item = g_object_get_data(G_OBJECT(action), "back-forward-list-item");
    if (!item)
        return;

    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    webkit_web_view_go_to_back_forward_list_item(webView, item);
}

static void ir8WindowCreateBackForwardMenu(Ir8Window *window, GList *list, gboolean isBack)
{
    if (!list)
        return;

    static guint64 actionId = 0;

    GSimpleActionGroup *actionGroup = g_simple_action_group_new();
    GMenu *menu = g_menu_new();
    GList *listItem;
    for (listItem = list; listItem; listItem = g_list_next(listItem)) {
        WebKitBackForwardListItem *item = (WebKitBackForwardListItem *)listItem->data;
        const char *title = webkit_back_forward_list_item_get_title(item);
        if (!title || !*title)
            title = webkit_back_forward_list_item_get_uri(item);

        char *displayTitle;
#define MAX_TITLE 100
        displayTitle = g_strndup(title, MIN(MAX_TITLE, strlen(title)));
        if (strlen(title) > MAX_TITLE) {
            displayTitle[MAX_TITLE - 1] = 0xA6;
            displayTitle[MAX_TITLE - 2] = 0x80;
            displayTitle[MAX_TITLE - 3] = 0xE2;
        }
#undef MAX_TITLE

        char *actionName = g_strdup_printf("action-%lu", ++actionId);
        GSimpleAction *action = g_simple_action_new(actionName, NULL);
        g_object_set_data_full(G_OBJECT(action), "back-forward-list-item", g_object_ref(item), g_object_unref);
        g_signal_connect_swapped(action, "activate", G_CALLBACK(ir8WindowHistoryItemActivated), window);
        g_action_map_add_action(G_ACTION_MAP(actionGroup), G_ACTION(action));
        g_object_unref(action);

        char *detailedActionName = g_strdup_printf("%s.%s", isBack ? "bf-back" : "bf-forward", actionName);
        GMenuItem *menuItem = g_menu_item_new(displayTitle, detailedActionName);
        g_menu_append_item(menu, menuItem);
        g_object_unref(menuItem);

        g_free(displayTitle);
        g_free(detailedActionName);
        g_free(actionName);
    }

    GtkWidget *popover = gtk_popover_menu_new();
    gtk_popover_bind_model(GTK_POPOVER(popover), G_MENU_MODEL(menu), NULL);
    g_object_unref(menu);
    gtk_widget_insert_action_group(popover, isBack ? "bf-back" : "bf-forward", G_ACTION_GROUP(actionGroup));
    g_object_unref(actionGroup);

    GtkWidget *button = isBack ? window->backItem : window->forwardItem;
    gtk_popover_set_relative_to(GTK_POPOVER(popover), button);
    g_object_set_data(G_OBJECT(button), "history-popover", popover);
    gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_BOTTOM);
}

static void ir8WindowUpdateNavigationMenu(Ir8Window *window, WebKitBackForwardList *backForwardlist)
{
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    GAction *action = g_action_map_lookup_action(G_ACTION_MAP(window), "go-back");
    g_simple_action_set_enabled(G_SIMPLE_ACTION(action), webkit_web_view_can_go_back(webView));
    action = g_action_map_lookup_action(G_ACTION_MAP(window), "go-forward");
    g_simple_action_set_enabled(G_SIMPLE_ACTION(action), webkit_web_view_can_go_forward(webView));

    GList *list = g_list_reverse(webkit_back_forward_list_get_back_list_with_limit(backForwardlist, 10));
    ir8WindowCreateBackForwardMenu(window, list, TRUE);
    g_list_free(list);

    list = webkit_back_forward_list_get_forward_list_with_limit(backForwardlist, 10);
    ir8WindowCreateBackForwardMenu(window, list, FALSE);
    g_list_free(list);
}

static gboolean navigationButtonPressCallback(GtkButton *button, GdkEvent *event, Ir8Window *window)
{
    guint eventButton;
    gdk_event_get_button(event, &eventButton);
    if (eventButton != GDK_BUTTON_SECONDARY)
        return GDK_EVENT_PROPAGATE;

    GtkWidget *popover = g_object_get_data(G_OBJECT(button), "history-popover");
    if (!popover)
        return GDK_EVENT_PROPAGATE;

    gtk_popover_popup(GTK_POPOVER(popover));

    return GDK_EVENT_STOP;
}

static void ir8WindowSaveSession(Ir8Window *window)
{
    if (!window->sessionFile)
        return;

    GKeyFile *session = g_key_file_new();
    int tabsCount = gtk_notebook_get_n_pages(GTK_NOTEBOOK(window->notebook));
    int i;
    for (i = 0; i < tabsCount; ++i) {
        Ir8Tab *tab = (Ir8Tab *)gtk_notebook_get_nth_page(GTK_NOTEBOOK(window->notebook), i);
        WebKitWebView *webView = ir8_tab_get_web_view(tab);
        WebKitWebViewSessionState *state = webkit_web_view_get_session_state(webView);
        GBytes *bytes = webkit_web_view_session_state_serialize(state);
        if (bytes) {
            gsize dataLength;
            gconstpointer data;
            data = g_bytes_get_data(bytes, &dataLength);

            gchar *groupName = g_strdup_printf("Tab-%d", i);
            gchar *base64 = g_base64_encode(data, dataLength);
            g_key_file_set_string(session, groupName, "state", base64);
            g_free(base64);
            g_free(groupName);
            g_bytes_unref(bytes);
        }
        webkit_web_view_session_state_unref(state);
    }
    g_key_file_save_to_file(session, window->sessionFile, NULL);
    g_key_file_free(session);
}

static void ir8WindowTryCloseCurrentWebView(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    int currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(window->notebook));
    Ir8Tab *tab = (Ir8Tab *)gtk_notebook_get_nth_page(GTK_NOTEBOOK(window->notebook), currentPage);
    webkit_web_view_try_close(ir8_tab_get_web_view(tab));
}

static void ir8WindowTryClose(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    ir8WindowSaveSession(window);

    GSList *webViews = NULL;
    int n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(window->notebook));
    int i;

    for (i = 0; i < n; ++i) {
        Ir8Tab *tab = (Ir8Tab *)gtk_notebook_get_nth_page(GTK_NOTEBOOK(window->notebook), i);
        webViews = g_slist_prepend(webViews, ir8_tab_get_web_view(tab));
    }

    GSList *link;
    for (link = webViews; link; link = link->next)
        webkit_web_view_try_close(link->data);
}

static void backForwardlistChanged(WebKitBackForwardList *backForwardlist, WebKitBackForwardListItem *itemAdded, GList *itemsRemoved, Ir8Window *window)
{
    ir8WindowUpdateNavigationMenu(window, backForwardlist);
}

static void webViewClose(WebKitWebView *webView, Ir8Window *window)
{
    int tabsCount = gtk_notebook_get_n_pages(GTK_NOTEBOOK(window->notebook));
    if (tabsCount == 1) {
        gtk_widget_destroy(GTK_WIDGET(window));
        return;
    }

    int i;
    for (i = 0; i < tabsCount; ++i) {
        Ir8Tab *tab = (Ir8Tab *)gtk_notebook_get_nth_page(GTK_NOTEBOOK(window->notebook), i);
        if (ir8_tab_get_web_view(tab) == webView) {
            gtk_widget_destroy(GTK_WIDGET(tab));
            return;
        }
    }
}

static void webViewRunAsModal(WebKitWebView *webView, Ir8Window *window)
{
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(window), window->parentWindow);
}

static void webViewReadyToShow(WebKitWebView *webView, Ir8Window *window)
{
    WebKitWindowProperties *windowProperties = webkit_web_view_get_window_properties(webView);

    GdkRectangle geometry;
    webkit_window_properties_get_geometry(windowProperties, &geometry);
    if (geometry.x >= 0 && geometry.y >= 0)
        gtk_window_move(GTK_WINDOW(window), geometry.x, geometry.y);
    if (geometry.width > 0 && geometry.height > 0)
        gtk_window_resize(GTK_WINDOW(window), geometry.width, geometry.height);

    if (!webkit_window_properties_get_toolbar_visible(windowProperties))
        gtk_widget_hide(window->toolbar);
    else if (!webkit_window_properties_get_locationbar_visible(windowProperties))
        gtk_widget_hide(window->uriEntry);

    if (!webkit_window_properties_get_resizable(windowProperties))
        gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    gtk_widget_show(GTK_WIDGET(window));
}

static GtkWidget *webViewCreate(WebKitWebView *webView, WebKitNavigationAction *navigation, Ir8Window *window)
{
    WebKitWebView *newWebView = WEBKIT_WEB_VIEW(g_object_new(WEBKIT_TYPE_WEB_VIEW,
        "related-view", webView,
        "settings", webkit_web_view_get_settings(webView),
        "user-content-manager", webkit_web_view_get_user_content_manager(webView),
        "website-policies", webkit_web_view_get_website_policies(webView),
        NULL));

    GtkWidget *newWindow = ir8_window_new(GTK_WINDOW(window), window->webContext);
    gtk_window_set_application(GTK_WINDOW(newWindow), gtk_window_get_application(GTK_WINDOW(window)));
    ir8_window_append_view(IR8_WINDOW(newWindow), newWebView);
    gtk_widget_grab_focus(GTK_WIDGET(newWebView));
    g_signal_connect(newWebView, "ready-to-show", G_CALLBACK(webViewReadyToShow), newWindow);
    g_signal_connect(newWebView, "run-as-modal", G_CALLBACK(webViewRunAsModal), newWindow);
    return GTK_WIDGET(newWebView);
}

static gboolean webViewEnterFullScreen(WebKitWebView *webView, Ir8Window *window)
{
    gtk_widget_hide(window->toolbar);
    ir8_tab_enter_fullscreen(window->activeTab);
    return FALSE;
}

static gboolean webViewLeaveFullScreen(WebKitWebView *webView, Ir8Window *window)
{
    ir8_tab_leave_fullscreen(window->activeTab);
    gtk_widget_show(window->toolbar);
    return FALSE;
}

static gboolean webViewLoadFailed(WebKitWebView *webView, WebKitLoadEvent loadEvent, const char *failingURI, GError *error, Ir8Window *window)
{
    gtk_entry_set_progress_fraction(GTK_ENTRY(window->uriEntry), 0.);
    return FALSE;
}

static gboolean webViewDecidePolicy(WebKitWebView *webView, WebKitPolicyDecision *decision, WebKitPolicyDecisionType decisionType, Ir8Window *window)
{
    if (decisionType != WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION)
        return FALSE;

    WebKitNavigationAction *navigationAction = webkit_navigation_policy_decision_get_navigation_action(WEBKIT_NAVIGATION_POLICY_DECISION(decision));
    if (webkit_navigation_action_get_navigation_type(navigationAction) != WEBKIT_NAVIGATION_TYPE_LINK_CLICKED
        || webkit_navigation_action_get_mouse_button(navigationAction) != GDK_BUTTON_MIDDLE)
        return FALSE;

    /* Multiple tabs are not allowed in editor mode. */
    if (webkit_web_view_is_editable(webView))
        return FALSE;

    /* Opening a new tab if link clicked with the middle button. */
    WebKitWebView *newWebView = WEBKIT_WEB_VIEW(g_object_new(WEBKIT_TYPE_WEB_VIEW,
        "web-context", webkit_web_view_get_context(webView),
        "settings", webkit_web_view_get_settings(webView),
        "user-content-manager", webkit_web_view_get_user_content_manager(webView),
        "website-policies", webkit_web_view_get_website_policies(webView),
        NULL));
    ir8_window_append_view(window, newWebView);
    webkit_web_view_load_request(newWebView, webkit_navigation_action_get_request(navigationAction));

    webkit_policy_decision_ignore(decision);
    return TRUE;
}

static void webViewMouseTargetChanged(WebKitWebView *webView, WebKitHitTestResult *hitTestResult, guint mouseModifiers, Ir8Window *window)
{
    if (!webkit_hit_test_result_context_is_link(hitTestResult)) {
        ir8WindowSetStatusText(window, NULL);
        return;
    }
    ir8WindowSetStatusText(window, webkit_hit_test_result_get_link_uri(hitTestResult));
}

static gboolean ir8WindowCanZoomIn(Ir8Window *window)
{
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    gdouble zoomLevel = webkit_web_view_get_zoom_level(webView) * zoomStep;
    return zoomLevel < maximumZoomLevel;
}

static gboolean ir8WindowCanZoomOut(Ir8Window *window)
{
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    gdouble zoomLevel = webkit_web_view_get_zoom_level(webView) / zoomStep;
    return zoomLevel > minimumZoomLevel;
}

static gboolean ir8WindowCanZoomDefault(Ir8Window *window)
{
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    return webkit_web_view_get_zoom_level(webView) != 1.0;
}

static gboolean ir8WindowZoomIn(Ir8Window *window)
{
    if (ir8WindowCanZoomIn(window)) {
        WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
        gdouble zoomLevel = webkit_web_view_get_zoom_level(webView) * zoomStep;
        webkit_web_view_set_zoom_level(webView, zoomLevel);
        return TRUE;
    }
    return FALSE;
}

static gboolean ir8WindowZoomOut(Ir8Window *window)
{
    if (ir8WindowCanZoomOut(window)) {
        WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
        gdouble zoomLevel = webkit_web_view_get_zoom_level(webView) / zoomStep;
        webkit_web_view_set_zoom_level(webView, zoomLevel);
        return TRUE;
    }
    return FALSE;
}

static gboolean scrollEventCallback(WebKitWebView *webView, const GdkEventScroll *event, Ir8Window *window)
{
    GdkModifierType mod = gtk_accelerator_get_default_mod_mask();

    if ((event->state & mod) != GDK_CONTROL_MASK)
        return FALSE;

    if (event->delta_y < 0)
        return ir8WindowZoomIn(window);

    return ir8WindowZoomOut(window);
}

static void ir8WindowUpdateZoomActions(Ir8Window *window)
{
    GAction *action = g_action_map_lookup_action(G_ACTION_MAP(window), "zoom-in");
    g_simple_action_set_enabled(G_SIMPLE_ACTION(action), ir8WindowCanZoomIn(window));
    action = g_action_map_lookup_action(G_ACTION_MAP(window), "zoom-out");
    g_simple_action_set_enabled(G_SIMPLE_ACTION(action), ir8WindowCanZoomOut(window));
    action = g_action_map_lookup_action(G_ACTION_MAP(window), "zoom-default");
    g_simple_action_set_enabled(G_SIMPLE_ACTION(action), ir8WindowCanZoomDefault(window));
}

static void webViewZoomLevelChanged(GObject *object, GParamSpec *paramSpec, Ir8Window *window)
{
    ir8WindowUpdateZoomActions(window);
}

static void updateUriEntryIcon(Ir8Window *window)
{
    GtkEntry *entry = GTK_ENTRY(window->uriEntry);
    if (window->favicon)
        gtk_entry_set_icon_from_pixbuf(entry, GTK_ENTRY_ICON_PRIMARY, window->favicon);
    else
        gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_PRIMARY, "document-new");
}

static void faviconChanged(WebKitWebView *webView, GParamSpec *paramSpec, Ir8Window *window)
{
    cairo_surface_t *surface = webkit_web_view_get_favicon(webView);
    GdkPixbuf *favicon = NULL;

    if (surface) {
        int width = cairo_image_surface_get_width(surface);
        int height = cairo_image_surface_get_height(surface);

        favicon = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
    }

    if (window->favicon)
        g_object_unref(window->favicon);
    window->favicon = favicon;

    updateUriEntryIcon(window);
}

static void webViewMediaCaptureStateChanged(WebKitWebView* webView, GParamSpec* paramSpec, Ir8Window* window)
{
    const gchar* name = g_param_spec_get_name(paramSpec);
    // FIXME: the URI entry is not great storage in case more than one capture device is in use,
    // because it can store only one secondary icon.
    GtkEntry *entry = GTK_ENTRY(window->uriEntry);

    if (g_str_has_prefix(name, "microphone")) {
        switch (webkit_web_view_get_microphone_capture_state(webView)) {
        case WEBKIT_MEDIA_CAPTURE_STATE_NONE:
            gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, NULL);
            break;
        case WEBKIT_MEDIA_CAPTURE_STATE_MUTED:
            gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, "microphone-sensivity-mutes-symbolic");
            break;
        case WEBKIT_MEDIA_CAPTURE_STATE_ACTIVE:
            gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, "audio-input-microphone-symbolic");
            break;
        }
    } else if (g_str_has_prefix(name, "camera")) {
        switch (webkit_web_view_get_camera_capture_state(webView)) {
        case WEBKIT_MEDIA_CAPTURE_STATE_NONE:
            gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, NULL);
            break;
        case WEBKIT_MEDIA_CAPTURE_STATE_MUTED:
            gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, "camera-disabled-symbolic");
            break;
        case WEBKIT_MEDIA_CAPTURE_STATE_ACTIVE:
            gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, "camera-web-symbolic");
            break;
        }
    } else if (g_str_has_prefix(name, "display")) {
        switch (webkit_web_view_get_display_capture_state(webView)) {
        case WEBKIT_MEDIA_CAPTURE_STATE_NONE:
            gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, NULL);
            break;
        case WEBKIT_MEDIA_CAPTURE_STATE_MUTED:
            // FIXME: I found no suitable icon for this.
            gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, "media-playback-stop-symbolic");
            break;
        case WEBKIT_MEDIA_CAPTURE_STATE_ACTIVE:
            gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY, "video-display-symbolic");
            break;
        }
    }
}

static void webViewUriEntryIconPressed(GtkEntry* entry, GtkEntryIconPosition position, GdkEvent* event, Ir8Window* window)
{
    if (position != GTK_ENTRY_ICON_SECONDARY)
        return;

    // FIXME: What about audio/video?
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    switch (webkit_web_view_get_display_capture_state(webView)) {
    case WEBKIT_MEDIA_CAPTURE_STATE_NONE:
        break;
    case WEBKIT_MEDIA_CAPTURE_STATE_MUTED:
        webkit_web_view_set_display_capture_state(webView, WEBKIT_MEDIA_CAPTURE_STATE_ACTIVE);
        break;
    case WEBKIT_MEDIA_CAPTURE_STATE_ACTIVE:
        webkit_web_view_set_display_capture_state(webView, WEBKIT_MEDIA_CAPTURE_STATE_MUTED);
        break;
    }
}

static void webViewIsLoadingChanged(WebKitWebView *webView, GParamSpec *paramSpec, Ir8Window *window)
{
    gboolean isLoading = webkit_web_view_is_loading(webView);
    GtkWidget *image = gtk_button_get_image(GTK_BUTTON(window->reloadOrStopButton));
    g_object_set(image, "icon-name", isLoading ? "process-stop-symbolic" : "view-refresh-symbolic", NULL);
    GAction *action = g_action_map_lookup_action(G_ACTION_MAP(window), "stop-load");
    g_simple_action_set_enabled(G_SIMPLE_ACTION(action), isLoading);
}

static void zoomInCallback(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    ir8WindowZoomIn(IR8_WINDOW(userData));
}

static void zoomOutCallback(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    ir8WindowZoomOut(IR8_WINDOW(userData));
}

static void defaultZoomCallback(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    WebKitWebView *webView = ir8_tab_get_web_view(IR8_WINDOW(userData)->activeTab);
    webkit_web_view_set_zoom_level(webView, defaultZoomLevel);
}

static void searchCallback(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    ir8_tab_start_search(IR8_WINDOW(userData)->activeTab);
}

static void newTabCallback(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    if (webkit_web_view_is_editable(webView))
        return;

    ir8_window_append_view(window, WEBKIT_WEB_VIEW(g_object_new(WEBKIT_TYPE_WEB_VIEW,
        "web-context", webkit_web_view_get_context(webView),
        "settings", webkit_web_view_get_settings(webView),
        "user-content-manager", webkit_web_view_get_user_content_manager(webView),
        "website-policies", webkit_web_view_get_website_policies(webView),
        NULL)));
    gtk_widget_grab_focus(window->uriEntry);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(window->notebook), -1);
}

static void toggleWebInspector(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    ir8_tab_toggle_inspector(IR8_WINDOW(userData)->activeTab);
}

static void openPrivateWindow(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);

    WebKitWebView *newWebView = WEBKIT_WEB_VIEW(g_object_new(WEBKIT_TYPE_WEB_VIEW,
        "web-context", webkit_web_view_get_context(webView),
        "settings", webkit_web_view_get_settings(webView),
        "user-content-manager", webkit_web_view_get_user_content_manager(webView),
        "is-ephemeral", TRUE,
        "website-policies", webkit_web_view_get_website_policies(webView),
        NULL));
    GtkWidget *newWindow = ir8_window_new(GTK_WINDOW(window), window->webContext);
    gtk_window_set_application(GTK_WINDOW(newWindow), gtk_window_get_application(GTK_WINDOW(window)));
    ir8_window_append_view(IR8_WINDOW(newWindow), newWebView);
    gtk_widget_grab_focus(GTK_WIDGET(newWebView));
    gtk_widget_show(GTK_WIDGET(newWindow));
}

static void focusLocationBar(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    gtk_widget_grab_focus(IR8_WINDOW(userData)->uriEntry);
}

static void reloadPage(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    WebKitWebView *webView = ir8_tab_get_web_view(IR8_WINDOW(userData)->activeTab);
    webkit_web_view_reload(webView);
}

static void reloadPageIgnoringCache(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    WebKitWebView *webView = ir8_tab_get_web_view(IR8_WINDOW(userData)->activeTab);
    webkit_web_view_reload_bypass_cache(webView);
}

static void stopPageLoad(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    if (webkit_web_view_is_loading(webView))
        webkit_web_view_stop_loading(webView);
}

static void loadHomePage(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    webkit_web_view_load_uri(webView, IR8_DEFAULT_URL);
}

static void toggleFullScreen(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    if (!window->fullScreenIsEnabled) {
        gtk_window_fullscreen(GTK_WINDOW(window));
        gtk_widget_hide(window->toolbar);
        window->fullScreenIsEnabled = TRUE;
    } else {
        gtk_window_unfullscreen(GTK_WINDOW(window));
        gtk_widget_show(window->toolbar);
        window->fullScreenIsEnabled = FALSE;
    }
}

static void webKitPrintOperationFailedCallback(WebKitPrintOperation *printOperation, GError *error)
{
    g_warning("Print failed: '%s'", error->message);
}

static void printPage(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    WebKitPrintOperation *printOperation = webkit_print_operation_new(webView);

    g_signal_connect(printOperation, "failed", G_CALLBACK(webKitPrintOperationFailedCallback), NULL);
    webkit_print_operation_run_dialog(printOperation, GTK_WINDOW(window));
    g_object_unref(printOperation);
}

static void editingActionCallback(GSimpleAction *action, GVariant *prameter, gpointer userData)
{
    Ir8Window *window = IR8_WINDOW(userData);
    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    webkit_web_view_execute_editing_command(webView, g_action_get_name(G_ACTION(action)));
}

static void insertImageDialogResponse(GtkDialog *dialog, int response, Ir8Window *window)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
        if (file) {
            char *uri = g_file_get_uri(file);
            WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
            webkit_web_view_execute_editing_command_with_argument(webView, WEBKIT_EDITING_COMMAND_INSERT_IMAGE, uri);
            g_free(uri);
            g_object_unref(file);
        }
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void insertImageCommandCallback(GtkWidget *widget, Ir8Window *window)
{
    GtkWidget *fileChooser = gtk_file_chooser_dialog_new("Insert Image", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Images");
    gtk_file_filter_add_pixbuf_formats(filter);
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(fileChooser), filter);

    g_signal_connect(fileChooser, "response", G_CALLBACK(insertImageDialogResponse), window);
    gtk_widget_show(fileChooser);
}

typedef struct {
    GtkWidget *entry;
    Ir8Window *window;
} InsertLinkDialogData;

static void insertLinkDialogResponse(GtkDialog *dialog, int response, InsertLinkDialogData *data)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *url = gtk_entry_get_text(GTK_ENTRY(data->entry));
        if (url && *url) {
            WebKitWebView *webView = ir8_tab_get_web_view(data->window->activeTab);
            webkit_web_view_execute_editing_command_with_argument(webView, WEBKIT_EDITING_COMMAND_CREATE_LINK, url);
        }
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));

    g_free(data);
}

static void insertLinkCommandCallback(GtkWidget *widget, Ir8Window *window)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Insert Link", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Insert", GTK_RESPONSE_ACCEPT, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "URL");
    gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), entry);
    gtk_widget_show(entry);

    InsertLinkDialogData *data = g_new(InsertLinkDialogData, 1);
    data->entry = entry;
    data->window = window;
    g_signal_connect(dialog, "response", G_CALLBACK(insertLinkDialogResponse), data);
    gtk_widget_show(dialog);
}

static void typingAttributesChanged(WebKitEditorState *editorState, GParamSpec *spec, Ir8Window *window)
{
    unsigned typingAttributes = webkit_editor_state_get_typing_attributes(editorState);
    GAction *action = g_action_map_lookup_action(G_ACTION_MAP(window->editActionGroup), "Bold");
    g_simple_action_set_state(G_SIMPLE_ACTION(action), g_variant_new_boolean(typingAttributes & WEBKIT_EDITOR_TYPING_ATTRIBUTE_BOLD));
    action = g_action_map_lookup_action(G_ACTION_MAP(window->editActionGroup), "Italic");
    g_simple_action_set_state(G_SIMPLE_ACTION(action), g_variant_new_boolean(typingAttributes & WEBKIT_EDITOR_TYPING_ATTRIBUTE_ITALIC));
    action = g_action_map_lookup_action(G_ACTION_MAP(window->editActionGroup), "Underline");
    g_simple_action_set_state(G_SIMPLE_ACTION(action), g_variant_new_boolean(typingAttributes & WEBKIT_EDITOR_TYPING_ATTRIBUTE_UNDERLINE));
    action = g_action_map_lookup_action(G_ACTION_MAP(window->editActionGroup), "Strikethrough");
    g_simple_action_set_state(G_SIMPLE_ACTION(action), g_variant_new_boolean(typingAttributes & WEBKIT_EDITOR_TYPING_ATTRIBUTE_STRIKETHROUGH));
}

static void ir8WindowFinalize(GObject *gObject)
{
    Ir8Window *window = IR8_WINDOW(gObject);

    g_signal_handlers_disconnect_matched(window->webContext, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, window);
    g_object_unref(window->webContext);


    if (window->favicon) {
        g_object_unref(window->favicon);
        window->favicon = NULL;
    }

    if (window->resetEntryProgressTimeoutId)
        g_source_remove(window->resetEntryProgressTimeoutId);

    g_clear_object(&window->editActionGroup);
    g_clear_pointer(&window->sessionFile, g_free);

    G_OBJECT_CLASS(ir8_window_parent_class)->finalize(gObject);
}

static void ir8WindowDispose(GObject *gObject)
{
    Ir8Window *window = IR8_WINDOW(gObject);

    if (window->parentWindow) {
        g_object_remove_weak_pointer(G_OBJECT(window->parentWindow), (gpointer *)&window->parentWindow);
        window->parentWindow = NULL;
    }


    G_OBJECT_CLASS(ir8_window_parent_class)->dispose(gObject);
}

typedef enum {
    TOOLBAR_BUTTON_NORMAL,
    TOOLBAR_BUTTON_TOGGLE,
    TOOLBAR_BUTTON_MENU
} ToolbarButtonType;

static GtkWidget *addToolbarButton(GtkWidget *box, ToolbarButtonType type, const char *iconName, const char *actionName)
{
    GtkWidget *button;
    switch (type) {
    case TOOLBAR_BUTTON_NORMAL:
        button = gtk_button_new_from_icon_name(iconName, GTK_ICON_SIZE_MENU);
        break;
    case TOOLBAR_BUTTON_TOGGLE:
        button = gtk_toggle_button_new();
        gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_icon_name(iconName, GTK_ICON_SIZE_MENU));
        break;
    case TOOLBAR_BUTTON_MENU:
        button = gtk_menu_button_new();
        gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_icon_name(iconName, GTK_ICON_SIZE_MENU));

        break;
    }

    gtk_widget_set_focus_on_click(button, FALSE);
    if (actionName)
        gtk_actionable_set_action_name(GTK_ACTIONABLE(button), actionName);

    gtk_container_add(GTK_CONTAINER(box), button);
    gtk_widget_show(button);

    return button;
}

static const GActionEntry editActions[] = {
    { "Bold", NULL, NULL, "false", editingActionCallback, { 0 } },
    { "Italic", NULL, NULL, "false", editingActionCallback, { 0 } },
    { "Underline", NULL, NULL, "false", editingActionCallback, { 0 } },
    { "Strikethrough", NULL, NULL, "false", editingActionCallback, { 0 } },
    { "Cut", editingActionCallback, NULL, NULL, NULL, { 0 } },
    { "Copy", editingActionCallback, NULL, NULL, NULL, { 0 } },
    { "Paste", editingActionCallback, NULL, NULL, NULL, { 0 } },
    { "Undo", editingActionCallback, NULL, NULL, NULL, { 0 } },
    { "Redo", editingActionCallback, NULL, NULL, NULL, { 0 } },
    { "JustifyLeft", NULL, NULL, "false", editingActionCallback, { 0 } },
    { "JustifyCenter", NULL, NULL, "false", editingActionCallback, { 0 } },
    { "JustifyRight", NULL, NULL, "false", editingActionCallback, { 0 } },
    { "Indent", editingActionCallback, NULL, NULL, NULL, { 0 } },
    { "Outdent", editingActionCallback, NULL, NULL, NULL, { 0 } },
    { "InsertUnorderedList", editingActionCallback, NULL, NULL, NULL, { 0 } },
    { "InsertOrderedList", editingActionCallback, NULL, NULL, NULL, { 0 } },
};

static void ir8WindowSetupEditorToolbar(Ir8Window *window)
{
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    window->editToolbar = toolbar;
    gtk_widget_set_margin_top(toolbar, 2);
    gtk_widget_set_margin_bottom(toolbar, 2);
    gtk_widget_set_margin_start(toolbar, 2);
    gtk_widget_set_margin_end(toolbar, 2);

    GSimpleActionGroup *actionGroup = g_simple_action_group_new();
    window->editActionGroup = G_ACTION_GROUP(actionGroup);
    g_action_map_add_action_entries(G_ACTION_MAP(actionGroup), editActions, G_N_ELEMENTS(editActions), window);
    gtk_widget_insert_action_group(toolbar, "edit", G_ACTION_GROUP(actionGroup));

    GtkWidget *groupBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(groupBox), GTK_STYLE_CLASS_LINKED);
    addToolbarButton(groupBox, TOOLBAR_BUTTON_TOGGLE, "format-text-bold-symbolic", "edit.Bold");
    addToolbarButton(groupBox, TOOLBAR_BUTTON_TOGGLE, "format-text-italic-symbolic", "edit.Italic");
    addToolbarButton(groupBox, TOOLBAR_BUTTON_TOGGLE, "format-text-underline-symbolic", "edit.Underline");
    addToolbarButton(groupBox, TOOLBAR_BUTTON_TOGGLE, "format-text-strikethrough-symbolic", "edit.Strikethrough");
    gtk_box_pack_start(GTK_BOX(toolbar), groupBox, FALSE, FALSE, 0);
    gtk_widget_show(groupBox);

    groupBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(groupBox), GTK_STYLE_CLASS_LINKED);
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "edit-cut-symbolic", "edit.Cut");
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "edit-copy-symbolic", "edit.Copy");
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "edit-paste-symbolic", "edit.Paste");
    gtk_box_pack_start(GTK_BOX(toolbar), groupBox, FALSE, FALSE, 0);
    gtk_widget_show(groupBox);

    groupBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(groupBox), GTK_STYLE_CLASS_LINKED);
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "edit-undo-symbolic", "edit.Undo");
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "edit-redo-symbolic", "edit.Redo");
    gtk_box_pack_start(GTK_BOX(toolbar), groupBox, FALSE, FALSE, 0);
    gtk_widget_show(groupBox);

    groupBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(groupBox), GTK_STYLE_CLASS_LINKED);
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "format-justify-left-symbolic", "edit.JustifyLeft");
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "format-justify-center-symbolic", "edit.JustifyCenter");
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "format-justify-right-symbolic", "edit.JustifyRight");
    gtk_box_pack_start(GTK_BOX(toolbar), groupBox, FALSE, FALSE, 0);
    gtk_widget_show(groupBox);

    groupBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(groupBox), GTK_STYLE_CLASS_LINKED);
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "format-indent-more-symbolic", "edit.Indent");
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "format-indent-less-symbolic", "edit.Outdent");
    gtk_box_pack_start(GTK_BOX(toolbar), groupBox, FALSE, FALSE, 0);
    gtk_widget_show(groupBox);

    groupBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(groupBox), GTK_STYLE_CLASS_LINKED);
    /* Not the best icons for these, but we don't have insert list icons in GTK. */
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "media-record-symbolic", "edit.InsertUnorderedList");
    addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "zoom-original-symbolic", "edit.InsertOrderedList");
    gtk_box_pack_start(GTK_BOX(toolbar), groupBox, FALSE, FALSE, 0);
    gtk_widget_show(groupBox);

    groupBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(groupBox), GTK_STYLE_CLASS_LINKED);
    GtkWidget *button = addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "insert-image-symbolic", NULL);
    g_signal_connect(button, "clicked", G_CALLBACK(insertImageCommandCallback), window);
    button = addToolbarButton(groupBox, TOOLBAR_BUTTON_NORMAL, "insert-link-symbolic", NULL);
    g_signal_connect(button, "clicked", G_CALLBACK(insertLinkCommandCallback), window);
    gtk_box_pack_start(GTK_BOX(toolbar), groupBox, FALSE, FALSE, 0);
    gtk_widget_show(groupBox);

    gtk_box_pack_start(GTK_BOX(window->mainBox), toolbar, FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(window->mainBox), toolbar, 1);
    gtk_widget_show(toolbar);
}

static void ir8WindowSwitchTab(GtkNotebook *notebook, Ir8Tab *tab, guint tabIndex, Ir8Window *window)
{
    if (window->activeTab == tab)
        return;

    if (window->activeTab) {
        ir8_tab_set_status_text(window->activeTab, NULL);
        g_clear_object(&window->favicon);

        WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
        g_signal_handlers_disconnect_by_data(webView, window);

        /* We always want close to be connected even for not active tabs */
        g_signal_connect(webView, "close", G_CALLBACK(webViewClose), window);

        WebKitBackForwardList *backForwardlist = webkit_web_view_get_back_forward_list(webView);
        g_signal_handlers_disconnect_by_data(backForwardlist, window);
    }

    window->activeTab = tab;

    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);
    if (webkit_web_view_is_editable(webView)) {
        ir8WindowSetupEditorToolbar(window);
        g_signal_connect(webkit_web_view_get_editor_state(webView), "notify::typing-attributes", G_CALLBACK(typingAttributesChanged), window);
    }
    webViewURIChanged(webView, NULL, window);
    webViewTitleChanged(webView, NULL, window);
    webViewIsLoadingChanged(webView, NULL, window);
    faviconChanged(webView, NULL, window);
    ir8WindowUpdateZoomActions(window);
    if (webkit_web_view_is_loading(webView))
        webViewLoadProgressChanged(webView, NULL, window);

    g_signal_connect(webView, "notify::uri", G_CALLBACK(webViewURIChanged), window);
    g_signal_connect(webView, "notify::estimated-load-progress", G_CALLBACK(webViewLoadProgressChanged), window);
    g_signal_connect(webView, "notify::title", G_CALLBACK(webViewTitleChanged), window);
    g_signal_connect(webView, "notify::is-loading", G_CALLBACK(webViewIsLoadingChanged), window);
    g_signal_connect(webView, "create", G_CALLBACK(webViewCreate), window);
    g_signal_connect(webView, "load-failed", G_CALLBACK(webViewLoadFailed), window);
    g_signal_connect(webView, "decide-policy", G_CALLBACK(webViewDecidePolicy), window);
    g_signal_connect(webView, "mouse-target-changed", G_CALLBACK(webViewMouseTargetChanged), window);
    g_signal_connect(webView, "notify::zoom-level", G_CALLBACK(webViewZoomLevelChanged), window);
    g_signal_connect(webView, "notify::favicon", G_CALLBACK(faviconChanged), window);
    g_signal_connect(webView, "enter-fullscreen", G_CALLBACK(webViewEnterFullScreen), window);
    g_signal_connect(webView, "leave-fullscreen", G_CALLBACK(webViewLeaveFullScreen), window);
    g_signal_connect(webView, "scroll-event", G_CALLBACK(scrollEventCallback), window);
    g_signal_connect_object(webView, "notify::camera-capture-state", G_CALLBACK(webViewMediaCaptureStateChanged), window, 0);
    g_signal_connect_object(webView, "notify::microphone-capture-state", G_CALLBACK(webViewMediaCaptureStateChanged), window, 0);
    g_signal_connect_object(webView, "notify::display-capture-state", G_CALLBACK(webViewMediaCaptureStateChanged), window, 0);

    g_object_set(window->uriEntry, "secondary-icon-activatable", TRUE, NULL);
    g_signal_connect(window->uriEntry, "icon-press", G_CALLBACK(webViewUriEntryIconPressed), window);

    WebKitBackForwardList *backForwardlist = webkit_web_view_get_back_forward_list(webView);
    ir8WindowUpdateNavigationMenu(window, backForwardlist);
    g_signal_connect(backForwardlist, "changed", G_CALLBACK(backForwardlistChanged), window);
}

static void ir8WindowTabAddedOrRemoved(GtkNotebook *notebook, Ir8Tab *tab, guint tabIndex, Ir8Window *window)
{
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(window->notebook), gtk_notebook_get_n_pages(notebook) > 1);
}

static void ir8WindowBuildPopoverMenu(Ir8Window *window, GtkWidget *parent)
{
    GMenu *menu = g_menu_new();
    GMenu *section = g_menu_new();
    GMenuItem *item = g_menu_item_new("Zoom Out", "win.zoom-out");
    g_menu_item_set_attribute(item, "verb-icon", "s", "zoom-out-symbolic");
    g_menu_append_item(section, item);
    g_object_unref(item);

    item = g_menu_item_new("Zoom Original", "win.zoom-default");
    g_menu_item_set_attribute(item, "verb-icon", "s", "zoom-original-symbolic");
    g_menu_append_item(section, item);
    g_object_unref(item);

    item = g_menu_item_new("Zoom In", "win.zoom-in");
    g_menu_item_set_attribute(item, "verb-icon", "s", "zoom-in-symbolic");
    g_menu_append_item(section, item);
    g_object_unref(item);

    GMenuItem *sectionItem = g_menu_item_new_section(NULL, G_MENU_MODEL(section));
    g_menu_item_set_attribute(sectionItem, "display-hint", "s", "horizontal-buttons");
    g_menu_append_item(menu, sectionItem);
    g_object_unref(sectionItem);
    g_object_unref(section);

    section = g_menu_new();
    g_menu_insert(section, -1, "_New Private Window", "win.open-private-window");
    g_menu_insert(section, -1, "_Print…", "win.print");
    g_menu_insert(section, -1, "Prefere_nces…", "win.preferences");
    g_menu_insert(section, -1, "_Quit", "win.quit");
    g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
    g_object_unref(section);

    GtkWidget *popover = gtk_popover_menu_new();
    gtk_popover_bind_model(GTK_POPOVER(popover), G_MENU_MODEL(menu), NULL);
    g_object_unref(menu);

    gtk_menu_button_set_popover(GTK_MENU_BUTTON(parent), popover);
}

static const GActionEntry actions[] = {
    { "reload", reloadPage, NULL, NULL, NULL, { 0 } },
    { "reload-no-cache", reloadPageIgnoringCache, NULL, NULL, NULL, { 0 } },
    { "reload-stop", reloadOrStopCallback, NULL, NULL, NULL, { 0 } },
    { "toggle-inspector", toggleWebInspector, NULL, NULL, NULL, { 0 } },
    { "open-private-window", openPrivateWindow, NULL, NULL, NULL, { 0 } },
    { "focus-location", focusLocationBar, NULL, NULL, NULL, { 0 } },
    { "stop-load", stopPageLoad, NULL, NULL, NULL, { 0 } },
    { "load-homepage", loadHomePage, NULL, NULL, NULL, { 0 } },
    { "go-back", goBackCallback, NULL, NULL, NULL, { 0 } },
    { "go-forward", goForwardCallback, NULL, NULL, NULL, { 0 } },
    { "zoom-in", zoomInCallback, NULL, NULL, NULL, { 0 } },
    { "zoom-out", zoomOutCallback, NULL, NULL, NULL, { 0 } },
    { "zoom-default", defaultZoomCallback, NULL, NULL, NULL, { 0 } },
    { "find", searchCallback, NULL, NULL, NULL, { 0 } },
    { "preferences", settingsCallback, NULL, NULL, NULL, { 0 } },
    { "new-tab", newTabCallback, NULL, NULL, NULL, { 0 } },
    { "toggle-fullscreen", toggleFullScreen, NULL, NULL, NULL, { 0 } },
    { "print", printPage, NULL, NULL, NULL, { 0 } },
    { "close", ir8WindowTryCloseCurrentWebView, NULL, NULL, NULL, { 0 } },
    { "quit", ir8WindowTryClose, NULL, NULL, NULL, { 0 } },
};

static void ir8_window_init(Ir8Window *window)
{
    window->backgroundColor.red = window->backgroundColor.green = window->backgroundColor.blue = 1.0;
    window->backgroundColor.alpha = 1.0;

    gtk_window_set_title(GTK_WINDOW(window), defaultWindowTitle);
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);

    g_action_map_add_action_entries(G_ACTION_MAP(window), actions, G_N_ELEMENTS(actions), window);

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    window->toolbar = toolbar;
    gtk_widget_set_margin_top(toolbar, 2);
    gtk_widget_set_margin_bottom(toolbar, 2);
    gtk_widget_set_margin_start(toolbar, 2);
    gtk_widget_set_margin_end(toolbar, 2);

    GtkWidget *navigationBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(navigationBox), GTK_STYLE_CLASS_LINKED);

    window->backItem = addToolbarButton(navigationBox, TOOLBAR_BUTTON_NORMAL, "go-previous-symbolic", "win.go-back");
    window->forwardItem = addToolbarButton(navigationBox, TOOLBAR_BUTTON_NORMAL, "go-next-symbolic", "win.go-forward");
    g_signal_connect(window->backItem, "button-press-event", G_CALLBACK(navigationButtonPressCallback), window);
    g_signal_connect(window->forwardItem, "button-press-event", G_CALLBACK(navigationButtonPressCallback), window);
    gtk_box_pack_start(GTK_BOX(toolbar), navigationBox, FALSE, FALSE, 0);
    gtk_widget_show(navigationBox);

    addToolbarButton(toolbar, TOOLBAR_BUTTON_NORMAL, "go-home-symbolic", "win.load-homepage");
    addToolbarButton(toolbar, TOOLBAR_BUTTON_NORMAL, "tab-new-symbolic", "win.new-tab");

    window->uriEntry = gtk_entry_new();
    gtk_widget_set_halign(window->uriEntry, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(window->uriEntry, TRUE);
    g_signal_connect_swapped(window->uriEntry, "activate", G_CALLBACK(activateUriEntryCallback), (gpointer)window);
    gtk_entry_set_icon_activatable(GTK_ENTRY(window->uriEntry), GTK_ENTRY_ICON_PRIMARY, FALSE);
    updateUriEntryIcon(window);
    gtk_container_add(GTK_CONTAINER(toolbar), window->uriEntry);
    gtk_widget_show(window->uriEntry);

    window->reloadOrStopButton = addToolbarButton(toolbar, TOOLBAR_BUTTON_NORMAL, "view-refresh-symbolic", "win.reload-stop");
    addToolbarButton(toolbar, TOOLBAR_BUTTON_NORMAL, "edit-find-symbolic", "win.find");
    GtkWidget *button = addToolbarButton(toolbar, TOOLBAR_BUTTON_MENU, "open-menu-symbolic", NULL);
    ir8WindowBuildPopoverMenu(window, button);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    window->mainBox = vbox;
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    gtk_widget_show(toolbar);

    window->notebook = gtk_notebook_new();
    g_signal_connect(window->notebook, "switch-page", G_CALLBACK(ir8WindowSwitchTab), window);
    g_signal_connect(window->notebook, "page-added", G_CALLBACK(ir8WindowTabAddedOrRemoved), window);
    g_signal_connect(window->notebook, "page-removed", G_CALLBACK(ir8WindowTabAddedOrRemoved), window);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(window->notebook), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(window->notebook), FALSE);
    gtk_box_pack_start(GTK_BOX(window->mainBox), window->notebook, TRUE, TRUE, 0);
    gtk_widget_show(window->notebook);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);

}

static gboolean ir8WindowDeleteEvent(GtkWidget *widget, GdkEventAny* event)
{
    ir8WindowTryClose(NULL, NULL, IR8_WINDOW(widget));
    return TRUE;
}

static void ir8_window_class_init(Ir8WindowClass *klass)
{
    GObjectClass *gobjectClass = G_OBJECT_CLASS(klass);
    gobjectClass->dispose = ir8WindowDispose;
    gobjectClass->finalize = ir8WindowFinalize;

    GtkWidgetClass *widgetClass = GTK_WIDGET_CLASS(klass);
    widgetClass->delete_event = ir8WindowDeleteEvent;
}

/* Public API. */
GtkWidget *ir8_window_new(GtkWindow *parent, WebKitWebContext *webContext)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_CONTEXT(webContext), NULL);

    Ir8Window *window = IR8_WINDOW(g_object_new(IR8_TYPE_WINDOW,
        "type", GTK_WINDOW_TOPLEVEL,
        NULL));

    window->webContext = g_object_ref(webContext);
    g_signal_connect(window->webContext, "download-started", G_CALLBACK(downloadStarted), window);

    if (parent) {
        window->parentWindow = parent;
        g_object_add_weak_pointer(G_OBJECT(parent), (gpointer *)&window->parentWindow);
    }

    return GTK_WIDGET(window);
}


WebKitWebContext *ir8_window_get_web_context(Ir8Window *window)
{
    g_return_val_if_fail(IR8_IS_WINDOW(window), NULL);

    return window->webContext;
}

void ir8_window_append_view(Ir8Window *window, WebKitWebView *webView)
{
    g_return_if_fail(IR8_IS_WINDOW(window));
    g_return_if_fail(WEBKIT_IS_WEB_VIEW(webView));

    if (window->activeTab && webkit_web_view_is_editable(ir8_tab_get_web_view(window->activeTab))) {
        g_warning("Only one tab is allowed in editable mode");
        return;
    }

    /* We always want close to be connected even for not active tabs */
    g_signal_connect(webView, "close", G_CALLBACK(webViewClose), window);

    GtkWidget *tab = ir8_tab_new(webView);
    if (gtk_widget_get_app_paintable(GTK_WIDGET(window)))
        ir8_tab_set_background_color(IR8_TAB(tab), &window->backgroundColor);
    gtk_notebook_append_page(GTK_NOTEBOOK(window->notebook), tab, ir8_tab_get_title_widget(IR8_TAB(tab)));
    gtk_container_child_set(GTK_CONTAINER(window->notebook), tab, "tab-expand", TRUE, NULL);
    gtk_widget_show(tab);
}

void ir8_window_load_uri(Ir8Window *window, const char *uri)
{
    g_return_if_fail(IR8_IS_WINDOW(window));
    g_return_if_fail(uri);

    ir8_tab_load_uri(window->activeTab, uri);
}

void ir8_window_load_session(Ir8Window *window, const char *sessionFile)
{
    g_return_if_fail(IR8_IS_WINDOW(window));
    g_return_if_fail(sessionFile);

    WebKitWebView *webView = ir8_tab_get_web_view(window->activeTab);

    window->sessionFile = g_strdup(sessionFile);
    GKeyFile *session = g_key_file_new();
    GError *error = NULL;
    if (!g_key_file_load_from_file(session, sessionFile, G_KEY_FILE_NONE, &error)) {
        if (!g_error_matches(error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
            g_warning("Failed to open session file: %s", error->message);
        g_error_free(error);
        webkit_web_view_load_uri(webView, IR8_DEFAULT_URL);
        g_key_file_free(session);
        return;
    }

    gsize groupCount;
    gchar **groups = g_key_file_get_groups(session, &groupCount);
    if (!groupCount) {
        webkit_web_view_load_uri(webView, IR8_DEFAULT_URL);
        g_strfreev(groups);
        g_key_file_free(session);
        return;
    }

    WebKitWebView *previousWebView = NULL;
    gsize i;
    for (i = 0; i < groupCount; ++i) {
        WebKitWebViewSessionState *state = NULL;
        gchar *base64 = g_key_file_get_string(session, groups[i], "state", NULL);
        if (base64) {
            gsize stateDataLength;
            guchar *stateData = g_base64_decode(base64, &stateDataLength);
            GBytes *bytes = g_bytes_new_take(stateData, stateDataLength);
            state = webkit_web_view_session_state_new(bytes);
            g_bytes_unref(bytes);
            g_free(base64);
        }

        if (!webView) {
            webView = WEBKIT_WEB_VIEW(g_object_new(WEBKIT_TYPE_WEB_VIEW,
                "web-context", webkit_web_view_get_context(previousWebView),
                "settings", webkit_web_view_get_settings(previousWebView),
                "user-content-manager", webkit_web_view_get_user_content_manager(previousWebView),
                "website-policies", webkit_web_view_get_website_policies(previousWebView),
                NULL));
            ir8_window_append_view(window, webView);
        }

        if (state) {
            webkit_web_view_restore_session_state(webView, state);
            webkit_web_view_session_state_unref(state);
        }

        WebKitBackForwardList *bfList = webkit_web_view_get_back_forward_list(webView);
        WebKitBackForwardListItem *item = webkit_back_forward_list_get_current_item(bfList);
        if (item)
            webkit_web_view_go_to_back_forward_list_item(webView, item);
        else
            webkit_web_view_load_uri(webView, "about:blank");

        previousWebView = webView;
        webView = NULL;
    }

    g_strfreev(groups);
    g_key_file_free(session);
}

void ir8_window_set_background_color(Ir8Window *window, GdkRGBA *rgba)
{
    g_return_if_fail(IR8_IS_WINDOW(window));
    g_return_if_fail(rgba);

    g_assert(!window->activeTab);

    if (gdk_rgba_equal(rgba, &window->backgroundColor))
        return;

    window->backgroundColor = *rgba;

    GdkVisual *rgbaVisual = gdk_screen_get_rgba_visual(gtk_window_get_screen(GTK_WINDOW(window)));
    if (!rgbaVisual)
        return;

    gtk_widget_set_visual(GTK_WIDGET(window), rgbaVisual);
    gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);
}

