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

#include "qgscategorizedsymbolrenderer.h"
#include "qgscircularstring.h"
#include "qgsclassificationcustom.h"
#include "qgsclassificationequalinterval.h"
#include "qgsclassificationfixedinterval.h"
#include "qgsclassificationjenks.h"
#include "qgsclassificationquantile.h"
#include "qgsclassificationstandarddeviation.h"
#include "qgscolorrampimpl.h"
#include "qgscurve.h"
#include "qgsfields.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsgeometryengine.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgslinestring.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultipolygon.h"
#include "qgsmultisurface.h"
#include "qgspallabeling.h"
#include "qgspolygon.h"
#include "qgspropertytransformer.h"
#include "qgsrectangle.h"
#include "qgsrenderer.h"
#include "qgsrulebasedlabeling.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayerlabeling.h"

#include <QRegularExpression>
#include <QUrl>

#include "moc_qgsarcgisrestutils.cpp"

QMetaType::Type QgsArcGisRestUtils::convertFieldType( const QString &esriFieldType )
{
  if ( esriFieldType == "esriFieldTypeInteger"_L1 )
    return QMetaType::Type::LongLong;
  if ( esriFieldType == "esriFieldTypeSmallInteger"_L1 )
    return QMetaType::Type::Int;
  if ( esriFieldType == "esriFieldTypeDouble"_L1 )
    return QMetaType::Type::Double;
  if ( esriFieldType == "esriFieldTypeSingle"_L1 )
    return QMetaType::Type::Double;
  if ( esriFieldType == "esriFieldTypeString"_L1 )
    return QMetaType::Type::QString;
  if ( esriFieldType == "esriFieldTypeDate"_L1 )
    return QMetaType::Type::QDateTime;
  if ( esriFieldType == "esriFieldTypeGeometry"_L1 )
    return QMetaType::Type::UnknownType; // Geometry column should not appear as field
  if ( esriFieldType == "esriFieldTypeOID"_L1 )
    return QMetaType::Type::LongLong;
  if ( esriFieldType == "esriFieldTypeBlob"_L1 )
    return QMetaType::Type::QByteArray;
  if ( esriFieldType == "esriFieldTypeGlobalID"_L1 )
    return QMetaType::Type::QString;
  if ( esriFieldType == "esriFieldTypeRaster"_L1 )
    return QMetaType::Type::QByteArray;
  if ( esriFieldType == "esriFieldTypeGUID"_L1 )
    return QMetaType::Type::QString;
  if ( esriFieldType == "esriFieldTypeXML"_L1 )
    return QMetaType::Type::QString;
  return QMetaType::Type::UnknownType;
}

Qgis::WkbType QgsArcGisRestUtils::convertGeometryType( const QString &esriGeometryType )
{
  // http://resources.arcgis.com/en/help/arcobjects-cpp/componenthelp/index.html#//000w0000001p000000
  if ( esriGeometryType == "esriGeometryNull"_L1 )
    return Qgis::WkbType::Unknown;
  else if ( esriGeometryType == "esriGeometryPoint"_L1 )
    return Qgis::WkbType::Point;
  else if ( esriGeometryType == "esriGeometryMultipoint"_L1 )
    return Qgis::WkbType::MultiPoint;
  else if ( esriGeometryType == "esriGeometryPolyline"_L1 )
    return Qgis::WkbType::MultiCurve;
  else if ( esriGeometryType == "esriGeometryPolygon"_L1 )
    return Qgis::WkbType::MultiPolygon;
  else if ( esriGeometryType == "esriGeometryEnvelope"_L1 )
    return Qgis::WkbType::Polygon;
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
  return Qgis::WkbType::Unknown;
}

std::unique_ptr< QgsPoint > QgsArcGisRestUtils::convertPoint( const QVariantList &coordList, Qgis::WkbType pointType )
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

std::unique_ptr< QgsCircularString > QgsArcGisRestUtils::convertCircularString( const QVariantMap &curveData, Qgis::WkbType pointType, const QgsPoint &startPoint )
{
  const QVariantList coordsList = curveData[u"c"_s].toList();
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
  auto curve = std::make_unique< QgsCircularString> ();
  curve->setPoints( points );
  return curve;
}

