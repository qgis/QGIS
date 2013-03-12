
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
  QCOMPARE( doc.toString( -1 ), QString( "<gml:Point><gml:coordinates cs=\",\" ts=\" \">111.0,222.0</gml:coordinates></gml:Point>" ) );
  doc.removeChild( elemPoint );

  QDomElement elemLine = QgsOgcUtils::geometryToGML( geomLine, doc );
  QVERIFY( !elemLine.isNull() );

  doc.appendChild( elemLine );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:LineString><gml:coordinates cs=\",\" ts=\" \">111.0,222.0 222.0,222.0</gml:coordinates></gml:LineString>" ) );
  doc.removeChild( elemLine );

  // Test GML3
  elemInvalid = QgsOgcUtils::geometryToGML( 0, doc, "GML3" );
  QVERIFY( elemInvalid.isNull() );

  elemPoint = QgsOgcUtils::geometryToGML( geomPoint, doc, "GML3" );
  QVERIFY( !elemPoint.isNull() );

  doc.appendChild( elemPoint );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:Point><gml:pos srsDimension=\"2\">111.0 222.0</gml:pos></gml:Point>" ) );
  doc.removeChild( elemPoint );

  elemLine = QgsOgcUtils::geometryToGML( geomLine, doc, "GML3" );
  QVERIFY( !elemLine.isNull() );

  doc.appendChild( elemLine );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:LineString><gml:posList srsDimension=\"2\">111.0 222.0 222.0 222.0</gml:posList></gml:LineString>" ) );
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

  QTest::newRow( "bbox" ) << QString(
    "<Filter>"
    "<BBOX><PropertyName>Name>NAME</PropertyName><gml:Box srsName='foo'>"
    "<gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box></BBOX>"
    "</Filter>" )
  << QString( "bbox($geometry, geomFromGML2('<Box srsName=\"foo\"><coordinates>135.2239,34.4879 135.8578,34.8471</coordinates></Box>'))" );

  QTest::newRow( "Intersects" ) << QString(
    "<Filter>"
    "<Intersects>"
    "<PropertyName>GEOMETRY</PropertyName>"
    "<gml:Point>"
    "<gml:coordinates>123,456</gml:coordinates>"
    "</gml:Point>"
    "</Intersects>"
    "</Filter>" )
  << QString( "intersects($geometry, geomFromGML2('<Point><coordinates>123,456</coordinates></Point>'))" );
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
  qDebug( "EXPR-DUMP: %s", expr->dump().toAscii().data() );

  if ( expr->hasParserError() )
    qDebug( "ERROR: %s ", expr->parserErrorString().toAscii().data() );
  QVERIFY( !expr->hasParserError() );

  QCOMPARE( dumpText, expr->dump() );

  delete expr;
}


QTEST_MAIN( TestQgsOgcUtils )
#include "moc_testqgsogcutils.cxx"
