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

/*
QgsWcsCapabilities::QgsWcsCapabilities( QString const &theUri )
{
  mUri.setEncodedUri( theUri );

  QgsDebugMsg( "theUri = " + theUri );
}
*/

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

void QgsWcsCapabilities::setUri( QgsDataSourceURI const &theUri )
{
  mUri = theUri;
  mCoverageCount = 0;
  mCoveragesSupported.clear();
  QgsWcsCapabilitiesProperty c;
  mCapabilities = c;

  retrieveServerCapabilities( true );
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
  QString url = mCapabilities.operationsMetadata.getCoverage.dcp.http.get.xlinkHref;
  if ( url.isEmpty() )
  {
    url = mUri.param( "url" );
  }
  return url;
}

bool QgsWcsCapabilities::retrieveServerCapabilities( bool forceRefresh )
{
  QgsDebugMsg( "entering." );

  if ( mCapabilitiesResponse.isNull() || forceRefresh )
  {
    // Check if user tried to force version
    QString userVersion = QUrl( mUri.param( "url" ) ).queryItemValue( "VERSION" );
    if ( !userVersion.isEmpty() && !userVersion.startsWith( "1.1." ) )
    {
      mErrorTitle = tr( "Version not supported" );
      mErrorFormat = "text/plain";
      mError = tr( "The version %1 specified in connection URL parameter VERSION is not supported by QGIS" ).arg( userVersion );
      return false;
    }

    QString url = prepareUri( mUri.param( "url" ) ) + "SERVICE=WCS&REQUEST=GetCapabilities&VERSION=1.1.0";

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
  }

  QgsDebugMsg( "exiting." );

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
  QgsDebugMsg( "entering." );

#ifdef QGISDEBUG
  QFile file( QDir::tempPath() + "/qgis-wcs-capabilities.xml" );
  if ( file.open( QIODevice::WriteOnly ) )
  {
    file.write( xml );
    file.close();
  }
#endif

  // Convert completed document into a Dom
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = mCapabilitiesDom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    mErrorTitle = tr( "Dom Exception" );
    mErrorFormat = "text/plain";
    mError = tr( "Could not get WCS capabilities: %1 at line %2 column %3\nThis is probably due to an incorrect WMS Server URL.\nResponse was:\n\n%4" )
             .arg( errorMsg )
             .arg( errorLine )
             .arg( errorColumn )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  QDomElement docElem = mCapabilitiesDom.documentElement();

  // Assert that the DTD is what we expected (i.e. a WCS Capabilities document)
  QgsDebugMsg( "testing tagName " + docElem.tagName() );

  if (
    // We don't support 1.0, but try WCS_Capabilities tag to get version
    docElem.tagName() != "WCS_Capabilities" && // 1.0
    docElem.tagName() != "Capabilities"  // 1.1
  )
  {
    mErrorTitle = tr( "Dom Exception" );
    mErrorFormat = "text/plain";
    mError = tr( "Could not get WCS capabilities in the expected format (DTD): no %1 found.\nThis might be due to an incorrect WMS Server URL.\nTag:%3\nResponse was:\n%4" )
             .arg( "Capabilities" )
             .arg( docElem.tagName() )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  capabilities.version = docElem.attribute( "version" );
  mVersion = capabilities.version;

  if ( !mVersion.startsWith( "1.1." ) )
  {
    mErrorTitle = tr( "Version not supported" );
    mErrorFormat = "text/plain";
    mError = tr( "Could not get WCS capabilities in the expected version 1.1.\nResponse version was: %1" )
             .arg( mVersion );

    QgsLogger::debug( "WCS version: " + mError );

    return false;
  }


  // Start walking through XML.
  QDomNode n = docElem.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if ( !e.isNull() )
    {
      QString tagName = stripNS( e.tagName() );
      if ( tagName == "ServiceIdentification" )
      {
        QgsDebugMsg( "  ServiceIdentification." );
        parseServiceIdentification( e, capabilities.serviceIdentification );
      }
      else if ( tagName == "OperationsMetadata" )
      {
        QgsDebugMsg( "  OperationsMetadata." );
        parseOperationsMetadata( e, capabilities.operationsMetadata );
      }
      else if ( tagName == "Contents" )
      {
        QgsDebugMsg( "  Contents." );
        parseCoverageSummary( e, capabilities.contents );
      }
    }
    n = n.nextSibling();
  }

  QgsDebugMsg( "exiting." );

  return true;
}


