/* searchopbg.cc
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2006-2014 Ralf Hoffmann.
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

#include "searchopbg.hh"
#include "listermode.h"
#include "virtualdirmode.hh"
#include "worker.h"
#include "execlass.h"
#include "nwc_virtualdir.hh"
#include "fileviewer.hh"
#include "flagreplacer.hh"
#include "aguix/acontainerbb.h"
#include <typeinfo>
#include "filenameshrinker.hh"
#include "aguix/kartei.h"
#include "aguix/stringgadget.h"
#include "aguix/choosebutton.h"
#include "aguix/button.h"
#include "calendar.hh"
#include "worker_locale.h"
#include "wconfig.h"
#include "resultstore.hh"
#include "virtualdirmode.hh"
#include "argclass.hh"

static const int searchopbg_timeout = 10;

// result store shared by all searchop instances
// public methods need to acquire lock, protected methods consider this structued as locked
// THIS IS NOT REALLY THREAD-SAFE! multiple SearchOp instances can use
// this structure but they are not allowed to run in to threads currently
// (which is not possible at the moment)
// currently it's secured by a lock so multiple instaces block each other
// but because of the pointer in the list it's not easily shareable

static std::list<ResultStore*> _stored_results;
static int _stored_results_changed = 0;
int SearchOpBG::s_search_number = 1;

#define DEFAULT_EDIT_COMMAND "{scripts}/xeditor -line {l} {f}"

void SearchOpBG::clear_stored_results()
{
    for ( std::list<ResultStore*>::iterator it1 = _stored_results.begin();
          it1 != _stored_results.end();
           ) {
        std::list<ResultStore*>::iterator new_it1;

        if ( (*it1)->is_exclusive() ) {
            it1++;
        } else {
            delete *it1;
            new_it1 = it1;
            new_it1++;

            _stored_results.erase( it1 );

            it1 = new_it1;
        }
    }

    _stored_results_changed++;
}

SearchOpBGTimeoutCB::SearchOpBGTimeoutCB( SearchOpBG *s ) : m_s( s )
{}

void SearchOpBGTimeoutCB::callback()
{
    if ( m_s ) {
        m_s->timeout_callback();
    }
}

void SearchOpBGTimeoutCB::reset()
{
    m_s = NULL;
}

void SearchOpBG::updateState( Text &state_text )
{
    std::string statetxt;

    if ( _sth != NULL && _sth->getCurrentMode() == SearchThread::SEARCHING ) {
        statetxt = _sth->getCurrentActionInfo();
    } else if ( _sth != NULL && _sth->getCurrentMode() == SearchThread::SUSPENDED ) {
        statetxt = catalog.getLocale( 745 );
    } else {
        statetxt = catalog.getLocale( 740 );
    }
    state_text.setText( statetxt.c_str() );
}

void SearchOpBG::prepareButton( Button &startb, buttonmode_t mode )
{
    std::string newtext;
    if ( mode == BUTTON_CONTINUE ) {
        newtext = catalog.getLocale( 743 );
    } else if ( mode == BUTTON_SUSPEND ) {
        newtext = catalog.getLocale( 744 );
    } else {
        newtext = catalog.getLocale( 741 );
    }

    if ( newtext != startb.getText( 0 ) )
        startb.setText( 0, newtext.c_str() );
}

void SearchOpBG::prepareStartCont( bool force )
{
    if ( _rst != NULL ) {
        bool result_available = ( _rst->getDir()->empty() == true ) ? false : true;
        if ( result_available != _last_startcont_state || force == true ) {
            if ( result_available == true ) {
                _startdir_sg->hide();
                _startdir_text->show();

                int minw = _start_ac->getMinWidth( 1, 0 );
                _start_ac->add( _startdir_text, 1, 0, AContainer::CO_INCW );
                _start_ac->setMinWidth( minw, 1, 0 );
        
                _start_ac->rearrange();
            } else {
                _startdir_sg->show();
                _startdir_text->hide();

                int minw = _start_ac->getMinWidth( 1, 0 );
                _start_ac->add( _startdir_sg, 1, 0, AContainer::CO_INCW );
                _start_ac->setMinWidth( minw, 1, 0 );
        
                _start_ac->rearrange();
            }
            _last_startcont_state = result_available;
        }
    }
}


bool SearchOpBG::consumeResult( SearchThread::SearchResult &result, FieldListView &lv )
{
    bool new_result = false;

    std::string fullname = result.getFullname();

    int row = lv.addRow();
  
    if ( result.getLineNr() >= 0 ) {
        char linestr[ A_BYTESFORNUMBER( int ) ];
        sprintf( linestr, "%d", result.getLineNr() + 1);
    
        lv.setText( row, 0, linestr );
    } else {
        lv.setText( row, 0, "" );
    }

    if ( result.getIsDir() == true && fullname != "/" )
        fullname += "/";
  
    lv.setText( row, 2, fullname );

    if ( result.getIsDir() == true ) {
        lv.setColor( row, dircol );
    } else {
        lv.setColor( row, filecol );
    }
  
    if ( _last_result != fullname ) {
        new_result = true;
        _last_result = fullname;
    }

    if ( row == 0 ) {
        lv.setActiveRow( row );
        storeActiveRow( row );
    }

    return new_result;
}

void SearchOpBG::enterResult( const std::string &fullname )
{
    Lister *l1;
    ListerMode *lm1;

    l1 = m_worker->getActiveLister();
    if ( l1 != NULL ) {
        if ( l1->getActiveMode() == NULL ||
             ! ( dynamic_cast< VirtualDirMode *>( l1->getActiveMode() ) ) ) {
            l1->switch2Mode( 0 );
        }

        lm1 = l1->getActiveMode();
        if ( lm1 != NULL ) {
            std::list< RefCount< ArgClass > > args;

            args.push_back( RefCount< ArgClass >( new StringArg( fullname ) ) );
            lm1->runCommand( "enter_dir", args );
        }
    }
}

void SearchOpBG::clearStoredResults()
{
    stopSearch();

    if ( _rst != NULL ) {
        _rst->unset_exclusive( this );
    }

    clear_stored_results();
  
    _rst = NULL;
  
    if ( _storelv != NULL )
        buildStoreLV();

    updateWindowTitle();
}

void SearchOpBG::buildLV()
{
    std::list<SearchThread::SearchResult> *res = _rst->getRes();
    SearchSettings sets = _rst->getSearchSettings();
    int active_row = sets.getActiveRow();

    _resultlv->setSize( 0 );
    for ( std::list<SearchThread::SearchResult>::iterator it = res->begin();
          it != res->end();
          it++ ) {
        consumeResult( *it, *_resultlv );
    }
    _resultlv->setActiveRow( active_row );
    storeActiveRow( active_row );
    _resultlv->redraw();
  
    prepareStartCont();
}

void SearchOpBG::prepareNewSearch()
{
    _last_result = "";
    _resultlv->setSize( 0 );
    _resultlv->redraw();

    prepareStartCont();
}

void SearchOpBG::buildStoreLV()
{
    if ( _storelv != NULL ) {
        _storelv->setSize( 0 );
        int row = -1;
        for ( std::list<ResultStore*>::iterator it = _stored_results.begin();
              it != _stored_results.end();
              it++ ) {
            row = _storelv->addRow();
            _storelv->setColor( row, resultcol );

            SearchSettings sets = (*it)->getSearchSettings();
            std::string resstr;
            char buf[A_BYTESFORNUMBER(int)];

            sprintf( buf, "%d", (int)( (*it)->getRes()->size() ) );
            _storelv->setText( row, 1, buf );
            _storelv->setText( row, 2, sets.getMatchName() );
            _storelv->setText( row, 3, sets.getMatchContent() );
            _storelv->setText( row, 4, sets.getBaseDir() );

            if ( (*it)->is_exclusive() && *it != _rst ) {
                _storelv->setText( row, 0, "L" );
            }
        }
        _storelv->showRow( row );
        _storelv->redraw();
    }
}

void SearchOpBG::setResultStore( int pos, const SearchSettings *sets )
{
    ResultStore *erg = NULL;

    // try to find position
    if ( pos >= 0 ) {
        for ( std::list<ResultStore*>::iterator it = _stored_results.begin();
              it != _stored_results.end();
              it++, pos-- ) {
            if ( pos == 0 ) {
                erg = *it;
                break;
            } 
        }
    }

    if ( erg != NULL &&
         erg != _rst ) {
        if ( erg->is_exclusive() ) {
            Requester req( Worker::getAGUIX(), m_win );
            req.request( catalog.getLocale( 124 ), catalog.getLocale( 962 ), catalog.getLocale( 11 ) );
            erg = NULL;
        }
    }

    if ( erg == NULL ) {
        // nothing found so create a new one with settings from
        // the current or latest entry

        std::string name = AGUIXUtils::formatStringToString( "search%d", s_search_number++ );

        if ( s_search_number < 0 ) {
            // avoid negative and zero search number
            s_search_number = 1;
        }

        erg = new ResultStore( name );

        std::list<ResultStore*>::iterator it;
        for ( it = _stored_results.begin();
              it != _stored_results.end();
              it++ ) {
            if ( *it == _rst ) {
                break;
            }
        }

        if ( sets != NULL ) {
            erg->setSearchSettings( *sets );
        } else {
            if ( it != _stored_results.end() ) {
                erg->setSearchSettings( (*it)->getSearchSettings() );
            } else {
                if ( _stored_results.size() > 0 ) {
                    erg->setSearchSettings( _stored_results.back()->getSearchSettings() );
                } else {
                    erg->setSearchSettings( getCurrentSearchSettings() );
                }
            }
        }
    
        _stored_results.push_back( erg );

        _stored_results_changed++;

        buildStoreLV();
    }

    if ( _rst != NULL ) {
        _rst->unset_exclusive( this );
    }

    _rst = erg;
    _rst->set_exclusive( this );

    updateWindowTitle();
}

class DW_SetFollowSymlinks : public NWC::DirWalkCallBack
{
public:
    DW_SetFollowSymlinks( bool follow_symlinks, bool search_same_fs ) : _follow_symlinks( follow_symlinks ),
                                                                        _search_same_fs( search_same_fs ) {}

    int visit( NWC::File &file, NWC::Dir::WalkControlObj &cobj ) { return 0; }
    int visitEnterDir( NWC::Dir &dir, NWC::Dir::WalkControlObj &cobj )
    {
        dir.setFollowSymlinks( _follow_symlinks );

        /* IRIX fix: use pointer cast to avoid std::bad_cast exception.
           The DWARF unwinder crashes on IRIX libpthread frames. */
        NWC::VirtualDir *vdir = dynamic_cast<NWC::VirtualDir*>( &dir );
        if ( vdir != NULL ) {
            vdir->setSearchSameFS( _search_same_fs );
        }
        return 0;
    }
    int visitLeaveDir( NWC::Dir &dir, NWC::Dir::WalkControlObj &cobj ) { return 0; }
    int prepareDirWalk( const NWC::Dir &dir, NWC::Dir::WalkControlObj &cobj,
                        std::list< RefCount< NWC::FSEntry > > &return_add_entries,
                        std::list< RefCount< NWC::FSEntry > > &return_skip_entries ) { return 0; }
