
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

#include <memory>

#include "qgstest.h"

//qgis includes...
#include "qgsgeometry.h"
#include "qgsogcutils.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgstestutils.h"

#include <QRegularExpression>

/**
 * \ingroup UnitTests
 * This is a unit test for OGC utilities
 */
class TestQgsOgcUtils : public QObject
{
    Q_OBJECT
  private slots:

    void initTestCase()
    {
      // Needed on Qt 5 so that the serialization of XML is consistent among all executions
      qSetGlobalQHashSeed( 0 );

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

    void testGeometryFromGMLWithZ_data();
    void testGeometryFromGMLWithZ();

    void testGeometryToGML();

    void testGeometryZToGML();
    void testGeometryZToGML_data();

    void testExpressionFromOgcFilter();
    void testExpressionFromOgcFilter_data();

    void testExpressionFromOgcFilterWithLongLong();
    void testExpressionFromOgcFilterWithLongLong_data();

    void testExpressionFromOgcFilterWFS20();
    void testExpressionFromOgcFilterWFS20_data();

    void testExpressionToOgcFilter();
    void testExpressionToOgcFilter_data();

    void testExpressionToOgcFilterWFS11();
    void testExpressionToOgcFilterWFS11_data();

    void testExpressionToOgcFilterWFS20();
    void testExpressionToOgcFilterWFS20_data();

    void testExpressionToOgcFilterWithXPath();

    void testSQLStatementToOgcFilterWithXPath();

    void testSQLStatementToOgcFilter();
    void testSQLStatementToOgcFilter_data();

    void testParseCrsName();
    void testParseCrsName_data();
};


void TestQgsOgcUtils::testGeometryFromGML()
{
  // Test GML2
  QgsGeometry geom( QgsOgcUtils::geometryFromGML( u"<Point><coordinates>123,456</coordinates></Point>"_s ) );
  QVERIFY( !geom.isNull() );
  QVERIFY( geom.wkbType() == Qgis::WkbType::Point );
  QVERIFY( geom.asPoint() == QgsPointXY( 123, 456 ) );

  QgsGeometry geomBox( QgsOgcUtils::geometryFromGML( u"<gml:Box srsName=\"foo\"><gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box>"_s ) );
  QVERIFY( !geomBox.isNull() );
  QVERIFY( geomBox.wkbType() == Qgis::WkbType::Polygon );

  // Test point GML2 with EPSG:4326
  // X/Y coordinates are not inverted
  geom = QgsOgcUtils::geometryFromGML( u"<gml:Point srsName=\"EPSG:4326\"><gml:coordinates>4,45</gml:coordinates></gml:Point>"_s );
  QVERIFY( !geom.isNull() );
  QVERIFY( geom.wkbType() == Qgis::WkbType::Point );
  QVERIFY( geom.equals( QgsGeometry::fromWkt( u"POINT (4 45)"_s ) ) );


  // Test GML3
  geom = QgsOgcUtils::geometryFromGML( u"<Point><pos>123 456</pos></Point>"_s );
  QVERIFY( !geom.isNull() );
  QVERIFY( geom.wkbType() == Qgis::WkbType::Point );
  QVERIFY( geom.asPoint() == QgsPointXY( 123, 456 ) );

  geomBox = QgsOgcUtils::geometryFromGML( u"<gml:Envelope srsName=\"foo\"><gml:lowerCorner>135.2239 34.4879</gml:lowerCorner><gml:upperCorner>135.8578 34.8471</gml:upperCorner></gml:Envelope>"_s );
  QVERIFY( !geomBox.isNull() );
  QVERIFY( geomBox.wkbType() == Qgis::WkbType::Polygon );

  // Test point GML3 Z
  geom = QgsOgcUtils::geometryFromGML( u"<gml:Point srsName=\"EPSG:4326\"><gml:pos srsDimension=\"3\">0 1 2</gml:pos></gml:Point>"_s );
  QVERIFY( !geom.isNull() );
  QVERIFY( geom.wkbType() == Qgis::WkbType::PointZ );
  QVERIFY( geom.equals( QgsGeometry::fromWkt( u"POINTZ(0 1 2)"_s ) ) );

  // Test polygon GML3 Z
  geom = QgsOgcUtils::geometryFromGML( QStringLiteral( R"GML(<gml:Polygon srsName="EPSG:4326"><gml:exterior><gml:LinearRing><gml:posList srsDimension="3">0 0 1200 0 1 1250 1 1 1230 1 0 1210 0 0 1200</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon>)GML" ) );
  QVERIFY( !geom.isNull() );
  QVERIFY( geom.wkbType() == Qgis::WkbType::PolygonZ );
  QVERIFY( geom.equals( QgsGeometry::fromWkt( u"POLYGONZ((0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210, 0 0 1200))"_s ) ) );

  // Test linestring GML3 Z
  geom = QgsOgcUtils::geometryFromGML( QStringLiteral( R"GML(<gml:LineString srsName="EPSG:4326"><gml:posList srsDimension="3">0 0 1200 0 1 1250 1 1 1230 1 0 1210</gml:posList></gml:LineString>)GML" ) );
  QVERIFY( !geom.isNull() );
  QVERIFY( geom.wkbType() == Qgis::WkbType::LineStringZ );
  QVERIFY( geom.equals( QgsGeometry::fromWkt( u"LINESTRINGZ(0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210)"_s ) ) );

  // Test point GML3 with urn:ogc:def:crs:EPSG::4326
  // X/Y coordinates are inverted
  geom = QgsOgcUtils::geometryFromGML( u"<gml:Point srsName=\"urn:ogc:def:crs:EPSG::4326\"><gml:pos>45 4</gml:pos></gml:Point>"_s );
  QVERIFY( !geom.isNull() );
  QVERIFY( geom.wkbType() == Qgis::WkbType::Point );
  QVERIFY( geom.equals( QgsGeometry::fromWkt( u"POINT (4 45)"_s ) ) );


  // Test point GML3 with urn:ogc:def:crs:EPSG::3857
  // X/Y coordinates are not inverted
  geom = QgsOgcUtils::geometryFromGML( u"<gml:Point srsName=\"urn:ogc:def:crs:EPSG::3857\"><gml:pos>32 2</gml:pos></gml:Point>"_s );
  QVERIFY( !geom.isNull() );
  QVERIFY( geom.wkbType() == Qgis::WkbType::Point );
  QVERIFY( geom.equals( QgsGeometry::fromWkt( u"POINT (32 2)"_s ) ) );
}

void TestQgsOgcUtils::testGeometryFromGMLWithZ_data()
{
  QTest::addColumn<QString>( "xmlText" );
  QTest::addColumn<Qgis::WkbType>( "type" );
  QTest::addColumn<QString>( "WKT" );

  QTest::newRow( "PointZ" )
    << u"<gml:Point srsName=\"EPSG:4326\"><gml:pos srsDimension=\"3\">0 1 2</gml:pos></gml:Point>"_s
    << Qgis::WkbType::PointZ
    << u"POINTZ( 0 1 2)"_s;

  QTest::newRow( "LineStringZ" )
    << QStringLiteral( R"GML(<gml:LineString srsName="EPSG:4326"><gml:posList srsDimension="3">0 0 1200 0 1 1250 1 1 1230 1 0 1210</gml:posList></gml:LineString>)GML" )
    << Qgis::WkbType::LineStringZ
    << u"LINESTRINGZ(0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210)"_s;

  QTest::newRow( "PolygonZ" )
    << QStringLiteral( R"GML(<gml:Polygon srsName="EPSG:4326"><gml:exterior><gml:LinearRing><gml:posList srsDimension="3">0 0 1200 0 1 1250 1 1 1230 1 0 1210 0 0 1200</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon>)GML" )
    << Qgis::WkbType::PolygonZ
    << u"POLYGONZ((0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210, 0 0 1200))"_s;

  // Test multipoint GML3 Z
  QTest::newRow( "MultiPointZ" )
    << QStringLiteral( R"GML(<gml:MultiPoint srsName="EPSG:4326"><gml:pointMember><gml:Point><gml:pos srsDimension="3">0 1 2</gml:pos></gml:Point></gml:pointMember><gml:pointMember><gml:Point><gml:pos srsDimension="3">3 4 5</gml:pos></gml:Point></gml:pointMember></gml:MultiPoint>)GML" )
    << Qgis::WkbType::MultiPointZ
    << u"MULTIPOINTZ((0 1 2), (3 4 5))"_s;

  // Test multilinestring GML2 Z
  QTest::newRow( "MultiLineStringZ GML2" )
    << QStringLiteral( R"GML(<gml:MultiLineString srsName="EPSG:4326"><gml:lineStringMember><gml:LineString><gml:coordinates>0,0,1200 0,1,1250 1,1,1230 1,0,1210</gml:coordinates></gml:LineString></gml:lineStringMember><gml:lineStringMember><gml:LineString><gml:coordinates>2,2,2200 2,3,2250 3,3,2230 3,2,2210</gml:coordinates></gml:LineString></gml:lineStringMember></gml:MultiLineString>)GML" )
    << Qgis::WkbType::MultiLineStringZ
    << u"MULTILINESTRINGZ((0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210), (2 2 2200, 2 3 2250, 3 3 2230, 3 2 2210))"_s;

  QTest::newRow( "MultiLineStringZ no curve" )
    << QStringLiteral( R"GML(<gml:MultiCurve srsName="EPSG:4326"><gml:curveMember><gml:LineString><gml:posList srsDimension="3">0 0 1200 0 1 1250 1 1 1230 1 0 1210</gml:posList></gml:LineString></gml:curveMember><gml:curveMember><gml:LineString><gml:posList srsDimension="3">2 2 2200 2 3 2250 3 3 2230 3 2 2210</gml:posList></gml:LineString></gml:curveMember></gml:MultiCurve>)GML" )
    << Qgis::WkbType::MultiLineStringZ
    << u"MULTILINESTRINGZ((0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210), (2 2 2200, 2 3 2250, 3 3 2230, 3 2 2210))"_s;

  // Test multilinestring GML3 Z
  QTest::newRow( "MultiLineStringZ" )
    << QStringLiteral( R"GML(<gml:MultiCurve srsName="EPSG:4326"><gml:curveMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension="3">0 0 1200 0 1 1250 1 1 1230 1 0 1210</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:curveMember><gml:curveMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension="3">2 2 2200 2 3 2250 3 3 2230 3 2 2210</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:curveMember></gml:MultiCurve>)GML" )
    << Qgis::WkbType::MultiLineStringZ
    << u"MULTILINESTRINGZ((0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210), (2 2 2200, 2 3 2250, 3 3 2230, 3 2 2210))"_s;
}


void TestQgsOgcUtils::testGeometryFromGMLWithZ()
{
  QFETCH( QString, xmlText );
  QFETCH( Qgis::WkbType, type );
  QFETCH( QString, WKT );

  QgsGeometry geom = QgsOgcUtils::geometryFromGML( xmlText );
  QVERIFY( !geom.isNull() );
  QCOMPARE( geom.wkbType(), type );
  QVERIFY( geom.equals( QgsGeometry::fromWkt( WKT ) ) );
}

static QDomElement comparableElement( const QString &xmlText )
{
  QDomDocument doc;
  if ( !doc.setContent( xmlText ) )
    return QDomElement();
  return doc.documentElement();
}


void TestQgsOgcUtils::testGeometryToGML()
{
  QDomDocument doc;
  const QgsGeometry geomPoint( QgsGeometry::fromPointXY( QgsPointXY( 111, 222 ) ) );
  const QgsGeometry geomLine( QgsGeometry::fromWkt( u"LINESTRING(111 222, 222 222)"_s ) );

  // Elements to compare
  QDomElement xmlElem;
  QDomElement ogcElem;

  // Test GML2
  QDomElement elemInvalid = QgsOgcUtils::geometryToGML( QgsGeometry(), doc );
  QVERIFY( elemInvalid.isNull() );

  QDomElement elemPoint = QgsOgcUtils::geometryToGML( geomPoint, doc );
  QVERIFY( !elemPoint.isNull() );

  doc.appendChild( elemPoint );
  xmlElem = comparableElement( u"<gml:Point><gml:coordinates ts=\" \" cs=\",\">111,222</gml:coordinates></gml:Point>"_s );
  ogcElem = comparableElement( doc.toString( -1 ) );
  QVERIFY( QgsTestUtils::compareDomElements( xmlElem, ogcElem ) );
  doc.removeChild( elemPoint );

  QDomElement elemLine = QgsOgcUtils::geometryToGML( geomLine, doc );
  QVERIFY( !elemLine.isNull() );

  doc.appendChild( elemLine );
  xmlElem = comparableElement( u"<gml:LineString><gml:coordinates ts=\" \" cs=\",\">111,222 222,222</gml:coordinates></gml:LineString>"_s );
  ogcElem = comparableElement( doc.toString( -1 ) );
  QVERIFY( QgsTestUtils::compareDomElements( xmlElem, ogcElem ) );
  doc.removeChild( elemLine );

  // Test GML3
  elemInvalid = QgsOgcUtils::geometryToGML( QgsGeometry(), doc, u"GML3"_s );
  QVERIFY( elemInvalid.isNull() );

  elemPoint = QgsOgcUtils::geometryToGML( geomPoint, doc, u"GML3"_s );
  QVERIFY( !elemPoint.isNull() );

  doc.appendChild( elemPoint );
  xmlElem = comparableElement( u"<gml:Point><gml:pos srsDimension=\"2\">111 222</gml:pos></gml:Point>"_s );
  ogcElem = comparableElement( doc.toString( -1 ) );
  QVERIFY( QgsTestUtils::compareDomElements( xmlElem, ogcElem ) );
  doc.removeChild( elemPoint );

  elemLine = QgsOgcUtils::geometryToGML( geomLine, doc, u"GML3"_s );
  QVERIFY( !elemLine.isNull() );

  doc.appendChild( elemLine );
  xmlElem = comparableElement( u"<gml:LineString><gml:posList srsDimension=\"2\">111 222 222 222</gml:posList></gml:LineString>"_s );
  ogcElem = comparableElement( doc.toString( -1 ) );
  QVERIFY( QgsTestUtils::compareDomElements( xmlElem, ogcElem ) );
  doc.removeChild( elemLine );
}

void TestQgsOgcUtils::testGeometryZToGML_data()
{
  QTest::addColumn<QString>( "wkt" );

  QTest::newRow( "PointZ" ) << u"POINT Z(0 1 2)"_s;
  QTest::newRow( "LineStringZ" ) << u"LINESTRING Z(0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210)"_s;
  QTest::newRow( "PolygonZ" ) << u"POLYGON Z((0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210, 0 0 1200))"_s;

  // Multi
  QTest::newRow( "MultiPointZ" ) << u"MULTIPOINT Z((0 1 2), (3 4 5))"_s;
  QTest::newRow( "MultiLineStringZ" ) << u"MULTILINESTRING Z((0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210), (2 2 2200, 2 3 2250, 3 3 2230, 3 2 2210))"_s;
  QTest::newRow( "MultiPolygonZ" ) << u"MULTIPOLYGON Z(((0 0 1200, 0 1 1250, 1 1 1230, 1 0 1210, 0 0 1200)), ((2 2 2200, 2 3 2250, 3 3 2230, 3 2 2210, 2 2 2200)))"_s;
}

void TestQgsOgcUtils::testGeometryZToGML()
{
  // Round trip test

  QFETCH( QString, wkt );

  const QgsGeometry geom( QgsGeometry::fromWkt( wkt ) );

  QVERIFY( !geom.isNull() );
  QVERIFY( QgsWkbTypes::hasZ( geom.wkbType() ) );

  // Test GML3
  QDomDocument doc;
  QDomElement elem = QgsOgcUtils::geometryToGML( geom, doc, u"GML3"_s );
  QVERIFY( !elem.isNull() );

  // Dump element to string
  QString str;
  QTextStream stream( &str );
  elem.save( stream, 0 /*indent*/ );

  QCOMPARE( QgsOgcUtils::geometryFromGML( str ).asWkt(), geom.asWkt() );

  //  Test GML2
  elem = QgsOgcUtils::geometryToGML( geom, doc, u"GML2"_s );
  QVERIFY( !elem.isNull() );
  str.clear();
  elem.save( stream, 0 /*indent*/ );
  QCOMPARE( QgsOgcUtils::geometryFromGML( str ).asWkt(), geom.asWkt() );
}

void TestQgsOgcUtils::testExpressionFromOgcFilterWFS20_data()
{
  QTest::addColumn<QString>( "xmlText" );
  QTest::addColumn<QString>( "dumpText" );

  QTest::newRow( "=" ) << QString(
    "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">"
    "<fes:PropertyIsEqualTo>"
    "<fes:ValueReference>NAME</fes:ValueReference>"
    "<fes:Literal>New York</fes:Literal>"
    "</fes:PropertyIsEqualTo></fes:Filter>"
  )
                       << u"NAME = 'New York'"_s;

  QTest::newRow( "bbox coordinates" ) << QString(
    "<Filter>"
    "<BBOX><ValueReference>Name>NAME</ValueReference><gml:Box srsName='foo'>"
    "<gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box></BBOX>"
    "</Filter>"
  )
                                      << u"intersects_bbox($geometry, geom_from_gml('<gml:Box xmlns:gml=\"http://www.opengis.net/gml\" srsName=\"foo\"><gml:coordinates xmlns:gml=\"http://www.opengis.net/gml\">135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box>'))"_s;

  QTest::newRow( "bbox corner" )
    << QString(
         "<fes:Filter>"
         "<fes:BBOX>"
         "<fes:ValueReference>my_geometry_name</fes:ValueReference>"
         "<gml:Envelope>"
         "<gml:lowerCorner>49 2</gml:lowerCorner>"
         "<gml:upperCorner>50 3</gml:upperCorner>"
         "</gml:Envelope>"
         "</fes:BBOX>"
         "</fes:Filter>"
       )
    << u"intersects_bbox($geometry, geom_from_gml('<gml:Envelope xmlns:gml=\"http://www.opengis.net/gml\"><gml:lowerCorner xmlns:gml=\"http://www.opengis.net/gml\">49 2</gml:lowerCorner><gml:upperCorner xmlns:gml=\"http://www.opengis.net/gml\">50 3</gml:upperCorner></gml:Envelope>'))"_s;
}

void TestQgsOgcUtils::testExpressionFromOgcFilterWFS20()
{
  QFETCH( QString, xmlText );
  QFETCH( QString, dumpText );

  QDomDocument doc;
  // wrap the string into a root tag to have "gml" and "fes" namespaces
  const QString xml = u"<tmp xmlns:gml=\"http://www.opengis.net/gml\" xmlns:fes=\"http://www.opengis.net/fes/2.0\">%1</tmp>"_s.arg( xmlText );
  QVERIFY( doc.setContent( xml, true ) );
  const QDomElement rootElem = doc.documentElement().firstChildElement();

  QgsVectorLayer layer( "Point?crs=epsg:4326&field=LITERAL:string(20)", "temp", "memory" );

  std::unique_ptr<QgsExpression> expr( QgsOgcUtils::expressionFromOgcFilter( rootElem, QgsOgcUtils::FILTER_FES_2_0, &layer ) );
  QVERIFY( expr.get() );

  //qDebug( "OGC XML  : %s", xmlText.toLatin1().data() );
  //qDebug( "EXPR-DUMP: %s", expr->expression().toLatin1().data() );

  if ( expr->hasParserError() )
    qDebug( "ERROR: %s ", expr->parserErrorString().toLatin1().data() );
  QVERIFY( !expr->hasParserError() );

  QCOMPARE( dumpText, expr->expression() );
}

void TestQgsOgcUtils::testExpressionFromOgcFilter_data()
{
  QTest::addColumn<QString>( "xmlText" );
  QTest::addColumn<QString>( "dumpText" );

  QTest::newRow( "=" ) << QString(
    "<Filter><PropertyIsEqualTo>"
    "<PropertyName>NAME</PropertyName>"
    "<Literal>New York</Literal>"
    "</PropertyIsEqualTo></Filter>"
  )
                       << u"NAME = 'New York'"_s;

  QTest::newRow( ">" ) << QString(
    "<Filter><PropertyIsGreaterThan>"
    "<PropertyName>COUNT</PropertyName>"
    "<Literal>3</Literal>"
    "</PropertyIsGreaterThan></Filter>"
  )
                       << u"COUNT > 3"_s;

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
    "</ogc:Filter>"
  )
                         << u"pop >= 50000 AND pop < 100000"_s;

