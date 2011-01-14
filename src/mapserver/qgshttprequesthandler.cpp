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
#include <fcgi_stdio.h>

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
  fwrite( ba->data(), ba->size(), 1, FCGI_stdout );
}

QString QgsHttpRequestHandler::formatToMimeType( const QString& format ) const
{
  if ( format.compare( "png", Qt::CaseInsensitive ) )
  {
    return "image/png";
  }
  else if ( format.compare( "jpg", Qt::CaseInsensitive ) )
  {
    return "image/jpeg";
  }
  else if ( format.compare( "svg", Qt::CaseInsensitive ) )
  {
    return "image/svg+xml";
  }
  else if ( format.compare( "pdf", Qt::CaseInsensitive ) )
  {
    return "application/pdf";
  }
  return format;
}
