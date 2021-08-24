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

#include <QRegularExpression>

QgsErrorMessage::QgsErrorMessage( const QString &message, const QString &tag, const QString &file, const QString &function, int line )
  : mMessage( message )
  , mTag( tag )
  , mFile( file )
  , mFunction( function )
  , mLine( line )
{
}

QgsError::QgsError( const QString &message, const QString &tag )
{
  append( message, tag );
}

void QgsError::append( const QString &message, const QString &tag )
{
  mMessageList.append( QgsErrorMessage( message, tag ) );
}

void QgsError::append( const QgsErrorMessage &message )
{
  mMessageList.append( message );
}

QString QgsError::message( QgsErrorMessage::Format format ) const
{
  QString str;

#ifdef QGISDEBUG
  QString srcUrl;
#endif

#if defined(QGISDEBUG) && defined(QGS_GIT_REMOTE_URL)
  // TODO: verify if we are not ahead to origin (remote hash does not exist)
  //       and there are no local not committed changes
  QString hash = QString( Qgis::devVersion() );
  QString remote = QStringLiteral( QGS_GIT_REMOTE_URL );
  if ( !hash.isEmpty() && !remote.isEmpty() && remote.contains( QLatin1String( "github.com" ) ) )
  {
    QString path = remote.remove( QRegularExpression( ".*github.com[:/]" ) ).remove( QStringLiteral( ".git" ) );
    srcUrl = "https://github.com/" + path + "/blob/" + hash;
  }
#endif

  const auto constMMessageList = mMessageList;
  for ( const QgsErrorMessage &m : constMMessageList )
  {
#ifdef QGISDEBUG
    QString file;
#ifndef _MSC_VER
    const int sPrefixLength = strlen( CMAKE_SOURCE_DIR ) + 1;
    file = m.file().mid( sPrefixLength );
#else
    file = m.file();
#endif
#endif

    if ( format == QgsErrorMessage::Text )
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
        where += QStringLiteral( "file: %1 row: %2" ).arg( file ).arg( m.line() );
      }
      if ( !m.function().isEmpty() )
      {
        where += QStringLiteral( "function %1:" ).arg( m.function() );
      }
      if ( !where.isEmpty() )
      {
        str += QStringLiteral( " (%1)" ).arg( where );
      }
#endif
    }
    else // QgsErrorMessage::Html
    {
      str += "<p><b>" + m.tag() + ":</b> " + m.message();
#ifdef QGISDEBUG
      const QString location = QStringLiteral( "%1 : %2 : %3" ).arg( file ).arg( m.line() ).arg( m.function() );
      if ( !srcUrl.isEmpty() )
      {
        const QString url = QStringLiteral( "%1/%2#L%3" ).arg( srcUrl, file ).arg( m.line() );
        str += QStringLiteral( "<br>(<a href='%1'>%2</a>)" ).arg( url, location );
      }
      else
      {
        str += QStringLiteral( "<br>(%1)" ).arg( location );
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
