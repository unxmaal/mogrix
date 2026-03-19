/* nwc_virtualdir.cc
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2006-2008,2011 Ralf Hoffmann.
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

#include "nwc_virtualdir.hh"
#include "nwc_file.hh"
#include <iostream>
#include <typeinfo>

namespace NWC
{

  VirtualDir::VirtualDir( const std::string &fullname,
                          bool follow_symlinks,
                          bool search_vfs,
                          bool search_same_fs ) : Dir( fullname, follow_symlinks ),
                                                  _search_vfs( search_vfs ),
                                                  _search_same_fs( search_same_fs ),
                                                  m_virtual_depth_count( 0 )
  {
  }

  VirtualDir::VirtualDir( const FSEntry &entry,
                          bool follow_symlinks,
                          bool search_vfs,
                          bool search_same_fs ) : Dir( entry, follow_symlinks ),
                                                  _search_vfs( search_vfs ),
                                                  _search_same_fs( search_same_fs ),
                                                  m_virtual_depth_count( 0 )
  {
      /* IRIX fix: use pointer cast to avoid std::bad_cast exception.
         The DWARF unwinder crashes on IRIX libpthread frames. */
      const VirtualDir *vdir = dynamic_cast<const VirtualDir*>( &entry );
      if ( vdir != NULL ) {
          setVirtualDepthCount( vdir->getVirtualDepthCount() );
      }
  }

  VirtualDir::~VirtualDir()
  {
  }

  void VirtualDir::handleDirEntry( const std::string &filename )
  {
      bool addit = true;

    if ( _search_same_fs == true ) {
        FSEntry te( filename );

        if ( te.isLink() == true && te.isDir( getFollowSymlinks() ) ) {
            if ( stat_dev() != te.stat_dest_dev() ) {
                addit = false;
            }
        } else {
            if ( stat_dev() != te.stat_dev() ) {
                addit = false;
            }
        }
    }

    if ( addit == true ) {
        addEntry( filename );
        
        if ( _search_vfs == true ) {
            VirtualDir vd( filename + "#", getFollowSymlinks(), _search_vfs, _search_same_fs );
            if ( vd.entryExists() == true && vd.isDir() ) {
                //TODO use worker filetypes to test for virtual file and use specific handler
                vd.setVirtualDepthCount( getVirtualDepthCount() + 1 );
                addEntry( vd );
            }
        }
    }
  }
    
  File *VirtualDir::createFile( const FSEntry &ent )
  {
    return new File( ent );
  }

  Dir *VirtualDir::createDir( const FSEntry &ent )
  {
      VirtualDir *v = new VirtualDir( ent, getFollowSymlinks(), _search_vfs, _search_same_fs );
      v->setVirtualDepthCount( m_virtual_depth_count );
      return v;
  }

  FSEntry *VirtualDir::clone() const
  {
    return new VirtualDir( *this );
  }

  void VirtualDir::setSearchVFS( bool nv )
  {
    _search_vfs = nv;
  }

  bool VirtualDir::getSearchVFS() const
  {
    return _search_vfs;
  }

  void VirtualDir::setSearchSameFS( bool nv )
  {
    _search_same_fs = nv;
  }

  bool VirtualDir::getSearchSameFS() const
  {
    return _search_same_fs;
  }

  void VirtualDir::setVirtualDepthCount( int depth )
  {
      if ( depth >= 0 ) {
          m_virtual_depth_count = depth;
      }
  }

  int VirtualDir::getVirtualDepthCount() const
  {
      return m_virtual_depth_count;
  }

}
