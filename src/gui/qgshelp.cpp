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

void QgsHelp::openHelp( const QString& key )
{
  QDesktopServices::openUrl( QgsHelp::helpUrl( key ) );
}

QUrl QgsHelp::helpUrl( const QString& key )
{
  QUrl helpNotFound = QUrl::fromLocalFile( QgsApplication::pkgDataPath() + "/doc/nohelp.html" );

  QSettings settings;
  QStringList paths = settings.value( QStringLiteral( "help/helpSearchPath" ) ).toStringList();
  if ( paths.isEmpty() )
  {
    return helpNotFound;
  }

  QString qgisLocale;
  bool overrideLocale = settings.value( QStringLiteral( "locale/overrideFlag" ), false ).toBool();
  if ( overrideLocale )
  {
    qgisLocale = settings.value( QStringLiteral( "locale/userLocale" ), QString() ).toString();
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

  QUrl helpUrl;
  QString helpPath;
  bool helpFound = false;

  Q_FOREACH ( const QString& path, paths )
  {
    helpPath = QStringLiteral( "%1/%2" ).arg( path ).arg( suffix );

    if ( path.startsWith( QStringLiteral( "http" ) ) )
    {
      if ( !QgsHelp::urlExists( helpPath ) )
      {
        continue;
      }
      helpUrl = QUrl( helpPath );
    }
    else
    {
      QString filePath = helpPath.mid( 0, helpPath.lastIndexOf( "#" ) );
      if ( !QFileInfo::exists( filePath ) )
      {
        continue;
      }
      helpUrl = QUrl::fromLocalFile( filePath );
      helpUrl.setFragment( helpPath.mid( helpPath.lastIndexOf( "#" ) + 1, -1 ) );
    }

    helpFound = true;
    break;
  }

  return helpFound ? helpUrl : helpNotFound;
}

bool QgsHelp::urlExists( const QString& url )
{
  QUrl helpUrl( url );
  QTcpSocket socket;

  socket.connectToHost( helpUrl.host(), 80 );
  if ( socket.waitForConnected() )
  {
    socket.write( "HEAD " + helpUrl.path().toUtf8() + " HTTP/1.1\r\n"
                  "Host: " + helpUrl.host().toUtf8() + "\r\n\r\n" );
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
