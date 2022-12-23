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
#include "qgsgpscanvasbridge.h"
#include "qgsappgpsdigitizing.h"
#include "qgslinesymbol.h"

#include <QTimeZone>

const int MAXACQUISITIONINTERVAL = 3000; // max gps information acquisition suspension interval (in seconds)
const int MAXDISTANCETHRESHOLD = 200; // max gps distance threshold (in meters)

//
// QgsGpsOptionsWidget
//

QgsGpsOptionsWidget::QgsGpsOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );
  setObjectName( "mGpsOptions" );

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
  mTrackLineStyleButton->setSymbolType( Qgis::SymbolType::Line );

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
  QString bearingLineSymbolXml = QgsGpsCanvasBridge::settingBearingLineSymbol.value();
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

  const QString trackLineSymbolXml = QgsAppGpsDigitizing::settingTrackLineSymbol.value();
  if ( !trackLineSymbolXml.isEmpty() )
  {
    doc.setContent( trackLineSymbolXml );
    elem = doc.documentElement();
    std::unique_ptr< QgsLineSymbol > trackLineSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( elem, QgsReadWriteContext() ) );
    if ( trackLineSymbol )
      mTrackLineStyleButton->setSymbol( trackLineSymbol.release() );
  }

  mCheckRotateLocationMarker->setChecked( QgsGpsMarker::settingRotateLocationMarker.value() );

  mSpinGpsdPort->setValue( 2947 );
  mSpinGpsdPort->setClearValue( 2947 );
  mSpinMapRotateInterval->setClearValue( 0 );
  mSpinMapExtentMultiplier->setClearValue( 50 );
  mOffsetFromUtc->setClearValue( 0 );

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

  mComboMValueAttribute->addItem( tr( "Do not Store M Values" ) );
  mComboMValueAttribute->addItem( tr( "Timestamp (Milliseconds Since Epoch)" ), QVariant::fromValue( Qgis::GpsInformationComponent::Timestamp ) );
  mComboMValueAttribute->addItem( tr( "Ground Speed" ), QVariant::fromValue( Qgis::GpsInformationComponent::GroundSpeed ) );
  mComboMValueAttribute->addItem( tr( "Bearing" ), QVariant::fromValue( Qgis::GpsInformationComponent::Bearing ) );
  mComboMValueAttribute->addItem( tr( "Altitude (Geoid)" ), QVariant::fromValue( Qgis::GpsInformationComponent::EllipsoidAltitude ) );
  mComboMValueAttribute->addItem( tr( "Altitude (WGS-84 Ellipsoid)" ), QVariant::fromValue( Qgis::GpsInformationComponent::Altitude ) );
  mComboMValueAttribute->addItem( tr( "PDOP" ), QVariant::fromValue( Qgis::GpsInformationComponent::Pdop ) );
  mComboMValueAttribute->addItem( tr( "HDOP" ), QVariant::fromValue( Qgis::GpsInformationComponent::Hdop ) );
  mComboMValueAttribute->addItem( tr( "VDOP" ), QVariant::fromValue( Qgis::GpsInformationComponent::Vdop ) );
  mComboMValueAttribute->addItem( tr( "Horizontal Accuracy" ), QVariant::fromValue( Qgis::GpsInformationComponent::HorizontalAccuracy ) );
  mComboMValueAttribute->addItem( tr( "Vertical Accuracy" ), QVariant::fromValue( Qgis::GpsInformationComponent::VerticalAccuracy ) );
  mComboMValueAttribute->addItem( tr( "Accuracy (3D RMS)" ), QVariant::fromValue( Qgis::GpsInformationComponent::HvAccuracy ) );
  mComboMValueAttribute->addItem( tr( "Satellites Used" ), QVariant::fromValue( Qgis::GpsInformationComponent::SatellitesUsed ) );
  mComboMValueAttribute->setCurrentIndex( 0 );

  Qgis::GpsConnectionType connectionType = Qgis::GpsConnectionType::Automatic;
  QString gpsdHost;
  int gpsdPort = 0;
  QString gpsdDevice;
  int acquisitionInterval = 0;
  double distanceThreshold = 0;
  bool bearingFromTravelDirection = false;
  int recenteringThreshold = 50;
  int rotateInterval = 0;
  bool applyLeapSeconds = true;
  int leapSeconds = 0;
  Qt::TimeSpec timeSpec = Qt::TimeSpec::LocalTime;
  QString timeZone;
  int offsetFromUtc = 0;
  if ( QgsGpsConnection::settingsGpsConnectionType.exists() )
  {
    connectionType = QgsGpsConnection::settingsGpsConnectionType.value();
    gpsdHost = QgsGpsConnection::settingsGpsdHostName.value();
    gpsdPort = static_cast< int >( QgsGpsConnection::settingsGpsdPortNumber.value() );
    gpsdDevice = QgsGpsConnection::settingsGpsdDeviceName.value();
    acquisitionInterval = static_cast< int >( QgsGpsConnection::settingGpsAcquisitionInterval.value() );
    distanceThreshold = QgsGpsConnection::settingGpsDistanceThreshold.value();
    bearingFromTravelDirection = QgsGpsConnection::settingGpsBearingFromTravelDirection.value();
    recenteringThreshold = static_cast< int >( QgsGpsCanvasBridge::settingMapExtentRecenteringThreshold.value() );
    rotateInterval = static_cast< int >( QgsGpsCanvasBridge::settingMapRotateInterval.value() );

    applyLeapSeconds = QgsGpsConnection::settingGpsApplyLeapSecondsCorrection.value();
    leapSeconds = static_cast< int >( QgsGpsConnection::settingGpsLeapSeconds.value() );
    timeSpec = QgsGpsConnection::settingsGpsTimeStampSpecification.value();
    timeZone = QgsGpsConnection::settingsGpsTimeStampTimeZone.value();
    offsetFromUtc = static_cast< int >( QgsGpsConnection::settingsGpsTimeStampOffsetFromUtc.value() );

    if ( QgsGpsLogger::settingsGpsStoreAttributeInMValues.value() )
    {
      mComboMValueAttribute->setCurrentIndex( mComboMValueAttribute->findData( QVariant::fromValue( QgsGpsLogger::settingsGpsMValueComponent.value() ) ) );
    }
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

    acquisitionInterval = settings.value( QStringLiteral( "acquisitionInterval" ), 0, QgsSettings::Gps ).toInt();
    distanceThreshold = settings.value( QStringLiteral( "distanceThreshold" ), 0, QgsSettings::Gps ).toDouble();

    bearingFromTravelDirection = settings.value( QStringLiteral( "calculateBearingFromTravel" ), "false", QgsSettings::Gps ).toBool();

    recenteringThreshold = settings.value( QStringLiteral( "mapExtentMultiplier" ), "50", QgsSettings::Gps ).toInt();
    rotateInterval = settings.value( QStringLiteral( "rotateMapInterval" ), 0, QgsSettings::Gps ).toInt();

    applyLeapSeconds = settings.value( QStringLiteral( "applyLeapSeconds" ), true, QgsSettings::Gps ).toBool();
    leapSeconds = settings.value( QStringLiteral( "leapSecondsCorrection" ), 18, QgsSettings::Gps ).toInt();

    switch ( settings.value( QStringLiteral( "timeStampFormat" ), Qt::LocalTime, QgsSettings::Gps ).toInt() )
    {
      case 0:
        timeSpec = Qt::TimeSpec::LocalTime;
        break;

      case 1:
        timeSpec = Qt::TimeSpec::UTC;
        break;

      case 2:
        timeSpec = Qt::TimeSpec::TimeZone;
        break;
    }
    timeZone = settings.value( QStringLiteral( "timestampTimeZone" ), QVariant(), QgsSettings::Gps ).toString();
  }

  mOffsetFromUtc->setValue( offsetFromUtc );

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

  mCboAcquisitionInterval->setCurrentText( QString::number( acquisitionInterval ) );
  mCboDistanceThreshold->setCurrentText( QString::number( distanceThreshold ) );

  mSpinMapExtentMultiplier->setValue( recenteringThreshold );
  mSpinMapRotateInterval->setValue( rotateInterval );

  // Qt::LocalTime  0 Locale dependent time (Timezones and Daylight Savings Time).
  // Qt::UTC  1 Coordinated Universal Time, replaces Greenwich Mean Time.
  // SKIP this one: Qt::OffsetFromUTC  2 An offset in seconds from Coordinated Universal Time.
  // Qt::TimeZone 3 A named time zone using a specific set of Daylight Savings rules.
  mCboTimestampFormat->addItem( tr( "Local Time" ), Qt::TimeSpec::LocalTime );
  mCboTimestampFormat->addItem( tr( "UTC" ), Qt::TimeSpec::UTC );
  mCboTimestampFormat->addItem( tr( "UTC with Offset" ), Qt::TimeSpec::OffsetFromUTC );
  mCboTimestampFormat->addItem( tr( "Time Zone" ), Qt::TimeSpec::TimeZone );
  mCboTimestampFormat->setCurrentIndex( mCboTimestampFormat->findData( timeSpec ) );
  if ( mCboTimestampFormat->currentIndex() < 0 )
    mCboTimestampFormat->setCurrentIndex( 0 );

  connect( mCboTimestampFormat, qOverload< int >( &QComboBox::currentIndexChanged ),
           this, &QgsGpsOptionsWidget::timestampFormatChanged );
  timestampFormatChanged( 0 );
  updateTimeZones();

  const QList<QByteArray> constTzs = QTimeZone::availableTimeZoneIds();
  for ( const QByteArray &tzId : constTzs )
  {
    mCboTimeZones->addItem( tzId );
  }
  int tzIdx = mCboTimeZones->findText( timeZone );
  if ( tzIdx == -1 )
  {
    const QString currentTz { QTimeZone::systemTimeZoneId() };
    tzIdx = mCboTimeZones->findText( currentTz );
  }
  mCboTimeZones->setCurrentIndex( tzIdx );

  mCbxLeapSeconds->setChecked( applyLeapSeconds );
  // Leap seconds as of 2019-06-20, if the default changes, it can be updated in qgis_global_settings.ini
  mLeapSeconds->setValue( leapSeconds );
  mLeapSeconds->setClearValue( 18 );

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
    QgsGpsCanvasBridge::settingBearingLineSymbol.setValue( doc.toString( 0 ) );
  }

  if ( QgsSymbol *lineSymbol = mTrackLineStyleButton->symbol() )
  {
    QDomDocument doc;
    const QDomElement elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "gps-track-symbol" ), lineSymbol, doc, QgsReadWriteContext() );
    doc.appendChild( elem );
    QgsAppGpsDigitizing::settingTrackLineSymbol.setValue( doc.toString( 0 ) );
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

  QgsGpsConnection::settingGpsAcquisitionInterval.setValue( mCboAcquisitionInterval->currentText().toInt() );
  QgsGpsConnection::settingGpsDistanceThreshold.setValue( mCboDistanceThreshold->currentText().toDouble() );
  QgsGpsConnection::settingGpsBearingFromTravelDirection.setValue( mTravelBearingCheckBox->isChecked() );

  QgsGpsCanvasBridge::settingMapExtentRecenteringThreshold.setValue( mSpinMapExtentMultiplier->value() );
  QgsGpsCanvasBridge::settingMapRotateInterval.setValue( mSpinMapRotateInterval->value() );

  QgsGpsConnection::settingsGpsTimeStampSpecification.setValue( static_cast< Qt::TimeSpec >( mCboTimestampFormat->currentData( ).toInt() ) );
  QgsGpsConnection::settingsGpsTimeStampTimeZone.setValue( mCboTimeZones->currentText() );
  QgsGpsConnection::settingGpsApplyLeapSecondsCorrection.setValue( mCbxLeapSeconds->isChecked() );
  QgsGpsConnection::settingGpsLeapSeconds.setValue( mLeapSeconds->value() );
  QgsGpsConnection::settingsGpsTimeStampOffsetFromUtc.setValue( mOffsetFromUtc->value() );

  if ( !mComboMValueAttribute->currentData().isValid() )
  {
    QgsGpsLogger::settingsGpsStoreAttributeInMValues.setValue( false );
  }
  else
  {
    QgsGpsLogger::settingsGpsStoreAttributeInMValues.setValue( true );
    QgsGpsLogger::settingsGpsMValueComponent.setValue( mComboMValueAttribute->currentData().value< Qgis::GpsInformationComponent >() );
  }
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

void QgsGpsOptionsWidget::timestampFormatChanged( int )
{
  const Qt::TimeSpec currentSpec = static_cast<Qt::TimeSpec>( mCboTimestampFormat->currentData( ).toInt() );
  const bool timeZoneEnabled {currentSpec == Qt::TimeSpec::TimeZone };
  mCboTimeZones->setEnabled( timeZoneEnabled );
  mLblTimeZone->setEnabled( timeZoneEnabled );
  const bool offsetFromUtcEnabled = currentSpec == Qt::TimeSpec::OffsetFromUTC;
  mOffsetFromUtc->setEnabled( offsetFromUtcEnabled );
  mLblOffsetFromUtc->setEnabled( offsetFromUtcEnabled );
}

void QgsGpsOptionsWidget::updateTimeZones()
{
  const bool enabled = static_cast<Qt::TimeSpec>( mCboTimestampFormat->currentData( ).toInt() ) == Qt::TimeSpec::TimeZone;
  mCboTimeZones->setEnabled( enabled );
  mLblTimeZone->setEnabled( enabled );
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
