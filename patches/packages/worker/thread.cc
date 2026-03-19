/* thread.cc
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2006,2016 Ralf Hoffmann.
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

#include "thread.hh"
#include <exception>
#include <cstdio>

Thread::Thread() : _id( 0 ), m_running( false ), m_cancel( false )
{
}

Thread::~Thread()
{
  join();
}

int Thread::start()
{
  int erg;
  pthread_attr_t attr;
  pthread_attr_init( &attr );
  pthread_attr_setstacksize( &attr, 8 * 1024 * 1024 );

  // lock var so actual run() only starts after _id is stored
  _lock.lock();
  m_running = true;
  erg = pthread_create( &_id, &attr, thread_entry, this );
  _lock.unlock();
  pthread_attr_destroy( &attr );
  return erg;
}

void *Thread::thread_entry( void *me )
{
    ((Thread*)me)->thread_run();
    return NULL;
}

int Thread::join()
{
  int erg = 0;

  if ( _id != 0 ) {
    erg = pthread_join( _id, NULL );
    _id = 0;
  }
  return erg;
}

bool Thread::amIThisThread() const
{
  if ( pthread_equal( _id, pthread_self() ) )
    return true;
  return false;
}

void Thread::thread_run()
{
    _lock.lock();
    _lock.unlock();
    // after having got the lock _id is correct so amIThisThread works
    //
    // IRIX workaround: wrap run() in try/catch to prevent C++ exceptions from
    // propagating into the pthread exit path. On IRIX 6.5, the GCC unwinder
    // (libgcc_s _Unwind_Backtrace) crashes with SIGSEGV when walking eh_frame
    // data in cross-compiled N32 binaries. Catching at the thread boundary
    // prevents the broken unwinder from being invoked for uncaught exceptions.
    try {
        run();
    } catch (const std::exception &e) {
        fprintf(stderr, "Worker: thread exception caught at boundary: %s\n", e.what());
    } catch (...) {
        fprintf(stderr, "Worker: unknown thread exception caught at boundary\n");
    }
    _lock.lock();
    m_running = false;
    _lock.unlock();
}

bool Thread::running()
{
    return m_running;
}

void Thread::signalCancel()
{
    m_cancel = true;
}

bool Thread::isCanceled()
{
    return m_cancel;
}
