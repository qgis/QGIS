
/***************************************************************************
     testqgsogcutils.cpp
     --------------------------------------
    Date                 : March 2013
    Copyright            : (C) 2013 Martin Dobias
    Email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>
#include <QSharedPointer>

//qgis includes...
#include <qgsgeometry.h>
#include <qgsogcutils.h>
#include "qgsapplication.h"

/** \ingroup UnitTests
 * This is a unit test for OGC utilities
 */
class TestQgsOgcUtils : public QObject
{
    Q_OBJECT
  private slots:

    void initTestCase()
    {
      //
      // Runs once before any tests are run
      //
      // init QGIS's paths - true means that all path will be inited from prefix
      QgsApplication::init();
      QgsApplication::initQgis();
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void testGeometryFromGML();
    void testGeometryToGML();

    void testExpressionFromOgcFilter();
    void testExpressionFromOgcFilter_data();

    void testExpressionToOgcFilter();
    void testExpressionToOgcFilter_data();

    void testExpressionToOgcFilterWFS11();
    void testExpressionToOgcFilterWFS11_data();

#if QT_VERSION < 0x050000
    void testExpressionToOgcFilterWFS20();
    void testExpressionToOgcFilterWFS20_data();
#endif
};


void TestQgsOgcUtils::testGeometryFromGML()
{
  // Test GML2
  QSharedPointer<QgsGeometry> geom( QgsOgcUtils::geometryFromGML( "<Point><coordinates>123,456</coordinates></Point>" ) );
  QVERIFY( geom );
  QVERIFY( geom->wkbType() == QGis::WKBPoint );
  QVERIFY( geom->asPoint() == QgsPoint( 123, 456 ) );
  geom.clear();

  QSharedPointer<QgsGeometry> geomBox( QgsOgcUtils::geometryFromGML( "<gml:Box srsName=\"foo\"><gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box>" ) );
  QVERIFY( geomBox );
  QVERIFY( geomBox->wkbType() == QGis::WKBPolygon );


  // Test GML3
  geom = QSharedPointer<QgsGeometry>( QgsOgcUtils::geometryFromGML( "<Point><pos>123 456</pos></Point>" ) );
  QVERIFY( geom );
  QVERIFY( geom->wkbType() == QGis::WKBPoint );
  QVERIFY( geom->asPoint() == QgsPoint( 123, 456 ) );

  geomBox = QSharedPointer<QgsGeometry>( QgsOgcUtils::geometryFromGML( "<gml:Envelope srsName=\"foo\"><gml:lowerCorner>135.2239 34.4879</gml:lowerCorner><gml:upperCorner>135.8578 34.8471</gml:upperCorner></gml:Envelope>" ) );
  QVERIFY( geomBox );
  QVERIFY( geomBox->wkbType() == QGis::WKBPolygon );
}

void TestQgsOgcUtils::testGeometryToGML()
{
  QDomDocument doc;
  QSharedPointer<QgsGeometry> geomPoint( QgsGeometry::fromPoint( QgsPoint( 111, 222 ) ) );
  QSharedPointer<QgsGeometry> geomLine( QgsGeometry::fromWkt( "LINESTRING(111 222, 222 222)" ) );

  // Test GML2
  QDomElement elemInvalid = QgsOgcUtils::geometryToGML( 0, doc );
  QVERIFY( elemInvalid.isNull() );

  QDomElement elemPoint = QgsOgcUtils::geometryToGML( geomPoint.data(), doc );
  QVERIFY( !elemPoint.isNull() );

  doc.appendChild( elemPoint );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:Point><gml:coordinates cs=\",\" ts=\" \">111,222</gml:coordinates></gml:Point>" ) );
  doc.removeChild( elemPoint );

  QDomElement elemLine = QgsOgcUtils::geometryToGML( geomLine.data(), doc );
  QVERIFY( !elemLine.isNull() );

  doc.appendChild( elemLine );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:LineString><gml:coordinates cs=\",\" ts=\" \">111,222 222,222</gml:coordinates></gml:LineString>" ) );
  doc.removeChild( elemLine );

  // Test GML3
  elemInvalid = QgsOgcUtils::geometryToGML( 0, doc, "GML3" );
  QVERIFY( elemInvalid.isNull() );

  elemPoint = QgsOgcUtils::geometryToGML( geomPoint.data(), doc, "GML3" );
  QVERIFY( !elemPoint.isNull() );

  doc.appendChild( elemPoint );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:Point><gml:pos srsDimension=\"2\">111 222</gml:pos></gml:Point>" ) );
  doc.removeChild( elemPoint );

  elemLine = QgsOgcUtils::geometryToGML( geomLine.data(), doc, "GML3" );
  QVERIFY( !elemLine.isNull() );

  doc.appendChild( elemLine );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:LineString><gml:posList srsDimension=\"2\">111 222 222 222</gml:posList></gml:LineString>" ) );
  doc.removeChild( elemLine );
}


