/***************************************************************************
    qgssensortablewidget.h
    ---------------------------------
    begin                : March 2023
    copyright            : (C) 2023 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensortablewidget.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgssensormanager.h"
#include "qgssensormodel.h"
#include "qgsproject.h"

#include <QTableWidget>

QgsSensorTableWidget::QgsSensorTableWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  setPanelTitle( tr( "Sensors List" ) );
  setObjectName( QStringLiteral( "SensorsList" ) );

  mActionConnection->setEnabled( false );

  mSensorModel = new QgsSensorModel( QgsProject::instance()->sensorManager(), this );

  mSensorTable->setModel( mSensorModel );
  mSensorTable->horizontalHeader()->setSectionResizeMode( static_cast<int>( QgsSensorModel::Column::Name ), QHeaderView::Stretch );
  mSensorTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  mSensorTable->setSelectionMode( QAbstractItemView::SingleSelection );

  connect( QgsProject::instance()->sensorManager(), &QgsSensorManager::sensorStatusChanged, this, [ = ]( const QString & id )
  {
    const QModelIndex index = mSensorTable->currentIndex();
    if ( index.isValid() )
    {
      if ( id == mSensorModel->data( index, QgsSensorModel::SensorId ).toString() )
      {
        QgsAbstractSensor *sensor = mSensorModel->data( index, QgsSensorModel::Sensor ).value<QgsAbstractSensor *>();
        if ( sensor )
        {
          if ( sensor->status() == Qgis::DeviceConnectionStatus::Disconnected )
          {
            mActionConnection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionStart.svg" ) ) );
          }
          else
          {
            mActionConnection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionStop.svg" ) ) );
          }
        }
      }
    }
  } );

  connect( mSensorTable->selectionModel(), &QItemSelectionModel::currentChanged, this, [ = ]( const QModelIndex & current, const QModelIndex & )
  {
    mActionConnection->setEnabled( current.isValid() );
    if ( current.isValid() && mSensorModel->data( current, QgsSensorModel::SensorStatus ).value<Qgis::DeviceConnectionStatus>() == Qgis::DeviceConnectionStatus::Connected )
    {
      mActionConnection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionStop.svg" ) ) );
    }
    else
    {
      mActionConnection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionStart.svg" ) ) );
    }
  } );

  connect( mActionConnection, &QToolButton::clicked, this, [ = ]()
  {
    const QModelIndex index = mSensorTable->currentIndex();
    if ( index.isValid() )
    {
      QgsAbstractSensor *sensor = mSensorModel->data( index, QgsSensorModel::Sensor ).value<QgsAbstractSensor *>();
      if ( sensor )
      {
        if ( sensor->status() == Qgis::DeviceConnectionStatus::Disconnected )
        {
          sensor->connectSensor();
        }
        else
        {
          sensor->disconnectSensor();
        }
      }
    }
  } );
}
