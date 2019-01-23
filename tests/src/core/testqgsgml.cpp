
/***************************************************************************
     testqgsogcutils.cpp
     --------------------------------------
    Date                 : March 2016
    Copyright            : (C) 2016 Even Rouault
    Email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QUrl>

//qgis includes...
#include <qgsgeometry.h>
#include <qgsgml.h>
#include "qgsapplication.h"
#include "qgslogger.h"

/**
 * \ingroup UnitTests
 * This is a unit test for GML parsing
 */
class TestQgsGML : public QObject
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
    }

    void cleanupTestCase()
    {
    }

    void testFromURL();
    void testFromByteArray();
    void testStreamingParser();
    void testStreamingParserInvalidGML();
    void testPointGML2();
    void testLineStringGML2();
    void testPolygonGML2();
    void testMultiPointGML2();
    void testMultiLineStringGML2();
    void testMultiPolygonGML2();
    void testPointGML3();
    void testPointGML3_EPSG_4326();
    void testPointGML3_urn_EPSG_4326();
    void testPointGML3_EPSG_4326_honour_EPSG();
    void testPointGML3_EPSG_4326_honour_EPSG_invert();
    void testLineStringGML3();
    void testLineStringGML3_LineStringSegment();
    void testPolygonGML3();
    void testPolygonGML3_srsDimension_on_Polygon();
    void testMultiLineStringGML3();
    void testMultiPolygonGML3();
    void testPointGML3_2();
    void testBoundingBoxGML2();
    void testBoundingBoxGML3();
    void testNumberMatchedNumberReturned();
    void testException();
    void testTuple();
    void testRenamedFields();
    void testTruncatedResponse();
    void testPartialFeature();
    void testThroughOGRGeometry();
    void testThroughOGRGeometry_urn_EPSG_4326();
    void testAccents();
    void testSameTypeameAsGeomName();
};

const QString data1( "<myns:FeatureCollection "
                     "xmlns:myns='http://myns' "
                     "xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' "
                     "xmlns:gml='http://www.opengis.net/gml'>"
                     "<gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>"
                     "<gml:featureMember>"
                     "<myns:mytypename fid='mytypename.1'>"
                     "<myns:intfield>1</myns:intfield>"
                     "<myns:nillablefield xsi:nil='true'/>"
                     "<myns:longfield>1234567890123</myns:longfield>"
                     "<myns:doublefield>1.23</myns:doublefield>"
                     "<myns:strfield>foo</myns:strfield>"
                     "<myns:datetimefield>2016-04-10T12:34:56.789Z</myns:datetimefield>"
                     "<myns:mygeom>"
                     "<gml:Point srsName='http://www.opengis.net/gml/srs/epsg.xml#27700'>"
                     "<gml:coordinates decimal='.' cs=',' ts=' '>10,20</gml:coordinates>"
                     "</gml:Point>"
                     "</myns:mygeom>"
                     "</myns:mytypename>"
                     "</gml:featureMember>"
                     "</myns:FeatureCollection>" );

void TestQgsGML::testFromURL()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "intfield" ), QVariant::Int, QStringLiteral( "int" ) ) );
  fields.append( QgsField( QStringLiteral( "nillablefield" ), QVariant::Int, QStringLiteral( "nillablefield" ) ) );
  QgsGml gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QgsWkbTypes::Type wkbType;
  QTemporaryFile tmpFile;
  tmpFile.open();
  tmpFile.write( data1.toAscii() );
  tmpFile.flush();
  QCOMPARE( gmlParser.getFeatures( QUrl::fromLocalFile( tmpFile.fileName() ).toString(), &wkbType ), 0 );
  QCOMPARE( wkbType, QgsWkbTypes::Point );
  QMap<QgsFeatureId, QgsFeature * > featureMaps = gmlParser.featuresMap();
  QCOMPARE( featureMaps.size(), 1 );
  QCOMPARE( gmlParser.idsMap().size(), 1 );
  QCOMPARE( gmlParser.crs().authid(), QString( "EPSG:27700" ) );
  QCOMPARE( featureMaps[0]->attribute( QStringLiteral( "intfield" ) ).toInt(), 1 );
  QVERIFY( featureMaps[0]->attribute( QStringLiteral( "nillablefield" ) ).isNull( ) );
  delete featureMaps[ 0 ];
}