private:
    bool _follow_symlinks;
    bool _search_same_fs;
};

void SearchOpBG::startSearch()
{
    stopSearch();

    if ( ! m_timeout_set ) {
        m_worker->registerTimeout( searchopbg_timeout );
        m_timeout_set = true;
    }

    SearchSettings sets = getCurrentSearchSettings();;

    std::unique_ptr<NWC::VirtualDir> use_dir( _rst->getClonedDir() );
  
    if ( use_dir->size() < 1 ) {
        std::string d = sets.getBaseDir();
        if ( d == "." || d.length() < 1 || d[0] != '/' ) {
            if ( m_initial_start_vdir.get() ) {
                use_dir->add( *m_initial_start_vdir );
            } else {
                use_dir->add( NWC::VirtualDir( m_initial_start_dir ) );
            }
        } else {
            use_dir->add( NWC::VirtualDir( d ) );
        }
    } else {
        setCurrentResultStore( -1, &sets );
    }
    prepareNewSearch();
    _resultlv->takeFocus();
  
    use_dir->setFollowSymlinks( sets.getFollowSymlinks() );
    use_dir->setSearchSameFS( sets.getSearchSameFS() );
  
    DW_SetFollowSymlinks dw1( sets.getFollowSymlinks(), sets.getSearchSameFS() );
    use_dir->walk( dw1 );

    _sth = new SearchThread( std::unique_ptr<NWC::Dir>( std::move( use_dir ) ) );
    _sth->setSearchSettings( sets );
    _sth->start();

    storeSearchSettings();
    buildStoreLV();
  
    _search_started = true;

    updateWindowTitle();
}

void SearchOpBG::stopSearch()
{
    // stop thread, gather remaining results into rst and delete thread
    if ( _sth != NULL ) {
        _sth->setOrder( SearchThread::STOP );
        _sth->join();
        gatherResults();
        delete _sth;
        _sth = NULL;
    }
}

void SearchOpBG::gatherResults( int get_number_of_results, int time_limit_ms )
{
    // get results from thread and put them into rst
    //TODO wait for a couple of results or idling search thread
    //     to reduce frequency of update
    if ( _sth != NULL && _sth->resultsAvailable() ) {
        _sth->lockResults();

        struct timeval start_time, end_time;
#ifdef DEBUG
        int results = 0;
#endif

        gettimeofday( &start_time, NULL );

        while ( _sth->resultsAvailable() ) {
            SearchThread::SearchResult erg = _sth->getResultLocked();
            _rst->getRes()->push_back( erg );
            if ( consumeResult( erg, *_resultlv ) == true ) {
                _rst->getDir()->add( NWC::FSEntry( erg.getFullname() ) );
            }

#ifdef DEBUG
            results++;
#endif

            if ( get_number_of_results > 0 ) {
                get_number_of_results--;
                if ( get_number_of_results == 0 )
                    break;
            }

            if ( time_limit_ms > 0 ) {
                gettimeofday( &end_time, NULL );

                if ( ldiffgtod_m( &end_time, &start_time ) >= time_limit_ms ) {
#ifdef DEBUG
                    printf("got %d results before %d\n", results, time_limit_ms );
#endif
                    break;
                }
            }
        }
        _sth->unlockResults();
        _resultlv->redraw();
    
#ifdef DEBUG
        printf("got %d results\n", results );
#endif

        prepareStartCont();
    }
}

