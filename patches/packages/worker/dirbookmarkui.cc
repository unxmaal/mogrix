/* dirbookmarkui.cc
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2007-2021 Ralf Hoffmann.
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

#include "dirbookmarkui.hh"
#include "aguix/aguix.h"
#include "aguix/awindow.h"
#include "aguix/text.h"
#include "aguix/fieldlistview.h"
#include "aguix/button.h"
#include "aguix/solidbutton.h"
#include "worker_locale.h"
#include "bookmarkdbentry.hh"
#include "bookmarkdbfilter.hh"
#include "dirbookmarkaddui.hh"
#include "dirbookmarkeditui.hh"
#include "bookmarkdbproxy.hh"
#include "nwc_path.hh"
#include <algorithm>
#include "prefixdb.hh"
#include "worker.h"
#include "stringmatcher_flexibleregex.hh"
#include "async_job_limiter.hh"
#include "wconfig.h"
#include <iostream>

DirBookmarkUI::DirBookmarkUI( Worker &worker,
                              AGUIX &aguix,
                              BookmarkDBProxy &data  ) : m_worker( worker ),
                                                         m_aguix( aguix ),
                                                         m_data( data )
{
    m_filtered_data = std::unique_ptr<BookmarkDBFilter>( new BookmarkDBFilter( data ) );

    m_win = std::unique_ptr<AWindow>( new AWindow( &m_aguix,
                                                   0, 0,
                                                   400, 400,
                                                   catalog.getLocale( 1292 ) ) );
    m_win->create();

    m_co1 = m_win->setContainer( new AContainer( m_win.get(), 1, 5 ), true );
    m_co1->setMaxSpace( 5 );
    
    AContainer *co1_2 = m_co1->add( new AContainer( m_win.get(), 2, 1 ), 0, 0 );
    co1_2->setMaxSpace( 5 );
    co1_2->setBorderWidth( 0 );
    co1_2->add( new Text( &m_aguix, 0, 0, catalog.getLocale( 758 ) ),
                0, 0, AContainer::CO_FIX );
    m_infixtext = static_cast<Text*>( co1_2->add( new Text( &m_aguix, 0, 0, "" ),
                                                  1, 0, AContainer::CO_INCW ) );
    
    AContainer *co1_5 = m_co1->add( new AContainer( m_win.get(), 3, 1 ), 0, 1 );
    co1_5->setMaxSpace( 5 );
    co1_5->setBorderWidth( 0 );
    co1_5->add( new Text( &m_aguix, 0, 0, catalog.getLocale( 822 ) ),
                0, 0, AContainer::CO_FIX );

    AContainer *co1_5_1 = co1_5->add( new AContainer( m_win.get(), 3, 1 ), 1, 0 );
    co1_5_1->setMaxSpace( 0 );
    co1_5_1->setMinSpace( 0 );
    co1_5_1->setBorderWidth( 0 );
    m_prev_cat_b = (Button*)co1_5_1->add( new Button( &m_aguix, 0, 0,
                                                      "<", 0 ), 0, 0, AContainer::CO_FIX );
    m_cat_sb = dynamic_cast<SolidButton*>( co1_5_1->add( new SolidButton( &m_aguix, 0, 0,
                                                                          "", true ), 1, 0, AContainer::CO_INCW ) );
    m_next_cat_b = (Button*)co1_5_1->add( new Button( &m_aguix, 0, 0,
                                                      ">", 0 ), 2, 0, AContainer::CO_FIX );
    
    m_lv = static_cast<FieldListView*>( m_co1->add( new FieldListView( &m_aguix, 
                                                                     0, 0, 
                                                                     400, 300, 0 ),
                                                  0, 2, AContainer::CO_MIN ) );
    m_lv->setNrOfFields( 4 );
    m_lv->setGlobalFieldSpace( 5 );
    m_lv->setShowHeader( true );
    m_lv->setFieldText( 0, catalog.getLocale( 809 ) );
    m_lv->setFieldSpace( 0, 0 );
    m_lv->setFieldSpace( 1, 0 );
    m_lv->setFieldTextMerged( 0, true );
    m_lv->setFieldWidth( 0, m_aguix.getTextWidth( "?" ) );
    m_lv->setFieldText( 2, catalog.getLocale( 810 ) );
    m_lv->setFieldText( 3, catalog.getLocale( 823 ) );
    m_lv->setHBarState( 2 );
    m_lv->setVBarState( 2 );
    m_lv->setAcceptFocus( true );
    m_lv->setDisplayFocus( true );
    m_lv->setOnDemandCallback( [this]( FieldListView *lv,
                                       int row,
                                       int field,
                                       struct FieldListView::on_demand_data &data )
                               {
                                   updateLVData( row, field, data );
                               } );


    AContainer *co1_4 = m_co1->add( new AContainer( m_win.get(), 3, 1 ), 0, 3 );
    co1_4->setMinSpace( 5 );
    co1_4->setMaxSpace( -1 );
    co1_4->setBorderWidth( 0 );

    m_addb = (Button*)co1_4->add( new Button( &m_aguix,
                                              0,
                                              0,
                                              catalog.getLocale( 759 ),
                                              0 ), 0, 0, AContainer::CO_FIX );

    m_editb = (Button*)co1_4->add( new Button( &m_aguix,
                                               0,
                                               0,
                                               catalog.getLocale( 805 ),
                                               0 ), 1, 0, AContainer::CO_FIX );

    m_delb = (Button*)co1_4->add( new Button( &m_aguix,
                                              0,
                                              0,
                                              catalog.getLocale( 760 ),
                                              0 ), 2, 0, AContainer::CO_FIX );

    AContainer *co1_3 = m_co1->add( new AContainer( m_win.get(), 2, 1 ), 0, 4 );
    co1_3->setMinSpace( 5 );
    co1_3->setMaxSpace( -1 );
    co1_3->setBorderWidth( 0 );
    m_okb = (Button*)co1_3->add( new Button( &m_aguix,
                                             0,
                                             0,
                                             catalog.getLocale( 761 ),
                                             0 ), 0, 0, AContainer::CO_FIX );
    m_cancelb = (Button*)co1_3->add( new Button( &m_aguix,
                                                 0,
                                                 0,
                                                 catalog.getLocale( 633 ),
                                                 0 ), 1, 0, AContainer::CO_FIX );

    m_win->contMaximize( true );
    m_win->setDoTabCycling( true );
}

DirBookmarkUI::~DirBookmarkUI()
{
}

int DirBookmarkUI::mainLoop()
{
    showData( "" );

    maximizeWin();
    m_win->show();

    m_lv->takeFocus();

    std::string current_cat = "";
    AGMessage *msg;
    int endmode = 0;

    std::string cfgfile = Worker::getWorkerConfigDir();
#ifdef USEOWNCONFIGFILES
    cfgfile = NWC::Path::join( cfgfile, "prefix.db2" );
#else
    cfgfile = NWC::Path::join( cfgfile, "prefix.db" );
#endif

    PrefixDB pdb( cfgfile );

    updateCategoryText( current_cat );

    {
        std::string start_entry = NWC::Path::join( m_dirname,
                                                   m_basename );
        bool found = false;

        for ( int row = 0;; row++ ) {
            if ( m_lv->isValidRow( row ) == false ) break;
            if ( m_lv->getText( row, 1 ) == start_entry ) {
                m_lv->setActiveRow( row );
                m_lv->showActive();
                found = true;
                break;
            }
        }

        if ( ! found ) {
            /* first try to find an entry for which the start_entry
               is a prefix */

            int best_hit = -1;
            std::string::size_type best_hit_length = 0;

            for ( int row = 0;; row++ ) {
                if ( m_lv->isValidRow( row ) == false ) break;

                const std::string &entry_text = m_lv->getText( row, 1 );

                if ( entry_text.length() > start_entry.length() ) {
                    if ( entry_text.compare( 0,
                                             start_entry.length(),
                                             start_entry ) == 0 ) {
                        if ( best_hit < 0 ||
                             entry_text.length() < best_hit_length ) {
                            best_hit = row;
                            best_hit_length = entry_text.length();
                        }
                    }
                }
            }

            if ( best_hit >= 0 ) {
                m_lv->setActiveRow( best_hit );
                m_lv->showActive();
                found = true;
            }
        }

        if ( ! found ) {
            /* now try to find the largest prefix */

            int best_hit = -1;
            std::string::size_type best_hit_length = 0;

            for ( int row = 0;; row++ ) {
                if ( m_lv->isValidRow( row ) == false ) break;

                const std::string &entry_text = m_lv->getText( row, 1 );

                if ( entry_text.length() < start_entry.length() ) {
                    if ( start_entry.compare( 0,
                                              entry_text.length(),
                                              entry_text ) == 0 ) {
                        if ( best_hit < 0 ||
                             entry_text.length() > best_hit_length ) {
                            best_hit = row;
                            best_hit_length = entry_text.length();
                        }
                    }
                }
            }

            if ( best_hit >= 0 ) {
                m_lv->setActiveRow( best_hit );
                m_lv->showActive();
                found = true;
            }
        }
    }

    for ( ; endmode == 0; ) {
        if ( m_existence_tests.empty() ) {
            msg = m_aguix.WaitMessage( NULL );
        } else {
            msg = m_aguix.GetMessage( NULL );
        }

        checkExistResults();

        if ( msg != NULL ) {
            switch ( msg->type ) {
              case AG_CLOSEWINDOW:
                  endmode = -1;
                  break;
              case AG_BUTTONCLICKED:
                  if ( msg->button.button == m_okb ) {
                      endmode = 1;
                  } else if ( msg->button.button == m_cancelb ) {
                      endmode = -1;
                  } else if ( msg->button.button == m_addb ) {
                      DirBookmarkAddUI addui( m_aguix, m_data, m_dirname, m_basename );

                      if ( addui.mainLoop() > 0 ) {
                          m_data.write();
                          showData( current_cat );
                      }
                  } else if ( msg->button.button == m_editb ) {
                      BookmarkDBEntry active_entry = getActiveEntry( current_cat );
                      if ( active_entry.getName().length() > 0 ) {
                          DirBookmarkEditUI editui( m_aguix, m_data, active_entry );
                          
                          if ( editui.mainLoop() > 0 ) {
                              m_data.write();
                              showData( current_cat );
                          }
                      }
                  } else if ( msg->button.button == m_delb ) {
                      BookmarkDBEntry active_entry = getActiveEntry( current_cat );
                      if ( active_entry.getName().length() > 0 ) {
                          std::string buttontext = catalog.getLocale( 11 );
                          buttontext += "|";
                          buttontext += catalog.getLocale( 8 );

                          int erg = m_win->request( catalog.getLocale( 123 ),
                                                    catalog.getLocale( 762 ),
                                                    buttontext.c_str() );
                          if ( erg == 0 ) {
                              m_data.delEntry( active_entry );
                              m_data.write();
                              showData( current_cat );
                          }
                      }
                  } else if ( msg->button.button == m_prev_cat_b ) {
                      current_cat = switchCat( current_cat, -1 );
                      showData( current_cat );
                      updateCategoryText( current_cat );
                  } else if ( msg->button.button == m_next_cat_b ) {
                      current_cat = switchCat( current_cat, 1 );
                      showData( current_cat );
                      updateCategoryText( current_cat );
                  }
                  break;
              case AG_KEYPRESSED:
                  if ( msg->key.key == XK_BackSpace ) {
                      if ( KEYSTATEMASK( msg->key.keystate ) == ShiftMask ) {
                          m_filtered_data->setInfixFilter( "" );
                          showData( current_cat );
			  highlightBestHit( pdb );
                      } else {
                          std::string cur_infix = m_filtered_data->getInfixFilter();
                          
                          if ( cur_infix.length() > 0 ) {
                              cur_infix.resize( cur_infix.length() - 1 );
                              m_filtered_data->setInfixFilter( cur_infix );
                              showData( current_cat );
			      highlightBestHit( pdb );
                          }
                      }
                  } else if ( msg->key.key == XK_Return ) {
                      endmode = 1;
                  } else if ( msg->key.key == XK_Escape ) {
                      endmode = -1;
                  } else if ( msg->key.key == XK_Left ) {
                      current_cat = switchCat( current_cat, -1 );
                      showData( current_cat );
		      highlightBestHit( pdb );
                      updateCategoryText( current_cat );
                  } else if ( msg->key.key == XK_Right ) {
                      current_cat = switchCat( current_cat, 1 );
                      showData( current_cat );
		      highlightBestHit( pdb );
                      updateCategoryText( current_cat );
                  } else if ( msg->key.key == XK_Up ) {
                  } else if ( msg->key.key == XK_Down ) {
                  } else if ( msg->key.key == XK_Next ) {
                  } else if ( msg->key.key == XK_Prior ) {
                  } else if ( msg->key.key == XK_Home ) {
                  } else if ( msg->key.key == XK_End ) {
                  } else if ( IsModifierKey( msg->key.key ) ||
                              IsCursorKey( msg->key.key ) ||
                              IsPFKey( msg->key.key ) ||
                              IsFunctionKey( msg->key.key ) ||
                              IsMiscFunctionKey( msg->key.key ) ||
                              ( ( msg->key.key >= XK_BackSpace ) && ( msg->key.key <= XK_Escape ) ) ) {
                  } else if ( strlen( msg->key.keybuf ) > 0 ) {
                      std::string cur_infix = m_filtered_data->getInfixFilter(), old_infix;
                      old_infix = cur_infix;
                      cur_infix += msg->key.keybuf;
                      m_filtered_data->setInfixFilter( cur_infix );
                      if ( m_filtered_data->getEntries( current_cat ).empty() == true ) {
                          m_filtered_data->setInfixFilter( old_infix );
                      }
                      showData( current_cat );
		      highlightBestHit( pdb );
                  }
                  break;
              case AG_FIELDLV_DOUBLECLICK:
                  if ( msg->fieldlv.lv == m_lv ) {
                      // double click in lv, actual element is unimportant here
                      endmode = 1;
                  }
                  break;
                case AG_FIELDLV_HEADERCLICKED:
                    if ( ( msg->fieldlv.lv == m_lv ) &&
                         ( msg->fieldlv.button == Button1 ) ) {
                        // store current active entry
                        BookmarkDBEntry active_entry = getActiveEntry( current_cat );

                        // switch sort mode
                        switch ( msg->fieldlv.row ) {
                            case 0:
                                m_filtered_data->toggleSortMode( BookmarkDBFilter::SORT_BY_NAME );
                                showData( current_cat );
                                break;
                            case 2:
                                m_filtered_data->toggleSortMode( BookmarkDBFilter::SORT_BY_ALIAS );
                                showData( current_cat );
                                break;
                            case 4:
                                m_filtered_data->toggleSortMode( BookmarkDBFilter::SORT_BY_CATEGORY );
                                showData( current_cat );
                                break;
                            default:
                                break;
                        }

                        // find previously stored entry and activate it
                        std::list<BookmarkDBEntry> e = m_filtered_data->getEntries( current_cat );
                        int row = 0;
                        for ( std::list<BookmarkDBEntry>::iterator it1 = e.begin();
                              it1 != e.end();
                              it1++, row++ ) {
                            if ( *it1 == active_entry ) {
                                m_lv->setActiveRow( row );
                                break;
                            }
                        }
                    }
                    break;
            }
            m_aguix.ReplyMessage( msg );

            if ( endmode == 1 ) {
                int row = m_lv->getActiveRow();
                if ( m_lv->isValidRow( row ) ) {
                    std::string path = m_lv->getText( row, 1 );

                    if ( ! NWC::FSEntry( path ).entryExists() ) {
                        endmode = 0;
                    }
                }
            }
        }
    }

    m_selected_entry.reset();

    if ( endmode == 1 ) {
        int row = m_lv->getActiveRow();
        if ( m_lv->isValidRow( row ) == true ) {
            std::list<BookmarkDBEntry> e = m_filtered_data->getEntries( current_cat );
            for ( std::list<BookmarkDBEntry>::iterator it1 = e.begin();
                  it1 != e.end();
                  it1++, row-- ) {
                if ( row == 0 ) {
                    m_selected_entry = std::unique_ptr<BookmarkDBEntry>( new BookmarkDBEntry( *it1 ) );
                    pdb.pushAccess( m_filtered_data->getInfixFilter(), it1->getName(), time( NULL ) );
                }
            }
        }
    }

    m_win->hide();
    
    moveTests();

    return endmode;
}

