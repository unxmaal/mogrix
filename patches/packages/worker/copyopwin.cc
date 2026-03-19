/* copyopwin.cc
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2001-2014 Ralf Hoffmann.
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

#include "worker.h"
#include "copyopwin.hh"
#include <iostream>
#include "aguix/util.h"
#include "aguix/lowlevelfunc.h"
#include "aguix/awindow.h"
#include "aguix/button.h"
#include "aguix/bevelbox.h"
#include "aguix/solidbutton.h"
#include "aguix/choosebutton.h"
#include "worker_locale.h"
#include "datei.h"
#include "pers_kvp.hh"

#define HUMAN_READABLE_DIALOG 1

CopyOpWin::CopyOpWin( AGUIX*taguix, bool _move )
{
  this->aguix=taguix;
  copied_bytes_curfile=0;
  bytes_curfile=0;
  copied_bytes=0;
  bytes=0;
  copied_files=0;
  files=0;
  copied_dirs=0;
  dirs=0;
  
  window=NULL;
  melw = 0;
  filenamespace[0] = 0;
  filenamespace[1] = 0;
  ismessage[0] = false;
  ismessage[1] = false;

  update_sourcetext = false;
  update_desttext = false;
  update_filetext = false;
  update_globtext = false;
  update_files2gotext = false;
  update_dirs2gotext = false;
  
  mintime = 1000000;
  lastbytes = 0;
  lastrate = 0.0;
  lastbytes_global = 0;
  lastrate_global = 0.0;
  
  glob_percent = 0;
  
  move = _move;
  
  lencalc = new AFontWidth( aguix, NULL );
  memset( &stoptime, 0, sizeof( stoptime ) );
  timer_stopped = false;
  
  gettimeofday( &last_update_time, NULL );
  skipped_updates = 0;

  m_master_thread_id = std::this_thread::get_id();

  m_finished = false;
  m_detached = false;

  int v = Worker::getPersKVPStore().getIntValue( "copy-keep-window" );

  // 0 is default for non-existing entry, 2 means enabled
  if ( v == 0 ||
       v == 2 ) {
      m_keep_window = true;
  } else {
      m_keep_window = false;
  }

  m_keep_window_cb = nullptr;
}

CopyOpWin::~CopyOpWin()
{
  if(window!=NULL) close();
  delete lencalc;
}

int
CopyOpWin::open()
{
  int w,h;
  int tx,ty;
  Text *ttext;

  close();
  w=400;
  h=10;
  window = new AWindow( aguix,
			10, 10,
			w, h,
			( move == true ) ? catalog.getLocale( 299 ) : catalog.getLocale( 1249 ),
                        AWindow::AWINDOW_NORMAL );
  window->create();
  tx=5;
  ty=5;
  sourcetext=(Text*)window->add(new Text(aguix,tx,ty,""));
  ismessage[0] = true;
  ty+=sourcetext->getHeight()+5;
  desttext=(Text*)window->add(new Text(aguix,tx,ty,""));
  ismessage[1] = true;
  ty+=desttext->getHeight()+5;
  fsb=(SolidButton*)window->add(new SolidButton(aguix,tx+1,ty+1,w-2*(tx+1),"","button-special1-fg", "button-special1-bg",0));
  sbw=fsb->getWidth();
  bb1=(BevelBox*)window->add(new BevelBox(aguix,tx,ty,w-2*tx,fsb->getHeight()+2,1));
  fsb->toFront();
  ty+=bb1->getHeight()+5;
  tx=10;
  filetext=(Text*)window->add(new Text(aguix,tx,ty,""));
  ty+=filetext->getHeight()+5;

  ttext = (Text*)window->add( new Text( aguix, tx, ty, catalog.getLocale( 90 ) ) );
  tx += ttext->getWidth() + 5;
  fileavgtext = (Text*)window->add( new Text( aguix, tx, ty, "" ) );
  ty += fileavgtext->getHeight() + 5;

  tx=5;
  bb2=(BevelBox*)window->add(new BevelBox(aguix,tx,ty,w-2*tx,2,0));
  ty+=bb2->getHeight()+5;

  ttext=(Text*)window->add(new Text(aguix,tx,ty,catalog.getLocale(118)));
  tx+=ttext->getWidth()+5;
  files2gotext=(Text*)window->add(new Text(aguix,tx,ty,""));
  tx=5;
  ty+=ttext->getHeight()+5;
  ttext=(Text*)window->add(new Text(aguix,tx,ty,catalog.getLocale(119)));
  tx+=ttext->getWidth()+5;
  dirs2gotext=(Text*)window->add(new Text(aguix,tx,ty,""));
  tx=5;
  ty+=ttext->getHeight()+5;
  gsb=(SolidButton*)window->add(new SolidButton(aguix,tx+1,ty+1,w-2*(tx+1),"","button-special1-fg", "button-special1-bg",0));
  bb3=(BevelBox*)window->add(new BevelBox(aguix,tx,ty,w-2*tx,gsb->getHeight()+2,1));
  gsb->toFront();
  ty+=bb3->getHeight()+5;
  tx=10;
  ttext=(Text*)window->add(new Text(aguix,tx,ty,catalog.getLocale(120)));
  tx+=ttext->getWidth()+5;
  globtext=(Text*)window->add(new Text(aguix,tx,ty,""));
  tx=5;
  ty+=globtext->getHeight()+5;
  ttext=(Text*)window->add(new Text(aguix,tx,ty,catalog.getLocale(121)));
  tx+=ttext->getWidth()+5;
  timetext=(Text*)window->add(new Text(aguix,tx,ty,""));
  ty+=timetext->getHeight()+5;
  tx=5;
  bb4=(BevelBox*)window->add(new BevelBox(aguix,tx,ty,w-2*tx,2,0));
  ty+=bb4->getHeight()+5;

  m_detach_b = (Button*)window->add( new Button( aguix, tx, ty, catalog.getLocale( 1039 ), 0 ) );

  m_keep_window_cb = (ChooseButton*)window->add( new ChooseButton( aguix, tx, ty,
                                                                   m_keep_window,
                                                                   catalog.getLocale( 1047 ),
                                                                   LABEL_RIGHT, 0 ) );
  m_keep_window_cb->hide();

  cb = (Button*)window->add( new Button( aguix, tx, ty, catalog.getLocale( 8 ), 0 ) );
  cb->move( w - 5 - cb->getWidth(), cb->getY() );

  ty+=cb->getHeight()+5;
  h=ty;
  window->setDoTabCycling( true );
  window->resize(w,h);
  window->setMinSize(w,h);
  //  window->setMaxSize(w,h);
  window->centerScreen();
  window->show();

  filenamespace[0] = w - window->getBorderWidth() - sourcetext->getX();
  filenamespace[0] -= aguix->getTextWidth( catalog.getLocale( ( move == true ) ? 594 : 116 ) ) - 2 * aguix->getTextWidth( " " );
  filenamespace[1] = w - window->getBorderWidth() - desttext->getX();
  filenamespace[1] -= aguix->getTextWidth( catalog.getLocale( 117 ) ) - 2 * aguix->getTextWidth( " " );

  return 0;
}

void
CopyOpWin::close()
{
  if(window!=NULL) {
    delete window;
  }
  window=NULL;
}

void
CopyOpWin::starttimer()
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

    gettimeofday( &starttime, NULL );
    lasttime_global = starttime;
}

void
CopyOpWin::set_files_to_copy(long nfiles)
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  if(nfiles>0) files=nfiles;
  else files=0;
  update_files2gotext=true;
}

void
CopyOpWin::set_dirs_to_copy(long ndirs)
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  if(ndirs>0) dirs=ndirs;
  else dirs=0;
  update_dirs2gotext=true;
}

void
CopyOpWin::set_bytes_to_copy(loff_t nbytes)
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  if(nbytes>0) bytes=nbytes;
  else bytes=0;
  update_globtext=true;
}

void
CopyOpWin::set_bytes_to_copy_curfile( loff_t nbytes )
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  if ( nbytes > 0 ) bytes_curfile = nbytes;
  else bytes_curfile = 0;
  update_filetext = true;
}

void
CopyOpWin::dir_finished()
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  copied_dirs++;
  update_dirs2gotext=true;
}

void
CopyOpWin::file_finished()
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  // inc counter
  copied_files++;
  update_files2gotext=true;
  if ( skipped_updates > 0 ) {
    update_filetext = true;
    update_globtext = true;
  }
}

void
CopyOpWin::add_curbytes_copied( loff_t nbytes )
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  if ( nbytes > 0 ) {
    copied_bytes_curfile += nbytes;
    copied_bytes += nbytes;
  }

  struct timeval curtime;
  gettimeofday( &curtime, NULL );
  if ( ldiffgtod_m( &curtime, &last_update_time ) >= ( 1000 / UPDATES_PER_SECOND ) ) {
    update_filetext = true;
    update_globtext = true;
    last_update_time = curtime;
    skipped_updates = 0;
  } else {
    skipped_updates++;
  }
}

void
CopyOpWin::newfile( const std::string &name,
                    const std::string &ndest )
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

    source = name;

    dest = ndest;

  gettimeofday( &filestarttime, NULL );
  update_sourcetext=true;
  update_desttext=true;
  ismessage[0] = false;
  ismessage[1] = false;
  copied_bytes_curfile = 0;
  update_filetext=true;

  lastbytes = 0;
  lasttime = filestarttime;
  lastrate = 0.0;
}

int
CopyOpWin::redraw( bool skip_message_handling )
{ /* this functions will redraw the window AND process events for this window
   * returns values:
   *  0 for regular exit
   *  1 when clicked Cancel-Button or closed window
   *  other not implemented */
  int returnvalue=0;
  long rtime, rtime_at_current_rate;
  double drtime;
  struct timeval curtime;
  double difftime2, lastdiff2;
  char tstr[1024],
       *tstr2;
  double rate;
  int tw;
  bool doresize = false;
  int newglob_percent;
  std::string copied_bytes_curfile_str,
              bytes_curfile_str,
              copied_bytes_str,
              bytes_str;
  
  char *tstr3;

    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  timerclear( &curtime );
  if ( update_sourcetext == true ) {
      if ( ismessage[0] == false ) {
          tstr3 = Datei::shrinkFilename( source.c_str(), filenamespace[0], *lencalc );
          if ( tstr3 == NULL)
              tstr3 = dupstring( source.c_str() );
          tstr2 = (char*)_allocsafe( strlen( catalog.getLocale( ( move == true ) ? 594 : 116 ) ) + strlen( tstr3 ) + 1 );
          sprintf( tstr2, catalog.getLocale( ( move == true ) ? 594 : 116 ), tstr3 );
          _freesafe( tstr3 );
          sourcetext->setText(tstr2);
          _freesafe(tstr2);
          /* no need for explicit redraw */
      } else {
          sourcetext->setText( source.c_str() );
      }
      update_sourcetext = false;
  }
  if ( update_desttext == true ) {
      if ( ismessage[1] == false ) {
          tstr3 = Datei::shrinkFilename( dest.c_str(), filenamespace[1], *lencalc );
          if ( tstr3 == NULL)
              tstr3 = dupstring( dest.c_str() );
          tstr2 = (char*)_allocsafe( strlen( catalog.getLocale( 117 ) ) + strlen( tstr3 ) + 1 );
          sprintf( tstr2, catalog.getLocale( 117 ), tstr3 );
          _freesafe(tstr3);
          desttext->setText(tstr2);
          _freesafe(tstr2);
          // no need for explicit redraw
      } else {
          desttext->setText( dest.c_str() );
      }
      update_desttext = false;
  }
  if(update_filetext==true) {
    if ( ! timerisset( &curtime ) ) gettimeofday( &curtime, NULL );
    difftime2 = diffgtod( &curtime, &filestarttime );
    lastdiff2 = diffgtod( &curtime, &lasttime );

    if ( lastdiff2 >= mintime ) {
      lastrate = (double)( copied_bytes_curfile - lastbytes ) * 1000000.0 / lastdiff2;
      lastbytes = copied_bytes_curfile;
      lasttime = curtime;
    }

    if ( HUMAN_READABLE_DIALOG ) {
        copied_bytes_curfile_str = AGUIXUtils::bytes_to_human_readable_f( (double)copied_bytes_curfile,
                                                                          1, ( lastrate > 0.0 ) ? 5 : 0,
                                                                          AGUIXUtils::BM_SYSTEM_SETTING,
                                                                          lastrate );
        bytes_curfile_str = AGUIXUtils::bytes_to_human_readable_f( (double)bytes_curfile,
                                                                   1, ( lastrate > 0.0 ) ? 5 : 0,
                                                                   AGUIXUtils::BM_SYSTEM_SETTING,
                                                                   lastrate );

        snprintf( tstr, sizeof( tstr ), "%s / %s @ %s/s",
                  copied_bytes_curfile_str.c_str(),
                  bytes_curfile_str.c_str(),
                  AGUIXUtils::bytes_to_human_readable_f( lastrate ).c_str() );
        tstr[ sizeof( tstr ) - 1 ] = '\0';
    } else {
        MakeLong2NiceStr( copied_bytes_curfile, copied_bytes_curfile_str );

        MakeLong2NiceStr( bytes_curfile, bytes_curfile_str );
        snprintf( tstr, sizeof( tstr ), "%s / %s @ %.2f KB/s",
                  copied_bytes_curfile_str.c_str(),
                  bytes_curfile_str.c_str(),
                  lastrate / 1024.0 );
        tstr[ sizeof( tstr ) - 1 ] = '\0';
    }

    filetext->setText(tstr);
    tw = filetext->getX() + filetext->getWidth() + window->getBorderWidth();
    if ( tw > melw ) {
      melw = tw;
      doresize = true;
    }
    
    if(difftime2>0) {
      rate = (double)copied_bytes_curfile * 1000000.0 / difftime2;
    } else rate=0.0;

    if ( HUMAN_READABLE_DIALOG ) {
        snprintf( tstr, sizeof( tstr ), "%s/s", AGUIXUtils::bytes_to_human_readable_f( rate ).c_str() );
    } else {
        snprintf( tstr, sizeof( tstr ), "%.2f KB/s", rate / 1024.0 );
    }

    tstr[ sizeof( tstr ) - 1 ] = '\0';
    fileavgtext->setText( tstr );

    int rate_precision = AGUIXUtils::calc_precision( rate * 100.0 / (double)bytes_curfile );
    if ( rate_precision < 0 ) rate_precision = 0;

    if(bytes_curfile>0)
      rate = (double)copied_bytes_curfile/(double)bytes_curfile;
    else
      rate=0.0;
    if ( rate < 0.01 )
      rate = 0.0;

    std::string fraction_str = AGUIXUtils::formatStringToString( "%%.%df %%%%",
                                                                 rate_precision );

    snprintf( tstr, sizeof( tstr ), fraction_str.c_str(), rate * 100.0 );
    tstr[ sizeof( tstr ) - 1 ] = '\0';
    fsb->setText(tstr);
    fsb->resize( w_max( (int)( sbw * rate ), 2 ), fsb->getHeight() );
    update_filetext=false;
  }
  if(update_globtext==true) {
    if ( ! timerisset( &curtime ) ) gettimeofday( &curtime, NULL );
    difftime2 = diffgtod( &curtime, &starttime );
    if(difftime2>0.0) {
      rate = (double)copied_bytes * 1000000.0 / difftime2;
    } else rate=0.0;

    double lastdiff_global = diffgtod( &curtime, &lasttime_global );

    if ( lastdiff_global >= 10.0 * mintime ) {
      lastrate_global = (double)( copied_bytes - lastbytes_global ) * 1000000.0 / lastdiff_global;
      lastbytes_global = copied_bytes;
      lasttime_global = curtime;
    }

    if ( HUMAN_READABLE_DIALOG ) {
        copied_bytes_str = AGUIXUtils::bytes_to_human_readable_f( (double)copied_bytes,
                                                                  1, ( rate > 0.0 ) ? 5 : 0,
                                                                  AGUIXUtils::BM_SYSTEM_SETTING,
                                                                  rate );

        bytes_str = AGUIXUtils::bytes_to_human_readable_f( (double)bytes,
                                                           1, ( rate > 0.0 ) ? 5 : 0,
                                                           AGUIXUtils::BM_SYSTEM_SETTING,
                                                           rate );

        snprintf( tstr, sizeof( tstr ), "%s / %s @ %s/s", copied_bytes_str.c_str(), bytes_str.c_str(), AGUIXUtils::bytes_to_human_readable_f( rate ).c_str() );
        tstr[ sizeof( tstr ) - 1 ] = '\0';
    } else {
        MakeLong2NiceStr( copied_bytes, copied_bytes_str );
        MakeLong2NiceStr( bytes, bytes_str );
        snprintf( tstr, sizeof( tstr ), "%s / %s @ %.2f KB/s", copied_bytes_str.c_str(), bytes_str.c_str(), rate / 1024.0 );
        tstr[ sizeof( tstr ) - 1 ] = '\0';
    }
    
    globtext->setText(tstr);
    tw = globtext->getX() + globtext->getWidth() + window->getBorderWidth();
    if ( tw > melw ) {
      melw = tw;
      doresize = true;
    }
    
    int rate_precision = AGUIXUtils::calc_precision( rate * 100.0 / (double)bytes );
    if ( rate_precision < 0 ) rate_precision = 0;

    if(bytes>0)
        rate = (double)copied_bytes/(double)bytes;
    else
        rate=0.0;
    if ( rate < 0.01 )
        rate = 0.0;

    std::string fraction_str = AGUIXUtils::formatStringToString( "%%.%df %%%%",
                                                                 rate_precision );

    snprintf( tstr, sizeof( tstr ), fraction_str.c_str(), rate * 100.0 );

    tstr[ sizeof( tstr ) - 1 ] = '\0';
    gsb->setText(tstr);
    gsb->resize( w_max( (int)( sbw * rate ), 2 ), gsb->getHeight() );
      
    newglob_percent = (int)( rate * 100.0 );
    if ( newglob_percent != glob_percent ) {
        glob_percent = newglob_percent;
        tstr3 = (char*)_allocsafe( strlen( ( move == true ) ? catalog.getLocale( 299 ) : catalog.getLocale( 1249 ) ) +
                                   strlen( " ( %)" ) + 
                                   A_BYTESFORNUMBER( glob_percent ) +
                                   1 );
        if ( ( glob_percent > 0 ) && ( glob_percent <= 100 ) ) {
            sprintf( tstr3, "%s (%d %%)",
                     ( move == true ) ? catalog.getLocale( 299 ) : catalog.getLocale( 1249 ),
                     glob_percent );
        } else {
            sprintf( tstr3, "%s",
                     ( move == true ) ? catalog.getLocale( 299 ) : catalog.getLocale( 1249 ) );
        }
        window->setTitle( tstr3 );
        _freesafe( tstr3 );
    }
      
    if(rate>0) {
        drtime = difftime2 / rate;
        drtime -= difftime2;
        rtime = (long)( drtime / 1000000.0 );
    } else
        rtime = 700000;  // no rate available => no real time calculatable

    if ( lastrate_global != 0.0 ) {
        double remaining_bytes = (double)bytes - (double)copied_bytes;
        rtime_at_current_rate = remaining_bytes / lastrate_global;
        rtime_at_current_rate++;
    } else {
        rtime_at_current_rate = 700000;
    }

    rtime++; // add a second because the last second will shown as 0:00
    // and at least I don't like this
    if ( rtime > 604800 ) // more then 7 days
        sprintf( tstr, "--:--" );
    else 
        sprintf( tstr, "%ld:%02ld", rtime / 60, rtime % 60 );

    if ( rtime_at_current_rate <= 604800 &&
         fabs( rtime - rtime_at_current_rate ) > 10.0 &&
         fabs( rtime - rtime_at_current_rate ) / (double)rtime > 0.1 ) {
        std::string alt_time_str = AGUIXUtils::formatStringToString( catalog.getLocale( 973 ),
                                                                     tstr,
                                                                     rtime_at_current_rate / 60,
                                                                     rtime_at_current_rate % 60 );
        timetext->setText( alt_time_str.c_str() );
    } else {
        timetext->setText(tstr);
    }

    tw = timetext->getX() + timetext->getWidth() + window->getBorderWidth();
    if ( tw > melw ) {
        melw = tw;
        doresize = true;
    }

    update_globtext=false;
  }
  if(update_files2gotext==true) {
    sprintf(tstr,"%ld",files-copied_files);
    files2gotext->setText(tstr);
    tw = files2gotext->getX() + files2gotext->getWidth() + window->getBorderWidth();
    if ( tw > melw ) {
      melw = tw;
      doresize = true;
    }
    
    update_files2gotext=false;
  }
  if(update_dirs2gotext==true) {
    sprintf(tstr,"%ld",dirs-copied_dirs);
    dirs2gotext->setText(tstr);
    tw = dirs2gotext->getX() + dirs2gotext->getWidth() + window->getBorderWidth();
    if ( tw > melw ) {
      melw = tw;
      doresize = true;
    }
    
    update_dirs2gotext=false;
  }
  if ( doresize == true ) {
    // only resize if window is smaller
    if ( window->getWidth() < melw ) {
      window->resize( melw, window->getHeight() );
    }
    // anyway the min size has changed so set it again
    window->setMinSize( melw, bb4->getY() + bb4->getHeight() + 5 +cb->getHeight() + window->getBorderWidth() );
  }

  //window->redraw();
  aguix->Flush();

  if ( ! skip_message_handling ) {
      /* update complete
         now check for X-messages */
      AGMessage *msg;
      do {
          msg=aguix->GetMessage( window );

          returnvalue |= handleMessage( msg );

          aguix->ReplyMessage(msg);
      } while(msg!=NULL);
  }
  
  return returnvalue;
}

