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
#include <typeinfo>

#define WCS_THRESHOLD 200  // time to wait for an answer without emitting dataChanged() 
#include "qgslogger.h"
#include "qgswcscapabilities.h"
#include "qgsowsconnection.h"

#include <cmath>

#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsnetworkaccessmanager.h"
#include <qgsmessageoutput.h>
#include <qgsmessagelog.h>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>

#if QT_VERSION >= 0x40500
#include <QNetworkDiskCache>
#endif

#include <QUrl>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QPixmap>
#include <QSet>
#include <QSettings>
#include <QEventLoop>
#include <QCoreApplication>
#include <QTime>

#ifdef _MSC_VER
#include <float.h>
#define isfinite(x) _finite(x)
#endif

#ifdef QGISDEBUG
#include <QFile>
#include <QDir>
#endif

QgsWcsCapabilities::QgsWcsCapabilities( QgsDataSourceURI const &theUri ):
    mUri( theUri ),
    mCoverageCount( 0 )
{
  QgsDebugMsg( "uri = " + mUri.encodedUri() );

  retrieveServerCapabilities();
}

QgsWcsCapabilities::QgsWcsCapabilities( ):
    mCoverageCount( 0 )
{
}

QgsWcsCapabilities::~QgsWcsCapabilities()
{
  QgsDebugMsg( "deconstructing." );
}

// TODO: return if successful
void QgsWcsCapabilities::setUri( QgsDataSourceURI const &theUri )
{
  mUri = theUri;

  clear();
  retrieveServerCapabilities( );
}

QString QgsWcsCapabilities::prepareUri( QString uri )
{
  if ( !uri.contains( "?" ) )
  {
    uri.append( "?" );
  }
  else if ( uri.right( 1 ) != "?" && uri.right( 1 ) != "&" )
  {
    uri.append( "&" );
  }

  return uri;
}

QgsWcsCapabilitiesProperty QgsWcsCapabilities::capabilities()
{
  return mCapabilities;
}

bool QgsWcsCapabilities::supportedCoverages( QVector<QgsWcsCoverageSummary> &coverageSummary )
{
  QgsDebugMsg( "Entering." );

  // Allow the provider to collect the capabilities first.
  if ( !retrieveServerCapabilities() )
  {
    return false;
  }

  coverageSummary = mCoveragesSupported;

  QgsDebugMsg( "Exiting." );

  return true;
}

QString QgsWcsCapabilities::getCoverageUrl() const
{
  QString url = mCapabilities.getCoverageGetUrl;
  if ( url.isEmpty() )
  {
    url = mUri.param( "url" );
  }
  return url;
}

bool QgsWcsCapabilities::sendRequest( QString const & url )
{
  QgsDebugMsg( "url = " + url );
  mError = "";
  QNetworkRequest request( url );
  setAuthorization( request );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  QgsDebugMsg( QString( "getcapabilities: %1" ).arg( url ) );
  mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

  connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
  connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( capabilitiesReplyProgress( qint64, qint64 ) ) );

  while ( mCapabilitiesReply )
  {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
  }

  if ( mCapabilitiesResponse.isEmpty() )
  {
    if ( mError.isEmpty() )
    {
      mErrorFormat = "text/plain";
      mError = tr( "empty capabilities document" );
    }
    return false;
  }

  if ( mCapabilitiesResponse.startsWith( "<html>" ) ||
       mCapabilitiesResponse.startsWith( "<HTML>" ) )
  {
    mErrorFormat = "text/html";
    mError = mCapabilitiesResponse;
    return false;
  }
  return true;
}

void QgsWcsCapabilities::clear()
{
  mCoverageCount = 0;
  mCoveragesSupported.clear();
  QgsWcsCapabilitiesProperty c;
  mCapabilities = c;
}

bool QgsWcsCapabilities::retrieveServerCapabilities( )
{
  clear();
  QStringList versions;

  // 1.0.0 - VERSION
  // 1.1.0 - AcceptedVersions (not supported by UMN Mapserver 6.0.3 - defaults to 1.1.1
  versions << "AcceptVersions=1.1.0,1.0.0" << "VERSION=1.0.0";

  foreach( QString v, versions )
  {
    if ( retrieveServerCapabilities( v ) )
    {
      return true;
    }
  }

  return false;
}