void TestQgsOgcUtils::testExpressionFromOgcFilter_data()
{
  QTest::addColumn<QString>( "xmlText" );
  QTest::addColumn<QString>( "dumpText" );

  QTest::newRow( "=" ) << QString(
    "<Filter><PropertyIsEqualTo>"
    "<PropertyName>NAME</PropertyName>"
    "<Literal>New York</Literal>"
    "</PropertyIsEqualTo></Filter>" )
  << QString( "NAME = 'New York'" );

  QTest::newRow( ">" ) << QString(
    "<Filter><PropertyIsGreaterThan>"
    "<PropertyName>COUNT</PropertyName>"
    "<Literal>3</Literal>"
    "</PropertyIsGreaterThan></Filter>" )
  << QString( "COUNT > 3" );

  QTest::newRow( "AND" ) << QString(
    "<ogc:Filter>"
    "<ogc:And>"
    "<ogc:PropertyIsGreaterThanOrEqualTo>"
    "<ogc:PropertyName>pop</ogc:PropertyName>"
    "<ogc:Literal>50000</ogc:Literal>"
    "</ogc:PropertyIsGreaterThanOrEqualTo>"
    "<ogc:PropertyIsLessThan>"
    "<ogc:PropertyName>pop</ogc:PropertyName>"
    "<ogc:Literal>100000</ogc:Literal>"
    "</ogc:PropertyIsLessThan>"
    "</ogc:And>"
    "</ogc:Filter>" )
  << QString( "pop >= 50000 AND pop < 100000" );

  // TODO: should work also without <Literal> tags in Lower/Upper-Boundary tags?
  QTest::newRow( "between" ) << QString(
    "<Filter>"
    "<PropertyIsBetween><PropertyName>POPULATION</PropertyName>"
    "<LowerBoundary><Literal>100</Literal></LowerBoundary>"
    "<UpperBoundary><Literal>200</Literal></UpperBoundary></PropertyIsBetween>"
    "</Filter>" )
  << QString( "POPULATION >= 100 AND POPULATION <= 200" );

  // TODO: needs to handle different wildcards, single chars, escape chars
  QTest::newRow( "like" ) << QString(
    "<Filter>"
    "<PropertyIsLike wildcard='*' singleChar='.' escape='!'>"
    "<PropertyName>NAME</PropertyName><Literal>*QGIS*</Literal></PropertyIsLike>"
    "</Filter>" )
  << QString( "NAME LIKE '*QGIS*'" );

  QTest::newRow( "is null" ) << QString(
    "<Filter>"
    "<ogc:PropertyIsNull>"
    "<ogc:PropertyName>FIRST_NAME</ogc:PropertyName>"
    "</ogc:PropertyIsNull>"
    "</Filter>" )
  << QString( "FIRST_NAME IS NULL" );

  QTest::newRow( "bbox with GML2 Box" ) << QString(
    "<Filter>"
    "<BBOX><PropertyName>Name>NAME</PropertyName><gml:Box srsName='foo'>"
    "<gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box></BBOX>"
    "</Filter>" )
  << QString( "intersects_bbox($geometry, geom_from_gml('<Box srsName=\"foo\"><coordinates>135.2239,34.4879 135.8578,34.8471</coordinates></Box>'))" );

  QTest::newRow( "Intersects" ) << QString(
    "<Filter>"
    "<Intersects>"
    "<PropertyName>GEOMETRY</PropertyName>"
    "<gml:Point>"
    "<gml:coordinates>123,456</gml:coordinates>"
    "</gml:Point>"
    "</Intersects>"
    "</Filter>" )
  << QString( "intersects($geometry, geom_from_gml('<Point><coordinates>123,456</coordinates></Point>'))" );
}

