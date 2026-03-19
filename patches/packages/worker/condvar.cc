/* condvar.cc
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

/* IRIX/MIPS patch — see mutex.h for full explanation.
 *
 * CondVar::wait() changes vs. original:
 *
 *   Original:
 *     1. acquire m_owner_data_mutex (std::mutex)
 *     2. clear _owner / save+clear _owncount
 *     3. release m_owner_data_mutex          <-- NO fence to pthread layer
 *     4. pthread_cond_wait(&_condvar, &_mutex)
 *     5. acquire m_owner_data_mutex
 *     6. restore _owner / _owncount
 *     7. release m_owner_data_mutex
 *
 *   Fixed:
 *     1. (already hold _mutex — it was acquired by MutEx::lock())
 *     2. clear _owner / save+clear _owncount  -- inside _mutex critical section
 *     3. pthread_cond_wait(&_condvar, &_mutex)
 *        -- atomically releases _mutex, blocks, reacquires _mutex on wakeup
 *        -- POSIX guarantees: the release and reacquire are each full fences
 *     4. restore _owner / _owncount           -- inside _mutex critical section
 *
 * No separate m_owner_data_mutex needed.  pthread_cond_wait itself forms
 * the fence boundary: stores before the call are visible after _mutex is
 * released; stores after the call are inside the newly-reacquired _mutex.
 */

#include "condvar.h"

CondVar::CondVar() : MutEx()
{
  pthread_cond_init( &_condvar, NULL );
}

CondVar::~CondVar()
{
  pthread_cond_destroy( &_condvar );
}

void CondVar::wait()
{
  int oldcount;

  /* We must already hold _mutex (enforced by caller using MutEx::lock()).
   * Clear owner metadata while still inside the _mutex critical section,
   * so pthread_cond_wait sees a consistent state when it atomically drops
   * _mutex.  No additional fence is needed: _mutex is already held. */
  if ( _owner != pthread_self() ) {
      /* Called without owning the mutex — same early-return as original. */
      return;
  }

  _owner = 0;
  oldcount = _owncount;
  _owncount = 0;

#ifdef THREAD_DEBUG
  printf( "(%d,%p)::wait %d\n", pthread_self(), this, oldcount );
#endif

  /* pthread_cond_wait atomically:
   *   - releases _mutex  (release fence — above stores are visible)
   *   - blocks on _condvar
   *   - reacquires _mutex on wakeup (acquire fence — below stores are safe)
   * This is the ONLY safe way to use a condvar; no secondary lock is needed. */
  pthread_cond_wait( &_condvar, &_mutex );

#ifdef THREAD_DEBUG
  printf( "(%d)::wait end\n", pthread_self() );
#endif

  /* Back inside _mutex critical section after wakeup. */
  _owner = pthread_self();
  _owncount = oldcount;
}

void CondVar::signal()
{
  pthread_cond_signal( &_condvar );

#ifdef THREAD_DEBUG
  printf( "(%d,%p)signal::count %d\n", pthread_self(), this, _owncount );
#endif
}

void CondVar::bcast()
{
  pthread_cond_broadcast( &_condvar );
}