  // TODO: should work also without <Literal> tags in Lower/Upper-Boundary tags?
  QTest::newRow( "between" ) << QString(
    "<Filter>"
    "<PropertyIsBetween><PropertyName>POPULATION</PropertyName>"
    "<LowerBoundary><Literal>100</Literal></LowerBoundary>"
    "<UpperBoundary><Literal>200</Literal></UpperBoundary></PropertyIsBetween>"
    "</Filter>"
  )
                             << u"POPULATION >= 100 AND POPULATION <= 200"_s;

  // handle different wildcards, single chars, escape chars
  QTest::newRow( "like" ) << QString(
    "<Filter>"
    "<PropertyIsLike wildCard=\"%\" singleChar=\"_\" escape=\"\\\">"
    "<PropertyName>NAME</PropertyName><Literal>*QGIS*</Literal></PropertyIsLike>"
    "</Filter>"
  )
                          << u"NAME LIKE '*QGIS*'"_s;
  QTest::newRow( "ilike" ) << QString(
    "<Filter>"
    "<PropertyIsLike matchCase=\"false\" wildCard=\"%\" singleChar=\"_\" escape=\"\\\">"
    "<PropertyName>NAME</PropertyName><Literal>*QGIS*</Literal></PropertyIsLike>"
    "</Filter>"
  )
                           << u"NAME ILIKE '*QGIS*'"_s;

