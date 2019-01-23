/***************************************************************************
                              qgs_map_serv.cpp
 A server application supporting WMS / WFS / WCS
                              -------------------
  begin                : July 04, 2006
  copyright            : (C) 2006 by Marco Hugentobler & Ionut Iosifescu Enescu
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//for CMAKE_INSTALL_PREFIX
#include "qgsconfig.h"
#include "qgsserver.h"
#include "qgsfcgiserverresponse.h"
#include "qgsfcgiserverrequest.h"
#include "qgsapplication.h"

#include <fcgi_stdio.h>
#include <cstdlib>

#include <QString>

int fcgi_accept()
{
#ifdef Q_OS_WIN
  if ( FCGX_IsCGI() )
    return FCGI_Accept();
  else
    return FCGX_Accept( &FCGI_stdin->fcgx_stream, &FCGI_stdout->fcgx_stream, &FCGI_stderr->fcgx_stream, &environ );
#else
  return FCGI_Accept();
#endif
}

int main( int argc, char *argv[] )
{
  // Test if the environ variable DISPLAY is defined
  // if it's not, the server is running in offscreen mode
  // Qt supports using various QPA (Qt Platform Abstraction) back ends
  // for rendering. You can specify the back end to use with the environment
  // variable QT_QPA_PLATFORM when invoking a Qt-based application.
  // Available platform plugins are: directfbegl, directfb, eglfs, linuxfb,
  // minimal, minimalegl, offscreen, wayland-egl, wayland, xcb.
  // https://www.ics.com/blog/qt-tips-and-tricks-part-1
  // http://doc.qt.io/qt-5/qpa.html
  const char *display = getenv( "DISPLAY" );
  bool withDisplay = true;
  if ( !display )
  {
    withDisplay = false;
    qputenv( "QT_QPA_PLATFORM", "offscreen" );
    QgsMessageLog::logMessage( "DISPLAY not set, running in offscreen mode, all printing capabilities will not be available.", "Server", Qgis::Info );
  }
  // since version 3.0 QgsServer now needs a qApp so initialize QgsApplication
  QgsApplication app( argc, argv, withDisplay, QString(), QStringLiteral( "server" ) );
  QgsServer server;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  server.initPython();
#endif
  // Starts FCGI loop
  while ( fcgi_accept() >= 0 )
  {
    QgsFcgiServerRequest  request;
    QgsFcgiServerResponse response( request.method() );
    if ( ! request.hasError() )
    {
      server.handleRequest( request, response );
    }
    else
    {
      response.sendError( 400, "Bad request" );
    }
  }
  app.exitQgis();
  return 0;
}