std::unique_ptr< QgsCompoundCurve > QgsArcGisRestUtils::convertCompoundCurve( const QVariantList &curvesList, Qgis::WkbType pointType )
{
  // [[6,3],[5,3],{"b":[[3,2],[6,1],[2,4]]},[1,2],{"c": [[3,3],[1,4]]}]
  auto compoundCurve = std::make_unique< QgsCompoundCurve >();

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
    if ( curveData.userType() == QMetaType::Type::QVariantList )
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
    else if ( curveData.userType() == QMetaType::Type::QVariantMap )
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

std::unique_ptr< QgsPoint > QgsArcGisRestUtils::convertGeometryPoint( const QVariantMap &geometryData, Qgis::WkbType pointType )
{
  // {"x" : <x>, "y" : <y>, "z" : <z>, "m" : <m>}
  bool xok = false, yok = false;
  double x = geometryData[u"x"_s].toDouble( &xok );
  double y = geometryData[u"y"_s].toDouble( &yok );
  if ( !xok || !yok )
    return nullptr;
  double z = geometryData[u"z"_s].toDouble();
  double m = geometryData[u"m"_s].toDouble();
  return std::make_unique< QgsPoint >( pointType, x, y, z, m );
}

std::unique_ptr< QgsMultiPoint > QgsArcGisRestUtils::convertMultiPoint( const QVariantMap &geometryData, Qgis::WkbType pointType )
{
  // {"points" : [[ <x1>, <y1>, <z1>, <m1> ] , [ <x2>, <y2>, <z2>, <m2> ], ... ]}
  const QVariantList coordsList = geometryData[u"points"_s].toList();

  auto multiPoint = std::make_unique< QgsMultiPoint >();
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

std::unique_ptr< QgsMultiCurve > QgsArcGisRestUtils::convertGeometryPolyline( const QVariantMap &geometryData, Qgis::WkbType pointType )
{
  // {"curvePaths": [[[0,0], {"c": [[3,3],[1,4]]} ]]}
  QVariantList pathsList;
  if ( geometryData[u"paths"_s].isValid() )
    pathsList = geometryData[u"paths"_s].toList();
  else if ( geometryData[u"curvePaths"_s].isValid() )
    pathsList = geometryData[u"curvePaths"_s].toList();
  if ( pathsList.isEmpty() )
    return nullptr;
  auto multiCurve = std::make_unique< QgsMultiCurve >();
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

std::unique_ptr< QgsMultiSurface > QgsArcGisRestUtils::convertGeometryPolygon( const QVariantMap &geometryData, Qgis::WkbType pointType )
{
  // {"curveRings": [[[0,0], {"c": [[3,3],[1,4]]} ]]}
  QVariantList ringsList;
  if ( geometryData[u"rings"_s].isValid() )
    ringsList = geometryData[u"rings"_s].toList();
  else if ( geometryData[u"ringPaths"_s].isValid() )
    ringsList = geometryData[u"ringPaths"_s].toList();
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

  auto result = std::make_unique< QgsMultiSurface >();
  if ( curves.count() == 1 )
  {
    // shortcut for exterior ring only
    auto newPolygon = std::make_unique< QgsCurvePolygon >();
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
  double xmin = geometryData[u"xmin"_s].toDouble( &xminOk );
  double ymin = geometryData[u"ymin"_s].toDouble( &yminOk );
  double xmax = geometryData[u"xmax"_s].toDouble( &xmaxOk );
  double ymax = geometryData[u"ymax"_s].toDouble( &ymaxOk );
  if ( !xminOk || !yminOk || !xmaxOk || !ymaxOk )
    return nullptr;
  auto ext = std::make_unique< QgsLineString> ();
  ext->addVertex( QgsPoint( xmin, ymin ) );
  ext->addVertex( QgsPoint( xmax, ymin ) );
  ext->addVertex( QgsPoint( xmax, ymax ) );
  ext->addVertex( QgsPoint( xmin, ymax ) );
  ext->addVertex( QgsPoint( xmin, ymin ) );
  auto poly = std::make_unique< QgsPolygon >();
  poly->setExteriorRing( ext.release() );
  return poly;
}

QgsAbstractGeometry *QgsArcGisRestUtils::convertGeometry( const QVariantMap &geometryData, const QString &esriGeometryType, bool readM, bool readZ, QgsCoordinateReferenceSystem *crs )
{
  Qgis::WkbType pointType = QgsWkbTypes::zmType( Qgis::WkbType::Point, readZ, readM );
  if ( crs )
  {
    *crs = convertSpatialReference( geometryData[u"spatialReference"_s].toMap() );
  }

  // http://resources.arcgis.com/en/help/arcgis-rest-api/index.html#/Geometry_Objects/02r3000000n1000000/
  if ( esriGeometryType == "esriGeometryNull"_L1 )
    return nullptr;
  else if ( esriGeometryType == "esriGeometryPoint"_L1 )
    return convertGeometryPoint( geometryData, pointType ).release();
  else if ( esriGeometryType == "esriGeometryMultipoint"_L1 )
    return convertMultiPoint( geometryData, pointType ).release();
  else if ( esriGeometryType == "esriGeometryPolyline"_L1 )
    return convertGeometryPolyline( geometryData, pointType ).release();
  else if ( esriGeometryType == "esriGeometryPolygon"_L1 )
    return convertGeometryPolygon( geometryData, pointType ).release();
  else if ( esriGeometryType == "esriGeometryEnvelope"_L1 )
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

  QString spatialReference = spatialReferenceMap[u"latestWkid"_s].toString();
  if ( spatialReference.isEmpty() )
    spatialReference = spatialReferenceMap[u"wkid"_s].toString();

  // prefer using authority/id wherever we can
  if ( !spatialReference.isEmpty() )
  {
    crs.createFromString( u"EPSG:%1"_s.arg( spatialReference ) );
    if ( !crs.isValid() )
    {
      // Try as an ESRI auth
      crs.createFromString( u"ESRI:%1"_s.arg( spatialReference ) );
    }
  }
  else if ( !spatialReferenceMap[u"wkt"_s].toString().isEmpty() )
  {
    // otherwise fallback to WKT
    crs.createFromWkt( spatialReferenceMap[u"wkt"_s].toString() );
  }

  if ( !crs.isValid() )
  {
    // If no spatial reference, just use WGS84
    // TODO -- this needs further investigation! Most ESRI server services default to 3857, so that would likely be
    // a safer fallback to use...
    crs.createFromString( u"EPSG:4326"_s );
  }
  return crs;
}

QgsSymbol *QgsArcGisRestUtils::convertSymbol( const QVariantMap &symbolData )
{
  const QString type = symbolData.value( u"type"_s ).toString();
  if ( type == "esriSMS"_L1 )
  {
    // marker symbol
    return parseEsriMarkerSymbolJson( symbolData ).release();
  }
  else if ( type == "esriSLS"_L1 )
  {
    // line symbol
    return parseEsriLineSymbolJson( symbolData ).release();
  }
  else if ( type == "esriSFS"_L1 )
  {
    // fill symbol
    return parseEsriFillSymbolJson( symbolData ).release();
  }
  else if ( type == "esriPFS"_L1 )
  {
    return parseEsriPictureFillSymbolJson( symbolData ).release();
  }
  else if ( type == "esriPMS"_L1 )
  {
    // picture marker
    return parseEsriPictureMarkerSymbolJson( symbolData ).release();
  }
  else if ( type == "esriTS"_L1 )
  {
    return parseEsriTextMarkerSymbolJson( symbolData ).release();
  }
  return nullptr;
}

std::unique_ptr<QgsLineSymbol> QgsArcGisRestUtils::parseEsriLineSymbolJson( const QVariantMap &symbolData )
{
  QColor lineColor = convertColor( symbolData.value( u"color"_s ) );
  if ( !lineColor.isValid() )
    return nullptr;

  bool ok = false;
  double widthInPoints = symbolData.value( u"width"_s ).toDouble( &ok );
  if ( !ok )
    return nullptr;

  QgsSymbolLayerList layers;
  Qt::PenStyle penStyle = convertLineStyle( symbolData.value( u"style"_s ).toString() );
  auto lineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( lineColor, widthInPoints, penStyle );
  lineLayer->setWidthUnit( Qgis::RenderUnit::Points );
  layers.append( lineLayer.release() );

  auto symbol = std::make_unique< QgsLineSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsFillSymbol> QgsArcGisRestUtils::parseEsriFillSymbolJson( const QVariantMap &symbolData )
{
  QColor fillColor = convertColor( symbolData.value( u"color"_s ) );
  Qt::BrushStyle brushStyle = convertFillStyle( symbolData.value( u"style"_s ).toString() );

  const QVariantMap outlineData = symbolData.value( u"outline"_s ).toMap();
  QColor lineColor = convertColor( outlineData.value( u"color"_s ) );
  Qt::PenStyle penStyle = convertLineStyle( outlineData.value( u"style"_s ).toString() );
  bool ok = false;
  double penWidthInPoints = outlineData.value( u"width"_s ).toDouble( &ok );

  QgsSymbolLayerList layers;
  auto fillLayer = std::make_unique< QgsSimpleFillSymbolLayer >( fillColor, brushStyle, lineColor, penStyle, penWidthInPoints );
  fillLayer->setStrokeWidthUnit( Qgis::RenderUnit::Points );
  layers.append( fillLayer.release() );

  auto symbol = std::make_unique< QgsFillSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsFillSymbol> QgsArcGisRestUtils::parseEsriPictureFillSymbolJson( const QVariantMap &symbolData )
{
  bool ok = false;

  double widthInPixels = symbolData.value( u"width"_s ).toInt( &ok );
  if ( !ok )
    return nullptr;

  const double xScale = symbolData.value( u"xscale"_s ).toDouble( &ok );
  if ( !qgsDoubleNear( xScale, 0.0 ) )
    widthInPixels *= xScale;

  const double angleCCW = symbolData.value( u"angle"_s ).toDouble( &ok );
  double angleCW = 0;
  if ( ok )
    angleCW = -angleCCW;

  const double xOffset = symbolData.value( u"xoffset"_s ).toDouble();
  const double yOffset = symbolData.value( u"yoffset"_s ).toDouble();

  QString symbolPath( symbolData.value( u"imageData"_s ).toString() );
  symbolPath.prepend( "base64:"_L1 );

  QgsSymbolLayerList layers;
  auto fillLayer = std::make_unique< QgsRasterFillSymbolLayer >( symbolPath );
  fillLayer->setWidth( widthInPixels );
  fillLayer->setAngle( angleCW );
  fillLayer->setSizeUnit( Qgis::RenderUnit::Points );
  fillLayer->setOffset( QPointF( xOffset, yOffset ) );
  fillLayer->setOffsetUnit( Qgis::RenderUnit::Points );
  layers.append( fillLayer.release() );

  const QVariantMap outlineData = symbolData.value( u"outline"_s ).toMap();
  QColor lineColor = convertColor( outlineData.value( u"color"_s ) );
  Qt::PenStyle penStyle = convertLineStyle( outlineData.value( u"style"_s ).toString() );
  double penWidthInPoints = outlineData.value( u"width"_s ).toDouble( &ok );

  auto lineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( lineColor, penWidthInPoints, penStyle );
  lineLayer->setWidthUnit( Qgis::RenderUnit::Points );
  layers.append( lineLayer.release() );

  auto symbol = std::make_unique< QgsFillSymbol >( layers );
  return symbol;
}

Qgis::MarkerShape QgsArcGisRestUtils::parseEsriMarkerShape( const QString &style )
{
  if ( style == "esriSMSCircle"_L1 )
    return Qgis::MarkerShape::Circle;
  else if ( style == "esriSMSCross"_L1 )
    return Qgis::MarkerShape::Cross;
  else if ( style == "esriSMSDiamond"_L1 )
    return Qgis::MarkerShape::Diamond;
  else if ( style == "esriSMSSquare"_L1 )
    return Qgis::MarkerShape::Square;
  else if ( style == "esriSMSX"_L1 )
    return Qgis::MarkerShape::Cross2;
  else if ( style == "esriSMSTriangle"_L1 )
    return Qgis::MarkerShape::Triangle;
  else
    return Qgis::MarkerShape::Circle;
}

std::unique_ptr<QgsMarkerSymbol> QgsArcGisRestUtils::parseEsriMarkerSymbolJson( const QVariantMap &symbolData )
{
  QColor fillColor = convertColor( symbolData.value( u"color"_s ) );
  bool ok = false;
  const double sizeInPoints = symbolData.value( u"size"_s ).toDouble( &ok );
  if ( !ok )
    return nullptr;
  const double angleCCW = symbolData.value( u"angle"_s ).toDouble( &ok );
  double angleCW = 0;
  if ( ok )
    angleCW = -angleCCW;

  Qgis::MarkerShape shape = parseEsriMarkerShape( symbolData.value( u"style"_s ).toString() );

  const double xOffset = symbolData.value( u"xoffset"_s ).toDouble();
  const double yOffset = symbolData.value( u"yoffset"_s ).toDouble();

  const QVariantMap outlineData = symbolData.value( u"outline"_s ).toMap();
  QColor lineColor = convertColor( outlineData.value( u"color"_s ) );
  Qt::PenStyle penStyle = convertLineStyle( outlineData.value( u"style"_s ).toString() );
  double penWidthInPoints = outlineData.value( u"width"_s ).toDouble( &ok );

  QgsSymbolLayerList layers;
  auto markerLayer = std::make_unique< QgsSimpleMarkerSymbolLayer >( shape, sizeInPoints, angleCW, Qgis::ScaleMethod::ScaleArea, fillColor, lineColor );
  markerLayer->setSizeUnit( Qgis::RenderUnit::Points );
  markerLayer->setStrokeWidthUnit( Qgis::RenderUnit::Points );
  markerLayer->setStrokeStyle( penStyle );
  markerLayer->setStrokeWidth( penWidthInPoints );
  markerLayer->setOffset( QPointF( xOffset, yOffset ) );
  markerLayer->setOffsetUnit( Qgis::RenderUnit::Points );
  layers.append( markerLayer.release() );

  auto symbol = std::make_unique< QgsMarkerSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsMarkerSymbol> QgsArcGisRestUtils::parseEsriPictureMarkerSymbolJson( const QVariantMap &symbolData )
{
  bool ok = false;
  const double widthInPixels = symbolData.value( u"width"_s ).toInt( &ok );
  if ( !ok )
    return nullptr;
  const double heightInPixels = symbolData.value( u"height"_s ).toInt( &ok );
  if ( !ok )
    return nullptr;

  const double angleCCW = symbolData.value( u"angle"_s ).toDouble( &ok );
  double angleCW = 0;
  if ( ok )
    angleCW = -angleCCW;

  const double xOffset = symbolData.value( u"xoffset"_s ).toDouble();
  const double yOffset = symbolData.value( u"yoffset"_s ).toDouble();

  //const QString contentType = symbolData.value( u"contentType"_s ).toString();

  QString symbolPath( symbolData.value( u"imageData"_s ).toString() );
  symbolPath.prepend( "base64:"_L1 );

  QgsSymbolLayerList layers;
  auto markerLayer = std::make_unique< QgsRasterMarkerSymbolLayer >( symbolPath, widthInPixels, angleCW, Qgis::ScaleMethod::ScaleArea );
  markerLayer->setSizeUnit( Qgis::RenderUnit::Points );

  // only change the default aspect ratio if the server height setting requires this
  if ( !qgsDoubleNear( static_cast< double >( heightInPixels ) / widthInPixels, markerLayer->defaultAspectRatio() ) )
    markerLayer->setFixedAspectRatio( static_cast< double >( heightInPixels ) / widthInPixels );

  markerLayer->setOffset( QPointF( xOffset, yOffset ) );
  markerLayer->setOffsetUnit( Qgis::RenderUnit::Points );
  layers.append( markerLayer.release() );

  auto symbol = std::make_unique< QgsMarkerSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsMarkerSymbol> QgsArcGisRestUtils::parseEsriTextMarkerSymbolJson( const QVariantMap &symbolData )
{
  QgsSymbolLayerList layers;

  const QString fontFamily = symbolData.value( u"font"_s ).toMap().value( u"family"_s ).toString();

  const QString chr = symbolData.value( u"text"_s ).toString();

  const double pointSize = symbolData.value( u"font"_s ).toMap().value( u"size"_s ).toDouble();

  const QColor color = convertColor( symbolData.value( u"color"_s ) );

  const double esriAngle = symbolData.value( u"angle"_s ).toDouble();

  const double angle = 90.0 - esriAngle;

  auto markerLayer = std::make_unique< QgsFontMarkerSymbolLayer >( fontFamily, chr, pointSize, color, angle );

  QColor strokeColor = convertColor( symbolData.value( u"borderLineColor"_s ) );
  markerLayer->setStrokeColor( strokeColor );

  double borderLineSize = symbolData.value( u"borderLineSize"_s ).toDouble();
  markerLayer->setStrokeWidth( borderLineSize );

  const QString fontStyle = symbolData.value( u"font"_s ).toMap().value( u"style"_s ).toString();
  markerLayer->setFontStyle( fontStyle );

  double xOffset = symbolData.value( u"xoffset"_s ).toDouble();
  double yOffset = symbolData.value( u"yoffset"_s ).toDouble();

  markerLayer->setOffset( QPointF( xOffset, yOffset ) );
  markerLayer->setOffsetUnit( Qgis::RenderUnit::Points );

  markerLayer->setSizeUnit( Qgis::RenderUnit::Points );
  markerLayer->setStrokeWidthUnit( Qgis::RenderUnit::Points );

  Qgis::HorizontalAnchorPoint hAlign = Qgis::HorizontalAnchorPoint::Center;
  Qgis::VerticalAnchorPoint vAlign = Qgis::VerticalAnchorPoint::Center;

  QString horizontalAnchorPoint = symbolData.value( u"horizontalAlignment"_s ).toString();
  QString verticalAnchorPoint = symbolData.value( u"verticalAlignment"_s ).toString();

  if ( horizontalAnchorPoint == QString( "center" ) )
  {
    hAlign = Qgis::HorizontalAnchorPoint::Center;
  }
  else if ( horizontalAnchorPoint == QString( "left" ) )
  {
    hAlign = Qgis::HorizontalAnchorPoint::Left;
  }
  else if ( horizontalAnchorPoint == QString( "right" ) )
  {
    hAlign = Qgis::HorizontalAnchorPoint::Right;
  }

  if ( verticalAnchorPoint == QString( "center" ) )
  {
    vAlign = Qgis::VerticalAnchorPoint::Center;
  }
  else if ( verticalAnchorPoint == QString( "top" ) )
  {
    vAlign = Qgis::VerticalAnchorPoint::Top;
  }
  else if ( verticalAnchorPoint == QString( "bottom" ) )
  {
    vAlign = Qgis::VerticalAnchorPoint::Bottom;
  }

  markerLayer->setHorizontalAnchorPoint( hAlign );
  markerLayer->setVerticalAnchorPoint( vAlign );

  layers.append( markerLayer.release() );

  auto symbol = std::make_unique< QgsMarkerSymbol >( layers );
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

    const QString placement = labeling.value( u"labelPlacement"_s ).toString();
    if ( placement == "esriServerPointLabelPlacementAboveCenter"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Above );
    }
    else if ( placement == "esriServerPointLabelPlacementBelowCenter"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Below );
    }
    else if ( placement == "esriServerPointLabelPlacementCenterCenter"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Over );
    }
    else if ( placement == "esriServerPointLabelPlacementAboveLeft"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::AboveLeft );
    }
    else if ( placement == "esriServerPointLabelPlacementBelowLeft"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::BelowLeft );
    }
    else if ( placement == "esriServerPointLabelPlacementCenterLeft"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Left );
    }
    else if ( placement == "esriServerPointLabelPlacementAboveRight"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::AboveRight );
    }
    else if ( placement == "esriServerPointLabelPlacementBelowRight"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::BelowRight );
    }
    else if ( placement == "esriServerPointLabelPlacementCenterRight"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::OverPoint;
      settings->pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Right );
    }
    else if ( placement == "esriServerLinePlacementAboveAfter"_L1 ||
              placement == "esriServerLinePlacementAboveStart"_L1 ||
              placement == "esriServerLinePlacementAboveAlong"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::Line;
      settings->lineSettings().setPlacementFlags( Qgis::LabelLinePlacementFlag::AboveLine | Qgis::LabelLinePlacementFlag::MapOrientation );
    }
    else if ( placement == "esriServerLinePlacementBelowAfter"_L1 ||
              placement == "esriServerLinePlacementBelowStart"_L1 ||
              placement == "esriServerLinePlacementBelowAlong"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::Line;
      settings->lineSettings().setPlacementFlags( Qgis::LabelLinePlacementFlag::BelowLine | Qgis::LabelLinePlacementFlag::MapOrientation );
    }
    else if ( placement == "esriServerLinePlacementCenterAfter"_L1 ||
              placement == "esriServerLinePlacementCenterStart"_L1 ||
              placement == "esriServerLinePlacementCenterAlong"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::Line;
      settings->lineSettings().setPlacementFlags( Qgis::LabelLinePlacementFlag::OnLine | Qgis::LabelLinePlacementFlag::MapOrientation );
    }
    else if ( placement == "esriServerPolygonPlacementAlwaysHorizontal"_L1 )
    {
      settings->placement = Qgis::LabelPlacement::Horizontal;
    }

    const double minScale = labeling.value( u"minScale"_s ).toDouble();
    const double maxScale = labeling.value( u"maxScale"_s ).toDouble();

    QVariantMap symbol = labeling.value( u"symbol"_s ).toMap();
    format.setColor( convertColor( symbol.value( u"color"_s ) ) );
    const double haloSize = symbol.value( u"haloSize"_s ).toDouble();
    if ( !qgsDoubleNear( haloSize, 0.0 ) )
    {
      QgsTextBufferSettings buffer;
      buffer.setEnabled( true );
      buffer.setSize( haloSize );
      buffer.setSizeUnit( Qgis::RenderUnit::Points );
      buffer.setColor( convertColor( symbol.value( u"haloColor"_s ) ) );
      format.setBuffer( buffer );
    }

    const QString fontFamily = symbol.value( u"font"_s ).toMap().value( u"family"_s ).toString();
    const QString fontStyle = symbol.value( u"font"_s ).toMap().value( u"style"_s ).toString();
    const QString fontWeight = symbol.value( u"font"_s ).toMap().value( u"weight"_s ).toString();
    const int fontSize = symbol.value( u"font"_s ).toMap().value( u"size"_s ).toInt();
    QFont font( fontFamily, fontSize );
    font.setStyleName( fontStyle );
    font.setWeight( fontWeight == "bold"_L1 ? QFont::Bold : QFont::Normal );

    format.setFont( font );
    format.setSize( fontSize );
    format.setSizeUnit( Qgis::RenderUnit::Points );

    settings->setFormat( format );

    QString where = labeling.value( u"where"_s ).toString();
    QgsExpression exp( where );
    // If the where clause isn't parsed as valid, don't use its
    if ( !exp.isValid() )
      where.clear();

    settings->fieldName = convertLabelingExpression( labeling.value( u"labelExpression"_s ).toString() );
    settings->isExpression = true;

    QgsRuleBasedLabeling::Rule *child = new QgsRuleBasedLabeling::Rule( settings, maxScale, minScale, where, QObject::tr( "ASF label %1" ).arg( i++ ), false );
    child->setActive( true );
    root->appendChild( child );
  }

  return new QgsRuleBasedLabeling( root );
}

