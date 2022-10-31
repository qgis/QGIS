/***************************************************************************
    qgsgpstoolbar.cpp
    -------------------
    begin                : October 2022
    copyright            : (C) 2022 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgpstoolbar.h"
#include "qgsappgpsconnection.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgis.h"

QgsGpsToolBar::QgsGpsToolBar( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QWidget *parent )
  : QToolBar( parent )
  , mConnection( connection )
  , mCanvas( canvas )
{
  setObjectName( QStringLiteral( "mGpsToolBar" ) );
  setWindowTitle( tr( "GPS Toolbar" ) );
  setToolTip( tr( "GPS Toolbar" ) );

  mWgs84CRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );

  mConnectAction = new QAction( tr( "Connect" ), this );
  mConnectAction->setCheckable( true );
  addAction( mConnectAction );

  connect( mConnection, &QgsAppGpsConnection::statusChanged, this, [ = ]( Qgis::GpsConnectionStatus status )
  {
    switch ( status )
    {
      case Qgis::GpsConnectionStatus::Disconnected:
        whileBlocking( mConnectAction )->setChecked( false );
        mConnectAction->setText( tr( "Connect" ) );
        mConnectAction->setEnabled( true );
        mRecenterAction->setEnabled( false );
        break;
      case Qgis::GpsConnectionStatus::Connecting:
        whileBlocking( mConnectAction )->setChecked( true );
        mConnectAction->setText( tr( "Connecting" ) );
        mConnectAction->setEnabled( false );
        mRecenterAction->setEnabled( false );
        break;
      case Qgis::GpsConnectionStatus::Connected:
        whileBlocking( mConnectAction )->setChecked( true );
        mConnectAction->setText( tr( "Disconnect" ) );
        mConnectAction->setEnabled( true );
        mRecenterAction->setEnabled( true );
        break;
    }
  } );

  connect( mConnectAction, &QAction::toggled, this, [ = ]( bool connect )
  {
    if ( connect )
      mConnection->connectGps();
    else
      mConnection->disconnectGps();
  } );

  mRecenterAction = new QAction( tr( "Recenter" ) );
  mRecenterAction->setToolTip( tr( "Recenter map on GPS location" ) );
  mRecenterAction->setEnabled( false );

  connect( mRecenterAction, &QAction::triggered, this, [ = ]
  {
    if ( mConnection->lastValidLocation().isEmpty() )
      return;

    const QgsCoordinateTransform canvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
    try
    {
      const QgsPointXY center = canvasToWgs84Transform.transform( mConnection->lastValidLocation(), Qgis::TransformDirection::Reverse );
      mCanvas->setCenter( center );
      mCanvas->refresh();
    }
    catch ( QgsCsException & )
    {

    }
  } );
  addAction( mRecenterAction );
}
