/* searchthread.cc
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2006-2008,2011,2013 Ralf Hoffmann.
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

#include "searchthread.hh"
#include "aguix/lowlevelfunc.h"
#include "nwc_file.hh"
#include "pdatei.h"
#include "worker_locale.h"
#include "nwc_virtualdir.hh"
#include <typeinfo>
#include <set>

#include <iostream>

SearchThread::SearchThread( std::unique_ptr<NWC::Dir> searchdir ) : _current_mode( SEARCHING ),
                                                                    _mode_order( SEARCH )
{
  _searchdir = searchdir.release();
}

SearchThread::~SearchThread()
{
  delete _searchdir;
}

int SearchThread::run()
{
  debugmsg( "Search thread started" );

  doSearch();
  
  debugmsg( "Search thread finished" );
  _current_mode = FINISHED;

  return 0;
}

void SearchThread::setOrder( search_order_t neworder )
{
  if ( amIThisThread() == true ) return;

  _order_cv.lock();
  if ( _mode_order != neworder ) {
    _mode_order = neworder;
    _order_cv.signal();
  }
  _order_cv.unlock();
}

int SearchThread::doSearch()
{
  _searchdir->walk( *this, NWC::Dir::RECURSIVE_DEMAND );
  return 0;
}

void SearchThread::lockResults()
{
  _result_lock.lock();
}

void SearchThread::unlockResults()
{
  _result_lock.unlock();
}

int SearchThread::getNrOfResults()
{
  _result_lock.lock();
  int erg = _results.size();
  _result_lock.unlock();
  return erg;
}

bool SearchThread::resultsAvailable()
{
  _result_lock.lock();
  bool erg = ! _results.empty();
  _result_lock.unlock();
  return erg;
}

SearchThread::SearchResult SearchThread::getResultLocked()
{
  SearchResult res = _results.front();

  _results.pop_front();

  return res;
}

SearchThread::search_mode_t SearchThread::getCurrentMode()
{
  return _current_mode;
}

int SearchThread::visit( NWC::File &file, NWC::Dir::WalkControlObj &cobj )
{
    bool candidate = true;

    if ( m_sets.getCheckYounger() == true && file.stat_lastmod() < m_sets.getYoungerThanDate() ) {
        candidate = false;
    } else if ( m_sets.getCheckOlder() == true && file.stat_lastmod() > m_sets.getOlderThanDate() ) {
        candidate = false;
    } else if ( m_sets.getCheckSizeGEQ() == true ||
                m_sets.getCheckSizeLEQ() == true ) {
        if ( file.isReg( m_sets.getFollowSymlinks() ) == true ) {
            loff_t filesize = 0;
            if ( file.isLink() == true && m_sets.getFollowSymlinks() == true ) {
                filesize = file.stat_dest_size();
            } else {
                filesize = file.stat_size();
            }

            if ( m_sets.getCheckSizeGEQ() == true && filesize < m_sets.getSizeGEQAsInt() ) {
                candidate = false;
            } else if ( m_sets.getCheckSizeLEQ() == true && filesize > m_sets.getSizeLEQAsInt() ) {
                candidate = false;
            }
        } else {
            candidate = false;
        }
    }

    if ( candidate == true ) {
        if ( checkName( m_sets.getMatchNameFullPath() == true ? file.getFullname() : file.getBasename() ) == true ) {
            checkContent( file );
        }
    }
    
    if ( checkInterrupt() == true ) {
        cobj.setMode( NWC::Dir::WalkControlObj::ABORT );
    }
    return 0;
}

int SearchThread::visitEnterDir( NWC::Dir &dir, NWC::Dir::WalkControlObj &cobj )
{
    bool candidate = true;
    
    _current_action_lock.lock();
    _current_action = catalog.getLocale( 747 );
    _current_action += dir.getFullname();
    _current_action_lock.unlock();

    if ( m_sets.getMatchAlsoDirectories() ) {
        if ( m_sets.getCheckYounger() == true && dir.stat_lastmod() < m_sets.getYoungerThanDate() ) {
            candidate = false;
        } else if ( m_sets.getCheckOlder() == true && dir.stat_lastmod() > m_sets.getOlderThanDate() ) {
            candidate = false;
        } else if ( m_sets.getCheckSizeGEQ() == true ||
                    m_sets.getCheckSizeLEQ() == true ) {
            candidate = false;
        }

        if ( candidate == true ) {
            if ( checkName( m_sets.getMatchNameFullPath() == true ? dir.getFullname() : dir.getBasename() ) == true ) {
                if ( m_sets.getMatchContent().length() < 1 ) {
                    _result_lock.lock();
                    _results.push_back( SearchResult( dir.getFullname(), -1, true ) );
                    _result_lock.unlock();
                }
            }
        }
    }
    
    return 0;
}

int SearchThread::visitLeaveDir( NWC::Dir &dir, NWC::Dir::WalkControlObj &cobj )
{
  return 0;
}

int SearchThread::prepareDirWalk( const NWC::Dir &dir, NWC::Dir::WalkControlObj &cobj,
                                  std::list< RefCount< NWC::FSEntry > > &return_add_entries,
                                  std::list< RefCount< NWC::FSEntry > > &return_skip_entries )
{
    if ( m_sets.getSearchVFS() == true && m_sets.getMaxVFSDepth() > 0 ) {
        /* IRIX fix: use pointer cast to avoid std::bad_cast exception.
           The DWARF unwinder crashes on IRIX libpthread frames. */
        const NWC::VirtualDir *vdir = dynamic_cast<const NWC::VirtualDir*>( &dir );

        if ( vdir != NULL && vdir->getVirtualDepthCount() < m_sets.getMaxVFSDepth() ) {

                std::list< std::string > entries = dir.getFullnamesOfSubentries();
                std::set< std::string > entries_set;

                for ( std::list< std::string >::const_iterator it1 = entries.begin();
                      it1 != entries.end();
                      it1++ ) {
                    entries_set.insert( *it1 );
                }

                for ( std::list< std::string >::const_iterator it1 = entries.begin();
                      it1 != entries.end();
                      it1++ ) {

                    std::string vname = *it1 + '#';

                    if ( entries_set.find( vname ) == entries_set.end() ) {
                        NWC::VirtualDir *vd =  new NWC::VirtualDir( vname,
                                                                    vdir->getFollowSymlinks(),
                                                                    false,
                                                                    vdir->getSearchSameFS() );
                        if ( vd->entryExists() == true && vd->isDir() ) {
                            vd->setVirtualDepthCount( vdir->getVirtualDepthCount() + 1 );
                            return_add_entries.push_back( RefCount< NWC::FSEntry >( vd ) );
                        }
                    }
                }
        }
    }

    return 0;
}

