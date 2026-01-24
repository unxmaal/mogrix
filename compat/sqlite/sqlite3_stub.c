/* Stub sqlite3 implementation for IRIX cross-compilation
 * All functions return error codes - history functionality will not work
 */
#include <stdlib.h>

#define SQLITE_OK           0
#define SQLITE_ERROR        1
#define SQLITE_NOMEM        7
#define SQLITE_ROW          100
#define SQLITE_DONE         101

typedef struct sqlite3 {
    int dummy;
} sqlite3;

typedef struct sqlite3_stmt {
    int dummy;
} sqlite3_stmt;

static const char *stub_errmsg = "sqlite3 not available (stub implementation)";

int sqlite3_open(const char *filename, sqlite3 **ppDb)
{
    (void)filename;
    *ppDb = NULL;
    return SQLITE_ERROR;
}

int sqlite3_close(sqlite3 *db)
{
    (void)db;
    return SQLITE_OK;
}

const char *sqlite3_errmsg(sqlite3 *db)
{
    (void)db;
    return stub_errmsg;
}

int sqlite3_exec(sqlite3 *db, const char *sql,
                 int (*callback)(void*,int,char**,char**),
                 void *arg, char **errmsg)
{
    (void)db;
    (void)sql;
    (void)callback;
    (void)arg;
    if (errmsg) *errmsg = NULL;
    return SQLITE_ERROR;
}

int sqlite3_prepare_v2(sqlite3 *db, const char *sql, int nbyte,
                       sqlite3_stmt **ppStmt, const char **pzTail)
{
    (void)db;
    (void)sql;
    (void)nbyte;
    *ppStmt = NULL;
    if (pzTail) *pzTail = NULL;
    return SQLITE_ERROR;
}

int sqlite3_step(sqlite3_stmt *pStmt)
{
    (void)pStmt;
    return SQLITE_DONE;
}

int sqlite3_finalize(sqlite3_stmt *pStmt)
{
    (void)pStmt;
    return SQLITE_OK;
}

int sqlite3_bind_int(sqlite3_stmt *pStmt, int i, int iValue)
{
    (void)pStmt;
    (void)i;
    (void)iValue;
    return SQLITE_ERROR;
}

int sqlite3_bind_int64(sqlite3_stmt *pStmt, int i, long long iValue)
{
    (void)pStmt;
    (void)i;
    (void)iValue;
    return SQLITE_ERROR;
}

int sqlite3_bind_text(sqlite3_stmt *pStmt, int i, const char *zData,
                      int nData, void(*xDel)(void*))
{
    (void)pStmt;
    (void)i;
    (void)zData;
    (void)nData;
    (void)xDel;
    return SQLITE_ERROR;
}

int sqlite3_column_int(sqlite3_stmt *pStmt, int i)
{
    (void)pStmt;
    (void)i;
    return 0;
}

long long sqlite3_column_int64(sqlite3_stmt *pStmt, int i)
{
    (void)pStmt;
    (void)i;
    return 0;
}

const unsigned char *sqlite3_column_text(sqlite3_stmt *pStmt, int i)
{
    (void)pStmt;
    (void)i;
    return (const unsigned char *)"";
}

long long sqlite3_last_insert_rowid(sqlite3 *db)
{
    (void)db;
    return 0;
}

void sqlite3_free(void *p)
{
    (void)p;
    /* Do nothing - we don't allocate memory */
}

int sqlite3_reset(sqlite3_stmt *pStmt)
{
    (void)pStmt;
    return SQLITE_OK;
}

int sqlite3_changes(sqlite3 *db)
{
    (void)db;
    return 0;
}