void TestQgsGML::testFromByteArray()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "intfield" ), QVariant::Int, QStringLiteral( "int" ) ) );
  fields.append( QgsField( QStringLiteral( "nillablefield" ), QVariant::Int, QStringLiteral( "nillablefield" ) ) );
  QgsGml gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QgsWkbTypes::Type wkbType;
  QCOMPARE( gmlParser.getFeatures( data1.toAscii(), &wkbType ), 0 );
  QMap<QgsFeatureId, QgsFeature * > featureMaps = gmlParser.featuresMap();
  QCOMPARE( featureMaps.size(), 1 );
  QVERIFY( featureMaps.constFind( 0 ) != featureMaps.constEnd() );
  QCOMPARE( featureMaps[ 0 ]->attributes().size(), 2 );
  QMap<QgsFeatureId, QString > idsMap = gmlParser.idsMap();
  QVERIFY( idsMap.constFind( 0 ) != idsMap.constEnd() );
  QCOMPARE( idsMap[ 0 ], QString( "mytypename.1" ) );
  QCOMPARE( featureMaps[0]->attribute( QStringLiteral( "intfield" ) ).toInt(), 1 );
  QVERIFY( featureMaps[0]->attribute( QStringLiteral( "nillablefield" ) ).isNull( ) );
  delete featureMaps[ 0 ];
}

void TestQgsGML::testStreamingParser()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "intfield" ), QVariant::Int, QStringLiteral( "int" ) ) );
  fields.append( QgsField( QStringLiteral( "nillablefield" ), QVariant::Int, QStringLiteral( "nillablefield" ) ) );
  fields.append( QgsField( QStringLiteral( "longfield" ), QVariant::LongLong, QStringLiteral( "longlong" ) ) );
  fields.append( QgsField( QStringLiteral( "doublefield" ), QVariant::Double, QStringLiteral( "double" ) ) );
  fields.append( QgsField( QStringLiteral( "strfield" ), QVariant::String, QStringLiteral( "string" ) ) );
  fields.append( QgsField( QStringLiteral( "datetimefield" ), QVariant::DateTime, QStringLiteral( "datetime" ) ) );
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( data1.mid( 0, data1.size() / 2 ).toAscii(), false ), true );
  QCOMPARE( gmlParser.getAndStealReadyFeatures().size(), 0 );
  QCOMPARE( gmlParser.processData( data1.mid( data1.size() / 2 ).toAscii(), true ), true );
  QCOMPARE( gmlParser.isException(), false );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QCOMPARE( features[0].first->attributes().size(), 6 );
  QCOMPARE( features[0].first->attributes().at( 0 ), QVariant( 1 ) );
  QCOMPARE( features[0].first->attributes().at( 1 ), QVariant( ) );
  QCOMPARE( features[0].first->attributes().at( 2 ), QVariant( Q_INT64_C( 1234567890123 ) ) );
  QCOMPARE( features[0].first->attributes().at( 3 ), QVariant( 1.23 ) );
  QCOMPARE( features[0].first->attributes().at( 4 ), QVariant( "foo" ) );
  QCOMPARE( features[0].first->attributes().at( 5 ), QVariant( QDateTime( QDate( 2016, 4, 10 ), QTime( 12, 34, 56, 789 ), Qt::UTC ) ) );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( gmlParser.getAndStealReadyFeatures().size(), 0 );
  QCOMPARE( gmlParser.getEPSGCode(), 27700 );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Point );
  delete features[0].first;
}

void TestQgsGML::testStreamingParserInvalidGML()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( "<FeatureCollection>", true ), false );
  QCOMPARE( gmlParser.getAndStealReadyFeatures().size(), 0 );
}

void TestQgsGML::testPointGML2()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Point srsName='EPSG:27700'>"
                                   "<gml:coordinates>10,20</gml:coordinates>"
                                   "</gml:Point>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Point );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  delete features[0].first;
}

