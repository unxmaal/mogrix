/* mutex.cc
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

/* IRIX/MIPS patch — see mutex.h for full explanation. */

#include "mutex.h"

MutEx::MutEx()
{
  pthread_mutex_init( &_mutex, NULL );
  _owner = 0;
  _owncount = 0;
}

MutEx::~MutEx()
{
  pthread_mutex_destroy( &_mutex );
}

void MutEx::lock()
{
    /* Reentrant-ownership check.
     *
     * We read _owner without holding _mutex.  This is safe on any
     * architecture — including weakly-ordered MIPS — because:
     *
     *   a) _owner is written to pthread_self() only by the thread that
     *      just acquired _mutex, i.e. exactly one thread, and that
     *      thread is the only one that can make "_owner == pthread_self()"
     *      true for any given caller.
     *
     *   b) _owner is zeroed (set to 0) only by the owning thread before
     *      it releases _mutex.  pthread_mutex_unlock() provides the
     *      release fence that makes the zeroing visible before _mutex
     *      becomes acquirable by another thread.
     *
     *   c) pthread_self() is a pure thread-local value — it never changes
     *      under the caller's feet.
     *
     * The __sync_synchronize() ensures the load of _owner is not
     * speculated or reordered before any preceding operation by the
     * CPU's out-of-order / weak-memory pipeline.
     */
    __sync_synchronize();
    if ( _owner == pthread_self() ) {
        _owncount++;
        return;
    }

    pthread_mutex_lock( &_mutex );
    /* pthread_mutex_lock provides acquire fence — _owner/_owncount are
     * now safe to write without an additional barrier. */
    _owner = pthread_self();
    _owncount = 1;
}

void MutEx::unlock()
{
    /* _mutex is held by our caller (invariant of correct use). */
    if ( _owner == pthread_self() ) {
        _owncount--;

        if ( _owncount < 1 ) {
            /* Zero _owner before releasing _mutex so that any thread
             * that subsequently acquires _mutex sees _owner == 0. */
            _owner = 0;
            /* pthread_mutex_unlock provides the release fence. */
            pthread_mutex_unlock( &_mutex );
        }
    }
}

bool MutEx::trylock()
{
    /* Same safe read as in lock(). */
    __sync_synchronize();
    if ( _owner == pthread_self() ) {
        _owncount++;
        return true;
    }

    if ( pthread_mutex_trylock( &_mutex ) != 0 ) return false;

    _owner = pthread_self();
    _owncount = 1;
    return true;
}