void TestQgsOgcUtils::testExpressionFromOgcFilter()
{
  QFETCH( QString, xmlText );
  QFETCH( QString, dumpText );

  QDomDocument doc;
  QVERIFY( doc.setContent( xmlText, true ) );
  QDomElement rootElem = doc.documentElement();

  QSharedPointer<QgsExpression> expr( QgsOgcUtils::expressionFromOgcFilter( rootElem ) );
  QVERIFY( expr );

  qDebug( "OGC XML  : %s", xmlText.toAscii().data() );
  qDebug( "EXPR-DUMP: %s", expr->expression().toAscii().data() );

  if ( expr->hasParserError() )
    qDebug( "ERROR: %s ", expr->parserErrorString().toAscii().data() );
  QVERIFY( !expr->hasParserError() );

  QCOMPARE( dumpText, expr->expression() );
}

void TestQgsOgcUtils::testExpressionToOgcFilter()
{
  QFETCH( QString, exprText );
  QFETCH( QString, xmlText );

  QgsExpression exp( exprText );
  QVERIFY( !exp.hasParserError() );

  QString errorMsg;
  QDomDocument doc;
  QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( exp, doc, &errorMsg );

  if ( !errorMsg.isEmpty() )
    qDebug( "ERROR: %s", errorMsg.toAscii().data() );

  QVERIFY( !filterElem.isNull() );

  doc.appendChild( filterElem );

  qDebug( "EXPR: %s", exp.expression().toAscii().data() );
  qDebug( "OGC : %s", doc.toString( -1 ).toAscii().data() );

  QCOMPARE( xmlText, doc.toString( -1 ) );
}

