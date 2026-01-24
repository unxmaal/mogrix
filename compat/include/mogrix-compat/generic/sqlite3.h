/* Stub sqlite3.h for IRIX cross-compilation
 * Provides minimal types and functions to allow tdnf to compile
 * History functionality will not work at runtime
 */
#ifndef _MOGRIX_SQLITE3_STUB_H
#define _MOGRIX_SQLITE3_STUB_H

#define SQLITE_OK           0
#define SQLITE_ERROR        1
#define SQLITE_INTERNAL     2
#define SQLITE_PERM         3
#define SQLITE_ABORT        4
#define SQLITE_BUSY         5
#define SQLITE_LOCKED       6
#define SQLITE_NOMEM        7
#define SQLITE_READONLY     8
#define SQLITE_INTERRUPT    9
#define SQLITE_IOERR        10
#define SQLITE_ROW          100
#define SQLITE_DONE         101

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

/* Stub function declarations */
int sqlite3_open(const char *filename, sqlite3 **ppDb);
int sqlite3_close(sqlite3 *db);
const char *sqlite3_errmsg(sqlite3 *db);
int sqlite3_exec(sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *arg, char **errmsg);
int sqlite3_prepare_v2(sqlite3 *db, const char *sql, int nbyte, sqlite3_stmt **ppStmt, const char **pzTail);
int sqlite3_step(sqlite3_stmt *pStmt);
int sqlite3_finalize(sqlite3_stmt *pStmt);
int sqlite3_bind_int(sqlite3_stmt *pStmt, int i, int iValue);
int sqlite3_bind_int64(sqlite3_stmt *pStmt, int i, long long iValue);
int sqlite3_bind_text(sqlite3_stmt *pStmt, int i, const char *zData, int nData, void(*xDel)(void*));
int sqlite3_column_int(sqlite3_stmt *pStmt, int i);
long long sqlite3_column_int64(sqlite3_stmt *pStmt, int i);
const unsigned char *sqlite3_column_text(sqlite3_stmt *pStmt, int i);
long long sqlite3_last_insert_rowid(sqlite3 *db);
void sqlite3_free(void *p);
int sqlite3_reset(sqlite3_stmt *pStmt);
int sqlite3_changes(sqlite3 *db);

#endif /* _MOGRIX_SQLITE3_STUB_H */
