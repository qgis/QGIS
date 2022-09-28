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
#include "qgsrectangle.h"
#include "qgspallabeling.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsrenderer.h"
#include "qgsrulebasedlabeling.h"
#include "qgssinglesymbolrenderer.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgscircularstring.h"
#include "qgsmulticurve.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgscurve.h"
#include "qgsgeometryengine.h"
#include "qgsmultisurface.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgsmultipoint.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsvariantutils.h"

#include <QRegularExpression>

QVariant::Type QgsArcGisRestUtils::convertFieldType( const QString &esriFieldType )
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
    return QVariant::DateTime;
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

QgsWkbTypes::Type QgsArcGisRestUtils::convertGeometryType( const QString &esriGeometryType )
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
    return QgsWkbTypes::MultiPolygon;
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

std::unique_ptr< QgsPoint > QgsArcGisRestUtils::convertPoint( const QVariantList &coordList, QgsWkbTypes::Type pointType )
{
  int nCoords = coordList.size();
  if ( nCoords < 2 )
    return nullptr;
  bool xok = false, yok = false;
  const double x = coordList[0].toDouble( &xok );
  const double y = coordList[1].toDouble( &yok );
  if ( !xok || !yok )
    return nullptr;
  const bool hasZ = QgsWkbTypes::hasZ( pointType );
  const double z = hasZ && nCoords >= 3 ? coordList[2].toDouble() : std::numeric_limits< double >::quiet_NaN();

  // if point has just M but not Z, then the point dimension list will only have X, Y, M, otherwise it will have X, Y, Z, M
  const double m = QgsWkbTypes::hasM( pointType ) && ( ( hasZ && nCoords >= 4 ) || ( !hasZ && nCoords >= 3 ) ) ? coordList[ hasZ ? 3 : 2].toDouble() : std::numeric_limits< double >::quiet_NaN();
  return std::make_unique< QgsPoint >( pointType, x, y, z, m );
}

std::unique_ptr< QgsCircularString > QgsArcGisRestUtils::convertCircularString( const QVariantMap &curveData, QgsWkbTypes::Type pointType, const QgsPoint &startPoint )
{
  const QVariantList coordsList = curveData[QStringLiteral( "c" )].toList();
  if ( coordsList.isEmpty() )
    return nullptr;
  const int coordsListSize = coordsList.size();

  QVector<QgsPoint> points;
  points.reserve( coordsListSize + 1 );
  points.append( startPoint );

  for ( int i = 0; i < coordsListSize - 1; )
  {
    // first point is end point, second is point on curve
    // i.e. the opposite to what QGIS requires!
    std::unique_ptr< QgsPoint > endPoint( convertPoint( coordsList.at( i ).toList(), pointType ) );
    if ( !endPoint )
      return nullptr;
    i++;
    std::unique_ptr< QgsPoint > interiorPoint( convertPoint( coordsList.at( i ).toList(), pointType ) );
    if ( !interiorPoint )
      return nullptr;
    i++;
    points << *interiorPoint;
    points << *endPoint;
  }
  std::unique_ptr< QgsCircularString > curve = std::make_unique< QgsCircularString> ();
  curve->setPoints( points );
  return curve;
}

std::unique_ptr< QgsCompoundCurve > QgsArcGisRestUtils::convertCompoundCurve( const QVariantList &curvesList, QgsWkbTypes::Type pointType )
{
  // [[6,3],[5,3],{"b":[[3,2],[6,1],[2,4]]},[1,2],{"c": [[3,3],[1,4]]}]
  std::unique_ptr< QgsCompoundCurve > compoundCurve = std::make_unique< QgsCompoundCurve >();

  QVector< double > lineX;
  QVector< double > lineY;
  QVector< double > lineZ;
  QVector< double > lineM;
  int maxCurveListSize = curvesList.size();
  lineX.resize( maxCurveListSize );
  lineY.resize( maxCurveListSize );

  const bool hasZ = QgsWkbTypes::hasZ( pointType );
  if ( hasZ )
    lineZ.resize( maxCurveListSize );
  const bool hasM = QgsWkbTypes::hasM( pointType );
  if ( hasM )
    lineM.resize( maxCurveListSize );

  double *outLineX = lineX.data();
  double *outLineY = lineY.data();
  double *outLineZ = lineZ.data();
  double *outLineM = lineM.data();
  int actualLineSize = 0;

  bool xok = false;
  bool yok = false;

  int curveListIndex = 0;
  for ( const QVariant &curveData : curvesList )
  {
    if ( curveData.type() == QVariant::List )
    {
      const QVariantList coordList = curveData.toList();
      const int nCoords = coordList.size();
      if ( nCoords < 2 )
        return nullptr;

      const double x = coordList[0].toDouble( &xok );
      const double y = coordList[1].toDouble( &yok );
      if ( !xok || !yok )
        return nullptr;

      actualLineSize++;
      *outLineX++ = x;
      *outLineY++ = y;
      if ( hasZ )
      {
        *outLineZ++ = nCoords >= 3 ? coordList[2].toDouble() : std::numeric_limits< double >::quiet_NaN();
      }

      if ( hasM )
      {
        // if point has just M but not Z, then the point dimension list will only have X, Y, M, otherwise it will have X, Y, Z, M
        *outLineM++ = ( ( hasZ && nCoords >= 4 ) || ( !hasZ && nCoords >= 3 ) ) ? coordList[ hasZ ? 3 : 2].toDouble() : std::numeric_limits< double >::quiet_NaN();
      }
    }
    else if ( curveData.type() == QVariant::Map )
    {
      // The last point of the linestring is the start point of this circular string
      QgsPoint lastLineStringPoint;
      if ( actualLineSize > 0 )
      {
        lastLineStringPoint = QgsPoint( lineX.at( actualLineSize - 1 ),
                                        lineY.at( actualLineSize - 1 ),
                                        hasZ ? lineZ.at( actualLineSize - 1 ) : std::numeric_limits< double >::quiet_NaN(),
                                        hasM ? lineM.at( actualLineSize - 1 ) : std::numeric_limits< double >::quiet_NaN() );
      }
      std::unique_ptr< QgsCircularString > circularString( convertCircularString( curveData.toMap(), pointType, lastLineStringPoint ) );
      if ( !circularString )
      {
        return nullptr;
      }

      if ( actualLineSize > 0 )
      {
        lineX.resize( actualLineSize );
        lineY.resize( actualLineSize );
        if ( hasZ )
          lineZ.resize( actualLineSize );
        if ( hasM )
          lineM.resize( actualLineSize );

        compoundCurve->addCurve( new QgsLineString( lineX, lineY, lineZ, lineM ) );
        lineX.resize( maxCurveListSize - curveListIndex );
        lineY.resize( maxCurveListSize - curveListIndex );
        if ( hasZ )
          lineZ.resize( maxCurveListSize - curveListIndex );
        if ( hasM )
          lineM.resize( maxCurveListSize - curveListIndex );
        outLineX = lineX.data();
        outLineY = lineY.data();
        outLineZ = lineZ.data();
        outLineM = lineM.data();
      }

      // If the previous curve had less than two points, remove it
      if ( compoundCurve->curveAt( compoundCurve->nCurves() - 1 )->nCoordinates() < 2 )
        compoundCurve->removeCurve( compoundCurve->nCurves() - 1 );

      const QgsPoint endPointCircularString = circularString->endPoint();
      compoundCurve->addCurve( circularString.release() );

      // Prepare a new line string
      actualLineSize = 1;
      *outLineX++ = endPointCircularString.x();
      *outLineY++ = endPointCircularString.y();
      if ( hasZ )
        *outLineZ++ = endPointCircularString.z();
      if ( hasM )
        *outLineM++ = endPointCircularString.m();
    }
    curveListIndex++;
  }

  if ( actualLineSize == 1 && compoundCurve->nCurves() > 0 )
  {
    const QgsCurve *finalCurve = compoundCurve->curveAt( compoundCurve->nCurves() - 1 );
    const QgsPoint finalCurveEndPoint = finalCurve->endPoint();
    if ( qgsDoubleNear( finalCurveEndPoint.x(), lineX.at( 0 ) )
         && qgsDoubleNear( finalCurveEndPoint.y(), lineY.at( 0 ) )
         && ( !hasZ || qgsDoubleNear( finalCurveEndPoint.z(), lineZ.at( 0 ) ) )
         && ( !hasM || qgsDoubleNear( finalCurveEndPoint.m(), lineM.at( 0 ) ) ) )
    {
      actualLineSize = 0; // redundant final curve containing a duplicate vertex
    }
  }

  if ( actualLineSize > 0 )
  {
    lineX.resize( actualLineSize );
    lineY.resize( actualLineSize );
    if ( hasZ )
      lineZ.resize( actualLineSize );
    if ( hasM )
      lineM.resize( actualLineSize );
    compoundCurve->addCurve( new QgsLineString( lineX, lineY, lineZ, lineM ) );
  }

  return compoundCurve;
}