  // different wildCards
  QTest::newRow( "like wildCard simple" ) << QString(
    "<Filter>"
    "<PropertyIsLike wildCard='*' singleChar='.' escape=\"\\\">"
    "<PropertyName>NAME</PropertyName><Literal>*QGIS*</Literal></PropertyIsLike>"
    "</Filter>"
  )
                                          << u"NAME LIKE '%QGIS%'"_s;

  QTest::newRow( "like wildCard complex" ) << QString(
    "<Filter>"
    "<PropertyIsLike wildCard='*' singleChar='.' escape=\"\\\">"
    "<PropertyName>NAME</PropertyName><Literal>*%QGIS*\\*</Literal></PropertyIsLike>"
    "</Filter>"
  )
                                           << u"NAME LIKE '%\\\\%QGIS%*'"_s;

  QTest::newRow( "ilike wildCard simple" ) << QString(
    "<Filter>"
    "<PropertyIsLike matchCase=\"false\" wildCard='*' singleChar='.' escape=\"\\\">"
    "<PropertyName>NAME</PropertyName><Literal>*QGIS*</Literal></PropertyIsLike>"
    "</Filter>"
  )
                                           << u"NAME ILIKE '%QGIS%'"_s;

  QTest::newRow( "ilike wildCard complex" ) << QString(
    "<Filter>"
    "<PropertyIsLike matchCase=\"false\"  wildCard='*' singleChar='.' escape=\"\\\">"
    "<PropertyName>NAME</PropertyName><Literal>*%QGIS*\\*</Literal></PropertyIsLike>"
    "</Filter>"
  )
                                            << u"NAME ILIKE '%\\\\%QGIS%*'"_s;

  // different single chars
  QTest::newRow( "like single char" ) << QString(
    "<Filter>"
    "<PropertyIsLike wildCard='*' singleChar='.' escape=\"\\\">"
    "<PropertyName>NAME</PropertyName><Literal>._QGIS.\\.</Literal></PropertyIsLike>"
    "</Filter>"
  )
                                      << u"NAME LIKE '_\\\\_QGIS_.'"_s;
  // different escape chars
  QTest::newRow( "like escape char" ) << QString(
    "<Filter>"
    "<PropertyIsLike wildCard=\"*\" singleChar=\".\" escape=\"!\">"
    "<PropertyName>NAME</PropertyName><Literal>_QGIS.!.!!%QGIS*!*</Literal></PropertyIsLike>"
    "</Filter>"
  )
                                      << u"NAME LIKE '\\\\_QGIS_.!\\\\%QGIS%*'"_s;

  QTest::newRow( "like escape char" ) << QString(
    "<Filter>"
    "<PropertyIsLike wildCard=\"*\" singleChar=\".\" escapeChar=\"!\">"
    "<PropertyName>NAME</PropertyName><Literal>_QGIS.!.!!%QGIS*!*</Literal></PropertyIsLike>"
    "</Filter>"
  )
                                      << u"NAME LIKE '\\\\_QGIS_.!\\\\%QGIS%*'"_s;

  QTest::newRow( "is null" ) << QString(
    "<Filter>"
    "<ogc:PropertyIsNull>"
    "<ogc:PropertyName>FIRST_NAME</ogc:PropertyName>"
    "</ogc:PropertyIsNull>"
    "</Filter>"
  )
                             << u"FIRST_NAME IS NULL"_s;

  QTest::newRow( "bbox with GML2 Box" ) << QString(
    "<Filter>"
    "<BBOX><PropertyName>Name>NAME</PropertyName><gml:Box srsName='foo'>"
    "<gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box></BBOX>"
    "</Filter>"
  )
                                        << u"intersects_bbox($geometry, geom_from_gml('<gml:Box xmlns:gml=\"http://www.opengis.net/gml\" srsName=\"foo\"><gml:coordinates xmlns:gml=\"http://www.opengis.net/gml\">135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box>'))"_s;

  QTest::newRow( "Intersects" ) << QString(
    "<Filter>"
    "<Intersects>"
    "<PropertyName>GEOMETRY</PropertyName>"
    "<gml:Point>"
    "<gml:coordinates>123,456</gml:coordinates>"
    "</gml:Point>"
    "</Intersects>"
    "</Filter>"
  )
                                << u"intersects($geometry, geom_from_gml('<gml:Point xmlns:gml=\"http://www.opengis.net/gml\"><gml:coordinates xmlns:gml=\"http://www.opengis.net/gml\">123,456</gml:coordinates></gml:Point>'))"_s;

  QTest::newRow( "Literal conversion" ) << QString(
    "<Filter><PropertyIsEqualTo>"
    "<PropertyName>LITERAL</PropertyName>"
    "<Literal>+2</Literal>"
    "</PropertyIsEqualTo></Filter>"
  )
                                        << u"LITERAL = '+2'"_s;

  QTest::newRow( "not or list" ) << QStringLiteral( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                    "<ogc:Not>"
                                                    " <ogc:Or>"
                                                    "  <ogc:PropertyIsEqualTo>"
                                                    "   <ogc:PropertyName>A</ogc:PropertyName>"
                                                    "   <ogc:Literal>1</ogc:Literal>"
                                                    "  </ogc:PropertyIsEqualTo>"
                                                    "  <ogc:PropertyIsEqualTo>"
                                                    "   <ogc:PropertyName>A</ogc:PropertyName>"
                                                    "   <ogc:Literal>2</ogc:Literal>"
                                                    "  </ogc:PropertyIsEqualTo>"
                                                    "  <ogc:PropertyIsEqualTo>"
                                                    "   <ogc:PropertyName>A</ogc:PropertyName>"
                                                    "   <ogc:Literal>3</ogc:Literal>"
                                                    "  </ogc:PropertyIsEqualTo>"
                                                    " </ogc:Or>"
                                                    "</ogc:Not>"
                                                    "</ogc:Filter>" )
                                 << u"NOT ( A = 1 OR A = 2 OR A = 3 )"_s;
}

void TestQgsOgcUtils::testExpressionFromOgcFilter()
{
  QFETCH( QString, xmlText );
  QFETCH( QString, dumpText );

  QDomDocument doc;
  // wrap the string into a root tag to have "gml" and "ogc" namespaces
  const QString xml = u"<tmp xmlns:gml=\"http://www.opengis.net/gml\" xmlns:ogc=\"http://www.opengis.net/ogc\">%1</tmp>"_s.arg( xmlText );
  QVERIFY( doc.setContent( xml, true ) );
  const QDomElement rootElem = doc.documentElement().firstChildElement();

  QgsVectorLayer layer( "Point?crs=epsg:4326&field=LITERAL:string(20)", "temp", "memory" );

  std::unique_ptr<QgsExpression> expr( QgsOgcUtils::expressionFromOgcFilter( rootElem, &layer ) );
  QVERIFY( expr.get() );

  //qDebug( "OGC XML  : %s", xmlText.toLatin1().data() );
  //qDebug( "EXPR-DUMP: %s", expr->expression().toLatin1().data() );

  if ( expr->hasParserError() )
    qDebug( "ERROR: %s ", expr->parserErrorString().toLatin1().data() );
  QVERIFY( !expr->hasParserError() );

  QCOMPARE( dumpText, expr->expression() );
}

