/***************************************************************************
                         qgserror.cpp  -  Error container
                             -------------------
    begin                : October 2012
    copyright            : (C) 2012 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgis.h"
#include "qgsversion.h"
#include "qgsconfig.h"
#include "qgserror.h"
#include "qgslogger.h"

#include <QRegExp>

QgsErrorMessage::QgsErrorMessage( const QString & theMessage, const QString & theTag, const QString & theFile, const QString & theFunction, int theLine )
    : mMessage( theMessage )
    , mTag( theTag )
    , mFile( theFile )
    , mFunction( theFunction )
    , mLine( theLine )
    , mFormat( Text )
{
}

QgsError::QgsError( const QString & theMessage, const QString & theTag )
{
  append( theMessage, theTag );
}

void QgsError::append( const QString & theMessage, const QString & theTag )
{
  mMessageList.append( QgsErrorMessage( theMessage, theTag ) );
}

void QgsError::append( const QgsErrorMessage & theMessage )
{
  mMessageList.append( theMessage );
}

QString QgsError::message( QgsErrorMessage::Format theFormat ) const
{
  QString str;

#ifdef QGISDEBUG
  QString srcUrl;
#endif

#if defined(QGISDEBUG) && defined(QGS_GIT_REMOTE_URL)
  // TODO: verify if we are not ahead to origin (remote hash does not exist)
  //       and there are no local not commited changes
  QString hash = QString( QGis::QGIS_DEV_VERSION );
  QString remote = QString( QGS_GIT_REMOTE_URL );
  if ( !hash.isEmpty() && !remote.isEmpty() && remote.contains( "github.com" ) )
  {
    QString path = remote.remove( QRegExp( ".*github.com[:/]" ) ).remove( ".git" );
    srcUrl = "https://github.com/" + path + "/blob/" + hash;
  }
#endif

  Q_FOREACH ( const QgsErrorMessage& m, mMessageList )
  {
#ifdef QGISDEBUG
    QString file;
#ifndef _MSC_VER
    int sPrefixLength = strlen( CMAKE_SOURCE_DIR ) + 1;
    file = m.file().mid( sPrefixLength );
#else
    file = m.file();
#endif
#endif

    if ( theFormat == QgsErrorMessage::Text )
    {
      if ( !str.isEmpty() )
      {
        str += '\n'; // new message
      }
      if ( !m.tag().isEmpty() )
      {
        str += m.tag() + ' ';
      }
      str += m.message();
#ifdef QGISDEBUG
      QString where;
      if ( !file.isEmpty() )
      {
        where += QString( "file: %1 row: %2" ).arg( file ).arg( m.line() );
      }
      if ( !m.function().isEmpty() )
      {
        where += QString( "function %1:" ).arg( m.function() );
      }
      if ( !where.isEmpty() )
      {
        str += QString( " (%1)" ).arg( where );
      }
#endif
    }
    else // QgsErrorMessage::Html
    {
      str += "<p><b>" + m.tag() + ":</b> " + m.message();
#ifdef QGISDEBUG
      QString location = QString( "%1 : %2 : %3" ).arg( file ).arg( m.line() ).arg( m.function() );
      if ( !srcUrl.isEmpty() )
      {
        QString url = QString( "%1/%2#L%3" ).arg( srcUrl, file ).arg( m.line() );
        str += QString( "<br>(<a href='%1'>%2</a>)" ).arg( url, location );
      }
      else
      {
        str += QString( "<br>(%1)" ).arg( location );
      }
#endif
    }
  }
  return str;
}

QString QgsError::summary() const
{
  // The first message in chain is usually the real error given by backend/server
  return mMessageList.first().message();
}