std::unique_ptr< QgsPoint > QgsArcGisRestUtils::convertGeometryPoint( const QVariantMap &geometryData, QgsWkbTypes::Type pointType )
{
  // {"x" : <x>, "y" : <y>, "z" : <z>, "m" : <m>}
  bool xok = false, yok = false;
  double x = geometryData[QStringLiteral( "x" )].toDouble( &xok );
  double y = geometryData[QStringLiteral( "y" )].toDouble( &yok );
  if ( !xok || !yok )
    return nullptr;
  double z = geometryData[QStringLiteral( "z" )].toDouble();
  double m = geometryData[QStringLiteral( "m" )].toDouble();
  return std::make_unique< QgsPoint >( pointType, x, y, z, m );
}

std::unique_ptr< QgsMultiPoint > QgsArcGisRestUtils::convertMultiPoint( const QVariantMap &geometryData, QgsWkbTypes::Type pointType )
{
  // {"points" : [[ <x1>, <y1>, <z1>, <m1> ] , [ <x2>, <y2>, <z2>, <m2> ], ... ]}
  const QVariantList coordsList = geometryData[QStringLiteral( "points" )].toList();

  std::unique_ptr< QgsMultiPoint > multiPoint = std::make_unique< QgsMultiPoint >();
  multiPoint->reserve( coordsList.size() );
  for ( const QVariant &coordData : coordsList )
  {
    const QVariantList coordList = coordData.toList();
    std::unique_ptr< QgsPoint > p = convertPoint( coordList, pointType );
    if ( !p )
    {
      continue;
    }
    multiPoint->addGeometry( p.release() );
  }

  // second chance -- sometimes layers are reported as multipoint but features have single
  // point geometries. Silently handle this and upgrade to multipoint.
  std::unique_ptr< QgsPoint > p = convertGeometryPoint( geometryData, pointType );
  if ( p )
    multiPoint->addGeometry( p.release() );

  if ( multiPoint->numGeometries() == 0 )
  {
    // didn't find any points, so reset geometry to null
    multiPoint.reset();
  }
  return multiPoint;
}

std::unique_ptr< QgsMultiCurve > QgsArcGisRestUtils::convertGeometryPolyline( const QVariantMap &geometryData, QgsWkbTypes::Type pointType )
{
  // {"curvePaths": [[[0,0], {"c": [[3,3],[1,4]]} ]]}
  QVariantList pathsList;
  if ( geometryData[QStringLiteral( "paths" )].isValid() )
    pathsList = geometryData[QStringLiteral( "paths" )].toList();
  else if ( geometryData[QStringLiteral( "curvePaths" )].isValid() )
    pathsList = geometryData[QStringLiteral( "curvePaths" )].toList();
  if ( pathsList.isEmpty() )
    return nullptr;
  std::unique_ptr< QgsMultiCurve > multiCurve = std::make_unique< QgsMultiCurve >();
  multiCurve->reserve( pathsList.size() );
  for ( const QVariant &pathData : std::as_const( pathsList ) )
  {
    std::unique_ptr< QgsCompoundCurve > curve = convertCompoundCurve( pathData.toList(), pointType );
    if ( !curve )
    {
      return nullptr;
    }
    multiCurve->addGeometry( curve.release() );
  }
  return multiCurve;
}

std::unique_ptr< QgsMultiSurface > QgsArcGisRestUtils::convertGeometryPolygon( const QVariantMap &geometryData, QgsWkbTypes::Type pointType )
{
  // {"curveRings": [[[0,0], {"c": [[3,3],[1,4]]} ]]}
  QVariantList ringsList;
  if ( geometryData[QStringLiteral( "rings" )].isValid() )
    ringsList = geometryData[QStringLiteral( "rings" )].toList();
  else if ( geometryData[QStringLiteral( "ringPaths" )].isValid() )
    ringsList = geometryData[QStringLiteral( "ringPaths" )].toList();
  if ( ringsList.isEmpty() )
    return nullptr;

  QList< QgsCompoundCurve * > curves;
  for ( int i = 0, n = ringsList.size(); i < n; ++i )
  {
    std::unique_ptr< QgsCompoundCurve > curve = convertCompoundCurve( ringsList[i].toList(), pointType );
    if ( !curve )
    {
      continue;
    }
    curves.append( curve.release() );
  }
  if ( curves.count() == 0 )
    return nullptr;

  std::unique_ptr< QgsMultiSurface > result = std::make_unique< QgsMultiSurface >();
  if ( curves.count() == 1 )
  {
    // shortcut for exterior ring only
    std::unique_ptr< QgsCurvePolygon > newPolygon = std::make_unique< QgsCurvePolygon >();
    newPolygon->setExteriorRing( curves.takeAt( 0 ) );
    result->addGeometry( newPolygon.release() );
    return result;
  }

  std::sort( curves.begin(), curves.end(), []( const QgsCompoundCurve * a, const QgsCompoundCurve * b )->bool{ double a_area = 0.0; double b_area = 0.0; a->sumUpArea( a_area ); b->sumUpArea( b_area ); return std::abs( a_area ) > std::abs( b_area ); } );
  result->reserve( curves.size() );
  while ( !curves.isEmpty() )
  {
    QgsCompoundCurve *exterior = curves.takeFirst();
    QgsCurvePolygon *newPolygon = new QgsCurvePolygon();
    newPolygon->setExteriorRing( exterior );
    std::unique_ptr<QgsGeometryEngine> engine( QgsGeometry::createGeometryEngine( newPolygon ) );
    engine->prepareGeometry();

    QMutableListIterator< QgsCompoundCurve * > it( curves );
    while ( it.hasNext() )
    {
      QgsCompoundCurve *curve = it.next();
      QgsRectangle boundingBox = newPolygon->boundingBox();
      if ( boundingBox.intersects( curve->boundingBox() ) )
      {
        QgsPoint point = curve->startPoint();
        if ( engine->contains( &point ) )
        {
          newPolygon->addInteriorRing( curve );
          it.remove();
          engine.reset( QgsGeometry::createGeometryEngine( newPolygon ) );
          engine->prepareGeometry();
        }
      }
    }
    result->addGeometry( newPolygon );
  }
  if ( result->numGeometries() == 0 )
    return nullptr;

  return result;
}

