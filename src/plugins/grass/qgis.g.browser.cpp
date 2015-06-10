/***************************************************************************
    qgis.g.browser.cpp
    ---------------------
    begin                : February 2010
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <QUrl>
#include <QDesktopServices>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <unistd.h>
#endif

// Open a URL by default browser
int main( int argc, char **argv )
{
  if ( argc < 2 )
  {
    fprintf( stderr, "URL argument missing\n" );
    exit( 1 );
  }
  QString urlStr( argv[1] );
  QUrl url( urlStr );
#ifdef Q_OS_WIN
  // openUrl on windows fails to open 'file://c:...' it must be 'file:///c:...' (3 slashes)
  if ( url.scheme() == "file" )
  {
    // this does not work, the drive was already removed by QT:
    //url.setPath ( "/" + url.path() );
    urlStr.replace( "file://", "file:///" );
    url.setUrl( urlStr );
    std::cout << "path reset to: " << qPrintable( url.path() ) << std::endl;
  }
#endif
  QDesktopServices::openUrl( url );
#ifdef Q_OS_WIN
  Sleep( 1000 );
#else
  sleep( 1 ); // not nice but if it exits immediately the page sometimes does not open
#endif
  exit( 0 );
}