QgsFeatureRenderer *QgsArcGisRestUtils::convertRenderer( const QVariantMap &rendererData )
{
  const QString type = rendererData.value( u"type"_s ).toString();
  if ( type == "simple"_L1 )
  {
    const QVariantMap symbolProps = rendererData.value( u"symbol"_s ).toMap();
    std::unique_ptr< QgsSymbol > symbol( convertSymbol( symbolProps ) );
    if ( symbol )
      return new QgsSingleSymbolRenderer( symbol.release() );
    else
      return nullptr;
  }
  else if ( type == "uniqueValue"_L1 )
  {
    const QString field1 = rendererData.value( u"field1"_s ).toString();
    const QString field2 = rendererData.value( u"field2"_s ).toString();
    const QString field3 = rendererData.value( u"field3"_s ).toString();
    QString attribute;
    if ( !field2.isEmpty() || !field3.isEmpty() )
    {
      const QString delimiter = rendererData.value( u"fieldDelimiter"_s ).toString();
      if ( !field3.isEmpty() )
      {
        attribute = u"concat(\"%1\",'%2',\"%3\",'%4',\"%5\")"_s.arg( field1, delimiter, field2, delimiter, field3 );
      }
      else
      {
        attribute = u"concat(\"%1\",'%2',\"%3\")"_s.arg( field1, delimiter, field2 );
      }
    }
    else
    {
      attribute = field1;
    }

    const QVariantList categories = rendererData.value( u"uniqueValueInfos"_s ).toList();
    QgsCategoryList categoryList;
    for ( const QVariant &category : categories )
    {
      const QVariantMap categoryData = category.toMap();
      const QString value = categoryData.value( u"value"_s ).toString();
      const QString label = categoryData.value( u"label"_s ).toString();
      std::unique_ptr< QgsSymbol > symbol( QgsArcGisRestUtils::convertSymbol( categoryData.value( u"symbol"_s ).toMap() ) );
      if ( symbol )
      {
        categoryList.append( QgsRendererCategory( value, symbol.release(), label ) );
      }
    }

    std::unique_ptr< QgsSymbol > defaultSymbol( convertSymbol( rendererData.value( u"defaultSymbol"_s ).toMap() ) );
    if ( defaultSymbol )
    {
      categoryList.append( QgsRendererCategory( QVariant(), defaultSymbol.release(), rendererData.value( u"defaultLabel"_s ).toString() ) );
    }

    if ( categoryList.empty() )
      return nullptr;

    return new QgsCategorizedSymbolRenderer( attribute, categoryList );
  }
  else if ( type == "classBreaks"_L1 )
  {
    const QString attrName = rendererData.value( u"field"_s ).toString();

    const QVariantList classBreakInfos = rendererData.value( u"classBreakInfos"_s ).toList();
    const QVariantMap authoringInfo = rendererData.value( u"authoringInfo"_s ).toMap();
    QVariantMap symbolData;

    QString esriMode = authoringInfo.value( u"classificationMethod"_s ).toString();
    if ( esriMode.isEmpty() )
    {
      esriMode = rendererData.value( u"classificationMethod"_s ).toString();
    }

    if ( !classBreakInfos.isEmpty() )
    {
      symbolData = classBreakInfos.at( 0 ).toMap().value( u"symbol"_s ).toMap();
    }
    std::unique_ptr< QgsSymbol > symbol( QgsArcGisRestUtils::convertSymbol( symbolData ) );
    if ( !symbol )
      return nullptr;

    const double transparency = rendererData.value( u"transparency"_s ).toDouble();
    const double opacity = ( 100.0 - transparency ) / 100.0;
    symbol->setOpacity( opacity );

    const QVariantList visualVariablesData = rendererData.value( u"visualVariables"_s ).toList();

    for ( const QVariant &visualVariable : visualVariablesData )
    {
      const QVariantMap visualVariableData = visualVariable.toMap();
      const QString variableType = visualVariableData.value( u"type"_s ).toString();
      if ( variableType == "sizeInfo"_L1 )
      {
        continue;
      }
      else if ( variableType == "colorInfo"_L1 )
      {
        const QVariantList stops = visualVariableData.value( u"stops"_s ).toList();
        if ( stops.size() < 2 )
          continue;

        // layer has continuous coloring, so convert to a symbol using color ramp assistant
        bool ok = false;
        const double minValue = stops.front().toMap().value( u"value"_s ).toDouble( &ok );
        if ( !ok )
          continue;
        const QColor minColor = convertColor( stops.front().toMap().value( u"color"_s ) );

        const double maxValue = stops.back().toMap().value( u"value"_s ).toDouble( &ok );
        if ( !ok )
          continue;
        const QColor maxColor = convertColor( stops.back().toMap().value( u"color"_s ) );

        QgsGradientStopsList gradientStops;
        for ( int i = 1; i < stops.size() - 1; ++i )
        {
          const QVariantMap stopData = stops.at( i ).toMap();
          const double breakpoint = stopData.value( u"value"_s ).toDouble();
          const double scaledBreakpoint = ( breakpoint - minValue ) / ( maxValue - minValue );
          const QColor fillColor = convertColor( stopData.value( u"color"_s ) );

          gradientStops.append( QgsGradientStop( scaledBreakpoint, fillColor ) );
        }

        auto colorRamp = std::make_unique< QgsGradientColorRamp >(
                           minColor, maxColor, false, gradientStops
                         );

        QgsProperty colorProperty = QgsProperty::fromField( attrName );
        colorProperty.setTransformer(
          new QgsColorRampTransformer( minValue, maxValue, colorRamp.release() )
        );
        for ( int layer = 0; layer < symbol->symbolLayerCount(); ++layer )
        {
          symbol->symbolLayer( layer )->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, colorProperty );
        }

        auto singleSymbolRenderer = std::make_unique< QgsSingleSymbolRenderer >( symbol.release() );

        return singleSymbolRenderer.release();
      }
      else
      {
        QgsDebugError( u"ESRI visualVariable type %1 is not currently supported"_s.arg( variableType ) );
      }
    }

    double lastValue = rendererData.value( u"minValue"_s ).toDouble();

    auto graduatedRenderer = std::make_unique< QgsGraduatedSymbolRenderer >( attrName );

    graduatedRenderer->setSourceSymbol( symbol.release() );

    if ( esriMode == "esriClassifyDefinedInterval"_L1 )
    {
      QgsClassificationFixedInterval *method = new QgsClassificationFixedInterval();
      graduatedRenderer->setClassificationMethod( method );
    }
    else if ( esriMode == "esriClassifyEqualInterval"_L1 )
    {
      QgsClassificationEqualInterval *method = new QgsClassificationEqualInterval();
      graduatedRenderer->setClassificationMethod( method );
    }
    else if ( esriMode == "esriClassifyGeometricalInterval"_L1 )
    {
      QgsClassificationCustom *method = new QgsClassificationCustom();
      graduatedRenderer->setClassificationMethod( method );
    }
    else if ( esriMode == "esriClassifyManual"_L1 )
    {
      QgsClassificationCustom *method = new QgsClassificationCustom();
      graduatedRenderer->setClassificationMethod( method );
    }
    else if ( esriMode == "esriClassifyNaturalBreaks"_L1 )
    {
      QgsClassificationJenks *method = new QgsClassificationJenks();
      graduatedRenderer->setClassificationMethod( method );
    }
    else if ( esriMode == "esriClassifyQuantile"_L1 )
    {
      QgsClassificationQuantile *method = new QgsClassificationQuantile();
      graduatedRenderer->setClassificationMethod( method );
    }
    else if ( esriMode == "esriClassifyStandardDeviation"_L1 )
    {
      QgsClassificationStandardDeviation *method = new QgsClassificationStandardDeviation();
      graduatedRenderer->setClassificationMethod( method );
    }
    else if ( !esriMode.isEmpty() )
    {
      QgsDebugError( u"ESRI classification mode %1 is not currently supported"_s.arg( esriMode ) );
    }

    for ( const QVariant &classBreakInfo : classBreakInfos )
    {
      const QVariantMap symbolData = classBreakInfo.toMap().value( u"symbol"_s ).toMap();
      std::unique_ptr< QgsSymbol > symbol( QgsArcGisRestUtils::convertSymbol( symbolData ) );
      double classMaxValue = classBreakInfo.toMap().value( u"classMaxValue"_s ).toDouble();
      const QString label = classBreakInfo.toMap().value( u"label"_s ).toString();

      QgsRendererRange range;

      range.setLowerValue( lastValue );
      range.setUpperValue( classMaxValue );
      range.setLabel( label );
      range.setSymbol( symbol.release() );

      lastValue = classMaxValue;
      graduatedRenderer->addClass( range );
    }

    return graduatedRenderer.release();
  }
  else if ( type == "heatmap"_L1 )
  {
    // currently unsupported
    return nullptr;
  }
  else if ( type == "vectorField"_L1 )
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
  const thread_local QRegularExpression rx1 = QRegularExpression( u"(?=([^\"\\\\]*(\\\\.|\"([^\"\\\\]*\\\\.)*[^\"\\\\]*\"))*[^\"]*$)(\\s|^)CONCAT(\\s|$)"_s );
  expression = expression.replace( rx1, u"\\4||\\5"_s );

  const thread_local QRegularExpression rx2 = QRegularExpression( u"(?=([^\"\\\\]*(\\\\.|\"([^\"\\\\]*\\\\.)*[^\"\\\\]*\"))*[^\"]*$)(\\s|^)NEWLINE(\\s|$)"_s );
  expression = expression.replace( rx2, u"\\4'\\n'\\5"_s );

  // ArcGIS's double quotes are single quotes in QGIS
  const thread_local QRegularExpression rx3 = QRegularExpression( u"\"(.*?(?<!\\\\))\""_s );
  expression = expression.replace( rx3, u"'\\1'"_s );
  const thread_local QRegularExpression rx4 = QRegularExpression( u"\\\\\""_s );
  expression = expression.replace( rx4, u"\""_s );

  // ArcGIS's square brakets are double quotes in QGIS
  const thread_local QRegularExpression rx5 = QRegularExpression( u"\\[([^]]*)\\]"_s );
  expression = expression.replace( rx5, u"\"\\1\""_s );

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
  if ( style == "esriSLSSolid"_L1 )
    return Qt::SolidLine;
  else if ( style == "esriSLSDash"_L1 )
    return Qt::DashLine;
  else if ( style == "esriSLSDashDot"_L1 )
    return Qt::DashDotLine;
  else if ( style == "esriSLSDashDotDot"_L1 )
    return Qt::DashDotDotLine;
  else if ( style == "esriSLSDot"_L1 )
    return Qt::DotLine;
  else if ( style == "esriSLSNull"_L1 )
    return Qt::NoPen;
  else
    return Qt::SolidLine;
}