std::unique_ptr< QgsPolygon > QgsArcGisRestUtils::convertEnvelope( const QVariantMap &geometryData )
{
  // {"xmin" : -109.55, "ymin" : 25.76, "xmax" : -86.39, "ymax" : 49.94}
  bool xminOk = false, yminOk = false, xmaxOk = false, ymaxOk = false;
  double xmin = geometryData[QStringLiteral( "xmin" )].toDouble( &xminOk );
  double ymin = geometryData[QStringLiteral( "ymin" )].toDouble( &yminOk );
  double xmax = geometryData[QStringLiteral( "xmax" )].toDouble( &xmaxOk );
  double ymax = geometryData[QStringLiteral( "ymax" )].toDouble( &ymaxOk );
  if ( !xminOk || !yminOk || !xmaxOk || !ymaxOk )
    return nullptr;
  std::unique_ptr< QgsLineString > ext = std::make_unique< QgsLineString> ();
  ext->addVertex( QgsPoint( xmin, ymin ) );
  ext->addVertex( QgsPoint( xmax, ymin ) );
  ext->addVertex( QgsPoint( xmax, ymax ) );
  ext->addVertex( QgsPoint( xmin, ymax ) );
  ext->addVertex( QgsPoint( xmin, ymin ) );
  std::unique_ptr< QgsPolygon > poly = std::make_unique< QgsPolygon >();
  poly->setExteriorRing( ext.release() );
  return poly;
}

