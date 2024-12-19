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

  mBaudRateComboBox->addItem( QStringLiteral( "1200 baud" ), static_cast<int>( QSerialPort::Baud1200 ) );
  mBaudRateComboBox->addItem( QStringLiteral( "2400 baud" ), static_cast<int>( QSerialPort::Baud2400 ) );
  mBaudRateComboBox->addItem( QStringLiteral( "4800 baud" ), static_cast<int>( QSerialPort::Baud4800 ) );
  mBaudRateComboBox->addItem( QStringLiteral( "9600 baud" ), static_cast<int>( QSerialPort::Baud9600 ) );
  mBaudRateComboBox->addItem( QStringLiteral( "19200 baud" ), static_cast<int>( QSerialPort::Baud19200 ) );
  mBaudRateComboBox->addItem( QStringLiteral( "38400 baud" ), static_cast<int>( QSerialPort::Baud38400 ) );
  mBaudRateComboBox->addItem( QStringLiteral( "57600 baud" ), static_cast<int>( QSerialPort::Baud57600 ) );
  mBaudRateComboBox->addItem( QStringLiteral( "115200 baud" ), static_cast<int>( QSerialPort::Baud115200 ) );
  mBaudRateComboBox->setCurrentIndex( 3 );

  mDataFrameDelimiterComboBox->addItem( tr( "No Delimiter" ), QString() );
  mDataFrameDelimiterComboBox->addItem( tr( "New Line" ), QString( "\n" ) );
  mDataFrameDelimiterComboBox->addItem( tr( "Custom Character" ), QString() );

  updateSerialPortDetails();

  connect( mSerialPortComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentTextChanged ), this, [ = ]()
  {
    updateSerialPortDetails();
    emit changed();
  } );

  connect( mBaudRateComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]()
  {
    updateSerialPortDetails();
    emit changed();
  } );

  connect( mDataFrameDelimiterComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]( int index )
  {
    if ( index == mDataFrameDelimiterComboBox->count() - 1 )
    {
      mDataFrameDelimiterLineEdit->setEnabled( true );
      mDataFrameDelimiterLineEdit->setFocus();
    }
    else
    {
      mDataFrameDelimiterLineEdit->setEnabled( false );
    }
    emit changed();
  } );

  connect( mDataFrameDelimiterLineEdit, &QLineEdit::textEdited, this, [ = ]()
  {
    emit changed();
  } );
}

QgsAbstractSensor *QgsSerialPortSensorWidget::createSensor()
{
  QgsSerialPortSensor *s = new QgsSerialPortSensor();
  s->setPortName( mSerialPortComboBox->findText( mSerialPortComboBox->currentText() ) != -1 ? mSerialPortComboBox->currentData().toString() : mSerialPortComboBox->currentText() );
  s->setBaudRate( static_cast< QSerialPort::BaudRate >( mBaudRateComboBox->currentData().toInt() ) );
  const QString delimiter = mDataFrameDelimiterComboBox->currentIndex() == mDataFrameDelimiterComboBox->count() - 1 ? mDataFrameDelimiterLineEdit->text() : mDataFrameDelimiterComboBox->currentData().toString();
  s->setDelimiter( delimiter.toLocal8Bit() );
  return s;
}

bool QgsSerialPortSensorWidget::updateSensor( QgsAbstractSensor *sensor )
{
  QgsSerialPortSensor *s = dynamic_cast<QgsSerialPortSensor *>( sensor );
  if ( !s )
    return false;

  s->setPortName( mSerialPortComboBox->findText( mSerialPortComboBox->currentText() ) != -1 ? mSerialPortComboBox->currentData().toString() : mSerialPortComboBox->currentText() );
  s->setBaudRate( static_cast< QSerialPort::BaudRate >( mBaudRateComboBox->currentData().toInt() ) );
  const QString delimiter = mDataFrameDelimiterComboBox->currentIndex() == mDataFrameDelimiterComboBox->count() - 1 ? mDataFrameDelimiterLineEdit->text() : mDataFrameDelimiterComboBox->currentData().toString();
  s->setDelimiter( delimiter.toLocal8Bit() );
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

  const int baudRateIndex = mBaudRateComboBox->findData( s->baudRate() );
  if ( index >= 0 )
  {
    mBaudRateComboBox->setCurrentIndex( baudRateIndex );
  }
  else
  {
    mBaudRateComboBox->setCurrentIndex( mBaudRateComboBox->count() - 1 );
  }

  const QString delimiter = QString( s->delimiter() );
  if ( !delimiter.isEmpty() )
  {
    const int delimiterIndex = mDataFrameDelimiterComboBox->findData( delimiter );
    if ( delimiterIndex > -1 )
    {
      mDataFrameDelimiterComboBox->setCurrentIndex( delimiterIndex );
    }
    else
    {
      mDataFrameDelimiterComboBox->setCurrentIndex( mDataFrameDelimiterComboBox->count() - 1 );
      mDataFrameDelimiterLineEdit->setText( delimiter );
    }
  }
  else
  {
    mDataFrameDelimiterComboBox->setCurrentIndex( 0 );
    mDataFrameDelimiterLineEdit->setText( QString() );
  }

  return true;
}

void QgsSerialPortSensorWidget::updateSerialPortDetails()
{
  if ( mSerialPortComboBox->currentText().isEmpty() )
  {
    return;
  }

  const QString &currentPortName = mSerialPortComboBox->findText( mSerialPortComboBox->currentText() ) != -1 ? mSerialPortComboBox->currentData().toString() : mSerialPortComboBox->currentText();
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
  else
  {
    mSerialPortDetails->setText( QString() );
  }
}
#endif

///@endcond