void TestQgsGML::testLineStringGML2()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:LineString srsName='EPSG:27700'>"
                                   "<gml:coordinates>10,20 30,40</gml:coordinates>"
                                   "</gml:LineString>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::LineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::LineString );
  QgsPolylineXY line = features[0].first->geometry().asPolyline();
  QCOMPARE( line.size(), 2 );
  QCOMPARE( line[0], QgsPointXY( 10, 20 ) );
  QCOMPARE( line[1], QgsPointXY( 30, 40 ) );
  delete features[0].first;
}

void TestQgsGML::testPolygonGML2()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Polygon srsName='EPSG:27700'>"
                                   "<gml:outerBoundaryIs>"
                                   "<gml:LinearRing>"
                                   "<gml:coordinates>0,0 0,10 10,10 10,0 0,0</gml:coordinates>"
                                   "</gml:LinearRing>"
                                   "</gml:outerBoundaryIs>"
                                   "<gml:innerBoundaryIs>"
                                   "<gml:LinearRing>"
                                   "<gml:coordinates>1,1 1,9 9,9 1,1</gml:coordinates>"
                                   "</gml:LinearRing>"
                                   "</gml:innerBoundaryIs>"
                                   "</gml:Polygon>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Polygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Polygon );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 2 );
  QCOMPARE( poly[0].size(), 5 );
  QCOMPARE( poly[1].size(), 4 );
  delete features[0].first;
}

void TestQgsGML::testMultiPointGML2()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:MultiPoint srsName='EPSG:27700'>"
                                   "<gml:pointMember>"
                                   "<gml:Point>"
                                   "<gml:coordinates>10,20</gml:coordinates>"
                                   "</gml:Point>"
                                   "</gml:pointMember>"
                                   "<gml:pointMember>"
                                   "<gml:Point>"
                                   "<gml:coordinates>30,40</gml:coordinates>"
                                   "</gml:Point>"
                                   "</gml:pointMember>"
                                   "</gml:MultiPoint>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::MultiPoint );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::MultiPoint );
  QgsMultiPointXY multi = features[0].first->geometry().asMultiPoint();
  QCOMPARE( multi.size(), 2 );
  QCOMPARE( multi[0], QgsPointXY( 10, 20 ) );
  QCOMPARE( multi[1], QgsPointXY( 30, 40 ) );
  delete features[0].first;
}

void TestQgsGML::testMultiLineStringGML2()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:MultiLineString srsName='EPSG:27700'>"
                                   "<gml:lineStringMember>"
                                   "<gml:LineString>"
                                   "<gml:coordinates>10,20 30,40</gml:coordinates>"
                                   "</gml:LineString>"
                                   "</gml:lineStringMember>"
                                   "<gml:lineStringMember>"
                                   "<gml:LineString>"
                                   "<gml:coordinates>50,60 70,80 90,100</gml:coordinates>"
                                   "</gml:LineString>"
                                   "</gml:lineStringMember>"
                                   "</gml:MultiLineString>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::MultiLineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::MultiLineString );
  QgsMultiPolylineXY multi = features[0].first->geometry().asMultiPolyline();
  QCOMPARE( multi.size(), 2 );
  QCOMPARE( multi[0].size(), 2 );
  QCOMPARE( multi[0][0], QgsPointXY( 10, 20 ) );
  QCOMPARE( multi[0][1], QgsPointXY( 30, 40 ) );
  QCOMPARE( multi[1].size(), 3 );
  delete features[0].first;
}

