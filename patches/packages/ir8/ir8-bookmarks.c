/*
 * ir8 — Web browser for IRIX
 * Bookmarks system — flat file storage at ~/.config/ir8/bookmarks.txt
 * Format: one bookmark per line, URL<TAB>TITLE
 */

#include "ir8-bookmarks.h"
#include <stdio.h>
#include <string.h>

static GList *bookmarksList = NULL;
static gboolean bookmarksLoaded = FALSE;

static char *getBookmarksPath(void)
{
    return g_build_filename(g_get_user_config_dir(), "ir8", "bookmarks.txt", NULL);
}

void ir8_bookmark_free(Ir8Bookmark *bookmark)
{
    if (!bookmark)
        return;
    g_free(bookmark->url);
    g_free(bookmark->title);
    g_free(bookmark);
}

static Ir8Bookmark *ir8BookmarkNew(const char *url, const char *title)
{
    Ir8Bookmark *b = g_new0(Ir8Bookmark, 1);
    b->url = g_strdup(url);
    b->title = g_strdup(title);
    return b;
}

GList *ir8_bookmarks_load(void)
{
    if (bookmarksLoaded)
        return bookmarksList;

    bookmarksLoaded = TRUE;
    char *path = getBookmarksPath();
    char *contents = NULL;
    gsize length = 0;

    if (!g_file_get_contents(path, &contents, &length, NULL)) {
        g_free(path);
        return NULL;
    }
    g_free(path);

    char **lines = g_strsplit(contents, "\n", -1);
    g_free(contents);

    for (int i = 0; lines[i]; i++) {
        if (!lines[i][0])
            continue;

        char *tab = strchr(lines[i], '\t');
        if (tab) {
            *tab = '\0';
            bookmarksList = g_list_append(bookmarksList,
                ir8BookmarkNew(lines[i], tab + 1));
        } else {
            bookmarksList = g_list_append(bookmarksList,
                ir8BookmarkNew(lines[i], lines[i]));
        }
    }
    g_strfreev(lines);
    return bookmarksList;
}

void ir8_bookmarks_save(GList *bookmarks)
{
    char *path = getBookmarksPath();
    char *dir = g_path_get_dirname(path);
    g_mkdir_with_parents(dir, 0755);
    g_free(dir);

    GString *buf = g_string_new(NULL);
    for (GList *l = bookmarks; l; l = l->next) {
        Ir8Bookmark *b = l->data;
        g_string_append_printf(buf, "%s\t%s\n", b->url, b->title);
    }

    g_file_set_contents(path, buf->str, buf->len, NULL);
    g_string_free(buf, TRUE);
    g_free(path);
}

void ir8_bookmarks_add(const char *url, const char *title)
{
    ir8_bookmarks_load();
    if (ir8_bookmarks_contains(url))
        return;

    bookmarksList = g_list_append(bookmarksList,
        ir8BookmarkNew(url, title));
    ir8_bookmarks_save(bookmarksList);
}

void ir8_bookmarks_remove(const char *url)
{
    ir8_bookmarks_load();
    for (GList *l = bookmarksList; l; l = l->next) {
        Ir8Bookmark *b = l->data;
        if (!g_strcmp0(b->url, url)) {
            bookmarksList = g_list_remove_link(bookmarksList, l);
            ir8_bookmark_free(b);
            g_list_free_1(l);
            ir8_bookmarks_save(bookmarksList);
            return;
        }
    }
}

gboolean ir8_bookmarks_contains(const char *url)
{
    ir8_bookmarks_load();
    for (GList *l = bookmarksList; l; l = l->next) {
        Ir8Bookmark *b = l->data;
        if (!g_strcmp0(b->url, url))
            return TRUE;
    }
    return FALSE;
}

GList *ir8_bookmarks_get_all(void)
{
    ir8_bookmarks_load();
    return bookmarksList;
}