void DirBookmarkUI::showData( const std::string &cat )
{
    StringMatcherFlexibleRegEx matcher;

    matcher.setMatchFlexible( false );

    matcher.setMatchString( m_filtered_data->getInfixFilter() );

    m_infixtext->setText( m_filtered_data->getInfixFilter().c_str() );

    int act_row = m_lv->getActiveRow();

    m_lv->setSize( 0 );
    
    // invalidate running futures
    for ( auto &e : m_existence_tests ) {
        std::get<2>( e ) = false;
    }

    std::list<BookmarkDBEntry> e = m_filtered_data->getEntries( cat );
    for ( std::list<BookmarkDBEntry>::iterator it1 = e.begin();
          it1 != e.end();
          it1++ ) {
        int row = m_lv->addRow();
        m_lv->setText( row, 1, it1->getName() );
        m_lv->setText( row, 2, it1->getAlias() );
        m_lv->setText( row, 3, it1->getCategory() );
        m_lv->setPreColors( row, FieldListView::PRECOLOR_ONLYACTIVE );
        
        if ( row == act_row )
            m_lv->setActiveRow( row );

        if ( ! m_filtered_data->getInfixFilter().empty() ) {
            std::vector< size_t > segments = matcher.getMatchSegments( it1->getName() );

            if ( segments.size() > 1 ) {
                m_lv->setHighlightSegments( row, 1, segments );
            }
        }
    }

    if ( m_lv->isValidRow( m_lv->getActiveRow() ) == false ) {
        if ( act_row < 0 ) {
            m_lv->setActiveRow( 0 );
        } else {
            m_lv->setActiveRow( e.size() - 1 );
        }
    }

    m_lv->redraw();
}

