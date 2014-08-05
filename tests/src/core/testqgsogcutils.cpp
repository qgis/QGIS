
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

#include <QtTest>

//qgis includes...
#include <qgsgeometry.h>
#include <qgsogcutils.h>


/** \ingroup UnitTests
 * This is a unit test for OGC utilities
 */
class TestQgsOgcUtils : public QObject
{
    Q_OBJECT
  private slots:

    void testGeometryFromGML();
    void testGeometryToGML();

    void testExpressionFromOgcFilter();
    void testExpressionFromOgcFilter_data();

    void testExpressionToOgcFilter();
    void testExpressionToOgcFilter_data();
};


void TestQgsOgcUtils::testGeometryFromGML()
{
  // Test GML2
  QgsGeometry* geom = QgsOgcUtils::geometryFromGML( "<Point><coordinates>123,456</coordinates></Point>" );
  QVERIFY( geom );
  QVERIFY( geom->wkbType() == QGis::WKBPoint );
  QVERIFY( geom->asPoint() == QgsPoint( 123, 456 ) );

  QgsGeometry* geomBox = QgsOgcUtils::geometryFromGML( "<gml:Box srsName=\"foo\"><gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box>" );
  QVERIFY( geomBox );
  QVERIFY( geomBox->wkbType() == QGis::WKBPolygon );

  // Test GML3
  geom = QgsOgcUtils::geometryFromGML( "<Point><pos>123 456</pos></Point>" );
  QVERIFY( geom );
  QVERIFY( geom->wkbType() == QGis::WKBPoint );
  QVERIFY( geom->asPoint() == QgsPoint( 123, 456 ) );

  geomBox = QgsOgcUtils::geometryFromGML( "<gml:Envelope srsName=\"foo\"><gml:lowerCorner>135.2239 34.4879</gml:lowerCorner><gml:upperCorner>135.8578 34.8471</gml:upperCorner></gml:Envelope>" );
  QVERIFY( geomBox );
  QVERIFY( geomBox->wkbType() == QGis::WKBPolygon );

  delete geom;
  delete geomBox;
}

void TestQgsOgcUtils::testGeometryToGML()
{
  QDomDocument doc;
  QgsGeometry* geomPoint = QgsGeometry::fromPoint( QgsPoint( 111, 222 ) );
  QgsGeometry* geomLine = QgsGeometry::fromWkt( "LINESTRING(111 222, 222 222)" );

  // Test GML2
  QDomElement elemInvalid = QgsOgcUtils::geometryToGML( 0, doc );
  QVERIFY( elemInvalid.isNull() );

  QDomElement elemPoint = QgsOgcUtils::geometryToGML( geomPoint, doc );
  QVERIFY( !elemPoint.isNull() );

  doc.appendChild( elemPoint );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:Point><gml:coordinates cs=\",\" ts=\" \">111,222</gml:coordinates></gml:Point>" ) );
  doc.removeChild( elemPoint );

  QDomElement elemLine = QgsOgcUtils::geometryToGML( geomLine, doc );
  QVERIFY( !elemLine.isNull() );

  doc.appendChild( elemLine );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:LineString><gml:coordinates cs=\",\" ts=\" \">111,222 222,222</gml:coordinates></gml:LineString>" ) );
  doc.removeChild( elemLine );

  // Test GML3
  elemInvalid = QgsOgcUtils::geometryToGML( 0, doc, "GML3" );
  QVERIFY( elemInvalid.isNull() );

  elemPoint = QgsOgcUtils::geometryToGML( geomPoint, doc, "GML3" );
  QVERIFY( !elemPoint.isNull() );

  doc.appendChild( elemPoint );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:Point><gml:pos srsDimension=\"2\">111 222</gml:pos></gml:Point>" ) );
  doc.removeChild( elemPoint );

  elemLine = QgsOgcUtils::geometryToGML( geomLine, doc, "GML3" );
  QVERIFY( !elemLine.isNull() );

  doc.appendChild( elemLine );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:LineString><gml:posList srsDimension=\"2\">111 222 222 222</gml:posList></gml:LineString>" ) );
  doc.removeChild( elemLine );

  delete geomPoint;
  delete geomLine;
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
  << QString( "bbox($geometry, geomFromGML('<Box srsName=\"foo\"><coordinates>135.2239,34.4879 135.8578,34.8471</coordinates></Box>'))" );

  QTest::newRow( "Intersects" ) << QString(
    "<Filter>"
    "<Intersects>"
    "<PropertyName>GEOMETRY</PropertyName>"
    "<gml:Point>"
    "<gml:coordinates>123,456</gml:coordinates>"
    "</gml:Point>"
    "</Intersects>"
    "</Filter>" )
  << QString( "intersects($geometry, geomFromGML('<Point><coordinates>123,456</coordinates></Point>'))" );
}