void TestQgsOgcUtils::testExpressionFromOgcFilterWithLongLong_data()
{
  QTest::addColumn<QString>( "xmlText" );
  QTest::addColumn<QString>( "dumpText" );
  QTest::newRow( "Literal less than" ) << QString(
    "<Filter><And>"
    "<PropertyIsGreaterThan>"
    "<PropertyName>id</PropertyName>"
    "<Literal>1</Literal>"
    "</PropertyIsGreaterThan>"
    "<PropertyIsLessThan>"
    "<PropertyName>id</PropertyName>"
    "<Literal>3</Literal>"
    "</PropertyIsLessThan>"
    "</And></Filter>"
  )
                                       << u"id > 1 AND id < 3"_s;
}

void TestQgsOgcUtils::testExpressionFromOgcFilterWithLongLong()
{
  QFETCH( QString, xmlText );
  QFETCH( QString, dumpText );

  QDomDocument doc;

  // wrap the string into a root tag to have "gml" namespace
  const QString xml = u"<tmp xmlns:gml=\"%1\">%2</tmp>"_s.arg( u"http://www.opengis.net/gml"_s, xmlText );
  QVERIFY( doc.setContent( xml, true ) );
  const QDomElement rootElem = doc.documentElement().firstChildElement();

  QgsVectorLayer layer( "Point?crs=epsg:4326", "temp", "memory" );

  const QgsField longlongField( u"id"_s, QMetaType::Type::LongLong );

  QList<QgsField> fields;
  fields.append( longlongField );
  layer.dataProvider()->addAttributes( fields );
  layer.updateFields();

  std::unique_ptr<QgsExpression> expr( QgsOgcUtils::expressionFromOgcFilter( rootElem, &layer ) );
  QVERIFY( expr.get() );

  //qDebug( "OGC XML  : %s", xmlText.toLatin1().data() );
  //qDebug( "EXPR-DUMP: %s", expr->expression().toLatin1().data() );

  if ( expr->hasParserError() )
    qDebug( "ERROR: %s ", expr->parserErrorString().toLatin1().data() );
  QVERIFY( !expr->hasParserError() );

  QCOMPARE( dumpText, expr->expression() );
}

void TestQgsOgcUtils::testExpressionToOgcFilter()
{
  QFETCH( QString, exprText );
  QFETCH( QString, xmlText );

  const QgsExpression exp( exprText );
  QVERIFY( !exp.hasParserError() );

  QString errorMsg;
  QDomDocument doc;
  const QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( exp, doc, &errorMsg );

  if ( !errorMsg.isEmpty() )
    qDebug( "ERROR: %s", errorMsg.toLatin1().data() );

  QVERIFY( !filterElem.isNull() );

  doc.appendChild( filterElem );

  //qDebug( "EXPR: %s", exp.expression().toLatin1().data() );
  //qDebug( "OGC : %s", doc.toString( -1 ).toLatin1().data() );


  QDomElement xmlElem = comparableElement( xmlText );
  QDomElement ogcElem = comparableElement( doc.toString( -1 ) );
  QVERIFY( QgsTestUtils::compareDomElements( xmlElem, ogcElem ) );
}

void TestQgsOgcUtils::testExpressionToOgcFilter_data()
{
  QTest::addColumn<QString>( "exprText" );
  QTest::addColumn<QString>( "xmlText" );

  QTest::newRow( "=" ) << u"NAME = 'New York'"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                             "<ogc:PropertyIsEqualTo>"
                                                             "<ogc:PropertyName>NAME</ogc:PropertyName>"
                                                             "<ogc:Literal>New York</ogc:Literal>"
                                                             "</ogc:PropertyIsEqualTo></ogc:Filter>" );

  QTest::newRow( ">" ) << u"\"COUNT\" > 3"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                         "<ogc:PropertyIsGreaterThan>"
                                                         "<ogc:PropertyName>COUNT</ogc:PropertyName>"
                                                         "<ogc:Literal>3</ogc:Literal>"
                                                         "</ogc:PropertyIsGreaterThan></ogc:Filter>" );

  QTest::newRow( "and+or" ) << u"(FIELD1 = 10 OR FIELD1 = 20) AND STATUS = 'VALID'"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
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

  QTest::newRow( "like" ) << u"NAME LIKE '*QGIS*'"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                 "<ogc:PropertyIsLike singleChar=\"_\" escape=\"\\\" wildCard=\"%\">"
                                                                 "<ogc:PropertyName>NAME</ogc:PropertyName>"
                                                                 "<ogc:Literal>*QGIS*</ogc:Literal>"
                                                                 "</ogc:PropertyIsLike>"
                                                                 "</ogc:Filter>" );

  QTest::newRow( "ilike" ) << u"NAME ILIKE '*QGIS*'"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                   "<ogc:PropertyIsLike matchCase=\"false\" singleChar=\"_\" escape=\"\\\" wildCard=\"%\">"
                                                                   "<ogc:PropertyName>NAME</ogc:PropertyName>"
                                                                   "<ogc:Literal>*QGIS*</ogc:Literal>"
                                                                   "</ogc:PropertyIsLike>"
                                                                   "</ogc:Filter>" );

  QTest::newRow( "is null" ) << u"A IS NULL"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                           "<ogc:PropertyIsNull>"
                                                           "<ogc:PropertyName>A</ogc:PropertyName>"
                                                           "</ogc:PropertyIsNull>"
                                                           "</ogc:Filter>" );

  QTest::newRow( "is not null" ) << u"A IS NOT NULL"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                   "<ogc:Not>"
                                                                   "<ogc:PropertyIsNull>"
                                                                   "<ogc:PropertyName>A</ogc:PropertyName>"
                                                                   "</ogc:PropertyIsNull>"
                                                                   "</ogc:Not>"
                                                                   "</ogc:Filter>" );

  QTest::newRow( "in" ) << u"A IN (10,20,30)"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
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

  QTest::newRow( "not in" ) << u"A NOT IN (10,20,30)"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                    "<ogc:Not>"
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
                                                                    "</ogc:Not>"
                                                                    "</ogc:Filter>" );

  QTest::newRow( "in" ) << u"A IN (10)"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                      "<ogc:PropertyIsEqualTo>"
                                                      "<ogc:PropertyName>A</ogc:PropertyName>"
                                                      "<ogc:Literal>10</ogc:Literal>"
                                                      "</ogc:PropertyIsEqualTo>"
                                                      "</ogc:Filter>" );

  QTest::newRow( "not in" ) << u"A NOT IN (10)"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                              "<ogc:Not>"
                                                              "<ogc:PropertyIsEqualTo>"
                                                              "<ogc:PropertyName>A</ogc:PropertyName>"
                                                              "<ogc:Literal>10</ogc:Literal>"
                                                              "</ogc:PropertyIsEqualTo>"
                                                              "</ogc:Not>"
                                                              "</ogc:Filter>" );

  QTest::newRow( "intersects_bbox $geometry" ) << u"intersects_bbox($geometry, geomFromWKT('POINT (5 6)'))"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
                                                                                                                          "<ogc:BBOX>"
                                                                                                                          "<ogc:PropertyName>geometry</ogc:PropertyName>"
                                                                                                                          "<gml:Box><gml:coordinates ts=\" \" cs=\",\">5,6 5,6</gml:coordinates></gml:Box>"
                                                                                                                          "</ogc:BBOX>"
                                                                                                                          "</ogc:Filter>" );

  QTest::newRow( "intersects + wkt $geometry" ) << u"intersects($geometry, geomFromWKT('POINT (5 6)'))"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
                                                                                                                      "<ogc:Intersects>"
                                                                                                                      "<ogc:PropertyName>geometry</ogc:PropertyName>"
                                                                                                                      "<gml:Point><gml:coordinates ts=\" \" cs=\",\">5,6</gml:coordinates></gml:Point>"
                                                                                                                      "</ogc:Intersects>"
                                                                                                                      "</ogc:Filter>" );

  QTest::newRow( "contains + gml $geometry" ) << u"contains($geometry, geomFromGML('<Point><coordinates cs=\",\" ts=\" \">5,6</coordinates></Point>'))"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
                                                                                                                                                                      "<ogc:Contains>"
                                                                                                                                                                      "<ogc:PropertyName>geometry</ogc:PropertyName>"
                                                                                                                                                                      "<gml:Point><gml:coordinates ts=\" \" cs=\",\">5,6</gml:coordinates></gml:Point>"
                                                                                                                                                                      "</ogc:Contains>"
                                                                                                                                                                      "</ogc:Filter>" );

  QTest::newRow( "intersects_bbox @geometry" ) << u"intersects_bbox(@geometry, geomFromWKT('POINT (5 6)'))"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
                                                                                                                          "<ogc:BBOX>"
                                                                                                                          "<ogc:PropertyName>geometry</ogc:PropertyName>"
                                                                                                                          "<gml:Box><gml:coordinates ts=\" \" cs=\",\">5,6 5,6</gml:coordinates></gml:Box>"
                                                                                                                          "</ogc:BBOX>"
                                                                                                                          "</ogc:Filter>" );

  QTest::newRow( "intersects + wkt @geometry" ) << u"intersects(@geometry, geomFromWKT('POINT (5 6)'))"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
                                                                                                                      "<ogc:Intersects>"
                                                                                                                      "<ogc:PropertyName>geometry</ogc:PropertyName>"
                                                                                                                      "<gml:Point><gml:coordinates ts=\" \" cs=\",\">5,6</gml:coordinates></gml:Point>"
                                                                                                                      "</ogc:Intersects>"
                                                                                                                      "</ogc:Filter>" );

  QTest::newRow( "contains + gml @geometry" ) << u"contains(@geometry, geomFromGML('<Point><coordinates cs=\",\" ts=\" \">5,6</coordinates></Point>'))"_s << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
                                                                                                                                                                      "<ogc:Contains>"
                                                                                                                                                                      "<ogc:PropertyName>geometry</ogc:PropertyName>"
                                                                                                                                                                      "<gml:Point><gml:coordinates ts=\" \" cs=\",\">5,6</gml:coordinates></gml:Point>"
                                                                                                                                                                      "</ogc:Contains>"
                                                                                                                                                                      "</ogc:Filter>" );
}