void TestQgsOgcUtils::testExpressionToOgcFilter_data()
{
  QTest::addColumn<QString>( "exprText" );
  QTest::addColumn<QString>( "xmlText" );

  QTest::newRow( "=" ) << QString( "NAME = 'New York'" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
    "<ogc:PropertyIsEqualTo>"
    "<ogc:PropertyName>NAME</ogc:PropertyName>"
    "<ogc:Literal>New York</ogc:Literal>"
    "</ogc:PropertyIsEqualTo></ogc:Filter>" );

  QTest::newRow( ">" ) << QString( "COUNT > 3" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
    "<ogc:PropertyIsGreaterThan>"
    "<ogc:PropertyName>COUNT</ogc:PropertyName>"
    "<ogc:Literal>3</ogc:Literal>"
    "</ogc:PropertyIsGreaterThan></ogc:Filter>" );

  QTest::newRow( "and+or" ) << QString( "(FIELD1 = 10 OR FIELD1 = 20) AND STATUS = 'VALID'" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
    "<ogc:And>"
    "<ogc:Or>"
    "<ogc:PropertyIsEqualTo>"
    "<ogc:PropertyName>FIELD1</ogc:PropertyName>"
    "<ogc:Literal>10</ogc:Literal>"
    "</ogc:PropertyIsEqualTo>"
    "<ogc:PropertyIsEqualTo>"
    "<ogc:PropertyName>FIELD1</ogc:PropertyName>"
    "<ogc:Literal>20</ogc:Literal>"
    "</ogc:PropertyIsEqualTo>"
    "</ogc:Or>"
    "<ogc:PropertyIsEqualTo>"
    "<ogc:PropertyName>STATUS</ogc:PropertyName>"
    "<ogc:Literal>VALID</ogc:Literal>"
    "</ogc:PropertyIsEqualTo>"
    "</ogc:And>"
    "</ogc:Filter>" );

  QTest::newRow( "is null" ) << QString( "A IS NULL" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
    "<ogc:PropertyIsNull>"
    "<ogc:PropertyName>A</ogc:PropertyName>"
    "</ogc:PropertyIsNull>"
    "</ogc:Filter>" );

  QTest::newRow( "is not null" ) << QString( "A IS NOT NULL" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
    "<ogc:Not>"
    "<ogc:PropertyIsNull>"
    "<ogc:PropertyName>A</ogc:PropertyName>"
    "</ogc:PropertyIsNull>"
    "</ogc:Not>"
    "</ogc:Filter>" );

  QTest::newRow( "in" ) << QString( "A IN (10,20,30)" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
    "<ogc:Or>"
    "<ogc:PropertyIsEqualTo>"
    "<ogc:PropertyName>A</ogc:PropertyName>"
    "<ogc:Literal>10</ogc:Literal>"
    "</ogc:PropertyIsEqualTo>"
    "<ogc:PropertyIsEqualTo>"
    "<ogc:PropertyName>A</ogc:PropertyName>"
    "<ogc:Literal>20</ogc:Literal>"
    "</ogc:PropertyIsEqualTo>"
    "<ogc:PropertyIsEqualTo>"
    "<ogc:PropertyName>A</ogc:PropertyName>"
    "<ogc:Literal>30</ogc:Literal>"
    "</ogc:PropertyIsEqualTo>"
    "</ogc:Or>"
    "</ogc:Filter>" );

  QTest::newRow( "intersects_bbox" ) << QString( "intersects_bbox($geometry, geomFromWKT('POINT (5 6)'))" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
    "<ogc:BBOX>"
    "<ogc:PropertyName>geometry</ogc:PropertyName>"
    "<gml:Box><gml:coordinates cs=\",\" ts=\" \">5,6 5,6</gml:coordinates></gml:Box>"
    "</ogc:BBOX>"
    "</ogc:Filter>" );

  QTest::newRow( "intersects + wkt" ) << QString( "intersects($geometry, geomFromWKT('POINT (5 6)'))" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
    "<ogc:Intersects>"
    "<ogc:PropertyName>geometry</ogc:PropertyName>"
    "<gml:Point><gml:coordinates cs=\",\" ts=\" \">5,6</gml:coordinates></gml:Point>"
    "</ogc:Intersects>"
    "</ogc:Filter>" );

  QTest::newRow( "contains + gml" ) << QString( "contains($geometry, geomFromGML('<Point><coordinates cs=\",\" ts=\" \">5,6</coordinates></Point>'))" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
    "<ogc:Contains>"
    "<ogc:PropertyName>geometry</ogc:PropertyName>"
    "<Point><coordinates cs=\",\" ts=\" \">5,6</coordinates></Point>"
    "</ogc:Contains>"
    "</ogc:Filter>" );
}

void TestQgsOgcUtils::testExpressionToOgcFilterWFS11()
{
  QFETCH( QString, exprText );
  QFETCH( QString, srsName );
  QFETCH( QString, xmlText );

  QgsExpression exp( exprText );
  QVERIFY( !exp.hasParserError() );

  QString errorMsg;
  QDomDocument doc;
  QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( exp, doc,
                           QgsOgcUtils::GML_3_1_0, QgsOgcUtils::FILTER_OGC_1_1, "my_geometry_name", srsName, true, false, &errorMsg );

  if ( !errorMsg.isEmpty() )
    qDebug( "ERROR: %s", errorMsg.toAscii().data() );

  QVERIFY( !filterElem.isNull() );

  doc.appendChild( filterElem );

  qDebug( "EXPR: %s", exp.expression().toAscii().data() );
  qDebug( "SRSNAME: %s", srsName.toAscii().data() );
  qDebug( "OGC : %s", doc.toString( -1 ).toAscii().data() );

  QCOMPARE( xmlText, doc.toString( -1 ) );
}

