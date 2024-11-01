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
#include "moc_qgssensortablewidget.cpp"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgsiodevicesensor.h"
#include "qgssensorguiregistry.h"
#include "qgssensormanager.h"
#include "qgssensormodel.h"
#include "qgssensorwidget.h"
#include "qgsproject.h"

#include <QDialogButtonBox>
#include <QTableWidget>


QgsSensorSettingsWidget::QgsSensorSettingsWidget( QgsAbstractSensor *sensor, QWidget *parent )
  : QgsPanelWidget( parent )
  , mSensor( sensor )
{
  setupUi( this );
  setPanelTitle( tr( "Sensor Settings" ) );
  setObjectName( QStringLiteral( "SensorSettings" ) );
  connect( this, &QgsPanelWidget::panelAccepted, this, [=]() { apply(); } );

  mNameLineEdit->setText( sensor->name() );
  connect( mNameLineEdit, &QLineEdit::textChanged, this, [=]() { mDirty = true; } );

  const QMap<QString, QString> sensorTypes = QgsGui::sensorGuiRegistry()->sensorTypes();
  for ( auto sensorIt = sensorTypes.begin(); sensorIt != sensorTypes.end(); ++sensorIt )
  {
    mTypeComboBox->addItem( QgsGui::sensorGuiRegistry()->sensorMetadata( sensorIt.key() )->creationIcon(), sensorIt.value(), sensorIt.key() );
  }
  mTypeComboBox->setCurrentIndex( mTypeComboBox->findData( sensor->type() ) );
  connect( mTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [=]() {
    mDirty = true;
    setSensorWidget();
  } );

  setSensorWidget();
}

void QgsSensorSettingsWidget::setSensorWidget()
{
  if ( mSensorWidget )
  {
    mSensorWidget->deleteLater();
    mSensorWidget = nullptr;
  }

  mSensorWidget = QgsGui::sensorGuiRegistry()->sensorMetadata( mTypeComboBox->currentData().toString() )->createSensorWidget( mSensor );
  if ( mSensorWidget )
  {
    mSensorWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    mSensorWidget->setSensor( mSensor );
    mTypeLayout->addWidget( mSensorWidget );
    connect( mSensorWidget, &QgsAbstractSensorWidget::changed, this, [=]() { mDirty = true; } );
  }
}

void QgsSensorSettingsWidget::apply()
{
  if ( mDirty && mSensorWidget && mSensor )
  {
    mSensor->disconnectSensor();
    if ( !mSensorWidget->updateSensor( mSensor ) )
    {
      // The sensor type has changed, remove sensor and add a fresh one
      QgsProject::instance()->sensorManager()->removeSensor( mSensor->id() );
      mSensor = mSensorWidget->createSensor();
      QgsProject::instance()->sensorManager()->addSensor( mSensor );
    }
    mSensor->setName( mNameLineEdit->text() );
  }
}

//----------------

QgsSensorTableWidget::QgsSensorTableWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  setPanelTitle( tr( "Sensors List" ) );
  setObjectName( QStringLiteral( "SensorsList" ) );

  mActionConnection->setEnabled( false );
  mActionRemoveSensor->setEnabled( false );
  mActionEditSensor->setEnabled( false );

  mSensorModel = new QgsSensorModel( QgsProject::instance()->sensorManager(), this );

  mSensorTable->setModel( mSensorModel );
  mSensorTable->horizontalHeader()->setSectionResizeMode( static_cast<int>( QgsSensorModel::Column::Name ), QHeaderView::Stretch );
  mSensorTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  mSensorTable->setSelectionMode( QAbstractItemView::SingleSelection );

  connect( mSensorTable, &QAbstractItemView::doubleClicked, this, [=]( const QModelIndex &index ) {
    if ( index.isValid() )
    {
      QgsSensorSettingsWidget *settingsWidget = new QgsSensorSettingsWidget( mSensorModel->data( index, static_cast<int>( QgsSensorModel::CustomRole::Sensor ) ).value<QgsAbstractSensor *>(), this );
      showPanel( settingsWidget );
    }
  } );

  connect( QgsProject::instance()->sensorManager(), &QgsSensorManager::sensorStatusChanged, this, [=]( const QString &id ) {
    const QModelIndex index = mSensorTable->currentIndex();
    if ( index.isValid() )
    {
      if ( id == mSensorModel->data( index, static_cast<int>( QgsSensorModel::CustomRole::SensorId ) ).toString() )
      {
        QgsAbstractSensor *sensor = mSensorModel->data( index, static_cast<int>( QgsSensorModel::CustomRole::Sensor ) ).value<QgsAbstractSensor *>();
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

  connect( mSensorTable->selectionModel(), &QItemSelectionModel::currentChanged, this, [=]( const QModelIndex &current, const QModelIndex & ) {
    mActionConnection->setEnabled( current.isValid() );
    mActionRemoveSensor->setEnabled( current.isValid() );
    mActionEditSensor->setEnabled( current.isValid() );
    if ( current.isValid() && mSensorModel->data( current, static_cast<int>( QgsSensorModel::CustomRole::SensorStatus ) ).value<Qgis::DeviceConnectionStatus>() == Qgis::DeviceConnectionStatus::Connected )
    {
      mActionConnection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionStop.svg" ) ) );
    }
    else
    {
      mActionConnection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionStart.svg" ) ) );
    }
  } );

  connect( mActionConnection, &QToolButton::clicked, this, [=]() {
    const QModelIndex index = mSensorTable->currentIndex();
    if ( index.isValid() )
    {
      QgsAbstractSensor *sensor = mSensorModel->data( index, static_cast<int>( QgsSensorModel::CustomRole::Sensor ) ).value<QgsAbstractSensor *>();
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

  connect( mActionAddSensor, &QToolButton::clicked, this, [=]() {
    QgsTcpSocketSensor *sensor = new QgsTcpSocketSensor();
    sensor->setName( tr( "New sensor" ) );
    QgsProject::instance()->sensorManager()->addSensor( sensor );

    QgsSensorSettingsWidget *settingsWidget = new QgsSensorSettingsWidget( sensor, this );
    showPanel( settingsWidget );
  } );

  connect( mActionRemoveSensor, &QToolButton::clicked, this, [=]() {
    const QModelIndex index = mSensorTable->currentIndex();
    if ( index.isValid() )
    {
      QgsProject::instance()->sensorManager()->removeSensor( mSensorModel->data( index, static_cast<int>( QgsSensorModel::CustomRole::SensorId ) ).toString() );
    }
  } );

  connect( mActionEditSensor, &QToolButton::clicked, this, [=]() {
    const QModelIndex index = mSensorTable->currentIndex();
    if ( index.isValid() )
    {
      QgsSensorSettingsWidget *settingsWidget = new QgsSensorSettingsWidget( mSensorModel->data( index, static_cast<int>( QgsSensorModel::CustomRole::Sensor ) ).value<QgsAbstractSensor *>(), this );
      showPanel( settingsWidget );
    }
  } );
}