int CopyOpWin::handleMessage( AGMessage *msg )
{
    int returnvalue=0;

    if(msg!=NULL) {
      switch(msg->type) {
        case AG_CLOSEWINDOW:
            if ( msg->closewindow.window == window->getWindow() ) {
                if ( m_finished ) {
                    returnvalue = 4;
                } else {
                    returnvalue = 1;
                }
            }
          break;
        case AG_BUTTONCLICKED:
            if ( msg->button.button == cb ) {
                if ( m_finished ) {
                    returnvalue = 4;
                } else {
                    returnvalue = 1;
                }
            } else if ( msg->button.button == m_detach_b ) {
                returnvalue = 2;
            }
          break;
        case AG_CHOOSECLICKED:
            if ( msg->choose.button == m_keep_window_cb ) {
                m_keep_window = m_keep_window_cb->getState();

                Worker::getPersKVPStore().setIntValue( "copy-keep-window",
                                                       ( m_keep_window ? 2 : 1 ) );
            }
            break;
        case AG_SIZECHANGED:
          if ( msg->size.window == window->getWindow() ) {
            std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

            bb1->resize( msg->size.neww - 2 * window->getBorderWidth(), bb1->getHeight() );
            sbw = bb1->getWidth() - 2;
            bb2->resize( msg->size.neww - 2 * window->getBorderWidth(), bb2->getHeight() );
            bb3->resize( msg->size.neww - 2 * window->getBorderWidth(), bb3->getHeight() );
            bb4->resize( msg->size.neww - 2 * window->getBorderWidth(), bb4->getHeight() );
            cb->move( msg->size.neww -5 - cb->getWidth(),
                      msg->size.newh - window->getBorderWidth() - cb->getHeight() );

            m_detach_b->move( 5,
                              msg->size.newh - window->getBorderWidth() - m_detach_b->getHeight() );
            m_keep_window_cb->move( 5,
                                    msg->size.newh - window->getBorderWidth() - m_keep_window_cb->getHeight() );

            filenamespace[0] = msg->size.neww - window->getBorderWidth() - sourcetext->getX();
            filenamespace[0] -= aguix->getTextWidth( catalog.getLocale( ( move == true ) ? 594 : 116 ) ) - 2 * aguix->getTextWidth( " " );
            filenamespace[1] = msg->size.neww - window->getBorderWidth() - desttext->getX();
            filenamespace[1] -= aguix->getTextWidth( catalog.getLocale( 117 ) ) - 2 * aguix->getTextWidth( " " );
            update_sourcetext = true;
            update_desttext = true;
  
            // now gsb and fsb are not scaled for new width
            // but next call of redraw they will so this shouldn't hurt anybody
            // but a solution would be to first react for messages and then redraw

            if ( m_finished ) {
                double rate = 0.0;

                if ( bytes_curfile > 0 ) {
                    rate = (double)copied_bytes_curfile / (double)bytes_curfile;
                }

                fsb->resize( w_max( (int)( sbw * rate ), 2 ), fsb->getHeight() );

                if ( bytes > 0 ) {
                    rate = (double)copied_bytes / (double)bytes;
                } else {
                    rate=0.0;
                }

                gsb->resize( w_max( (int)( sbw * rate ), 2 ), gsb->getHeight() );
            }
          }
          break;
      }
    }
  
  return returnvalue;
}

