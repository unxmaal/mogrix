/* magic_db.cc
 * This file is part of Worker, a file manager for UN*X/X11.
 * Copyright (C) 2008-2020 Ralf Hoffmann.
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

#include "magic_db.hh"
#include <pthread.h>

MagicDB *MagicDB::m_instance = NULL;
static pthread_mutex_t s_instance_mutex = PTHREAD_MUTEX_INITIALIZER;

MagicDB::MagicDB() : m_magic_init( false )
{
    init();
}

MagicDB::~MagicDB()
{
}

MagicDB &MagicDB::getInstance()
{
    pthread_mutex_lock( &s_instance_mutex );
    if ( m_instance == NULL ) {
        m_instance = new MagicDB();
    }
    pthread_mutex_unlock( &s_instance_mutex );
    return *m_instance;
}

void MagicDB::init()
{
#ifdef HAVE_LIBMAGIC
    m_magic_cookie = magic_open( MAGIC_NONE );
    if ( m_magic_cookie == NULL ) return;
    magic_load( m_magic_cookie, NULL );
#endif
    m_magic_init = true;
}

void MagicDB::fini()
{
    m_lock.lock();

    if ( m_magic_init == true ) {
#ifdef HAVE_LIBMAGIC
        magic_close( m_magic_cookie );
        m_magic_cookie = NULL;
#endif
        m_magic_init = false;
    }

    m_lock.unlock();
}

std::string MagicDB::getInfo( const char *filename, int flags )
{
#ifdef HAVE_LIBMAGIC
    m_lock.lock();

    if ( m_magic_init == false ) {
        m_lock.unlock();
        return "";
    }

    std::string res2;
    const char *res;
    int magic_flags = MAGIC_SYMLINK;

    if ( flags & DECOMPRESS ) magic_flags |= MAGIC_COMPRESS;
    if ( flags & USE_MIME_ENCODING ) magic_flags |= MAGIC_MIME_ENCODING;
    if ( flags & USE_MIME_TYPE ) magic_flags |= MAGIC_MIME_TYPE;

    magic_setflags( m_magic_cookie, magic_flags );
    res = magic_file( m_magic_cookie, filename );
    if ( res != NULL ) {
        res2 = res;
    }
    m_lock.unlock();

    return res2;
#else
    return "";
#endif
}

std::string MagicDB::getInfo( const std::string &filename, int flags )
{
    return getInfo( filename.c_str(), flags );
}

std::string MagicDB::getInfo( const void *buffer, size_t len, int flags )
{
#ifdef HAVE_LIBMAGIC
    m_lock.lock();

    if ( m_magic_init == false ) {
        m_lock.unlock();
        return "";
    }

    std::string res2;
    const char *res;
    int magic_flags = MAGIC_NONE;

    if ( flags & DECOMPRESS ) magic_flags |= MAGIC_COMPRESS;
    if ( flags & USE_MIME_ENCODING ) magic_flags |= MAGIC_MIME_ENCODING;
    if ( flags & USE_MIME_TYPE ) magic_flags |= MAGIC_MIME_TYPE;

    magic_setflags( m_magic_cookie, magic_flags );
    res = magic_buffer( m_magic_cookie, buffer, len );
    if ( res != NULL ) {
        res2 = res;
    }
    m_lock.unlock();

    return res2;
#else
    return "";
#endif
}