BookmarkDBEntry DirBookmarkUI::getSelectedEntry()
{
    if ( m_selected_entry.get() != NULL ) {
        return *m_selected_entry;
    }
    /* IRIX fix: return empty entry instead of throwing.
       DWARF unwinder crashes on IRIX libpthread frames. */
    return BookmarkDBEntry( "", "" );
}

std::string DirBookmarkUI::getFilterString()
{
    return m_filtered_data->getInfixFilter();
}

void DirBookmarkUI::setCurrentDirname( const std::string &dirname )
{
    m_dirname = dirname;
}

void DirBookmarkUI::setCurrentBasename( const std::string &basename )
{
    m_basename = basename;
}

BookmarkDBEntry DirBookmarkUI::getActiveEntry( const std::string &cat )
{
    int row = m_lv->getActiveRow();
    BookmarkDBEntry dbe( "", "" );
    
    if ( m_lv->isValidRow( row ) == true ) {
        std::list<BookmarkDBEntry> e = m_filtered_data->getEntries( cat );
        for ( std::list<BookmarkDBEntry>::iterator it1 = e.begin();
              it1 != e.end();
              it1++, row-- ) {
            if ( row == 0 ) {
                dbe = *it1;
            }
        }
    }
    return dbe;
}

std::string DirBookmarkUI::switchCat( const std::string &current_cat, int dir )
{
    std::list<std::string> cats = m_data.getCats();
    std::string new_cat = current_cat;

    if ( dir == 0 || cats.empty() == true ) return "";

    if ( current_cat == "" ) {
        // "all"
        if ( dir > 0 ) {
            new_cat = cats.front();
        } else {
            new_cat = cats.back();
        }
    } else {
        std::list<std::string>::iterator it1 = std::find( cats.begin(),
                                                          cats.end(),
                                                          current_cat );

        if ( it1 != cats.end() ) {
            if ( dir < 0 ) {
                // to the left
                if ( it1 == cats.begin() )
                    new_cat = "";
                else {
                    it1--;
                    new_cat = *it1;;
                }
            } else {
                it1++;
                if ( it1 == cats.end() )
                    new_cat = "";
                else
                    new_cat = *it1;
            }
        } else {
            new_cat = "";
        }
    }
    return new_cat;
}