void
CopyOpWin::setmessage( const std::string &msg, int line )
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

    switch(line) {
        case 0:
            source = msg;
            ismessage[0] = true;
            update_sourcetext = true;
            break;
        case 1:
            dest = msg;
            ismessage[1] = true;
            update_desttext = true;
            break;
    }
}

void
CopyOpWin::dec_file_counter(unsigned long f)
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  files-=f;
  update_files2gotext=true;
}

void
CopyOpWin::dec_dir_counter(unsigned long d)
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  dirs-=d;
  update_dirs2gotext=true;
}

void
CopyOpWin::dec_byte_counter(loff_t b)
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  bytes-=b;
  update_globtext=true;
}

void
CopyOpWin::stoptimer()
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  gettimeofday( &stoptime, NULL );
  timer_stopped = true;
}

void
CopyOpWin::conttimer()
{
  struct timeval curtime;
  long s, us;

    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  if ( timer_stopped == false ) return;
  
  gettimeofday( &curtime, NULL );
  
  s = curtime.tv_sec - stoptime.tv_sec;
  us = curtime.tv_usec - stoptime.tv_usec;
  if ( us < 0 ) {
    s--;
    us += 1000000;
  }

  starttime.tv_sec += s;
  starttime.tv_usec += us;
  if ( starttime.tv_usec > 1000000 ) {
    starttime.tv_sec++;
    starttime.tv_usec -= 1000000;
  }
  filestarttime.tv_sec += s;
  filestarttime.tv_usec += us;
  if ( filestarttime.tv_usec > 1000000 ) {
    filestarttime.tv_sec++;
    filestarttime.tv_usec -= 1000000;
  }

  lasttime_global.tv_sec += s;
  lasttime_global.tv_usec += us;
  if ( lasttime_global.tv_usec > 1000000 ) {
      lasttime_global.tv_sec++;
      lasttime_global.tv_usec -= 1000000;
  }
}