bool QgsWcsCapabilities::retrieveServerCapabilities( QString preferredVersion )
{
  clear();

  QString url = prepareUri( mUri.param( "url" ) ) + "SERVICE=WCS&REQUEST=GetCapabilities";

  if ( !preferredVersion.isEmpty() )
  {
    url += "&" + preferredVersion;
  }

  if ( ! sendRequest( url ) ) { return false; }

  QgsDebugMsg( "Converting to Dom." );

  bool domOK;
  domOK = parseCapabilitiesDom( mCapabilitiesResponse, mCapabilities );

  if ( !domOK )
  {
    // We had an Dom exception -
    // mErrorTitle and mError are pre-filled by parseCapabilitiesDom

    mError += tr( "\nTried URL: %1" ).arg( url );

    QgsDebugMsg( "!domOK: " + mError );

    return false;
  }

  return true;
}

bool QgsWcsCapabilities::describeCoverage( QString const &identifier, bool forceRefresh )
{
  QgsDebugMsg( " identifier = " + identifier );

  QgsWcsCoverageSummary *coverage = coverageSummary( identifier );
  if ( !coverage )
  {
    QgsDebugMsg( "coverage not found" );
    return false;
  }

  if ( coverage->described && ! forceRefresh ) return true;

  QString url = prepareUri( mUri.param( "url" ) ) + "SERVICE=WCS&REQUEST=DescribeCoverage&COVERAGE=" + coverage->identifier;
  url += "&VERSION=" + mVersion;

  if ( ! sendRequest( url ) ) { return false; }

  QgsDebugMsg( "Converting to Dom." );

  bool domOK = false;
  if ( mVersion.startsWith( "1.0" ) )
  {
    domOK = parseDescribeCoverageDom10( mCapabilitiesResponse, coverage );
  }
  else if ( mVersion.startsWith( "1.1" ) )
  {
    domOK = parseDescribeCoverageDom11( mCapabilitiesResponse, coverage );
  }

  if ( !domOK )
  {
    // We had an Dom exception -
    // mErrorTitle and mError are pre-filled by parseCapabilitiesDom

    mError += tr( "\nTried URL: %1" ).arg( url );

    QgsDebugMsg( "!domOK: " + mError );

    return false;
  }

  QgsDebugMsg( "supportedFormat = " + coverage->supportedFormat.join( "," ) );

  return true;
}

void QgsWcsCapabilities::capabilitiesReplyFinished()
{
  if ( mCapabilitiesReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mCapabilitiesReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      emit statusChanged( tr( "Capabilities request redirected." ) );

      QNetworkRequest request( redirect.toUrl() );
      setAuthorization( request );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

      mCapabilitiesReply->deleteLater();
      QgsDebugMsg( QString( "redirected getcapabilities: %1" ).arg( redirect.toString() ) );
      mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

      connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
      connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( capabilitiesReplyProgress( qint64, qint64 ) ) );
      return;
    }

    mCapabilitiesResponse = mCapabilitiesReply->readAll();

    if ( mCapabilitiesResponse.isEmpty() )
    {
      mErrorFormat = "text/plain";
      mError = tr( "empty of capabilities: %1" ).arg( mCapabilitiesReply->errorString() );
    }
  }
  else
  {
    mErrorFormat = "text/plain";
    mError = tr( "Download of capabilities failed: %1" ).arg( mCapabilitiesReply->errorString() );
    QgsMessageLog::logMessage( mError, tr( "WCS" ) );
    mCapabilitiesResponse.clear();
  }

  mCapabilitiesReply->deleteLater();
  mCapabilitiesReply = 0;
}

void QgsWcsCapabilities::capabilitiesReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of capabilities downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  emit statusChanged( msg );
}

QString QgsWcsCapabilities::stripNS( const QString & name )
{
  return name.contains( ":" ) ? name.section( ':', 1 ) : name;
}