void DirBookmarkUI::updateCategoryText( const std::string &current_cat )
{
    if ( current_cat == "" ) {
        m_cat_sb->setText( catalog.getLocale( 824 ) );
    } else {
        m_cat_sb->setText( current_cat.c_str() );
    }
}

void DirBookmarkUI::maximizeWin()
{
    m_lv->maximizeX();
    m_lv->maximizeY();
    int my_w = m_lv->getWidth() + 10;
    int my_h = m_lv->getHeight() + 10;

    if ( my_w < 400 ) my_w = 400;
    if ( my_h < 300 ) my_h = 300;

    int rx, ry, rw, rh;

    m_aguix.getLargestDimensionOfCurrentScreen( &rx, &ry,
                                                &rw, &rh );

    int mw = rw * 80 / 100;
    int mh = rh * 80 / 100;
    m_co1->resize( mw, mh );
    m_co1->rearrange();

    if ( my_w < m_lv->getWidth() ) {
        m_co1->setMinWidth( my_w, 0, 2 );
    } else {
        m_co1->setMinWidth( m_lv->getWidth(), 0, 2 );
    }
    if ( my_h < m_lv->getHeight() ) {
        m_co1->setMinHeight( my_h, 0, 2 );
    } else {
        m_co1->setMinHeight( m_lv->getHeight(), 0, 2 );
    }
    m_win->contMaximize( true );
}

