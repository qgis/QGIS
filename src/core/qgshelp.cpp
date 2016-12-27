/***************************************************************************
  qgshelp.cpp
  --------------------------------------
  Date                 : December 2016
  Copyright            : (C) 2016 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshelp.h"

#include <QSettings>
#include <QLocale>
#include <QUrl>
#include <QFileInfo>
#include <QTcpSocket>
#include <QDesktopServices>

#include "qgis.h"
#include "qgsapplication.h"

QgsHelp *QgsHelp::sHelp = nullptr; // Singleton instance

void QgsHelp::openHelp( const QString& key )
{
  if ( !sHelp )
  {
    sHelp = new QgsHelp();
  }

  QDesktopServices::openUrl( sHelp->helpUrl( key ) );
}

QUrl QgsHelp::helpUrl( const QString& key )
{
  if ( !sHelp )
  {
    sHelp = new QgsHelp();
  }

  QSettings settings;
  QUrl helpNotFound = QUrl::fromLocalFile( QgsApplication::pkgDataPath() + "/doc/nohelp.html" );

  QString paths = settings.value( QStringLiteral( "help/helpSearchPath" ), "" ).toString();
  if ( paths.isEmpty() )
  {
    return helpNotFound;
  }

  QString qgisLocale;
  bool overrideLocale = settings.value( QStringLiteral( "locale/overrideFlag" ), false ).toBool();
  if ( overrideLocale )
  {
    qgisLocale = settings.value( QStringLiteral( "locale/userLocale" ), "" ).toString();
  }
  else
  {
    qgisLocale = QLocale::system().name().left( 2 );
  }

  QString qgisVersion;
  if ( Qgis::QGIS_VERSION_INT / 100 % 100 == 99 )
  {
    qgisVersion = QStringLiteral( "testing" );
    qgisLocale = QStringLiteral( "en" );
  }
  else
  {
    qgisVersion = QStringLiteral( "%1.%2" ).arg( Qgis::QGIS_VERSION_INT / 10000 ).arg( Qgis::QGIS_VERSION_INT / 100 % 100 );
  }

  QString suffix = QStringLiteral( "%1/%2/docs/user_manual/%3" ).arg( qgisVersion ).arg( qgisLocale ).arg( key );

  QUrl myUrl;
  QString helpPath;
  bool helpFound = false;

  QStringList pathList = paths.split( '|' );
  QStringList::const_iterator pathIt = pathList.constBegin();
  for ( ; pathIt != pathList.constEnd(); ++pathIt )
  {
    helpPath = QStringLiteral( "%1/%2" ).arg( *pathIt ).arg( suffix );

    if (( *pathIt ).startsWith( QStringLiteral( "http://" ) ) )
    {
      if ( !sHelp->urlExists( helpPath ) )
      {
        continue;
      }
      myUrl = QUrl( helpPath );
    }
    else
    {
      if ( !QFileInfo( helpPath.mid( 0, helpPath.lastIndexOf( "#" ) ) ).exists() )
      {
        continue;
      }
      myUrl = QUrl::fromLocalFile( helpPath );
      myUrl.setFragment( helpPath.mid( helpPath.lastIndexOf( "#" ), -1 ) );
    }

    helpFound = true;
    break;
  }

  return helpFound ? myUrl : helpNotFound;
}


QgsHelp::QgsHelp()
{
}

QgsHelp::~QgsHelp()
{
}

bool QgsHelp::urlExists( const QString& url ) const
{
  QUrl myUrl( url );
  QTcpSocket socket;

  socket.connectToHost( myUrl.host(), 80 );
  if ( socket.waitForConnected() )
  {
    socket.write( "HEAD " + myUrl.path().toUtf8() + " HTTP/1.1\r\n"
                  "Host: " + myUrl.host().toUtf8() + "\r\n\r\n" );
    if ( socket.waitForReadyRead() )
    {
      QByteArray bytes = socket.readAll();
      if ( bytes.contains( "200 OK" ) ||  bytes.contains( "302 Found" ) )
      {
        return true;
      }
    }
  }

  return false;
}