bool QgsWcsCapabilities::parseCapabilitiesDom( QByteArray const &xml, QgsWcsCapabilitiesProperty &capabilities )
{
  QgsDebugMsg( "Entered." );
#ifdef QGISDEBUG
  QFile file( QDir::tempPath() + "/qgis-wcs-capabilities.xml" );
  if ( file.open( QIODevice::WriteOnly ) )
  {
    file.write( xml );
    file.close();
  }
#endif

  if ( ! convertToDom( xml ) ) return false;

  QDomElement docElem = mCapabilitiesDom.documentElement();

  // Assert that the DTD is what we expected (i.e. a WCS Capabilities document)
  QgsDebugMsg( "testing tagName " + docElem.tagName() );

  QString tagName = stripNS( docElem.tagName() );
  if (
    // We don't support 1.0, but try WCS_Capabilities tag to get version
    tagName != "WCS_Capabilities" && // 1.0
    tagName != "Capabilities"  // 1.1, tags seen: Capabilities, wcs:Capabilities
  )
  {
    mErrorTitle = tr( "Dom Exception" );
    mErrorFormat = "text/plain";
    mError = tr( "Could not get WCS capabilities in the expected format (DTD): no %1 found.\nThis might be due to an incorrect WCS Server URL.\nTag:%3\nResponse was:\n%4" )
             .arg( "Capabilities" )
             .arg( docElem.tagName() )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  capabilities.version = docElem.attribute( "version" );
  mVersion = capabilities.version;

  if ( !mVersion.startsWith( "1.0" ) && !mVersion.startsWith( "1.1" ) )
  {
    mErrorTitle = tr( "Version not supported" );
    mErrorFormat = "text/plain";
    mError = tr( "WCS server version %1 is not supported by Quantum GIS (supported versions: 1.0.0, 1.1.0, 1.1.2)" )
             .arg( mVersion );

    QgsLogger::debug( "WCS version: " + mError );

    return false;
  }

  if ( mVersion.startsWith( "1.0" ) )
  {
    capabilities.title = domElementText( docElem, "Service.name" );
    capabilities.abstract = domElementText( docElem, "Service.description" );
    // There is also "label" in 1.0

    capabilities.getCoverageGetUrl = domElement( docElem, "Capability.Request.GetCoverage.DCPType.HTTP.Get.OnlineResource" ).attribute( "xlink:href" );

    parseContentMetadata( domElement( docElem, "ContentMetadata" ), capabilities.contents );
  }
  else if ( mVersion.startsWith( "1.1" ) )
  {
    capabilities.title = domElementText( docElem, "ServiceIdentification.Title" );
    capabilities.abstract = domElementText( docElem, "ServiceIdentification.Abstract" );

    QList<QDomElement> operationElements = domElements( docElem, "OperationsMetadata.Operation" );
    foreach( QDomElement el, operationElements )
    {
      if ( el.attribute( "name" ) == "GetCoverage" )
      {
        capabilities.getCoverageGetUrl = domElement( el, "DCP.HTTP.Get" ).attribute( "xlink:href" );
      }
    }

    parseCoverageSummary( domElement( docElem, "Contents" ), capabilities.contents );
  }

  return true;
}

QDomElement QgsWcsCapabilities::firstChild( const QDomElement &element, const QString & name )
{
  QDomNode n1 = element.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement();
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );
      if ( tagName == name )
      {
        return el;
      }
    }
    n1 = n1.nextSibling();
  }
  return QDomElement();
}

QString QgsWcsCapabilities::firstChildText( const QDomElement &element, const QString & name )
{
  QDomElement el = firstChild( element, name );
  if ( !el.isNull() )
  {
    return el.text();
  }
  return QString();
}

QList<QDomElement> QgsWcsCapabilities::domElements( const QDomElement &element, const QString & path )
{
  QList<QDomElement> list;

  QStringList names = path.split( "." );
  if ( names.size() == 0 ) return list;
  QString name = names.value( 0 );
  names.removeFirst();

  QDomNode n1 = element.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement();
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );
      if ( tagName == name )
      {
        if ( names.size() == 0 )
        {
          list.append( el );
        }
        else
        {
          list.append( domElements( el,  names.join( "." ) ) );
        }
      }
    }
    n1 = n1.nextSibling();
  }

  return list;
}

QStringList QgsWcsCapabilities::domElementsTexts( const QDomElement &element, const QString &path )
{
  QStringList list;
  QList<QDomElement> elems = domElements( element, path );

  foreach( QDomElement el, elems )
  {
    list << el.text();
  }
  return list;
}

QDomElement QgsWcsCapabilities::domElement( const QDomElement &element, const QString & path )
{
  QStringList names = path.split( "." );
  if ( names.size() == 0 ) return QDomElement();

  QDomElement el = firstChild( element, names.value( 0 ) );
  if ( names.size() == 1 || el.isNull() )
  {
    return el;
  }
  names.removeFirst();
  return domElement( el, names.join( "." ) );
}