void SearchOpBG::newSearch()
{
    stopSearch();

    if ( m_initial_start_vdir.get() ) {
        _startdir_sg->setText( m_initial_start_vdir->getFullname().c_str() );
    } else {
        _startdir_sg->setText( m_initial_start_dir.c_str() );
    }
    SearchSettings sets = getCurrentSearchSettings();

    if ( _rst == NULL ||_rst->getDir()->size() > 0 ) {
        // first try last result in store whether it's empty or not

        bool last_entry_blocked_or_nonempty = false;

        if ( ! _stored_results.empty() ) {
            std::list<ResultStore*>::iterator it1 = _stored_results.end();

            it1--;

            if ( (*it1)->is_exclusive() && *it1 != _rst ) {
                last_entry_blocked_or_nonempty = true;
            } else if ( (*it1)->getDir()->size() > 0 ) {
                last_entry_blocked_or_nonempty = true;
            }
        }

        if ( ! last_entry_blocked_or_nonempty ) {
            setCurrentResultStore( _stored_results.size() - 1 );
            _rst->setSearchSettings( sets );
        } else {
            setCurrentResultStore( -1, &sets );
        }

        restoreSearchSettings();
    }

    prepareNewSearch();
    m_options_k1->optionChange( 0 );
    _matchname_sg->takeFocus();
}

SearchSettings SearchOpBG::getCurrentSearchSettings()
{
    SearchSettings sets( _startdir_sg->getText(),
                         _matchname_sg->getText(),
                         _matchname_regexp_cb->getState(),
                         _matchname_fullpath_cb->getState(),
                         _matchname_case_cb->getState(),
                         _matchcontent_sg->getText(),
                         _matchcontent_case_cb->getState(),
                         _follow_symlinks_cb->getState(),
                         _search_vfs_cb->getState(),
                         _search_same_fs_cb->getState(),
                         m_younger_cb->getState(),
                         m_younger_sg->getText(),
                         m_older_cb->getState(),
                         m_older_sg->getText() );

    sets.setCheckSizeGEQ( m_size_geq_cb->getState() );
    sets.setCheckSizeLEQ( m_size_leq_cb->getState() );
    sets.setSizeGEQ( m_size_geq_sg->getText() );
    sets.setSizeLEQ( m_size_leq_sg->getText() );
    sets.setMaxVFSDepth( atoi( m_max_vfs_depth_sg->getText() ) );

    sets.setMatchAlsoDirectories( m_also_dirs_cb->getState() );

    return sets;
}

void SearchOpBG::storeSearchSettings()
{
    _rst->setSearchSettings( getCurrentSearchSettings() );
}

void SearchOpBG::restoreSearchSettings()
{
    if ( _rst == NULL ) return;
  
    SearchSettings sets = _rst->getSearchSettings();
    if ( sets.getBaseDir().length() < 1 ) {
        if ( m_initial_start_vdir.get() ) {
            _startdir_sg->setText( m_initial_start_vdir->getFullname().c_str() );
        } else {
            _startdir_sg->setText( m_initial_start_dir.c_str() );
        }
    } else {
        _startdir_sg->setText( sets.getBaseDir().c_str() );
    }
    _matchname_sg->setText( sets.getMatchName().c_str() );
    _matchname_regexp_cb->setState( sets.getMatchNameRE() );
    _matchname_fullpath_cb->setState( sets.getMatchNameFullPath() );
    _matchname_case_cb->setState( sets.getMatchNameCase() );
    _matchcontent_sg->setText( sets.getMatchContent().c_str() );
    _matchcontent_case_cb->setState( sets.getMatchContentCase() );
    _follow_symlinks_cb->setState( sets.getFollowSymlinks() );
    _search_vfs_cb->setState( sets.getSearchVFS() );
    _search_same_fs_cb->setState( sets.getSearchSameFS() );
    m_younger_cb->setState( sets.getCheckYounger() );
    m_younger_sg->setText( sets.getYoungerThan().c_str() );
    m_older_cb->setState( sets.getCheckOlder() );
    m_older_sg->setText( sets.getOlderThan().c_str() );

    m_size_geq_cb->setState( sets.getCheckSizeGEQ() );
    m_size_leq_cb->setState( sets.getCheckSizeLEQ() );
    m_size_geq_sg->setText( sets.getSizeGEQ().c_str() );
    m_size_leq_sg->setText( sets.getSizeLEQ().c_str() );

    std::string s1 = AGUIXUtils::formatStringToString( "%d", sets.getMaxVFSDepth() );
    m_max_vfs_depth_sg->setText( s1.c_str() );

    m_also_dirs_cb->setState( sets.getMatchAlsoDirectories() );
}

void SearchOpBG::setCurrentResultStore( int pos, const SearchSettings *sets )
{
    setResultStore( pos, sets );
    if ( pos >= 0 ) {
        restoreSearchSettings();
    }
}

void SearchOpBG::storeActiveRow( int row )
{
    if ( _rst != NULL ) {
        _rst->setActiveRow( row );
    }
}

void SearchOpBG::removeSelectedResults()
{
    // first get all selected unique entries
    // for each entry search all results which uses the
    // same entry (for different lines)
    // then remove it from searchdir

    std::unique_ptr< std::list< std::pair< std::string, int > > > selected_entries = getSelectedResults();
  
    _rst->removeEntries( *( selected_entries.get() ) );
    buildLV();
    _resultlv->centerActive();
}

std::unique_ptr< std::list< std::pair< std::string, int > > > SearchOpBG::getSelectedResults()
{
    std::list<SearchThread::SearchResult> *res = _rst->getRes();
    std::list<SearchThread::SearchResult>::iterator active_entry = res->end();

    std::unique_ptr< std::list< std::pair< std::string, int > > > selected_entries( new std::list< std::pair< std::string, int > > );

    int lvpos = 0;
    for ( std::list<SearchThread::SearchResult>::iterator it = res->begin();
          it != res->end();
          it++, lvpos++ ) {
        if ( _resultlv->getSelect( lvpos ) == true ) {
            if ( selected_entries->empty() == true ) {
                selected_entries->push_back( std::make_pair( it->getFullname(),
                                                             it->getLineNr() ) );
            } else {
                if ( selected_entries->back() != std::make_pair( it->getFullname(),
                                                                 it->getLineNr() ) ) {
                    selected_entries->push_back( std::make_pair( it->getFullname(),
                                                                 it->getLineNr() ) );
                }
            }
        } else if ( lvpos == _resultlv->getActiveRow() ) {
            active_entry = it;
        }
    }

    if ( selected_entries->empty() == true && active_entry != res->end() ) {
        selected_entries->push_back( std::make_pair( active_entry->getFullname(),
                                                     active_entry->getLineNr() ) );
    }

    return selected_entries;
}

