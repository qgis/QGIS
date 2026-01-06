/***************************************************************************
    qgswfsnewconnection.cpp
    ---------------------
    begin                : June 2018
    copyright            : (C) 2018 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsnewconnection.h"

#include <algorithm>
#include <memory>

#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsowsconnection.h"
#include "qgswfsguiutils.h"

#include <QMap>
#include <QMessageBox>

#include "moc_qgswfsnewconnection.cpp"

static QString translatedImageFormatFromMediaType( const QString &type )
{
  static QMap<QString, QString> mapMimeTypeToTranslated {
    { u"default"_s, QObject::tr( "Default" ) },
    { u"application/fg+json"_s, QObject::tr( "JSON-FG" ) },
    { u"application/flatgeobuf"_s, QObject::tr( "FlatGeoBuf" ) },
    { u"application/geo+json"_s, QObject::tr( "GeoJSON" ) },
    { u"application/gml+xml"_s, QObject::tr( "GML" ) },
    { u"application/gml+xml;version=3.2"_s, QObject::tr( "GML 3.2" ) },
    { u"application/gml+xml;version=3.2;profile=\"http://www.opengis.net/def/profile/ogc/2.0/gml-sf0\""_s, QObject::tr( "GML 3.2, Simple Features 0 profile" ) },
    { u"application/gml+xml;version=3.2;profile=\"http://www.opengis.net/def/profile/ogc/2.0/gml-sf2\""_s, QObject::tr( "GML 3.2, Simple Features 2 profile" ) },
  };

  const auto iter = mapMimeTypeToTranslated.constFind( type );
  if ( iter != mapMimeTypeToTranslated.constEnd() )
    return iter.value();
  return type;
}

QgsWFSNewConnection::QgsWFSNewConnection( QWidget *parent, const QString &connName )
  : QgsNewHttpConnection( parent, QgsNewHttpConnection::ConnectionWfs, u"WFS"_s, connName )
{
  connect( wfsVersionDetectButton(), &QPushButton::clicked, this, &QgsWFSNewConnection::versionDetectButton );
  connect( featureFormatDetectButton(), &QPushButton::clicked, this, &QgsWFSNewConnection::detectFormat );

  const QStringList detailsParameters = { u"wfs"_s, originalConnectionName() };
  QString featureFormat = QgsOwsConnection::settingsDefaultFeatureFormat->value( detailsParameters );

  if ( featureFormat.isEmpty() )
  {
    // Read from global default setting
    featureFormat = QgsSettings().value( u"qgis/lastFeatureFormatEncoding"_s, QString() ).toString();
  }

  // Check the settings for available formats
  const QStringList availableFormats = QgsOwsConnection::settingsAvailableFeatureFormats->value( detailsParameters );
  featureFormatComboBox()->clear();
  if ( availableFormats.empty() )
  {
    featureFormatComboBox()->addItem( translatedImageFormatFromMediaType( u"default"_s ), u"default"_s );
  }
  else
  {
    int itemCount = 0;
    for ( const QString &format : availableFormats )
    {
      featureFormatComboBox()->addItem( translatedImageFormatFromMediaType( format ), format );
      if ( format == featureFormat )
      {
        featureFormatComboBox()->setCurrentIndex( itemCount );
      }
      ++itemCount;
    }
  }
}

QgsWFSNewConnection::~QgsWFSNewConnection()
{
  if ( mCapabilities || mOAPIFLandingPage || mOAPIFApi || mOAPIFCollectionsRequest )
  {
    QApplication::restoreOverrideCursor();
  }
}

void QgsWFSNewConnection::detectFormat()
{
  mDetectFormatInProgress = true;
  if ( mOAPIFCollectionsUrl.isEmpty() )
  {
    if ( wfsVersionComboBox()->currentIndex() == WFS_VERSION_MAX )
      startCapabilitiesRequest();
    else
      startOapifLandingPageRequest();
  }
  else
    startOapifCollectionsRequest();
}

QgsDataSourceUri QgsWFSNewConnection::createUri()
{
  // Honor any defined authentication settings
  QgsDataSourceUri uri;
  uri.setParam( u"url"_s, urlTrimmed().toString() );
  if ( authSettingsWidget()->configurationTabIsSelected() )
  {
    uri.setAuthConfigId( authSettingsWidget()->configId() );
  }
  else
  {
    uri.setUsername( authSettingsWidget()->username() );
    uri.setPassword( authSettingsWidget()->password() );
  }
  return uri;
}

void QgsWFSNewConnection::versionDetectButton()
{
  startCapabilitiesRequest();
}

void QgsWFSNewConnection::startCapabilitiesRequest()
{
  mCapabilities = std::make_unique<QgsWfsGetCapabilitiesRequest>( createUri().uri( false ) );
  connect( mCapabilities.get(), &QgsWfsGetCapabilitiesRequest::gotCapabilities, this, &QgsWFSNewConnection::capabilitiesReplyFinished );
  const bool synchronous = false;
  const bool forceRefresh = true;
  mCapabilities->setLogErrors( false ); // as this might be a OAPIF server
  if ( mCapabilities->requestCapabilities( synchronous, forceRefresh ) )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
  }
  else
  {
    mDetectFormatInProgress = false;
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), tr( "Could not get capabilities" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();
    mCapabilities.reset();
  }
}

void QgsWFSNewConnection::capabilitiesReplyFinished()
{
  if ( !mCapabilities )
    return;

  QApplication::restoreOverrideCursor();

  const auto err = mCapabilities->errorCode();
  if ( err != QgsBaseNetworkRequest::NoError )
  {
    startOapifLandingPageRequest();
    return;
  }

  mDetectFormatInProgress = false;

  const QgsWfsCapabilities &caps = mCapabilities->capabilities();
  int versionIdx = WFS_VERSION_MAX;
  wfsPageSizeLineEdit()->clear();
  if ( caps.version.startsWith( "1.0"_L1 ) )
  {
    versionIdx = WFS_VERSION_1_0;
  }
  else if ( caps.version.startsWith( "1.1"_L1 ) )
  {
    versionIdx = WFS_VERSION_1_1;
  }
  else if ( caps.version.startsWith( "2.0"_L1 ) )
  {
    versionIdx = WFS_VERSION_2_0;
    wfsPageSizeLineEdit()->setText( QString::number( caps.maxFeatures ) );
  }
  wfsVersionComboBox()->setCurrentIndex( versionIdx );

  wfsPagingComboBox()->setCurrentIndex(
    static_cast<int>( caps.supportsPaging ? QgsNewHttpConnection::WfsFeaturePagingIndex::ENABLED : QgsNewHttpConnection::WfsFeaturePagingIndex::DISABLED )
  );

  mCapabilities.reset();
}

void QgsWFSNewConnection::startOapifLandingPageRequest()
{
  mOAPIFLandingPage = std::make_unique<QgsOapifLandingPageRequest>( createUri() );
  connect( mOAPIFLandingPage.get(), &QgsOapifLandingPageRequest::gotResponse, this, &QgsWFSNewConnection::oapifLandingPageReplyFinished );
  const bool synchronous = false;
  const bool forceRefresh = true;
  if ( mOAPIFLandingPage->request( synchronous, forceRefresh ) )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
  }
  else
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), tr( "Could not get landing page" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();

    mOAPIFLandingPage.reset();
  }
}

void QgsWFSNewConnection::oapifLandingPageReplyFinished()
{
  if ( !mOAPIFLandingPage )
    return;

  QApplication::restoreOverrideCursor();

  if ( mOAPIFLandingPage->errorCode() != QgsBaseNetworkRequest::NoError )
  {
    if ( mOAPIFLandingPage->errorCode() == QgsBaseNetworkRequest::ApplicationLevelError )
    {
      QMessageBox *box = new QMessageBox( QMessageBox::Critical, QObject::tr( "Invalid response" ), mOAPIFLandingPage->errorMessage(), QMessageBox::Ok, this );
      box->setAttribute( Qt::WA_DeleteOnClose );
      box->setModal( true );
      box->open();
    }
    else if ( mCapabilities )
    {
      QgsMessageLog::logMessage( mCapabilities->errorMessage(), tr( "WFS" ) );
      QgsWfsGuiUtils::displayErrorMessageOnFailedCapabilities( mCapabilities.get(), this );
    }
    mDetectFormatInProgress = false;
    mCapabilities.reset();
    mOAPIFLandingPage.reset();
    return;
  }

  mOAPIFApiUrl = mOAPIFLandingPage->apiUrl();
  mOAPIFCollectionsUrl = mOAPIFLandingPage->collectionsUrl();
  mOAPIFLandingPage.reset();

  wfsVersionComboBox()->setCurrentIndex( WFS_VERSION_API_FEATURES_1_0 );
  wfsPagingComboBox()->setCurrentIndex( static_cast<int>( QgsNewHttpConnection::WfsFeaturePagingIndex::ENABLED ) );

  mCapabilities.reset();

  if ( mDetectFormatInProgress )
  {
    startOapifCollectionsRequest();
  }
  else
  {
    startOapifApiRequest();
  }
}

void QgsWFSNewConnection::startOapifApiRequest()
{
  mOAPIFApi = std::make_unique<QgsOapifApiRequest>( createUri(), mOAPIFApiUrl );

  connect( mOAPIFApi.get(), &QgsOapifApiRequest::gotResponse, this, &QgsWFSNewConnection::oapifApiReplyFinished );
  const bool synchronous = false;
  const bool forceRefresh = true;
  if ( mOAPIFApi->request( synchronous, forceRefresh ) )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
  }
  else
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), tr( "Could not get API" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();

    mOAPIFApi.reset();
  }
}

void QgsWFSNewConnection::oapifApiReplyFinished()
{
  if ( !mOAPIFApi )
    return;

  QApplication::restoreOverrideCursor();

  if ( mOAPIFApi->errorCode() != QgsBaseNetworkRequest::NoError )
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, QObject::tr( "Invalid response" ), mOAPIFApi->errorMessage(), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();

    mOAPIFApi.reset();
    return;
  }

  wfsPageSizeLineEdit()->clear();
  if ( mOAPIFApi->defaultLimit() > 0 && mOAPIFApi->maxLimit() > 0 )
    wfsPageSizeLineEdit()->setText( QString::number( std::min( std::max( 1000, mOAPIFApi->defaultLimit() ), mOAPIFApi->maxLimit() ) ) );
  else if ( mOAPIFApi->defaultLimit() > 0 )
    wfsPageSizeLineEdit()->setText( QString::number( std::max( 1000, mOAPIFApi->defaultLimit() ) ) );
  else if ( mOAPIFApi->maxLimit() > 0 )
    wfsPageSizeLineEdit()->setText( QString::number( mOAPIFApi->maxLimit() ) );

  mOAPIFApi.reset();
}

void QgsWFSNewConnection::startOapifCollectionsRequest()
{
  mOAPIFCollectionsRequest = std::make_unique<QgsOapifCollectionsRequest>( createUri(), mOAPIFCollectionsUrl );

  connect( mOAPIFCollectionsRequest.get(), &QgsOapifCollectionsRequest::gotResponse, this, &QgsWFSNewConnection::oapifCollectionsReplyFinished );
  const bool synchronous = false;
  const bool forceRefresh = true;
  if ( mOAPIFCollectionsRequest->request( synchronous, forceRefresh ) )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
  }
  else
  {
    mDetectFormatInProgress = false;
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), tr( "Could not get collections" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();
    mOAPIFCollectionsRequest.reset();
  }
}

void QgsWFSNewConnection::oapifCollectionsReplyFinished()
{
  if ( !mOAPIFCollectionsRequest )
    return;

  mDetectFormatInProgress = false;

  QApplication::restoreOverrideCursor();

  if ( mOAPIFCollectionsRequest->errorCode() != QgsBaseNetworkRequest::NoError )
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, QObject::tr( "Invalid response" ), mOAPIFCollectionsRequest->errorMessage(), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();

    mOAPIFCollectionsRequest.reset();
    return;
  }

  const QStringList detailsParameters = { u"wfs"_s, originalConnectionName() };

  // Store current format value
  QString currentFormat { featureFormatComboBox()->currentData().toString() };
  if ( currentFormat.isEmpty() )
  {
    currentFormat = QgsSettings().value( u"qgis/lastFeatureFormatEncoding"_s, QString() ).toString();
  }

  featureFormatComboBox()->clear();
  featureFormatComboBox()->addItem( translatedImageFormatFromMediaType( u"default"_s ), u"default"_s );
  int itemCount = 1;
  QStringList featureFormats;
  featureFormats << u"default"_s;
  for ( const QString &format : mOAPIFCollectionsRequest->featureFormats() )
  {
    featureFormatComboBox()->addItem( translatedImageFormatFromMediaType( format ), format );
    if ( format == currentFormat )
    {
      featureFormatComboBox()->setCurrentIndex( itemCount );
    }
    featureFormats << format;
    itemCount++;
  }

  // Update the connection list of available formats
  QgsOwsConnection::settingsAvailableFeatureFormats->setValue( featureFormats, detailsParameters );

  mOAPIFCollectionsRequest.reset();
}