void TestQgsOgcUtils::testExpressionToOgcFilterWFS11()
{
  QFETCH( QString, exprText );
  QFETCH( QString, srsName );
  QFETCH( QString, xmlText );

  const QgsExpression exp( exprText );
  QVERIFY( !exp.hasParserError() );

  QString errorMsg;
  QDomDocument doc;
  const QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( exp, doc, QgsOgcUtils::GML_3_1_0, QgsOgcUtils::FILTER_OGC_1_1, QString(), QString(), u"my_geometry_name"_s, srsName, true, false, &errorMsg );

  if ( !errorMsg.isEmpty() )
    qDebug( "ERROR: %s", errorMsg.toLatin1().data() );

  QVERIFY( !filterElem.isNull() );

  doc.appendChild( filterElem );

  //qDebug( "EXPR: %s", exp.expression().toLatin1().data() );
  //qDebug( "SRSNAME: %s", srsName.toLatin1().data() );
  //qDebug( "OGC : %s", doc.toString( -1 ).toLatin1().data() );


  QDomElement xmlElem = comparableElement( xmlText );
  QDomElement ogcElem = comparableElement( doc.toString( -1 ) );
  QVERIFY( QgsTestUtils::compareDomElements( xmlElem, ogcElem ) );
}

void TestQgsOgcUtils::testExpressionToOgcFilterWFS11_data()
{
  QTest::addColumn<QString>( "exprText" );
  QTest::addColumn<QString>( "srsName" );
  QTest::addColumn<QString>( "xmlText" );

  QTest::newRow( "bbox $geometry" )
    << u"intersects_bbox($geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))"_s
    << u"urn:ogc:def:crs:EPSG::4326"_s
    << QString(
         "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
         "<ogc:BBOX>"
         "<ogc:PropertyName>my_geometry_name</ogc:PropertyName>"
         "<gml:Envelope srsName=\"urn:ogc:def:crs:EPSG::4326\">"
         "<gml:lowerCorner>49 2</gml:lowerCorner>"
         "<gml:upperCorner>50 3</gml:upperCorner>"
         "</gml:Envelope>"
         "</ogc:BBOX>"
         "</ogc:Filter>"
       );

  QTest::newRow( "bbox @geometry" )
    << u"intersects_bbox(@geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))"_s
    << u"urn:ogc:def:crs:EPSG::4326"_s
    << QString(
         "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
         "<ogc:BBOX>"
         "<ogc:PropertyName>my_geometry_name</ogc:PropertyName>"
         "<gml:Envelope srsName=\"urn:ogc:def:crs:EPSG::4326\">"
         "<gml:lowerCorner>49 2</gml:lowerCorner>"
         "<gml:upperCorner>50 3</gml:upperCorner>"
         "</gml:Envelope>"
         "</ogc:BBOX>"
         "</ogc:Filter>"
       );
}

void TestQgsOgcUtils::testExpressionToOgcFilterWFS20()
{
  QFETCH( QString, exprText );
  QFETCH( QString, srsName );
  QFETCH( QString, xmlText );
  QFETCH( QString, namespacePrefix );
  QFETCH( QString, namespaceURI );

  const QgsExpression exp( exprText );
  QVERIFY( !exp.hasParserError() );

  QString errorMsg;
  QDomDocument doc;
  const QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( exp, doc, QgsOgcUtils::GML_3_2_1, QgsOgcUtils::FILTER_FES_2_0, namespacePrefix, namespaceURI, u"my_geometry_name"_s, srsName, true, false, &errorMsg );

  if ( !errorMsg.isEmpty() )
    qDebug( "ERROR: %s", errorMsg.toLatin1().data() );

  QVERIFY( !filterElem.isNull() );

  doc.appendChild( filterElem );

  //qDebug( "EXPR: %s", exp.expression().toLatin1().data() );
  //qDebug( "SRSNAME: %s", srsName.toLatin1().data() );
  //qDebug( "OGC : %s", doc.toString( -1 ).toLatin1().data() );

  QDomElement xmlElem = comparableElement( xmlText );
  QDomElement ogcElem = comparableElement( doc.toString( -1 ) );
  QVERIFY( QgsTestUtils::compareDomElements( xmlElem, ogcElem ) );
}

void TestQgsOgcUtils::testExpressionToOgcFilterWFS20_data()
{
  QTest::addColumn<QString>( "exprText" );
  QTest::addColumn<QString>( "srsName" );
  QTest::addColumn<QString>( "xmlText" );
  QTest::addColumn<QString>( "namespacePrefix" );
  QTest::addColumn<QString>( "namespaceURI" );

  QTest::newRow( "=" ) << u"NAME = 'New York'"_s << QString() << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">"
                                                                          "<fes:PropertyIsEqualTo>"
                                                                          "<fes:ValueReference>NAME</fes:ValueReference>"
                                                                          "<fes:Literal>New York</fes:Literal>"
                                                                          "</fes:PropertyIsEqualTo></fes:Filter>" )
                       << QString() << QString();

  QTest::newRow( "= with namespace" ) << u"NAME = 'New York'"_s << QString() << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:myns=\"http://example.com/myns\">"
                                                                                         "<fes:PropertyIsEqualTo>"
                                                                                         "<fes:ValueReference>myns:NAME</fes:ValueReference>"
                                                                                         "<fes:Literal>New York</fes:Literal>"
                                                                                         "</fes:PropertyIsEqualTo></fes:Filter>" )
                                      << u"myns"_s << u"http://example.com/myns"_s;

  QTest::newRow( "bbox $geometry" )
    << u"intersects_bbox($geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))"_s
    << u"urn:ogc:def:crs:EPSG::4326"_s
    << QString(
         "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:gml=\"http://www.opengis.net/gml/3.2\">"
         "<fes:BBOX>"
         "<fes:ValueReference>my_geometry_name</fes:ValueReference>"
         "<gml:Envelope srsName=\"urn:ogc:def:crs:EPSG::4326\">"
         "<gml:lowerCorner>49 2</gml:lowerCorner>"
         "<gml:upperCorner>50 3</gml:upperCorner>"
         "</gml:Envelope>"
         "</fes:BBOX>"
         "</fes:Filter>"
       )
    << QString() << QString();

  QTest::newRow( "bbox with namespace $geometry" )
    << u"intersects_bbox($geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))"_s
    << u"urn:ogc:def:crs:EPSG::4326"_s
    << QString(
         "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:gml=\"http://www.opengis.net/gml/3.2\" xmlns:myns=\"http://example.com/myns\">"
         "<fes:BBOX>"
         "<fes:ValueReference>myns:my_geometry_name</fes:ValueReference>"
         "<gml:Envelope srsName=\"urn:ogc:def:crs:EPSG::4326\">"
         "<gml:lowerCorner>49 2</gml:lowerCorner>"
         "<gml:upperCorner>50 3</gml:upperCorner>"
         "</gml:Envelope>"
         "</fes:BBOX>"
         "</fes:Filter>"
       )
    << u"myns"_s << u"http://example.com/myns"_s;

  QTest::newRow( "intersects $geometry" )
    << u"intersects($geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))"_s
    << u"urn:ogc:def:crs:EPSG::4326"_s
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
         "</fes:Filter>"
       )
    << QString() << QString();

  QTest::newRow( "bbox @geometry" )
    << u"intersects_bbox(@geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))"_s
    << u"urn:ogc:def:crs:EPSG::4326"_s
    << QString(
         "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:gml=\"http://www.opengis.net/gml/3.2\">"
         "<fes:BBOX>"
         "<fes:ValueReference>my_geometry_name</fes:ValueReference>"
         "<gml:Envelope srsName=\"urn:ogc:def:crs:EPSG::4326\">"
         "<gml:lowerCorner>49 2</gml:lowerCorner>"
         "<gml:upperCorner>50 3</gml:upperCorner>"
         "</gml:Envelope>"
         "</fes:BBOX>"
         "</fes:Filter>"
       )
    << QString() << QString();

  QTest::newRow( "bbox with namespace @geometry" )
    << u"intersects_bbox(@geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))"_s
    << u"urn:ogc:def:crs:EPSG::4326"_s
    << QString(
         "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:gml=\"http://www.opengis.net/gml/3.2\" xmlns:myns=\"http://example.com/myns\">"
         "<fes:BBOX>"
         "<fes:ValueReference>myns:my_geometry_name</fes:ValueReference>"
         "<gml:Envelope srsName=\"urn:ogc:def:crs:EPSG::4326\">"
         "<gml:lowerCorner>49 2</gml:lowerCorner>"
         "<gml:upperCorner>50 3</gml:upperCorner>"
         "</gml:Envelope>"
         "</fes:BBOX>"
         "</fes:Filter>"
       )
    << u"myns"_s << u"http://example.com/myns"_s;

  QTest::newRow( "intersects @geometry" )
    << u"intersects(@geometry, geomFromWKT('POLYGON((2 49,2 50,3 50,3 49,2 49))'))"_s
    << u"urn:ogc:def:crs:EPSG::4326"_s
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
         "</fes:Filter>"
       )
    << QString() << QString();
}

Q_DECLARE_METATYPE( QgsOgcUtils::GMLVersion )
Q_DECLARE_METATYPE( QgsOgcUtils::FilterVersion )
Q_DECLARE_METATYPE( QList<QgsOgcUtils::LayerProperties> )