bool SearchThread::checkName( const std::string &str )
{
  if ( m_sets.getMatchNameRE() == true ) {
      return re.match( m_sets.getMatchName().c_str(), str.c_str(), ( m_sets.getMatchNameCase() == false ) ? true : false );
  } else {
    if ( m_sets.getMatchNameCase() == false ) {
#ifdef HAVE_GNU_FNMATCH
      if ( fnmatch( m_sets.getMatchName().c_str(), str.c_str(), FNM_CASEFOLD ) == 0 ) return true;
#else
      bool matched = false;
      char *lstr, *pstr;
      int i;
      
      //TODO this is not UTF8 safe
      lstr = dupstring( str.c_str() );
      for ( i = 0; lstr[i] != '\0'; i++ )
        lstr[i] = tolower( lstr[i] );
      
      pstr = dupstring( m_sets.getMatchName().c_str() );
      for ( i = 0; pstr[i] != '\0'; i++ )
        pstr[i] = tolower( pstr[i] );
      
      if ( fnmatch( pstr, lstr, 0 ) == 0 ) matched = true;
      _freesafe( lstr );
      _freesafe( pstr );
      return matched;
#endif
    } else {
        if ( fnmatch( m_sets.getMatchName().c_str(), str.c_str(), 0 ) == 0 ) return true;
    }
  }
  return false;                                                                 
}

int SearchThread::matchContentLine( const NWC::File &file, const char *line, int current_line )
{
  int found = 0;
  if ( re.match( m_sets.getMatchContent().c_str(), line, ( m_sets.getMatchContentCase() == true ) ? false : true ) == true ) {
    _result_lock.lock();
    _results.push_back( SearchResult( file.getFullname(), current_line ) );
    _result_lock.unlock();
    found = 1;
  }
  return found;
}

