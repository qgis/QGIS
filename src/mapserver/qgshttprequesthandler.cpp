/***************************************************************************
                              qgshttprequesthandler.cpp
                              -------------------------
  begin                : June 29, 2007
  copyright            : (C) 2007 by Marco Hugentobler
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

#include "qgshttprequesthandler.h"
#include <QByteArray>
#ifdef WIN32 //don't use fast cgi on windows
#include <stdio.h>
#else
#include <fcgi_stdio.h>
#endif //WIN32

QgsHttpRequestHandler::QgsHttpRequestHandler(): QgsRequestHandler()
{

}

QgsHttpRequestHandler::~QgsHttpRequestHandler()
{

}

void QgsHttpRequestHandler::sendHttpResponse( QByteArray* ba, const QString& format ) const
{
  if ( !ba )
  {
    return;
  }

  if ( ba->size() < 1 )
  {
    return;
  }

  printf( "Content-Type: " );
  printf( format.toLocal8Bit() );
  printf( "\n" );
  printf( "Content-Length: %d\n", ba->size() );
  printf( "\n" );
#ifdef WIN32
  fwrite( ba->data(), ba->size(), 1, stdout );
#else
  fwrite( ba->data(), ba->size(), 1, FCGI_stdout );
#endif //WIN32
}
