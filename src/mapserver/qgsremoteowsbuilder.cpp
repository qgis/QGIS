/***************************************************************************
                              qgsremoteowsbuilder.cpp
                              -----------------------
  begin                : July, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatasourceuri.h"
#include "qgsremoteowsbuilder.h"
#include "qgshttptransaction.h"
#include "qgslogger.h"
#include "qgsmslayercache.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include <QDomElement>
#include <QTemporaryFile>

QgsRemoteOWSBuilder::QgsRemoteOWSBuilder( const QMap<QString, QString>& parameterMap )
    : QgsMSLayerBuilder()
    , mParameterMap( parameterMap )
{

}

QgsRemoteOWSBuilder::QgsRemoteOWSBuilder()
    : QgsMSLayerBuilder()
{

}

QgsRemoteOWSBuilder::~QgsRemoteOWSBuilder()
{

}

QgsMapLayer* QgsRemoteOWSBuilder::createMapLayer(
  const QDomElement& elem,
  const QString& layerName,
  QList<QTemporaryFile*>& filesToRemove,
  QList<QgsMapLayer*>& layersToRemove, bool allowCaching ) const
{
  if ( elem.isNull() )
  {
    return 0;
  }

  //parse service element
  QDomNode serviceNode = elem.namedItem( "Service" );
  if ( serviceNode.isNull() )
  {
    QgsDebugMsg( "No <Service> node found, returning 0" );
    return 0; //service node is necessary
  }

  //parse OnlineResource element
  QDomNode onlineResourceNode = elem.namedItem( "OnlineResource" );
  if ( onlineResourceNode.isNull() )
  {
    QgsDebugMsg( "No <OnlineResource> element, returning 0" );
    return 0;
  }

  //get uri
  QDomElement onlineResourceElement = onlineResourceNode.toElement();
  QString url = onlineResourceElement.attribute( "href" );

  QgsMapLayer* result = 0;
  QString serviceName = serviceNode.toElement().text();

  //append missing ? or & at the end of the url, but only for WFS and WMS
  if ( serviceName == "WFS" || serviceName == "WMS" )
  {
    if ( !url.endsWith( "?" ) && !url.endsWith( "&" ) )
    {
      if ( url.contains( "?" ) )
      {
        url.append( "&" );
      }
      else
      {
        url.append( "?" );
      }
    }
  }


  if ( serviceName == "WFS" )
  {
    //support for old format where type is explicitly given and not part of url
    QString tname = onlineResourceElement.attribute( "type" );
    if ( !tname.isEmpty() )
    {
      url.append( "SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=" + tname );
    }

    if ( allowCaching )
    {
      result = QgsMSLayerCache::instance()->searchLayer( url, layerName );
    }
    if ( result )
    {
      return result;
    }
    result = new QgsVectorLayer( url, layerNameFromUri( url ), "WFS" );
    if ( result->isValid() )
    {
      if ( allowCaching )
      {
        QgsMSLayerCache::instance()->insertLayer( url, layerName, result );
      }
      else
      {
        layersToRemove.push_back( result );
      }
    }
  }
  else if ( serviceName == "WMS" )
  {
    result = wmsLayerFromUrl( url, layerName, layersToRemove, allowCaching );
  }
  else if ( serviceName == "WCS" )
  {
    QgsDebugMsg( "Trying to get WCS layer" );
    result = wcsLayerFromUrl( url, layerName, filesToRemove, layersToRemove );
  }
  else if ( serviceName == "SOS" )
  {
    result = sosLayer( elem, url, layerName, layersToRemove, allowCaching );
  }

  if ( !result || !result->isValid() )
  {
    QgsDebugMsg( "Error, maplayer is 0 or invalid" );
    if ( result )
    {
      delete result;
    }
    return 0;
  }

  return result;
}

QgsRasterLayer* QgsRemoteOWSBuilder::wmsLayerFromUrl( const QString& url, const QString& layerName, QList<QgsMapLayer*>& layersToRemove, bool allowCaching ) const
{
  QgsDebugMsg( "Entering" );
  QgsRasterLayer* result = 0;
  QString baseUrl, format, crs;
  QStringList layerList, styleList;

  if ( allowCaching )
  {
    result = dynamic_cast<QgsRasterLayer*>( QgsMSLayerCache::instance()->searchLayer( url, layerName ) );
  }

  if ( result )
  {
    return result;
  }

  QStringList urlList = url.split( "?" );
  if ( urlList.size() < 2 )
  {
    return 0;
  }
  baseUrl = urlList.at( 0 );
  QStringList paramList = urlList.at( 1 ).split( "&" );

  QStringList::const_iterator paramIt;
  for ( paramIt = paramList.constBegin(); paramIt != paramList.constEnd(); ++paramIt )
  {
    if ( paramIt->startsWith( "http", Qt::CaseInsensitive ) )
    {
      baseUrl = paramIt->split( "=" ).at( 1 );
    }
    else if ( paramIt->startsWith( "FORMAT", Qt::CaseInsensitive ) )
    {
      format = paramIt->split( "=" ).at( 1 );
    }
    else if ( paramIt->startsWith( "CRS", Qt::CaseInsensitive ) )
    {
      crs = paramIt->split( "=" ).at( 1 );
    }
    else if ( paramIt->startsWith( "LAYERS", Qt::CaseInsensitive ) )
    {
      layerList = paramIt->split( "=" ).at( 1 ).split( "," );
    }
    else if ( paramIt->startsWith( "STYLES", Qt::CaseInsensitive ) )
    {
      styleList = paramIt->split( "=" ).at( 1 ).split( "," );
    }
  }

  QgsDebugMsg( "baseUrl: " + baseUrl );
  QgsDebugMsg( "format: " + format );
  QgsDebugMsg( "crs: " + crs );
  QgsDebugMsg( "layerList first item: " + layerList.at( 0 ) );
  QgsDebugMsg( "styleList first item: " + styleList.at( 0 ) );

  QgsDataSourceURI uri;
  uri.setParam( "url", baseUrl );
  uri.setParam( "format", format );
  uri.setParam( "crs", crs );
  uri.setParam( "layers", layerList );
  uri.setParam( "styles", styleList );
  result = new QgsRasterLayer( uri.encodedUri(), "", "wms" );
  if ( !result->isValid() )
  {
    return 0;
  }

  //insert into cache
  if ( allowCaching )
  {
    QgsMSLayerCache::instance()->insertLayer( url, layerName, result );
  }
  else
  {
    layersToRemove.push_back( result );
  }
  return result;
}

QgsRasterLayer* QgsRemoteOWSBuilder::wcsLayerFromUrl( const QString &url,
    const QString &layerName,
    QList<QTemporaryFile*> &filesToRemove,
    QList<QgsMapLayer*> &layersToRemove,
    bool allowCaching ) const
{
  Q_UNUSED( layerName );
  Q_UNUSED( allowCaching );
  QgsDebugMsg( "Entering" );

  //write server url and coverage name to a temporary file
  QString fileName = createTempFile();
  if ( fileName.isEmpty() )
  {
    return 0;
  }

  QFile tempFile( fileName );

  QTemporaryFile* tmpFile = new QTemporaryFile();
  if ( !tmpFile->open() )
  {
    delete tmpFile;
    return 0;
  }

  filesToRemove.push_back( tmpFile ); //make sure the temporary file gets deleted after each request

  QgsDebugMsg( "opening successful" );
  QgsDebugMsg( "url: " + url );
  //extract server url and coverage name from string
  QStringList serverSplit = url.split( "?" );
  if ( serverSplit.size() < 2 )
  {
    QgsDebugMsg( "error, no '?' contained in url" );
    return 0;
  }
  QString serverUrl = serverSplit.at( 0 );
  QString request = serverSplit.at( 1 );
  QStringList parameterSplit = request.split( "&" );
  QString coverageName;
  QString format;
  for ( int i = 0; i < parameterSplit.size(); ++i )
  {
    if ( parameterSplit.at( i ).startsWith( "COVERAGE", Qt::CaseInsensitive ) )
    {
      coverageName = parameterSplit.at( i ).split( "=" ).at( 1 );
    }
    else if ( parameterSplit.at( i ).startsWith( "FORMAT", Qt::CaseInsensitive ) )
    {
      format = parameterSplit.at( i ).split( "=" ).at( 1 );
    }
  }

  if ( coverageName.isEmpty() )
  {
    QgsDebugMsg( "coverage name is empty" );
    return 0;
  }

  if ( format.isEmpty() )
  {
    format = "GeoTIFF"; //use geotiff as default
  }

  QgsDebugMsg( "wcs server url: " + serverUrl );
  QgsDebugMsg( "coverage name: " + coverageName );

  //fetch WCS layer in the current resolution as geotiff
  QString wcsRequest = serverUrl + "?SERVICE=WCS&VERSION=1.0.0&REQUEST=GetCoverage&COVERAGE=" + coverageName + "&FORMAT=" + format;

  //CRS (or SRS)
  QString crs = mParameterMap.value( "CRS", mParameterMap.value( "SRS" ) );
  if ( crs.isEmpty() )
  {
    QgsDebugMsg( "No CRS or SRS parameter found for wcs layer, returning 0" );
    return 0;
  }
  wcsRequest += "&CRS=" + crs;

  //width
  QString width = mParameterMap.value( "WIDTH" );
  if ( width.isEmpty() )
  {
    QgsDebugMsg( "No WIDTH parameter found for wcs layer, returning 0" );
    return 0;
  }
  wcsRequest += "&WIDTH=" + width;

  //height
  QString height = mParameterMap.value( "HEIGHT" );
  if ( height.isEmpty() )
  {
    QgsDebugMsg( "No HEIGHT parameter found for wcs layer, returning 0" );
    return 0;
  }
  wcsRequest += "&HEIGHT=" + height;

  //bbox
  QString bbox = mParameterMap.value( "BBOX" );
  if ( bbox.isEmpty() )
  {
    QgsDebugMsg( "No BBOX parameter found for wcs layer, returning 0" );
    return 0;
  }
  wcsRequest += "&BBOX=" + bbox;

  QgsDebugMsg( "WCS request is: " + wcsRequest );

  //make request and store byte array into temporary file
  QgsHttpTransaction httpTransaction( wcsRequest );
  QByteArray result;
  if ( !httpTransaction.getSynchronously( result ) )
  {
    return 0;
  }

  QDataStream tempFileStream( &tempFile );
  tempFileStream.writeRawData( result.data(), result.size() );
  tempFile.close();

  QgsRasterLayer* rl = new QgsRasterLayer( fileName, layerNameFromUri( fileName ) );
  layersToRemove.push_back( rl ); //make sure the layer gets deleted after each request
  return rl;
}

QgsVectorLayer* QgsRemoteOWSBuilder::sosLayer( const QDomElement& remoteOWSElem, const QString& url, const QString& layerName, QList<QgsMapLayer*>& layersToRemove, bool allowCaching ) const
{
  //url for sos provider is: "url=... method=... xml=....
  QString method = remoteOWSElem.attribute( "method", "POST" ); //possible GET/POST/SOAP

  //search for <LayerSensorObservationConstraints> node that is sibling of remoteOSW element
  //parent element of <remoteOWS> (normally <UserLayer>)
  QDomElement parentElem = remoteOWSElem.parentNode().toElement();
  if ( parentElem.isNull() )
  {
    return 0;
  }


  //Root element of the request (can be 'GetObservation' or 'GetObservationById' at the moment, probably also 'GetFeatureOfInterest' or 'GetFeatureOfInterestTime' in the future)
  QDomElement requestRootElem;

  QDomNodeList getObservationNodeList = parentElem.elementsByTagName( "GetObservation" );
  if ( getObservationNodeList.size() > 0 )
  {
    requestRootElem = getObservationNodeList.at( 0 ).toElement();
  }
  else //GetObservationById?
  {
    QDomNodeList getObservationByIdNodeList = parentElem.elementsByTagName( "GetObservationById" );
    if ( getObservationByIdNodeList.size() > 0 )
    {
      requestRootElem = getObservationByIdNodeList.at( 0 ).toElement();
    }
  }

  if ( requestRootElem.isNull() )
  {
    return 0;
  }

  QDomDocument requestDoc;
  QDomNode newDocRoot = requestDoc.importNode( requestRootElem, true );
  requestDoc.appendChild( newDocRoot );

  QString providerUrl = "url=" + url + " method=" + method + " xml=" + requestDoc.toString();

  //check if layer is already in cache
  QgsVectorLayer* sosLayer = 0;
  if ( allowCaching )
  {
    sosLayer = dynamic_cast<QgsVectorLayer*>( QgsMSLayerCache::instance()->searchLayer( providerUrl, layerName ) );
    if ( sosLayer )
    {
      return sosLayer;
    }
  }

  sosLayer = new QgsVectorLayer( providerUrl, "Sensor layer", "SOS" );

  if ( !sosLayer->isValid() )
  {
    delete sosLayer;
    return 0;
  }
  else
  {
    if ( allowCaching )
    {
      QgsMSLayerCache::instance()->insertLayer( providerUrl, layerName, sosLayer );
    }
    else
    {
      layersToRemove.push_back( sosLayer );
    }
  }
  return sosLayer;
}