Qt::BrushStyle QgsArcGisRestUtils::convertFillStyle( const QString &style )
{
  if ( style == "esriSFSBackwardDiagonal"_L1 )
    return Qt::BDiagPattern;
  else if ( style == "esriSFSCross"_L1 )
    return Qt::CrossPattern;
  else if ( style == "esriSFSDiagonalCross"_L1 )
    return Qt::DiagCrossPattern;
  else if ( style == "esriSFSForwardDiagonal"_L1 )
    return Qt::FDiagPattern;
  else if ( style == "esriSFSHorizontal"_L1 )
    return Qt::HorPattern;
  else if ( style == "esriSFSNull"_L1 )
    return Qt::NoBrush;
  else if ( style == "esriSFSSolid"_L1 )
    return Qt::SolidPattern;
  else if ( style == "esriSFSVertical"_L1 )
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
    QgsDebugError( u"Invalid value %1 for datetime"_s.arg( value.toString() ) );
    return QDateTime();
  }
  else
    return dt;
}

QgsRectangle QgsArcGisRestUtils::convertRectangle( const QVariant &value )
{
  if ( QgsVariantUtils::isNull( value ) )
    return QgsRectangle();

  const QVariantMap coords = value.toMap();
  if ( coords.isEmpty() ) return QgsRectangle();

  bool ok;

  const double xmin = coords.value( u"xmin"_s ).toDouble( &ok );
  if ( ! ok ) return QgsRectangle();

  const double ymin = coords.value( u"ymin"_s ).toDouble( &ok );
  if ( ! ok ) return QgsRectangle();

  const double xmax = coords.value( u"xmax"_s ).toDouble( &ok );
  if ( ! ok ) return QgsRectangle();

  const double ymax = coords.value( u"ymax"_s ).toDouble( &ok );
  if ( ! ok ) return QgsRectangle();

  return QgsRectangle( xmin, ymin, xmax, ymax );

}


