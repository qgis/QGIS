/***************************************************************************
    qgssensorwidget.cpp
    ---------------------
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

#include "qgsconfig.h"

#include "qgssensorwidget.h"
#include "qgsiodevicesensor.h"

#if defined( HAVE_QTSERIALPORT )
#include <QSerialPort>
#include <QSerialPortInfo>
#endif

QgsAbstractSensorWidget::QgsAbstractSensorWidget( QWidget *parent SIP_TRANSFERTHIS )
  : QWidget( parent )
{
}

// ------------------------

///@cond PRIVATE

QgsTcpSocketSensorWidget::QgsTcpSocketSensorWidget( QWidget *parent )
  : QgsAbstractSensorWidget( parent )
{
  setupUi( this );

  connect( mHostNameLineEdit, &QLineEdit::textChanged, this, &QgsAbstractSensorWidget::changed );
  connect( mPortSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsAbstractSensorWidget::changed );
}

QgsAbstractSensor *QgsTcpSocketSensorWidget::createSensor()
{
  QgsTcpSocketSensor *s = new QgsTcpSocketSensor();
  s->setHostName( mHostNameLineEdit->text() );
  s->setPort( mPortSpinBox->value() );
  return s;
}

bool QgsTcpSocketSensorWidget::updateSensor( QgsAbstractSensor *sensor )
{
  QgsTcpSocketSensor *s = dynamic_cast<QgsTcpSocketSensor *>( sensor );
  if ( !s )
    return false;

  s->setHostName( mHostNameLineEdit->text() );
  s->setPort( mPortSpinBox->value() );

  return true;
}

bool QgsTcpSocketSensorWidget::setSensor( QgsAbstractSensor *sensor )
{
  QgsTcpSocketSensor *ts = dynamic_cast<QgsTcpSocketSensor *>( sensor );
  if ( ts )
  {
    mHostNameLineEdit->setText( ts->hostName() );
    mPortSpinBox->setValue( ts->port() );
    return true;
  }

  QgsUdpSocketSensor *us = dynamic_cast<QgsUdpSocketSensor *>( sensor );
  if ( us )
  {
    mHostNameLineEdit->setText( us->hostName() );
    mPortSpinBox->setValue( us->port() );
    return true;
  }

  return false;
}

// ------------------------

QgsUdpSocketSensorWidget::QgsUdpSocketSensorWidget( QWidget *parent )
  : QgsAbstractSensorWidget( parent )
{
  setupUi( this );

  connect( mHostNameLineEdit, &QLineEdit::textChanged, this, &QgsAbstractSensorWidget::changed );
  connect( mPortSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsAbstractSensorWidget::changed );
}

QgsAbstractSensor *QgsUdpSocketSensorWidget::createSensor()
{
  QgsUdpSocketSensor *s = new QgsUdpSocketSensor();
  s->setHostName( mHostNameLineEdit->text() );
  s->setPort( mPortSpinBox->value() );
  return s;
}

bool QgsUdpSocketSensorWidget::updateSensor( QgsAbstractSensor *sensor )
{
  QgsUdpSocketSensor *s = dynamic_cast<QgsUdpSocketSensor *>( sensor );
  if ( !s )
    return false;

  s->setHostName( mHostNameLineEdit->text() );
  s->setPort( mPortSpinBox->value() );

  return true;
}

bool QgsUdpSocketSensorWidget::setSensor( QgsAbstractSensor *sensor )
{
  QgsTcpSocketSensor *ts = dynamic_cast<QgsTcpSocketSensor *>( sensor );
  if ( ts )
  {
    mHostNameLineEdit->setText( ts->hostName() );
    mPortSpinBox->setValue( ts->port() );
    return true;
  }

  QgsUdpSocketSensor *us = dynamic_cast<QgsUdpSocketSensor *>( sensor );
  if ( us )
  {
    mHostNameLineEdit->setText( us->hostName() );
    mPortSpinBox->setValue( us->port() );
    return true;
  }

  return false;
}

// ------------------------

#if defined( HAVE_QTSERIALPORT )
QgsSerialPortSensorWidget::QgsSerialPortSensorWidget( QWidget *parent )
  : QgsAbstractSensorWidget( parent )
{
  setupUi( this );

  for ( const QSerialPortInfo &info : QSerialPortInfo::availablePorts() )
  {
    mSerialPortComboBox->addItem( QStringLiteral( "%1: %2" ).arg( info.portName(), info.description() ), info.portName() );
  }

  updateSerialPortDetails();

  connect( mSerialPortComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]()
  {
    updateSerialPortDetails();
    emit changed();
  } );

  connect( mBaudrateComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]()
  {
    updateSerialPortDetails();
    emit changed();
  } );

}

QgsAbstractSensor *QgsSerialPortSensorWidget::createSensor()
{
  QgsSerialPortSensor *s = new QgsSerialPortSensor();
  s->setPortName( mSerialPortComboBox->currentData().toString() );
  s->setBaudrate( mBaudrateComboBox->currentText() );
  return s;
}

bool QgsSerialPortSensorWidget::updateSensor( QgsAbstractSensor *sensor )
{
  QgsSerialPortSensor *s = dynamic_cast<QgsSerialPortSensor *>( sensor );
  if ( !s )
    return false;

  s->setPortName( mSerialPortComboBox->currentData().toString() );
  s->setBaudrate( mBaudrateComboBox->currentText() );

  return true;
}

bool QgsSerialPortSensorWidget::setSensor( QgsAbstractSensor *sensor )
{
  QgsSerialPortSensor *s = dynamic_cast<QgsSerialPortSensor *>( sensor );
  if ( !s )
    return false;

  const int index = mSerialPortComboBox->findData( s->portName() );
  if ( index >= 0 )
  {
    mSerialPortComboBox->setCurrentIndex( index );
  }
  else
  {
    mSerialPortComboBox->addItem( s->portName(), s->portName() );
    mSerialPortComboBox->setCurrentIndex( mSerialPortComboBox->count() - 1 );
  }

  const int baudRateIndex = mBaudrateComboBox->findText( s->baudrate() );
  if ( index >= 0 )
  {
    mBaudrateComboBox->setCurrentIndex( baudRateIndex );
  }
  else
  {
    mBaudrateComboBox->setCurrentIndex( mBaudrateComboBox->count() - 1 );
  }


  return true;
}

void QgsSerialPortSensorWidget::updateSerialPortDetails()
{
  if ( mSerialPortComboBox->currentIndex() < 0 )
  {
    return;
  }

  const QString &currentPortName = mSerialPortComboBox->currentData().toString();
  bool serialPortFound = false;
  for ( const QSerialPortInfo &info : QSerialPortInfo::availablePorts() )
  {
    serialPortFound = info.portName() == currentPortName;
    if ( serialPortFound )
    {
      mSerialPortDetails->setText( QStringLiteral( "%1:\n- %2: %3\n- %4: %5\n- %6: %7\n- %8: %9\n- %10: %11" ).arg( tr( "Serial port details" ),
                                   tr( "Port name" ), info.portName(),
                                   tr( "Description" ), info.description(),
                                   tr( "Manufacturer" ), info.manufacturer(),
                                   tr( "Product identifier" ), QString::number( info.productIdentifier() ),
                                   tr( "Serial number" ), info.serialNumber() ) );
      break;
    }
  }
  if ( !serialPortFound )
  {
    mSerialPortDetails->setText( QStringLiteral( "%1:\n- %2: %3" ).arg( tr( "Serial port details" ),
                                 tr( "Port name" ), currentPortName ) );
  }
}
#endif

///@endcond
