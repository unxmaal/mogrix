/*
 * Mogrix IRIX compat - spawn.h
 * POSIX spawn API for IRIX (which doesn't have it natively)
 *
 * Provides posix_spawn() and posix_spawnp() via fork+exec.
 * File actions and spawn attributes are not fully supported -
 * only the basic spawn functionality is provided.
 */

#ifndef _MOGRIX_SPAWN_H
#define _MOGRIX_SPAWN_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Spawn file actions - stores up to 16 actions to perform in child
 */
#define _MOGRIX_SPAWN_MAX_ACTIONS 16

typedef struct {
    int count;
    struct {
        int type;       /* 0=close, 1=dup2, 2=open */
        int fd;         /* fd to close or dup2 target */
        int newfd;      /* for dup2: source fd */
    } actions[_MOGRIX_SPAWN_MAX_ACTIONS];
} posix_spawn_file_actions_t;

/* Spawn attributes - placeholder (not fully implemented) */
typedef struct {
    int __dummy;
} posix_spawnattr_t;

/*
 * posix_spawn - spawn a new process
 *
 * Creates a new process using fork+execv. The child process executes
 * the program at 'path' with arguments 'argv' and environment 'envp'.
 *
 * Note: file_actions and attrp are ignored in this implementation.
 */
int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[], char *const envp[]);

/*
 * posix_spawnp - spawn a new process, searching PATH
 *
 * Like posix_spawn(), but searches for 'file' in PATH if it doesn't
 * contain a slash.
 */
int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp,
                 char *const argv[], char *const envp[]);

/*
 * File actions management functions
 */
int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions);
int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions);
int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions, int fd);
int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions, int fd, int newfd);

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_SPAWN_H */