QVariantMap QgsArcGisRestUtils::geometryToJson( const QgsGeometry &geometry, const QgsArcGisRestContext &, const QgsCoordinateReferenceSystem &crs )
{
  QVariantMap res;
  if ( geometry.isNull() )
    return QVariantMap();

  const QgsAbstractGeometry *geom = geometry.constGet()->simplifiedTypeRef();
  switch ( QgsWkbTypes::flatType( geom->wkbType() ) )
  {
    case Qgis::WkbType::Unknown:
    case Qgis::WkbType::NoGeometry:
      return QVariantMap();

    case Qgis::WkbType::Point:
      res = pointToJson( qgsgeometry_cast< const QgsPoint * >( geom ) );
      break;

    case Qgis::WkbType::LineString:
      res = lineStringToJson( qgsgeometry_cast< const QgsLineString * >( geom ) );
      break;

    case Qgis::WkbType::CircularString:
    case Qgis::WkbType::CompoundCurve:
      res = curveToJson( qgsgeometry_cast< const QgsCurve * >( geom ) );
      break;

    case Qgis::WkbType::Polygon:
      res = polygonToJson( qgsgeometry_cast< const QgsPolygon * >( geom ) );
      break;

    case Qgis::WkbType::MultiPoint:
      res = multiPointToJson( qgsgeometry_cast< const QgsMultiPoint * >( geom ) );
      break;

    case Qgis::WkbType::MultiLineString:
      res = multiLineStringToJson( qgsgeometry_cast< const QgsMultiLineString * >( geom ) );
      break;

    case Qgis::WkbType::MultiCurve:
      res = multiCurveToJson( qgsgeometry_cast< const QgsMultiCurve * >( geom ) );
      break;

    case Qgis::WkbType::MultiPolygon:
      res = multiPolygonToJson( qgsgeometry_cast< const QgsMultiPolygon * >( geom ) );
      break;

    case Qgis::WkbType::CurvePolygon:
      res = curvePolygonToJson( qgsgeometry_cast< const QgsCurvePolygon * >( geom ) );
      break;

    case Qgis::WkbType::MultiSurface:
      res = multiSurfaceToJson( qgsgeometry_cast< const QgsMultiSurface * >( geom ) );
      break;

    case Qgis::WkbType::GeometryCollection:
      return QVariantMap(); // not supported by REST API

    case Qgis::WkbType::Triangle:
      return QVariantMap(); //not yet supported, but could be

    default:
      return QVariantMap(); //unreachable

  }

  if ( crs.isValid() )
  {
    // add spatialReference information
    res.insert( u"spatialReference"_s, crsToJson( crs ) );
  }

  return res;
}