void DirBookmarkUI::highlightBestHit( class PrefixDB &pdb )
{
    if ( m_filtered_data->getInfixFilter().empty() ) return;

    time_t t1;
    std::string best_hit = pdb.getBestHit( m_filtered_data->getInfixFilter(), t1 );

    if ( best_hit.empty() ) return;

    // search for best_hit in visible entries

    for ( int row = 0;; row++ ) {
	if ( m_lv->isValidRow( row ) == false ) break;
	if ( m_lv->getText( row, 1 ) == best_hit ) {
	    m_lv->setActiveRow( row );
	    m_lv->showActive();
	    break;
	}
    }
}

void DirBookmarkUI::updateLVData( int row, int field,
                                  struct FieldListView::on_demand_data &data )
{
    if ( field == 0 ) {
        std::string path = m_lv->getText( row, 1 );

        if ( m_existence_results.count( path ) == 0 ) {
            bool async_started = false;
            bool do_check = true;

            if ( ! wconfig->getDisableBGCheckPrefix().empty() &&
                 AGUIXUtils::starts_with( path, wconfig->getDisableBGCheckPrefix() ) ) {
                do_check = false;
            }
            
            if ( do_check && ! AsyncJobLimiter::async_job_limit_reached() ) {
                try {
                    auto f = std::async( std::launch::async,
                                         [path] {
                                             AsyncJobLimiter::inc_async_job();
                                             auto res = ExistenceTest::checkPathExistence( path );
                                             AsyncJobLimiter::dec_async_job();

                                             return res;
                                         } );
                    m_existence_tests.push_back( std::make_tuple( std::move( f ), row, true ) );

                    data.text = "?";
                    data.text_set = true;

                    async_started = true;
                } catch ( std::system_error &e ) {
                    std::cout << e.what() << std::endl;
                }
            }

            if ( ! async_started ) {
                std::promise< ExistenceTest::existence_state_t > promise;
                auto f = promise.get_future();

                m_existence_tests.push_back( std::make_tuple( std::move( f ), row, true ) );

                if ( do_check ) {
                    promise.set_value( ExistenceTest::checkPathExistence( path ) );
                } else {
                    promise.set_value( { .exists = ExistenceTest::EXISTS_YES,
                            .path_length = path.size() } );
                }

                data.text = "?";
                data.text_set = true;
            }
        } else {
            auto &res = m_existence_results.at( path );

            if ( res.exists == ExistenceTest::EXISTS_YES ) {
                data.text = "";
                data.text_set = true;
            } else if ( res.exists == ExistenceTest::EXISTS_NO ) {
                data.text = "";
                data.text_set = true;
            }
        }
    } else if ( field == 1 ) {
        std::string path = m_lv->getText( row, 1 );

        if ( m_existence_results.count( path ) > 0 ) {
            auto &res = m_existence_results.at( path );

            if ( res.exists == ExistenceTest::EXISTS_NO ) {
                data.strike_out = true;
                data.strike_out_set = true;
                data.strike_out_start = res.path_length + 1;
                data.strike_out_start_set = true;
            }

            data.done = true;
        }
    }
}