void TestQgsOgcUtils::testSQLStatementToOgcFilter()
{
  QFETCH( QString, statementText );
  QFETCH( QgsOgcUtils::GMLVersion, gmlVersion );
  QFETCH( QgsOgcUtils::FilterVersion, filterVersion );
  QFETCH( QList<QgsOgcUtils::LayerProperties>, layerProperties );
  QFETCH( QString, xmlText );

  const QgsSQLStatement statement( statementText );
  if ( !statement.hasParserError() )
  {
    //qDebug( "%s", statement.parserErrorString().toLatin1().data() );
    QVERIFY( !statement.hasParserError() );
  }

  QString errorMsg;
  QDomDocument doc;
  //QgsOgcUtils::GMLVersion gmlVersion = QgsOgcUtils::GML_3_2_1;
  //QgsOgcUtils::FilterVersion filterVersion = QgsOgcUtils::FILTER_FES_2_0;
  const bool honourAxisOrientation = true;
  const bool invertAxisOrientation = false;
  //QList<QgsOgcUtils::LayerProperties> layerProperties;
  const QDomElement filterElem = QgsOgcUtils::SQLStatementToOgcFilter( statement, doc, gmlVersion, filterVersion, layerProperties, honourAxisOrientation, invertAxisOrientation, QMap<QString, QString>(), &errorMsg );

  if ( !errorMsg.isEmpty() )
    qDebug( "ERROR: %s", errorMsg.toLatin1().data() );

  QVERIFY( !filterElem.isNull() );

  doc.appendChild( filterElem );

#if 0
  qDebug( "SQL:    %s", statement.statement().toLatin1().data() );
  qDebug( "GML:    %s", gmlVersion == QgsOgcUtils::GML_2_1_2 ? "2.1.2" : gmlVersion == QgsOgcUtils::GML_3_1_0 ? "3.1.0"
                                                                       : gmlVersion == QgsOgcUtils::GML_3_2_1 ? "3.2.1"
                                                                                                              : "unknown" );
  qDebug( "FILTER: %s", filterVersion == QgsOgcUtils::FILTER_OGC_1_0 ? "OGC 1.0" : filterVersion == QgsOgcUtils::FILTER_OGC_1_1 ? "OGC 1.1"
                                                                                 : filterVersion == QgsOgcUtils::FILTER_FES_2_0 ? "FES 2.0"
                                                                                                                                : "unknown" );
  qDebug( "OGC :   %s", doc.toString( -1 ).toLatin1().data() );
#endif

  QDomElement xmlElem = comparableElement( xmlText );
  QDomElement ogcElem = comparableElement( doc.toString( -1 ) );
  QVERIFY( QgsTestUtils::compareDomElements( xmlElem, ogcElem ) );
}

