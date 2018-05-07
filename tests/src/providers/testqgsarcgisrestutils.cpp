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
#include "qgssinglesymbolrenderer.h"
#include "qgscategorizedsymbolrenderer.h"
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
    void testParseEsriFillStyle();
    void testParseEsriLineStyle();
    void testParseEsriColorJson();
    void testParseMarkerSymbol();
    void testParseLineSymbol();
    void testParseFillSymbol();
    void testParseRendererSimple();
    void testParseRendererCategorized();

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
  QCOMPARE( crs.toWkt(), QStringLiteral( "PROJCS[\"unnamed\",GEOGCS[\"WGS 84\",DATUM[\"unknown\",SPHEROID[\"WGS84\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",49.225],PARAMETER[\"central_meridian\",-2.135],PARAMETER[\"scale_factor\",0.9999999],PARAMETER[\"false_easting\",40000],PARAMETER[\"false_northing\",70000],UNIT[\"Meter\",1]]" ) );
}

void TestQgsArcGisRestUtils::testMapEsriGeometryType()
{
  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "esriGeometryNull" ) ), QgsWkbTypes::Unknown );
  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "esriGeometryPoint" ) ), QgsWkbTypes::Point );
  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "esriGeometryMultipoint" ) ), QgsWkbTypes::MultiPoint );
  //unsure why this maps to multicurve and not multilinestring
  //QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral("esriGeometryPolyline") ),QgsWkbTypes::MultiCurve );
  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "esriGeometryPolygon" ) ), QgsWkbTypes::Polygon );
  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "esriGeometryEnvelope" ) ), QgsWkbTypes::Polygon );

  QCOMPARE( QgsArcGisRestUtils::mapEsriGeometryType( QStringLiteral( "xxx" ) ), QgsWkbTypes::Unknown );
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
