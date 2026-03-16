/* largefile-config.h — IRIX n32 large file support
 *
 * IRIX n32 ABI uses 64-bit off_t natively, so no _FILE_OFFSET_BITS hack
 * is needed. This header satisfies the #include in filesystem sources.
 */
#ifndef _GLIBCXX_LARGEFILE_CONFIG_H
#define _GLIBCXX_LARGEFILE_CONFIG_H

/* IRIX n32: off_t is already 64 bits, nothing to configure */

#endif
