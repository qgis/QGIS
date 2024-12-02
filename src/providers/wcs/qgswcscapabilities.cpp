/***************************************************************************
  qgswcscapabilities.cpp  -  WCS capabilities
                             -------------------
    begin                : 17 Mar, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au

    wcs                  : 4/2012 Radim Blazek, based on qgswmsprovider.cpp

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslogger.h"
#include "qgswcscapabilities.h"
#include "moc_qgswcscapabilities.cpp"
#include "qgsowsconnection.h"

#include <cmath>

#include "qgsauthmanager.h"
#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSet>
#include <QEventLoop>

#ifdef _MSC_VER
#include <float.h>
#define isfinite( x ) _finite( x )
#endif

#ifdef QGISDEBUG
#include <QFile>
#include <QDir>
#endif

QgsWcsCapabilities::QgsWcsCapabilities( QgsDataSourceUri const &uri )
  : mUri( uri )
{
  QgsDebugMsgLevel( "uri = " + mUri.encodedUri(), 2 );

  parseUri();

  retrieveServerCapabilities();
}

QgsWcsCapabilities::QgsWcsCapabilities( const QgsWcsCapabilities &other )
  : QObject()
  , mUri( other.mUri )
  , mVersion( other.mVersion )
  , mCapabilitiesResponse( other.mCapabilitiesResponse )
  , mCapabilitiesDom( other.mCapabilitiesDom )
  , mServiceExceptionReportDom( other.mServiceExceptionReportDom )
  , mCapabilities( other.mCapabilities )
  , mCoveragesSupported( other.mCoveragesSupported )
  , mCapabilitiesReply( nullptr ) // not copied from other
  , mErrorTitle()                 // not copied from other
  , mError()                      // not copied from other
  , mErrorFormat()                // not copied from other
  , mCoverageCount( other.mCoverageCount )
  , mCoverageParents( other.mCoverageParents )
  , mCoverageParentIdentifiers( other.mCoverageParentIdentifiers )
  , mUserName( other.mUserName )
  , mPassword( other.mPassword )
  , mCacheLoadControl( other.mCacheLoadControl )
{
}


void QgsWcsCapabilities::parseUri()
{
  mCacheLoadControl = QNetworkRequest::PreferNetwork;

  QString cache = mUri.param( QStringLiteral( "cache" ) );
  QgsDebugMsgLevel( "cache = " + cache, 2 );

  if ( !cache.isEmpty() )
  {
    mCacheLoadControl = QgsNetworkAccessManager::cacheLoadControlFromName( cache );
  }
  QgsDebugMsgLevel( QStringLiteral( "mCacheLoadControl = %1" ).arg( mCacheLoadControl ), 2 );
}

// TODO: return if successful
void QgsWcsCapabilities::setUri( QgsDataSourceUri const &uri )
{
  mUri = uri;

  clear();

  parseUri();

  retrieveServerCapabilities();
}

QString QgsWcsCapabilities::prepareUri( QString uri )
{
  if ( !uri.contains( '?' ) )
  {
    uri.append( '?' );
  }
  else if ( uri.right( 1 ) != QLatin1String( "?" ) && uri.right( 1 ) != QLatin1String( "&" ) )
  {
    uri.append( '&' );
  }

  return uri;
}

const QgsWcsCapabilitiesProperty &QgsWcsCapabilities::capabilities() const
{
  return mCapabilities;
}

bool QgsWcsCapabilities::supportedCoverages( QVector<QgsWcsCoverageSummary> &coverageSummary )
{
  QgsDebugMsgLevel( QStringLiteral( "Entering." ), 3 );

  coverageSummary = mCoveragesSupported;

  QgsDebugMsgLevel( QStringLiteral( "Exiting." ), 3 );

  return true;
}

QString QgsWcsCapabilities::getCoverageUrl() const
{
  QString url = mCapabilities.getCoverageGetUrl;
  if ( url.isEmpty() )
  {
    url = mUri.param( QStringLiteral( "url" ) );
  }
  return url;
}

bool QgsWcsCapabilities::sendRequest( QString const &url )
{
  QgsDebugMsgLevel( "url = " + url, 2 );
  mError.clear();
  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWcsCapabilities" ) );
  if ( !setAuthorization( request ) )
  {
    mError = tr( "Download of capabilities failed: network request update failed for authentication config" );
    QgsMessageLog::logMessage( mError, tr( "WCS" ) );
    return false;
  }
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mCacheLoadControl );
  QgsDebugMsgLevel( QStringLiteral( "mCacheLoadControl = %1" ).arg( mCacheLoadControl ), 2 );

  QgsDebugMsgLevel( QStringLiteral( "getcapabilities: %1" ).arg( url ), 2 );
  mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );
  if ( !setAuthorizationReply( mCapabilitiesReply ) )
  {
    mCapabilitiesReply->deleteLater();
    mCapabilitiesReply = nullptr;
    mError = tr( "Download of capabilities failed: network reply update failed for authentication config" );
    QgsMessageLog::logMessage( mError, tr( "WCS" ) );
    return false;
  }

  connect( mCapabilitiesReply, &QNetworkReply::finished, this, &QgsWcsCapabilities::capabilitiesReplyFinished );
  connect( mCapabilitiesReply, &QNetworkReply::downloadProgress, this, &QgsWcsCapabilities::capabilitiesReplyProgress );

  QEventLoop loop;
  connect( this, &QgsWcsCapabilities::downloadFinished, &loop, &QEventLoop::quit );
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  if ( mCapabilitiesResponse.isEmpty() )
  {
    if ( mError.isEmpty() )
    {
      mErrorFormat = QStringLiteral( "text/plain" );
      mError = tr( "empty capabilities document" );
    }
    return false;
  }

  if ( mCapabilitiesResponse.startsWith( "<html>" ) || mCapabilitiesResponse.startsWith( "<HTML>" ) )
  {
    mErrorFormat = QStringLiteral( "text/html" );
    mError = mCapabilitiesResponse;
    return false;
  }
  return true;
}

void QgsWcsCapabilities::clear()
{
  mCoverageCount = 0;
  mCoveragesSupported.clear();
  mCapabilities = QgsWcsCapabilitiesProperty();
}

QString QgsWcsCapabilities::getCapabilitiesUrl( const QString &version ) const
{
  QString url = prepareUri( mUri.param( QStringLiteral( "url" ) ) ) + "SERVICE=WCS&REQUEST=GetCapabilities";

  if ( !version.isEmpty() )
  {
    // 1.0.0 - VERSION
    // 1.1.0 - AcceptVersions (not supported by UMN Mapserver 6.0.3 - defaults to latest 1.1
    if ( version.startsWith( QLatin1String( "1.0" ) ) )
    {
      url += "&VERSION=" + version;
    }
    else if ( version.startsWith( QLatin1String( "1.1" ) ) )
    {
      // Ignored by UMN Mapserver 6.0.3, see below
      url += "&AcceptVersions=" + version;
    }
  }
  return url;
}

QString QgsWcsCapabilities::getCapabilitiesUrl() const
{
  return getCapabilitiesUrl( mVersion );
}

bool QgsWcsCapabilities::retrieveServerCapabilities()
{
  clear();

  QStringList versions;

  QString preferredVersion = mUri.param( QStringLiteral( "version" ) );

  if ( !preferredVersion.isEmpty() )
  {
    versions << preferredVersion;
  }
  else
  {
    // We prefer 1.0 because 1.1 has many issues, each server implements it in
    // a different way with various particularities.
    // It may happen that server supports 1.1.0 but gives error for 1.1
    versions << QStringLiteral( "1.0.0" ) << QStringLiteral( "1.1.0,1.0.0" );
  }

  const auto constVersions = versions;
  for ( const QString &v : constVersions )
  {
    if ( retrieveServerCapabilities( v ) )
    {
      return true;
    }
  }

  return false;
}

bool QgsWcsCapabilities::retrieveServerCapabilities( const QString &preferredVersion )
{
  clear();

  QString url = getCapabilitiesUrl( preferredVersion );

  if ( !sendRequest( url ) )
  {
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "Converting to Dom." ), 2 );

  bool domOK;
  domOK = parseCapabilitiesDom( mCapabilitiesResponse, mCapabilities );

  if ( !domOK )
  {
    // We had an Dom exception -
    // mErrorTitle and mError are pre-filled by parseCapabilitiesDom

    mError += tr( "\nTried URL: %1" ).arg( url );

    QgsDebugError( "!domOK: " + mError );

    return false;
  }

  return true;
}

QString QgsWcsCapabilities::getDescribeCoverageUrl( QString const &identifier ) const
{
  QString url = prepareUri( mUri.param( QStringLiteral( "url" ) ) ) + "SERVICE=WCS&REQUEST=DescribeCoverage&VERSION=" + mVersion;

  if ( mVersion.startsWith( QLatin1String( "1.0" ) ) )
  {
    url += "&COVERAGE=" + identifier;
  }
  else if ( mVersion.startsWith( QLatin1String( "1.1" ) ) )
  {
    // in 1.1.0, 1.1.1, 1.1.2 the name of param is 'identifier'
    // but in KVP 'identifiers'
    url += "&IDENTIFIERS=" + identifier;
  }
  return url;
}

bool QgsWcsCapabilities::describeCoverage( QString const &identifier, bool forceRefresh )
{
  QgsDebugMsgLevel( " identifier = " + identifier, 2 );

  QgsWcsCoverageSummary *coverage = coverageSummary( identifier );
  if ( !coverage )
  {
    QgsDebugError( QStringLiteral( "coverage not found" ) );
    return false;
  }

  if ( coverage->described && !forceRefresh )
    return true;

  QString url = getDescribeCoverageUrl( coverage->identifier );

  if ( !sendRequest( url ) )
  {
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "Converting to Dom." ), 2 );

  bool domOK = false;
  if ( mVersion.startsWith( QLatin1String( "1.0" ) ) )
  {
    domOK = parseDescribeCoverageDom10( mCapabilitiesResponse, coverage );
  }
  else if ( mVersion.startsWith( QLatin1String( "1.1" ) ) )
  {
    domOK = parseDescribeCoverageDom11( mCapabilitiesResponse, coverage );
  }

  if ( !domOK )
  {
    // We had an Dom exception -
    // mErrorTitle and mError are pre-filled by parseCapabilitiesDom

    mError += tr( "\nTried URL: %1" ).arg( url );

    QgsDebugError( "!domOK: " + mError );

    return false;
  }

  return true;
}

void QgsWcsCapabilities::capabilitiesReplyFinished()
{
  if ( mCapabilitiesReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mCapabilitiesReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !QgsVariantUtils::isNull( redirect ) )
    {
      emit statusChanged( tr( "Capabilities request redirected." ) );

      QNetworkRequest request( redirect.toUrl() );
      QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWcsCapabilities" ) );
      if ( !setAuthorization( request ) )
      {
        mCapabilitiesResponse.clear();
        mError = tr( "Download of capabilities failed: network request update failed for authentication config" );
        QgsMessageLog::logMessage( mError, tr( "WCS" ) );
        return;
      }
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

      mCapabilitiesReply->deleteLater();
      QgsDebugMsgLevel( QStringLiteral( "redirected getcapabilities: %1" ).arg( redirect.toString() ), 2 );
      mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );
      if ( !setAuthorizationReply( mCapabilitiesReply ) )
      {
        mCapabilitiesResponse.clear();
        mCapabilitiesReply->deleteLater();
        mCapabilitiesReply = nullptr;
        mError = tr( "Download of capabilities failed: network reply update failed for authentication config" );
        QgsMessageLog::logMessage( mError, tr( "WCS" ) );
        return;
      }

      connect( mCapabilitiesReply, &QNetworkReply::finished, this, &QgsWcsCapabilities::capabilitiesReplyFinished );
      connect( mCapabilitiesReply, &QNetworkReply::downloadProgress, this, &QgsWcsCapabilities::capabilitiesReplyProgress );
      return;
    }

    mCapabilitiesResponse = mCapabilitiesReply->readAll();

    if ( mCapabilitiesResponse.isEmpty() )
    {
      mErrorFormat = QStringLiteral( "text/plain" );
      mError = tr( "empty of capabilities: %1" ).arg( mCapabilitiesReply->errorString() );
    }
  }
  else
  {
    // Resend request if AlwaysCache
    QNetworkRequest request = mCapabilitiesReply->request();
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWcsCapabilities" ) );
    if ( request.attribute( QNetworkRequest::CacheLoadControlAttribute ).toInt() == QNetworkRequest::AlwaysCache )
    {
      QgsDebugMsgLevel( QStringLiteral( "Resend request with PreferCache" ), 2 );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );

      mCapabilitiesReply->deleteLater();

      mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );
      if ( !setAuthorizationReply( mCapabilitiesReply ) )
      {
        mCapabilitiesResponse.clear();
        mCapabilitiesReply->deleteLater();
        mCapabilitiesReply = nullptr;
        mError = tr( "Download of capabilities failed: network reply update failed for authentication config" );
        QgsMessageLog::logMessage( mError, tr( "WCS" ) );
        return;
      }
      connect( mCapabilitiesReply, &QNetworkReply::finished, this, &QgsWcsCapabilities::capabilitiesReplyFinished );
      connect( mCapabilitiesReply, &QNetworkReply::downloadProgress, this, &QgsWcsCapabilities::capabilitiesReplyProgress );
      return;
    }

    mErrorFormat = QStringLiteral( "text/plain" );
    mError = tr( "Download of capabilities failed: %1" ).arg( mCapabilitiesReply->errorString() );
    QgsMessageLog::logMessage( mError, tr( "WCS" ) );
    mCapabilitiesResponse.clear();
  }

  mCapabilitiesReply->deleteLater();
  mCapabilitiesReply = nullptr;

  emit downloadFinished();
}

void QgsWcsCapabilities::capabilitiesReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString message = tr( "%1 of %2 bytes of capabilities downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QStringLiteral( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsgLevel( message, 2 );
  emit statusChanged( message );
}

QString QgsWcsCapabilities::stripNS( const QString &name )
{
  return name.contains( ':' ) ? name.section( ':', 1 ) : name;
}

bool QgsWcsCapabilities::parseCapabilitiesDom( QByteArray const &xml, QgsWcsCapabilitiesProperty &capabilities )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );
#ifdef QGISDEBUG
  QFile file( QDir::tempPath() + "/qgis-wcs-capabilities.xml" );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    file.write( xml );
    file.close();
  }
#endif

  if ( !convertToDom( xml ) )
    return false;

  QDomElement documentElement = mCapabilitiesDom.documentElement();

  // Assert that the DTD is what we expected (i.e. a WCS Capabilities document)
  QgsDebugMsgLevel( "testing tagName " + documentElement.tagName(), 2 );

  QString tagName = stripNS( documentElement.tagName() );
  if (
    // We don't support 1.0, but try WCS_Capabilities tag to get version
    tagName != QLatin1String( "WCS_Capabilities" ) && // 1.0
    tagName != QLatin1String( "Capabilities" )        // 1.1, tags seen: Capabilities, wcs:Capabilities
  )
  {
    if ( tagName == QLatin1String( "ExceptionReport" ) )
    {
      mErrorTitle = tr( "Exception" );
      mErrorFormat = QStringLiteral( "text/plain" );
      mError = tr( "Could not get WCS capabilities: %1" ).arg( domElementText( documentElement, QStringLiteral( "Exception.ExceptionText" ) ) );
    }
    else
    {
      mErrorTitle = tr( "Dom Exception" );
      mErrorFormat = QStringLiteral( "text/plain" );
      mError = tr( "Could not get WCS capabilities in the expected format (DTD): no %1 found.\nThis might be due to an incorrect WCS Server URL.\nTag: %3\nResponse was:\n%4" )
                 .arg( QStringLiteral( "Capabilities" ), documentElement.tagName(), QString( xml ) );
    }

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  capabilities.version = documentElement.attribute( QStringLiteral( "version" ) );
  mVersion = capabilities.version;

  if ( !mVersion.startsWith( QLatin1String( "1.0" ) ) && !mVersion.startsWith( QLatin1String( "1.1" ) ) )
  {
    mErrorTitle = tr( "Version not supported" );
    mErrorFormat = QStringLiteral( "text/plain" );
    mError = tr( "WCS server version %1 is not supported by QGIS (supported versions: 1.0.0, 1.1.0, 1.1.2)" )
               .arg( mVersion );

    QgsLogger::debug( "WCS version: " + mError );

    return false;
  }

  if ( mVersion.startsWith( QLatin1String( "1.0" ) ) )
  {
    capabilities.title = domElementText( documentElement, QStringLiteral( "Service.name" ) );
    capabilities.abstract = domElementText( documentElement, QStringLiteral( "Service.description" ) );
    // There is also "label" in 1.0

    capabilities.getCoverageGetUrl = domElement( documentElement, QStringLiteral( "Capability.Request.GetCoverage.DCPType.HTTP.Get.OnlineResource" ) ).attribute( QStringLiteral( "xlink:href" ) );

    parseContentMetadata( domElement( documentElement, QStringLiteral( "ContentMetadata" ) ), capabilities.contents );
  }
  else if ( mVersion.startsWith( QLatin1String( "1.1" ) ) )
  {
    capabilities.title = domElementText( documentElement, QStringLiteral( "ServiceIdentification.Title" ) );
    capabilities.abstract = domElementText( documentElement, QStringLiteral( "ServiceIdentification.Abstract" ) );

    QList<QDomElement> operationElements = domElements( documentElement, QStringLiteral( "OperationsMetadata.Operation" ) );
    const auto constOperationElements = operationElements;
    for ( const QDomElement &el : constOperationElements )
    {
      if ( el.attribute( QStringLiteral( "name" ) ) == QLatin1String( "GetCoverage" ) )
      {
        capabilities.getCoverageGetUrl = domElement( el, QStringLiteral( "DCP.HTTP.Get" ) ).attribute( QStringLiteral( "xlink:href" ) );
      }
    }

    parseCoverageSummary( domElement( documentElement, QStringLiteral( "Contents" ) ), capabilities.contents );
  }

  return true;
}

QDomElement QgsWcsCapabilities::firstChild( const QDomElement &element, const QString &name )
{
  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = stripNS( nodeElement.tagName() );
      if ( tagName == name )
      {
        return nodeElement;
      }
    }
    node = node.nextSibling();
  }
  return QDomElement();
}

QString QgsWcsCapabilities::firstChildText( const QDomElement &element, const QString &name )
{
  QDomElement firstChildElement = firstChild( element, name );
  if ( !firstChildElement.isNull() )
  {
    return firstChildElement.text();
  }
  return QString();
}

QList<QDomElement> QgsWcsCapabilities::domElements( const QDomElement &element, const QString &path )
{
  QList<QDomElement> list;

  QStringList names = path.split( '.' );
  if ( names.isEmpty() )
    return list;
  QString name = names.value( 0 );
  names.removeFirst();

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = stripNS( nodeElement.tagName() );
      if ( tagName == name )
      {
        if ( names.isEmpty() )
        {
          list.append( nodeElement );
        }
        else
        {
          list.append( domElements( nodeElement, names.join( QLatin1Char( '.' ) ) ) );
        }
      }
    }
    node = node.nextSibling();
  }

  return list;
}

QStringList QgsWcsCapabilities::domElementsTexts( const QDomElement &element, const QString &path )
{
  QStringList list;
  QList<QDomElement> elements = domElements( element, path );

  const auto constElements = elements;
  for ( const QDomElement &el : constElements )
  {
    list << el.text();
  }
  return list;
}

QDomElement QgsWcsCapabilities::domElement( const QDomElement &element, const QString &path )
{
  QStringList names = path.split( '.' );
  if ( names.isEmpty() )
    return QDomElement();

  QDomElement firstChildElement = firstChild( element, names.value( 0 ) );
  if ( names.size() == 1 || firstChildElement.isNull() )
  {
    return firstChildElement;
  }
  names.removeFirst();
  return domElement( firstChildElement, names.join( QLatin1Char( '.' ) ) );
}

QString QgsWcsCapabilities::domElementText( const QDomElement &element, const QString &path )
{
  QDomElement pathElement = domElement( element, path );
  return pathElement.text();
}

QList<int> QgsWcsCapabilities::parseInts( const QString &text )
{
  QList<int> list;
  const QStringList parts = text.split( ' ' );
  for ( const QString &s : parts )
  {
    bool ok;
    list.append( s.toInt( &ok ) );
    if ( !ok )
    {
      list.clear();
      return list;
    }
  }
  return list;
}

QList<double> QgsWcsCapabilities::parseDoubles( const QString &text )
{
  QList<double> list;
  const QStringList parts = text.split( ' ' );
  for ( const QString &s : parts )
  {
    bool ok;
    list.append( s.toDouble( &ok ) );
    if ( !ok )
    {
      list.clear();
      return list;
    }
  }
  return list;
}

QString QgsWcsCapabilities::crsUrnToAuthId( const QString &text )
{
  QString authid = text; // may be also non URN, for example 'EPSG:4326'

  // URN format: urn:ogc:def:objectType:authority:version:code
  // URN example: urn:ogc:def:crs:EPSG::4326
  QStringList urn = text.split( ':' );
  if ( urn.size() == 7 )
  {
    authid = urn.value( 4 ) + ':' + urn.value( 6 );
  }

  return authid;
}

// ------------------------ 1.0 ----------------------------------------------


void QgsWcsCapabilities::parseContentMetadata( const QDomElement &element, QgsWcsCoverageSummary &coverageSummary )
{
  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = stripNS( nodeElement.tagName() );

      if ( tagName == QLatin1String( "CoverageOfferingBrief" ) )
      {
        QgsWcsCoverageSummary subCoverageSummary;

        initCoverageSummary( subCoverageSummary );

        parseCoverageOfferingBrief( nodeElement, subCoverageSummary, &coverageSummary );

        subCoverageSummary.valid = true;

        coverageSummary.coverageSummary.push_back( subCoverageSummary );
      }
    }
    node = node.nextSibling();
  }
}

void QgsWcsCapabilities::parseCoverageOfferingBrief( const QDomElement &element, QgsWcsCoverageSummary &coverageSummary, QgsWcsCoverageSummary *parent )
{
  Q_UNUSED( parent )
  coverageSummary.orderId = ++mCoverageCount;

  coverageSummary.identifier = firstChildText( element, QStringLiteral( "name" ) );
  coverageSummary.title = firstChildText( element, QStringLiteral( "label" ) );
  coverageSummary.abstract = firstChildText( element, QStringLiteral( "description" ) );

  parseMetadataLink( element, coverageSummary.metadataLink );

  QList<QDomElement> posElements = domElements( element, QStringLiteral( "lonLatEnvelope.pos" ) );
  if ( posElements.size() != 2 )
  {
    QgsDebugError( QStringLiteral( "Wrong number of pos elements" ) );
  }
  else
  {
    QList<double> low = parseDoubles( posElements.value( 0 ).text() );
    QList<double> high = parseDoubles( posElements.value( 1 ).text() );
    if ( low.size() == 2 && high.size() == 2 )
    {
      coverageSummary.wgs84BoundingBox = QgsRectangle( low[0], low[1], high[0], high[1] );
      QgsDebugMsgLevel( "wgs84BoundingBox = " + coverageSummary.wgs84BoundingBox.toString(), 2 );
    }
  }

  if ( !coverageSummary.identifier.isEmpty() )
  {
    QgsDebugMsgLevel( "add coverage " + coverageSummary.identifier + " to supported", 2 );
    mCoveragesSupported.push_back( coverageSummary );
  }

  if ( !coverageSummary.coverageSummary.empty() )
  {
    mCoverageParentIdentifiers[coverageSummary.orderId] = QStringList() << coverageSummary.identifier << coverageSummary.title << coverageSummary.abstract;
  }
  QgsDebugMsgLevel( QStringLiteral( "coverage orderId = %1 identifier = %2" ).arg( coverageSummary.orderId ).arg( coverageSummary.identifier ), 2 );
}

void QgsWcsCapabilities::parseMetadataLink( const QDomElement &element, QgsWcsMetadataLinkProperty &metadataLink )
{
  QDomElement metadataElement = firstChild( element, QStringLiteral( "metadataLink" ) );

  if ( !metadataElement.isNull() )
  {
    metadataLink.metadataType = metadataElement.attribute( QStringLiteral( "metadataType" ) );
    metadataLink.xlinkHref = elementLink( metadataElement );
  }
}

QString QgsWcsCapabilities::elementLink( const QDomElement &element )
{
  if ( !element.isNull() )
  {
    return QUrl::fromEncoded( element.attribute( QStringLiteral( "xlink:href" ) ).toUtf8() ).toString();
  }

  return QString();
}

bool QgsWcsCapabilities::convertToDom( QByteArray const &xml )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );
  // Convert completed document into a Dom
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = mCapabilitiesDom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    mErrorTitle = tr( "Dom Exception" );
    mErrorFormat = QStringLiteral( "text/plain" );
    mError = tr( "Could not get WCS capabilities: %1 at line %2 column %3\nThis is probably due to an incorrect WCS Server URL.\nResponse was:\n\n%4" )
               .arg( errorMsg )
               .arg( errorLine )
               .arg( errorColumn )
               .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }
  return true;
}

bool QgsWcsCapabilities::parseDescribeCoverageDom10( QByteArray const &xml, QgsWcsCoverageSummary *coverage )
{
  QgsDebugMsgLevel( "coverage->identifier = " + coverage->identifier, 2 );
  if ( !convertToDom( xml ) )
    return false;

  QDomElement documentElement = mCapabilitiesDom.documentElement();

  QgsDebugMsgLevel( "testing tagName " + documentElement.tagName(), 2 );

  QString tagName = stripNS( documentElement.tagName() );
  if ( tagName != QLatin1String( "CoverageDescription" ) )
  {
    mErrorTitle = tr( "Dom Exception" );
    mErrorFormat = QStringLiteral( "text/plain" );
    mError = tr( "Could not get WCS capabilities in the expected format (DTD): no %1 found.\nThis might be due to an incorrect WCS Server URL.\nTag: %3\nResponse was:\n%4" )
               .arg( QStringLiteral( "CoverageDescription" ), documentElement.tagName(), QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  QDomElement coverageOfferingElement = firstChild( documentElement, QStringLiteral( "CoverageOffering" ) );

  if ( coverageOfferingElement.isNull() )
    return false;
  QDomElement supportedCRSsElement = firstChild( coverageOfferingElement, QStringLiteral( "supportedCRSs" ) );

  // requestResponseCRSs and requestCRSs + responseCRSs are alternatives
  // we try to parse one or the other
  QStringList crsList;
  crsList = domElementsTexts( coverageOfferingElement, QStringLiteral( "supportedCRSs.requestResponseCRSs" ) );
  if ( crsList.isEmpty() )
  {
    crsList = domElementsTexts( coverageOfferingElement, QStringLiteral( "supportedCRSs.requestCRSs" ) );
    crsList << domElementsTexts( coverageOfferingElement, QStringLiteral( "supportedCRSs.responseCRSs" ) );
  }

  // exclude invalid CRSs from the lists
  for ( const QString &crsid : std::as_const( crsList ) )
  {
    if ( QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsid ).isValid() )
    {
      coverage->supportedCrs << crsid;
    }
  }

  // TODO: requestCRSs, responseCRSs - must be then implemented also in provider
  //QgsDebugMsgLevel( "supportedCrs = " + coverage->supportedCrs.join( "," ), 2 );

  coverage->nativeCrs = domElementText( coverageOfferingElement, QStringLiteral( "supportedCRSs.nativeCRSs" ) );

  // may be GTiff, GeoTIFF, TIFF, GIF, ....
  coverage->supportedFormat = domElementsTexts( coverageOfferingElement, QStringLiteral( "supportedFormats.formats" ) );
  QgsDebugMsgLevel( "supportedFormat = " + coverage->supportedFormat.join( "," ), 2 );

  // spatialDomain and Grid/RectifiedGrid are optional according to specificationi.
  // If missing, we cannot get native resolution and size.
  QDomElement gridElement = domElement( coverageOfferingElement, QStringLiteral( "domainSet.spatialDomain.RectifiedGrid" ) );

  if ( gridElement.isNull() )
  {
    // Grid has also GridEnvelope from which we can get coverage size but it does not
    gridElement = domElement( coverageOfferingElement, QStringLiteral( "domainSet.spatialDomain.Grid" ) );
  }

  // If supportedCRSs.nativeCRSs is not defined we try to get it from RectifiedGrid
  if ( coverage->nativeCrs.isEmpty() )
  {
    QString crs = gridElement.attribute( QStringLiteral( "srsName" ) );
    if ( QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs ).isValid() )
    {
      coverage->nativeCrs = crs;
    }
  }

  if ( !gridElement.isNull() )
  {
    QList<int> low = parseInts( domElementText( gridElement, QStringLiteral( "limits.GridEnvelope.low" ) ) );
    QList<int> high = parseInts( domElementText( gridElement, QStringLiteral( "limits.GridEnvelope.high" ) ) );
    if ( low.size() == 2 && high.size() == 2 )
    {
      // low/high are indexes in grid -> size is height - low + 1
      double width = high[0] - low[0] + 1;
      double height = high[1] - low[1] + 1;
      if ( width > 0 && height > 0 )
      {
        coverage->width = width;
        coverage->height = height;
        coverage->hasSize = true;
      }
    }
    // RectifiedGrid has also gml:origin which we don't need I think (attention however
    // it should contain gml:Point but mapserver 6.0.3 / WCS 1.0.0 is using gml:pos instead)
    // RectifiedGrid also contains 2 gml:offsetVector which could be used to get resolution
    // but it should be sufficient to calc resolution from size

    // TODO: check if coverage is rotated, in that case probably treat as without size
    //       or recalc resolution from rotated grid to base CRS
  }

  QList<QDomElement> envelopeElements = domElements( coverageOfferingElement, QStringLiteral( "domainSet.spatialDomain.Envelope" ) );

  QgsDebugMsgLevel( QStringLiteral( "%1 envelopeElements found" ).arg( envelopeElements.size() ), 2 );

  const auto constEnvelopeElements = envelopeElements;
  for ( const QDomElement &el : constEnvelopeElements )
  {
    QString srsName = el.attribute( QStringLiteral( "srsName" ) );

    QList<QDomElement> posElements = domElements( el, QStringLiteral( "pos" ) );
    if ( posElements.size() != 2 )
    {
      QgsDebugError( QStringLiteral( "Wrong number of pos elements" ) );
      continue;
    }

    QList<double> low = parseDoubles( posElements.value( 0 ).text() );
    QList<double> high = parseDoubles( posElements.value( 1 ).text() );
    if ( low.size() == 2 && high.size() == 2 )
    {
      QgsRectangle box( low[0], low[1], high[0], high[1] );
      coverage->boundingBoxes.insert( srsName, box );
      QgsDebugMsgLevel( "Envelope: " + srsName + " : " + box.toString(), 2 );
    }
  }

  coverage->times = domElementsTexts( coverageOfferingElement, QStringLiteral( "domainSet.temporalDomain.timePosition" ) );

  QList<QDomElement> timePeriodElements = domElements( coverageOfferingElement, QStringLiteral( "domainSet.temporalDomain.timePeriod" ) );

  QgsDebugMsgLevel( QStringLiteral( "%1 timePeriod found" ).arg( timePeriodElements.size() ), 2 );

  const auto constTimePeriodElements = timePeriodElements;
  for ( const QDomElement &el : constTimePeriodElements )
  {
    QString beginPosition = domElementText( el, QStringLiteral( "beginPosition" ) );
    QString endPosition = domElementText( el, QStringLiteral( "endPosition" ) );
    QString timeResolution = domElementText( el, QStringLiteral( "timeResolution" ) );
    // Format used in request
    QString time = beginPosition + '/' + endPosition;
    if ( !timeResolution.isEmpty() )
    {
      time += '/' + timeResolution;
    }
    coverage->times << time;
  }

  // Find native bounding box
  if ( !coverage->nativeCrs.isEmpty() )
  {
    const auto boundingBoxes = coverage->boundingBoxes;
    for ( auto it = boundingBoxes.constBegin(); it != boundingBoxes.constEnd(); ++it )
    {
      if ( it.key() == coverage->nativeCrs )
      {
        coverage->nativeBoundingBox = it.value();
      }
    }
  }

  // NULL / no data values
  // TODO: handle multiple range sets
  const QStringList elements = domElementsTexts( coverageOfferingElement, "rangeSet.RangeSet.nullValue.singleValue" );
  for ( const QString &text : elements )
  {
    bool ok;
    double val = text.toDouble( &ok );
    if ( ok )
    {
      coverage->nullValues.append( val );
    }
  }

  coverage->described = true;

  return true;
}
// ------------------------ 1.1 ----------------------------------------------
bool QgsWcsCapabilities::parseDescribeCoverageDom11( QByteArray const &xml, QgsWcsCoverageSummary *coverage )
{
  QgsDebugMsgLevel( "coverage->identifier = " + coverage->identifier, 2 );
  if ( !convertToDom( xml ) )
    return false;

  QDomElement documentElement = mCapabilitiesDom.documentElement();

  QgsDebugMsgLevel( "testing tagName " + documentElement.tagName(), 2 );

  QString tagName = stripNS( documentElement.tagName() );
  if ( tagName != QLatin1String( "CoverageDescriptions" ) )
  {
    mErrorTitle = tr( "Dom Exception" );
    mErrorFormat = QStringLiteral( "text/plain" );
    mError = tr( "Could not get WCS capabilities in the expected format (DTD): no %1 found.\nThis might be due to an incorrect WCS Server URL.\nTag: %3\nResponse was:\n%4" )
               .arg( QStringLiteral( "CoverageDescriptions" ), documentElement.tagName(), QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  // Get image size, we can get it from BoundingBox with crs=urn:ogc:def:crs:OGC::imageCRS
  // but while at least one BoundingBox is mandatory, it does not have to be urn:ogc:def:crs:OGC::imageCRS
  // TODO: if BoundingBox with crs=urn:ogc:def:crs:OGC::imageCRS is not found,
  // we could calculate image size from GridCRS.GridOffsets (if available)
  QList<QDomElement> boundingBoxElements = domElements( documentElement, QStringLiteral( "CoverageDescription.Domain.SpatialDomain.BoundingBox" ) );

  QgsDebugMsgLevel( QStringLiteral( "%1 BoundingBox found" ).arg( boundingBoxElements.size() ), 2 );

  const auto constBoundingBoxElements = boundingBoxElements;
  for ( const QDomElement &el : constBoundingBoxElements )
  {
    QString authid = crsUrnToAuthId( el.attribute( QStringLiteral( "crs" ) ) );
    QList<double> low = parseDoubles( domElementText( el, QStringLiteral( "LowerCorner" ) ) );
    QList<double> high = parseDoubles( domElementText( el, QStringLiteral( "UpperCorner" ) ) );

    if ( low.size() != 2 && high.size() != 2 )
      continue;

    if ( el.attribute( QStringLiteral( "crs" ) ) == QLatin1String( "urn:ogc:def:crs:OGC::imageCRS" ) )
    {
      coverage->width = ( int ) ( high[0] - low[0] + 1 );
      coverage->height = ( int ) ( high[1] - low[1] + 1 );
      coverage->hasSize = true;
    }
    else
    {
      QgsRectangle box;
      QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( authid );
      if ( crs.isValid() && crs.hasAxisInverted() )
      {
        box = QgsRectangle( low[1], low[0], high[1], high[0] );
      }
      else
      {
        box = QgsRectangle( low[0], low[1], high[0], high[1] );
      }
      coverage->boundingBoxes.insert( authid, box );
      QgsDebugMsgLevel( "crs: " + crs.userFriendlyIdentifier() + QString( " axisInverted = %1" ).arg( crs.hasAxisInverted() ), 2 );
      QgsDebugMsgLevel( "BoundingBox: " + authid + " : " + box.toString(), 2 );
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2" ).arg( coverage->width ).arg( coverage->height ), 2 );

  // Each georectified coverage should have GridCRS
  QDomElement gridCRSElement = domElement( documentElement, QStringLiteral( "CoverageDescription.Domain.SpatialDomain.GridCRS" ) );

  if ( !gridCRSElement.isNull() )
  {
    QString crsUrn = firstChildText( gridCRSElement, QStringLiteral( "GridBaseCRS" ) );
    coverage->nativeCrs = crsUrnToAuthId( crsUrn );
    QgsDebugMsgLevel( "nativeCrs = " + coverage->nativeCrs, 2 );

    // TODO: consider getting coverage size from GridOffsets (resolution)
    // if urn:ogc:def:crs:OGC::imageCRS BoundingBox was not found
  }

  coverage->times = domElementsTexts( documentElement, QStringLiteral( "CoverageDescription.Domain.TemporalDomain.timePosition" ) );

  QList<QDomElement> timePeriodElements = domElements( documentElement, QStringLiteral( "CoverageDescription.Domain.TemporalDomain.timePeriod" ) );

  QgsDebugMsgLevel( QStringLiteral( "%1 timePeriod found" ).arg( timePeriodElements.size() ), 2 );

  const auto constTimePeriodElements = timePeriodElements;
  for ( const QDomElement &el : constTimePeriodElements )
  {
    QString beginPosition = domElementText( el, QStringLiteral( "beginTime" ) );
    QString endPosition = domElementText( el, QStringLiteral( "endTime" ) );
    QString timeResolution = domElementText( el, QStringLiteral( "timeResolution" ) );
    // Format used in request
    QString time = beginPosition + '/' + endPosition;
    if ( !timeResolution.isEmpty() )
    {
      time += '/' + timeResolution;
    }
    coverage->times << time;
  }

  // NULL / no data values
  // TODO: handle multiple fields / ranges (?)
  const QStringList elements = domElementsTexts( documentElement, "CoverageDescription.Range.Field.NullValue" );
  for ( const QString &text : elements )
  {
    bool ok;
    double val = text.toDouble( &ok );
    if ( ok )
    {
      coverage->nullValues.append( val );
    }
  }

  QStringList formats = domElementsTexts( documentElement, QStringLiteral( "CoverageDescription.SupportedFormat" ) );
  // There could be formats from GetCapabilities
  if ( !formats.isEmpty() )
  {
    coverage->supportedFormat = formats;
  }


  QStringList crss = domElementsTexts( documentElement, QStringLiteral( "CoverageDescription.SupportedCRS" ) );
  QSet<QString> authids; // Set, in case one CRS is in more formats (URN, non URN)
  const auto constCrss = crss;
  for ( const QString &crs : constCrss )
  {
    authids.insert( crsUrnToAuthId( crs ) );
  }
  if ( !authids.isEmpty() )
  {
    coverage->supportedCrs = qgis::setToList( authids );
  }

  coverage->described = true;

  return true;
}


void QgsWcsCapabilities::parseCoverageSummary( const QDomElement &element, QgsWcsCoverageSummary &coverageSummary, QgsWcsCoverageSummary *parent )
{
  coverageSummary.orderId = ++mCoverageCount;

  coverageSummary.identifier = firstChildText( element, QStringLiteral( "Identifier" ) );
  coverageSummary.title = firstChildText( element, QStringLiteral( "Title" ) );
  coverageSummary.abstract = firstChildText( element, QStringLiteral( "Abstract" ) );

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = stripNS( nodeElement.tagName() );
      QgsDebugMsgLevel( tagName + " : " + nodeElement.text(), 2 );

      if ( tagName == QLatin1String( "SupportedFormat" ) )
      {
        // image/tiff, ...
        // Formats may be here (UMN Mapserver) or may not (GeoServer)
        coverageSummary.supportedFormat << nodeElement.text();
      }
      else if ( tagName == QLatin1String( "SupportedCRS" ) )
      {
        // TODO: SupportedCRS may be URL referencing a document
        coverageSummary.supportedCrs << crsUrnToAuthId( nodeElement.text() );
      }
      else if ( tagName == QLatin1String( "WGS84BoundingBox" ) )
      {
        QList<double> low = parseDoubles( domElementText( nodeElement, QStringLiteral( "LowerCorner" ) ) );
        QList<double> high = parseDoubles( domElementText( nodeElement, QStringLiteral( "UpperCorner" ) ) );

        if ( low.size() == 2 && high.size() == 2 )
        {
          coverageSummary.wgs84BoundingBox = QgsRectangle( low[0], low[1], high[0], high[1] );
        }
      }
    }
    node = node.nextSibling();
  }
  //QgsDebugMsgLevel( "supportedFormat = " + coverageSummary.supportedFormat.join( "," ), 2 );

  // We collected params to be inherited, do children
  node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = stripNS( nodeElement.tagName() );

      if ( tagName == QLatin1String( "CoverageSummary" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      Nested coverage." ), 2 );

        QgsWcsCoverageSummary subCoverageSummary;

        initCoverageSummary( subCoverageSummary );

        // Inherit
        subCoverageSummary.supportedCrs = coverageSummary.supportedCrs;
        subCoverageSummary.supportedFormat = coverageSummary.supportedFormat;

        parseCoverageSummary( nodeElement, subCoverageSummary, &coverageSummary );
        subCoverageSummary.valid = true;

        coverageSummary.coverageSummary.push_back( subCoverageSummary );
      }
    }
    node = node.nextSibling();
  }

  if ( parent && parent->orderId > 1 ) // ignore Contents to put them on top level
  {
    QgsDebugMsgLevel( QStringLiteral( "coverage orderId = %1 identifier = %2 has parent %3" ).arg( coverageSummary.orderId ).arg( coverageSummary.identifier ).arg( parent->orderId ), 2 );
    mCoverageParents[coverageSummary.orderId] = parent->orderId;
  }

  if ( !coverageSummary.identifier.isEmpty() )
  {
    QgsDebugMsgLevel( "add coverage " + coverageSummary.identifier + " to supported", 2 );
    mCoveragesSupported.push_back( coverageSummary );
  }

  if ( !coverageSummary.coverageSummary.empty() )
  {
    mCoverageParentIdentifiers[coverageSummary.orderId] = QStringList() << coverageSummary.identifier << coverageSummary.title << coverageSummary.abstract;
  }
  QgsDebugMsgLevel( QStringLiteral( "coverage orderId = %1 identifier = %2" ).arg( coverageSummary.orderId ).arg( coverageSummary.identifier ), 2 );
}

void QgsWcsCapabilities::coverageParents( QMap<int, int> &parents, QMap<int, QStringList> &parentNames ) const
{
  parents = mCoverageParents;
  parentNames = mCoverageParentIdentifiers;
}

void QgsWcsCapabilities::initCoverageSummary( QgsWcsCoverageSummary &coverageSummary )
{
  coverageSummary.valid = false;
  coverageSummary.described = false;
  coverageSummary.width = 0;
  coverageSummary.height = 0;
  coverageSummary.hasSize = false;
}

QString QgsWcsCapabilities::lastErrorTitle()
{
  return mErrorTitle;
}

QString QgsWcsCapabilities::lastError()
{
  QgsDebugMsgLevel( "returning '" + mError + "'.", 2 );
  return mError;
}

QString QgsWcsCapabilities::lastErrorFormat()
{
  return mErrorFormat;
}

bool QgsWcsCapabilities::setAuthorization( QNetworkRequest &request ) const
{
  if ( !mUri.authConfigId().isEmpty() )
  {
    return QgsApplication::authManager()->updateNetworkRequest( request, mUri.authConfigId() );
  }
  else if ( !mUri.username().isEmpty() && !mUri.password().isEmpty() )
  {
    QgsDebugMsgLevel( "setAuthorization " + mUri.username(), 2 );
    request.setRawHeader( "Authorization", "Basic " + QStringLiteral( "%1:%2" ).arg( mUri.username(), mUri.password() ).toLatin1().toBase64() );
  }
  return true;
}

bool QgsWcsCapabilities::setAuthorizationReply( QNetworkReply *reply ) const
{
  if ( !mUri.authConfigId().isEmpty() )
  {
    return QgsApplication::authManager()->updateNetworkReply( reply, mUri.authConfigId() );
  }
  return true;
}

void QgsWcsCapabilities::showMessageBox( const QString &title, const QString &text )
{
  QgsMessageOutput *message = QgsMessageOutput::createMessageOutput();
  message->setTitle( title );
  message->setMessage( text, QgsMessageOutput::MessageText );
  message->showMessage();
}

QgsWcsCoverageSummary QgsWcsCapabilities::coverage( QString const &identifier )
{
  QgsWcsCoverageSummary *coverageSummaryPointer = coverageSummary( identifier );
  if ( coverageSummaryPointer )
    return *coverageSummaryPointer;

  QgsWcsCoverageSummary coverageSummary;
  initCoverageSummary( coverageSummary );
  return coverageSummary;
}

QgsWcsCoverageSummary *QgsWcsCapabilities::coverageSummary( QString const &identifier, QgsWcsCoverageSummary *parent )
{
  QgsDebugMsgLevel( "theIdentifier = " + identifier, 5 );
  if ( !parent )
  {
    parent = &( mCapabilities.contents );
  }

  for ( QVector<QgsWcsCoverageSummary>::iterator c = parent->coverageSummary.begin(); c != parent->coverageSummary.end(); ++c )
  {
    if ( c->identifier == identifier )
    {
      return &( *c );
    }
    else
    {
      // search sub coverages
      QgsWcsCoverageSummary *subCoverage = coverageSummary( identifier, &( *c ) );
      if ( subCoverage )
      {
        return subCoverage;
      }
    }
  }
  return nullptr;
}

QList<QgsWcsCoverageSummary> QgsWcsCapabilities::coverages() const
{
  return coverageSummaries();
}

QList<QgsWcsCoverageSummary> QgsWcsCapabilities::coverageSummaries( const QgsWcsCoverageSummary *parent ) const
{
  QList<QgsWcsCoverageSummary> list;
  if ( !parent )
  {
    parent = &( mCapabilities.contents );
  }

  for ( QVector<QgsWcsCoverageSummary>::const_iterator c = parent->coverageSummary.constBegin(); c != parent->coverageSummary.constEnd(); ++c )
  {
    list.append( *c );
    list.append( coverageSummaries( &( *c ) ) );
  }
  return list;
}