void TestQgsOgcUtils::testExpressionToOgcFilterWFS11_data()
{
  QTest::addColumn<QString>( "exprText" );
  QTest::addColumn<QString>( "srsName" );
  QTest::addColumn<QString>( "xmlText" );

  QTest::newRow( "bbox" )
  << QString( "intersects_bbox($geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))" )
  << QString( "urn:ogc:def:crs:EPSG::4326" )
  << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
    "<ogc:BBOX>"
    "<ogc:PropertyName>my_geometry_name</ogc:PropertyName>"
    "<gml:Envelope srsName=\"urn:ogc:def:crs:EPSG::4326\">"
    "<gml:lowerCorner>49 2</gml:lowerCorner>"
    "<gml:upperCorner>50 3</gml:upperCorner>"
    "</gml:Envelope>"
    "</ogc:BBOX>"
    "</ogc:Filter>" );
}

// There's an issue with QT 5 that appears to reverse the order of multiple attributes
#if QT_VERSION < 0x050000

void TestQgsOgcUtils::testExpressionToOgcFilterWFS20()
{
  QFETCH( QString, exprText );
  QFETCH( QString, srsName );
  QFETCH( QString, xmlText );

  QgsExpression exp( exprText );
  QVERIFY( !exp.hasParserError() );

  QString errorMsg;
  QDomDocument doc;
  QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( exp, doc,
                           QgsOgcUtils::GML_3_2_1, QgsOgcUtils::FILTER_FES_2_0, "my_geometry_name", srsName, true, false, &errorMsg );

  if ( !errorMsg.isEmpty() )
    qDebug( "ERROR: %s", errorMsg.toAscii().data() );

  QVERIFY( !filterElem.isNull() );

  doc.appendChild( filterElem );

  qDebug( "EXPR: %s", exp.expression().toAscii().data() );
  qDebug( "SRSNAME: %s", srsName.toAscii().data() );
  qDebug( "OGC : %s", doc.toString( -1 ).toAscii().data() );

  QCOMPARE( xmlText, doc.toString( -1 ) );
}

void TestQgsOgcUtils::testExpressionToOgcFilterWFS20_data()
{
  QTest::addColumn<QString>( "exprText" );
  QTest::addColumn<QString>( "srsName" );
  QTest::addColumn<QString>( "xmlText" );

  QTest::newRow( "=" ) << QString( "NAME = 'New York'" ) << QString() << QString(
    "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">"
    "<fes:PropertyIsEqualTo>"
    "<fes:ValueReference>NAME</fes:ValueReference>"
    "<fes:Literal>New York</fes:Literal>"
    "</fes:PropertyIsEqualTo></fes:Filter>" );

  QTest::newRow( "bbox" )
  << QString( "intersects_bbox($geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))" )
  << QString( "urn:ogc:def:crs:EPSG::4326" )
  << QString(
    "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:gml=\"http://www.opengis.net/gml/3.2\">"
    "<fes:BBOX>"
    "<fes:ValueReference>my_geometry_name</fes:ValueReference>"
    "<gml:Envelope srsName=\"urn:ogc:def:crs:EPSG::4326\">"
    "<gml:lowerCorner>49 2</gml:lowerCorner>"
    "<gml:upperCorner>50 3</gml:upperCorner>"
    "</gml:Envelope>"
    "</fes:BBOX>"
    "</fes:Filter>" );

  QTest::newRow( "intersects" )
  << QString( "intersects($geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))" )
  << QString( "urn:ogc:def:crs:EPSG::4326" )
  << QString(
    "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:gml=\"http://www.opengis.net/gml/3.2\">"
    "<fes:Intersects>"
    "<fes:ValueReference>my_geometry_name</fes:ValueReference>"
    "<gml:Polygon gml:id=\"qgis_id_geom_1\" srsName=\"urn:ogc:def:crs:EPSG::4326\">"
    "<gml:exterior>"
    "<gml:LinearRing>"
    "<gml:posList srsDimension=\"2\">49 2 50 2 50 3 49 3 49 2</gml:posList>"
    "</gml:LinearRing>"
    "</gml:exterior>"
    "</gml:Polygon>"
    "</fes:Intersects>"
    "</fes:Filter>" );
}

#endif


QTEST_MAIN( TestQgsOgcUtils )
#include "testqgsogcutils.moc"