void QgsWcsCapabilities::parseServiceIdentification( QDomElement const & e, QgsWcsServiceIdentification &serviceIdentification )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement(); // try to convert the node to an element.
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );

      if ( tagName == "Title" )
      {
        serviceIdentification.title = el.text();
      }
      else if ( tagName == "Abstract" )
      {
        serviceIdentification.abstract = el.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWcsCapabilities::parseHttp( QDomElement const & e, QgsWcsHTTP& http )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement(); // try to convert the node to an element.
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );
      if ( tagName == "Get" )
      {
        QgsDebugMsg( "      Get." );
        http.get.xlinkHref = el.attribute( "xlink:href" );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWcsCapabilities::parseDcp( QDomElement const & e, QgsWcsDCP& dcp )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement(); // try to convert the node to an element.
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );
      if ( tagName == "HTTP" )
      {
        QgsDebugMsg( "      HTTP." );
        parseHttp( el, dcp.http );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWcsCapabilities::parseOperation( QDomElement const & e, QgsWcsOperation& operation )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement(); // try to convert the node to an element.
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );

      if ( tagName == "DCP" )
      {
        QgsDebugMsg( "      DCP." );
        parseDcp( el, operation.dcp );
      }
      // TODO Parameter version
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWcsCapabilities::parseOperationsMetadata( QDomElement const & e,  QgsWcsOperationsMetadata &operationsMetadata )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement(); // try to convert the node to an element.
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );

      if ( tagName == "GetCoverage" )
      {
        QgsDebugMsg( "      GetCoverage." );
        parseOperation( el, operationsMetadata.getCoverage );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWcsCapabilities::parseCoverageSummary( QDomElement const & e, QgsWcsCoverageSummary &coverageSummary, QgsWcsCoverageSummary *parent )
{
  QgsDebugMsg( "entering." );

  coverageSummary.orderId = ++mCoverageCount;

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement(); // try to convert the node to an element.
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );
      QgsDebugMsg( tagName + " : " + el.text() );

      if ( tagName == "Identifier" )
      {
        coverageSummary.identifier = el.text();
        QgsDebugMsg( "Identifier = " +  el.text() );
      }
      else if ( tagName == "Title" )
      {
        coverageSummary.title = el.text();
      }
      else if ( tagName == "Abstract" )
      {
        coverageSummary.abstract = el.text();
      }
      else if ( tagName == "SupportedFormat" )
      {
        coverageSummary.supportedFormat << el.text();
      }
      else if ( tagName == "SupportedCRS" )
      {
        // TODO: SupportedCRS may be URL referencing a document
        // URN format: urn:ogc:def:objectType:authority:version:code
        // URN example: urn:ogc:def:crs:EPSG::4326
        QStringList urn = el.text().split( ":" );
        if ( urn.size() == 7 )
        {
          coverageSummary.supportedCrs << urn.value( 4 ) + ":" + urn.value( 6 );
        }
      }
      else if ( tagName == "WGS84BoundingBox" )
      {
        QDomElement wBoundLongitudeElem, eBoundLongitudeElem, sBoundLatitudeElem, nBoundLatitudeElem;

        QStringList lower = n1.namedItem( "ows:LowerCorner" ).toElement().text().split( QRegExp( " +" ) );
        QStringList upper = n1.namedItem( "ows:UpperCorner" ).toElement().text().split( QRegExp( " +" ) );

        double w, e, s, n;
        bool wOk, eOk, sOk, nOk;
        w = lower.value( 0 ).toDouble( &wOk );
        s = lower.value( 1 ).toDouble( &sOk );
        e = upper.value( 0 ).toDouble( &eOk );
        n = upper.value( 1 ).toDouble( &nOk );
        if ( wOk && eOk && sOk && nOk )
        {
          coverageSummary.wgs84BoundingBox = QgsRectangle( w, s, e, n );
        }
      }
    }
    n1 = n1.nextSibling();
  }

  // We collected params to be inherited, do children
  n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement(); // try to convert the node to an element.
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );

      if ( tagName == "CoverageSummary" )
      {
        QgsDebugMsg( "      Nested coverage." );

        QgsWcsCoverageSummary subCoverageSummary;

        // Inherit
        subCoverageSummary.supportedCrs = coverageSummary.supportedCrs;
        subCoverageSummary.supportedFormat = coverageSummary.supportedFormat;

        parseCoverageSummary( el, subCoverageSummary, &coverageSummary );

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

  QgsDebugMsg( "exiting." );
}

void QgsWcsCapabilities::coverageParents( QMap<int, int> &parents, QMap<int, QStringList> &parentNames ) const
{
  parents = mCoverageParents;
  parentNames = mCoverageParentIdentifiers;
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

QgsWcsCoverageSummary QgsWcsCapabilities::coverageSummary( QString const & theIdentifier )
{
  QgsDebugMsg( "entered" );
  foreach( QgsWcsCoverageSummary c, mCoveragesSupported )
  {
    if ( c.identifier == theIdentifier ) return c;
  }
  QgsWcsCoverageSummary c;
  return c;
}