void TestQgsGML::testMultiPolygonGML2()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:MultiPolygon srsName='EPSG:27700'>"
                                   "<gml:polygonMember>"
                                   "<gml:Polygon>"
                                   "<gml:outerBoundaryIs>"
                                   "<gml:LinearRing>"
                                   "<gml:coordinates>0,0 0,10 10,10 10,0 0,0</gml:coordinates>"
                                   "</gml:LinearRing>"
                                   "</gml:outerBoundaryIs>"
                                   "</gml:Polygon>"
                                   "</gml:polygonMember>"
                                   "</gml:MultiPolygon>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::MultiPolygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 1 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testPointGML3()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename gml:id='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Point srsName='urn:ogc:def:crs:EPSG::27700'>"
                                   "<gml:pos>10 20</gml:pos>"
                                   "</gml:Point>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( gmlParser.getEPSGCode(), 27700 );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_EPSG_4326()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename gml:id='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Point srsName='EPSG:4326'>"
                                   "<gml:pos>2 49</gml:pos>"
                                   "</gml:Point>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( gmlParser.getEPSGCode(), 4326 );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_urn_EPSG_4326()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename gml:id='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Point srsName='urn:ogc:def:crs:EPSG::4326'>"
                                   "<gml:pos>49 2</gml:pos>"
                                   "</gml:Point>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( gmlParser.getEPSGCode(), 4326 );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_EPSG_4326_honour_EPSG()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields, QgsGmlStreamingParser::Honour_EPSG );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename gml:id='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Point srsName='EPSG:4326'>"
                                   "<gml:pos>49 2</gml:pos>"
                                   "</gml:Point>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( gmlParser.getEPSGCode(), 4326 );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_EPSG_4326_honour_EPSG_invert()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields, QgsGmlStreamingParser::Honour_EPSG, true );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename gml:id='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Point srsName='EPSG:4326'>"
                                   "<gml:pos>2 49</gml:pos>"
                                   "</gml:Point>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( gmlParser.getEPSGCode(), 4326 );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testLineStringGML3()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:LineString srsName='EPSG:27700'>"
                                   "<gml:posList>10 20 30 40</gml:posList>"
                                   "</gml:LineString>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::LineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::LineString );
  QgsPolylineXY line = features[0].first->geometry().asPolyline();
  QCOMPARE( line.size(), 2 );
  QCOMPARE( line[0], QgsPointXY( 10, 20 ) );
  QCOMPARE( line[1], QgsPointXY( 30, 40 ) );
  delete features[0].first;
}

void TestQgsGML::testLineStringGML3_LineStringSegment()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Curve srsName='EPSG:27700'><gml:segments><gml:LineStringSegment><gml:posList>10 20 30 40</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::LineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::LineString );
  QgsPolylineXY line = features[0].first->geometry().asPolyline();
  QCOMPARE( line.size(), 2 );
  QCOMPARE( line[0], QgsPointXY( 10, 20 ) );
  QCOMPARE( line[1], QgsPointXY( 30, 40 ) );
  delete features[0].first;
}

void TestQgsGML::testPolygonGML3()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Polygon srsName='EPSG:27700'>"
                                   "<gml:exterior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>0 0 0 10 10 10 10 0 0 0</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:exterior>"
                                   "<gml:interior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>1 1 1 9 9 9 1 1</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:interior>"
                                   "</gml:Polygon>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Polygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Polygon );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 2 );
  QCOMPARE( poly[0].size(), 5 );
  QCOMPARE( poly[1].size(), 4 );
  delete features[0].first;
}

