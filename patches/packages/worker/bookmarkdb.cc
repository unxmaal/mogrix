/* bookmarkdb.cc
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2007-2008,2013 Ralf Hoffmann.
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

#include "bookmarkdb.hh"
#include "bookmarkdbentry.hh"
#include <fstream>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <sys/time.h>
#include "bookmarkdb_scanner.hh"
#include "datei.h"
#include "aguix/lowlevelfunc.h"

BookmarkDB::BookmarkDB( const std::string &filename ) : m_filename( filename )
{
}

BookmarkDB::~BookmarkDB()
{
}

int BookmarkDB::read()
{
    FILE *fp = NULL;
    std::list< BookmarkDBEntry > loaded_entries;

    fp = worker_fopen( m_filename.c_str(), "r" );

    /* IRIX workaround: eliminate all throw/catch(int) from parsing.
     * On IRIX 6.5, the GCC DWARF unwinder crashes (uw_update_context_1
     * NULL deref) when processing throw because IRIX libpthread startup
     * frames have incompatible unwind info.  Use goto-based error flow
     * instead of exceptions for parse error handling. */
    if ( fp != NULL ) {
        bmdbrestart( fp );
        bmdb_readtoken();

        if ( bmdb_token == BMDB_BOOKMARKS ) {
            if ( match( BMDB_BOOKMARKS ) < 0 ) goto parse_error;
            if ( match( '{' ) < 0 ) goto parse_error;

            while ( bmdb_token == BMDB_ENTRY ) {
                if ( match( BMDB_ENTRY ) < 0 ) goto parse_error;
                if ( match( '{' ) < 0 ) goto parse_error;

                BookmarkDBEntry fbe( "", "" );
                for (;;) {
                    if ( bmdb_token == BMDB_NAME ) {
                        if ( match( BMDB_NAME ) < 0 ) goto parse_error;
                        if ( match( '=' ) < 0 ) goto parse_error;
                        if ( bmdb_token == BMDB_STRING ) {
                            fbe.setName( bmdb_token_str );
                        }
                        if ( match( BMDB_STRING ) < 0 ) goto parse_error;
                        if ( match( ';' ) < 0 ) goto parse_error;
                    } else if ( bmdb_token == BMDB_ALIAS ) {
                        if ( match( BMDB_ALIAS ) < 0 ) goto parse_error;
                        if ( match( '=' ) < 0 ) goto parse_error;
                        if ( bmdb_token == BMDB_STRING ) {
                            fbe.setAlias( bmdb_token_str );
                        }
                        if ( match( BMDB_STRING ) < 0 ) goto parse_error;
                        if ( match( ';' ) < 0 ) goto parse_error;
                    } else if ( bmdb_token == BMDB_USEPARENT ) {
                        if ( match( BMDB_USEPARENT ) < 0 ) goto parse_error;
                        if ( match( '=' ) < 0 ) goto parse_error;
                        if ( bmdb_token == BMDB_TRUE ) {
                            fbe.setUseParent( true );
                            if ( match( BMDB_TRUE ) < 0 ) goto parse_error;
                        } else if ( bmdb_token == BMDB_FALSE ) {
                            fbe.setUseParent( false );
                            if ( match( BMDB_FALSE ) < 0 ) goto parse_error;
                        } else {
                            goto parse_error;
                        }
                        if ( match( ';' ) < 0 ) goto parse_error;
                    } else if ( bmdb_token == BMDB_CATEGORY ) {
                        if ( match( BMDB_CATEGORY ) < 0 ) goto parse_error;
                        if ( match( '=' ) < 0 ) goto parse_error;
                        if ( bmdb_token == BMDB_STRING ) {
                            fbe.setCategory( bmdb_token_str );
                        }
                        if ( match( BMDB_STRING ) < 0 ) goto parse_error;
                        if ( match( ';' ) < 0 ) goto parse_error;
                    } else {
                        break;
                    }
                }
                if ( match( '}' ) < 0 ) goto parse_error;

                loaded_entries.push_back( fbe );
            }

            if ( match( '}' ) < 0 ) goto parse_error;
        }

        if (0) {
parse_error:
            std::cerr << "Error in line " << bmdb_linenr << " while parsing bookmarkdb config." << std::endl;
        }
        worker_fclose( fp );
        fp = NULL;
    }

    bool same_bookmarks = true;
    std::list< BookmarkDBEntry >::iterator old_it = _entries.begin();
    std::list< BookmarkDBEntry >::iterator new_it = loaded_entries.begin();

    for ( ;; ++new_it, ++old_it ) {
        if ( old_it == _entries.end() && new_it == loaded_entries.end() ) break;

        if ( old_it == _entries.end() ) {
            same_bookmarks = false;
            break;
        }
        if ( new_it == loaded_entries.end() ) {
            same_bookmarks = false;
            break;
        }

        if ( new_it->equals( *old_it, true ) == false ) {
            same_bookmarks = false;
            break;
        }
    }

    if ( same_bookmarks == false ) {
        _entries = loaded_entries;
        return 0;
    } else {
        return 1;
    }
}

