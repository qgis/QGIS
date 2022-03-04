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
#include "qgsmultipoint.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"

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
  double x = coordList[0].toDouble( &xok );
  double y = coordList[1].toDouble( &yok );
  if ( !xok || !yok )
    return nullptr;
  double z = nCoords >= 3 ? coordList[2].toDouble() : 0;
  double m = nCoords >= 4 ? coordList[3].toDouble() : 0;
  return std::make_unique< QgsPoint >( pointType, x, y, z, m );
}

std::unique_ptr< QgsCircularString > QgsArcGisRestUtils::convertCircularString( const QVariantMap &curveData, QgsWkbTypes::Type pointType, const QgsPoint &startPoint )
{
  const QVariantList coordsList = curveData[QStringLiteral( "c" )].toList();
  if ( coordsList.isEmpty() )
    return nullptr;
  QVector<QgsPoint> points;
  points.append( startPoint );
  for ( const QVariant &coordData : coordsList )
  {
    std::unique_ptr< QgsPoint > point( convertPoint( coordData.toList(), pointType ) );
    if ( !point )
    {
      return nullptr;
    }
    points.append( *point );
  }
  std::unique_ptr< QgsCircularString > curve = std::make_unique< QgsCircularString> ();
  curve->setPoints( points );
  return curve;
}

std::unique_ptr< QgsCompoundCurve > QgsArcGisRestUtils::convertCompoundCurve( const QVariantList &curvesList, QgsWkbTypes::Type pointType )
{
  // [[6,3],[5,3],{"b":[[3,2],[6,1],[2,4]]},[1,2],{"c": [[3,3],[1,4]]}]
  std::unique_ptr< QgsCompoundCurve > compoundCurve = std::make_unique< QgsCompoundCurve >();
  QgsLineString *lineString = new QgsLineString();
  compoundCurve->addCurve( lineString );
  for ( const QVariant &curveData : curvesList )
  {
    if ( curveData.type() == QVariant::List )
    {
      std::unique_ptr< QgsPoint > point( convertPoint( curveData.toList(), pointType ) );
      if ( !point )
      {
        return nullptr;
      }
      lineString->addVertex( *point );
    }
    else if ( curveData.type() == QVariant::Map )
    {
      // The last point of the linestring is the start point of this circular string
      std::unique_ptr< QgsCircularString > circularString( convertCircularString( curveData.toMap(), pointType, lineString->endPoint() ) );
      if ( !circularString )
      {
        return nullptr;
      }

      // If the previous curve had less than two points, remove it
      if ( compoundCurve->curveAt( compoundCurve->nCurves() - 1 )->nCoordinates() < 2 )
        compoundCurve->removeCurve( compoundCurve->nCurves() - 1 );

      const QgsPoint endPointCircularString = circularString->endPoint();
      compoundCurve->addCurve( circularString.release() );

      // Prepare a new line string
      lineString = new QgsLineString;
      compoundCurve->addCurve( lineString );
      lineString->addVertex( endPointCircularString );
    }
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

  std::sort( curves.begin(), curves.end(), []( const QgsCompoundCurve * a, const QgsCompoundCurve * b )->bool{ double a_area = 0.0; double b_area = 0.0; a->sumUpArea( a_area ); b->sumUpArea( b_area ); return std::abs( a_area ) > std::abs( b_area ); } );
  std::unique_ptr< QgsMultiSurface > result = std::make_unique< QgsMultiSurface >();
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
      settings->placement = QgsPalLayerSettings::OverPoint;
      settings->quadOffset = QgsPalLayerSettings::QuadrantAbove;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementBelowCenter" ) )
    {
      settings->placement = QgsPalLayerSettings::OverPoint;
      settings->quadOffset = QgsPalLayerSettings::QuadrantBelow;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementCenterCenter" ) )
    {
      settings->placement = QgsPalLayerSettings::OverPoint;
      settings->quadOffset = QgsPalLayerSettings::QuadrantOver;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementAboveLeft" ) )
    {
      settings->placement = QgsPalLayerSettings::OverPoint;
      settings->quadOffset = QgsPalLayerSettings::QuadrantAboveLeft;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementBelowLeft" ) )
    {
      settings->placement = QgsPalLayerSettings::OverPoint;
      settings->quadOffset = QgsPalLayerSettings::QuadrantBelowLeft;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementCenterLeft" ) )
    {
      settings->placement = QgsPalLayerSettings::OverPoint;
      settings->quadOffset = QgsPalLayerSettings::QuadrantLeft;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementAboveRight" ) )
    {
      settings->placement = QgsPalLayerSettings::OverPoint;
      settings->quadOffset = QgsPalLayerSettings::QuadrantAboveRight;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementBelowRight" ) )
    {
      settings->placement = QgsPalLayerSettings::OverPoint;
      settings->quadOffset = QgsPalLayerSettings::QuadrantBelowRight;
    }
    else if ( placement == QLatin1String( "esriServerPointLabelPlacementCenterRight" ) )
    {
      settings->placement = QgsPalLayerSettings::OverPoint;
      settings->quadOffset = QgsPalLayerSettings::QuadrantRight;
    }
    else if ( placement == QLatin1String( "esriServerLinePlacementAboveAfter" ) ||
              placement == QLatin1String( "esriServerLinePlacementAboveStart" ) ||
              placement == QLatin1String( "esriServerLinePlacementAboveAlong" ) )
    {
      settings->placement = QgsPalLayerSettings::Line;
      settings->lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::MapOrientation );
    }
    else if ( placement == QLatin1String( "esriServerLinePlacementBelowAfter" ) ||
              placement == QLatin1String( "esriServerLinePlacementBelowStart" ) ||
              placement == QLatin1String( "esriServerLinePlacementBelowAlong" ) )
    {
      settings->placement = QgsPalLayerSettings::Line;
      settings->lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::BelowLine | QgsLabeling::LinePlacementFlag::MapOrientation );
    }
    else if ( placement == QLatin1String( "esriServerLinePlacementCenterAfter" ) ||
              placement == QLatin1String( "esriServerLinePlacementCenterStart" ) ||
              placement == QLatin1String( "esriServerLinePlacementCenterAlong" ) )
    {
      settings->placement = QgsPalLayerSettings::Line;
      settings->lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::OnLine | QgsLabeling::LinePlacementFlag::MapOrientation );
    }
    else if ( placement == QLatin1String( "esriServerPolygonPlacementAlwaysHorizontal" ) )
    {
      settings->placement = QgsPalLayerSettings::Horizontal;
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
  expression = expression.replace( QRegularExpression( "(?=([^\"\\\\]*(\\\\.|\"([^\"\\\\]*\\\\.)*[^\"\\\\]*\"))*[^\"]*$)(\\s|^)CONCAT(\\s|$)" ), QStringLiteral( "\\4||\\5" ) );
  expression = expression.replace( QRegularExpression( "(?=([^\"\\\\]*(\\\\.|\"([^\"\\\\]*\\\\.)*[^\"\\\\]*\"))*[^\"]*$)(\\s|^)NEWLINE(\\s|$)" ), QStringLiteral( "\\4'\\n'\\5" ) );

  // ArcGIS's double quotes are single quotes in QGIS
  expression = expression.replace( QRegularExpression( "\"(.*?(?<!\\\\))\"" ), QStringLiteral( "'\\1'" ) );
  expression = expression.replace( QRegularExpression( "\\\\\"" ), QStringLiteral( "\"" ) );

  // ArcGIS's square brakets are double quotes in QGIS
  expression = expression.replace( QRegularExpression( "\\[([^]]*)\\]" ), QStringLiteral( "\"\\1\"" ) );

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