QString QgsWcsCapabilities::domElementText( const QDomElement &element, const QString & path )
{
  QDomElement el = domElement( element, path );
  return el.text();
}

QList<int> QgsWcsCapabilities::parseInts( const QString &text )
{
  QList<int> list;
  foreach( QString s, text.split( " " ) )
  {
    int i;
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
  foreach( QString s, text.split( " " ) )
  {
    int i;
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
  QString authid;

  // URN format: urn:ogc:def:objectType:authority:version:code
  // URN example: urn:ogc:def:crs:EPSG::4326
  QStringList urn = text.split( ":" );
  if ( urn.size() == 7 )
  {
    authid = urn.value( 4 ) + ":" + urn.value( 6 );
  }

  return authid;
}

// ------------------------ 1.0 ----------------------------------------------


void QgsWcsCapabilities::parseContentMetadata( QDomElement const & e, QgsWcsCoverageSummary &coverageSummary )
{
  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement();
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );

      if ( tagName == "CoverageOfferingBrief" )
      {
        QgsWcsCoverageSummary subCoverageSummary;

        initCoverageSummary( subCoverageSummary );

        parseCoverageOfferingBrief( el, subCoverageSummary, &coverageSummary );

        subCoverageSummary.valid = true;

        coverageSummary.coverageSummary.push_back( subCoverageSummary );
      }
    }
    n1 = n1.nextSibling();
  }
}

void QgsWcsCapabilities::parseCoverageOfferingBrief( QDomElement const & e, QgsWcsCoverageSummary &coverageSummary, QgsWcsCoverageSummary *parent )
{
  Q_UNUSED( parent );
  QgsDebugMsg( "Entered" );
  coverageSummary.orderId = ++mCoverageCount;

  coverageSummary.identifier = firstChildText( e, "name" );
  coverageSummary.title = firstChildText( e, "label" );
  coverageSummary.abstract = firstChildText( e, "description" );

  QList<QDomElement> posElements = domElements( e, "lonLatEnvelope.pos" );
  if ( posElements.size() != 2 )
  {
    QgsDebugMsg( "Wrong number of pos elements" );
  }
  else
  {
    QList<double> low = parseDoubles( posElements.value( 0 ).text() );
    QList<double> high = parseDoubles( posElements.value( 1 ).text() );
    if ( low.size() == 2 && high.size() == 2 )
    {
      coverageSummary.wgs84BoundingBox = QgsRectangle( low[0], low[1], high[0], high[1] ) ;
      QgsDebugMsg( "wgs84BoundingBox = " + coverageSummary.wgs84BoundingBox.toString() );
    }
  }

  if ( !coverageSummary.identifier.isEmpty() )
  {
    QgsDebugMsg( "add coverage " + coverageSummary.identifier + " to supported" );
    mCoveragesSupported.push_back( coverageSummary );
  }

  if ( !coverageSummary.coverageSummary.empty() )
  {
    mCoverageParentIdentifiers[ coverageSummary.orderId ] = QStringList() << coverageSummary.identifier << coverageSummary.title << coverageSummary.abstract;
  }
  QgsDebugMsg( QString( "coverage orderId = %1 identifier = %2" ).arg( coverageSummary.orderId ).arg( coverageSummary.identifier ) );
}