int BookmarkDB::write()
{
    Datei fh;
    if ( fh.open( m_filename.c_str(), "w" ) == 0 ) {

        fh.configOpenSection( "bookmarks" );

        for ( entries_it it1 = _entries.begin();
              it1 != _entries.end();
              it1++ ) {

            fh.configOpenSection( "entry" );

            fh.configPutPairString( "name", it1->getName().c_str() );
            fh.configPutPairBool( "useparent", it1->getUseParent() );

            if ( it1->getAlias().length() > 0 ) {
                fh.configPutPairString( "alias", it1->getAlias().c_str() );
            }

            if ( it1->getCategory().length() > 0 ) {
                fh.configPutPairString( "category", it1->getCategory().c_str() );
            }

            fh.configCloseSection();
        }

        fh.configCloseSection();

        fh.close();
    } else {
        return 1;
    }

    return 0;
}

std::list<std::string> BookmarkDB::getCats()
{
    std::list<std::string> cats;

    for ( entries_it it1 = _entries.begin();
          it1 != _entries.end();
          it1++ ) {
        std::string c = it1->getCategory();
        if ( c.length() > 0 ) {
            if ( std::find( cats.begin(), cats.end(), c ) == cats.end() )
                cats.push_back( c );
        }
    }
    return cats;
}

std::list< BookmarkDBEntry> BookmarkDB::getEntries( const std::string &cat )
{
    std::list< BookmarkDBEntry > matched_entries;

    for ( entries_it it1 = _entries.begin();
          it1 != _entries.end();
          it1++ ) {
        std::string c = it1->getCategory();
        if ( cat == "" ||
             c == cat ) {
            matched_entries.push_back( *it1 );
        }
    }
    return matched_entries;
}

std::list< std::string > BookmarkDB::getNamesOfEntries( const std::string &cat )
{
    std::list< std::string > matched_entries;

    for ( entries_it it1 = _entries.begin();
          it1 != _entries.end();
          it1++ ) {
        std::string c = it1->getCategory();
        if ( cat == "" ||
             c == cat ) {
            matched_entries.push_back( it1->getName() );
        }
    }
    return matched_entries;
}

void BookmarkDB::addEntry( const BookmarkDBEntry &entry )
{
    _entries.push_back( entry );
}

void BookmarkDB::delEntry( const BookmarkDBEntry &entry )
{
    entries_it it1 = std::find( _entries.begin(),
                                _entries.end(),
                                entry );
    if ( it1 != _entries.end() ) {
        _entries.erase( it1 );
    }
}

int BookmarkDB::match( int token )
{
    if ( bmdb_token == token ) {
        bmdb_readtoken();
        return 0;
    }
    return -1;
}

void BookmarkDB::updateEntry( const BookmarkDBEntry &entry,
                              const BookmarkDBEntry &newentry )
{
    entries_it it1 = std::find( _entries.begin(),
                                _entries.end(),
                                entry );
    if ( it1 != _entries.end() ) {
        *it1 = newentry;
    }
}

std::unique_ptr<BookmarkDBEntry> BookmarkDB::getEntry( const std::string &name )
{
    std::unique_ptr<BookmarkDBEntry> res;

    for ( entries_it it1 = _entries.begin();
          it1 != _entries.end();
          it1++ ) {
        if ( it1->getName() == name ) {
            res = std::unique_ptr<BookmarkDBEntry>( new BookmarkDBEntry( *it1 ) );
            break;
        }
    }
    return res;
}