SearchThread::SearchResult SearchOpBG::getResultForRow( int row )
{
    std::list<SearchThread::SearchResult> *res = _rst->getRes();
    SearchThread::SearchResult r( "", -1 );

    int lvpos = 0;
    for ( std::list<SearchThread::SearchResult>::iterator it = res->begin();
          it != res->end();
          it++, lvpos++ ) {
        if ( lvpos == row ) {
            r = *it;
            break;
        }
    }

    return r;
}

void SearchOpBG::viewResult()
{
    int row = _resultlv->getActiveRow();
    if ( _resultlv->isValidRow( row ) ) {
        SearchThread::SearchResult r = getResultForRow( row );
        if ( r.getFullname().length() > 0 ) {
            std::list<std::string> l;
            //TODO check whether file system exceeds basic textstorage size and ask for increase
            l.push_back( r.getFullname() );

            FileViewer fv( m_worker );
            fv.setShowLineNumbers( true );
            fv.view( l, NULL, r.getLineNr(), true );
        }
    }
}

void SearchOpBG::editResult()
{
    int row = _resultlv->getActiveRow();
    if ( _resultlv->isValidRow( row ) ) {
        SearchThread::SearchResult r = getResultForRow( row );
        if ( r.getFullname().length() > 0 ) {
            editFile( r.getFullname(), r.getLineNr() );
        }
    }
}

void SearchOpBG::editFile( const std::string &filename, int line_number )
{
    auto fl = std::make_shared< FlagReplacer::FlagProducerMap >();
    NWC::FSEntry fse( filename );
    char linestr[ A_BYTESFORNUMBER( int ) ];

    sprintf( linestr, "%d", line_number + 1 );

    fl->registerFlag( "f", fse.getBasename() );
    fl->registerFlag( "F", filename );
    fl->registerFlag( "l", linestr );

    std::string scriptdir = Worker::getDataDir();
    scriptdir += "/scripts";
    fl->registerFlag( "scripts", scriptdir );
  
    FlagReplacer rpl( fl );
  
    std::string exestr = rpl.replaceFlags( m_edit_command );

    std::unique_ptr<ExeClass> ec( new ExeClass() );

    auto dirname = fse.getDirname();
    if ( dirname.empty() ) {
        ec->cdDir( "." );
    } else {
        ec->cdDir( dirname.c_str() );
    }

    ec->addCommand( "%s", exestr.c_str() );
    ec->execute();
}

void SearchOpBG::initColorDefs()
{
    AGUIX *aguix = Worker::getAGUIX();

    dircol.setFG( FieldListView::CC_NORMAL, aguix->getFaces().getColor( "dirview-dir-normal-fg" ) );
    dircol.setBG( FieldListView::CC_NORMAL, aguix->getFaces().getColor( "dirview-dir-normal-bg" ) );
    dircol.setFG( FieldListView::CC_SELECT, aguix->getFaces().getColor( "dirview-dir-select-fg" ) );
    dircol.setBG( FieldListView::CC_SELECT, aguix->getFaces().getColor( "dirview-dir-select-bg" ) );
    dircol.setFG( FieldListView::CC_ACTIVE, aguix->getFaces().getColor( "dirview-dir-active-fg" ) );
    dircol.setBG( FieldListView::CC_ACTIVE, aguix->getFaces().getColor( "dirview-dir-active-bg" ) );
    dircol.setFG( FieldListView::CC_SELACT, aguix->getFaces().getColor( "dirview-dir-selact-fg" ) );
    dircol.setBG( FieldListView::CC_SELACT, aguix->getFaces().getColor( "dirview-dir-selact-bg" ) );
  
    filecol.setFG( FieldListView::CC_NORMAL, aguix->getFaces().getColor( "dirview-file-normal-fg" ) );
    filecol.setBG( FieldListView::CC_NORMAL, aguix->getFaces().getColor( "dirview-file-normal-bg" ) );
    filecol.setFG( FieldListView::CC_SELECT, aguix->getFaces().getColor( "dirview-file-select-fg" ) );
    filecol.setBG( FieldListView::CC_SELECT, aguix->getFaces().getColor( "dirview-file-select-bg" ) );
    filecol.setFG( FieldListView::CC_ACTIVE, aguix->getFaces().getColor( "dirview-file-active-fg" ) );
    filecol.setBG( FieldListView::CC_ACTIVE, aguix->getFaces().getColor( "dirview-file-active-bg" ) );
    filecol.setFG( FieldListView::CC_SELACT, aguix->getFaces().getColor( "dirview-file-selact-fg" ) );
    filecol.setBG( FieldListView::CC_SELACT, aguix->getFaces().getColor( "dirview-file-selact-bg" ) );

    resultcol.setFG( FieldListView::CC_NORMAL, aguix->getFaces().getColor( "dirview-file-normal-fg" ) );
    resultcol.setBG( FieldListView::CC_NORMAL, aguix->getFaces().getColor( "dirview-file-normal-bg" ) );
    resultcol.setFG( FieldListView::CC_SELECT, aguix->getFaces().getColor( "dirview-file-normal-fg" ) );
    resultcol.setBG( FieldListView::CC_SELECT, aguix->getFaces().getColor( "dirview-file-normal-bg" ) );
    resultcol.setFG( FieldListView::CC_ACTIVE, aguix->getFaces().getColor( "dirview-file-active-fg" ) );
    resultcol.setBG( FieldListView::CC_ACTIVE, aguix->getFaces().getColor( "dirview-file-active-bg" ) );
    resultcol.setFG( FieldListView::CC_SELACT, aguix->getFaces().getColor( "dirview-file-active-fg" ) );
    resultcol.setBG( FieldListView::CC_SELACT, aguix->getFaces().getColor( "dirview-file-active-bg" ) );
}

SearchOpBG::SearchOpBG( Worker *worker ) :
    m_worker( worker),
    m_show_prev_results( false ),
    m_edit_command( DEFAULT_EDIT_COMMAND ),
    _start_ac( NULL ),
    _startdir_text( NULL ),
    _startdir_sg( NULL ),
    _storelv( NULL ),
    _last_startcont_state( false ),
    _last_result( "" ),
    _rst( NULL ),
    _resultlv( NULL ),
    _sth( NULL ),
    _matchname_sg( NULL ),
    _matchname_regexp_cb( NULL ),
    _matchname_fullpath_cb( NULL ),
    _matchname_case_cb( NULL ),
    _matchcontent_sg( NULL ),
    _matchcontent_case_cb( NULL ),
    _follow_symlinks_cb( NULL ),
    _search_vfs_cb( NULL ),
    _search_same_fs_cb( NULL ),
    m_younger_cb( NULL ),
    m_older_cb( NULL ),
    m_younger_sg( NULL ),
    m_older_sg( NULL ),
    m_size_geq_cb( NULL ),
    m_size_leq_cb( NULL ),
    m_size_geq_sg( NULL ),
    m_size_leq_sg( NULL ),
    m_options_k1( NULL ),
    m_max_vfs_depth_sg( NULL ),
    m_also_dirs_cb( NULL ),
    _search_started( false ),
    m_update_ctr( 0 ),
    m_stored_result_last_changed( 0 ),
    m_timeout_set( false )
{
    m_timeout_cb.reset( new SearchOpBGTimeoutCB( this ) );
}