void TestQgsGML::testPolygonGML3_srsDimension_on_Polygon()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Polygon srsDimension='3' srsName='EPSG:27700'>"
                                   "<gml:exterior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>0 0 -100 0 10 -100 10 10 -100 10 0 -100 0 0 -100</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:exterior>"
                                   "</gml:Polygon>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Polygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Polygon );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 1 );
  QCOMPARE( poly[0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testMultiLineStringGML3()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:MultiCurve srsName='EPSG:27700'>"
                                   "<gml:curveMember>"
                                   "<gml:LineString>"
                                   "<gml:posList>10 20 30 40</gml:posList>"
                                   "</gml:LineString>"
                                   "</gml:curveMember>"
                                   "<gml:curveMember>"
                                   "<gml:LineString>"
                                   "<gml:posList>50 60 70 80 90 100</gml:posList>"
                                   "</gml:LineString>"
                                   "</gml:curveMember>"
                                   "</gml:MultiCurve>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::MultiLineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::MultiLineString );
  QgsMultiPolylineXY multi = features[0].first->geometry().asMultiPolyline();
  QCOMPARE( multi.size(), 2 );
  QCOMPARE( multi[0].size(), 2 );
  QCOMPARE( multi[0][0], QgsPointXY( 10, 20 ) );
  QCOMPARE( multi[0][1], QgsPointXY( 30, 40 ) );
  QCOMPARE( multi[1].size(), 3 );
  delete features[0].first;
}

void TestQgsGML::testMultiPolygonGML3()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:MultiSurface srsName='EPSG:27700'>"
                                   "<gml:surfaceMember>"
                                   "<gml:Polygon srsName='EPSG:27700'>"
                                   "<gml:exterior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>0 0 0 10 10 10 10 0 0 0</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:exterior>"
                                   "</gml:Polygon>"
                                   "</gml:surfaceMember>"
                                   "<gml:surfaceMember>"
                                   "<gml:Polygon srsName='EPSG:27700'>"
                                   "<gml:exterior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>0 0 0 10 10 10 10 0 0 0</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:exterior>"
                                   "</gml:Polygon>"
                                   "</gml:surfaceMember>"
                                   "</gml:MultiSurface>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::MultiPolygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 2 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_2()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:wfs='http://wfs' "
                                   "xmlns:gml='http://www.opengis.net/gml/3.2'>"
                                   "<wfs:member>"
                                   "<myns:mytypename gml:id='mytypename.1'>" /* First use of gml: */
                                   "<myns:mygeom>"
                                   "<gml:Point gml:id='geomid' srsName='urn:ogc:def:crs:EPSG::27700'>"
                                   "<gml:pos>10 20</gml:pos>"
                                   "</gml:Point>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</wfs:member>"
                                   "</wfs:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( gmlParser.getEPSGCode(), 27700 );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  delete features[0].first;
}

void TestQgsGML::testBoundingBoxGML2()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<gml:boundedBy>"
                                   "<gml:Box srsName='EPSG:27700'>"
                                   "<gml:coordinates>0,0 10,10</gml:coordinates>"
                                   "</gml:Box>"
                                   "</gml:boundedBy>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  //QCOMPARE(gmlParser.wkbType(), QgsWkbTypes::Polygon);
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Polygon );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 1 );
  QCOMPARE( poly[0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testBoundingBoxGML3()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<gml:boundedBy>"
                                   "<gml:Envelope srsName='EPSG:27700'>"
                                   "<gml:lowerCorner>0 0</gml:lowerCorner>"
                                   "<gml:upperCorner>10 10</gml:upperCorner>"
                                   "</gml:Envelope>"
                                   "</gml:boundedBy>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  //QCOMPARE(gmlParser.wkbType(), QgsWkbTypes::Polygon);
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Polygon );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 1 );
  QCOMPARE( poly[0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testNumberMatchedNumberReturned()
{
  QgsFields fields;
  // No attribute
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                     "xmlns:wfs='http://wfs' "
                                     "xmlns:gml='http://www.opengis.net/gml'>"
                                     "</wfs:FeatureCollection>" ), true ), true );
    QCOMPARE( gmlParser.numberReturned(), -1 );
    QCOMPARE( gmlParser.numberMatched(), -1 );
  }
  // Valid numberOfFeatures
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                     "numberOfFeatures='1' "
                                     "xmlns:wfs='http://wfs' "
                                     "xmlns:gml='http://www.opengis.net/gml'>"
                                     "</wfs:FeatureCollection>" ), true ), true );
    QCOMPARE( gmlParser.numberReturned(), 1 );
  }
  // Invalid numberOfFeatures
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                     "numberOfFeatures='invalid' "
                                     "xmlns:wfs='http://wfs' "
                                     "xmlns:gml='http://www.opengis.net/gml'>"
                                     "</wfs:FeatureCollection>" ), true ), true );
    QCOMPARE( gmlParser.numberReturned(), -1 );
  }
  // Valid numberReturned
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                     "numberReturned='1' "
                                     "xmlns:wfs='http://wfs' "
                                     "xmlns:gml='http://www.opengis.net/gml'>"
                                     "</wfs:FeatureCollection>" ), true ), true );
    QCOMPARE( gmlParser.numberReturned(), 1 );
  }
  // Invalid numberReturned
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                     "numberReturned='invalid' "
                                     "xmlns:wfs='http://wfs' "
                                     "xmlns:gml='http://www.opengis.net/gml'>"
                                     "</wfs:FeatureCollection>" ), true ), true );
    QCOMPARE( gmlParser.numberReturned(), -1 );
  }
  // Valid numberMatched
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                     "numberMatched='1' "
                                     "xmlns:wfs='http://wfs' "
                                     "xmlns:gml='http://www.opengis.net/gml'>"
                                     "</wfs:FeatureCollection>" ), true ), true );
    QCOMPARE( gmlParser.numberMatched(), 1 );
  }
  // numberMatched=unknown
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                     "numberMatched='unknown' "
                                     "xmlns:wfs='http://wfs' "
                                     "xmlns:gml='http://www.opengis.net/gml'>"
                                     "</wfs:FeatureCollection>" ), true ), true );
    QCOMPARE( gmlParser.numberMatched(), -1 );
  }
}

