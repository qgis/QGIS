/***************************************************************************
                           qgsserverresponse.h

  Define response class for services
  -------------------
  begin                : 2016-12-05
  copyright            : (C) 2016 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverresponse.h"
#include "qgsmessagelog.h"
#include "qgsserverexception.h"


void QgsServerResponse::write( const QString &data )
{
  QIODevice *iodev = io();
  if ( iodev )
  {
    iodev->write( data.toUtf8() );
  }
  else
  {
    QgsMessageLog::logMessage( "Error: No IODevice in QgsServerResponse !!!" );
  }
}

qint64 QgsServerResponse::write( const QByteArray &byteArray )
{
  QIODevice *iodev = io();
  if ( iodev )
  {
    return iodev->write( byteArray );
  }
  return 0;
}

qint64 QgsServerResponse::write( const char *data, qint64 maxsize )
{
  QIODevice *iodev = io();
  if ( iodev )
  {
    return iodev->write( data, maxsize );
  }
  return 0;
}

qint64 QgsServerResponse::write( const char *data )
{
  QIODevice *iodev = io();
  if ( iodev )
  {
    return iodev->write( data );
  }
  return 0;
}

void QgsServerResponse::finish()
{

}

void QgsServerResponse::flush()
{

}

qint64 QgsServerResponse::write( const std::string data )
{
  return write( data.c_str() );
}

void QgsServerResponse::write( const QgsServerException &ex )
{
  QString responseFormat;
  const QByteArray ba = ex.formatResponse( responseFormat );

  if ( headersSent() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Error: Cannot write exception after header sent !" ) );
    return;
  }

  clear();
  setStatusCode( ex.responseCode() );
  setHeader( "Content-Type", responseFormat );
  write( ba );
}
