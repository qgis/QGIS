/***************************************************************************
 qgscolumntypethread.cpp - lookup oracle geometry type and srid in a thread
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

#include "qgsoraclecolumntypethread.h"

#include <QMetaType>

QgsOracleColumnTypeThread::QgsOracleColumnTypeThread( QgsOracleConn *conn, bool useEstimatedMetaData )
    : QThread()
    , mConn( conn )
    , mUseEstimatedMetadata( useEstimatedMetaData )
{
  qRegisterMetaType<QgsOracleLayerProperty>( "QgsOracleLayerProperty" );
}

void QgsOracleColumnTypeThread::addGeometryColumn( QgsOracleLayerProperty layerProperty )
{
  layerProperties << layerProperty;
}

void QgsOracleColumnTypeThread::stop()
{
  mStopped = true;
}

void QgsOracleColumnTypeThread::run()
{
  if ( !mConn )
    return;

  mStopped = false;

  foreach ( QgsOracleLayerProperty layerProperty, layerProperties )
  {
    if ( !mStopped )
    {
      mConn->retrieveLayerTypes( layerProperty, mUseEstimatedMetadata );
    }

    if ( mStopped )
    {
      layerProperty.types.clear();
      layerProperty.srids.clear();
    }

    // Now tell the layer list dialog box...
    emit setLayerType( layerProperty );
  }

  mConn->disconnect();
  mConn = 0;
}