void TestQgsGML::testException()
{
  QgsGmlStreamingParser gmlParser( ( QString() ), ( QString() ), QgsFields() );
  QCOMPARE( gmlParser.processData( QByteArray( "<ows:ExceptionReport xmlns:ows='http://www.opengis.net/ows/1.1' version='2.0.0'>"
                                   "  <ows:Exception exceptionCode='NoApplicableCode'>"
                                   "    <ows:ExceptionText>my_exception</ows:ExceptionText>"
                                   "  </ows:Exception>"
                                   "</ows:ExceptionReport>" ), true ), true );
  QCOMPARE( gmlParser.isException(), true );
  QCOMPARE( gmlParser.exceptionText(), QString( "my_exception" ) );
}

void TestQgsGML::testTuple()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "my_first_attr" ), QVariant::Int, QStringLiteral( "int" ) ) );
  fields.append( QgsField( QStringLiteral( "my_second_attr" ), QVariant::Int, QStringLiteral( "int" ) ) );
  QList<QgsGmlStreamingParser::LayerProperties> layerProperties;
  QgsGmlStreamingParser::LayerProperties prop;
  prop.mName = QStringLiteral( "ns:firstlayer" );
  prop.mGeometryAttribute = QStringLiteral( "geom" );
  layerProperties.append( prop );
  prop.mName = QStringLiteral( "ns:secondlayer" );
  prop.mGeometryAttribute = QStringLiteral( "geom" );
  layerProperties.append( prop );
  QMap< QString, QPair<QString, QString> > mapFieldNameToSrcLayerNameFieldName;
  mapFieldNameToSrcLayerNameFieldName.insert( QStringLiteral( "my_first_attr" ), QPair<QString, QString>( QStringLiteral( "ns:firstlayer" ), QStringLiteral( "a" ) ) );
  mapFieldNameToSrcLayerNameFieldName.insert( QStringLiteral( "my_second_attr" ), QPair<QString, QString>( QStringLiteral( "ns:secondlayer" ), QStringLiteral( "a" ) ) );
  QgsGmlStreamingParser gmlParser( layerProperties, fields, mapFieldNameToSrcLayerNameFieldName );
  QCOMPARE( gmlParser.processData( QByteArray(
                                     "<wfs:FeatureCollection numberMatched=\"unknown\" numberReturned=\"1\" "
                                     "xmlns:wfs='http://wfs' "
                                     "xmlns:ns='http://ns' "
                                     "xmlns:gml='http://www.opengis.net/gml/3.2'>"
                                     "<wfs:member>"
                                     "<wfs:Tuple>"
                                     "<wfs:member>"
                                     "<ns:firstlayer gml:id=\"firstlayer.1\">"
                                     "<ns:a>1</ns:a>"
                                     "<ns:geom><gml:Point gml:id='geomid' srsName='urn:ogc:def:crs:EPSG::27700'><gml:pos>10 20</gml:pos></gml:Point></ns:geom>"
                                     "</ns:firstlayer>"
                                     "</wfs:member>"
                                     "<wfs:member>"
                                     "<ns:secondlayer gml:id=\"secondlayer.1\">"
                                     "<ns:a>2</ns:a>"
                                     "<ns:geom><gml:Point gml:id='geomid' srsName='urn:ogc:def:crs:EPSG::32632'><gml:pos>20 40</gml:pos></gml:Point></ns:geom>"
                                     "</ns:secondlayer>"
                                     "</wfs:member>"
                                     "</wfs:Tuple>"
                                     "</wfs:member>"
                                     "</wfs:FeatureCollection>"
                                   ), true ), true );
  QCOMPARE( gmlParser.isException(), false );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( gmlParser.getEPSGCode(), 27700 );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QCOMPARE( features[0].first->attributes().size(), 2 );
  QCOMPARE( features[0].first->attributes().at( 0 ), QVariant( 1 ) );
  QCOMPARE( features[0].first->attributes().at( 1 ), QVariant( 2 ) );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "firstlayer.1|secondlayer.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  delete features[0].first;
}