int CopyOpWin::request_int( const char *title,
                            const char *text,
                            const char *buttons,
                            Requester::request_flags_t flags )
{
  if ( window != NULL ) {
    return window->request( title, text, buttons, flags );
  } else {
    return Worker::getRequester()->request( title, text, buttons, flags );
  }
}

int CopyOpWin::string_request_int( const char *title,
                                   const char *lines,
                                   const char *default_str,
                                   const char *buttons,
                                   char **return_str,
                                   Requester::request_flags_t flags )
{
  if ( window != NULL ) {
    return window->string_request( title, lines, default_str, buttons, return_str, flags );
  } else {
    return Worker::getRequester()->string_request( title, lines, default_str, buttons, return_str, flags );
  }
}

void CopyOpWin::file_skipped( loff_t byteSum, bool subCopied )
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  if ( files > 0 ) files--;
  if ( byteSum > 0 ) bytes -= byteSum;
  if ( subCopied == true ) copied_bytes -= copied_bytes_curfile;
  update_globtext = true;
  update_files2gotext = true;
}

void CopyOpWin::update_file_status()
{
    std::unique_lock< std::recursive_mutex > lck( m_status_update_lock );

  if ( skipped_updates > 0 ) {
    update_filetext = true;
    update_globtext = true;
  }
}