bool QgsWcsCapabilities::convertToDom( QByteArray const &xml )
{
  QgsDebugMsg( "Entered." );
  // Convert completed document into a Dom
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = mCapabilitiesDom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    mErrorTitle = tr( "Dom Exception" );
    mErrorFormat = "text/plain";
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
  QgsDebugMsg( "coverage->identifier = " + coverage->identifier );
  if ( ! convertToDom( xml ) ) return false;

  QDomElement docElem = mCapabilitiesDom.documentElement();

  QgsDebugMsg( "testing tagName " + docElem.tagName() );

  QString tagName = stripNS( docElem.tagName() );
  if ( tagName != "CoverageDescription" )
  {
    mErrorTitle = tr( "Dom Exception" );
    mErrorFormat = "text/plain";
    mError = tr( "Could not get WCS capabilities in the expected format (DTD): no %1 found.\nThis might be due to an incorrect WCS Server URL.\nTag:%3\nResponse was:\n%4" )
             .arg( "CoverageDescription" )
             .arg( docElem.tagName() )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  QDomElement coverageOfferingElement = firstChild( docElem, "CoverageOffering" );

  if ( coverageOfferingElement.isNull() ) return false;
  QDomElement supportedCRSsElement = firstChild( coverageOfferingElement, "supportedCRSs" );

  // requestResponseCRSs and requestCRSs + responseCRSs are alternatives
  coverage->supportedCrs = domElementsTexts( coverageOfferingElement, "supportedCRSs.requestResponseCRSs" );
  // TODO: requestCRSs, responseCRSs - must be then implemented also in provider
  QgsDebugMsg( "supportedCrs = " + coverage->supportedCrs.join( "," ) );

  coverage->nativeCrs = domElementText( coverageOfferingElement, "supportedCRSs.nativeCRSs" );

  // may be GTiff, GeoTIFF, TIFF, GIF, ....
  coverage->supportedFormat = domElementsTexts( coverageOfferingElement, "supportedFormats.formats" );
  QgsDebugMsg( "supportedFormat = " + coverage->supportedFormat.join( "," ) );

  // spatialDomain and Grid/RectifiedGrid are optional according to specificationi.
  // If missing, we cannot get native resolution and size.
  QDomElement gridElement = domElement( coverageOfferingElement, "domainSet.spatialDomain.RectifiedGrid" );

  if ( gridElement.isNull() )
  {
    // Grid has also GridEnvelope from which we can get coverage size but it does not
    gridElement = domElement( coverageOfferingElement, "domainSet.spatialDomain.Grid" );
  }

  if ( !gridElement.isNull() )
  {
    QList<int> low = parseInts( domElementText( gridElement, "limits.GridEnvelope.low" ) );
    QList<int> high = parseInts( domElementText( gridElement, "limits.GridEnvelope.high" ) );
    if ( low.size() == 2 && high.size() == 2 )
    {
      double width = high[0] - low[0] + 1;
      double height = high[1] - low[1] + 1;
      if ( width > 0 && height > 0 )
      {
        coverage->width = width;
        coverage->height = height;
        coverage->hasSize = true;
      }
    }
    // RectifiedGrid has also gml:origin which we dont need I think (attention however
    // it should contain gml:Point but mapserver 6.0.3 / WCS 1.0.0 is using gml:pos instead)
    // RectifiedGrid also contains 2 gml:offsetVector which could be used to get resolution
    // but it should be sufficient to calc resolution from size

    // TODO: check if coverage is rotated, in that case probably treat as without size
    //       or recalc resolution from rotated grid to base CRS
  }

  QList<QDomElement> envelopeElements = domElements( coverageOfferingElement, "domainSet.spatialDomain.Envelope" );

  QgsDebugMsg( QString( "%1 envelopeElements found" ).arg( envelopeElements.size() ) );

  foreach( QDomElement el, envelopeElements )
  {
    QString srsName = el.attribute( "srsName" );

    QList<QDomElement> posElements = domElements( el, "pos" );
    if ( posElements.size() != 2 )
    {
      QgsDebugMsg( "Wrong number of pos elements" );
      continue;
    }

    QList<double> low = parseDoubles( posElements.value( 0 ).text() );
    QList<double> high = parseDoubles( posElements.value( 1 ).text() );
    if ( low.size() == 2 && high.size() == 2 )
    {
      QgsRectangle box( low[0], low[1], high[0], high[1] ) ;
      coverage->boundingBoxes.insert( srsName, box );
      QgsDebugMsg( "Envelope: " + srsName + " : " + box.toString() );
    }
  }

  // Find native bounding box
  if ( !coverage->nativeCrs.isEmpty() )
  {
    foreach( QString srsName, coverage->boundingBoxes.keys() )
    {
      if ( srsName == coverage->nativeCrs )
      {
        coverage->nativeBoundingBox = coverage->boundingBoxes.value( srsName );
      }
    }
  }

  coverage->described = true;

  return true;
}
// ------------------------ 1.1 ----------------------------------------------
bool QgsWcsCapabilities::parseDescribeCoverageDom11( QByteArray const &xml, QgsWcsCoverageSummary *coverage )
{
  QgsDebugMsg( "coverage->identifier = " + coverage->identifier );
  if ( ! convertToDom( xml ) ) return false;

  QDomElement docElem = mCapabilitiesDom.documentElement();

  QgsDebugMsg( "testing tagName " + docElem.tagName() );

  QString tagName = stripNS( docElem.tagName() );
  if ( tagName != "CoverageDescriptions" )
  {
    mErrorTitle = tr( "Dom Exception" );
    mErrorFormat = "text/plain";
    mError = tr( "Could not get WCS capabilities in the expected format (DTD): no %1 found.\nThis might be due to an incorrect WCS Server URL.\nTag:%3\nResponse was:\n%4" )
             .arg( "CoverageDescriptions" )
             .arg( docElem.tagName() )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  // Get image size, we can get it from BoundingBox with crs=urn:ogc:def:crs:OGC::imageCRS
  // but while at least one BoundingBox is mandatory, it does not have to be urn:ogc:def:crs:OGC::imageCRS
  // TODO: if BoundingBox with crs=urn:ogc:def:crs:OGC::imageCRS is not found,
  // we could calculate image size from GridCRS.GridOffsets (if available)
  QList<QDomElement> boundingBoxElements = domElements( docElem, "CoverageDescription.Domain.SpatialDomain.BoundingBox" );

  QgsDebugMsg( QString( "%1 BoundingBox found" ).arg( boundingBoxElements.size() ) );

  foreach( QDomElement el, boundingBoxElements )
  {
    QString authid = crsUrnToAuthId( el.attribute( "crs" ) );
    QList<double> low = parseDoubles( domElementText( el, "LowerCorner" ) );
    QList<double> high = parseDoubles( domElementText( el, "UpperCorner" ) );

    if ( low.size() != 2 && high.size() != 2 ) continue;

    if ( el.attribute( "crs" ) == "urn:ogc:def:crs:OGC::imageCRS" )
    {
      coverage->width = ( int )( high[0] - low[0] + 1 );
      coverage->height = ( int )( high[1] - low[1] + 1 );
      coverage->hasSize = true;
    }
    else
    {
      QgsRectangle box( low[0], low[1], high[0], high[1] ) ;
      coverage->boundingBoxes.insert( authid, box );
      QgsDebugMsg( "BoundingBox: " + authid + " : " + box.toString() );
    }
  }
  QgsDebugMsg( QString( "width = %1 height = %2" ).arg( coverage->width ).arg( coverage->height ) );

  // Each georectified coverage should have GridCRS
  QDomElement gridCRSElement = domElement( docElem, "CoverageDescription.Domain.SpatialDomain.GridCRS" );

  if ( !gridCRSElement.isNull() )
  {
    QString crsUrn = firstChildText( gridCRSElement, "GridBaseCRS" );
    coverage->nativeCrs = crsUrnToAuthId( crsUrn );
    QgsDebugMsg( "nativeCrs = " + coverage->nativeCrs );

    // TODO: consider getting coverage size from GridOffsets (resolution)
    // if urn:ogc:def:crs:OGC::imageCRS BoundingBox was not found
  }

  coverage->described = true;

  return true;
}


void QgsWcsCapabilities::parseCoverageSummary( QDomElement const & e, QgsWcsCoverageSummary &coverageSummary, QgsWcsCoverageSummary *parent )
{
  coverageSummary.orderId = ++mCoverageCount;

  coverageSummary.identifier = firstChildText( e, "Identifier" );
  coverageSummary.title = firstChildText( e, "Title" );
  coverageSummary.abstract = firstChildText( e, "Abstract" );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement();
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );
      QgsDebugMsg( tagName + " : " + el.text() );

      if ( tagName == "SupportedFormat" )
      {
        // image/tiff, ...
        coverageSummary.supportedFormat << el.text();
      }
      else if ( tagName == "SupportedCRS" )
      {
        // TODO: SupportedCRS may be URL referencing a document
        coverageSummary.supportedCrs << crsUrnToAuthId( el.text() );
      }
      else if ( tagName == "WGS84BoundingBox" )
      {
        QList<double> low = parseDoubles( domElementText( el, "LowerCorner" ) );
        QList<double> high = parseDoubles( domElementText( el, "UpperCorner" ) );

        if ( low.size() == 2 && high.size() == 2 )
        {
          coverageSummary.wgs84BoundingBox = QgsRectangle( low[0], low[1], high[0], high[1] );
        }
      }
    }
    n1 = n1.nextSibling();
  }
  QgsDebugMsg( "supportedFormat = " + coverageSummary.supportedFormat.join( "," ) );

  // We collected params to be inherited, do children
  n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement();
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );

      if ( tagName == "CoverageSummary" )
      {
        QgsDebugMsg( "      Nested coverage." );

        QgsWcsCoverageSummary subCoverageSummary;

        initCoverageSummary( subCoverageSummary );

        // Inherit
        subCoverageSummary.supportedCrs = coverageSummary.supportedCrs;
        subCoverageSummary.supportedFormat = coverageSummary.supportedFormat;

        parseCoverageSummary( el, subCoverageSummary, &coverageSummary );
        subCoverageSummary.valid = true;

        coverageSummary.coverageSummary.push_back( subCoverageSummary );
      }
    }
    n1 = n1.nextSibling();
  }

  if ( parent && parent->orderId > 1 ) // ignore Contents to put them on top level
  {
    QgsDebugMsg( QString( "coverage orderId = %1 identifier = %2 has parent %3" ).arg( coverageSummary.orderId ).arg( coverageSummary.identifier ).arg( parent->orderId ) );
    mCoverageParents[ coverageSummary.orderId ] = parent->orderId;
  }

  if ( !coverageSummary.identifier.isEmpty() )
  {
    QgsDebugMsg( "add coverage " + coverageSummary.identifier + " to supported" );
    mCoveragesSupported.push_back( coverageSummary );
  }

  if ( !coverageSummary.coverageSummary.empty() )
  {
    mCoverageParentIdentifiers[ coverageSummary.orderId ] = QStringList() << coverageSummary.identifier << coverageSummary.title << coverageSummary.abstract;
  }
  QgsDebugMsg( QString( "coverage orderId = %1 identifier = %2" ).arg( coverageSummary.orderId ).arg( coverageSummary.identifier ) );

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
  QgsDebugMsg( "returning '" + mError  + "'." );
  return mError;
}

