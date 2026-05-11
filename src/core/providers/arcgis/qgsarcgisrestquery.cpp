/***************************************************************************
    qgsarcgisrestquery.cpp
    ----------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarcgisrestquery.h"

#include <ranges>

#include "qgsapplication.h"
#include "qgsarcgisrestutils.h"
#include "qgsauthmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsvariantutils.h"

#include <QCryptographicHash>
#include <QFile>
#include <QImageReader>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

#include "moc_qgsarcgisrestquery.cpp"

using namespace Qt::StringLiterals;

Qgis::ArcGisRestServiceType QgsArcGisRestQueryUtils::sniffServiceTypeFromUrl( const QUrl &url )
{
  if ( url.isEmpty() || !url.isValid() )
  {
    return Qgis::ArcGisRestServiceType::Unknown;
  }

  const QString path = url.path();
  if ( path.isEmpty() )
  {
    return Qgis::ArcGisRestServiceType::Unknown;
  }
  const QStringList pathSegments = path.split( '/', Qt::SkipEmptyParts );

  // iterate backwards through the URL segments.
  // we want to catch both root services (.../FeatureServer)
  // and layer-specific URLs (.../FeatureServer/0 or .../FeatureServer/query)
  for ( const QString &segment : pathSegments | std::ranges::views::reverse )
  {
    if ( segment.compare( "FeatureServer"_L1, Qt::CaseInsensitive ) == 0 )
    {
      return Qgis::ArcGisRestServiceType::FeatureServer;
    }
    if ( segment.compare( "MapServer"_L1, Qt::CaseInsensitive ) == 0 )
    {
      return Qgis::ArcGisRestServiceType::MapServer;
    }
    if ( segment.compare( "ImageServer"_L1, Qt::CaseInsensitive ) == 0 )
    {
      return Qgis::ArcGisRestServiceType::ImageServer;
    }
    if ( segment.compare( "SceneServer"_L1, Qt::CaseInsensitive ) == 0 )
    {
      return Qgis::ArcGisRestServiceType::SceneServer;
    }
  }

  return Qgis::ArcGisRestServiceType::Unknown;
}

Qgis::ArcGisRestServiceType QgsArcGisRestQueryUtils::sniffServiceTypeFromJson( const QVariantMap &json )
{
  if ( json.empty() )
  {
    return Qgis::ArcGisRestServiceType::Unknown;
  }

  // try to sniff an imageserver
  if ( json.contains( u"pixelSizeX"_s )
       || json.contains( u"bandCount"_s )
       || json.contains( u"mensurationCapabilities"_s )
       || json.value( u"serviceDataType"_s ).toString().startsWith( "esriImageService"_L1, Qt::CaseInsensitive ) )
  {
    return Qgis::ArcGisRestServiceType::ImageServer;
  }

  // try to sniff a mapserver
  if ( json.contains( u"mapName"_s ) || json.contains( u"singleFusedMapCache"_s ) ) // note that imageservers also have this tag, hence why we checked for those first
  {
    return Qgis::ArcGisRestServiceType::MapServer;
  }

  // try to sniff FeatureServer
  if ( json.contains( u"hasVersionedData"_s ) || json.contains( u"syncEnabled"_s ) || json.contains( u"allowGeometryUpdates"_s ) || json.contains( u"supportsDisconnectedEditing"_s ) )
  {
    return Qgis::ArcGisRestServiceType::FeatureServer;
  }

  // try to sniff SceneServer -- this works for direct layer endpoints (e.g. SceneServer/0)
  if ( json.contains( u"store"_s ) && json.contains( u"layerType"_s ) )
  {
    return Qgis::ArcGisRestServiceType::SceneServer;
  }

  if ( json.contains( u"layers"_s ) )
  {
    const QVariantList layersList = json.value( u"layers"_s ).toList();
    if ( !layersList.empty() )
    {
      const QVariantMap firstLayer = layersList.first().toMap();
      // "layerType" is distinct to SceneServer layers:
      if ( firstLayer.contains( u"layerType"_s ) || firstLayer.contains( u"store"_s ) )
      {
        return Qgis::ArcGisRestServiceType::SceneServer;
      }
    }
  }

  // catch feature-server layer-level endpoints (e.g. MapServer/0)
  // this can detect feature servers only -- eg for a mapserver layer it will be flagged
  // as a feature server
  if ( json.contains( u"type"_s ) )
  {
    const QString typeStr = json.value( u"type"_s ).toString();
    if ( typeStr.compare( "Feature Layer"_L1, Qt::CaseInsensitive ) == 0 || typeStr.compare( "Table"_L1, Qt::CaseInsensitive ) == 0 )
    {
      return Qgis::ArcGisRestServiceType::FeatureServer;
    }
  }

  return Qgis::ArcGisRestServiceType::Unknown;
}

QVariantMap QgsArcGisRestQueryUtils::getServiceInfo(
  const QString &baseurl, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders, const QString &urlPrefix, bool forceRefresh
)
{
  // http://sampleserver5.arcgisonline.com/arcgis/rest/services/Energy/Geology/FeatureServer?f=json
  QUrl queryUrl( baseurl );
  QUrlQuery query( queryUrl );
  query.addQueryItem( u"f"_s, u"json"_s );
  queryUrl.setQuery( query );
  return queryServiceJSON( queryUrl, authcfg, errorTitle, errorText, requestHeaders, nullptr, urlPrefix, forceRefresh );
}

QVariantMap QgsArcGisRestQueryUtils::getLayerInfo( const QString &layerurl, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders, const QString &urlPrefix )
{
  // http://sampleserver5.arcgisonline.com/arcgis/rest/services/Energy/Geology/FeatureServer/1?f=json
  QUrl queryUrl( layerurl );
  QUrlQuery query( queryUrl );
  query.addQueryItem( u"f"_s, u"json"_s );
  queryUrl.setQuery( query );
  return queryServiceJSON( queryUrl, authcfg, errorTitle, errorText, requestHeaders, nullptr, urlPrefix );
}

QVariantMap QgsArcGisRestQueryUtils::getObjectIds(
  const QString &layerurl, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders, const QString &urlPrefix, const QgsRectangle &bbox, const QString &whereClause
)
{
  // http://sampleserver5.arcgisonline.com/arcgis/rest/services/Energy/Geology/FeatureServer/1/query?where=1%3D1&returnIdsOnly=true&f=json
  QUrl queryUrl( layerurl + "/query" );
  QUrlQuery query( queryUrl );
  query.addQueryItem( u"f"_s, u"json"_s );
  query.addQueryItem( u"where"_s, whereClause.isEmpty() ? u"1=1"_s : whereClause );
  query.addQueryItem( u"returnIdsOnly"_s, u"true"_s );
  if ( !bbox.isNull() )
  {
    query.addQueryItem( u"geometry"_s, u"%1,%2,%3,%4"_s.arg( bbox.xMinimum(), 0, 'f', -1 ).arg( bbox.yMinimum(), 0, 'f', -1 ).arg( bbox.xMaximum(), 0, 'f', -1 ).arg( bbox.yMaximum(), 0, 'f', -1 ) );
    query.addQueryItem( u"geometryType"_s, u"esriGeometryEnvelope"_s );
    query.addQueryItem( u"spatialRel"_s, u"esriSpatialRelEnvelopeIntersects"_s );
  }
  queryUrl.setQuery( query );
  return queryServiceJSON( queryUrl, authcfg, errorTitle, errorText, requestHeaders, nullptr, urlPrefix );
}

QgsRectangle QgsArcGisRestQueryUtils::getExtent( const QString &layerurl, const QString &whereClause, const QString &authcfg, const QgsHttpHeaders &requestHeaders, const QString &urlPrefix )
{
  // http://sampleserver5.arcgisonline.com/arcgis/rest/services/Energy/Geology/FeatureServer/1/query?where=1%3D1&returnExtentOnly=true&f=json
  QUrl queryUrl( layerurl + "/query" );
  QUrlQuery query( queryUrl );
  query.addQueryItem( u"f"_s, u"json"_s );
  query.addQueryItem( u"where"_s, whereClause );
  query.addQueryItem( u"returnExtentOnly"_s, u"true"_s );
  queryUrl.setQuery( query );
  QString errorTitle;
  QString errorText;
  const QVariantMap res = queryServiceJSON( queryUrl, authcfg, errorTitle, errorText, requestHeaders, nullptr, urlPrefix );
  if ( res.isEmpty() )
  {
    QgsDebugError( u"getExtent failed: %1 - %2"_s.arg( errorTitle, errorText ) );
    return QgsRectangle();
  }

  return QgsArcGisRestUtils::convertRectangle( res.value( u"extent"_s ) );
}

QVariantMap QgsArcGisRestQueryUtils::getObjects(
  const QString &layerurl,
  const QString &authcfg,
  const QList<quint32> &objectIds,
  const QString &crs,
  bool fetchGeometry,
  const QStringList &fetchAttributes,
  bool fetchM,
  bool fetchZ,
  QString &errorTitle,
  QString &errorText,
  const QgsHttpHeaders &requestHeaders,
  QgsFeedback *feedback,
  const QString &urlPrefix
)
{
  QStringList ids;
  for ( const int id : objectIds )
  {
    ids.append( QString::number( id ) );
  }
  QUrl queryUrl( layerurl + "/query" );
  QUrlQuery query( queryUrl );
  query.addQueryItem( u"f"_s, u"json"_s );
  query.addQueryItem( u"objectIds"_s, ids.join( ','_L1 ) );
  if ( !crs.isEmpty() && crs.contains( ':' ) )
  {
    const QString wkid = crs.indexOf( ':'_L1 ) >= 0 ? crs.split( ':' )[1] : QString();
    query.addQueryItem( u"inSR"_s, wkid );
    query.addQueryItem( u"outSR"_s, wkid );
  }

  query.addQueryItem( u"returnGeometry"_s, fetchGeometry ? u"true"_s : u"false"_s );

  QString outFields;
  if ( fetchAttributes.isEmpty() )
    outFields = u"*"_s;
  else
    outFields = fetchAttributes.join( ',' );
  query.addQueryItem( u"outFields"_s, outFields );

  query.addQueryItem( u"returnM"_s, fetchM ? u"true"_s : u"false"_s );
  query.addQueryItem( u"returnZ"_s, fetchZ ? u"true"_s : u"false"_s );
  queryUrl.setQuery( query );
  return queryServiceJSON( queryUrl, authcfg, errorTitle, errorText, requestHeaders, feedback, urlPrefix );
}

QList<quint32> QgsArcGisRestQueryUtils::getObjectIdsByExtent(
  const QString &layerurl,
  const QgsRectangle &filterRect,
  QString &errorTitle,
  QString &errorText,
  const QString &authcfg,
  const QgsHttpHeaders &requestHeaders,
  QgsFeedback *feedback,
  const QString &whereClause,
  const QString &urlPrefix
)
{
  QUrl queryUrl( layerurl + "/query" );
  QUrlQuery query( queryUrl );
  query.addQueryItem( u"f"_s, u"json"_s );
  query.addQueryItem( u"where"_s, whereClause.isEmpty() ? u"1=1"_s : whereClause );
  query.addQueryItem( u"returnIdsOnly"_s, u"true"_s );
  query.addQueryItem( u"geometry"_s, u"%1,%2,%3,%4"_s.arg( filterRect.xMinimum(), 0, 'f', -1 ).arg( filterRect.yMinimum(), 0, 'f', -1 ).arg( filterRect.xMaximum(), 0, 'f', -1 ).arg( filterRect.yMaximum(), 0, 'f', -1 ) );
  query.addQueryItem( u"geometryType"_s, u"esriGeometryEnvelope"_s );
  query.addQueryItem( u"spatialRel"_s, u"esriSpatialRelEnvelopeIntersects"_s );
  queryUrl.setQuery( query );
  const QVariantMap objectIdData = queryServiceJSON( queryUrl, authcfg, errorTitle, errorText, requestHeaders, feedback, urlPrefix );

  if ( objectIdData.isEmpty() )
  {
    return QList<quint32>();
  }

  QList<quint32> ids;
  const QVariantList objectIdsList = objectIdData[u"objectIds"_s].toList();
  ids.reserve( objectIdsList.size() );
  for ( const QVariant &objectId : objectIdsList )
  {
    ids << objectId.toInt();
  }
  return ids;
}

QByteArray QgsArcGisRestQueryUtils::queryService(
  const QUrl &u, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders, QgsFeedback *feedback, QString *contentType, const QString &urlPrefix, bool forceRefresh
)
{
  QUrl url = parseUrl( u );

  if ( !urlPrefix.isEmpty() )
    url = QUrl( urlPrefix + url.toString() );

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, u"QgsArcGisRestUtils"_s );
  requestHeaders.updateNetworkRequest( request );

  QgsBlockingNetworkRequest networkRequest;
  networkRequest.setAuthCfg( authcfg );
  const QgsBlockingNetworkRequest::ErrorCode error = networkRequest.get( request, forceRefresh, feedback );

  if ( feedback && feedback->isCanceled() )
    return QByteArray();

  // Handle network errors
  if ( error != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugError( u"Network error: %1"_s.arg( networkRequest.errorMessage() ) );
    errorTitle = u"Network error"_s;
    errorText = networkRequest.errorMessage();

    // try to get detailed error message from reply
    const QString content = networkRequest.reply().content();
    const thread_local QRegularExpression errorRx( u"Error: <.*?>(.*?)<"_s );
    const QRegularExpressionMatch match = errorRx.match( content );
    if ( match.hasMatch() )
    {
      errorText = match.captured( 1 );
    }

    return QByteArray();
  }

  const QgsNetworkReplyContent content = networkRequest.reply();
  if ( contentType )
    *contentType = content.rawHeader( "Content-Type" );
  return content.content();
}

QVariantMap QgsArcGisRestQueryUtils::queryServiceJSON(
  const QUrl &url, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders, QgsFeedback *feedback, const QString &urlPrefix, bool forceRefresh
)
{
  const QByteArray reply = queryService( url, authcfg, errorTitle, errorText, requestHeaders, feedback, nullptr, urlPrefix, forceRefresh );
  if ( !errorTitle.isEmpty() )
  {
    return QVariantMap();
  }
  if ( feedback && feedback->isCanceled() )
    return QVariantMap();

  // Parse data
  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson( reply, &err );
  if ( doc.isNull() )
  {
    errorTitle = u"Parsing error"_s;
    errorText = err.errorString();
    QgsDebugError( u"Parsing error: %1"_s.arg( err.errorString() ) );
    return QVariantMap();
  }
  const QVariantMap res = doc.object().toVariantMap();
  if ( res.contains( u"error"_s ) )
  {
    const QVariantMap error = res.value( u"error"_s ).toMap();
    errorText = error.value( u"message"_s ).toString();
    errorTitle = QObject::tr( "Error %1" ).arg( error.value( u"code"_s ).toString() );
    return QVariantMap();
  }
  return res;
}

QUrl QgsArcGisRestQueryUtils::parseUrl( const QUrl &url, bool *isTestEndpoint )
{
  if ( isTestEndpoint )
    *isTestEndpoint = false;

  QUrl modifiedUrl( url );
  if ( modifiedUrl.toString().contains( "fake_qgis_http_endpoint"_L1 ) )
  {
    if ( isTestEndpoint )
      *isTestEndpoint = true;

    // Just for testing with local files instead of http:// resources
    QString modifiedUrlString = modifiedUrl.toString();
    // Qt5 does URL encoding from some reason (of the FILTER parameter for example)
    modifiedUrlString = QUrl::fromPercentEncoding( modifiedUrlString.toUtf8() );
    modifiedUrlString.replace( "fake_qgis_http_endpoint/"_L1, "fake_qgis_http_endpoint_"_L1 );
    QgsDebugMsgLevel( u"Get %1"_s.arg( modifiedUrlString ), 2 );
    modifiedUrlString = modifiedUrlString.mid( u"http://"_s.size() );
    QString args = modifiedUrlString.indexOf( '?' ) >= 0 ? modifiedUrlString.mid( modifiedUrlString.indexOf( '?' ) ) : QString();
    if ( modifiedUrlString.size() > 150 )
    {
      args = QCryptographicHash::hash( args.toUtf8(), QCryptographicHash::Md5 ).toHex();
    }
    else
    {
      args.replace( "?"_L1, "_"_L1 );
      args.replace( "&"_L1, "_"_L1 );
      args.replace( "<"_L1, "_"_L1 );
      args.replace( ">"_L1, "_"_L1 );
      args.replace( "'"_L1, "_"_L1 );
      args.replace( "\""_L1, "_"_L1 );
      args.replace( " "_L1, "_"_L1 );
      args.replace( ":"_L1, "_"_L1 );
      args.replace( "/"_L1, "_"_L1 );
      args.replace( "\n"_L1, "_"_L1 );
    }
#ifdef Q_OS_WIN
    // Passing "urls" like "http://c:/path" to QUrl 'eats' the : after c,
    // so we must restore it
    if ( modifiedUrlString[1] == '/' )
    {
      modifiedUrlString = modifiedUrlString[0] + ":/" + modifiedUrlString.mid( 2 );
    }
#endif
    modifiedUrlString = modifiedUrlString.mid( 0, modifiedUrlString.indexOf( '?' ) ) + args;

    if ( modifiedUrlString.contains( "tile/" ) )
    {
      modifiedUrlString = modifiedUrlString.left( modifiedUrlString.indexOf( "tile/" ) + 4 ) + modifiedUrlString.mid( modifiedUrlString.indexOf( "tile/" ) + 4 ).replace( '/', '_' );
    }

    QgsDebugMsgLevel( u"Get %1 (after laundering)"_s.arg( modifiedUrlString ), 2 );
    modifiedUrl = QUrl::fromLocalFile( modifiedUrlString );
    if ( !QFile::exists( modifiedUrlString ) )
    {
      QgsDebugError( u"Local test file %1 for URL %2 does not exist!!!"_s.arg( modifiedUrlString, url.toString() ) );
    }
  }

  return modifiedUrl;
}

void QgsArcGisRestQueryUtils::adjustBaseUrl( QString &baseUrl, const QString &name )
{
  const QStringList parts = name.split( '/' );
  QString checkString;
  for ( const QString &part : parts )
  {
    if ( !checkString.isEmpty() )
      checkString += QString( '/' );

    checkString += part;
    if ( baseUrl.indexOf( QRegularExpression( checkString.replace( '/', "\\/"_L1 ) + u"\\/?$"_s ) ) > -1 )
    {
      baseUrl = baseUrl.left( baseUrl.length() - checkString.length() - 1 );
      break;
    }
  }
}

void QgsArcGisRestQueryUtils::visitFolderItems( const std::function< void( const QString &, const QString & ) > &visitor, const QVariantMap &serviceData, const QString &baseUrl )
{
  QString base( baseUrl );
  bool baseChecked = false;
  if ( !base.endsWith( '/' ) )
    base += '/'_L1;

  const QStringList folderList = serviceData.value( u"folders"_s ).toStringList();
  for ( const QString &folder : folderList )
  {
    if ( !baseChecked )
    {
      adjustBaseUrl( base, folder );
      baseChecked = true;
    }
    visitor( folder, base + folder );
  }
}

void QgsArcGisRestQueryUtils::visitServiceItems( const std::function<void( const QString &, const QString &, Qgis::ArcGisRestServiceType )> &visitor, const QVariantMap &serviceData, const QString &baseUrl )
{
  QString base( baseUrl );
  bool baseChecked = false;
  if ( !base.endsWith( '/' ) )
    base += '/'_L1;

  const QVariantList serviceList = serviceData.value( u"services"_s ).toList();
  for ( const QVariant &service : serviceList )
  {
    const QVariantMap serviceMap = service.toMap();
    const QString serviceTypeString = serviceMap.value( u"type"_s ).toString();
    const Qgis::ArcGisRestServiceType serviceType = QgsArcGisRestUtils::serviceTypeFromString( serviceTypeString );

    switch ( serviceType )
    {
      case Qgis::ArcGisRestServiceType::FeatureServer:
      case Qgis::ArcGisRestServiceType::MapServer:
      case Qgis::ArcGisRestServiceType::ImageServer:
      case Qgis::ArcGisRestServiceType::SceneServer:
        // supported
        break;

      case Qgis::ArcGisRestServiceType::GlobeServer:
      case Qgis::ArcGisRestServiceType::GPServer:
      case Qgis::ArcGisRestServiceType::GeocodeServer:
      case Qgis::ArcGisRestServiceType::Unknown:
        // unsupported
        continue;
    }

    const QString serviceName = serviceMap.value( u"name"_s ).toString();
    const QString displayName = serviceName.split( '/' ).last();
    if ( !baseChecked )
    {
      adjustBaseUrl( base, serviceName );
      baseChecked = true;
    }

    visitor( displayName, base + serviceName + '/' + serviceTypeString, serviceType );
  }
}

void QgsArcGisRestQueryUtils::addLayerItems(
  const std::function< void( const LayerItemDetails &details )> &visitor, const QVariantMap &serviceData, const QString &parentUrl, const QString &parentSupportedFormats, Qgis::ArcGisRestServiceType serviceType
)
{
  const QgsCoordinateReferenceSystem crs = QgsArcGisRestUtils::convertSpatialReference( serviceData.value( u"spatialReference"_s ).toMap() );

  bool found = false;
  const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
  const QStringList supportedImageFormatTypes = serviceData.value( u"supportedImageFormatTypes"_s ).toString().isEmpty() ? parentSupportedFormats.split( ',' )
                                                                                                                         : serviceData.value( u"supportedImageFormatTypes"_s ).toString().split( ',' );
  QString format = supportedImageFormatTypes.value( 0 );
  for ( const QString &encoding : supportedImageFormatTypes )
  {
    for ( const QByteArray &fmt : supportedFormats )
    {
      if ( encoding.startsWith( fmt, Qt::CaseInsensitive ) )
      {
        format = encoding;
        found = true;
        break;
      }
    }
    if ( found )
      break;
  }
  Qgis::ArcGisRestServiceCapabilities capabilities = QgsArcGisRestUtils::serviceCapabilitiesFromString( serviceData.value( u"capabilities"_s ).toString() );
  if ( serviceType == Qgis::ArcGisRestServiceType::ImageServer )
  {
    // consider ImageServices as having both render and query capabilities, so we can load them
    // as either raster or vector
    capabilities.setFlag( Qgis::ArcGisRestServiceCapability::Map, true );
    capabilities.setFlag( Qgis::ArcGisRestServiceCapability::Query, true );
  }

  const QVariantList layerInfoList = serviceData.value( u"layers"_s ).toList();
  for ( const QVariant &layerInfo : layerInfoList )
  {
    const QVariantMap layerInfoMap = layerInfo.toMap();

    LayerItemDetails details;
    details.layerId = layerInfoMap.value( u"id"_s ).toString();
    details.parentLayerId = layerInfoMap.value( u"parentLayerId"_s ).toString();
    details.name = layerInfoMap.value( u"name"_s ).toString();
    details.description = layerInfoMap.value( u"description"_s ).toString();

    const QString geometryType = layerInfoMap.value( u"geometryType"_s ).toString();
#if 0
    // we have a choice here -- if geometryType is unknown and the service reflects that it supports Map capabilities,
    // then we can't be sure whether or not the individual sublayers support Query or Map requests only. So we either:
    // 1. Send off additional requests for each individual layer's capabilities (too expensive)
    // 2. Err on the side of only showing services we KNOW will work for layer -- but this has the side effect that layers
    //    which ARE available as feature services will only show as raster mapserver layers, which is VERY bad/restrictive
    // 3. Err on the side of showing services we THINK may work, even though some of them may or may not work depending on the actual
    //    server configuration
    // We opt for 3, because otherwise we're making it impossible for users to load valid vector layers into QGIS

      if ( capabilities.testFlag( Qgis::ArcGisRestServiceCapability::Map ) )
      {
        if ( geometryType.isEmpty() )
          continue;
      }
#endif

    // deal with the easy stuff first -- if we found a scene server layer, then it can ONLY be loaded as a scene
    if ( serviceType == Qgis::ArcGisRestServiceType::SceneServer )
    {
      details.serviceType = Qgis::ArcGisRestServiceType::SceneServer;
      details.geometryType = Qgis::GeometryType::Unknown;
      details.url = parentUrl;
      details.isParentLayer = false;
      details.crs = crs;
      details.format = format;
      details.isMapServerWithQueryCapability = false;
      visitor( details );
      continue;
    }

    // Yes, potentially we may visit twice, once as as a raster (if applicable), and once as a vector (if applicable)!
    bool exposedAsVector = false;
    if ( capabilities.testFlag( Qgis::ArcGisRestServiceCapability::Query ) && ( serviceType == Qgis::ArcGisRestServiceType::FeatureServer || serviceType == Qgis::ArcGisRestServiceType::MapServer ) )
    {
      exposedAsVector = true;
      const Qgis::WkbType wkbType = QgsArcGisRestUtils::convertGeometryType( geometryType );
      details.serviceType = Qgis::ArcGisRestServiceType::FeatureServer;
      details.geometryType = QgsWkbTypes::geometryType( wkbType );
      details.url = parentUrl + '/' + details.layerId;
      details.format = format;
      details.isMapServerWithQueryCapability = capabilities.testFlag( Qgis::ArcGisRestServiceCapability::Map );
      if ( !layerInfoMap.value( u"subLayerIds"_s ).toList().empty() )
      {
        details.isParentLayer = true;
        details.crs = QgsCoordinateReferenceSystem();
        visitor( details );
      }
      else
      {
        details.isParentLayer = false;
        details.crs = crs;
        visitor( details );
      }
    }

    if ( capabilities.testFlag( Qgis::ArcGisRestServiceCapability::Map ) && ( serviceType == Qgis::ArcGisRestServiceType::FeatureServer || serviceType == Qgis::ArcGisRestServiceType::MapServer ) )
    {
      Qgis::WkbType wkbType = Qgis::WkbType::Unknown;
      if ( capabilities.testFlag( Qgis::ArcGisRestServiceCapability::Query ) )
        wkbType = QgsArcGisRestUtils::convertGeometryType( geometryType );

      details.serviceType = serviceType == Qgis::ArcGisRestServiceType::Unknown ? Qgis::ArcGisRestServiceType::MapServer : serviceType;
      details.geometryType = QgsWkbTypes::geometryType( wkbType );
      details.url = parentUrl + '/' + details.layerId;
      details.format = format;
      details.isMapServerWithQueryCapability = exposedAsVector;
      if ( !layerInfoMap.value( u"subLayerIds"_s ).toList().empty() )
      {
        if ( !exposedAsVector )
        {
          details.isParentLayer = true;
          details.crs = QgsCoordinateReferenceSystem();
          visitor( details );
        }
      }
      else
      {
        details.isParentLayer = false;
        details.crs = crs;
        visitor( details );
      }
    }
  }

  const QVariantList tableInfoList = serviceData.value( u"tables"_s ).toList();
  if ( capabilities.testFlag( Qgis::ArcGisRestServiceCapability::Query ) && serviceType == Qgis::ArcGisRestServiceType::FeatureServer )
  {
    for ( const QVariant &tableInfo : tableInfoList )
    {
      const QVariantMap tableInfoMap = tableInfo.toMap();

      LayerItemDetails details;
      details.layerId = tableInfoMap.value( u"id"_s ).toString();
      details.parentLayerId = tableInfoMap.value( u"parentLayerId"_s ).toString();
      details.name = tableInfoMap.value( u"name"_s ).toString();
      details.description = tableInfoMap.value( u"description"_s ).toString();

      details.serviceType = Qgis::ArcGisRestServiceType::FeatureServer;
      details.geometryType = Qgis::GeometryType::Null;
      details.url = parentUrl + '/' + details.layerId;
      details.format = format;
      details.isMapServerWithQueryCapability = false;
      if ( !tableInfoMap.value( u"subLayerIds"_s ).toList().empty() )
      {
        details.isParentLayer = true;
        details.crs = QgsCoordinateReferenceSystem();
        visitor( details );
      }
      else
      {
        details.isParentLayer = false;
        details.crs = crs;
        visitor( details );
      }
    }
  }

  if ( layerInfoList.isEmpty() && tableInfoList.isEmpty() && serviceType != Qgis::ArcGisRestServiceType::Unknown )
  {
    // haven't found any layers yet. But maybe the definition is for a layer itself.
    switch ( serviceType )
    {
      case Qgis::ArcGisRestServiceType::FeatureServer:
      {
        LayerItemDetails details;
        details.layerId = serviceData.value( u"id"_s ).toString();
        details.name = serviceData.value( u"name"_s ).toString();
        details.description = serviceData.value( u"description"_s ).toString();
        const QString geometryType = serviceData.value( u"geometryType"_s ).toString();
        const Qgis::WkbType wkbType = QgsArcGisRestUtils::convertGeometryType( geometryType );
        details.serviceType = Qgis::ArcGisRestServiceType::FeatureServer;
        details.geometryType = QgsWkbTypes::geometryType( wkbType );
        details.url = parentUrl;
        details.format = format;
        details.isMapServerWithQueryCapability = capabilities.testFlag( Qgis::ArcGisRestServiceCapability::Map );
        details.isParentLayer = false;
        details.crs = crs;
        visitor( details );
        break;
      }
      case Qgis::ArcGisRestServiceType::MapServer:
      {
        LayerItemDetails details;
        details.name = serviceData.value( u"serviceDescription"_s ).toString();
        details.description = serviceData.value( u"description"_s ).toString();
        details.serviceType = Qgis::ArcGisRestServiceType::MapServer;
        details.geometryType = Qgis::GeometryType::Unknown;
        details.url = parentUrl;
        details.isParentLayer = false;
        details.crs = crs;
        details.format = format;
        details.isMapServerWithQueryCapability = false;
        details.isMapServerSpecialAllLayersOption = false;
        visitor( details );
        break;
      }
      case Qgis::ArcGisRestServiceType::ImageServer:
      {
        LayerItemDetails details;
        details.layerId = QString();
        details.parentLayerId = QString();
        details.name = serviceData.value( u"name"_s ).toString();
        details.description = serviceData.value( u"description"_s ).toString();
        details.serviceType = Qgis::ArcGisRestServiceType::ImageServer;
        details.geometryType = Qgis::GeometryType::Unknown;
        details.url = parentUrl;
        details.isParentLayer = false;
        details.crs = crs;
        details.format = format;
        details.isMapServerWithQueryCapability = false;
        visitor( details );
        break;
      }
      case Qgis::ArcGisRestServiceType::SceneServer:
      {
        LayerItemDetails details;
        details.layerId = serviceData.value( u"id"_s ).toString();
        details.name = serviceData.value( u"name"_s ).toString();
        details.description = serviceData.value( u"description"_s ).toString();
        details.serviceType = Qgis::ArcGisRestServiceType::SceneServer;
        details.geometryType = Qgis::GeometryType::Unknown;
        details.url = parentUrl;
        details.isParentLayer = false;
        details.crs = crs;
        details.format = format;
        details.isMapServerWithQueryCapability = false;
        visitor( details );
        break;
      }
      case Qgis::ArcGisRestServiceType::GlobeServer:
      case Qgis::ArcGisRestServiceType::GPServer:
      case Qgis::ArcGisRestServiceType::GeocodeServer:
      case Qgis::ArcGisRestServiceType::Unknown:
        break;
    }
  }

  // Add root MapServer as raster layer when multiple layers are listed
  if ( serviceType != Qgis::ArcGisRestServiceType::FeatureServer && layerInfoList.count() > 1 && serviceData.contains( u"supportedImageFormatTypes"_s ) )
  {
    LayerItemDetails details;
    details.parentLayerId = QString();
    details.serviceType = serviceType == Qgis::ArcGisRestServiceType::Unknown ? Qgis::ArcGisRestServiceType::MapServer : serviceType;
    details.geometryType = Qgis::GeometryType::Unknown;
    details.url = parentUrl;
    details.isParentLayer = false;
    details.crs = crs;
    details.format = format;
    details.isMapServerWithQueryCapability = false;
    details.isMapServerSpecialAllLayersOption = true;
    visitor( details );
  }
}


///@cond PRIVATE

//
// QgsArcGisAsyncQuery
//

QgsArcGisAsyncQuery::QgsArcGisAsyncQuery( QObject *parent )
  : QObject( parent )
{}

QgsArcGisAsyncQuery::~QgsArcGisAsyncQuery()
{
  if ( mReply )
    mReply->deleteLater();
}

void QgsArcGisAsyncQuery::start( const QUrl &url, const QString &authCfg, QByteArray *result, bool allowCache, const QgsHttpHeaders &headers, const QString &urlPrefix )
{
  mResult = result;
  QUrl mUrl = url;
  if ( !urlPrefix.isEmpty() )
    mUrl = QUrl( urlPrefix + url.toString() );
  QNetworkRequest request( mUrl );

  headers.updateNetworkRequest( request );

  if ( !authCfg.isEmpty() && !QgsApplication::authManager()->updateNetworkRequest( request, authCfg ) )
  {
    const QString error = tr( "network request update failed for authentication config" );
    emit failed( u"Network"_s, error );
    return;
  }

  QgsSetRequestInitiatorClass( request, u"QgsArcGisAsyncQuery"_s );
  request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
  if ( allowCache )
  {
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  }

  mReply = QgsNetworkAccessManager::instance()->get( request );
  connect( mReply, &QNetworkReply::finished, this, &QgsArcGisAsyncQuery::handleReply );
}

void QgsArcGisAsyncQuery::handleReply()
{
  mReply->deleteLater();
  // Handle network errors
  if ( mReply->error() != QNetworkReply::NoError )
  {
    QgsDebugError( u"Network error: %1"_s.arg( mReply->errorString() ) );
    emit failed( u"Network error"_s, mReply->errorString() );
    return;
  }

  // Handle HTTP redirects
  const QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  if ( !QgsVariantUtils::isNull( redirect ) )
  {
    QNetworkRequest request = mReply->request();
    request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
    QgsSetRequestInitiatorClass( request, u"QgsArcGisAsyncQuery"_s );
    QgsDebugMsgLevel( "redirecting to " + redirect.toUrl().toString(), 2 );
    request.setUrl( redirect.toUrl() );
    mReply = QgsNetworkAccessManager::instance()->get( request );
    connect( mReply, &QNetworkReply::finished, this, &QgsArcGisAsyncQuery::handleReply );
    return;
  }

  *mResult = mReply->readAll();
  mResult = nullptr;
  emit finished();
}

//
// QgsArcGisAsyncParallelQuery
//

QgsArcGisAsyncParallelQuery::QgsArcGisAsyncParallelQuery( const QString &authcfg, const QgsHttpHeaders &requestHeaders, QObject *parent )
  : QObject( parent )
  , mAuthCfg( authcfg )
  , mRequestHeaders( requestHeaders )
{}

void QgsArcGisAsyncParallelQuery::start( const QVector<QUrl> &urls, QVector<QByteArray> *results, bool allowCache )
{
  Q_ASSERT( results->size() == urls.size() );
  mResults = results;
  mPendingRequests = mResults->size();
  for ( int i = 0, n = urls.size(); i < n; ++i )
  {
    QNetworkRequest request( urls[i] );
    QgsSetRequestInitiatorClass( request, u"QgsArcGisAsyncParallelQuery"_s );
    QgsSetRequestInitiatorId( request, QString::number( i ) );

    mRequestHeaders.updateNetworkRequest( request );
    if ( !mAuthCfg.isEmpty() && !QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg ) )
    {
      const QString error = tr( "network request update failed for authentication config" );
      mErrors.append( error );
      QgsMessageLog::logMessage( error, tr( "Network" ) );
      continue;
    }

    request.setAttribute( QNetworkRequest::HttpPipeliningAllowedAttribute, true );
    if ( allowCache )
    {
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
      request.setRawHeader( "Connection", "keep-alive" );
    }
    QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
    reply->setProperty( "idx", i );
    connect( reply, &QNetworkReply::finished, this, &QgsArcGisAsyncParallelQuery::handleReply );
  }
}

void QgsArcGisAsyncParallelQuery::handleReply()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( QObject::sender() );
  const QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  const int idx = reply->property( "idx" ).toInt();
  reply->deleteLater();
  if ( reply->error() != QNetworkReply::NoError )
  {
    // Handle network errors
    mErrors.append( reply->errorString() );
    --mPendingRequests;
  }
  else if ( !QgsVariantUtils::isNull( redirect ) )
  {
    // Handle HTTP redirects
    QNetworkRequest request = reply->request();
    request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
    QgsSetRequestInitiatorClass( request, u"QgsArcGisAsyncParallelQuery"_s );
    QgsDebugMsgLevel( "redirecting to " + redirect.toUrl().toString(), 2 );
    request.setUrl( redirect.toUrl() );
    reply = QgsNetworkAccessManager::instance()->get( request );
    reply->setProperty( "idx", idx );
    connect( reply, &QNetworkReply::finished, this, &QgsArcGisAsyncParallelQuery::handleReply );
  }
  else
  {
    // All OK
    ( *mResults )[idx] = reply->readAll();
    --mPendingRequests;
  }
  if ( mPendingRequests == 0 )
  {
    emit finished( mErrors );
    mResults = nullptr;
    mErrors.clear();
  }
}

///@endcond