void TestQgsOgcUtils::testExpressionFromOgcFilter()
{
  QFETCH( QString, xmlText );
  QFETCH( QString, dumpText );

  QDomDocument doc;
  QVERIFY( doc.setContent( xmlText, true ) );
  QDomElement rootElem = doc.documentElement();

  QgsExpression* expr = QgsOgcUtils::expressionFromOgcFilter( rootElem );
  QVERIFY( expr );

  qDebug( "OGC XML  : %s", xmlText.toAscii().data() );
  qDebug( "EXPR-DUMP: %s", expr->expression().toAscii().data() );

  if ( expr->hasParserError() )
    qDebug( "ERROR: %s ", expr->parserErrorString().toAscii().data() );
  QVERIFY( !expr->hasParserError() );

  QCOMPARE( dumpText, expr->expression() );

  delete expr;
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

  QTest::newRow( "is null" ) << QString( "X IS NULL" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
    "<ogc:PropertyIsNull>"
    "<ogc:PropertyName>X</ogc:PropertyName>"
    "</ogc:PropertyIsNull>"
    "</ogc:Filter>" );

  QTest::newRow( "is not null" ) << QString( "X IS NOT NULL" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
    "<ogc:Not>"
    "<ogc:PropertyIsNull>"
    "<ogc:PropertyName>X</ogc:PropertyName>"
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

  QTest::newRow( "intersects + wkt" ) << QString( "intersects($geometry, geomFromWKT('POINT (5 6)'))" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
    "<ogc:Intersects>"
    "<ogc:PropertyName>geometry</ogc:PropertyName>"
    "<gml:Point><gml:coordinates cs=\",\" ts=\" \">5,6</gml:coordinates></gml:Point>"
    "</ogc:Intersects>"
    "</ogc:Filter>" );

  QTest::newRow( "contains + gml" ) << QString( "contains($geometry, geomFromGML('<Point><coordinates cs=\",\" ts=\" \">5,6</coordinates></Point>'))" ) << QString(
    "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
    "<ogc:Contains>"
    "<ogc:PropertyName>geometry</ogc:PropertyName>"
    "<Point><coordinates cs=\",\" ts=\" \">5,6</coordinates></Point>"
    "</ogc:Contains>"
    "</ogc:Filter>" );

  /*
  QTest::newRow( "bbox with GML3 Envelope" )
  << QString( "bbox($geometry, geomFromGML('<gml:Envelope><gml:lowerCorner>13.0983 31.5899</gml:lowerCorner><gml:upperCorner>35.5472 42.8143</gml:upperCorner></gml:Envelope>'))" )
  << QString(
  "<ogc:Filter>"
    "<ogc:BBOX>"
      "<ogc:PropertyName>Geometry</ogc:PropertyName>"
        "<gml:Envelope>"
        "<gml:lowerCorner>13.0983 31.5899</gml:lowerCorner>"
        "<gml:upperCorner>35.5472 42.8143</gml:upperCorner>"
      "</gml:Envelope>"
    "</ogc:BBOX>"
  "</ogc:Filter>" );
  */
}


QTEST_MAIN( TestQgsOgcUtils )
#include "testqgsogcutils.moc"