void TestQgsGML::testRenamedFields()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "my_first_attr" ), QVariant::Int, QStringLiteral( "int" ) ) );
  QList<QgsGmlStreamingParser::LayerProperties> layerProperties;
  QgsGmlStreamingParser::LayerProperties prop;
  prop.mName = QStringLiteral( "ns:mylayer" );
  prop.mGeometryAttribute = QStringLiteral( "geom" );
  layerProperties.append( prop );
  QMap< QString, QPair<QString, QString> > mapFieldNameToSrcLayerNameFieldName;
  mapFieldNameToSrcLayerNameFieldName.insert( QStringLiteral( "my_first_attr" ), QPair<QString, QString>( QStringLiteral( "ns:mylayer" ), QStringLiteral( "b" ) ) );
  QgsGmlStreamingParser gmlParser( layerProperties, fields, mapFieldNameToSrcLayerNameFieldName );
  QCOMPARE( gmlParser.processData( QByteArray(
                                     "<wfs:FeatureCollection numberMatched=\"unknown\" numberReturned=\"1\" "
                                     "xmlns:wfs='http://wfs' "
                                     "xmlns:ns='http://ns' "
                                     "xmlns:gml='http://www.opengis.net/gml/3.2'>"
                                     "<wfs:member>"
                                     "<ns:mylayer gml:id=\"mylayer.1\">"
                                     "<ns:a>1</ns:a>"
                                     "<ns:b>2</ns:b>"
                                     "<ns:geom><gml:Point gml:id='geomid' srsName='urn:ogc:def:crs:EPSG::27700'><gml:pos>10 20</gml:pos></gml:Point></ns:geom>"
                                     "</ns:mylayer>"
                                     "</wfs:member>"
                                     "</wfs:FeatureCollection>"
                                   ), true ), true );
  QCOMPARE( gmlParser.isException(), false );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( gmlParser.getEPSGCode(), 27700 );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QCOMPARE( features[0].first->attributes().size(), 1 );
  QCOMPARE( features[0].first->attributes().at( 0 ), QVariant( 2 ) );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mylayer.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  delete features[0].first;
}

void TestQgsGML::testTruncatedResponse()
{
  QgsGmlStreamingParser gmlParser( ( QString() ), ( QString() ), QgsFields() );
  QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                   "xmlns:wfs='http://wfs' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<wfs:truncatedResponse/>"
                                   "</wfs:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.isTruncatedResponse(), true );
}

void TestQgsGML::testPartialFeature()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:Point srsName='EPSG:27700'>"
                                   "<gml:coordinates>10,20</gml:coordinates>"
                                             ), true ), false );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 0 );
}