void CopyOpWin::master_process_request()
{
    std::unique_lock<std::mutex> lck( m_request_lock );

    if ( m_request ) {
        process_request( *m_request );

        std::unique_lock<std::mutex> lck( m_response_lock );

        m_response = std::move( m_request );

        m_response_cond.notify_one();
    }
}

void CopyOpWin::push_request( const BGCopyRequestMessage &msg )
{
    std::unique_lock<std::mutex> lck( m_request_lock );

    m_request = std::unique_ptr< BGCopyRequestMessage >( new BGCopyRequestMessage( msg ) );
}

BGCopyRequestMessage CopyOpWin::wait_for_response()
{
    std::unique_lock<std::mutex> lck( m_response_lock );

    while ( ! m_response ) {
        m_response_cond.wait( lck );
    }

    BGCopyRequestMessage res = *m_response;

    m_response.reset();

    return res;
}

int CopyOpWin::request( const std::string &title,
                        const std::string &text,
                        const std::string &buttons,
                        Requester::request_flags_t flags )
{
    BGCopyRequestMessage msg( title, text, buttons );

    if ( std::this_thread::get_id() == m_master_thread_id ) {
        process_request( msg );

        return msg.getRes();
    } else {
        push_request( msg );

        return wait_for_response().getRes();
    }
}