QString QgsWcsCapabilities::lastErrorFormat()
{
  return mErrorFormat;
}

void QgsWcsCapabilities::setAuthorization( QNetworkRequest &request ) const
{
  QgsDebugMsg( "entered" );
  if ( mUri.hasParam( "username" ) && mUri.hasParam( "password" ) )
  {
    QgsDebugMsg( "setAuthorization " + mUri.param( "username" ) );
    request.setRawHeader( "Authorization", "Basic " + QString( "%1:%2" ).arg( mUri.param( "username" ) ).arg( mUri.param( "password" ) ).toAscii().toBase64() );
  }
}

void QgsWcsCapabilities::showMessageBox( const QString& title, const QString& text )
{
  QgsMessageOutput *message = QgsMessageOutput::createMessageOutput();
  message->setTitle( title );
  message->setMessage( text, QgsMessageOutput::MessageText );
  message->showMessage();
}

QgsWcsCoverageSummary QgsWcsCapabilities::coverage( QString const & theIdentifier )
{
  QgsWcsCoverageSummary * cp = coverageSummary( theIdentifier );
  QgsDebugMsg( "supportedFormat = " + ( *cp ).supportedFormat.join( "," ) );
  if ( cp ) return *cp;

  QgsWcsCoverageSummary c;
  initCoverageSummary( c );
  QgsDebugMsg( "supportedFormat = " + c.supportedFormat.join( "," ) );
  return c;
}