struct input_buffer {
  char buf[4096];
  int left, pos;
  PDatei *pd;
  bool eof_reached;
};

struct line_buffer {
  char buf[1024];
  int size;
  int used;
};

static void fillLineBuffer( struct input_buffer &inbuf, struct line_buffer &linebuf, bool &newline_found )
{
  char ch;
  linebuf.used = 0;
  newline_found = false;

  // fill line buffer from buf (load on demand)
  while ( linebuf.used < linebuf.size ) {

    // first read from buf until no byte is left or line buffer is full
    ch = 0;
    while ( inbuf.left > 0 && linebuf.used < linebuf.size ) {
      ch = inbuf.buf[inbuf.pos++];
      inbuf.left--;
      if ( ch != '\n' ) {
        if ( ch != 0 ) // ignore null byte
          linebuf.buf[linebuf.used++] = ch;
      } else {
        newline_found = true;
        break;
      }
    }
    if ( ch == '\n' || linebuf.used == linebuf.size ) break;

    // no newline found and line buffer not filled means no bytes left in buf
    if ( inbuf.eof_reached == false ) {
      int len = inbuf.pd->read( inbuf.buf, sizeof(inbuf.buf) );
      if ( len < 1 ) {
        // <0 isn't actually eof but it doesn't matter for content matching
        inbuf.eof_reached = true;
      } else {
        inbuf.left = len;
        inbuf.pos = 0;
      }
    } else break;
  }
  linebuf.buf[linebuf.used] = '\0';
}

int SearchThread::checkContent( const NWC::File &file )
{
  int matches = 0;

  if ( checkInterrupt() == true )
    return matches;

  if ( m_sets.getMatchContent().length() < 1 ) {
    _result_lock.lock();
    _results.push_back( SearchResult( file.getFullname(), -1 ) );
    _result_lock.unlock();
    matches = 1;
    return matches;
  }

  if ( file.isReg( true ) == false )
    return matches;

  struct input_buffer inbuf;
  struct line_buffer linebuf;
  PDatei pd;

  inbuf.left = 0;
  inbuf.pos = 0;
  inbuf.pd = &pd;
  inbuf.eof_reached = false;

  linebuf.size = sizeof( linebuf.buf ) - 1;
  linebuf.used = 0;

  int current_line = 0;
  int next_line = 0;
  bool newline_found = false;

  if ( pd.open( file.getFullname().c_str() ) == 0 ) {
    
    _current_action_lock.lock();
    _current_action = catalog.getLocale( 748 );
    _current_action += file.getFullname();
    _current_action_lock.unlock();
    
    while ( checkInterrupt() == false ) {
      // stop if no bytes are left in buffer and eof reached
      if ( inbuf.eof_reached == true && inbuf.left < 1 ) break;
      
      current_line = next_line;
      
      fillLineBuffer( inbuf, linebuf, newline_found );
      
      if ( newline_found == true )
        next_line = current_line + 1;
      
      matches += matchContentLine( file, linebuf.buf, current_line );
    }
  }
  return matches;
}

bool SearchThread::checkInterrupt()
{
  bool abort = false;

  _order_cv.lock();
  if ( _mode_order == SUSPEND ) {
    _current_mode = SUSPENDED;
    while ( _mode_order != STOP && _mode_order != SEARCH ) {
      _order_cv.wait();
    }
    _current_mode = SEARCHING;
  }
  if ( _mode_order == STOP ) {
    abort = true;
  }
  _order_cv.unlock();
  return abort;
}

SearchThread::SearchResult::SearchResult( const std::string &fullname, int linenr, bool is_dir ) : _fullname( fullname ),
                                                                                                   _linenr( linenr ),
                                                                                                   _is_dir( is_dir )
{
}

SearchThread::SearchResult::~SearchResult()
{
}

std::string SearchThread::SearchResult::getFullname() const
{
  return _fullname;
}

int SearchThread::SearchResult::getLineNr() const
{
  return _linenr;
}

bool SearchThread::SearchResult::getIsDir() const
{
  return _is_dir;
}

std::string SearchThread::getCurrentActionInfo()
{
  _current_action_lock.lock();
  std::string erg = _current_action;
  _current_action_lock.unlock();

  return erg;
}

void SearchThread::setSearchSettings( const SearchSettings &sets )
{
    m_sets = sets;
}
