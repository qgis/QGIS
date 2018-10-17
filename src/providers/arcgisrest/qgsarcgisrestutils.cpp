/***************************************************************************
    qgsarcgisrestutils.cpp
    ----------------------
    begin                : Nov 25, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : manisandro@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarcgisrestutils.h"
#include "qgsfields.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsrectangle.h"
#include "geometry/qgsabstractgeometry.h"
#include "geometry/qgscircularstring.h"
#include "geometry/qgscompoundcurve.h"
#include "geometry/qgscurvepolygon.h"
#include "geometry/qgslinestring.h"
#include "geometry/qgsmultipoint.h"
#include "geometry/qgsmulticurve.h"
#include "geometry/qgspolygon.h"
#include "geometry/qgspoint.h"
#include "qgsfeedback.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgscategorizedsymbolrenderer.h"
#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>


QVariant::Type QgsArcGisRestUtils::mapEsriFieldType( const QString &esriFieldType )
{
  if ( esriFieldType == QLatin1String( "esriFieldTypeInteger" ) )
    return QVariant::LongLong;
  if ( esriFieldType == QLatin1String( "esriFieldTypeSmallInteger" ) )
    return QVariant::Int;
  if ( esriFieldType == QLatin1String( "esriFieldTypeDouble" ) )
    return QVariant::Double;
  if ( esriFieldType == QLatin1String( "esriFieldTypeSingle" ) )
    return QVariant::Double;
  if ( esriFieldType == QLatin1String( "esriFieldTypeString" ) )
    return QVariant::String;
  if ( esriFieldType == QLatin1String( "esriFieldTypeDate" ) )
    return QVariant::Date;
  if ( esriFieldType == QLatin1String( "esriFieldTypeGeometry" ) )
    return QVariant::Invalid; // Geometry column should not appear as field
  if ( esriFieldType == QLatin1String( "esriFieldTypeOID" ) )
    return QVariant::LongLong;
  if ( esriFieldType == QLatin1String( "esriFieldTypeBlob" ) )
    return QVariant::ByteArray;
  if ( esriFieldType == QLatin1String( "esriFieldTypeGlobalID" ) )
    return QVariant::String;
  if ( esriFieldType == QLatin1String( "esriFieldTypeRaster" ) )
    return QVariant::ByteArray;
  if ( esriFieldType == QLatin1String( "esriFieldTypeGUID" ) )
    return QVariant::String;
  if ( esriFieldType == QLatin1String( "esriFieldTypeXML" ) )
    return QVariant::String;
  return QVariant::Invalid;
}

QgsWkbTypes::Type QgsArcGisRestUtils::mapEsriGeometryType( const QString &esriGeometryType )
{
  // http://resources.arcgis.com/en/help/arcobjects-cpp/componenthelp/index.html#//000w0000001p000000
  if ( esriGeometryType == QLatin1String( "esriGeometryNull" ) )
    return QgsWkbTypes::Unknown;
  else if ( esriGeometryType == QLatin1String( "esriGeometryPoint" ) )
    return QgsWkbTypes::Point;
  else if ( esriGeometryType == QLatin1String( "esriGeometryMultipoint" ) )
    return QgsWkbTypes::MultiPoint;
  else if ( esriGeometryType == QLatin1String( "esriGeometryPolyline" ) )
    return QgsWkbTypes::MultiCurve;
  else if ( esriGeometryType == QLatin1String( "esriGeometryPolygon" ) )
    return QgsWkbTypes::Polygon;
  else if ( esriGeometryType == QLatin1String( "esriGeometryEnvelope" ) )
    return QgsWkbTypes::Polygon;
  // Unsupported (either by qgis, or format unspecified by the specification)
  //  esriGeometryCircularArc
  //  esriGeometryEllipticArc
  //  esriGeometryBezier3Curve
  //  esriGeometryPath
  //  esriGeometryRing
  //  esriGeometryLine
  //  esriGeometryAny
  //  esriGeometryMultiPatch
  //  esriGeometryTriangleStrip
  //  esriGeometryTriangleFan
  //  esriGeometryRay
  //  esriGeometrySphere
  //  esriGeometryTriangles
  //  esriGeometryBag
  return QgsWkbTypes::Unknown;
}

static std::unique_ptr< QgsPoint > parsePoint( const QVariantList &coordList, QgsWkbTypes::Type pointType )
{
  int nCoords = coordList.size();
  if ( nCoords < 2 )
    return nullptr;
  bool xok = false, yok = false;
  double x = coordList[0].toDouble( &xok );
  double y = coordList[1].toDouble( &yok );
  if ( !xok || !yok )
    return nullptr;
  double z = nCoords >= 3 ? coordList[2].toDouble() : 0;
  double m = nCoords >= 4 ? coordList[3].toDouble() : 0;
  return qgis::make_unique< QgsPoint >( pointType, x, y, z, m );
}

static std::unique_ptr< QgsCircularString > parseCircularString( const QVariantMap &curveData, QgsWkbTypes::Type pointType, const QgsPoint &startPoint )
{
  const QVariantList coordsList = curveData[QStringLiteral( "c" )].toList();
  if ( coordsList.isEmpty() )
    return nullptr;
  QVector<QgsPoint> points;
  points.append( startPoint );
  for ( const QVariant &coordData : coordsList )
  {
    std::unique_ptr< QgsPoint > point = parsePoint( coordData.toList(), pointType );
    if ( !point )
    {
      return nullptr;
    }
    points.append( *point );
  }
  std::unique_ptr< QgsCircularString > curve = qgis::make_unique< QgsCircularString> ();
  curve->setPoints( points );
  return curve;
}

static std::unique_ptr< QgsCompoundCurve > parseCompoundCurve( const QVariantList &curvesList, QgsWkbTypes::Type pointType )
{
  // [[6,3],[5,3],{"b":[[3,2],[6,1],[2,4]]},[1,2],{"c": [[3,3],[1,4]]}]
  std::unique_ptr< QgsCompoundCurve > compoundCurve = qgis::make_unique< QgsCompoundCurve >();
  QgsLineString *lineString = new QgsLineString();
  compoundCurve->addCurve( lineString );
  for ( const QVariant &curveData : curvesList )
  {
    if ( curveData.type() == QVariant::List )
    {
      std::unique_ptr< QgsPoint > point = parsePoint( curveData.toList(), pointType );
      if ( !point )
      {
        return nullptr;
      }
      lineString->addVertex( *point );
    }
    else if ( curveData.type() == QVariant::Map )
    {
      // The last point of the linestring is the start point of this circular string
      std::unique_ptr< QgsCircularString > circularString = parseCircularString( curveData.toMap(), pointType, lineString->endPoint() );
      if ( !circularString )
      {
        return nullptr;
      }

      // If the previous curve had less than two points, remove it
      if ( compoundCurve->curveAt( compoundCurve->nCurves() - 1 )->nCoordinates() < 2 )
        compoundCurve->removeCurve( compoundCurve->nCurves() - 1 );

      compoundCurve->addCurve( circularString.release() );

      // Prepare a new line string
      lineString = new QgsLineString;
      compoundCurve->addCurve( lineString );
      lineString->addVertex( circularString->endPoint() );
    }
  }
  return compoundCurve;
}

static std::unique_ptr< QgsPoint > parseEsriGeometryPoint( const QVariantMap &geometryData, QgsWkbTypes::Type pointType )
{
  // {"x" : <x>, "y" : <y>, "z" : <z>, "m" : <m>}
  bool xok = false, yok = false;
  double x = geometryData[QStringLiteral( "x" )].toDouble( &xok );
  double y = geometryData[QStringLiteral( "y" )].toDouble( &yok );
  if ( !xok || !yok )
    return nullptr;
  double z = geometryData[QStringLiteral( "z" )].toDouble();
  double m = geometryData[QStringLiteral( "m" )].toDouble();
  return qgis::make_unique< QgsPoint >( pointType, x, y, z, m );
}

static std::unique_ptr< QgsMultiPoint > parseEsriGeometryMultiPoint( const QVariantMap &geometryData, QgsWkbTypes::Type pointType )
{
  // {"points" : [[ <x1>, <y1>, <z1>, <m1> ] , [ <x2>, <y2>, <z2>, <m2> ], ... ]}
  QVariantList coordsList = geometryData[QStringLiteral( "points" )].toList();
  if ( coordsList.isEmpty() )
    return nullptr;

  std::unique_ptr< QgsMultiPoint > multiPoint = qgis::make_unique< QgsMultiPoint >();
  Q_FOREACH ( const QVariant &coordData, coordsList )
  {
    QVariantList coordList = coordData.toList();
    std::unique_ptr< QgsPoint > p = parsePoint( coordList, pointType );
    if ( !p )
    {
      return nullptr;
    }
    multiPoint->addGeometry( p.release() );
  }
  return multiPoint;
}

static std::unique_ptr< QgsMultiCurve > parseEsriGeometryPolyline( const QVariantMap &geometryData, QgsWkbTypes::Type pointType )
{
  // {"curvePaths": [[[0,0], {"c": [[3,3],[1,4]]} ]]}
  QVariantList pathsList;
  if ( geometryData[QStringLiteral( "paths" )].isValid() )
    pathsList = geometryData[QStringLiteral( "paths" )].toList();
  else if ( geometryData[QStringLiteral( "curvePaths" )].isValid() )
    pathsList = geometryData[QStringLiteral( "curvePaths" )].toList();
  if ( pathsList.isEmpty() )
    return nullptr;
  std::unique_ptr< QgsMultiCurve > multiCurve = qgis::make_unique< QgsMultiCurve >();
  for ( const QVariant &pathData : qgis::as_const( pathsList ) )
  {
    std::unique_ptr< QgsCompoundCurve > curve = parseCompoundCurve( pathData.toList(), pointType );
    if ( !curve )
    {
      return nullptr;
    }
    multiCurve->addGeometry( curve.release() );
  }
  return multiCurve;
}

static std::unique_ptr< QgsCurvePolygon > parseEsriGeometryPolygon( const QVariantMap &geometryData, QgsWkbTypes::Type pointType )
{
  // {"curveRings": [[[0,0], {"c": [[3,3],[1,4]]} ]]}
  QVariantList ringsList;
  if ( geometryData[QStringLiteral( "rings" )].isValid() )
    ringsList = geometryData[QStringLiteral( "rings" )].toList();
  else if ( geometryData[QStringLiteral( "ringPaths" )].isValid() )
    ringsList = geometryData[QStringLiteral( "ringPaths" )].toList();
  if ( ringsList.isEmpty() )
    return nullptr;
  std::unique_ptr< QgsCurvePolygon > polygon = qgis::make_unique< QgsCurvePolygon >();
  std::unique_ptr< QgsCompoundCurve > ext = parseCompoundCurve( ringsList.front().toList(), pointType );
  if ( !ext )
  {
    return nullptr;
  }
  polygon->setExteriorRing( ext.release() );
  for ( int i = 1, n = ringsList.size(); i < n; ++i )
  {
    std::unique_ptr< QgsCompoundCurve > curve = parseCompoundCurve( ringsList[i].toList(), pointType );
    if ( !curve )
    {
      return nullptr;
    }
    polygon->addInteriorRing( curve.release() );
  }
  return polygon;
}

static std::unique_ptr< QgsPolygon > parseEsriEnvelope( const QVariantMap &geometryData )
{
  // {"xmin" : -109.55, "ymin" : 25.76, "xmax" : -86.39, "ymax" : 49.94}
  bool xminOk = false, yminOk = false, xmaxOk = false, ymaxOk = false;
  double xmin = geometryData[QStringLiteral( "xmin" )].toDouble( &xminOk );
  double ymin = geometryData[QStringLiteral( "ymin" )].toDouble( &yminOk );
  double xmax = geometryData[QStringLiteral( "xmax" )].toDouble( &xmaxOk );
  double ymax = geometryData[QStringLiteral( "ymax" )].toDouble( &ymaxOk );
  if ( !xminOk || !yminOk || !xmaxOk || !ymaxOk )
    return nullptr;
  std::unique_ptr< QgsLineString > ext = qgis::make_unique< QgsLineString> ();
  ext->addVertex( QgsPoint( xmin, ymin ) );
  ext->addVertex( QgsPoint( xmax, ymin ) );
  ext->addVertex( QgsPoint( xmax, ymax ) );
  ext->addVertex( QgsPoint( xmin, ymax ) );
  ext->addVertex( QgsPoint( xmin, ymin ) );
  std::unique_ptr< QgsPolygon > poly = qgis::make_unique< QgsPolygon >();
  poly->setExteriorRing( ext.release() );
  return poly;
}

std::unique_ptr<QgsAbstractGeometry> QgsArcGisRestUtils::parseEsriGeoJSON( const QVariantMap &geometryData, const QString &esriGeometryType, bool readM, bool readZ, QgsCoordinateReferenceSystem *crs )
{
  QgsWkbTypes::Type pointType = QgsWkbTypes::zmType( QgsWkbTypes::Point, readZ, readM );
  if ( crs )
  {
    *crs = parseSpatialReference( geometryData[QStringLiteral( "spatialReference" )].toMap() );
  }

  // http://resources.arcgis.com/en/help/arcgis-rest-api/index.html#/Geometry_Objects/02r3000000n1000000/
  if ( esriGeometryType == QLatin1String( "esriGeometryNull" ) )
    return nullptr;
  else if ( esriGeometryType == QLatin1String( "esriGeometryPoint" ) )
    return parseEsriGeometryPoint( geometryData, pointType );
  else if ( esriGeometryType == QLatin1String( "esriGeometryMultipoint" ) )
    return parseEsriGeometryMultiPoint( geometryData, pointType );
  else if ( esriGeometryType == QLatin1String( "esriGeometryPolyline" ) )
    return parseEsriGeometryPolyline( geometryData, pointType );
  else if ( esriGeometryType == QLatin1String( "esriGeometryPolygon" ) )
    return parseEsriGeometryPolygon( geometryData, pointType );
  else if ( esriGeometryType == QLatin1String( "esriGeometryEnvelope" ) )
    return parseEsriEnvelope( geometryData );
  // Unsupported (either by qgis, or format unspecified by the specification)
  //  esriGeometryCircularArc
  //  esriGeometryEllipticArc
  //  esriGeometryBezier3Curve
  //  esriGeometryPath
  //  esriGeometryRing
  //  esriGeometryLine
  //  esriGeometryAny
  //  esriGeometryMultiPatch
  //  esriGeometryTriangleStrip
  //  esriGeometryTriangleFan
  //  esriGeometryRay
  //  esriGeometrySphere
  //  esriGeometryTriangles
  //  esriGeometryBag
  return nullptr;
}

QgsCoordinateReferenceSystem QgsArcGisRestUtils::parseSpatialReference( const QVariantMap &spatialReferenceMap )
{
  QString spatialReference = spatialReferenceMap[QStringLiteral( "latestWkid" )].toString();
  if ( spatialReference.isEmpty() )
    spatialReference = spatialReferenceMap[QStringLiteral( "wkid" )].toString();
  if ( spatialReference.isEmpty() )
    spatialReference = spatialReferenceMap[QStringLiteral( "wkt" )].toString();
  else
    spatialReference = QStringLiteral( "EPSG:%1" ).arg( spatialReference );
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( spatialReference );
  if ( !crs.isValid() )
  {
    // If not spatial reference, just use WGS84
    crs.createFromString( QStringLiteral( "EPSG:4326" ) );
  }
  return crs;
}


QVariantMap QgsArcGisRestUtils::getServiceInfo( const QString &baseurl, QString &errorTitle, QString &errorText )
{
  // http://sampleserver5.arcgisonline.com/arcgis/rest/services/Energy/Geology/FeatureServer?f=json
  QUrl queryUrl( baseurl );
  queryUrl.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
  return queryServiceJSON( queryUrl, errorTitle, errorText );
}

QVariantMap QgsArcGisRestUtils::getLayerInfo( const QString &layerurl, QString &errorTitle, QString &errorText )
{
  // http://sampleserver5.arcgisonline.com/arcgis/rest/services/Energy/Geology/FeatureServer/1?f=json
  QUrl queryUrl( layerurl );
  queryUrl.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
  return queryServiceJSON( queryUrl, errorTitle, errorText );
}

QVariantMap QgsArcGisRestUtils::getObjectIds( const QString &layerurl, const QString &objectIdFieldName, QString &errorTitle, QString &errorText, const QgsRectangle &bbox )
{
  // http://sampleserver5.arcgisonline.com/arcgis/rest/services/Energy/Geology/FeatureServer/1/query?where=objectid%3Dobjectid&returnIdsOnly=true&f=json
  QUrl queryUrl( layerurl + "/query" );
  queryUrl.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
  queryUrl.addQueryItem( QStringLiteral( "where" ), QStringLiteral( "%1=%1" ).arg( objectIdFieldName ) );
  queryUrl.addQueryItem( QStringLiteral( "returnIdsOnly" ), QStringLiteral( "true" ) );
  if ( !bbox.isNull() )
  {
    queryUrl.addQueryItem( QStringLiteral( "geometry" ), QStringLiteral( "%1,%2,%3,%4" )
                           .arg( bbox.xMinimum(), 0, 'f', -1 ).arg( bbox.yMinimum(), 0, 'f', -1 )
                           .arg( bbox.xMaximum(), 0, 'f', -1 ).arg( bbox.yMaximum(), 0, 'f', -1 ) );
    queryUrl.addQueryItem( QStringLiteral( "geometryType" ), QStringLiteral( "esriGeometryEnvelope" ) );
    queryUrl.addQueryItem( QStringLiteral( "spatialRel" ), QStringLiteral( "esriSpatialRelEnvelopeIntersects" ) );
  }
  return queryServiceJSON( queryUrl, errorTitle, errorText );
}

QVariantMap QgsArcGisRestUtils::getObjects( const QString &layerurl, const QList<quint32> &objectIds, const QString &crs,
    bool fetchGeometry, const QStringList &fetchAttributes,
    bool fetchM, bool fetchZ,
    const QgsRectangle &filterRect,
    QString &errorTitle, QString &errorText, QgsFeedback *feedback )
{
  QStringList ids;
  for ( int id : objectIds )
  {
    ids.append( QString::number( id ) );
  }
  QUrl queryUrl( layerurl + "/query" );
  queryUrl.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
  queryUrl.addQueryItem( QStringLiteral( "objectIds" ), ids.join( QStringLiteral( "," ) ) );
  QString wkid = crs.indexOf( QLatin1String( ":" ) ) >= 0 ? crs.split( ':' )[1] : QString();
  queryUrl.addQueryItem( QStringLiteral( "inSR" ), wkid );
  queryUrl.addQueryItem( QStringLiteral( "outSR" ), wkid );
  QString outFields = fetchAttributes.join( QStringLiteral( "," ) );
  if ( fetchGeometry )
  {
    queryUrl.addQueryItem( QStringLiteral( "returnGeometry" ), QStringLiteral( "true" ) );
    queryUrl.addQueryItem( QStringLiteral( "outFields" ), outFields );
  }
  else
  {
    queryUrl.addQueryItem( QStringLiteral( "returnGeometry" ), QStringLiteral( "false" ) );
    queryUrl.addQueryItem( QStringLiteral( "outFields" ), outFields );
  }
  queryUrl.addQueryItem( QStringLiteral( "returnM" ), fetchM ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  queryUrl.addQueryItem( QStringLiteral( "returnZ" ), fetchZ ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  if ( !filterRect.isNull() )
  {
    queryUrl.addQueryItem( QStringLiteral( "geometry" ), QStringLiteral( "%1,%2,%3,%4" )
                           .arg( filterRect.xMinimum(), 0, 'f', -1 ).arg( filterRect.yMinimum(), 0, 'f', -1 )
                           .arg( filterRect.xMaximum(), 0, 'f', -1 ).arg( filterRect.yMaximum(), 0, 'f', -1 ) );
    queryUrl.addQueryItem( QStringLiteral( "geometryType" ), QStringLiteral( "esriGeometryEnvelope" ) );
    queryUrl.addQueryItem( QStringLiteral( "spatialRel" ), QStringLiteral( "esriSpatialRelEnvelopeIntersects" ) );
  }
  return queryServiceJSON( queryUrl, errorTitle, errorText, feedback );
}

QList<quint32> QgsArcGisRestUtils::getObjectIdsByExtent( const QString &layerurl, const QString &objectIdField, const QgsRectangle &filterRect, QString &errorTitle, QString &errorText, QgsFeedback *feedback )
{
  QUrl queryUrl( layerurl + "/query" );
  queryUrl.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
  queryUrl.addQueryItem( QStringLiteral( "where" ), QStringLiteral( "%1=%1" ).arg( objectIdField ) );
  queryUrl.addQueryItem( QStringLiteral( "returnIdsOnly" ), QStringLiteral( "true" ) );
  queryUrl.addQueryItem( QStringLiteral( "geometry" ), QStringLiteral( "%1,%2,%3,%4" )
                         .arg( filterRect.xMinimum(), 0, 'f', -1 ).arg( filterRect.yMinimum(), 0, 'f', -1 )
                         .arg( filterRect.xMaximum(), 0, 'f', -1 ).arg( filterRect.yMaximum(), 0, 'f', -1 ) );
  queryUrl.addQueryItem( QStringLiteral( "geometryType" ), QStringLiteral( "esriGeometryEnvelope" ) );
  queryUrl.addQueryItem( QStringLiteral( "spatialRel" ), QStringLiteral( "esriSpatialRelEnvelopeIntersects" ) );
  const QVariantMap objectIdData = queryServiceJSON( queryUrl, errorTitle, errorText, feedback );

  if ( objectIdData.isEmpty() )
  {
    return QList<quint32>();
  }

  QList<quint32> ids;
  foreach ( const QVariant &objectId, objectIdData["objectIds"].toList() )
  {
    ids << objectId.toInt();
  }
  return ids;
}

QByteArray QgsArcGisRestUtils::queryService( const QUrl &u, QString &errorTitle, QString &errorText, QgsFeedback *feedback )
{
  QEventLoop loop;
  QUrl url = parseUrl( u );

  QNetworkRequest request( url );
  QNetworkReply *reply = nullptr;
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

  // Request data, handling redirects
  while ( true )
  {
    reply = nam->get( request );
    QObject::connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
    if ( feedback )
    {
      QObject::connect( feedback, &QgsFeedback::canceled, reply, &QNetworkReply::abort );
    }

    loop.exec( QEventLoop::ExcludeUserInputEvents );

    reply->deleteLater();

    if ( feedback && feedback->isCanceled() )
      return QByteArray();

    // Handle network errors
    if ( reply->error() != QNetworkReply::NoError )
    {
      QgsDebugMsg( QStringLiteral( "Network error: %1" ).arg( reply->errorString() ) );
      errorTitle = QStringLiteral( "Network error" );
      errorText = reply->errorString();
      return QByteArray();
    }

    // Handle HTTP redirects
    QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( redirect.isNull() )
    {
      break;
    }

    QgsDebugMsg( "redirecting to " + redirect.toUrl().toString() );
    request.setUrl( redirect.toUrl() );
  }
  QByteArray result = reply->readAll();
  return result;
}

QVariantMap QgsArcGisRestUtils::queryServiceJSON( const QUrl &url, QString &errorTitle, QString &errorText, QgsFeedback *feedback )
{
  QByteArray reply = queryService( url, errorTitle, errorText, feedback );
  if ( !errorTitle.isEmpty() )
  {
    return QVariantMap();
  }
  if ( feedback && feedback->isCanceled() )
    return QVariantMap();

  // Parse data
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson( reply, &err );
  if ( doc.isNull() )
  {
    errorTitle = QStringLiteral( "Parsing error" );
    errorText = err.errorString();
    QgsDebugMsg( QStringLiteral( "Parsing error: %1" ).arg( err.errorString() ) );
    return QVariantMap();
  }
  const QVariantMap res = doc.object().toVariantMap();
  if ( res.contains( QStringLiteral( "error" ) ) )
  {
    const QVariantMap error = res.value( QStringLiteral( "error" ) ).toMap();
    errorText = error.value( QStringLiteral( "message" ) ).toString();
    errorTitle = QObject::tr( "Error %1" ).arg( error.value( QStringLiteral( "code" ) ).toString() );
    return QVariantMap();
  }
  return res;
}

std::unique_ptr<QgsSymbol> QgsArcGisRestUtils::parseEsriSymbolJson( const QVariantMap &symbolData )
{
  const QString type = symbolData.value( QStringLiteral( "type" ) ).toString();
  if ( type == QLatin1String( "esriSMS" ) )
  {
    // marker symbol
    return parseEsriMarkerSymbolJson( symbolData );
  }
  else if ( type == QLatin1String( "esriSLS" ) )
  {
    // line symbol
    return parseEsriLineSymbolJson( symbolData );
  }
  else if ( type == QLatin1String( "esriSFS" ) )
  {
    // fill symbol
    return parseEsriFillSymbolJson( symbolData );
  }
  else if ( type == QLatin1String( "esriPFS" ) )
  {
    // picture fill - not supported
    return nullptr;
  }
  else if ( type == QLatin1String( "esriPMS" ) )
  {
    // picture marker - not supported
    return nullptr;
  }
  else if ( type == QLatin1String( "esriTS" ) )
  {
    // text symbol - not supported
    return nullptr;
  }
  return nullptr;
}

std::unique_ptr<QgsLineSymbol> QgsArcGisRestUtils::parseEsriLineSymbolJson( const QVariantMap &symbolData )
{
  QColor lineColor = parseEsriColorJson( symbolData.value( QStringLiteral( "color" ) ) );
  if ( !lineColor.isValid() )
    return nullptr;

  bool ok = false;
  double widthInPoints = symbolData.value( QStringLiteral( "width" ) ).toDouble( &ok );
  if ( !ok )
    return nullptr;

  QgsSymbolLayerList layers;
  Qt::PenStyle penStyle = parseEsriLineStyle( symbolData.value( QStringLiteral( "style" ) ).toString() );
  std::unique_ptr< QgsSimpleLineSymbolLayer > lineLayer = qgis::make_unique< QgsSimpleLineSymbolLayer >( lineColor, widthInPoints, penStyle );
  lineLayer->setWidthUnit( QgsUnitTypes::RenderPoints );
  layers.append( lineLayer.release() );

  std::unique_ptr< QgsLineSymbol > symbol = qgis::make_unique< QgsLineSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsFillSymbol> QgsArcGisRestUtils::parseEsriFillSymbolJson( const QVariantMap &symbolData )
{
  QColor fillColor = parseEsriColorJson( symbolData.value( QStringLiteral( "color" ) ) );
  Qt::BrushStyle brushStyle = parseEsriFillStyle( symbolData.value( QStringLiteral( "style" ) ).toString() );

  const QVariantMap outlineData = symbolData.value( QStringLiteral( "outline" ) ).toMap();
  QColor lineColor = parseEsriColorJson( outlineData.value( QStringLiteral( "color" ) ) );
  Qt::PenStyle penStyle = parseEsriLineStyle( outlineData.value( QStringLiteral( "style" ) ).toString() );
  bool ok = false;
  double penWidthInPoints = outlineData.value( QStringLiteral( "width" ) ).toDouble( &ok );

  QgsSymbolLayerList layers;
  std::unique_ptr< QgsSimpleFillSymbolLayer > fillLayer = qgis::make_unique< QgsSimpleFillSymbolLayer >( fillColor, brushStyle, lineColor, penStyle, penWidthInPoints );
  fillLayer->setStrokeWidthUnit( QgsUnitTypes::RenderPoints );
  layers.append( fillLayer.release() );

  std::unique_ptr< QgsFillSymbol > symbol = qgis::make_unique< QgsFillSymbol >( layers );
  return symbol;
}

QgsSimpleMarkerSymbolLayerBase::Shape parseEsriMarkerShape( const QString &style )
{
  if ( style == QLatin1String( "esriSMSCircle" ) )
    return QgsSimpleMarkerSymbolLayerBase::Circle;
  else if ( style == QLatin1String( "esriSMSCross" ) )
    return QgsSimpleMarkerSymbolLayerBase::Cross;
  else if ( style == QLatin1String( "esriSMSDiamond" ) )
    return QgsSimpleMarkerSymbolLayerBase::Diamond;
  else if ( style == QLatin1String( "esriSMSSquare" ) )
    return QgsSimpleMarkerSymbolLayerBase::Square;
  else if ( style == QLatin1String( "esriSMSX" ) )
    return QgsSimpleMarkerSymbolLayerBase::Cross2;
  else if ( style == QLatin1String( "esriSMSTriangle" ) )
    return QgsSimpleMarkerSymbolLayerBase::Triangle;
  else
    return QgsSimpleMarkerSymbolLayerBase::Circle;
}

std::unique_ptr<QgsMarkerSymbol> QgsArcGisRestUtils::parseEsriMarkerSymbolJson( const QVariantMap &symbolData )
{
  QColor fillColor = parseEsriColorJson( symbolData.value( QStringLiteral( "color" ) ) );
  bool ok = false;
  const double sizeInPoints = symbolData.value( QStringLiteral( "size" ) ).toDouble( &ok );
  if ( !ok )
    return nullptr;
  const double angleCCW = symbolData.value( QStringLiteral( "angle" ) ).toDouble( &ok );
  double angleCW = 0;
  if ( ok )
    angleCW = -angleCCW;

  QgsSimpleMarkerSymbolLayerBase::Shape shape = parseEsriMarkerShape( symbolData.value( QStringLiteral( "style" ) ).toString() );

  const double xOffset = symbolData.value( QStringLiteral( "xoffset" ) ).toDouble();
  const double yOffset = symbolData.value( QStringLiteral( "yoffset" ) ).toDouble();

  const QVariantMap outlineData = symbolData.value( QStringLiteral( "outline" ) ).toMap();
  QColor lineColor = parseEsriColorJson( outlineData.value( QStringLiteral( "color" ) ) );
  Qt::PenStyle penStyle = parseEsriLineStyle( outlineData.value( QStringLiteral( "style" ) ).toString() );
  double penWidthInPoints = outlineData.value( QStringLiteral( "width" ) ).toDouble( &ok );

  QgsSymbolLayerList layers;
  std::unique_ptr< QgsSimpleMarkerSymbolLayer > markerLayer = qgis::make_unique< QgsSimpleMarkerSymbolLayer >( shape, sizeInPoints, angleCW, QgsSymbol::ScaleArea, fillColor, lineColor );
  markerLayer->setSizeUnit( QgsUnitTypes::RenderPoints );
  markerLayer->setStrokeWidthUnit( QgsUnitTypes::RenderPoints );
  markerLayer->setStrokeStyle( penStyle );
  markerLayer->setStrokeWidth( penWidthInPoints );
  markerLayer->setOffset( QPointF( xOffset, yOffset ) );
  markerLayer->setOffsetUnit( QgsUnitTypes::RenderPoints );
  layers.append( markerLayer.release() );

  std::unique_ptr< QgsMarkerSymbol > symbol = qgis::make_unique< QgsMarkerSymbol >( layers );
  return symbol;
}

QgsFeatureRenderer *QgsArcGisRestUtils::parseEsriRenderer( const QVariantMap &rendererData )
{
  const QString type = rendererData.value( QStringLiteral( "type" ) ).toString();
  if ( type == QLatin1String( "simple" ) )
  {
    const QVariantMap symbolProps = rendererData.value( QStringLiteral( "symbol" ) ).toMap();
    std::unique_ptr< QgsSymbol > symbol = parseEsriSymbolJson( symbolProps );
    if ( symbol )
      return new QgsSingleSymbolRenderer( symbol.release() );
    else
      return nullptr;
  }
  else if ( type == QLatin1String( "uniqueValue" ) )
  {
    const QString attribute = rendererData.value( QStringLiteral( "field1" ) ).toString();
    // TODO - handle field2, field3
    const QVariantList categories = rendererData.value( QStringLiteral( "uniqueValueInfos" ) ).toList();
    QgsCategoryList categoryList;
    for ( const QVariant &category : categories )
    {
      const QVariantMap categoryData = category.toMap();
      const QString value = categoryData.value( QStringLiteral( "value" ) ).toString();
      const QString label = categoryData.value( QStringLiteral( "label" ) ).toString();
      std::unique_ptr< QgsSymbol > symbol = QgsArcGisRestUtils::parseEsriSymbolJson( categoryData.value( QStringLiteral( "symbol" ) ).toMap() );
      if ( symbol )
      {
        categoryList.append( QgsRendererCategory( value, symbol.release(), label ) );
      }
    }

    std::unique_ptr< QgsSymbol > defaultSymbol = parseEsriSymbolJson( rendererData.value( QStringLiteral( "defaultSymbol" ) ).toMap() );
    if ( defaultSymbol )
    {
      categoryList.append( QgsRendererCategory( QVariant(), defaultSymbol.release(), rendererData.value( QStringLiteral( "defaultLabel" ) ).toString() ) );
    }

    if ( categoryList.empty() )
      return nullptr;

    return new QgsCategorizedSymbolRenderer( attribute, categoryList );
  }
  else if ( type == QLatin1String( "classBreaks" ) )
  {
    // currently unsupported
    return nullptr;
  }
  else if ( type == QLatin1String( "heatmap" ) )
  {
    // currently unsupported
    return nullptr;
  }
  else if ( type == QLatin1String( "vectorField" ) )
  {
    // currently unsupported
    return nullptr;
  }
  return nullptr;
}

QColor QgsArcGisRestUtils::parseEsriColorJson( const QVariant &colorData )
{
  const QVariantList colorParts = colorData.toList();
  if ( colorParts.count() < 4 )
    return QColor();

  int red = colorParts.at( 0 ).toInt();
  int green = colorParts.at( 1 ).toInt();
  int blue = colorParts.at( 2 ).toInt();
  int alpha = colorParts.at( 3 ).toInt();
  return QColor( red, green, blue, alpha );
}

Qt::PenStyle QgsArcGisRestUtils::parseEsriLineStyle( const QString &style )
{
  if ( style == QLatin1String( "esriSLSSolid" ) )
    return Qt::SolidLine;
  else if ( style == QLatin1String( "esriSLSDash" ) )
    return Qt::DashLine;
  else if ( style == QLatin1String( "esriSLSDashDot" ) )
    return Qt::DashDotLine;
  else if ( style == QLatin1String( "esriSLSDashDotDot" ) )
    return Qt::DashDotDotLine;
  else if ( style == QLatin1String( "esriSLSDot" ) )
    return Qt::DotLine;
  else if ( style == QLatin1String( "esriSLSNull" ) )
    return Qt::NoPen;
  else
    return Qt::SolidLine;
}

Qt::BrushStyle QgsArcGisRestUtils::parseEsriFillStyle( const QString &style )
{
  if ( style == QLatin1String( "esriSFSBackwardDiagonal" ) )
    return Qt::BDiagPattern;
  else if ( style == QLatin1String( "esriSFSCross" ) )
    return Qt::CrossPattern;
  else if ( style == QLatin1String( "esriSFSDiagonalCross" ) )
    return Qt::DiagCrossPattern;
  else if ( style == QLatin1String( "esriSFSForwardDiagonal" ) )
    return Qt::FDiagPattern;
  else if ( style == QLatin1String( "esriSFSHorizontal" ) )
    return Qt::HorPattern;
  else if ( style == QLatin1String( "esriSFSNull" ) )
    return Qt::NoBrush;
  else if ( style == QLatin1String( "esriSFSSolid" ) )
    return Qt::SolidPattern;
  else if ( style == QLatin1String( "esriSFSVertical" ) )
    return Qt::VerPattern;
  else
    return Qt::SolidPattern;
}

QDateTime QgsArcGisRestUtils::parseDateTime( const QVariant &value )
{
  if ( value.isNull() )
    return QDateTime();
  bool ok = false;
  QDateTime dt = QDateTime::fromMSecsSinceEpoch( value.toLongLong( &ok ) );
  if ( !ok )
  {
    QgsDebugMsg( QStringLiteral( "Invalid value %1 for datetime" ).arg( value.toString() ) );
    return QDateTime();
  }
  else
    return dt;
}

QUrl QgsArcGisRestUtils::parseUrl( const QUrl &url )
{
  QUrl modifiedUrl( url );
  if ( modifiedUrl.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
  {
    // Just for testing with local files instead of http:// resources
    QString modifiedUrlString = modifiedUrl.toString();
    // Qt5 does URL encoding from some reason (of the FILTER parameter for example)
    modifiedUrlString = QUrl::fromPercentEncoding( modifiedUrlString.toUtf8() );
    modifiedUrlString.replace( QStringLiteral( "fake_qgis_http_endpoint/" ), QStringLiteral( "fake_qgis_http_endpoint_" ) );
    QgsDebugMsg( QStringLiteral( "Get %1" ).arg( modifiedUrlString ) );
    modifiedUrlString = modifiedUrlString.mid( QStringLiteral( "http://" ).size() );
    QString args = modifiedUrlString.mid( modifiedUrlString.indexOf( '?' ) );
    if ( modifiedUrlString.size() > 150 )
    {
      args = QCryptographicHash::hash( args.toUtf8(), QCryptographicHash::Md5 ).toHex();
    }
    else
    {
      args.replace( QLatin1String( "?" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "&" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "<" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( ">" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "'" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "\"" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( " " ), QLatin1String( "_" ) );
      args.replace( QLatin1String( ":" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "/" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "\n" ), QLatin1String( "_" ) );
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
    QgsDebugMsg( QStringLiteral( "Get %1 (after laundering)" ).arg( modifiedUrlString ) );
    modifiedUrl = QUrl::fromLocalFile( modifiedUrlString );
  }

  return modifiedUrl;
}

///////////////////////////////////////////////////////////////////////////////

QgsArcGisAsyncQuery::QgsArcGisAsyncQuery( QObject *parent )
  : QObject( parent )
{
}

QgsArcGisAsyncQuery::~QgsArcGisAsyncQuery()
{
  if ( mReply )
    mReply->deleteLater();
}

void QgsArcGisAsyncQuery::start( const QUrl &url, QByteArray *result, bool allowCache )
{
  mResult = result;
  QNetworkRequest request( url );
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
    QgsDebugMsg( QStringLiteral( "Network error: %1" ).arg( mReply->errorString() ) );
    emit failed( QStringLiteral( "Network error" ), mReply->errorString() );
    return;
  }

  // Handle HTTP redirects
  QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  if ( !redirect.isNull() )
  {
    QNetworkRequest request = mReply->request();
    QgsDebugMsg( "redirecting to " + redirect.toUrl().toString() );
    request.setUrl( redirect.toUrl() );
    mReply = QgsNetworkAccessManager::instance()->get( request );
    connect( mReply, &QNetworkReply::finished, this, &QgsArcGisAsyncQuery::handleReply );
    return;
  }

  *mResult = mReply->readAll();
  mResult = nullptr;
  emit finished();
}

///////////////////////////////////////////////////////////////////////////////

QgsArcGisAsyncParallelQuery::QgsArcGisAsyncParallelQuery( QObject *parent )
  : QObject( parent )
{
}

void QgsArcGisAsyncParallelQuery::start( const QVector<QUrl> &urls, QVector<QByteArray> *results, bool allowCache )
{
  Q_ASSERT( results->size() == urls.size() );
  mResults = results;
  mPendingRequests = mResults->size();
  for ( int i = 0, n = urls.size(); i < n; ++i )
  {
    QNetworkRequest request( urls[i] );
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
  QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  int idx = reply->property( "idx" ).toInt();
  reply->deleteLater();
  if ( reply->error() != QNetworkReply::NoError )
  {
    // Handle network errors
    mErrors.append( reply->errorString() );
    --mPendingRequests;
  }
  else if ( !redirect.isNull() )
  {
    // Handle HTTP redirects
    QNetworkRequest request = reply->request();
    QgsDebugMsg( "redirecting to " + redirect.toUrl().toString() );
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