SearchOpBG::~SearchOpBG()
{
    stopSearch();

    if ( _rst != NULL ) {
        _rst->unset_exclusive( this );
    }

    m_worker->getEventHandler().unregisterCallback( m_timeout_cb );

    if ( m_timeout_set ) {
        m_worker->unregisterTimeout( searchopbg_timeout );
    }

    m_timeout_cb->reset();
    m_timeout_cb = NULL;

    delete m_win;
}

void SearchOpBG::createWindow()
{
    AGUIX *aguix = Worker::getAGUIX();
    std::string str1;
    int w, h, sw, sh;
    initColorDefs();

    m_win = new AWindow( aguix, 10, 10, 10, 10, catalog.getLocale( 1291 ) );
    m_win->create();

    AContainer *ac1 = m_win->setContainer( new AContainer( m_win, 1, 5 ), true );
    ac1->setMinSpace( 5 );
    ac1->setMaxSpace( 5 );

    AContainerBB *ac1_1 = dynamic_cast<AContainerBB*>( ac1->add( new AContainerBB( m_win, 1, 1 ), 0, 0 ) );
    ac1_1->setMinSpace( 5 );
    ac1_1->setMaxSpace( 5 );
    ac1_1->setBorderWidth( 5 );

    m_options_k1 = new Kartei( aguix, 10, 10, 10, 10, "" );
    ac1_1->add( m_options_k1, 0, 0, AContainer::CO_INCW );
    m_options_k1->create();

    AWindow *subwin1 = new AWindow( aguix, 0, 0, 10, 10, "" );
    m_options_k1->add( subwin1 );
    subwin1->create();

    AContainer *acsw1_1 = subwin1->setContainer( new AContainer( subwin1, 1, 3 ), true );
    acsw1_1->setMinSpace( 5 );
    acsw1_1->setMaxSpace( 5 );
    acsw1_1->setBorderWidth( 5 );

    _start_ac = acsw1_1->add( new AContainer( subwin1, 2, 3 ), 0, 0 );
    _start_ac->setMinSpace( 5 );
    _start_ac->setMaxSpace( 5 );
    _start_ac->setBorderWidth( 0 );

    _start_ac->add( new Text( aguix, 0, 0, catalog.getLocale( 722 ) ), 0, 0, AContainer::CO_FIX );
    _startdir_text = (Text*)_start_ac->add( new Text( aguix, 0, 0, catalog.getLocale( 723 ) ), 1, 0, AContainer::CO_INCW );
    _startdir_text->hide();
    _startdir_sg = (StringGadget*)_start_ac->add( new StringGadget( aguix, 0, 0, 100,
                                                                    m_initial_start_vdir.get() != NULL ?
                                                                    m_initial_start_vdir->getFullname().c_str() :
                                                                    m_initial_start_dir.c_str(),
                                                                    0 ), 1, 0, AContainer::CO_INCW );
    _startdir_text->resize( _startdir_text->getWidth(), _startdir_sg->getHeight() );

    AContainer *ac1_2 = _start_ac->add( new AContainer( subwin1, 4, 1 ), 1, 1 );
    ac1_2->setMinSpace( 0 );
    ac1_2->setMaxSpace( 0 );
    ac1_2->setBorderWidth( 0 );

    _start_ac->add( new Text( aguix, 0, 0, catalog.getLocale( 724 ) ), 0, 1, AContainer::CO_FIX );
    _matchname_sg = (StringGadget*)ac1_2->add( new StringGadget( aguix, 0, 0, 100, "*", 0 ), 0, 0, AContainer::CO_INCW );
    _matchname_regexp_cb = (ChooseButton*)ac1_2->add( new ChooseButton( aguix, 0, 0, false,
                                                                        catalog.getLocale( 502 ), LABEL_RIGHT, 0 ), 1, 0, AContainer::CO_FIXNR );
    _matchname_fullpath_cb = (ChooseButton*)ac1_2->add( new ChooseButton( aguix, 0, 0, false,
                                                                          catalog.getLocale( 570 ), LABEL_RIGHT, 0 ), 2, 0, AContainer::CO_FIXNR );
    _matchname_case_cb = (ChooseButton*)ac1_2->add( new ChooseButton( aguix, 0, 0, false,
                                                                      catalog.getLocale( 725 ), LABEL_RIGHT, 0 ), 3, 0, AContainer::CO_FIXNR );
    _matchname_regexp_cb->resize( _matchname_sg->getHeight(), _matchname_sg->getHeight() );
    _matchname_fullpath_cb->resize( _matchname_sg->getHeight(), _matchname_sg->getHeight() );
    _matchname_case_cb->resize( _matchname_sg->getHeight(), _matchname_sg->getHeight() );
    ac1_2->readLimits();
  
    AContainer *ac1_3 = _start_ac->add( new AContainer( subwin1, 2, 1 ), 1, 2 );
    ac1_3->setMinSpace( 0 );
    ac1_3->setMaxSpace( 0 );
    ac1_3->setBorderWidth( 0 );

    _start_ac->add( new Text( aguix, 0, 0, catalog.getLocale( 726 ) ), 0, 2, AContainer::CO_FIX );
    _matchcontent_sg = (StringGadget*)ac1_3->add( new StringGadget( aguix, 0, 0, 100, "", 0 ), 0, 0, AContainer::CO_INCW );
    _matchcontent_case_cb = (ChooseButton*)ac1_3->add( new ChooseButton( aguix, 0, 0, false,
                                                                         catalog.getLocale( 725 ), LABEL_RIGHT, 0 ), 1, 0, AContainer::CO_FIXNR );
    _matchcontent_case_cb->resize( _matchcontent_sg->getHeight(), _matchcontent_sg->getHeight() );
    ac1_3->readLimits();

    AContainer *ac1_globsets = acsw1_1->add( new AContainer( subwin1, 5, 1 ), 0, 1 );
    ac1_globsets->setMinSpace( 5 );
    ac1_globsets->setMaxSpace( 5 );
    ac1_globsets->setBorderWidth( 0 );

    _follow_symlinks_cb = (ChooseButton*)ac1_globsets->add( new ChooseButton( aguix, 0, 0, false,
                                                                              catalog.getLocale( 298 ), LABEL_RIGHT, 0 ), 0, 0, AContainer::CO_FIXNR );
    _search_vfs_cb = (ChooseButton*)ac1_globsets->add( new ChooseButton( aguix, 0, 0, false,
                                                                         catalog.getLocale( 727 ), LABEL_RIGHT, 0 ), 1, 0, AContainer::CO_FIXNR );
    _search_same_fs_cb = (ChooseButton*)ac1_globsets->add( new ChooseButton( aguix, 0, 0, false,
                                                                             catalog.getLocale( 792 ), LABEL_RIGHT, 0 ), 2, 0, AContainer::CO_FIXNR );
    m_newsearchb = (Button*)ac1_globsets->add( new Button( aguix,
                                                           0,
                                                           0,
                                                           catalog.getLocale( 728 ),
                                                           0 ), 4, 0, AContainer::CO_FIX );

    AContainer *ac1_set2 = acsw1_1->add( new AContainer( subwin1, 2, 1 ), 0, 2 );
    ac1_set2->setMinSpace( 5 );
    ac1_set2->setMaxSpace( 5 );
    ac1_set2->setBorderWidth( 0 );

    m_also_dirs_cb = (ChooseButton*)ac1_set2->add( new ChooseButton( aguix, 0, 0, false,
                                                                     catalog.getLocale( 1003 ), LABEL_RIGHT, 0 ),
                                                   0, 0, AContainer::CO_FIXNR );

    AWindow *subwin2 = new AWindow( aguix, 0, 0, 10, 10, "" );
    m_options_k1->add( subwin2 );
    subwin2->create();

    AContainer *acsw2_1 = subwin2->setContainer( new AContainer( subwin2, 1, 3 ), true );
    acsw2_1->setMinSpace( 5 );
    acsw2_1->setMaxSpace( 5 );
    acsw2_1->setBorderWidth( 5 );

    AContainer *acsub1 = acsw2_1->add( new AContainer( subwin2, 6, 1 ), 0, 0 );
    acsub1->setMinSpace( 5 );
    acsub1->setMaxSpace( 5 );
    acsub1->setBorderWidth( 0 );

    m_younger_cb = (ChooseButton*)acsub1->add( new ChooseButton( aguix, 0, 0, false,
                                                                 catalog.getLocale( 813 ), LABEL_RIGHT, 0 ), 0, 0, AContainer::CO_FIXNR );
    m_younger_sg = (StringGadget*)acsub1->add( new StringGadget( aguix, 0, 0, 100, "", 0 ), 1, 0, AContainer::CO_INCW );
    m_younger_cal_b = (Button*)acsub1->add( new Button( aguix, 0, 0, catalog.getLocale( 819 ), 0 ), 2, 0, AContainer::CO_FIX );
    m_older_cb = (ChooseButton*)acsub1->add( new ChooseButton( aguix, 0, 0, false,
                                                               catalog.getLocale( 814 ), LABEL_RIGHT, 0 ), 3, 0, AContainer::CO_FIXNR );
    m_older_sg = (StringGadget*)acsub1->add( new StringGadget( aguix, 0, 0, 100, "", 0 ), 4, 0, AContainer::CO_INCW );
    m_older_cal_b = (Button*)acsub1->add( new Button( aguix, 0, 0, catalog.getLocale( 819 ), 0 ), 5, 0, AContainer::CO_FIX );

    AContainer *acsub2 = acsw2_1->add( new AContainer( subwin2, 5, 1 ), 0, 1 );
    acsub2->setMinSpace( 5 );
    acsub2->setMaxSpace( 5 );
    acsub2->setBorderWidth( 0 );

    m_size_geq_cb = (ChooseButton*)acsub2->add( new ChooseButton( aguix, 0, 0, false,
                                                                  catalog.getLocale( 817 ), LABEL_RIGHT, 0 ), 0, 0, AContainer::CO_FIXNR );
    m_size_geq_sg = (StringGadget*)acsub2->add( new StringGadget( aguix, 0, 0, 100, "", 0 ), 1, 0, AContainer::CO_INCW );
    m_size_leq_cb = (ChooseButton*)acsub2->add( new ChooseButton( aguix, 0, 0, false,
                                                                  catalog.getLocale( 818 ), LABEL_RIGHT, 0 ), 2, 0, AContainer::CO_FIXNR );
    m_size_leq_sg = (StringGadget*)acsub2->add( new StringGadget( aguix, 0, 0, 100, "", 0 ), 3, 0, AContainer::CO_INCW );
    m_size_help_b = (Button*)acsub2->add( new Button( aguix, 0, 0, catalog.getLocale( 483 ), 0 ), 4, 0, AContainer::CO_FIX );

    AContainer *acsub3 = acsw2_1->add( new AContainer( subwin2, 3, 1 ), 0, 2 );
    acsub3->setMinSpace( 5 );
    acsub3->setMaxSpace( 5 );
    acsub3->setBorderWidth( 0 );

    acsub3->add( new Text( aguix, 0, 0, catalog.getLocale( 940 ) ), 0, 0, AContainer::CO_FIX );
    m_max_vfs_depth_sg = (StringGadget*)acsub3->add( new StringGadget( aguix, 0, 0, aguix->getTextWidth( "MMMMMMMMM" ), "3", 0 ),
                                                     1, 0, AContainer::CO_FIX );

    m_options_k1->setOption( subwin1, 0, catalog.getLocale( 815 ) );
    m_options_k1->setOption( subwin2, 1, catalog.getLocale( 816 ) );
    m_options_k1->maximize();
    m_options_k1->contMaximize();
    ac1_1->readLimits();
  
    AContainer *ac1_res = ac1->add( new AContainer( m_win, 1, 3 ), 0, 1 );
    ac1_res->setMinSpace( 5 );
    ac1_res->setMaxSpace( 5 );
    ac1_res->setBorderWidth( 0 );
    ac1->setHWeight( 2, 0, 1 );

    _resultlv = (FieldListView*)ac1_res->add( new FieldListView( aguix, 0, 0, 100, 100, 0 ), 0, 0, AContainer::CO_MIN );
    _resultlv->setNrOfFields( 3 );
    _resultlv->setFieldWidth( 1, 5 );
    _resultlv->setFieldText( 0, catalog.getLocale( 718 ) );
    _resultlv->setFieldText( 2, catalog.getLocale( 729 ) );
    _resultlv->setFieldTextMerged( 0, true );
    _resultlv->setHBarState( 2 );
    _resultlv->setVBarState( 2 );
    _resultlv->setConsiderHeaderWidthForDynamicWidth( true );
    _resultlv->setShowHeader( true );
    _resultlv->setAcceptFocus( true );
    _resultlv->setDisplayFocus( true );
    _resultlv->setMBG( aguix->getFaces().getColor( "dirview-bg" ) );

    AContainer *ac1_4 = ac1_res->add( new AContainer( m_win, 2, 1 ), 0, 1 );
    ac1_4->setMinSpace( 0 );
    ac1_4->setMaxSpace( 0 );
    ac1_4->setBorderWidth( 0 );

    m_enterb =(Button*)ac1_4->add( new Button( aguix, 0, 0,
                                               catalog.getLocale( 730 ), catalog.getLocale( 967 ),
                                               0 ), 0, 0, AContainer::CO_INCW );
    m_panelize_b =(Button*)ac1_4->add( new Button( aguix, 0, 0,
                                                   catalog.getLocale( 995 ),
                                                   catalog.getLocale( 996 ),
                                                   0 ), 1, 0, AContainer::CO_INCW );

    AContainer *ac1_4_1 = ac1_res->add( new AContainer( m_win, 3, 1 ), 0, 2 );
    ac1_4_1->setMinSpace( 0 );
    ac1_4_1->setMaxSpace( 0 );
    ac1_4_1->setBorderWidth( 0 );

    m_viewb = (Button*)ac1_4_1->add( new Button( aguix, 0, 0, catalog.getLocale( 731 ), 0 ), 0, 0, AContainer::CO_INCW );
    m_editb = (Button*)ac1_4_1->add( new Button( aguix, 0, 0, catalog.getLocale( 732 ), 0 ), 1, 0, AContainer::CO_INCW );
    m_removeb = (Button*)ac1_4_1->add( new Button( aguix, 0, 0, catalog.getLocale( 733 ), 0 ),
                                     2, 0, AContainer::CO_INCW );
  
    AContainerBB *ac1_res_store = dynamic_cast<AContainerBB*>( ac1->add( new AContainerBB( m_win, 1, 3 ), 0, 2 ) );
    ac1_res_store->setMinSpace( 5 );
    ac1_res_store->setMaxSpace( 5 );
    ac1_res_store->setBorderWidth( 5 );

    ac1_res_store->add( new Text( aguix, 0, 0, catalog.getLocale( 734 ) ), 0, 0, AContainer::CO_INCW );
    _storelv = (FieldListView*)ac1_res_store->add( new FieldListView( aguix, 0, 0, 100, 5 * aguix->getCharHeight(), 0 ), 0, 1, AContainer::CO_MIN );
    _storelv->setHBarState( 0 );
    _storelv->setVBarState( 2 );
    _storelv->setNrOfFields( 5 );
    _storelv->setFieldText( 0, catalog.getLocale( 963 ) );
    _storelv->setFieldText( 1, catalog.getLocale( 735 ) );
    _storelv->setFieldText( 2, catalog.getLocale( 729 ) );
    _storelv->setFieldText( 3, catalog.getLocale( 736 ) );
    _storelv->setFieldText( 4, catalog.getLocale( 737 ) );
    _storelv->setConsiderHeaderWidthForDynamicWidth( true );
    _storelv->setShowHeader( true );
    _storelv->setAcceptFocus( true );
    _storelv->setDisplayFocus( true );
    _storelv->setMBG( aguix->getFaces().getColor( "dirview-bg" ) );
    _storelv->setGlobalFieldSpace( 5 );

    m_clearstoreb =(Button*)ac1_res_store->add( new Button( aguix, 0, 0, catalog.getLocale( 738 ), 0 ), 0, 2, AContainer::CO_INCW );

    AContainer *ac1_6 = ac1->add( new AContainer( m_win, 2, 1 ), 0, 3 );
    ac1_6->setMinSpace( 5 );
    ac1_6->setMaxSpace( 5 );
    ac1_6->setBorderWidth( 0 );

    ac1_6->add( new Text( aguix, 0, 0, catalog.getLocale( 739 ) ), 0, 0, AContainer::CO_FIX );
    m_state_text = (Text*)ac1_6->add( new Text( aguix, 0, 0, catalog.getLocale( 740 ) ), 1, 0, AContainer::CO_INCW );
    m_state_text->setTextShrinker( RefCount<TextShrinker>( new FileNameShrinker() ) );

    AContainer *ac1_5 = ac1->add( new AContainer( m_win, 4, 1 ), 0, 4 );
    ac1_5->setMinSpace( 5 );
    ac1_5->setMaxSpace( 5 );
    ac1_5->setBorderWidth( 0 );
    m_startb =(Button*)ac1_5->add( new Button( aguix,
                                               0,
                                               0,
                                               catalog.getLocale( 741 ),
                                               0 ), 0, 0, AContainer::CO_FIX );
    m_stopb = (Button*)ac1_5->add( new Button( aguix,
                                               0,
                                               0,
                                               catalog.getLocale( 742 ),
                                               0 ), 1, 0, AContainer::CO_FIX );
    m_closeb = (Button*)ac1_5->add( new Button( aguix,
                                                0,
                                                0,
                                                catalog.getLocale( 633 ),
                                                0 ), 3, 0, AContainer::CO_FIX );
  
    int startb_maxwidth = aguix->getTextWidth( catalog.getLocale( 741 ) ), temp_width;
    temp_width = aguix->getTextWidth( catalog.getLocale( 743 ) );
    if ( temp_width > startb_maxwidth ) startb_maxwidth = temp_width;
    temp_width = aguix->getTextWidth( catalog.getLocale( 744 ) );
    if ( temp_width > startb_maxwidth ) startb_maxwidth = temp_width;

    startb_maxwidth += aguix->getTextWidth( "  " ) + 10;
    m_startb->resize( startb_maxwidth, m_startb->getHeight() );
    ac1_5->readLimits();

    m_win->setDoTabCycling( true );
    m_win->contMaximize( true );

    w = m_win->getWidth();
    h = m_win->getHeight();

    int rx, ry, rw, rh;

    aguix->getLargestDimensionOfCurrentScreen( &rx, &ry,
                                               &rw, &rh );

    sw = rw;
    sh = rh;
    sw = (int)( (double)sw * 0.6 );
    sh = (int)( (double)sh * 0.7 );
    if ( sw < 200 ) sw = 200;
    if ( sh < 100 ) sh = 100;
    if ( w < sw ) w = sw;
    if ( h < sh ) h = sh;
    m_win->resize( w, h );

    m_win->centerScreen();

    _sth = NULL;
    if ( m_show_prev_results == true ) {
        setCurrentResultStore( _stored_results.size() - 1 );
    } else {
        //    setCurrentResultStore( -1 );
        newSearch();
    }

    prepareStartCont( true );
    buildLV();
    buildStoreLV();

    if ( m_show_prev_results == true ) {
        _resultlv->takeFocus();
    } else {
        _matchname_sg->takeFocus();
    }
    m_win->useStippleBackground();

    _startdir_sg->selectAll( false );
    _matchname_sg->selectAll( false );
    _matchcontent_sg->selectAll( false );

    m_win->show();
    m_options_k1->show();

    m_worker->getEventHandler().registerCallback( m_timeout_cb,
                                                  EventCallbacks::TIMEOUT );

    updateWindowTitle();

    return;
}