void TestQgsOgcUtils::testSQLStatementToOgcFilter_data()
{
  const QList<QgsOgcUtils::LayerProperties> layerProperties;

  QTest::addColumn<QString>( "statementText" );
  QTest::addColumn<QgsOgcUtils::GMLVersion>( "gmlVersion" );
  QTest::addColumn<QgsOgcUtils::FilterVersion>( "filterVersion" );
  QTest::addColumn<QList<QgsOgcUtils::LayerProperties>>( "layerProperties" );
  QTest::addColumn<QString>( "xmlText" );

  QTest::newRow( "= 1.0" ) << u"SELECT * FROM t WHERE NAME = 'New York'"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                                   "<ogc:PropertyIsEqualTo>"
                                                                                                                                                                   "<ogc:PropertyName>NAME</ogc:PropertyName>"
                                                                                                                                                                   "<ogc:Literal>New York</ogc:Literal>"
                                                                                                                                                                   "</ogc:PropertyIsEqualTo>"
                                                                                                                                                                   "</ogc:Filter>" );

  QTest::newRow( "= 2.0" ) << u"SELECT * FROM t WHERE NAME = 'New York'"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerProperties << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">"
                                                                                                                                                                   "<fes:PropertyIsEqualTo>"
                                                                                                                                                                   "<fes:ValueReference>NAME</fes:ValueReference>"
                                                                                                                                                                   "<fes:Literal>New York</fes:Literal>"
                                                                                                                                                                   "</fes:PropertyIsEqualTo>"
                                                                                                                                                                   "</fes:Filter>" );

  QTest::newRow( ">" ) << u"SELECT * FROM t WHERE COUNT > 3"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                       "<ogc:PropertyIsGreaterThan>"
                                                                                                                                                       "<ogc:PropertyName>COUNT</ogc:PropertyName>"
                                                                                                                                                       "<ogc:Literal>3</ogc:Literal>"
                                                                                                                                                       "</ogc:PropertyIsGreaterThan></ogc:Filter>" );

  QTest::newRow( "and+or" ) << u"SELECT * FROM t WHERE (FIELD1 <= 10 OR FIELD1 > 20) AND STATUS >= 1.5"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                                                                  "<ogc:And>"
                                                                                                                                                                                                  "<ogc:Or>"
                                                                                                                                                                                                  "<ogc:PropertyIsLessThanOrEqualTo>"
                                                                                                                                                                                                  "<ogc:PropertyName>FIELD1</ogc:PropertyName>"
                                                                                                                                                                                                  "<ogc:Literal>10</ogc:Literal>"
                                                                                                                                                                                                  "</ogc:PropertyIsLessThanOrEqualTo>"
                                                                                                                                                                                                  "<ogc:PropertyIsGreaterThan>"
                                                                                                                                                                                                  "<ogc:PropertyName>FIELD1</ogc:PropertyName>"
                                                                                                                                                                                                  "<ogc:Literal>20</ogc:Literal>"
                                                                                                                                                                                                  "</ogc:PropertyIsGreaterThan>"
                                                                                                                                                                                                  "</ogc:Or>"
                                                                                                                                                                                                  "<ogc:PropertyIsGreaterThanOrEqualTo>"
                                                                                                                                                                                                  "<ogc:PropertyName>STATUS</ogc:PropertyName>"
                                                                                                                                                                                                  "<ogc:Literal>1.5</ogc:Literal>"
                                                                                                                                                                                                  "</ogc:PropertyIsGreaterThanOrEqualTo>"
                                                                                                                                                                                                  "</ogc:And>"
                                                                                                                                                                                                  "</ogc:Filter>" );

  QTest::newRow( "is null" ) << u"SELECT * FROM t WHERE A IS NULL"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                             "<ogc:PropertyIsNull>"
                                                                                                                                                             "<ogc:PropertyName>A</ogc:PropertyName>"
                                                                                                                                                             "</ogc:PropertyIsNull>"
                                                                                                                                                             "</ogc:Filter>" );

  QTest::newRow( "is not null" ) << u"SELECT * FROM t WHERE A IS NOT NULL"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                                     "<ogc:Not>"
                                                                                                                                                                     "<ogc:PropertyIsNull>"
                                                                                                                                                                     "<ogc:PropertyName>A</ogc:PropertyName>"
                                                                                                                                                                     "</ogc:PropertyIsNull>"
                                                                                                                                                                     "</ogc:Not>"
                                                                                                                                                                     "</ogc:Filter>" );

  QTest::newRow( "in" ) << u"SELECT * FROM t WHERE A IN (10,20,30)"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
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

  QTest::newRow( "not in" ) << u"SELECT * FROM t WHERE A NOT IN (10,20,30)"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                                      "<ogc:Not>"
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
                                                                                                                                                                      "</ogc:Not>"
                                                                                                                                                                      "</ogc:Filter>" );

  QTest::newRow( "in" ) << u"SELECT * FROM t WHERE A IN (10)"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                        "<ogc:PropertyIsEqualTo>"
                                                                                                                                                        "<ogc:PropertyName>A</ogc:PropertyName>"
                                                                                                                                                        "<ogc:Literal>10</ogc:Literal>"
                                                                                                                                                        "</ogc:PropertyIsEqualTo>"
                                                                                                                                                        "</ogc:Filter>" );

  QTest::newRow( "not in" ) << u"SELECT * FROM t WHERE A NOT IN (10)"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                                "<ogc:Not>"
                                                                                                                                                                "<ogc:PropertyIsEqualTo>"
                                                                                                                                                                "<ogc:PropertyName>A</ogc:PropertyName>"
                                                                                                                                                                "<ogc:Literal>10</ogc:Literal>"
                                                                                                                                                                "</ogc:PropertyIsEqualTo>"
                                                                                                                                                                "</ogc:Not>"
                                                                                                                                                                "</ogc:Filter>" );

  QTest::newRow( "between" ) << u"SELECT * FROM t WHERE A BETWEEN 1 AND 2"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                                     "<ogc:PropertyIsBetween>"
                                                                                                                                                                     "<ogc:PropertyName>A</ogc:PropertyName>"
                                                                                                                                                                     "<ogc:LowerBoundary><ogc:Literal>1</ogc:Literal></ogc:LowerBoundary>"
                                                                                                                                                                     "<ogc:UpperBoundary><ogc:Literal>2</ogc:Literal></ogc:UpperBoundary>"
                                                                                                                                                                     "</ogc:PropertyIsBetween>"
                                                                                                                                                                     "</ogc:Filter>" );

  QTest::newRow( "not between" ) << u"SELECT * FROM t WHERE A NOT BETWEEN 1 AND 2"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                                             "<ogc:Not>"
                                                                                                                                                                             "<ogc:PropertyIsBetween>"
                                                                                                                                                                             "<ogc:PropertyName>A</ogc:PropertyName>"
                                                                                                                                                                             "<ogc:LowerBoundary><ogc:Literal>1</ogc:Literal></ogc:LowerBoundary>"
                                                                                                                                                                             "<ogc:UpperBoundary><ogc:Literal>2</ogc:Literal></ogc:UpperBoundary>"
                                                                                                                                                                             "</ogc:PropertyIsBetween>"
                                                                                                                                                                             "</ogc:Not>"
                                                                                                                                                                             "</ogc:Filter>" );

  QTest::newRow( "intersects + wkt" ) << u"SELECT * FROM t WHERE ST_Intersects(geom, ST_GeometryFromText('POINT (5 6)'))"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
                                                                                                                                                                                                                    "<ogc:Intersects>"
                                                                                                                                                                                                                    "<ogc:PropertyName>geom</ogc:PropertyName>"
                                                                                                                                                                                                                    "<gml:Point><gml:coordinates ts=\" \" cs=\",\">5,6</gml:coordinates></gml:Point>"
                                                                                                                                                                                                                    "</ogc:Intersects>"
                                                                                                                                                                                                                    "</ogc:Filter>" );

  QTest::newRow( "contains + gml" ) << u"SELECT * FROM t WHERE ST_Contains(geom, ST_GeomFromGML('<gml:Point xmlns:gml=\"http://www.opengis.net/gml\"><gml:coordinates cs=\",\" ts=\" \">5,6</gml:coordinates></gml:Point>'))"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
                                                                                                                                                                                                                                                                                                                        "<ogc:Contains>"
                                                                                                                                                                                                                                                                                                                        "<ogc:PropertyName>geom</ogc:PropertyName>"
                                                                                                                                                                                                                                                                                                                        "<gml:Point xmlns:gml=\"http://www.opengis.net/gml\"><gml:coordinates xmlns:gml=\"http://www.opengis.net/gml\" ts=\" \" cs=\",\">5,6</gml:coordinates></gml:Point>"
                                                                                                                                                                                                                                                                                                                        "</ogc:Contains>"
                                                                                                                                                                                                                                                                                                                        "</ogc:Filter>" );

  QTest::newRow( "abs" ) << u"SELECT * FROM t WHERE ABS(x) < 5"_s << QgsOgcUtils::GML_2_1_2 << QgsOgcUtils::FILTER_OGC_1_0 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\">"
                                                                                                                                                          "<ogc:PropertyIsLessThan>"
                                                                                                                                                          "<ogc:Function name=\"ABS\">"
                                                                                                                                                          "<ogc:PropertyName>x</ogc:PropertyName>"
                                                                                                                                                          "</ogc:Function>"
                                                                                                                                                          "<ogc:Literal>5</ogc:Literal>"
                                                                                                                                                          "</ogc:PropertyIsLessThan>"
                                                                                                                                                          "</ogc:Filter>" );

  QTest::newRow( "bbox + wkt + explicit srs" ) << u"SELECT * FROM t WHERE BBOX(geom, ST_MakeEnvelope(2.2, 49, 3, 50, 4326))"_s << QgsOgcUtils::GML_3_1_0 << QgsOgcUtils::FILTER_OGC_1_1 << layerProperties << QString( "<ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:gml=\"http://www.opengis.net/gml\">"
                                                                                                                                                                                                                       "<ogc:BBOX>"
                                                                                                                                                                                                                       "<ogc:PropertyName>geom</ogc:PropertyName>"
                                                                                                                                                                                                                       "<gml:Envelope srsName=\"urn:ogc:def:crs:EPSG::4326\">"
                                                                                                                                                                                                                       "<gml:lowerCorner>49 2.2</gml:lowerCorner>"
                                                                                                                                                                                                                       "<gml:upperCorner>50 3</gml:upperCorner>"
                                                                                                                                                                                                                       "</gml:Envelope>"
                                                                                                                                                                                                                       "</ogc:BBOX>"
                                                                                                                                                                                                                       "</ogc:Filter>" );

  QTest::newRow( "intersects + wkt + explicit srs" ) << u"SELECT * FROM t WHERE ST_Intersects(geom, ST_GeometryFromText('POINT (5 6)', 'urn:ogc:def:crs:EPSG::4326'))"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerProperties << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:gml=\"http://www.opengis.net/gml/3.2\">"
                                                                                                                                                                                                                                                                 "<fes:Intersects>"
                                                                                                                                                                                                                                                                 "<fes:ValueReference>geom</fes:ValueReference>"
                                                                                                                                                                                                                                                                 "<gml:Point gml:id=\"qgis_id_geom_1\" srsName=\"urn:ogc:def:crs:EPSG::4326\">"
                                                                                                                                                                                                                                                                 "<gml:pos srsDimension=\"2\">6 5</gml:pos>"
                                                                                                                                                                                                                                                                 "</gml:Point>"
                                                                                                                                                                                                                                                                 "</fes:Intersects>"
                                                                                                                                                                                                                                                                 "</fes:Filter>" );

  QTest::newRow( "intersects + wkt + explicit srs int" ) << u"SELECT * FROM t WHERE ST_Intersects(geom, ST_GeometryFromText('POINT (5 6)', 4326))"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerProperties << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:gml=\"http://www.opengis.net/gml/3.2\">"
                                                                                                                                                                                                                                             "<fes:Intersects>"
                                                                                                                                                                                                                                             "<fes:ValueReference>geom</fes:ValueReference>"
                                                                                                                                                                                                                                             "<gml:Point gml:id=\"qgis_id_geom_1\" srsName=\"urn:ogc:def:crs:EPSG::4326\">"
                                                                                                                                                                                                                                             "<gml:pos srsDimension=\"2\">6 5</gml:pos>"
                                                                                                                                                                                                                                             "</gml:Point>"
                                                                                                                                                                                                                                             "</fes:Intersects>"
                                                                                                                                                                                                                                             "</fes:Filter>" );

  QTest::newRow( "dwithin + wkt" ) << u"SELECT * FROM t WHERE ST_DWithin(geom, ST_GeometryFromText('POINT (5 6)', 'urn:ogc:def:crs:EPSG::4326'), '3 m')"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerProperties << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:gml=\"http://www.opengis.net/gml/3.2\">"
                                                                                                                                                                                                                                                   "<fes:DWithin>"
                                                                                                                                                                                                                                                   "<fes:ValueReference>geom</fes:ValueReference>"
                                                                                                                                                                                                                                                   "<gml:Point gml:id=\"qgis_id_geom_1\" srsName=\"urn:ogc:def:crs:EPSG::4326\">"
                                                                                                                                                                                                                                                   "<gml:pos srsDimension=\"2\">6 5</gml:pos>"
                                                                                                                                                                                                                                                   "</gml:Point>"
                                                                                                                                                                                                                                                   "<fes:Distance uom=\"m\">3</fes:Distance>"
                                                                                                                                                                                                                                                   "</fes:DWithin>"
                                                                                                                                                                                                                                                   "</fes:Filter>" );

  QList<QgsOgcUtils::LayerProperties> layerProperties4326_FES20;
  QgsOgcUtils::LayerProperties prop;
  prop.mSRSName = u"urn:ogc:def:crs:EPSG::4326"_s;
  prop.mGeometryAttribute = u"geom"_s;
  layerProperties4326_FES20.append( prop );

  QTest::newRow( "intersects + wkt + implicit SRS" ) << u"SELECT * FROM t WHERE ST_Intersects(geom, ST_GeometryFromText('POINT (5 6)'))"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerProperties4326_FES20 << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:gml=\"http://www.opengis.net/gml/3.2\">"
                                                                                                                                                                                                                                             "<fes:Intersects>"
                                                                                                                                                                                                                                             "<fes:ValueReference>geom</fes:ValueReference>"
                                                                                                                                                                                                                                             "<gml:Point gml:id=\"qgis_id_geom_1\" srsName=\"urn:ogc:def:crs:EPSG::4326\">"
                                                                                                                                                                                                                                             "<gml:pos srsDimension=\"2\">6 5</gml:pos>"
                                                                                                                                                                                                                                             "</gml:Point>"
                                                                                                                                                                                                                                             "</fes:Intersects>"
                                                                                                                                                                                                                                             "</fes:Filter>" );

  QTest::newRow( "intersects join 2.0" ) << u"SELECT * FROM t, t2 WHERE ST_Intersects(t.geom, t2.geom)"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerProperties << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">"
                                                                                                                                                                                                  "<fes:Intersects>"
                                                                                                                                                                                                  "<fes:ValueReference>t/geom</fes:ValueReference>"
                                                                                                                                                                                                  "<fes:ValueReference>t2/geom</fes:ValueReference>"
                                                                                                                                                                                                  "</fes:Intersects>"
                                                                                                                                                                                                  "</fes:Filter>" );

  QTest::newRow( "attrib join USING 2.0" ) << u"SELECT * FROM t JOIN t2 USING (a)"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerProperties << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">"
                                                                                                                                                                             "<fes:PropertyIsEqualTo>"
                                                                                                                                                                             "<fes:ValueReference>t/a</fes:ValueReference>"
                                                                                                                                                                             "<fes:ValueReference>t2/a</fes:ValueReference>"
                                                                                                                                                                             "</fes:PropertyIsEqualTo>"
                                                                                                                                                                             "</fes:Filter>" );

  QTest::newRow( "attrib join multi USING 2.0" ) << u"SELECT * FROM t JOIN t2 USING (a, b)"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerProperties << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">"
                                                                                                                                                                                      "<fes:And>"
                                                                                                                                                                                      "<fes:PropertyIsEqualTo>"
                                                                                                                                                                                      "<fes:ValueReference>t/a</fes:ValueReference>"
                                                                                                                                                                                      "<fes:ValueReference>t2/a</fes:ValueReference>"
                                                                                                                                                                                      "</fes:PropertyIsEqualTo>"
                                                                                                                                                                                      "<fes:PropertyIsEqualTo>"
                                                                                                                                                                                      "<fes:ValueReference>t/b</fes:ValueReference>"
                                                                                                                                                                                      "<fes:ValueReference>t2/b</fes:ValueReference>"
                                                                                                                                                                                      "</fes:PropertyIsEqualTo>"
                                                                                                                                                                                      "</fes:And>"
                                                                                                                                                                                      "</fes:Filter>" );

  QTest::newRow( "attrib join ON 2.0" ) << u"SELECT * FROM t aliased_t JOIN t2 aliasted_t2 ON aliased_t.a = aliasted_t2.b"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerProperties << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">"
                                                                                                                                                                                                                     "<fes:PropertyIsEqualTo>"
                                                                                                                                                                                                                     "<fes:ValueReference>t/a</fes:ValueReference>"
                                                                                                                                                                                                                     "<fes:ValueReference>t2/b</fes:ValueReference>"
                                                                                                                                                                                                                     "</fes:PropertyIsEqualTo>"
                                                                                                                                                                                                                     "</fes:Filter>" );

  QTest::newRow( "attrib multi join 2.0" ) << u"SELECT * FROM t aliased_t JOIN t2 aliasted_t2 ON aliased_t.a = aliasted_t2.b JOIN t3 USING (c)"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerProperties << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">"
                                                                                                                                                                                                                                          "<fes:And>"
                                                                                                                                                                                                                                          "<fes:PropertyIsEqualTo>"
                                                                                                                                                                                                                                          "<fes:ValueReference>t/a</fes:ValueReference>"
                                                                                                                                                                                                                                          "<fes:ValueReference>t2/b</fes:ValueReference>"
                                                                                                                                                                                                                                          "</fes:PropertyIsEqualTo>"
                                                                                                                                                                                                                                          "<fes:PropertyIsEqualTo>"
                                                                                                                                                                                                                                          "<fes:ValueReference>t2/c</fes:ValueReference>"
                                                                                                                                                                                                                                          "<fes:ValueReference>t3/c</fes:ValueReference>"
                                                                                                                                                                                                                                          "</fes:PropertyIsEqualTo>"
                                                                                                                                                                                                                                          "</fes:And>"
                                                                                                                                                                                                                                          "</fes:Filter>" );

  QList<QgsOgcUtils::LayerProperties> layerPropertiesWithNameSpace;
  QgsOgcUtils::LayerProperties props;
  props.mName = u"prefix:mylayer"_s;
  props.mNamespacePrefix = u"prefix"_s;
  props.mNamespaceURI = u"http://example.com/prefix"_s;
  layerPropertiesWithNameSpace << props;

  QTest::newRow( "namespace" ) << u"SELECT * FROM mylayer WHERE NAME = 'New York'"_s << QgsOgcUtils::GML_3_2_1 << QgsOgcUtils::FILTER_FES_2_0 << layerPropertiesWithNameSpace << QString( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\" xmlns:prefix=\"http://example.com/prefix\">"
                                                                                                                                                                                          "<fes:PropertyIsEqualTo>"
                                                                                                                                                                                          "<fes:ValueReference>prefix:NAME</fes:ValueReference>"
                                                                                                                                                                                          "<fes:Literal>New York</fes:Literal>"
                                                                                                                                                                                          "</fes:PropertyIsEqualTo>"
                                                                                                                                                                                          "</fes:Filter>" );
}

