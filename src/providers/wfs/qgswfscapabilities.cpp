/***************************************************************************
    qgswfscapabilities.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfscapabilities.h"
#include "qgsexpression.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsogcutils.h"
#include <QDomDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QStringList>

static const QString WFS_NAMESPACE = "http://www.opengis.net/wfs";

QgsWFSCapabilities::QgsWFSCapabilities( QString theUri ) :
    //QObject( parent ),
    //mConnName( connName ),
    mCapabilitiesReply( 0 ),
    mErrorCode( QgsWFSCapabilities::NoError )
{
  mUri.setEncodedUri( theUri );
  QgsDebugMsg( "theUri = " + theUri );
  mBaseUrl = prepareUri( mUri.param( "url" ) );

  QgsDebugMsg( "mBaseUrl = " + mBaseUrl );

  //find out the server URL
  /*
  QSettings settings;
  QString key = "/Qgis/connections-wfs/" + mConnName + "/url";
  mUri = settings.value( key ).toString();
  QgsDebugMsg( QString( "url is: %1" ).arg( mUri ) );

  //make a GetCapabilities request
  //modify mUri to add '?' or '&' at the end if it is not already there
  if ( !( mUri.contains( "?" ) ) )
  {
    mUri.append( "?" );
  }
  else if (( mUri.right( 1 ) != "?" ) && ( mUri.right( 1 ) != "&" ) )
  {
    mUri.append( "&" );
  }
  */
}

QString QgsWFSCapabilities::prepareUri( QString uri )
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

QString QgsWFSCapabilities::uriGetCapabilities() const
{
  return mBaseUrl + "SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.0.0";
}

QString QgsWFSCapabilities::uriDescribeFeatureType( const QString& typeName ) const
{
  return mBaseUrl + "SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=" + typeName;
}

QString QgsWFSCapabilities::uriGetFeature( QString typeName, QString crsString, QString filter, QgsRectangle bBox ) const
{
  //get CRS
  if ( !crsString.isEmpty() )
  {
    crsString.prepend( "&SRSNAME=" );
  }

  QString filterString;

  //if the xml comes from the dialog, it needs to be a string to pass the validity test
  if ( filter.startsWith( "'" ) && filter.endsWith( "'" ) && filter.size() > 1 )
  {
    filter.chop( 1 );
    filter.remove( 0, 1 );
  }

  if ( !filter.isEmpty() )
  {
    //test if filterString is already an OGC filter xml
    QDomDocument filterDoc;
    if ( !filterDoc.setContent( filter ) )
    {
      //if not, if must be a QGIS expression
      QgsExpression filterExpression( filter );
      QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( filterExpression, filterDoc );
      if ( !filterElem.isNull() )
      {
        filterDoc.appendChild( filterElem );
      }

    }
    filterString = "&FILTER=" + filterDoc.toString();
  }

  QString bBoxString;
  if ( !bBox.isEmpty() )
  {
    bBoxString = QString( "&BBOX=%1,%2,%3,%4" )
                 .arg( bBox.xMinimum(), 0, 'f' )
                 .arg( bBox.yMinimum(), 0, 'f' )
                 .arg( bBox.xMaximum(), 0, 'f' )
                 .arg( bBox.yMaximum(), 0, 'f' );
  }

  QString uri = mBaseUrl;

  //add a wfs layer to the map
  uri += "SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=" + typeName + crsString + bBoxString + filterString;
  QgsDebugMsg( uri );
  return uri;
}


void QgsWFSCapabilities::requestCapabilities()
{
  mErrorCode = QgsWFSCapabilities::NoError;
  mErrorMessage.clear();

  QNetworkRequest request( uriGetCapabilities() );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );
  connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
}

void QgsWFSCapabilities::capabilitiesReplyFinished()
{
  QNetworkReply* reply = mCapabilitiesReply;

  reply->deleteLater();
  mCapabilitiesReply = 0;

  // handle network errors
  if ( reply->error() != QNetworkReply::NoError )
  {
    mErrorCode = QgsWFSCapabilities::NetworkError;
    mErrorMessage = reply->errorString();
    emit gotCapabilities();
    return;
  }

  // handle HTTP redirects
  QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  if ( !redirect.isNull() )
  {
    QgsDebugMsg( "redirecting to " + redirect.toUrl().toString() );
    QNetworkRequest request( redirect.toUrl() );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

    connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
    return;
  }

  QByteArray buffer = reply->readAll();

  QgsDebugMsg( "parsing capabilities: " + buffer );

  // parse XML
  QString capabilitiesDocError;
  QDomDocument capabilitiesDocument;
  if ( !capabilitiesDocument.setContent( buffer, true, &capabilitiesDocError ) )
  {
    mErrorCode = QgsWFSCapabilities::XmlError;
    mErrorMessage = capabilitiesDocError;
    emit gotCapabilities();
    return;
  }

  QDomElement doc = capabilitiesDocument.documentElement();

  // hangle exceptions
  if ( doc.tagName() == "ExceptionReport" )
  {
    QDomNode ex = doc.firstChild();
    QString exc = ex.toElement().attribute( "exceptionCode", "Exception" );
    QDomElement ext = ex.firstChild().toElement();
    mErrorCode = QgsWFSCapabilities::ServerExceptionError;
    mErrorMessage = exc + ": " + ext.firstChild().nodeValue();
    emit gotCapabilities();
    return;
  }

  mCaps.clear();

  //test wfs version
  QString version = capabilitiesDocument.documentElement().attribute( "version" );
  if ( version != "1.0.0" && version != "1.0" )
  {
    mErrorCode = WFSVersionNotSupported;
    mErrorMessage = tr( "Either the WFS server does not support WFS version 1.0.0 or the WFS url is wrong" );
    emit gotCapabilities();
    return;
  }

  // get the <FeatureType> elements
  QDomNodeList featureTypeList = capabilitiesDocument.elementsByTagNameNS( WFS_NAMESPACE, "FeatureType" );
  for ( unsigned int i = 0; i < featureTypeList.length(); ++i )
  {
    FeatureType featureType;
    QDomElement featureTypeElem = featureTypeList.at( i ).toElement();

    //Name
    QDomNodeList nameList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "Name" );
    if ( nameList.length() > 0 )
    {
      featureType.name = nameList.at( 0 ).toElement().text();
    }
    //Title
    QDomNodeList titleList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "Title" );
    if ( titleList.length() > 0 )
    {
      featureType.title = titleList.at( 0 ).toElement().text();
    }
    //Abstract
    QDomNodeList abstractList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "Abstract" );
    if ( abstractList.length() > 0 )
    {
      featureType.abstract = abstractList.at( 0 ).toElement().text();
    }

    //DefaultSRS is always the first entry in the feature srs list
    QDomNodeList defaultCRSList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "DefaultSRS" );
    if ( defaultCRSList.length() > 0 )
    {
      featureType.crslist.append( defaultCRSList.at( 0 ).toElement().text() );
    }

    //OtherSRS
    QDomNodeList otherCRSList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "OtherSRS" );
    for ( unsigned int i = 0; i < otherCRSList.length(); ++i )
    {
      featureType.crslist.append( otherCRSList.at( i ).toElement().text() );
    }

    //Support <SRS> for compatibility with older versions
    QDomNodeList srsList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "SRS" );
    for ( unsigned int i = 0; i < srsList.length(); ++i )
    {
      featureType.crslist.append( srsList.at( i ).toElement().text() );
    }

    mCaps.featureTypes.append( featureType );
  }

  emit gotCapabilities();
}