QVariantMap QgsArcGisRestUtils::pointToJson( const QgsPoint *point )
{
  QVariantMap data;
  if ( point->isEmpty() )
    data[u"x"_s] = u"NaN"_s;
  else
  {
    data[u"x"_s] = point->x();
    data[u"y"_s] = point->y();

    if ( point->is3D() )
      data[u"z"_s] = !std::isnan( point->z() ) ? QVariant( point->z() ) :  QVariant( u"NaN"_s );

    if ( point->isMeasure() )
      data[u"m"_s] = !std::isnan( point->m() ) ? QVariant( point->m() ) :  QVariant( u"NaN"_s );
  }
  return data;
}

QVariantMap QgsArcGisRestUtils::multiPointToJson( const QgsMultiPoint *multiPoint )
{
  QVariantMap data;
  const bool hasZ = multiPoint->is3D();
  const bool hasM = multiPoint->isMeasure();
  data[u"hasM"_s] = hasM;
  data[u"hasZ"_s] = hasZ;

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

  data[u"points"_s] = pointsList;
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
    case Qgis::WkbType::LineString:
    {
      QVariantList part = lineStringToJsonPath( qgsgeometry_cast< const QgsLineString *>( curve ) );
      if ( !part.isEmpty() && !includeStart )
        part.removeAt( 0 );
      res = part;
      break;
    }

    case Qgis::WkbType::CircularString:
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

        curvePart.insert( u"c"_s, curveList );
        res.push_back( curvePart );
      }
      break;
    }

    case Qgis::WkbType::CompoundCurve:
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
  data[u"hasM"_s] = hasM;
  data[u"hasZ"_s] = hasZ;

  const QVariantList pointsList = lineStringToJsonPath( line );

  QVariantList pointsData = QVariantList();
  pointsData.push_back( pointsList );
  data[u"paths"_s] = pointsData;

  return data;
}