QgsAbstractGeometry *QgsArcGisRestUtils::convertGeometry( const QVariantMap &geometryData, const QString &esriGeometryType, bool readM, bool readZ, QgsCoordinateReferenceSystem *crs )
{
  QgsWkbTypes::Type pointType = QgsWkbTypes::zmType( QgsWkbTypes::Point, readZ, readM );
  if ( crs )
  {
    *crs = convertSpatialReference( geometryData[QStringLiteral( "spatialReference" )].toMap() );
  }

  // http://resources.arcgis.com/en/help/arcgis-rest-api/index.html#/Geometry_Objects/02r3000000n1000000/
  if ( esriGeometryType == QLatin1String( "esriGeometryNull" ) )
    return nullptr;
  else if ( esriGeometryType == QLatin1String( "esriGeometryPoint" ) )
    return convertGeometryPoint( geometryData, pointType ).release();
  else if ( esriGeometryType == QLatin1String( "esriGeometryMultipoint" ) )
    return convertMultiPoint( geometryData, pointType ).release();
  else if ( esriGeometryType == QLatin1String( "esriGeometryPolyline" ) )
    return convertGeometryPolyline( geometryData, pointType ).release();
  else if ( esriGeometryType == QLatin1String( "esriGeometryPolygon" ) )
    return convertGeometryPolygon( geometryData, pointType ).release();
  else if ( esriGeometryType == QLatin1String( "esriGeometryEnvelope" ) )
    return convertEnvelope( geometryData ).release();
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

QgsCoordinateReferenceSystem QgsArcGisRestUtils::convertSpatialReference( const QVariantMap &spatialReferenceMap )
{
  QgsCoordinateReferenceSystem crs;

  QString spatialReference = spatialReferenceMap[QStringLiteral( "latestWkid" )].toString();
  if ( spatialReference.isEmpty() )
    spatialReference = spatialReferenceMap[QStringLiteral( "wkid" )].toString();

  // prefer using authority/id wherever we can
  if ( !spatialReference.isEmpty() )
  {
    crs.createFromString( QStringLiteral( "EPSG:%1" ).arg( spatialReference ) );
    if ( !crs.isValid() )
    {
      // Try as an ESRI auth
      crs.createFromString( QStringLiteral( "ESRI:%1" ).arg( spatialReference ) );
    }
  }
  else if ( !spatialReferenceMap[QStringLiteral( "wkt" )].toString().isEmpty() )
  {
    // otherwise fallback to WKT
    crs.createFromWkt( spatialReferenceMap[QStringLiteral( "wkt" )].toString() );
  }

  if ( !crs.isValid() )
  {
    // If no spatial reference, just use WGS84
    // TODO -- this needs further investigation! Most ESRI server services default to 3857, so that would likely be
    // a safer fallback to use...
    crs.createFromString( QStringLiteral( "EPSG:4326" ) );
  }
  return crs;
}

QgsSymbol *QgsArcGisRestUtils::convertSymbol( const QVariantMap &symbolData )
{
  const QString type = symbolData.value( QStringLiteral( "type" ) ).toString();
  if ( type == QLatin1String( "esriSMS" ) )
  {
    // marker symbol
    return parseEsriMarkerSymbolJson( symbolData ).release();
  }
  else if ( type == QLatin1String( "esriSLS" ) )
  {
    // line symbol
    return parseEsriLineSymbolJson( symbolData ).release();
  }
  else if ( type == QLatin1String( "esriSFS" ) )
  {
    // fill symbol
    return parseEsriFillSymbolJson( symbolData ).release();
  }
  else if ( type == QLatin1String( "esriPFS" ) )
  {
    return parseEsriPictureFillSymbolJson( symbolData ).release();
  }
  else if ( type == QLatin1String( "esriPMS" ) )
  {
    // picture marker
    return parseEsriPictureMarkerSymbolJson( symbolData ).release();
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
  QColor lineColor = convertColor( symbolData.value( QStringLiteral( "color" ) ) );
  if ( !lineColor.isValid() )
    return nullptr;

  bool ok = false;
  double widthInPoints = symbolData.value( QStringLiteral( "width" ) ).toDouble( &ok );
  if ( !ok )
    return nullptr;

  QgsSymbolLayerList layers;
  Qt::PenStyle penStyle = convertLineStyle( symbolData.value( QStringLiteral( "style" ) ).toString() );
  std::unique_ptr< QgsSimpleLineSymbolLayer > lineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( lineColor, widthInPoints, penStyle );
  lineLayer->setWidthUnit( QgsUnitTypes::RenderPoints );
  layers.append( lineLayer.release() );

  std::unique_ptr< QgsLineSymbol > symbol = std::make_unique< QgsLineSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsFillSymbol> QgsArcGisRestUtils::parseEsriFillSymbolJson( const QVariantMap &symbolData )
{
  QColor fillColor = convertColor( symbolData.value( QStringLiteral( "color" ) ) );
  Qt::BrushStyle brushStyle = convertFillStyle( symbolData.value( QStringLiteral( "style" ) ).toString() );

  const QVariantMap outlineData = symbolData.value( QStringLiteral( "outline" ) ).toMap();
  QColor lineColor = convertColor( outlineData.value( QStringLiteral( "color" ) ) );
  Qt::PenStyle penStyle = convertLineStyle( outlineData.value( QStringLiteral( "style" ) ).toString() );
  bool ok = false;
  double penWidthInPoints = outlineData.value( QStringLiteral( "width" ) ).toDouble( &ok );

  QgsSymbolLayerList layers;
  std::unique_ptr< QgsSimpleFillSymbolLayer > fillLayer = std::make_unique< QgsSimpleFillSymbolLayer >( fillColor, brushStyle, lineColor, penStyle, penWidthInPoints );
  fillLayer->setStrokeWidthUnit( QgsUnitTypes::RenderPoints );
  layers.append( fillLayer.release() );

  std::unique_ptr< QgsFillSymbol > symbol = std::make_unique< QgsFillSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsFillSymbol> QgsArcGisRestUtils::parseEsriPictureFillSymbolJson( const QVariantMap &symbolData )
{
  bool ok = false;

  double widthInPixels = symbolData.value( QStringLiteral( "width" ) ).toInt( &ok );
  if ( !ok )
    return nullptr;

  const double xScale = symbolData.value( QStringLiteral( "xscale" ) ).toDouble( &ok );
  if ( !qgsDoubleNear( xScale, 0.0 ) )
    widthInPixels *= xScale;

  const double angleCCW = symbolData.value( QStringLiteral( "angle" ) ).toDouble( &ok );
  double angleCW = 0;
  if ( ok )
    angleCW = -angleCCW;

  const double xOffset = symbolData.value( QStringLiteral( "xoffset" ) ).toDouble();
  const double yOffset = symbolData.value( QStringLiteral( "yoffset" ) ).toDouble();

  QString symbolPath( symbolData.value( QStringLiteral( "imageData" ) ).toString() );
  symbolPath.prepend( QLatin1String( "base64:" ) );

  QgsSymbolLayerList layers;
  std::unique_ptr< QgsRasterFillSymbolLayer > fillLayer = std::make_unique< QgsRasterFillSymbolLayer >( symbolPath );
  fillLayer->setWidth( widthInPixels );
  fillLayer->setAngle( angleCW );
  fillLayer->setWidthUnit( QgsUnitTypes::RenderPoints );
  fillLayer->setOffset( QPointF( xOffset, yOffset ) );
  fillLayer->setOffsetUnit( QgsUnitTypes::RenderPoints );
  layers.append( fillLayer.release() );

  const QVariantMap outlineData = symbolData.value( QStringLiteral( "outline" ) ).toMap();
  QColor lineColor = convertColor( outlineData.value( QStringLiteral( "color" ) ) );
  Qt::PenStyle penStyle = convertLineStyle( outlineData.value( QStringLiteral( "style" ) ).toString() );
  double penWidthInPoints = outlineData.value( QStringLiteral( "width" ) ).toDouble( &ok );

  std::unique_ptr< QgsSimpleLineSymbolLayer > lineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( lineColor, penWidthInPoints, penStyle );
  lineLayer->setWidthUnit( QgsUnitTypes::RenderPoints );
  layers.append( lineLayer.release() );

  std::unique_ptr< QgsFillSymbol > symbol = std::make_unique< QgsFillSymbol >( layers );
  return symbol;
}

Qgis::MarkerShape QgsArcGisRestUtils::parseEsriMarkerShape( const QString &style )
{
  if ( style == QLatin1String( "esriSMSCircle" ) )
    return Qgis::MarkerShape::Circle;
  else if ( style == QLatin1String( "esriSMSCross" ) )
    return Qgis::MarkerShape::Cross;
  else if ( style == QLatin1String( "esriSMSDiamond" ) )
    return Qgis::MarkerShape::Diamond;
  else if ( style == QLatin1String( "esriSMSSquare" ) )
    return Qgis::MarkerShape::Square;
  else if ( style == QLatin1String( "esriSMSX" ) )
    return Qgis::MarkerShape::Cross2;
  else if ( style == QLatin1String( "esriSMSTriangle" ) )
    return Qgis::MarkerShape::Triangle;
  else
    return Qgis::MarkerShape::Circle;
}

std::unique_ptr<QgsMarkerSymbol> QgsArcGisRestUtils::parseEsriMarkerSymbolJson( const QVariantMap &symbolData )
{
  QColor fillColor = convertColor( symbolData.value( QStringLiteral( "color" ) ) );
  bool ok = false;
  const double sizeInPoints = symbolData.value( QStringLiteral( "size" ) ).toDouble( &ok );
  if ( !ok )
    return nullptr;
  const double angleCCW = symbolData.value( QStringLiteral( "angle" ) ).toDouble( &ok );
  double angleCW = 0;
  if ( ok )
    angleCW = -angleCCW;

  Qgis::MarkerShape shape = parseEsriMarkerShape( symbolData.value( QStringLiteral( "style" ) ).toString() );

  const double xOffset = symbolData.value( QStringLiteral( "xoffset" ) ).toDouble();
  const double yOffset = symbolData.value( QStringLiteral( "yoffset" ) ).toDouble();

  const QVariantMap outlineData = symbolData.value( QStringLiteral( "outline" ) ).toMap();
  QColor lineColor = convertColor( outlineData.value( QStringLiteral( "color" ) ) );
  Qt::PenStyle penStyle = convertLineStyle( outlineData.value( QStringLiteral( "style" ) ).toString() );
  double penWidthInPoints = outlineData.value( QStringLiteral( "width" ) ).toDouble( &ok );

  QgsSymbolLayerList layers;
  std::unique_ptr< QgsSimpleMarkerSymbolLayer > markerLayer = std::make_unique< QgsSimpleMarkerSymbolLayer >( shape, sizeInPoints, angleCW, Qgis::ScaleMethod::ScaleArea, fillColor, lineColor );
  markerLayer->setSizeUnit( QgsUnitTypes::RenderPoints );
  markerLayer->setStrokeWidthUnit( QgsUnitTypes::RenderPoints );
  markerLayer->setStrokeStyle( penStyle );
  markerLayer->setStrokeWidth( penWidthInPoints );
  markerLayer->setOffset( QPointF( xOffset, yOffset ) );
  markerLayer->setOffsetUnit( QgsUnitTypes::RenderPoints );
  layers.append( markerLayer.release() );

  std::unique_ptr< QgsMarkerSymbol > symbol = std::make_unique< QgsMarkerSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsMarkerSymbol> QgsArcGisRestUtils::parseEsriPictureMarkerSymbolJson( const QVariantMap &symbolData )
{
  bool ok = false;
  const double widthInPixels = symbolData.value( QStringLiteral( "width" ) ).toInt( &ok );
  if ( !ok )
    return nullptr;
  const double heightInPixels = symbolData.value( QStringLiteral( "height" ) ).toInt( &ok );
  if ( !ok )
    return nullptr;

  const double angleCCW = symbolData.value( QStringLiteral( "angle" ) ).toDouble( &ok );
  double angleCW = 0;
  if ( ok )
    angleCW = -angleCCW;

  const double xOffset = symbolData.value( QStringLiteral( "xoffset" ) ).toDouble();
  const double yOffset = symbolData.value( QStringLiteral( "yoffset" ) ).toDouble();

  //const QString contentType = symbolData.value( QStringLiteral( "contentType" ) ).toString();

  QString symbolPath( symbolData.value( QStringLiteral( "imageData" ) ).toString() );
  symbolPath.prepend( QLatin1String( "base64:" ) );

  QgsSymbolLayerList layers;
  std::unique_ptr< QgsRasterMarkerSymbolLayer > markerLayer = std::make_unique< QgsRasterMarkerSymbolLayer >( symbolPath, widthInPixels, angleCW, Qgis::ScaleMethod::ScaleArea );
  markerLayer->setSizeUnit( QgsUnitTypes::RenderPoints );

  // only change the default aspect ratio if the server height setting requires this
  if ( !qgsDoubleNear( static_cast< double >( heightInPixels ) / widthInPixels, markerLayer->defaultAspectRatio() ) )
    markerLayer->setFixedAspectRatio( static_cast< double >( heightInPixels ) / widthInPixels );

  markerLayer->setOffset( QPointF( xOffset, yOffset ) );
  markerLayer->setOffsetUnit( QgsUnitTypes::RenderPoints );
  layers.append( markerLayer.release() );

  std::unique_ptr< QgsMarkerSymbol > symbol = std::make_unique< QgsMarkerSymbol >( layers );
  return symbol;
}

QgsAbstractVectorLayerLabeling *QgsArcGisRestUtils::convertLabeling( const QVariantList &labelingData )
{
  if ( labelingData.empty() )
    return nullptr;

  QgsRuleBasedLabeling::Rule *root = new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings(), 0, 0, QString(), QString(), false );
  root->setActive( true );

  int i = 1;
  for ( const QVariant &lbl : labelingData )
  {
    const QVariantMap labeling = lbl.toMap();

    QgsPalLayerSettings *settings = new QgsPalLayerSettings();
    QgsTextFormat format;

    const QString placement = labeling.value( QStringLiteral( "labelPlacement" ) ).toString();
    if ( placement == QLatin1String( "esriServerPointLabelPlacementAboveCenter" ) )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->quadOffset = Qgis::LabelQuadrantPosition::Above;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementBelowCenter" ) )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->quadOffset = Qgis::LabelQuadrantPosition::Below;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementCenterCenter" ) )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->quadOffset = Qgis::LabelQuadrantPosition::Over;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementAboveLeft" ) )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->quadOffset = Qgis::LabelQuadrantPosition::AboveLeft;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementBelowLeft" ) )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->quadOffset = Qgis::LabelQuadrantPosition::BelowLeft;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementCenterLeft" ) )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->quadOffset = Qgis::LabelQuadrantPosition::Left;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementAboveRight" ) )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->quadOffset = Qgis::LabelQuadrantPosition::AboveRight;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementBelowRight" ) )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->quadOffset = Qgis::LabelQuadrantPosition::BelowRight;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementCenterRight" ) )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->quadOffset = Qgis::LabelQuadrantPosition::Right;
    }
    else if ( placement == QLatin1String( "esriServerLinePlacementAboveAfter" ) ||
              placement == QLatin1String( "esriServerLinePlacementAboveStart" ) ||
              placement == QLatin1String( "esriServerLinePlacementAboveAlong" ) )
    {
      settings->placement = Qgis::LabelPlacement::Line;
      settings->lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::MapOrientation );
    }
    else if ( placement == QLatin1String( "esriServerLinePlacementBelowAfter" ) ||
              placement == QLatin1String( "esriServerLinePlacementBelowStart" ) ||
              placement == QLatin1String( "esriServerLinePlacementBelowAlong" ) )
    {
      settings->placement = Qgis::LabelPlacement::Line;
      settings->lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::BelowLine | QgsLabeling::LinePlacementFlag::MapOrientation );
    }
    else if ( placement == QLatin1String( "esriServerLinePlacementCenterAfter" ) ||
              placement == QLatin1String( "esriServerLinePlacementCenterStart" ) ||
              placement == QLatin1String( "esriServerLinePlacementCenterAlong" ) )
    {
      settings->placement = Qgis::LabelPlacement::Line;
      settings->lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::OnLine | QgsLabeling::LinePlacementFlag::MapOrientation );
    }
    else if ( placement == QLatin1String( "esriServerPolygonPlacementAlwaysHorizontal" ) )
    {
      settings->placement = Qgis::LabelPlacement::Horizontal;
    }

    const double minScale = labeling.value( QStringLiteral( "minScale" ) ).toDouble();
    const double maxScale = labeling.value( QStringLiteral( "maxScale" ) ).toDouble();

    QVariantMap symbol = labeling.value( QStringLiteral( "symbol" ) ).toMap();
    format.setColor( convertColor( symbol.value( QStringLiteral( "color" ) ) ) );
    const double haloSize = symbol.value( QStringLiteral( "haloSize" ) ).toDouble();
    if ( !qgsDoubleNear( haloSize, 0.0 ) )
    {
      QgsTextBufferSettings buffer;
      buffer.setEnabled( true );
      buffer.setSize( haloSize );
      buffer.setSizeUnit( QgsUnitTypes::RenderPoints );
      buffer.setColor( convertColor( symbol.value( QStringLiteral( "haloColor" ) ) ) );
      format.setBuffer( buffer );
    }

    const QString fontFamily = symbol.value( QStringLiteral( "font" ) ).toMap().value( QStringLiteral( "family" ) ).toString();
    const QString fontStyle = symbol.value( QStringLiteral( "font" ) ).toMap().value( QStringLiteral( "style" ) ).toString();
    const QString fontWeight = symbol.value( QStringLiteral( "font" ) ).toMap().value( QStringLiteral( "weight" ) ).toString();
    const int fontSize = symbol.value( QStringLiteral( "font" ) ).toMap().value( QStringLiteral( "size" ) ).toInt();
    QFont font( fontFamily, fontSize );
    font.setStyleName( fontStyle );
    font.setWeight( fontWeight == QLatin1String( "bold" ) ? QFont::Bold : QFont::Normal );

    format.setFont( font );
    format.setSize( fontSize );
    format.setSizeUnit( QgsUnitTypes::RenderPoints );

    settings->setFormat( format );

    QString where = labeling.value( QStringLiteral( "where" ) ).toString();
    QgsExpression exp( where );
    // If the where clause isn't parsed as valid, don't use its
    if ( !exp.isValid() )
      where.clear();

    settings->fieldName = convertLabelingExpression( labeling.value( QStringLiteral( "labelExpression" ) ).toString() );
    settings->isExpression = true;

    QgsRuleBasedLabeling::Rule *child = new QgsRuleBasedLabeling::Rule( settings, maxScale, minScale, where, QObject::tr( "ASF label %1" ).arg( i++ ), false );
    child->setActive( true );
    root->appendChild( child );
  }

  return new QgsRuleBasedLabeling( root );
}