void TestQgsGML::testThroughOGRGeometry()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:CompositeSurface srsName='EPSG:27700'><gml:surfaceMember>"
                                   "<gml:Polygon srsName='EPSG:27700'>"
                                   "<gml:exterior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>0 0 0 10 10 10 10 0 0 0</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:exterior>"
                                   "</gml:Polygon>"
                                   "</gml:surfaceMember></gml:CompositeSurface>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( gmlParser.getEPSGCode(), 27700 );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 1 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testThroughOGRGeometry_urn_EPSG_4326()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "mytypename" ), QStringLiteral( "mygeom" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:mytypename fid='mytypename.1'>"
                                   "<myns:mygeom>"
                                   "<gml:CompositeSurface srsName='urn:ogc:def:crs:EPSG::4326'><gml:surfaceMember>"
                                   "<gml:Polygon srsName='urn:ogc:def:crs:EPSG::4326'>"
                                   "<gml:exterior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>49 2 49 3 59 3 49 2</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:exterior>"
                                   "</gml:Polygon>"
                                   "</gml:surfaceMember></gml:CompositeSurface>"
                                   "</myns:mygeom>"
                                   "</myns:mytypename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( gmlParser.getEPSGCode(), 4326 );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 1 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 4 );
  QgsDebugMsg( multi[0][0][0].toString() );
  QCOMPARE( multi[0][0][0], QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testAccents()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QString::fromUtf8( QByteArray( "my\xc3\xa9typename" ) ),
                                   QString::fromUtf8( QByteArray( "my\xc3\xa9geom" ) ),
                                   fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:my\xc3\xa9typename fid='mytypename.1'>"
                                   "<myns:my\xc3\xa9geom>"
                                   "<gml:MultiSurface srsName='EPSG:27700'>"
                                   "<gml:surfaceMember>"
                                   "<gml:Polygon srsName='EPSG:27700'>"
                                   "<gml:exterior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>0 0 0 10 10 10 10 0 0 0</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:exterior>"
                                   "</gml:Polygon>"
                                   "</gml:surfaceMember>"
                                   "<gml:surfaceMember>"
                                   "<gml:Polygon srsName='EPSG:27700'>"
                                   "<gml:exterior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>0 0 0 10 10 10 10 0 0 0</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:exterior>"
                                   "</gml:Polygon>"
                                   "</gml:surfaceMember>"
                                   "</gml:MultiSurface>"
                                   "</myns:my\xc3\xa9geom>"
                                   "</myns:my\xc3\xa9typename>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::MultiPolygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 2 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testSameTypeameAsGeomName()
{
  QgsFields fields;
  QgsGmlStreamingParser gmlParser( QStringLiteral( "foo" ), QStringLiteral( "foo" ), fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                   "xmlns:myns='http://myns' "
                                   "xmlns:gml='http://www.opengis.net/gml'>"
                                   "<gml:featureMember>"
                                   "<myns:foo fid='foo.1'>"
                                   "<myns:foo>"
                                   "<gml:MultiSurface srsName='EPSG:27700'>"
                                   "<gml:surfaceMember>"
                                   "<gml:Polygon srsName='EPSG:27700'>"
                                   "<gml:exterior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>0 0 0 10 10 10 10 0 0 0</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:exterior>"
                                   "</gml:Polygon>"
                                   "</gml:surfaceMember>"
                                   "<gml:surfaceMember>"
                                   "<gml:Polygon srsName='EPSG:27700'>"
                                   "<gml:exterior>"
                                   "<gml:LinearRing>"
                                   "<gml:posList>0 0 0 10 10 10 10 0 0 0</gml:posList>"
                                   "</gml:LinearRing>"
                                   "</gml:exterior>"
                                   "</gml:Polygon>"
                                   "</gml:surfaceMember>"
                                   "</gml:MultiSurface>"
                                   "</myns:foo>"
                                   "</myns:foo>"
                                   "</gml:featureMember>"
                                   "</myns:FeatureCollection>" ), true ), true );
  QCOMPARE( gmlParser.wkbType(), QgsWkbTypes::MultiPolygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), QgsWkbTypes::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 2 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 5 );
  delete features[0].first;
}

QGSTEST_MAIN( TestQgsGML )
#include "testqgsgml.moc"