int CopyOpWin::string_request( const std::string &title,
                               const std::string &lines,
                               const std::string &default_str,
                               const std::string &buttons,
                               std::string &return_str,
                               Requester::request_flags_t flags )
{
    BGCopyRequestMessage msg( title, lines, default_str, buttons, flags );

    if ( std::this_thread::get_id() == m_master_thread_id ) {
        process_request( msg );

        return_str = msg.getResultString();

        return msg.getRes();
    } else {
        push_request( msg );

        auto res = wait_for_response();

        return_str = res.getResultString();

        return res.getRes();
    }
}

void CopyOpWin::process_request( BGCopyRequestMessage &msg )
{
    if ( msg.getType() == BGCopyRequestMessage::REGULAR_REQUEST ) {
        msg.setRes( request_int( msg.getTitle().c_str(),
                                 msg.getText().c_str(),
                                 msg.getButtons().c_str() ) );
    } else if ( msg.getType() == BGCopyRequestMessage::STRING_REQUEST ) {
        char *result_string = NULL;
        int res = string_request_int( msg.getTitle().c_str(),
                                      msg.getText().c_str(),
                                      msg.getDefaultString().c_str(),
                                      msg.getButtons().c_str(),
                                      &result_string,
                                      msg.getFlags() );

        if ( result_string ) {
            msg.setResultString( result_string );

            _freesafe( result_string );
        }
        msg.setRes( res );
    } else {
        /* IRIX fix: log instead of throwing on unknown message type.
           DWARF unwinder crashes on IRIX libpthread frames. */
        std::cerr << "CopyOpWin: unknown message type" << std::endl;
    }
}

void CopyOpWin::hide_detach_button()
{
    m_detach_b->hide();

    m_keep_window_cb->show();

    m_detached = true;
}

bool CopyOpWin::getKeepWindow() const
{
    if ( ! m_detached ) return false;

    return m_keep_window;
}

void CopyOpWin::setFinished()
{
    m_finished = true;

    if ( getKeepWindow() ) {
        cb->setText( 0, catalog.getLocale( 633 ) );

        int tw = cb->getMaximumWidth();

        if ( tw > cb->getWidth() ) {
            cb->resize( tw, cb->getHeight() );
            cb->move( window->getWidth() - 5 - cb->getWidth(), cb->getY() );
        }

        setmessage( catalog.getLocale( 1046 ), 0 );
        setmessage( "", 1 );

        redraw();
    }
}
