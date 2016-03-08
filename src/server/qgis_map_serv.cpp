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

#include <fcgi_stdio.h>

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

int main( int argc, char * argv[] )
{
  QgsServer server( argc, argv );
  // Starts FCGI loop
  while ( fcgi_accept() >= 0 )
  {
    server.handleRequest();
  }
  return 0;
}