void DirBookmarkUI::moveTests()
{
    for ( auto it = m_existence_tests.begin();
          it != m_existence_tests.end();
          ) {
        std::future_status status = std::get<0>( *it ).wait_for( std::chrono::duration<int>::zero() );

        auto next_it = it;
        next_it++;

        if ( status != std::future_status::ready ) {
            auto f = std::move( std::get<0>( *it ) );

            m_worker.enqueueFuture( std::move( f ) );
        }

        m_existence_tests.erase( it );
        it = next_it;
    }
}

void DirBookmarkUI::checkExistResults()
{
    if ( m_existence_tests.empty() ) {
        return;
    }

    bool update_lv = false;

    for ( auto it = m_existence_tests.begin();
          it != m_existence_tests.end();
          ) {
        std::future_status status = std::get<0>( *it ).wait_for(std::chrono::duration<int>::zero());

        if ( status == std::future_status::ready ) {
            ExistenceTest::existence_state_t res = std::get<0>( *it ).get();

            if ( std::get<2>( *it ) == true ) {
                int row = std::get<1>( *it );

                if ( m_lv->isValidRow( row ) ) {
                    std::string path = m_lv->getText( row, 1 );

                    m_existence_results[ path ] = res;

                    m_lv->clearOnDemandDataAvailableFlag( row );
                    update_lv = true;
                }
            }

            auto next_it = it;
            next_it++;

            m_existence_tests.erase( it );

            it = next_it;
        } else {
            it++;
        }
    }

    if ( update_lv ) {
        m_lv->redraw();
    }
}