QVariantMap QgsArcGisRestUtils::curveToJson( const QgsCurve *curve )
{
  QVariantMap data;
  const bool hasZ = curve->is3D();
  const bool hasM = curve->isMeasure();
  data[u"hasM"_s] = hasM;
  data[u"hasZ"_s] = hasZ;

  const QVariantList curveList = curveToJsonCurve( curve, true );

  QVariantList curveData = QVariantList();
  curveData.push_back( curveList );
  data[u"curvePaths"_s] = curveData;

  return data;
}

QVariantMap QgsArcGisRestUtils::multiLineStringToJson( const QgsMultiLineString *multiLine )
{
  QVariantMap data;
  const bool hasZ = multiLine->is3D();
  const bool hasM = multiLine->isMeasure();
  data[u"hasM"_s] = hasM;
  data[u"hasZ"_s] = hasZ;

  const int size = multiLine->numGeometries();
  QVariantList paths;
  paths.reserve( size );
  for ( int i = 0; i < size; ++i )
  {
    const QgsLineString *line = multiLine->lineStringN( i );
    paths.push_back( lineStringToJsonPath( line ) );
  }

  data[u"paths"_s] = paths;
  return data;
}

QVariantMap QgsArcGisRestUtils::multiCurveToJson( const QgsMultiCurve *multiCurve )
{
  QVariantMap data;
  const bool hasZ = multiCurve->is3D();
  const bool hasM = multiCurve->isMeasure();
  data[u"hasM"_s] = hasM;
  data[u"hasZ"_s] = hasZ;

  const int size = multiCurve->numGeometries();
  QVariantList paths;
  paths.reserve( size );
  for ( int i = 0; i < size; ++i )
  {
    const QgsCurve *curve = multiCurve->curveN( i );
    paths.push_back( curveToJsonCurve( curve, true ) );
  }

  data[u"curvePaths"_s] = paths;
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
      case Qgis::AngularDirection::NoOrientation:
        break;
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
      case Qgis::AngularDirection::NoOrientation:
        break;
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
      case Qgis::AngularDirection::NoOrientation:
        break;
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
      case Qgis::AngularDirection::NoOrientation:
        break;
    }
  }
  return rings;
}

QVariantMap QgsArcGisRestUtils::polygonToJson( const QgsPolygon *polygon )
{
  QVariantMap data;
  const bool hasZ = polygon->is3D();
  const bool hasM = polygon->isMeasure();
  data[u"hasM"_s] = hasM;
  data[u"hasZ"_s] = hasZ;
  data[u"rings"_s] = polygonToJsonRings( polygon );
  return data;
}

