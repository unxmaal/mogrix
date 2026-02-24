/*
 * ir8 — Web browser for IRIX
 * Bookmarks system
 */

#ifndef Ir8Bookmarks_h
#define Ir8Bookmarks_h

#include <glib.h>
#include <stdbool.h>

G_BEGIN_DECLS

typedef struct {
    char *url;
    char *title;
} Ir8Bookmark;

/* Load bookmarks from ~/.config/ir8/bookmarks.txt */
GList *ir8_bookmarks_load(void);

/* Save bookmarks list to file */
void ir8_bookmarks_save(GList *bookmarks);

/* Add a bookmark (also saves to file) */
void ir8_bookmarks_add(const char *url, const char *title);

/* Remove a bookmark by URL (also saves to file) */
void ir8_bookmarks_remove(const char *url);

/* Check if a URL is bookmarked */
gboolean ir8_bookmarks_contains(const char *url);

/* Get the full bookmarks list (do not free; owned by bookmarks system) */
GList *ir8_bookmarks_get_all(void);

/* Free a single bookmark */
void ir8_bookmark_free(Ir8Bookmark *bookmark);

G_END_DECLS

#endif
