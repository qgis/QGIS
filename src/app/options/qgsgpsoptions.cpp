/***************************************************************************
    qgsgpsoptions.cpp
    -----------------
    begin                : July 2022
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

#include "qgsgpsoptions.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsgpsmarker.h"
#include "qgsmarkersymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsgpsdetector.h"
#include "qgsgpsconnection.h"
#include "qgsgpsinformationwidget.h"
#include "qgslinesymbol.h"

const int MAXACQUISITIONINTERVAL = 3000; // max gps information acquisition suspension interval (in seconds)
const int MAXDISTANCETHRESHOLD = 200; // max gps distance threshold (in meters)

//
// QgsGpsOptionsWidget
//

QgsGpsOptionsWidget::QgsGpsOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  connect( mRadAutodetect, &QRadioButton::toggled, mCboDevices, &QWidget::setDisabled );
  connect( mRadAutodetect, &QRadioButton::toggled, mGpsdHost, &QWidget::setDisabled );
  connect( mRadAutodetect, &QRadioButton::toggled, mSpinGpsdPort, &QWidget::setDisabled );
  connect( mRadAutodetect, &QRadioButton::toggled, mGpsdDevice, &QWidget::setDisabled );
  connect( mRadSerialDevice, &QRadioButton::toggled, mCboDevices, &QWidget::setEnabled );
  connect( mRadSerialDevice, &QRadioButton::toggled, mGpsdHost, &QWidget::setDisabled );
  connect( mRadSerialDevice, &QRadioButton::toggled, mSpinGpsdPort, &QWidget::setDisabled );
  connect( mRadSerialDevice, &QRadioButton::toggled, mGpsdDevice, &QWidget::setDisabled );
  connect( mRadSerialDevice, &QRadioButton::toggled, mBtnRefreshDevices, &QWidget::setEnabled );
  connect( mRadAutodetect, &QRadioButton::toggled, mBtnRefreshDevices, &QWidget::setDisabled );
  connect( mRadGpsd, &QRadioButton::toggled, mGpsdHost, &QWidget::setEnabled );
  connect( mRadGpsd, &QRadioButton::toggled, mSpinGpsdPort, &QWidget::setEnabled );
  connect( mRadGpsd, &QRadioButton::toggled, mGpsdDevice, &QWidget::setEnabled );
  connect( mRadGpsd, &QRadioButton::toggled, mCboDevices, &QWidget::setDisabled );
  connect( mRadGpsd, &QRadioButton::toggled, mBtnRefreshDevices, &QWidget::setDisabled );
  connect( mRadInternal, &QRadioButton::toggled, mGpsdHost, &QWidget::setDisabled );
  connect( mRadInternal, &QRadioButton::toggled, mSpinGpsdPort, &QWidget::setDisabled );
  connect( mRadInternal, &QRadioButton::toggled, mGpsdDevice, &QWidget::setDisabled );
  connect( mRadInternal, &QRadioButton::toggled, mCboDevices, &QWidget::setDisabled );
  connect( mRadInternal, &QRadioButton::toggled, mBtnRefreshDevices, &QWidget::setDisabled );

  mGpsMarkerSymbolButton->setSymbolType( Qgis::SymbolType::Marker );
  mBearingLineStyleButton->setSymbolType( Qgis::SymbolType::Line );

  const QString defaultSymbol = QgsGpsMarker::settingLocationMarkerSymbol.value();
  QDomDocument symbolDoc;
  symbolDoc.setContent( defaultSymbol );
  const QDomElement markerElement = symbolDoc.documentElement();
  std::unique_ptr< QgsMarkerSymbol > gpsMarkerSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( markerElement, QgsReadWriteContext() ) );
  if ( gpsMarkerSymbol )
    mGpsMarkerSymbolButton->setSymbol( gpsMarkerSymbol.release() );


  QgsSettings settings;
  QDomDocument doc;
  QDomElement elem;
  QString bearingLineSymbolXml = QgsGpsInformationWidget::settingBearingLineSymbol.value();
  if ( bearingLineSymbolXml.isEmpty() )
  {
    bearingLineSymbolXml = settings.value( QStringLiteral( "bearingLineSymbol" ), QVariant(), QgsSettings::Gps ).toString();
  }

  if ( !bearingLineSymbolXml.isEmpty() )
  {
    doc.setContent( bearingLineSymbolXml );
    elem = doc.documentElement();
    std::unique_ptr< QgsLineSymbol > bearingSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( elem, QgsReadWriteContext() ) );
    if ( bearingSymbol )
      mBearingLineStyleButton->setSymbol( bearingSymbol.release() );
  }

  mCheckRotateLocationMarker->setChecked( QgsGpsMarker::settingRotateLocationMarker.value() );

  mSpinGpsdPort->setValue( 2947 );
  mSpinGpsdPort->setClearValue( 2947 );
  mSpinTrackWidth->setClearValue( 2 );
  mSpinMapRotateInterval->setClearValue( 0 );
  mSpinMapExtentMultiplier->setClearValue( 50 );

  mBtnTrackColor->setAllowOpacity( true );
  mBtnTrackColor->setColorDialogTitle( tr( "Track Color" ) );

  connect( mBtnRefreshDevices, &QToolButton::clicked, this, &QgsGpsOptionsWidget::refreshDevices );

  mCboAcquisitionInterval->addItem( QStringLiteral( "0" ), 0 );
  mCboAcquisitionInterval->addItem( QStringLiteral( "2" ), 2 );
  mCboAcquisitionInterval->addItem( QStringLiteral( "5" ), 5 );
  mCboAcquisitionInterval->addItem( QStringLiteral( "10" ), 10 );
  mCboAcquisitionInterval->addItem( QStringLiteral( "15" ), 15 );
  mCboAcquisitionInterval->addItem( QStringLiteral( "30" ), 30 );
  mCboAcquisitionInterval->addItem( QStringLiteral( "60" ), 60 );
  mCboDistanceThreshold->addItem( QStringLiteral( "0" ), 0 );
  mCboDistanceThreshold->addItem( QStringLiteral( "3" ), 3 );
  mCboDistanceThreshold->addItem( QStringLiteral( "5" ), 5 );
  mCboDistanceThreshold->addItem( QStringLiteral( "10" ), 10 );
  mCboDistanceThreshold->addItem( QStringLiteral( "15" ), 15 );

  mAcquisitionIntValidator = new QIntValidator( 0, MAXACQUISITIONINTERVAL, this );
  mDistanceThresholdValidator = new QIntValidator( 0, MAXDISTANCETHRESHOLD, this );

  mCboAcquisitionInterval->setValidator( mAcquisitionIntValidator );
  mCboDistanceThreshold->setValidator( mDistanceThresholdValidator );


  Qgis::GpsConnectionType connectionType = Qgis::GpsConnectionType::Automatic;
  QString gpsdHost;
  int gpsdPort = 0;
  QString gpsdDevice;
  double trackWidth = 2;
  QColor trackColor;
  int acquisitionInterval = 0;
  double distanceThreshold = 0;
  bool bearingFromTravelDirection = false;
  int recenteringThreshold = 50;
  int rotateInterval = 0;
  if ( QgsGpsConnection::settingsGpsConnectionType.exists() )
  {
    connectionType = QgsGpsConnection::settingsGpsConnectionType.value();
    gpsdHost = QgsGpsConnection::settingsGpsdHostName.value();
    gpsdPort = QgsGpsConnection::settingsGpsdPortNumber.value();
    gpsdDevice = QgsGpsConnection::settingsGpsdDeviceName.value();
    trackWidth = QgsGpsInformationWidget::settingGpsTrackWidth.value();
    trackColor = QgsGpsInformationWidget::settingGpsTrackColor.value();
    acquisitionInterval = QgsGpsConnection::settingGpsAcquisitionInterval.value();
    distanceThreshold = QgsGpsConnection::settingGpsDistanceThreshold.value();
    bearingFromTravelDirection = QgsGpsConnection::settingGpsBearingFromTravelDirection.value();
    recenteringThreshold = QgsGpsInformationWidget::settingMapExtentRecenteringThreshold.value();
    rotateInterval = QgsGpsInformationWidget::settingMapRotateInterval.value();
  }
  else
  {
    // legacy settings
    const QString portMode = settings.value( QStringLiteral( "portMode" ), "scanPorts", QgsSettings::Gps ).toString();

    if ( portMode == QLatin1String( "scanPorts" ) )
    {
      connectionType = Qgis::GpsConnectionType::Automatic;
    }
    else if ( portMode == QLatin1String( "internalGPS" ) )
    {
      connectionType = Qgis::GpsConnectionType::Internal;
    }
    else if ( portMode == QLatin1String( "explicitPort" ) )
    {
      connectionType = Qgis::GpsConnectionType::Serial;
    }
    else if ( portMode == QLatin1String( "gpsd" ) )
    {
      connectionType = Qgis::GpsConnectionType::Gpsd;
    }

    gpsdHost = settings.value( QStringLiteral( "gpsdHost" ), "localhost", QgsSettings::Gps ).toString();
    gpsdPort = settings.value( QStringLiteral( "gpsdPort" ), 2947, QgsSettings::Gps ).toInt();
    gpsdDevice = settings.value( QStringLiteral( "gpsdDevice" ), QVariant(), QgsSettings::Gps ).toString();

    trackWidth = settings.value( QStringLiteral( "trackWidth" ), "2", QgsSettings::Gps ).toInt();
    trackColor = settings.value( QStringLiteral( "trackColor" ), QColor( Qt::red ), QgsSettings::Gps ).value<QColor>();

    acquisitionInterval = settings.value( QStringLiteral( "acquisitionInterval" ), 0, QgsSettings::Gps ).toInt();
    distanceThreshold = settings.value( QStringLiteral( "distanceThreshold" ), 0, QgsSettings::Gps ).toDouble();

    bearingFromTravelDirection = settings.value( QStringLiteral( "calculateBearingFromTravel" ), "false", QgsSettings::Gps ).toBool();

    recenteringThreshold = settings.value( QStringLiteral( "mapExtentMultiplier" ), "50", QgsSettings::Gps ).toInt();
    rotateInterval = settings.value( QStringLiteral( "rotateMapInterval" ), 0, QgsSettings::Gps ).toInt();
  }

  mGpsdHost->setText( gpsdHost );
  mSpinGpsdPort->setValue( gpsdPort );
  mGpsdDevice->setText( gpsdDevice );

  switch ( connectionType )
  {
    case Qgis::GpsConnectionType::Automatic:
      mRadAutodetect->setChecked( true );
      break;
    case Qgis::GpsConnectionType::Internal:
      mRadInternal->setChecked( true );
      break;
    case Qgis::GpsConnectionType::Serial:
      mRadSerialDevice->setChecked( true );
      break;
    case Qgis::GpsConnectionType::Gpsd:
      mRadGpsd->setChecked( true );
      break;
  }

  if ( mRadInternal->isChecked() )
  {
    mRadAutodetect->setChecked( true );
  }
  mRadInternal->hide();

  mTravelBearingCheckBox->setChecked( bearingFromTravelDirection );

  mSpinTrackWidth->setValue( trackWidth );
  mBtnTrackColor->setColor( trackColor );

  mCboAcquisitionInterval->setCurrentText( QString::number( acquisitionInterval ) );
  mCboDistanceThreshold->setCurrentText( QString::number( distanceThreshold ) );

  mSpinMapExtentMultiplier->setValue( recenteringThreshold );
  mSpinMapRotateInterval->setValue( rotateInterval );

  refreshDevices();
}

void QgsGpsOptionsWidget::apply()
{
  if ( QgsSymbol *markerSymbol = mGpsMarkerSymbolButton->symbol() )
  {
    QDomDocument doc;
    QDomElement elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "location-marker-symbol" ), markerSymbol, doc, QgsReadWriteContext() );
    doc.appendChild( elem );
    QgsGpsMarker::settingLocationMarkerSymbol.setValue( doc.toString( 0 ) );
  }
  QgsGpsMarker::settingRotateLocationMarker.setValue( mCheckRotateLocationMarker->isChecked() );

  if ( QgsSymbol *lineSymbol = mBearingLineStyleButton->symbol() )
  {
    QDomDocument doc;
    const QDomElement elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "gps-bearing-symbol" ), lineSymbol, doc, QgsReadWriteContext() );
    doc.appendChild( elem );
    QgsGpsInformationWidget::settingBearingLineSymbol.setValue( doc.toString( 0 ) );
  }

  QgsGpsConnection::settingsGpsSerialDevice.setValue( mCboDevices->currentData().toString() );

  if ( mRadAutodetect->isChecked() )
  {
    QgsGpsConnection::settingsGpsConnectionType.setValue( Qgis::GpsConnectionType::Automatic );
  }
  else if ( mRadInternal->isChecked() )
  {
    QgsGpsConnection::settingsGpsConnectionType.setValue( Qgis::GpsConnectionType::Internal );
  }
  else if ( mRadSerialDevice->isChecked() )
  {
    QgsGpsConnection::settingsGpsConnectionType.setValue( Qgis::GpsConnectionType::Serial );
  }
  else
  {
    QgsGpsConnection::settingsGpsConnectionType.setValue( Qgis::GpsConnectionType::Gpsd );
  }

  QgsGpsConnection::settingsGpsdHostName.setValue( mGpsdHost->text() );
  QgsGpsConnection::settingsGpsdPortNumber.setValue( mSpinGpsdPort->value() );
  QgsGpsConnection::settingsGpsdDeviceName.setValue( mGpsdDevice->text() );

  QgsGpsInformationWidget::settingGpsTrackColor.setValue( mBtnTrackColor->color() );
  QgsGpsInformationWidget::settingGpsTrackWidth.setValue( mSpinTrackWidth->value() );

  QgsGpsConnection::settingGpsAcquisitionInterval.setValue( mCboAcquisitionInterval->currentText().toInt() );
  QgsGpsConnection::settingGpsDistanceThreshold.setValue( mCboDistanceThreshold->currentText().toDouble() );
  QgsGpsConnection::settingGpsBearingFromTravelDirection.setValue( mTravelBearingCheckBox->isChecked() );

  QgsGpsInformationWidget::settingMapExtentRecenteringThreshold.setValue( mSpinMapExtentMultiplier->value() );
  QgsGpsInformationWidget::settingMapRotateInterval.setValue( mSpinMapRotateInterval->value() );
}

void QgsGpsOptionsWidget::refreshDevices()
{
  QList< QPair<QString, QString> > ports = QgsGpsDetector::availablePorts();

  mCboDevices->clear();

  // add devices to combobox, but skip gpsd which is first.
  for ( int i = 1; i < ports.size(); i++ )
  {
    mCboDevices->addItem( ports[i].second, ports[i].first );
  }

  // remember the last device used
  QString serialDevice;
  if ( QgsGpsConnection::settingsGpsSerialDevice.exists() )
  {
    serialDevice = QgsGpsConnection::settingsGpsSerialDevice.value();
  }
  else
  {
    // legacy setting
    const QgsSettings settings;
    serialDevice = settings.value( QStringLiteral( "lastPort" ), "", QgsSettings::Gps ).toString();
  }

  const int idx = mCboDevices->findData( serialDevice );
  mCboDevices->setCurrentIndex( idx < 0 ? 0 : idx );
}


//
// QgsGpsOptionsFactory
//
QgsGpsOptionsFactory::QgsGpsOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "GPS" ), QIcon() )
{

}

QIcon QgsGpsOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconGps.svg" ) );
}

QgsOptionsPageWidget *QgsGpsOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsGpsOptionsWidget( parent );
}

QString QgsGpsOptionsFactory::pagePositionHint() const
{
  return QStringLiteral( "mOptionsLocatorSettings" );
}
