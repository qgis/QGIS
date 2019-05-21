/***************************************************************************
  testqgsarcgisrestutils.cpp

 ---------------------
 begin                : March 2018
 copyright            : (C) 2018 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"
#include "qgis.h"
#include "qgsarcgisrestutils.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsrulebasedlabeling.h"
#include "qgssinglesymbolrenderer.h"
#include "qgscategorizedsymbolrenderer.h"
#include "geometry/qgsmultisurface.h"

#include <QObject>

class TestQgsArcGisRestUtils : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.
    void testMapEsriFieldType();
    void testParseSpatialReference();
    void testMapEsriGeometryType();
    void testParseEsriGeometryPolygon();
    void testParseEsriFillStyle();
    void testParseEsriLineStyle();
    void testParseEsriColorJson();
    void testParseMarkerSymbol();
    void testPictureMarkerSymbol();
    void testParseLineSymbol();
    void testParseFillSymbol();
    void testParsePictureFillSymbol();
    void testParseRendererSimple();
    void testParseRendererCategorized();
    void testParseLabeling();

  private:

    QVariantMap jsonStringToMap( const QString &string ) const;

};



//runs before all tests
void TestQgsArcGisRestUtils::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

//runs after all tests
void TestQgsArcGisRestUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsArcGisRestUtils::testMapEsriFieldType()
{
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeInteger" ) ), QVariant::LongLong );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeSmallInteger" ) ), QVariant::Int );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeDouble" ) ), QVariant::Double );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeSingle" ) ), QVariant::Double );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeString" ) ), QVariant::String );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeDate" ) ), QVariant::Date );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeOID" ) ), QVariant::LongLong );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeBlob" ) ), QVariant::ByteArray );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeGlobalID" ) ), QVariant::String );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeRaster" ) ), QVariant::ByteArray );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeGUID" ) ), QVariant::String );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeXML" ) ), QVariant::String );

  // not valid fields
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "esriFieldTypeGeometry" ) ), QVariant::Invalid );
  QCOMPARE( QgsArcGisRestUtils::mapEsriFieldType( QStringLiteral( "xxx" ) ), QVariant::Invalid );
}

void TestQgsArcGisRestUtils::testParseSpatialReference()
{
  QVariantMap map;
  map.insert(
    QStringLiteral( "wkt" ),
    QStringLiteral( "PROJCS[\"NewJTM\",GEOGCS[\"GCS_ETRF_1989\",DATUM[\"D_ETRF_1989\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",40000.0],PARAMETER[\"False_Northing\",70000.0],PARAMETER[\"Central_Meridian\",-2.135],PARAMETER[\"Scale_Factor\",0.9999999],PARAMETER[\"Latitude_Of_Origin\",49.225],UNIT[\"Meter\",1.0]]" ) );

  QgsCoordinateReferenceSystem crs = QgsArcGisRestUtils::parseSpatialReference( map );
  QVERIFY( crs.isValid() );

#if PROJ_VERSION_MAJOR>=6
  QCOMPARE( crs.toWkt(), QStringLiteral( "PROJCS[\"unknown\",GEOGCS[\"unknown\",DATUM[\"Unknown_based_on_WGS84_ellipsoid\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",49.225],PARAMETER[\"central_meridian\",-2.135],PARAMETER[\"scale_factor\",0.9999999],PARAMETER[\"false_easting\",40000],PARAMETER[\"false_northing\",70000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]" ) );
#else
  QCOMPARE( crs.toWkt(), QStringLiteral( "PROJCS[\"unnamed\",GEOGCS[\"WGS 84\",DATUM[\"unknown\",SPHEROID[\"WGS84\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",49.225],PARAMETER[\"central_meridian\",-2.135],PARAMETER[\"scale_factor\",0.9999999],PARAMETER[\"false_easting\",40000],PARAMETER[\"false_northing\",70000],UNIT[\"Meter\",1]]" ) );
#endif
}

void TestQgsArcGisRestUtils::testMapEsriGeometryType()
{
  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "esriGeometryNull" ) ), QgsWkbTypes::Unknown );
  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "esriGeometryPoint" ) ), QgsWkbTypes::Point );
  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "esriGeometryMultipoint" ) ), QgsWkbTypes::MultiPoint );
  //unsure why this maps to multicurve and not multilinestring
  //QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral("esriGeometryPolyline") ),QgsWkbTypes::MultiCurve );
  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "esriGeometryPolygon" ) ), QgsWkbTypes::MultiPolygon );
  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "esriGeometryEnvelope" ) ), QgsWkbTypes::Polygon );

  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "xxx" ) ), QgsWkbTypes::Unknown );
}

void TestQgsArcGisRestUtils::testParseEsriGeometryPolygon()
{
  QVariantMap map = jsonStringToMap( "{"
                                     "\"rings\": ["
                                     "[[12,0],[13,0],[13,10],[12,10],[12,0]],"
                                     "[[3,3],[9,3],[6,9],[3,3]],"
                                     "[[0,0],[10,0],[10,10],[0,10],[0,0]]"
                                     "]"
                                     "}" );
  QCOMPARE( map[QStringLiteral( "rings" )].isValid(), true );
  std::unique_ptr<QgsMultiSurface> geometry = QgsArcGisRestUtils::parseEsriGeometryPolygon( map, QgsWkbTypes::Point );
  QVERIFY( geometry.get() );
  QCOMPARE( geometry->asWkt(), QStringLiteral( "MultiSurface (CurvePolygon (CompoundCurve ((0 0, 10 0, 10 10, 0 10, 0 0)),CompoundCurve ((3 3, 9 3, 6 9, 3 3))),CurvePolygon (CompoundCurve ((12 0, 13 0, 13 10, 12 10, 12 0))))" ) );
}

void TestQgsArcGisRestUtils::testParseEsriFillStyle()
{
  QCOMPARE( QgsArcGisRestUtils::parseEsriFillStyle( QStringLiteral( "esriSFSBackwardDiagonal" ) ), Qt::BDiagPattern );
  QCOMPARE( QgsArcGisRestUtils::parseEsriFillStyle( QStringLiteral( "esriSFSCross" ) ), Qt::CrossPattern );
  QCOMPARE( QgsArcGisRestUtils::parseEsriFillStyle( QStringLiteral( "esriSFSDiagonalCross" ) ), Qt::DiagCrossPattern );
  QCOMPARE( QgsArcGisRestUtils::parseEsriFillStyle( QStringLiteral( "esriSFSForwardDiagonal" ) ), Qt::FDiagPattern );
  QCOMPARE( QgsArcGisRestUtils::parseEsriFillStyle( QStringLiteral( "esriSFSHorizontal" ) ), Qt::HorPattern );
  QCOMPARE( QgsArcGisRestUtils::parseEsriFillStyle( QStringLiteral( "esriSFSNull" ) ), Qt::NoBrush );
  QCOMPARE( QgsArcGisRestUtils::parseEsriFillStyle( QStringLiteral( "esriSFSSolid" ) ), Qt::SolidPattern );
  QCOMPARE( QgsArcGisRestUtils::parseEsriFillStyle( QStringLiteral( "esriSFSVertical" ) ), Qt::VerPattern );
  QCOMPARE( QgsArcGisRestUtils::parseEsriFillStyle( QStringLiteral( "xxx" ) ), Qt::SolidPattern );
}

void TestQgsArcGisRestUtils::testParseEsriLineStyle()
{
  QCOMPARE( QgsArcGisRestUtils::parseEsriLineStyle( QStringLiteral( "esriSLSSolid" ) ), Qt::SolidLine );
  QCOMPARE( QgsArcGisRestUtils::parseEsriLineStyle( QStringLiteral( "esriSLSDash" ) ), Qt::DashLine );
  QCOMPARE( QgsArcGisRestUtils::parseEsriLineStyle( QStringLiteral( "esriSLSDashDot" ) ), Qt::DashDotLine );
  QCOMPARE( QgsArcGisRestUtils::parseEsriLineStyle( QStringLiteral( "esriSLSDashDotDot" ) ), Qt::DashDotDotLine );
  QCOMPARE( QgsArcGisRestUtils::parseEsriLineStyle( QStringLiteral( "esriSLSDot" ) ), Qt::DotLine );
  QCOMPARE( QgsArcGisRestUtils::parseEsriLineStyle( QStringLiteral( "esriSLSNull" ) ), Qt::NoPen );
  QCOMPARE( QgsArcGisRestUtils::parseEsriLineStyle( QStringLiteral( "xxx" ) ), Qt::SolidLine );
}

void TestQgsArcGisRestUtils::testParseEsriColorJson()
{
  QVERIFY( !QgsArcGisRestUtils::parseEsriColorJson( QString( "x" ) ).isValid() );
  QVERIFY( !QgsArcGisRestUtils::parseEsriColorJson( QVariantList() ).isValid() );
  QVERIFY( !QgsArcGisRestUtils::parseEsriColorJson( QVariantList() << 1 ).isValid() );
  QVERIFY( !QgsArcGisRestUtils::parseEsriColorJson( QVariantList() << 10 << 20 ).isValid() );
  QVERIFY( !QgsArcGisRestUtils::parseEsriColorJson( QVariantList() << 10 << 20 << 30 ).isValid() );
  QColor res = QgsArcGisRestUtils::parseEsriColorJson( QVariantList() << 10 << 20 << 30 << 40 );
  QCOMPARE( res.red(), 10 );
  QCOMPARE( res.green(), 20 );
  QCOMPARE( res.blue(), 30 );
  QCOMPARE( res.alpha(), 40 );
}

void TestQgsArcGisRestUtils::testParseMarkerSymbol()
{
  QVariantMap map = jsonStringToMap( "{"
                                     "\"type\": \"esriSMS\","
                                     "\"style\": \"esriSMSSquare\","
                                     "\"color\": ["
                                     "76,"
                                     "115,"
                                     "10,"
                                     "200"
                                     "],"
                                     "\"size\": 8,"
                                     "\"angle\": 10,"
                                     "\"xoffset\": 7,"
                                     "\"yoffset\": 17,"
                                     "\"outline\": {"
                                     "\"color\": ["
                                     "152,"
                                     "230,"
                                     "17,"
                                     "176"
                                     "],"
                                     "\"width\": 5"
                                     "}"
                                     "}" );
  std::unique_ptr<QgsSymbol> symbol = QgsArcGisRestUtils::parseEsriSymbolJson( map );
  QgsMarkerSymbol *marker = dynamic_cast< QgsMarkerSymbol * >( symbol.get() );
  QVERIFY( marker );
  QCOMPARE( marker->symbolLayerCount(), 1 );
  QgsSimpleMarkerSymbolLayer *markerLayer = dynamic_cast< QgsSimpleMarkerSymbolLayer * >( marker->symbolLayer( 0 ) );
  QVERIFY( markerLayer );
  QCOMPARE( markerLayer->fillColor(), QColor( 76, 115, 10, 200 ) );
  QCOMPARE( markerLayer->shape(), QgsSimpleMarkerSymbolLayerBase::Square );
  QCOMPARE( markerLayer->size(), 8.0 );
  QCOMPARE( markerLayer->sizeUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( markerLayer->angle(), -10.0 ); // opposite direction to esri spec!
  QCOMPARE( markerLayer->offset(), QPointF( 7, 17 ) );
  QCOMPARE( markerLayer->offsetUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( markerLayer->strokeColor(), QColor( 152, 230, 17, 176 ) );
  QCOMPARE( markerLayer->strokeWidth(), 5.0 );
  QCOMPARE( markerLayer->strokeWidthUnit(), QgsUnitTypes::RenderPoints );

  // invalid json
  symbol = QgsArcGisRestUtils::parseEsriMarkerSymbolJson( QVariantMap() );
  QVERIFY( !symbol );
}

void TestQgsArcGisRestUtils::testPictureMarkerSymbol()
{
  QVariantMap map = jsonStringToMap( "{"
                                     "\"type\": \"esriPMS\","
                                     "\"url\": \"471E7E31\","
                                     "\"imageData\": \"abcdef\","
                                     "\"contentType\": \"image/png\","
                                     "\"width\": 20,"
                                     "\"height\": 25,"
                                     "\"angle\": 10,"
                                     "\"xoffset\": 7,"
                                     "\"yoffset\": 17"
                                     "}" );
  std::unique_ptr<QgsSymbol> symbol = QgsArcGisRestUtils::parseEsriSymbolJson( map );
  QgsMarkerSymbol *marker = dynamic_cast< QgsMarkerSymbol * >( symbol.get() );
  QVERIFY( marker );
  QCOMPARE( marker->symbolLayerCount(), 1 );
  QgsRasterMarkerSymbolLayer *markerLayer = dynamic_cast< QgsRasterMarkerSymbolLayer * >( marker->symbolLayer( 0 ) );
  QVERIFY( markerLayer );
  QCOMPARE( markerLayer->path(), QStringLiteral( "base64:abcdef" ) );
  QCOMPARE( markerLayer->size(), 20.0 );
  QCOMPARE( markerLayer->fixedAspectRatio(), 1.25 );
  QCOMPARE( markerLayer->sizeUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( markerLayer->angle(), -10.0 ); // opposite direction to esri spec!
  QCOMPARE( markerLayer->offset(), QPointF( 7, 17 ) );
  QCOMPARE( markerLayer->offsetUnit(), QgsUnitTypes::RenderPoints );

  // invalid json
  symbol = QgsArcGisRestUtils::parseEsriPictureMarkerSymbolJson( QVariantMap() );
  QVERIFY( !symbol );
}

void TestQgsArcGisRestUtils::testParseLineSymbol()
{
  QVariantMap map = jsonStringToMap( "{"
                                     "\"type\": \"esriSLS\","
                                     "\"style\": \"esriSLSDot\","
                                     "\"color\": ["
                                     "115,"
                                     "76,"
                                     "10,"
                                     "212"
                                     "],"
                                     "\"width\": 7"
                                     "}" );
  std::unique_ptr<QgsSymbol> symbol = QgsArcGisRestUtils::parseEsriSymbolJson( map );
  QgsLineSymbol *line = dynamic_cast< QgsLineSymbol * >( symbol.get() );
  QVERIFY( line );
  QCOMPARE( line->symbolLayerCount(), 1 );
  QgsSimpleLineSymbolLayer *lineLayer = dynamic_cast< QgsSimpleLineSymbolLayer * >( line->symbolLayer( 0 ) );
  QVERIFY( lineLayer );
  QCOMPARE( lineLayer->color(), QColor( 115, 76, 10, 212 ) );
  QCOMPARE( lineLayer->width(), 7.0 );
  QCOMPARE( lineLayer->widthUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( lineLayer->penStyle(), Qt::DotLine );

  // invalid json
  symbol = QgsArcGisRestUtils::parseEsriLineSymbolJson( QVariantMap() );
  QVERIFY( !symbol );
}

void TestQgsArcGisRestUtils::testParseFillSymbol()
{
  QVariantMap map = jsonStringToMap( "{"
                                     "\"type\": \"esriSFS\","
                                     "\"style\": \"esriSFSHorizontal\","
                                     "\"color\": ["
                                     "115,"
                                     "76,"
                                     "10,"
                                     "200"
                                     "],"
                                     "\"outline\": {"
                                     "\"type\": \"esriSLS\","
                                     "\"style\": \"esriSLSDashDot\","
                                     "\"color\": ["
                                     "110,"
                                     "120,"
                                     "130,"
                                     "215"
                                     "],"
                                     "\"width\": 5"
                                     "}"
                                     "}" );
  std::unique_ptr<QgsSymbol> symbol = QgsArcGisRestUtils::parseEsriSymbolJson( map );
  QgsFillSymbol *fill = dynamic_cast< QgsFillSymbol * >( symbol.get() );
  QVERIFY( fill );
  QCOMPARE( fill->symbolLayerCount(), 1 );
  QgsSimpleFillSymbolLayer *fillLayer = dynamic_cast< QgsSimpleFillSymbolLayer * >( fill->symbolLayer( 0 ) );
  QVERIFY( fillLayer );
  QCOMPARE( fillLayer->fillColor(), QColor( 115, 76, 10, 200 ) );
  QCOMPARE( fillLayer->brushStyle(), Qt::HorPattern );
  QCOMPARE( fillLayer->strokeColor(), QColor( 110, 120, 130, 215 ) );
  QCOMPARE( fillLayer->strokeWidth(), 5.0 );
  QCOMPARE( fillLayer->strokeWidthUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( fillLayer->strokeStyle(), Qt::DashDotLine );
}


void TestQgsArcGisRestUtils::testParsePictureFillSymbol()
{
  QVariantMap map = jsonStringToMap( "{"
                                     "\"type\": \"esriPFS\","
                                     "\"url\": \"866880A0\","
                                     "\"imageData\": \"abcdef\","
                                     "\"contentType\": \"image/png\","
                                     "\"width\": 20,"
                                     "\"height\": 25,"
                                     "\"angle\": 0,"
                                     "\"outline\": {"
                                     "\"type\": \"esriSLS\","
                                     "\"style\": \"esriSLSDashDot\","
                                     "\"color\": ["
                                     "110,"
                                     "120,"
                                     "130,"
                                     "215"
                                     "],"
                                     "\"width\": 5"
                                     "}"
                                     "}" );
  std::unique_ptr<QgsSymbol> symbol = QgsArcGisRestUtils::parseEsriSymbolJson( map );
  QgsFillSymbol *fill = dynamic_cast< QgsFillSymbol * >( symbol.get() );
  QVERIFY( fill );
  QCOMPARE( fill->symbolLayerCount(), 2 );
  QgsRasterFillSymbolLayer *fillLayer = dynamic_cast< QgsRasterFillSymbolLayer * >( fill->symbolLayer( 0 ) );
  QVERIFY( fillLayer );
  QCOMPARE( fillLayer->imageFilePath(), QString( "base64:abcdef" ) );
  QCOMPARE( fillLayer->width(), 20.0 );
  QCOMPARE( fillLayer->widthUnit(), QgsUnitTypes::RenderPoints );
  QgsSimpleLineSymbolLayer *lineLayer = dynamic_cast< QgsSimpleLineSymbolLayer * >( fill->symbolLayer( 1 ) );
  QVERIFY( lineLayer );
  QCOMPARE( lineLayer->color(), QColor( 110, 120, 130, 215 ) );
  QCOMPARE( lineLayer->width(), 5.0 );
  QCOMPARE( lineLayer->widthUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( lineLayer->penStyle(), Qt::DashDotLine );
}

void TestQgsArcGisRestUtils::testParseRendererSimple()
{
  QVariantMap map = jsonStringToMap( "{"
                                     "\"type\": \"simple\","
                                     "\"symbol\": {"
                                     "\"color\": ["
                                     "0,"
                                     "0,"
                                     "128,"
                                     "128"
                                     "],"
                                     "\"size\": 15,"
                                     "\"angle\": 0,"
                                     "\"xoffset\": 0,"
                                     "\"yoffset\": 0,"
                                     "\"type\": \"esriSMS\","
                                     "\"style\": \"esriSMSCircle\","
                                     "\"outline\": {"
                                     "\"color\": ["
                                     "0,"
                                     "0,"
                                     "128,"
                                     "255"
                                     "],"
                                     "\"width\": 0.99975,"
                                     "\"type\": \"esriSLS\","
                                     "\"style\": \"esriSLSSolid\""
                                     "}"
                                     "}"
                                     "}" );
  std::unique_ptr< QgsFeatureRenderer > renderer( QgsArcGisRestUtils::parseEsriRenderer( map ) );
  QgsSingleSymbolRenderer *ssRenderer = dynamic_cast< QgsSingleSymbolRenderer *>( renderer.get() );
  QVERIFY( ssRenderer );
  QVERIFY( ssRenderer->symbol() );
}

void TestQgsArcGisRestUtils::testParseRendererCategorized()
{
  QVariantMap map = jsonStringToMap( "{"
                                     "\"type\": \"uniqueValue\","
                                     "\"field1\": \"COUNTRY\","
                                     "\"uniqueValueInfos\": ["
                                     "{"
                                     "\"value\": \"US\","
                                     "\"symbol\": {"
                                     "\"color\": ["
                                     "253,"
                                     "127,"
                                     "111,"
                                     "255"
                                     "],"
                                     "\"size\": 12.75,"
                                     "\"angle\": 0,"
                                     "\"xoffset\": 0,"
                                     "\"yoffset\": 0,"
                                     "\"type\": \"esriSMS\","
                                     "\"style\": \"esriSMSCircle\","
                                     "\"outline\": {"
                                     "\"color\": ["
                                     "26,"
                                     "26,"
                                     "26,"
                                     "255"
                                     "],"
                                     "\"width\": 0.75,"
                                     "\"type\": \"esriSLS\","
                                     "\"style\": \"esriSLSSolid\""
                                     "}"
                                     "},"
                                     "\"label\": \"United States\""
                                     "},"
                                     "{"
                                     "\"value\": \"Canada\","
                                     "\"symbol\": {"
                                     "\"color\": ["
                                     "126,"
                                     "176,"
                                     "213,"
                                     "255"
                                     "],"
                                     "\"size\": 12.75,"
                                     "\"angle\": 0,"
                                     "\"xoffset\": 0,"
                                     "\"yoffset\": 0,"
                                     "\"type\": \"esriSMS\","
                                     "\"style\": \"esriSMSCircle\","
                                     "\"outline\": {"
                                     "\"color\": ["
                                     "26,"
                                     "26,"
                                     "26,"
                                     "255"
                                     "],"
                                     "\"width\": 0.75,"
                                     "\"type\": \"esriSLS\","
                                     "\"style\": \"esriSLSSolid\""
                                     "}"
                                     "},"
                                     "\"label\": \"Canada\""
                                     "}"
                                     "]"
                                     "}" );
  std::unique_ptr< QgsFeatureRenderer > renderer( QgsArcGisRestUtils::parseEsriRenderer( map ) );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast< QgsCategorizedSymbolRenderer *>( renderer.get() );
  QVERIFY( catRenderer );
  QCOMPARE( catRenderer->categories().count(), 2 );
  QCOMPARE( catRenderer->categories().at( 0 ).value().toString(), QStringLiteral( "US" ) );
  QCOMPARE( catRenderer->categories().at( 0 ).label(), QStringLiteral( "United States" ) );
  QVERIFY( catRenderer->categories().at( 0 ).symbol() );
  QCOMPARE( catRenderer->categories().at( 1 ).value().toString(), QStringLiteral( "Canada" ) );
  QCOMPARE( catRenderer->categories().at( 1 ).label(), QStringLiteral( "Canada" ) );
  QVERIFY( catRenderer->categories().at( 1 ).symbol() );
}

void TestQgsArcGisRestUtils::testParseLabeling()
{
  QVariantMap map = jsonStringToMap( "{"
                                     "\"labelingInfo\": ["
                                     "{"
                                     "\"labelPlacement\": \"esriServerPointLabelPlacementAboveRight\","
                                     "\"where\": \"1=1\","
                                     "\"labelExpression\": \"[Name]\","
                                     "\"useCodedValues\": true,"
                                     "\"symbol\": {"
                                     "\"type\": \"esriTS\","
                                     "\"color\": ["
                                     "255,"
                                     "0,"
                                     "0,"
                                     "255"
                                     "],"
                                     "\"backgroundColor\": null,"
                                     "\"borderLineColor\": null,"
                                     "\"borderLineSize\": null,"
                                     "\"verticalAlignment\": \"bottom\","
                                     "\"horizontalAlignment\": \"center\","
                                     "\"rightToLeft\": false,"
                                     "\"angle\": 0,"
                                     "\"xoffset\": 0,"
                                     "\"yoffset\": 0,"
                                     "\"haloColor\": null,"
                                     "\"haloSize\": null,"
                                     "\"font\": {"
                                     "\"family\": \"Arial\","
                                     "\"size\": 8,"
                                     "\"style\": \"normal\","
                                     "\"weight\": \"bold\","
                                     "\"decoration\": \"none\""
                                     "}"
                                     "},"
                                     "\"minScale\": 200000,"
                                     "\"maxScale\": 0"
                                     "},{"
                                     "\"labelPlacement\": \"esriServerPointLabelPlacementAboveRight\","
                                     "\"where\": \"1_testing broken where string\","
                                     "\"labelExpression\": \"\\\"Name: \\\" CONCAT [Name] CONCAT NEWLINE CONCAT [Size]\","
                                     "\"useCodedValues\": true,"
                                     "\"symbol\": {"
                                     "\"type\": \"esriTS\","
                                     "\"color\": ["
                                     "255,"
                                     "0,"
                                     "0,"
                                     "255"
                                     "],"
                                     "\"backgroundColor\": null,"
                                     "\"borderLineColor\": null,"
                                     "\"borderLineSize\": null,"
                                     "\"verticalAlignment\": \"bottom\","
                                     "\"horizontalAlignment\": \"center\","
                                     "\"rightToLeft\": false,"
                                     "\"angle\": 0,"
                                     "\"xoffset\": 0,"
                                     "\"yoffset\": 0,"
                                     "\"haloColor\": ["
                                     "255,"
                                     "255,"
                                     "255,"
                                     "255"
                                     "],"
                                     "\"haloSize\": 1,"
                                     "\"font\": {"
                                     "\"family\": \"Arial\","
                                     "\"size\": 8,"
                                     "\"style\": \"normal\","
                                     "\"weight\": \"bold\","
                                     "\"decoration\": \"none\""
                                     "}"
                                     "},"
                                     "\"minScale\": 200000,"
                                     "\"maxScale\": 0"
                                     "}"
                                     "]"
                                     "}" );
  std::unique_ptr< QgsAbstractVectorLayerLabeling > labeling( QgsArcGisRestUtils::parseEsriLabeling( map.value( QStringLiteral( "labelingInfo" ) ).toList() ) );
  QVERIFY( labeling );
  QgsRuleBasedLabeling *rules = dynamic_cast< QgsRuleBasedLabeling *>( labeling.get() );
  QVERIFY( rules );
  QgsRuleBasedLabeling::Rule *root = rules->rootRule();
  QVERIFY( root );

  QgsRuleBasedLabeling::RuleList children = root->children();
  QCOMPARE( children.count(), 2 );
  //checking filter expression from valid where string
  QCOMPARE( children.at( 0 )->filterExpression(), QStringLiteral( "1=1" ) );
  //checking empty filter expression from invalid where string
  QCOMPARE( children.at( 1 )->filterExpression(), QString( "" ) );
  QCOMPARE( children.at( 0 )->minimumScale(), 200000.0 );
  QCOMPARE( children.at( 0 )->maximumScale(), 0.0 );

  QgsPalLayerSettings *settings = children.at( 0 )->settings();
  QVERIFY( settings );
  QCOMPARE( settings->placement, QgsPalLayerSettings::OverPoint );
  QCOMPARE( settings->quadOffset, QgsPalLayerSettings::QuadrantAboveRight );
  QCOMPARE( settings->fieldName, QStringLiteral( "\"Name\"" ) );

  QgsTextFormat textFormat = settings->format();
  QCOMPARE( textFormat.color(), QColor( 255, 0, 0 ) );
  QCOMPARE( textFormat.size(), 8.0 );
  QCOMPARE( textFormat.buffer().enabled(), false );

  settings = children.at( 1 )->settings();
  QVERIFY( settings );
  QCOMPARE( settings->fieldName, QStringLiteral( "'Name: ' || \"Name\" || '\\n' || \"Size\"" ) );

  textFormat = settings->format();
  QCOMPARE( textFormat.buffer().enabled(), true );
  QCOMPARE( textFormat.buffer().color(), QColor( 255, 255, 255 ) );
  QCOMPARE( textFormat.buffer().size(), 1.0 );
  QCOMPARE( textFormat.buffer().sizeUnit(), QgsUnitTypes::RenderPoints );
}

QVariantMap TestQgsArcGisRestUtils::jsonStringToMap( const QString &string ) const
{
  QJsonDocument doc = QJsonDocument::fromJson( string.toUtf8() );
  if ( doc.isNull() )
  {
    return QVariantMap();
  }
  return doc.object().toVariantMap();
}

QGSTEST_MAIN( TestQgsArcGisRestUtils )
#include "testqgsarcgisrestutils.moc"