void TestQgsOgcUtils::testExpressionToOgcFilterWithXPath()
{
  const QgsExpression exp( "a = 1" );
  QString errorMsg;

  QMap<QString, QString> mapFieldNameToXPath;
  mapFieldNameToXPath["a"] = "myns:foo/myns:bar/otherns:a";

  QMap<QString, QString> mapNamespacePrefixToUri;
  mapNamespacePrefixToUri["myns"] = "https://myns";
  mapNamespacePrefixToUri["otherns"] = "https://otherns";

  QDomDocument doc;
  const QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( exp, doc, QgsOgcUtils::GML_3_2_1, QgsOgcUtils::FILTER_FES_2_0, QString(), QString(), u"my_geometry_name"_s, QString(), true, false, &errorMsg, mapFieldNameToXPath, mapNamespacePrefixToUri );

  if ( !errorMsg.isEmpty() )
    qDebug( "ERROR: %s", errorMsg.toLatin1().data() );

  QDomElement xmlElem = comparableElement( u"<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\"><fes:PropertyIsEqualTo><fes:ValueReference xmlns:otherns=\"https://otherns\" xmlns:myns=\"https://myns\">myns:foo/myns:bar/otherns:a</fes:ValueReference><fes:Literal>1</fes:Literal></fes:PropertyIsEqualTo></fes:Filter>"_s );
  doc.appendChild( filterElem );
  //qDebug( "OGC :   %s", doc.toString( -1 ).toLatin1().data() );

  QDomElement ogcElem = comparableElement( doc.toString( -1 ) );
  QVERIFY( QgsTestUtils::compareDomElements( xmlElem, ogcElem ) );
}

void TestQgsOgcUtils::testSQLStatementToOgcFilterWithXPath()
{
  const QgsSQLStatement statement( "SELECT * FROM t WHERE a = 1" );
  if ( !statement.hasParserError() )
  {
    qDebug( "%s", statement.parserErrorString().toLatin1().data() );
    QVERIFY( !statement.hasParserError() );
  }

  QMap<QString, QString> mapFieldNameToXPath;
  mapFieldNameToXPath["a"] = "myns:foo/myns:bar/otherns:a";

  QMap<QString, QString> mapNamespacePrefixToUri;
  mapNamespacePrefixToUri["myns"] = "https://myns";
  mapNamespacePrefixToUri["otherns"] = "https://otherns";

  QString errorMsg;
  QDomDocument doc;
  const bool honourAxisOrientation = true;
  const bool invertAxisOrientation = false;
  QList<QgsOgcUtils::LayerProperties> layerProperties;
  QgsOgcUtils::LayerProperties prop;
  prop.mSRSName = u"urn:ogc:def:crs:EPSG::4326"_s;
  prop.mGeometryAttribute = u"geom"_s;
  layerProperties.append( prop );
  const QDomElement filterElem = QgsOgcUtils::SQLStatementToOgcFilter( statement, doc, QgsOgcUtils::GML_3_2_1, QgsOgcUtils::FILTER_FES_2_0, layerProperties, honourAxisOrientation, invertAxisOrientation, QMap<QString, QString>(), &errorMsg, mapFieldNameToXPath, mapNamespacePrefixToUri );

  if ( !errorMsg.isEmpty() )
    qDebug( "ERROR: %s", errorMsg.toLatin1().data() );

  QDomElement xmlElem = comparableElement( u"<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\"><fes:PropertyIsEqualTo><fes:ValueReference xmlns:otherns=\"https://otherns\" xmlns:myns=\"https://myns\">myns:foo/myns:bar/otherns:a</fes:ValueReference><fes:Literal>1</fes:Literal></fes:PropertyIsEqualTo></fes:Filter>"_s );
  doc.appendChild( filterElem );
  //qDebug( "OGC :   %s", doc.toString( -1 ).toLatin1().data() );

  QDomElement ogcElem = comparableElement( doc.toString( -1 ) );
  QVERIFY( QgsTestUtils::compareDomElements( xmlElem, ogcElem ) );
}


void TestQgsOgcUtils::testParseCrsName()
{
  QFETCH( QString, crsName );
  QFETCH( QgsOgcCrsUtils::CRSFlavor, expectedFlavor );
  QFETCH( QString, expectedAuthority );
  QFETCH( QString, expectedCode );

  QString authority;
  QString code;
  const QgsOgcCrsUtils::CRSFlavor crsFlavor = QgsOgcCrsUtils::parseCrsName( crsName, authority, code );
  QCOMPARE( expectedFlavor, crsFlavor );
  QCOMPARE( expectedAuthority, authority );
  QCOMPARE( expectedCode, code );
}

void TestQgsOgcUtils::testParseCrsName_data()
{
  QTest::addColumn<QString>( "crsName" );
  QTest::addColumn<QgsOgcCrsUtils::CRSFlavor>( "expectedFlavor" );
  QTest::addColumn<QString>( "expectedAuthority" );
  QTest::addColumn<QString>( "expectedCode" );

  QTest::newRow( "unknown" ) << u"foo"_s << QgsOgcCrsUtils::CRSFlavor::UNKNOWN << QString() << QString();
  QTest::newRow( "unknown2" ) << u"EPSG:"_s << QgsOgcCrsUtils::CRSFlavor::UNKNOWN << QString() << QString();
  QTest::newRow( "AUTH_CODE" ) << u"EPSG:1234"_s << QgsOgcCrsUtils::CRSFlavor::AUTH_CODE << u"EPSG"_s << u"1234"_s;
  QTest::newRow( "HTTP_EPSG_DOT_XML" ) << u"http://www.opengis.net/gml/srs/epsg.xml#1234"_s << QgsOgcCrsUtils::CRSFlavor::HTTP_EPSG_DOT_XML << u"EPSG"_s << u"1234"_s;
  QTest::newRow( "OGC_URN" ) << u"urn:ogc:def:crs:EPSG::1234"_s << QgsOgcCrsUtils::CRSFlavor::OGC_URN << u"EPSG"_s << u"1234"_s;
  QTest::newRow( "OGC_URN missing col" ) << u"urn:ogc:def:crs:EPSG:1234"_s << QgsOgcCrsUtils::CRSFlavor::OGC_URN << u"EPSG"_s << u"1234"_s;
  QTest::newRow( "X_OGC_URN" ) << u"urn:x-ogc:def:crs:EPSG::1234"_s << QgsOgcCrsUtils::CRSFlavor::X_OGC_URN << u"EPSG"_s << u"1234"_s;
  QTest::newRow( "X_OGC_URN missing col" ) << u"urn:x-ogc:def:crs:EPSG:1234"_s << QgsOgcCrsUtils::CRSFlavor::X_OGC_URN << u"EPSG"_s << u"1234"_s;
  QTest::newRow( "OGC_HTTP_URI" ) << u"http://www.opengis.net/def/crs/EPSG/0/1234"_s << QgsOgcCrsUtils::CRSFlavor::OGC_HTTP_URI << u"EPSG"_s << u"1234"_s;
}


QGSTEST_MAIN( TestQgsOgcUtils )
#include "testqgsogcutils.moc"
