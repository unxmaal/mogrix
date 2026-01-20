/* getline.c - provide getline() and getdelim() for IRIX
 *
 * IRIX 6.5 libc does not provide these POSIX functions.
 * Only compiled when __sgi is defined.
 */

#if defined(__sgi)

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define GETLINE_INITIAL_SIZE 128

ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
    char *buf;
    size_t size;
    size_t used = 0;
    int c;

    if (lineptr == NULL || n == NULL || stream == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = *lineptr;
    size = *n;

    if (buf == NULL || size == 0) {
        size = GETLINE_INITIAL_SIZE;
        buf = (char *)malloc(size);
        if (buf == NULL) {
            errno = ENOMEM;
            return -1;
        }
    }

    while ((c = fgetc(stream)) != EOF) {
        /* Ensure space for character plus null terminator */
        if (used + 2 > size) {
            size_t newsize = size * 2;
            char *newbuf = (char *)realloc(buf, newsize);
            if (newbuf == NULL) {
                errno = ENOMEM;
                return -1;
            }
            buf = newbuf;
            size = newsize;
        }

        buf[used++] = (char)c;

        if (c == delim)
            break;
    }

    if (used == 0 && c == EOF) {
        /* No characters read and EOF reached */
        *lineptr = buf;
        *n = size;
        return -1;
    }

    buf[used] = '\0';
    *lineptr = buf;
    *n = size;

    return (ssize_t)used;
}

ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    return getdelim(lineptr, n, '\n', stream);
}

#endif /* __sgi */
