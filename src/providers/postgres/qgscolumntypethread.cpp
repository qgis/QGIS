/***************************************************************************
 qgscolumntypethread.cpp - lookup postgres geometry type and srid in a thread
                              -------------------
begin                : 3.1.2012
copyright            : (C) 2012 by Juergen E. Fischer
email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolumntypethread.h"

#include <QMetaType>

QgsGeomColumnTypeThread::QgsGeomColumnTypeThread( QgsPostgresConn *conn, bool useEstimatedMetaData )
    : QThread()
    , mConn( conn )
    , mUseEstimatedMetadata( useEstimatedMetaData )
{
  qRegisterMetaType<QgsPostgresLayerProperty>( "QgsPostgresLayerProperty" );
}

void QgsGeomColumnTypeThread::addGeometryColumn( QgsPostgresLayerProperty layerProperty )
{
  layerProperties << layerProperty;
}

void QgsGeomColumnTypeThread::stop()
{
  mConn->cancel();
  mStopped = true;
}

void QgsGeomColumnTypeThread::run()
{
  if ( !mConn )
    return;

  mStopped = false;

  foreach ( QgsPostgresLayerProperty layerProperty, layerProperties )
  {
    if ( !mStopped )
    {
      mConn->retrieveLayerTypes( layerProperty, mUseEstimatedMetadata );
    }

    if ( mStopped )
    {
      layerProperty.type = "";
      layerProperty.srid = "";
    }

    // Now tell the layer list dialog box...
    emit setLayerType( layerProperty );
  }

  mConn->disconnect();
  mConn = 0;
}
