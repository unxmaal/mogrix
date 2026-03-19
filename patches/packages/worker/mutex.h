/* mutex.h
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2002-2014 Ralf Hoffmann.
 * You can contact me at: ralf@boomerangsworld.de
 *   or http://www.boomerangsworld.de/worker
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* IRIX/MIPS patch: eliminated std::mutex m_owner_data_mutex.
 *
 * Original design used two lock layers:
 *   _mutex               — the real pthread serialisation lock
 *   m_owner_data_mutex   — a std::mutex guarding _owner/_owncount metadata
 *
 * On IRIX 6.5 (weakly-ordered MIPS R-series) there is no guaranteed store
 * fence between releasing m_owner_data_mutex and the pthread_cond_wait call
 * in CondVar::wait().  A libpthread timer signal (IRIX signal 48) arriving
 * in that window can observe stale _owner / _owncount values, causing the
 * reentrant-lock fast path in lock() to fire incorrectly and ultimately
 * produce a SIGSEGV in the main thread.
 *
 * Fix: _owner and _owncount are now protected exclusively by _mutex itself.
 * The reentrant check in lock() reads _owner without holding _mutex; this is
 * safe because only the thread that currently owns _mutex ever writes _owner
 * to a non-zero value, and pthread_self() is always the calling thread — so
 * the comparison "_owner == pthread_self()" can only be true for the one
 * thread that already holds _mutex.  A GCC __sync_synchronize() fence before
 * the read guarantees the load is not speculated across the preceding store on
 * weak-order hardware.  All other reads/writes of _owner/_owncount happen
 * while _mutex is held, which provides the full POSIX acquire/release fence.
 */

#ifndef MUTEX_H
#define MUTEX_H

#include "aguixdefs.h"
#include <pthread.h>

class MutEx
{
 public:
    MutEx();
    virtual ~MutEx();
    MutEx( const MutEx &other );
    MutEx &operator=( const MutEx &other );
    virtual void lock();
    virtual void unlock();
    virtual bool trylock();
 protected:
    /* the actual pthread mutex — also guards _owner and _owncount */
    pthread_mutex_t _mutex;

    pthread_t _owner;
    int _owncount;
};

#endif