void SearchOpBG::handleAGUIXMessage( AGMessage &msg )
{
    int endmode = -1;
    AGUIX *aguix = Worker::getAGUIX();

    switch ( msg.type ) {
        case AG_CLOSEWINDOW:
            if ( msg.closewindow.window == m_win->getWindow() ) endmode = 1;
            break;
        case AG_BUTTONCLICKED:
            if ( msg.button.button == m_closeb ) endmode = 1;
            else if ( msg.button.button == m_startb ) {
                if ( _sth == NULL || _sth->getCurrentMode() == SearchThread::FINISHED ) {
                    startSearch();
                } else {
                    if ( _sth->getCurrentMode() == SearchThread::SEARCHING ) {
                        _sth->setOrder( SearchThread::SUSPEND );
                    } else {
                        _sth->setOrder( SearchThread::SEARCH );
                    }
                }
            } else if ( msg.button.button == m_newsearchb ) {
                newSearch();
            } else if ( msg.button.button == m_stopb ) {
                if ( _sth != NULL )
                    _sth->setOrder( SearchThread::STOP );
            } else if ( msg.button.button == m_enterb ) {
                if ( msg.button.state == 2 ) {
                    endmode = 3;
                } else {
                    endmode = 2;
                }
            } else if ( msg.button.button == m_panelize_b ) {
                if ( msg.button.state == 2 ) {
                    endmode = 5;
                } else {
                    endmode = 4;
                }
            } else if ( msg.button.button == m_clearstoreb ) {
                clearStoredResults();
                newSearch();
            } else if ( msg.button.button == m_removeb ) {
                removeSelectedResults();
            } else if ( msg.button.button == m_viewb ) {
                viewResult();
            } else if ( msg.button.button == m_editb ) {
                editResult();
            } else if ( msg.button.button == m_younger_cal_b ) {
                Calendar c;
                c.setInitialDate( m_younger_sg->getText() );
                if ( c.showCalendar() ) {
                    m_younger_sg->setText( c.getDate().c_str() );
                    m_younger_cb->setState( true );
                }
            } else if ( msg.button.button == m_older_cal_b ) {
                Calendar c;
                c.setInitialDate( m_older_sg->getText() );
                if ( c.showCalendar() ) {
                    m_older_sg->setText( c.getDate().c_str() );
                    m_older_cb->setState( true );
                }
            } else if ( msg.button.button == m_size_help_b ) {
                Requester req( aguix, m_win );
                req.request( catalog.getLocale( 124 ), catalog.getLocale( 820 ), catalog.getLocale( 11 ) );
            }
            break;
        case AG_FIELDLV_ONESELECT:
        case AG_FIELDLV_MULTISELECT:
            if ( msg.fieldlv.lv == _storelv ) {
                int row = _storelv->getActiveRow();
                if ( _storelv->isValidRow( row ) == true ) {
                    stopSearch();
                    setCurrentResultStore( row );
                    buildLV();
                }
            } else if ( msg.fieldlv.lv == _resultlv ) {
                int row = _resultlv->getActiveRow();
                if ( _resultlv->isValidRow( row ) == true ) {
                    storeActiveRow( row );
                }
            }
            break;
        case AG_FIELDLV_DOUBLECLICK:
            if ( msg.fieldlv.lv == _resultlv ) {
                endmode = 2;
            }
            break;
        case AG_STRINGGADGET_OK:
            if ( msg.stringgadget.sg == _startdir_sg ) {
                if ( msg.stringgadget.keystate == ControlMask ) {
                    startSearch();
                } else {
                    _matchname_sg->takeFocus();
                }
            } else if ( msg.stringgadget.sg == _matchname_sg ) {
                if ( msg.stringgadget.keystate == ControlMask ) {
                    startSearch();
                } else {
                    _matchcontent_sg->takeFocus();
                }
            } else if ( msg.stringgadget.sg == _matchcontent_sg ) {
                startSearch();
            } else if ( msg.stringgadget.sg == m_younger_sg ) {
                m_younger_cb->setState( true );
            } else if ( msg.stringgadget.sg == m_older_sg ) {
                m_older_cb->setState( true );
            } else if ( msg.stringgadget.sg == m_size_geq_sg ) {
                m_size_geq_cb->setState( true );
            } else if ( msg.stringgadget.sg == m_size_leq_sg ) {
                m_size_leq_cb->setState( true );
            }
            break;
        case AG_KEYPRESSED:
            if ( m_win->isParent( msg.key.window, false ) ) {
                if ( msg.key.key == XK_Return && _resultlv->getHasFocus() == true ) {
                    int row = _resultlv->getActiveRow();
                    if ( _resultlv->isValidRow( row ) == true ) {
                        endmode = 2;
                    }
                } else if ( msg.key.key == XK_Return && KEYSTATEMASK( msg.key.keystate ) == ControlMask ) {
                    startSearch();
                } else if ( msg.key.key == XK_Escape ) {
                    endmode = 1;
                } else if ( msg.key.key == XK_F1 ) {
                    endmode = 2;
                } else if ( msg.key.key == XK_F2 ) {
                    endmode = 4;
                } else if ( msg.key.key == XK_F3 ) {
                    viewResult();
                } else if ( msg.key.key == XK_F4 ) {
                    editResult();
                } else if ( msg.key.key == XK_BackSpace && _resultlv->getHasFocus() == true ) {
                    removeSelectedResults();
                } else if ( msg.key.key == XK_F5 ) {
                    newSearch();
                }
            }
            break;
    }

    if ( endmode == 2 || endmode == 3 ) {
        //enter active entry
        int row = _resultlv->getActiveRow();
        if ( _resultlv->isValidRow( row ) ) {
            enterResult( _resultlv->getText( row, 2 ) );
        }
    } else if ( endmode == 4 || endmode == 5 ) {
        // panelize
        panelizeResults();
    }

    if ( endmode == 1 || endmode == 2 || endmode == 4) {
        stopSearch();

        aguix->unregisterBGHandler( m_win );
    }
}