QgsFeatureRenderer *QgsArcGisRestUtils::convertRenderer( const QVariantMap &rendererData )
{
  const QString type = rendererData.value( QStringLiteral( "type" ) ).toString();
  if ( type == QLatin1String( "simple" ) )
  {
    const QVariantMap symbolProps = rendererData.value( QStringLiteral( "symbol" ) ).toMap();
    std::unique_ptr< QgsSymbol > symbol( convertSymbol( symbolProps ) );
    if ( symbol )
      return new QgsSingleSymbolRenderer( symbol.release() );
    else
      return nullptr;
  }
  else if ( type == QLatin1String( "uniqueValue" ) )
  {
    const QString field1 = rendererData.value( QStringLiteral( "field1" ) ).toString();
    const QString field2 = rendererData.value( QStringLiteral( "field2" ) ).toString();
    const QString field3 = rendererData.value( QStringLiteral( "field3" ) ).toString();
    QString attribute;
    if ( !field2.isEmpty() || !field3.isEmpty() )
    {
      const QString delimiter = rendererData.value( QStringLiteral( "fieldDelimiter" ) ).toString();
      if ( !field3.isEmpty() )
      {
        attribute = QStringLiteral( "concat(\"%1\",'%2',\"%3\",'%4',\"%5\")" ).arg( field1, delimiter, field2, delimiter, field3 );
      }
      else
      {
        attribute = QStringLiteral( "concat(\"%1\",'%2',\"%3\")" ).arg( field1, delimiter, field2 );
      }
    }
    else
    {
      attribute = field1;
    }

    const QVariantList categories = rendererData.value( QStringLiteral( "uniqueValueInfos" ) ).toList();
    QgsCategoryList categoryList;
    for ( const QVariant &category : categories )
    {
      const QVariantMap categoryData = category.toMap();
      const QString value = categoryData.value( QStringLiteral( "value" ) ).toString();
      const QString label = categoryData.value( QStringLiteral( "label" ) ).toString();
      std::unique_ptr< QgsSymbol > symbol( QgsArcGisRestUtils::convertSymbol( categoryData.value( QStringLiteral( "symbol" ) ).toMap() ) );
      if ( symbol )
      {
        categoryList.append( QgsRendererCategory( value, symbol.release(), label ) );
      }
    }

    std::unique_ptr< QgsSymbol > defaultSymbol( convertSymbol( rendererData.value( QStringLiteral( "defaultSymbol" ) ).toMap() ) );
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

QString QgsArcGisRestUtils::convertLabelingExpression( const QString &string )
{
  QString expression = string;

  // Replace a few ArcGIS token to QGIS equivalents
  const thread_local QRegularExpression rx1 = QRegularExpression( QStringLiteral( "(?=([^\"\\\\]*(\\\\.|\"([^\"\\\\]*\\\\.)*[^\"\\\\]*\"))*[^\"]*$)(\\s|^)CONCAT(\\s|$)" ) );
  expression = expression.replace( rx1, QStringLiteral( "\\4||\\5" ) );

  const thread_local QRegularExpression rx2 = QRegularExpression( QStringLiteral( "(?=([^\"\\\\]*(\\\\.|\"([^\"\\\\]*\\\\.)*[^\"\\\\]*\"))*[^\"]*$)(\\s|^)NEWLINE(\\s|$)" ) );
  expression = expression.replace( rx2, QStringLiteral( "\\4'\\n'\\5" ) );

  // ArcGIS's double quotes are single quotes in QGIS
  const thread_local QRegularExpression rx3 = QRegularExpression( QStringLiteral( "\"(.*?(?<!\\\\))\"" ) );
  expression = expression.replace( rx3, QStringLiteral( "'\\1'" ) );
  const thread_local QRegularExpression rx4 = QRegularExpression( QStringLiteral( "\\\\\"" ) );
  expression = expression.replace( rx4, QStringLiteral( "\"" ) );

  // ArcGIS's square brakets are double quotes in QGIS
  const thread_local QRegularExpression rx5 = QRegularExpression( QStringLiteral( "\\[([^]]*)\\]" ) );
  expression = expression.replace( rx5, QStringLiteral( "\"\\1\"" ) );

  return expression;
}

QColor QgsArcGisRestUtils::convertColor( const QVariant &colorData )
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

Qt::PenStyle QgsArcGisRestUtils::convertLineStyle( const QString &style )
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

Qt::BrushStyle QgsArcGisRestUtils::convertFillStyle( const QString &style )
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

QDateTime QgsArcGisRestUtils::convertDateTime( const QVariant &value )
{
  if ( QgsVariantUtils::isNull( value ) )
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

QVariantMap QgsArcGisRestUtils::geometryToJson( const QgsGeometry &geometry, const QgsArcGisRestContext &, const QgsCoordinateReferenceSystem &crs )
{
  QVariantMap res;
  if ( geometry.isNull() )
    return QVariantMap();

  const QgsAbstractGeometry *geom = geometry.constGet()->simplifiedTypeRef();
  switch ( QgsWkbTypes::flatType( geom->wkbType() ) )
  {
    case QgsWkbTypes::Unknown:
    case QgsWkbTypes::NoGeometry:
      return QVariantMap();

    case QgsWkbTypes::Point:
      res = pointToJson( qgsgeometry_cast< const QgsPoint * >( geom ) );
      break;

    case QgsWkbTypes::LineString:
      res = lineStringToJson( qgsgeometry_cast< const QgsLineString * >( geom ) );
      break;

    case QgsWkbTypes::CircularString:
    case QgsWkbTypes::CompoundCurve:
      res = curveToJson( qgsgeometry_cast< const QgsCurve * >( geom ) );
      break;

    case QgsWkbTypes::Polygon:
      res = polygonToJson( qgsgeometry_cast< const QgsPolygon * >( geom ) );
      break;

    case QgsWkbTypes::MultiPoint:
      res = multiPointToJson( qgsgeometry_cast< const QgsMultiPoint * >( geom ) );
      break;

    case QgsWkbTypes::MultiLineString:
      res = multiLineStringToJson( qgsgeometry_cast< const QgsMultiLineString * >( geom ) );
      break;

    case QgsWkbTypes::MultiCurve:
      res = multiCurveToJson( qgsgeometry_cast< const QgsMultiCurve * >( geom ) );
      break;

    case QgsWkbTypes::MultiPolygon:
      res = multiPolygonToJson( qgsgeometry_cast< const QgsMultiPolygon * >( geom ) );
      break;

    case QgsWkbTypes::CurvePolygon:
      res = curvePolygonToJson( qgsgeometry_cast< const QgsCurvePolygon * >( geom ) );
      break;

    case QgsWkbTypes::MultiSurface:
      res = multiSurfaceToJson( qgsgeometry_cast< const QgsMultiSurface * >( geom ) );
      break;

    case QgsWkbTypes::GeometryCollection:
      return QVariantMap(); // not supported by REST API

    case QgsWkbTypes::Triangle:
      return QVariantMap(); //not yet supported, but could be

    default:
      return QVariantMap(); //unreachable

  }

  if ( crs.isValid() )
  {
    // add spatialReference information
    res.insert( QStringLiteral( "spatialReference" ), crsToJson( crs ) );
  }

  return res;
}

QVariantMap QgsArcGisRestUtils::pointToJson( const QgsPoint *point )
{
  QVariantMap data;
  if ( point->isEmpty() )
    data[QStringLiteral( "x" )] = QStringLiteral( "NaN" );
  else
  {
    data[QStringLiteral( "x" )] = point->x();
    data[QStringLiteral( "y" )] = point->y();

    if ( point->is3D() )
      data[QStringLiteral( "z" )] = !std::isnan( point->z() ) ? QVariant( point->z() ) :  QVariant( QStringLiteral( "NaN" ) );

    if ( point->isMeasure() )
      data[QStringLiteral( "m" )] = !std::isnan( point->m() ) ? QVariant( point->m() ) :  QVariant( QStringLiteral( "NaN" ) );
  }
  return data;
}

QVariantMap QgsArcGisRestUtils::multiPointToJson( const QgsMultiPoint *multiPoint )
{
  QVariantMap data;
  const bool hasZ = multiPoint->is3D();
  const bool hasM = multiPoint->isMeasure();
  data[QStringLiteral( "hasM" )] = hasM;
  data[QStringLiteral( "hasZ" )] = hasZ;

  QVariantList pointsList;
  const int size = multiPoint->numGeometries();
  pointsList.reserve( size );

  QVariantList pointList;
  for ( int i = 0; i < size; ++i )
  {
    const QgsPoint *point = multiPoint->pointN( i );

    pointList.clear();
    pointList.append( point->x() );
    pointList.append( point->y() );
    if ( hasZ )
      pointList.append( point->z() );
    if ( hasM && !std::isnan( point->m() ) )
      pointList.append( point->m() );

    pointsList.push_back( pointList );
  }

  data[QStringLiteral( "points" )] = pointsList;
  return data;
}

QVariantList QgsArcGisRestUtils::lineStringToJsonPath( const QgsLineString *line )
{
  const bool hasZ = line->is3D();
  const bool hasM = line->isMeasure();

  QVariantList pointsList;
  const int size = line->numPoints();
  pointsList.reserve( size );

  QVariantList pointList;
  const double *xData = line->xData();
  const double *yData = line->yData();
  const double *zData = hasZ ? line->zData() : nullptr;
  const double *mData = hasM ? line->mData() : nullptr;

  for ( int i = 0; i < size; ++i )
  {
    pointList.clear();
    pointList.append( *xData++ );
    pointList.append( *yData++ );

    if ( hasZ )
      pointList.append( *zData++ );

    if ( hasM && !std::isnan( *mData ) )
      pointList.append( *mData );
    if ( hasM )
      mData++;

    pointsList.push_back( pointList );
  }
  return pointsList;
}

QVariantList QgsArcGisRestUtils::curveToJsonCurve( const QgsCurve *curve, bool includeStart )
{
  const bool hasZ = curve->is3D();
  const bool hasM = curve->isMeasure();

  auto pointToList = [hasZ, hasM]( const QgsPoint & point ) -> QVariantList
  {
    QVariantList pointList;

    pointList.append( point.x() );
    pointList.append( point.y() );

    if ( hasZ )
      pointList.append( point.z() );

    if ( hasM && !std::isnan( point.m() ) )
      pointList.append( point.m() );

    return pointList;
  };

  QVariantList res;
  switch ( QgsWkbTypes::flatType( curve->wkbType() ) )
  {
    case QgsWkbTypes::LineString:
    {
      QVariantList part = lineStringToJsonPath( qgsgeometry_cast< const QgsLineString *>( curve ) );
      if ( !part.isEmpty() && !includeStart )
        part.removeAt( 0 );
      res = part;
      break;
    }

    case QgsWkbTypes::CircularString:
    {
      const QgsCircularString *circularString = qgsgeometry_cast<const QgsCircularString * >( curve );
      if ( includeStart && !circularString->isEmpty() )
      {
        res.push_back( pointToList( circularString->startPoint() ) );
      }

      const int size = circularString->numPoints();
      for ( int i = 1; i + 1 < size; i += 2 )
      {
        // end point comes BEFORE interior point!
        QVariantMap curvePart;
        QVariantList curveList;
        curveList.push_back( pointToList( circularString->pointN( i + 1 ) ) );

        curveList.push_back( pointToList( circularString->pointN( i ) ) );

        curvePart.insert( QStringLiteral( "c" ), curveList );
        res.push_back( curvePart );
      }
      break;
    }

    case QgsWkbTypes::CompoundCurve:
    {
      const QgsCompoundCurve *compoundCurve = qgsgeometry_cast<const QgsCompoundCurve * >( curve );

      const int size = compoundCurve->nCurves();
      for ( int i = 0; i < size; ++i )
      {
        const QgsCurve *subCurve = compoundCurve->curveAt( i );
        res.append( curveToJsonCurve( subCurve, i == 0 ) );
      }
      break;
    }

    default:
      break;
  }
  return res;
}

QVariantMap QgsArcGisRestUtils::lineStringToJson( const QgsLineString *line )
{
  QVariantMap data;
  const bool hasZ = line->is3D();
  const bool hasM = line->isMeasure();
  data[QStringLiteral( "hasM" )] = hasM;
  data[QStringLiteral( "hasZ" )] = hasZ;

  const QVariantList pointsList = lineStringToJsonPath( line );

  QVariantList pointsData = QVariantList();
  pointsData.push_back( pointsList );
  data[QStringLiteral( "paths" )] = pointsData;

  return data;
}

QVariantMap QgsArcGisRestUtils::curveToJson( const QgsCurve *curve )
{
  QVariantMap data;
  const bool hasZ = curve->is3D();
  const bool hasM = curve->isMeasure();
  data[QStringLiteral( "hasM" )] = hasM;
  data[QStringLiteral( "hasZ" )] = hasZ;

  const QVariantList curveList = curveToJsonCurve( curve, true );

  QVariantList curveData = QVariantList();
  curveData.push_back( curveList );
  data[QStringLiteral( "curvePaths" )] = curveData;

  return data;
}

QVariantMap QgsArcGisRestUtils::multiLineStringToJson( const QgsMultiLineString *multiLine )
{
  QVariantMap data;
  const bool hasZ = multiLine->is3D();
  const bool hasM = multiLine->isMeasure();
  data[QStringLiteral( "hasM" )] = hasM;
  data[QStringLiteral( "hasZ" )] = hasZ;

  const int size = multiLine->numGeometries();
  QVariantList paths;
  paths.reserve( size );
  for ( int i = 0; i < size; ++i )
  {
    const QgsLineString *line = multiLine->lineStringN( i );
    paths.push_back( lineStringToJsonPath( line ) );
  }

  data[QStringLiteral( "paths" )] = paths;
  return data;
}

QVariantMap QgsArcGisRestUtils::multiCurveToJson( const QgsMultiCurve *multiCurve )
{
  QVariantMap data;
  const bool hasZ = multiCurve->is3D();
  const bool hasM = multiCurve->isMeasure();
  data[QStringLiteral( "hasM" )] = hasM;
  data[QStringLiteral( "hasZ" )] = hasZ;

  const int size = multiCurve->numGeometries();
  QVariantList paths;
  paths.reserve( size );
  for ( int i = 0; i < size; ++i )
  {
    const QgsCurve *curve = multiCurve->curveN( i );
    paths.push_back( curveToJsonCurve( curve, true ) );
  }

  data[QStringLiteral( "curvePaths" )] = paths;
  return data;
}

QVariantList QgsArcGisRestUtils::polygonToJsonRings( const QgsPolygon *polygon )
{
  QVariantList rings;
  const int numInteriorRings = polygon->numInteriorRings();
  rings.reserve( numInteriorRings + 1 );

  if ( const QgsLineString *exterior = qgsgeometry_cast< const QgsLineString * >( polygon->exteriorRing() ) )
  {
    // exterior ring MUST be clockwise
    switch ( exterior->orientation() )
    {
      case Qgis::AngularDirection::Clockwise:
        rings.push_back( lineStringToJsonPath( exterior ) );
        break;

      case Qgis::AngularDirection::CounterClockwise:
      {
        std::unique_ptr< QgsLineString > reversed( exterior->reversed() );
        rings.push_back( lineStringToJsonPath( reversed.get() ) );
        break;
      }
    }
  }

  for ( int i = 0; i < numInteriorRings; ++i )
  {
    const QgsLineString *ring = qgsgeometry_cast< const QgsLineString * >( polygon->interiorRing( i ) );
    // holes MUST be counter-clockwise
    switch ( ring->orientation() )
    {
      case Qgis::AngularDirection::CounterClockwise:
        rings.push_back( lineStringToJsonPath( ring ) );
        break;

      case Qgis::AngularDirection::Clockwise:
      {
        std::unique_ptr< QgsLineString > reversed( ring->reversed() );
        rings.push_back( lineStringToJsonPath( reversed.get() ) );
        break;
      }
    }
  }
  return rings;
}

QVariantList QgsArcGisRestUtils::curvePolygonToJsonRings( const QgsCurvePolygon *polygon )
{
  QVariantList rings;
  const int numInteriorRings = polygon->numInteriorRings();
  rings.reserve( numInteriorRings + 1 );

  if ( const QgsCurve *exterior = qgsgeometry_cast< const QgsCurve * >( polygon->exteriorRing() ) )
  {
    // exterior ring MUST be clockwise
    switch ( exterior->orientation() )
    {
      case Qgis::AngularDirection::Clockwise:
        rings.push_back( curveToJsonCurve( exterior, true ) );
        break;

      case Qgis::AngularDirection::CounterClockwise:
      {
        std::unique_ptr< QgsCurve > reversed( exterior->reversed() );
        rings.push_back( curveToJsonCurve( reversed.get(), true ) );
        break;
      }
    }
  }

  for ( int i = 0; i < numInteriorRings; ++i )
  {
    const QgsCurve *ring = qgsgeometry_cast< const QgsCurve * >( polygon->interiorRing( i ) );
    // holes MUST be counter-clockwise
    switch ( ring->orientation() )
    {
      case Qgis::AngularDirection::CounterClockwise:
        rings.push_back( curveToJsonCurve( ring, true ) );
        break;

      case Qgis::AngularDirection::Clockwise:
      {
        std::unique_ptr< QgsCurve > reversed( ring->reversed() );
        rings.push_back( curveToJsonCurve( reversed.get(), true ) );
        break;
      }
    }
  }
  return rings;
}

QVariantMap QgsArcGisRestUtils::polygonToJson( const QgsPolygon *polygon )
{
  QVariantMap data;
  const bool hasZ = polygon->is3D();
  const bool hasM = polygon->isMeasure();
  data[QStringLiteral( "hasM" )] = hasM;
  data[QStringLiteral( "hasZ" )] = hasZ;
  data[QStringLiteral( "rings" )] = polygonToJsonRings( polygon );
  return data;
}

QVariantMap QgsArcGisRestUtils::curvePolygonToJson( const QgsCurvePolygon *polygon )
{
  QVariantMap data;
  const bool hasZ = polygon->is3D();
  const bool hasM = polygon->isMeasure();
  data[QStringLiteral( "hasM" )] = hasM;
  data[QStringLiteral( "hasZ" )] = hasZ;
  data[QStringLiteral( "curveRings" )] = curvePolygonToJsonRings( polygon );
  return data;
}

QVariantMap QgsArcGisRestUtils::multiPolygonToJson( const QgsMultiPolygon *multiPolygon )
{
  QVariantMap data;
  const bool hasZ = multiPolygon->is3D();
  const bool hasM = multiPolygon->isMeasure();
  data[QStringLiteral( "hasM" )] = hasM;
  data[QStringLiteral( "hasZ" )] = hasZ;

  const int size = multiPolygon->numGeometries();
  QVariantList rings;
  for ( int i = 0; i < size; ++i )
  {
    const QgsPolygon *polygon = multiPolygon->polygonN( i );
    rings.append( polygonToJsonRings( polygon ) );
  }

  data[QStringLiteral( "rings" )] = rings;
  return data;
}

QVariantMap QgsArcGisRestUtils::multiSurfaceToJson( const QgsMultiSurface *multiSurface )
{
  QVariantMap data;
  const bool hasZ = multiSurface->is3D();
  const bool hasM = multiSurface->isMeasure();
  data[QStringLiteral( "hasM" )] = hasM;
  data[QStringLiteral( "hasZ" )] = hasZ;

  const int size = multiSurface->numGeometries();
  QVariantList rings;
  for ( int i = 0; i < size; ++i )
  {
    const QgsCurvePolygon *polygon = qgsgeometry_cast< const QgsCurvePolygon * >( multiSurface->geometryN( i ) );
    if ( !polygon )
      continue;

    rings.append( curvePolygonToJsonRings( polygon ) );
  }

  data[QStringLiteral( "curveRings" )] = rings;
  return data;
}

QVariantMap QgsArcGisRestUtils::crsToJson( const QgsCoordinateReferenceSystem &crs )
{
  QVariantMap res;
  if ( !crs.isValid() )
    return res;

  const QString authid = crs.authid();
  if ( !authid.isEmpty() )
  {
    const thread_local QRegularExpression rxAuthid( QStringLiteral( "(\\w+):(\\d+)" ) );
    const QRegularExpressionMatch match = rxAuthid.match( authid );
    if ( match.hasMatch()
         && (
           ( match.captured( 1 ).compare( QLatin1String( "EPSG" ), Qt::CaseInsensitive ) == 0 )
           || ( match.captured( 1 ).compare( QLatin1String( "ESRI" ), Qt::CaseInsensitive ) == 0 )
         )
       )
    {
      const QString wkid = match.captured( 2 );
      res.insert( QStringLiteral( "wkid" ), wkid );
      return res;
    }
  }

  // docs don't mention the WKT version support, so let's hope for 2.0...
  res.insert( QStringLiteral( "wkt" ), crs.toWkt( QgsCoordinateReferenceSystem::WKT2_2019_SIMPLIFIED ) );

  return res;
}

QVariantMap QgsArcGisRestUtils::featureToJson( const QgsFeature &feature, const QgsArcGisRestContext &context, const QgsCoordinateReferenceSystem &crs, QgsArcGisRestUtils::FeatureToJsonFlags flags )
{
  QVariantMap res;
  if ( ( flags & FeatureToJsonFlag::IncludeGeometry ) && feature.hasGeometry() )
  {
    res.insert( QStringLiteral( "geometry" ), geometryToJson( feature.geometry(), context, crs ) );
  }

  QVariantMap attributes;
  const QgsFields fields = feature.fields();
  for ( const QgsField &field : fields )
  {
    if ( ( flags & FeatureToJsonFlag::IncludeNonObjectIdAttributes ) || field.name() == context.objectIdFieldName() )
      attributes.insert( field.name(), variantToAttributeValue( feature.attribute( field.name() ), field.type(), context ) );
  }
  if ( !attributes.isEmpty() )
  {
    res.insert( QStringLiteral( "attributes" ), attributes );
  }
  return res;
}

QVariant QgsArcGisRestUtils::variantToAttributeValue( const QVariant &variant, QVariant::Type expectedType, const QgsArcGisRestContext &context )
{
  if ( QgsVariantUtils::isNull( variant ) )
    return QVariant();

  switch ( expectedType )
  {
    case QVariant::DateTime:
    case QVariant::Date:
    {
      switch ( variant.type() )
      {
        case QVariant::DateTime:
          return variant.toDateTime().toMSecsSinceEpoch();

        case QVariant::Date:
          // for date values, assume start of day -- the REST api requires datetime values only, not plain dates
          if ( context.timeZone().isValid() )
            return QDateTime( variant.toDate(), QTime( 0, 0, 0 ), context.timeZone() ).toMSecsSinceEpoch();
          else
            return QDateTime( variant.toDate(), QTime( 0, 0, 0 ) ).toMSecsSinceEpoch();

        default:
          return QVariant();
      }
    }

    default:
      return variant;
  }
}

QVariantMap QgsArcGisRestUtils::fieldDefinitionToJson( const QgsField &field )
{
  QVariantMap res;
  res.insert( QStringLiteral( "name" ), field.name() );

  QString fieldType;
  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = QStringLiteral( "esriFieldTypeInteger" );
      break;

    case QVariant::Int:
      fieldType = QStringLiteral( "esriFieldTypeSmallInteger" );
      break;

    case QVariant::Double:
      fieldType = QStringLiteral( "esriFieldTypeDouble" );
      break;

    case QVariant::String:
      fieldType = QStringLiteral( "esriFieldTypeString" );
      break;

    case QVariant::DateTime:
    case QVariant::Date:
      fieldType = QStringLiteral( "esriFieldTypeDate" );
      break;

    case QVariant::ByteArray:
      fieldType = QStringLiteral( "esriFieldTypeBlob" );
      break;

    default:
      // fallback to string
      fieldType = QStringLiteral( "esriFieldTypeString" );
      break;
  }
  res.insert( QStringLiteral( "type" ), fieldType );

  if ( !field.alias().isEmpty() )
    res.insert( QStringLiteral( "alias" ), field.alias() );

  // nullable
  const bool notNullable = field.constraints().constraints() & QgsFieldConstraints::Constraint::ConstraintNotNull;
  res.insert( QStringLiteral( "nullable" ), !notNullable );

  // editable
  res.insert( QStringLiteral( "editable" ), true );

  return res;
}

Qgis::ArcGisRestServiceType QgsArcGisRestUtils::serviceTypeFromString( const QString &type )
{
  if ( type.compare( QLatin1String( "FeatureServer" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::FeatureServer;
  else if ( type.compare( QLatin1String( "MapServer" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::MapServer;
  else if ( type.compare( QLatin1String( "ImageServer" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::ImageServer;
  else if ( type.compare( QLatin1String( "GlobeServer" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::GlobeServer;
  else if ( type.compare( QLatin1String( "GPServer" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::GPServer;
  else if ( type.compare( QLatin1String( "GeocodeServer" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::GeocodeServer;

  return Qgis::ArcGisRestServiceType::Unknown;
}

