/***************************************************************************
    qgsprojectsensorsettingswidget.cpp
    ---------------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectsensorsettingswidget.h"
#include "moc_qgsprojectsensorsettingswidget.cpp"

#include "qgis.h"
#include "qgsabstractsensor.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgssensormanager.h"
#include "qgssensortablewidget.h"

QgsProjectSensorSettingsWidget::QgsProjectSensorSettingsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  QgsSensorTableWidget *widget = new QgsSensorTableWidget( this );
  mPanelStack->setMainPanel( widget );
  connect( widget, &QgsPanelWidget::showPanel, this, [=]( QgsPanelWidget *panel ) {
    mSensorIntroductionLabel->setVisible( false );
    connect( panel, &QgsPanelWidget::panelAccepted, this, [=]() { mSensorIntroductionLabel->setVisible( true ); } );
  } );

  QDomElement sensorElem = QgsProject::instance()->sensorManager()->writeXml( mPreviousSensors );
  mPreviousSensors.appendChild( sensorElem );

  const QList<QgsAbstractSensor *> sensors = QgsProject::instance()->sensorManager()->sensors();
  for ( QgsAbstractSensor *sensor : sensors )
  {
    if ( sensor->status() == Qgis::DeviceConnectionStatus::Connected )
    {
      mConnectedSensors << sensor->id();
    }
  }

  connect( QgsProject::instance()->sensorManager(), &QgsSensorManager::sensorErrorOccurred, this, [=]( const QString &id ) {
    if ( QgsAbstractSensor *sensor = QgsProject::instance()->sensorManager()->sensor( id ) )
    {
      mMessageBar->pushCritical( tr( "Sensor Error" ), QStringLiteral( "%1: %2" ).arg( sensor->name(), sensor->errorString() ) );
    }
  } );
}

void QgsProjectSensorSettingsWidget::cancel()
{
  // Capture connected state of current sensors even if we're about to revert as someone might have
  // activated a sensor then closed the dialog using the window bar's close button
  QList<QgsAbstractSensor *> sensors = QgsProject::instance()->sensorManager()->sensors();
  for ( QgsAbstractSensor *sensor : sensors )
  {
    if ( sensor->status() == Qgis::DeviceConnectionStatus::Connected )
    {
      mConnectedSensors << sensor->id();
    }
  }

  QgsProject::instance()->sensorManager()->clear();
  QgsProject::instance()->sensorManager()->readXml( mPreviousSensors.documentElement(), mPreviousSensors );

  sensors = QgsProject::instance()->sensorManager()->sensors();
  for ( QgsAbstractSensor *sensor : sensors )
  {
    if ( mConnectedSensors.contains( sensor->id() ) )
    {
      sensor->connectSensor();
    }
  }
}

void QgsProjectSensorSettingsWidget::apply()
{
  // If a sensor settings panel is open, apply changes
  if ( QgsSensorSettingsWidget *widget = dynamic_cast<QgsSensorSettingsWidget *>( mPanelStack->currentPanel() ) )
  {
    widget->apply();
  }

  mPreviousSensors = QDomDocument();
  QDomElement sensorElem = QgsProject::instance()->sensorManager()->writeXml( mPreviousSensors );
  mPreviousSensors.appendChild( sensorElem );

  mConnectedSensors.clear();
  const QList<QgsAbstractSensor *> sensors = QgsProject::instance()->sensorManager()->sensors();
  for ( QgsAbstractSensor *sensor : sensors )
  {
    if ( sensor->status() == Qgis::DeviceConnectionStatus::Connected )
    {
      mConnectedSensors << sensor->id();
    }
  }

  return;
}

bool QgsProjectSensorSettingsWidget::isValid()
{
  return true;
}


//
// QgsProjectSensorSettingsWidgetFactory
//

QgsProjectSensorSettingsWidgetFactory::QgsProjectSensorSettingsWidgetFactory( QObject *parent )
  : QgsOptionsWidgetFactory( tr( "Sensors" ), QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/sensor.svg" ) ), QStringLiteral( "sensor" ) )
{
  setParent( parent );
}


QgsOptionsPageWidget *QgsProjectSensorSettingsWidgetFactory::createWidget( QWidget *parent ) const
{
  return new QgsProjectSensorSettingsWidget( parent );
}