QVariantMap QgsArcGisRestUtils::curvePolygonToJson( const QgsCurvePolygon *polygon )
{
  QVariantMap data;
  const bool hasZ = polygon->is3D();
  const bool hasM = polygon->isMeasure();
  data[u"hasM"_s] = hasM;
  data[u"hasZ"_s] = hasZ;
  data[u"curveRings"_s] = curvePolygonToJsonRings( polygon );
  return data;
}

QVariantMap QgsArcGisRestUtils::multiPolygonToJson( const QgsMultiPolygon *multiPolygon )
{
  QVariantMap data;
  const bool hasZ = multiPolygon->is3D();
  const bool hasM = multiPolygon->isMeasure();
  data[u"hasM"_s] = hasM;
  data[u"hasZ"_s] = hasZ;

  const int size = multiPolygon->numGeometries();
  QVariantList rings;
  for ( int i = 0; i < size; ++i )
  {
    const QgsPolygon *polygon = multiPolygon->polygonN( i );
    rings.append( polygonToJsonRings( polygon ) );
  }

  data[u"rings"_s] = rings;
  return data;
}

QVariantMap QgsArcGisRestUtils::multiSurfaceToJson( const QgsMultiSurface *multiSurface )
{
  QVariantMap data;
  const bool hasZ = multiSurface->is3D();
  const bool hasM = multiSurface->isMeasure();
  data[u"hasM"_s] = hasM;
  data[u"hasZ"_s] = hasZ;

  const int size = multiSurface->numGeometries();
  QVariantList rings;
  for ( int i = 0; i < size; ++i )
  {
    const QgsCurvePolygon *polygon = qgsgeometry_cast< const QgsCurvePolygon * >( multiSurface->geometryN( i ) );
    if ( !polygon )
      continue;

    rings.append( curvePolygonToJsonRings( polygon ) );
  }

  data[u"curveRings"_s] = rings;
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
    const thread_local QRegularExpression rxAuthid( u"(\\w+):(\\d+)"_s );
    const QRegularExpressionMatch match = rxAuthid.match( authid );
    if ( match.hasMatch()
         && (
           ( match.captured( 1 ).compare( "EPSG"_L1, Qt::CaseInsensitive ) == 0 )
           || ( match.captured( 1 ).compare( "ESRI"_L1, Qt::CaseInsensitive ) == 0 )
         )
       )
    {
      const QString wkid = match.captured( 2 );
      res.insert( u"wkid"_s, wkid );
      return res;
    }
  }

  // docs don't mention the WKT version support, so let's hope for 2.0...
  res.insert( u"wkt"_s, crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019Simplified ) );

  return res;
}

QVariantMap QgsArcGisRestUtils::featureToJson( const QgsFeature &feature, const QgsArcGisRestContext &context, const QgsCoordinateReferenceSystem &crs, QgsArcGisRestUtils::FeatureToJsonFlags flags )
{
  QVariantMap res;
  if ( ( flags & FeatureToJsonFlag::IncludeGeometry ) && feature.hasGeometry() )
  {
    res.insert( u"geometry"_s, geometryToJson( feature.geometry(), context, crs ) );
  }

  QVariantMap attributes;
  const QgsFields fields = feature.fields();
  for ( const QgsField &field : fields )
  {
    QVariant value = feature.attribute( field.name() );
    if ( value.userType() == qMetaTypeId< QgsUnsetAttributeValue >() )
    {
      if ( flags.testFlag( FeatureToJsonFlag::SkipUnsetAttributes ) )
        continue;
      else
        value = QVariant(); // reset to null, we can't store 'QgsUnsetAttributeValue' as json
    }

    if ( ( flags & FeatureToJsonFlag::IncludeNonObjectIdAttributes ) || field.name() == context.objectIdFieldName() )
      attributes.insert( field.name(), variantToAttributeValue( value, field.type(), context ) );
  }
  if ( !attributes.isEmpty() )
  {
    res.insert( u"attributes"_s, attributes );
  }
  return res;
}

QVariant QgsArcGisRestUtils::variantToAttributeValue( const QVariant &variant, QMetaType::Type expectedType, const QgsArcGisRestContext &context )
{
  if ( QgsVariantUtils::isNull( variant ) )
    return QVariant();

  switch ( expectedType )
  {
    case QMetaType::Type::QString:
    {
      const QString escaped = variant.toString().replace( '\\', "\\\\"_L1 ).replace( '"', "\\\""_L1 );
      return QString( QUrl::toPercentEncoding( escaped, "'" ) );
    }

    case QMetaType::Type::QDateTime:
    case QMetaType::Type::QDate:
    {
      switch ( variant.userType() )
      {
        case QMetaType::Type::QDateTime:
          return variant.toDateTime().toMSecsSinceEpoch();

        case QMetaType::Type::QDate:
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
  res.insert( u"name"_s, field.name() );

  QString fieldType;
  switch ( field.type() )
  {
    case QMetaType::Type::LongLong:
      fieldType = u"esriFieldTypeInteger"_s;
      break;

    case QMetaType::Type::Int:
      fieldType = u"esriFieldTypeSmallInteger"_s;
      break;

    case QMetaType::Type::Double:
      fieldType = u"esriFieldTypeDouble"_s;
      break;

    case QMetaType::Type::QString:
      fieldType = u"esriFieldTypeString"_s;
      break;

    case QMetaType::Type::QDateTime:
    case QMetaType::Type::QDate:
      fieldType = u"esriFieldTypeDate"_s;
      break;

    case QMetaType::Type::QByteArray:
      fieldType = u"esriFieldTypeBlob"_s;
      break;

    default:
      // fallback to string
      fieldType = u"esriFieldTypeString"_s;
      break;
  }
  res.insert( u"type"_s, fieldType );

  if ( !field.alias().isEmpty() )
    res.insert( u"alias"_s, field.alias() );

  // nullable
  const bool notNullable = field.constraints().constraints() & QgsFieldConstraints::Constraint::ConstraintNotNull;
  res.insert( u"nullable"_s, !notNullable );

  // editable
  res.insert( u"editable"_s, true );

  return res;
}

Qgis::ArcGisRestServiceType QgsArcGisRestUtils::serviceTypeFromString( const QString &type )
{
  if ( type.compare( "FeatureServer"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::FeatureServer;
  else if ( type.compare( "MapServer"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::MapServer;
  else if ( type.compare( "ImageServer"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::ImageServer;
  else if ( type.compare( "GlobeServer"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::GlobeServer;
  else if ( type.compare( "GPServer"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::GPServer;
  else if ( type.compare( "GeocodeServer"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::GeocodeServer;
  else if ( type.compare( "SceneServer"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::ArcGisRestServiceType::SceneServer;

  return Qgis::ArcGisRestServiceType::Unknown;
}

