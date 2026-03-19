/* kvpstore.cc
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2014 Ralf Hoffmann.
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

#include "kvpstore.hh"

/* IRIX fix: eliminate throw/catch — DWARF unwinder crashes on IRIX 6.5
   when exceptions traverse libpthread frames with unsaved CFA registers. */
int KVPStore::getIntValue( const std::string &key )
{
    if ( m_int_elements.count( key ) > 0 ) return m_int_elements[key];
    return 0;
}

int KVPStore::getIntValue( const std::string &key, int default_value )
{
    if ( m_int_elements.count( key ) > 0 ) return m_int_elements[key];
    return default_value;
}

void KVPStore::setIntValue( const std::string &key,
                            int value )
{
    m_int_elements[key] = value;
}

const std::string &KVPStore::getStringValue( const std::string &key )
{
    static const std::string empty_str;
    if ( m_string_elements.count( key ) < 1 ) return empty_str;
    return m_string_elements[key];
}

const std::string &KVPStore::getStringValue( const std::string &key, const std::string &default_value )
{
    if ( m_string_elements.count( key ) < 1 ) return default_value;
    return m_string_elements[key];
}

void KVPStore::setStringValue( const std::string &key,
                               const std::string &value )
{
    m_string_elements[key] = value;
}

void KVPStore::eraseStringValue( const std::string &key )
{
    m_string_elements.erase( key );
}

void KVPStore::eraseIntValue( const std::string &key )
{
    m_int_elements.erase( key );
}