void SearchOpBG::setInitialStartDir( const std::string &dir )
{
    m_initial_start_dir = dir;
}

void SearchOpBG::setShowPrevResults( bool nv )
{
    m_show_prev_results = nv;
}

void SearchOpBG::setEditCommand( const std::string &str )
{
    m_edit_command = str;
}

AWindow *SearchOpBG::getAWindow()
{
    return m_win;
}

void SearchOpBG::timeout_callback()
{
    checkForResults();
}

void SearchOpBG::checkForResults()
{
    bool build_lv = false;

    if ( _stored_results_changed != m_stored_result_last_changed ) {
        m_stored_result_last_changed = _stored_results_changed;
        build_lv = true;
    }

    if ( _sth == NULL || _sth->getCurrentMode() == SearchThread::FINISHED ) {
        prepareButton( *m_startb, BUTTON_IDLE );

        /* the thread is finished so update store to show total nr of results
         * but only after all results are gathered */
        if ( _search_started == true &&
             ( _sth == NULL || _sth->resultsAvailable() == false ) ) {

            build_lv = true;

            _search_started = false;

            if ( m_timeout_set ) {
                m_worker->unregisterTimeout( searchopbg_timeout );
                m_timeout_set = false;
            }

            updateState( *m_state_text );
        }
    } else if ( _sth->getCurrentMode() == SearchThread::SEARCHING ) {
        prepareButton( *m_startb, BUTTON_SUSPEND );
    } else if ( _sth->getCurrentMode() == SearchThread::SUSPENDED ) {
        prepareButton( *m_startb, BUTTON_CONTINUE );
    }

    if ( build_lv ) {
        buildStoreLV();
    }

    if ( (++m_update_ctr % 10 ) == 0 )
        updateState( *m_state_text );

    if ( _sth != NULL && _sth->resultsAvailable() ) {
        gatherResults( -1, searchopbg_timeout / 2 );
    }
}

void SearchOpBG::panelizeResults()
{
    Lister *l1;

    l1 = m_worker->getActiveLister();
    if ( l1 != NULL ) {
        l1->switch2Mode( 0 );

        VirtualDirMode *vdm = dynamic_cast< VirtualDirMode* >( l1->getActiveMode() );
        if ( vdm != NULL ) {
            vdm->newTab();

            vdm->showDir( *_rst->getDir() );
        }
    }
}

void SearchOpBG::setInitialVDir( std::unique_ptr< NWC::Dir > d )
{
    m_initial_start_vdir = std::move( d );
}

void SearchOpBG::updateWindowTitle()
{
    std::string new_title = catalog.getLocale( 1291 );

    if ( _rst != NULL ) {

        new_title += ": ";

        SearchSettings sets = _rst->getSearchSettings();

        new_title += sets.getBaseDir();

        new_title += "/";

        new_title += sets.getMatchName();

        if ( ! sets.getMatchContent().empty() ) {

            new_title += ":";

            new_title += sets.getMatchContent();
        }
    }

    m_win->setTitle( new_title );
}
