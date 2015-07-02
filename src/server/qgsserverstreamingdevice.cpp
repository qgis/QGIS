/***************************************************************************
                        qgsserverstreamingdevice.cpp
  -------------------------------------------------------------------
Date                 : 25 May 2015
Copyright            : (C) 2015 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverstreamingdevice.h"
#include "qgsrequesthandler.h"

QgsServerStreamingDevice::QgsServerStreamingDevice( const QString& formatName, QgsRequestHandler* rh, QObject* parent ): QIODevice( parent ), mFormatName( formatName ), mRequestHandler( rh )
{
}

QgsServerStreamingDevice::QgsServerStreamingDevice(): QIODevice( 0 ), mRequestHandler( 0 )
{

}

QgsServerStreamingDevice::~QgsServerStreamingDevice()
{
}

bool QgsServerStreamingDevice::open( OpenMode mode )
{
  if ( !mRequestHandler || mode != QIODevice::WriteOnly )
  {
    return false;
  }

  mRequestHandler->setHeader( "Content-Type", mFormatName );
  mRequestHandler->sendResponse();
  return QIODevice::open( mode );
}

void QgsServerStreamingDevice::close()
{
  QIODevice::close();
}

qint64 QgsServerStreamingDevice::writeData( const char * data, qint64 maxSize )
{
  QByteArray ba( data, maxSize );
  mRequestHandler->setGetFeatureResponse( &ba );
  return maxSize;
}

qint64 QgsServerStreamingDevice::readData( char * data, qint64 maxSize )
{
  Q_UNUSED( data );
  Q_UNUSED( maxSize );
  return -1; //reading not supported
}