QgsWcsCoverageSummary* QgsWcsCapabilities::coverageSummary( QString const & theIdentifier, QgsWcsCoverageSummary* parent )
{
  QgsDebugMsgLevel( "theIdentifier = " + theIdentifier, 5 );
  if ( !parent )
  {
    parent = &( mCapabilities.contents );
  }

  for ( QVector<QgsWcsCoverageSummary>::iterator c = parent->coverageSummary.begin(); c != parent->coverageSummary.end(); ++c )
  {
    if ( c->identifier == theIdentifier )
    {
      return c;
    }
    else
    {
      // search sub coverages
      QgsWcsCoverageSummary * sc = coverageSummary( theIdentifier, c );
      if ( sc )
      {
        return sc;
      }
    }
  }
  return 0;
}

QList<QgsWcsCoverageSummary> QgsWcsCapabilities::coverages( )
{
  return coverageSummaries( );
}

QList<QgsWcsCoverageSummary> QgsWcsCapabilities::coverageSummaries( QgsWcsCoverageSummary* parent )
{
  QList<QgsWcsCoverageSummary> list;
  if ( !parent )
  {
    parent = &( mCapabilities.contents );
  }

  for ( QVector<QgsWcsCoverageSummary>::iterator c = parent->coverageSummary.begin(); c != parent->coverageSummary.end(); ++c )
  {
    list.append( *c );
    list.append( coverageSummaries( c ) );
  }
  return list;
}
