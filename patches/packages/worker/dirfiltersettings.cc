/* dirfiltersets.cc
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2007-2022 Ralf Hoffmann.
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

#include "dirfiltersettings.hh"
#include "fileentry.hh"
#include "nwcentryselectionstate.hh"
#include "wconfig.h"
#include "nwc_path.hh"
#include <algorithm>

DirFilterSettings::DirFilterSettings() : _serial_nr( 0 ),
                                         _show_hidden( true ),
                                         _ins( 0 ),
                                         _outs( 0 ),
                                         m_highlight_bookmark_prefix( true ),
                                         m_bookmark_filter( SHOW_ALL ),
                                         m_bookmark_label( "" )
{
}

DirFilterSettings::~DirFilterSettings()
{
}

DirFilterSettings::DirFilterSettings( const DirFilterSettings &other ) :
    _serial_nr( other._serial_nr ),
    _show_hidden( other._show_hidden ),
    _filters( other._filters ),
    _ins( other._ins ),
    _outs( other._outs ),
    m_highlight_bookmark_prefix( other.m_highlight_bookmark_prefix ),
    m_bookmark_filter( other.m_bookmark_filter ),
    m_bookmark_label( other.m_bookmark_label )
{
}

bool DirFilterSettings::check( const std::string &dirname,
                               const NWC::FSEntry *e )
{
    bool use = true;

    if ( e == NULL ) return false;

    if ( _show_hidden == false &&
         e->isHiddenEntry() == true &&
         e->getBasename() != ".." ) {
        
        use = false;
    }

    if ( use == true ) {
        if ( _string_filter.get() != NULL &&
             e->getBasename() != ".." ) {
            if ( dirname.empty() ) {
                if ( _string_filter->match( e->getBasename() ) == false ) {
                    use = false;
                }
            } else {
                if ( _string_filter->match( NWC::Path::get_extended_basename( dirname,
                                                                              e->getFullname() ) ) == false ) {
                    use = false;
                }
            }
        }
    }

    if ( use == true ) {
        bool isIn;
        
        if ( e->isDir() == true ) {
            // a dir is always visible
            use = true;
        } else {
            if ( _ins > 0 ) isIn = false;
            else isIn = true;
            
            for ( const auto &it : _filters ) {
                if ( it.getCheck() > 0 ) {
                    if ( fnmatch( it.getPattern(), e->getBasename().c_str(), 0 ) == 0 ) {
                        if ( it.getCheck() == 1 ) isIn = true;
                        else isIn = false;
                        break;
                    }
                }
            }
            use = isIn;
        }
    }

    return use;
}

bool DirFilterSettings::check( const std::string &dirname,
                               NWCEntrySelectionState &es )
{
    bool use = true;

    const auto e = es.getNWCEntry();

    if ( e == NULL ) return false;

    use = check( dirname, e );

    if ( use == true ) {
        if ( m_generic_filter.get() != NULL &&
             e->getBasename() != ".." ) {

            if ( m_generic_filter->check( es ) == false ) {
                use = false;
            }
        }
    }

    if ( use == true && m_bookmark_filter != SHOW_ALL ) {
        if ( m_bookmark_filter == SHOW_ONLY_BOOKMARKS ) {
            if ( es.getBookmarkMatch() == FileEntry::NOT_IN_BOOKMARKS ) {
                use = false;
            }
        } else if ( m_bookmark_filter == SHOW_ONLY_LABEL ) {
            if ( es.hasMatchingLabels() == true ) {
                try {
                    const auto &labels = es.getMatchingLabels();
                    
                    if ( std::find( labels.begin(),
                                    labels.end(),
                                    m_bookmark_label ) == labels.end() ) {
                        use = false;
                    }
                } catch ( int ) {
                    use = false;
                }
            } else {
                use = false;
            }
        }
    }

    if ( use == true ) {
        bool clear_override = true;

        if ( m_highlight_bookmark_prefix == true || es.getBookmarkMatch() == FileEntry::BOOKMARK ) {
            if ( es.hasMatchingLabels() == true ) {
                try {
                    const auto &matching_labels = es.getMatchingLabels();
                    const auto &label_colors = wconfig->getColorDefs().getLabelColors();
                    
                    if ( matching_labels.empty() == false ) {
                        auto label_it = label_colors.end();

                        if ( m_bookmark_filter != SHOW_ONLY_LABEL ) {
                            label_it = label_colors.find( matching_labels.front() );
                        } else {
                            label_it = label_colors.find( m_bookmark_label );
                        }
                        if ( label_it != label_colors.end() ) {
                            WConfig::ColorDef::label_colors_t col = label_it->second;
                            es.setOverrideColor( FileEntryCustomColor::NORMAL,
                                                 col.normal_fg,
                                                 col.normal_bg );
                            es.setOverrideColor( FileEntryCustomColor::ACTIVE,
                                                 col.active_fg,
                                                 col.active_bg );
                            clear_override = false;
                        }
                    }
                } catch ( int ) {
                }
            }
        }
        if ( clear_override == true ) {
            es.clearOverrideColor();
        }
    }

    es.setUse( use );

    return use;
}

void DirFilterSettings::setShowHidden( bool nv )
{
    _show_hidden = nv;
    _serial_nr++;
}

bool DirFilterSettings::getShowHidden() const
{
    return _show_hidden;
}

int DirFilterSettings::getSerialNr() const
{
    return _serial_nr;
}

void DirFilterSettings::setFilters( const std::list<NM_Filter> &filters )
{
    _filters = filters;
    calcInAndOuts();
    _serial_nr++;
}

std::list<NM_Filter> DirFilterSettings::getFilters() const
{
    return _filters;
}

bool DirFilterSettings::filterActive() const
{
    bool res = false;

    std::list<NM_Filter>::const_iterator it;
    
    for ( it = _filters.begin();
          it != _filters.end();
          it++ ) {
        if ( it->getCheck() != 0 ) {
            res = true;
            break;
        }
    }

    return res;
}

void DirFilterSettings::unsetAllFilters()
{
    std::list<NM_Filter>::iterator it;
    
    for ( it = _filters.begin();
          it != _filters.end();
          it++ ) {
        it->setCheck( NM_Filter::INACTIVE );
    }
    
    calcInAndOuts();
    _serial_nr++;
}

void DirFilterSettings::setFilter( const char *filter, NM_Filter::check_t mode )
{
    std::list<NM_Filter>::iterator it;
    
    for ( it = _filters.begin();
          it != _filters.end();
          it++ ) {
        if ( strcmp( it->getPattern(), filter ) == 0 )
            break;
    }

    if ( it != _filters.end() ) {
        it->setCheck( mode );
    } else {
        NM_Filter fi;
        fi.setCheck( mode );
        fi.setPattern( filter );
        _filters.push_back( fi );
    }
    
    calcInAndOuts();
    _serial_nr++;
}

void DirFilterSettings::calcInAndOuts()
{
    std::list<NM_Filter>::const_iterator it;
        
    _ins = 0;
    _outs = 0;
    
    for ( it = _filters.begin();
          it != _filters.end();
          it++ ) {
        if ( it->getCheck() == 1 ) _ins++;
        if ( it->getCheck() == 2 ) _outs++;
    }
}

void DirFilterSettings::setStringFilter( std::unique_ptr<StringMatcher> filter )
{
    _string_filter = std::move( filter );

    _serial_nr++;
}

void DirFilterSettings::setGenericFilter( std::unique_ptr< GenericDirectoryFilter > filter )
{
    m_generic_filter = std::move( filter );

    _serial_nr++;
}

void DirFilterSettings::bookmarksChanged()
{
    _serial_nr++;
}

void DirFilterSettings::setHighlightBookmarkPrefix( bool nv )
{
    m_highlight_bookmark_prefix = nv;

    _serial_nr++;
}

bool DirFilterSettings::getHighlightBookmarkPrefix() const
{
    return m_highlight_bookmark_prefix;
}

void DirFilterSettings::setBookmarkFilter( bookmark_filter_t v )
{
    m_bookmark_filter = v;

    _serial_nr++;
}

DirFilterSettings::bookmark_filter_t DirFilterSettings::getBookmarkFilter() const
{
    return m_bookmark_filter;
}

void DirFilterSettings::setSpecificBookmarkLabel( const std::string &l )
{
    m_bookmark_label = l;

    _serial_nr++;
}

const std::string &DirFilterSettings::getSpecificBookmarkLabel() const
{
    return m_bookmark_label;
}

NM_Filter::check_t DirFilterSettings::checkFilter( const char *filter )
{
    std::list<NM_Filter>::iterator it;
    
    for ( it = _filters.begin();
          it != _filters.end();
          ++it ) {
        if ( strcmp( it->getPattern(), filter ) == 0 ) {
            break;
        }
    }

    if ( it == _filters.end() ) {
        /* IRIX fix: return INACTIVE instead of throwing.
           DWARF unwinder crashes on IRIX libpthread frames. */
        return NM_Filter::INACTIVE;
    }

    return it->getCheck();
}
