
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

#include <QTemporaryFile>
#include <QTextCodec>
#include <QUrl>

//qgis includes...
#include <qgsgeometry.h>
#include <qgsgml.h>
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"

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
    void testPointZGML2();
    void testLineStringGML2();
    void testPolygonGML2();
    void testMultiPointGML2();
    void testMultiLineStringGML2();
    void testMultiPolygonGML2();
    void testPointGML3();
    void testPointGML3_EPSG_4326();
    void testPointGML3_uri_EPSG_4326();
    void testPointGML3_urn_EPSG_4326();
    void testPointGML3_EPSG_4326_honour_EPSG();
    void testPointGML3_EPSG_4326_honour_EPSG_invert();
    void testLineStringGML3();
    void testLineStringGML3_LineStringSegment();
    void testLineStringGML3_pos();
    void testPolygonGML3();
    void testPolygonGML3_srsDimension_on_Polygon();
    void testPolygonGML3_srsDimension_on_posList();
    void testPolygonGML3_pos();
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
    void testUnknownEncoding_data();
    void testUnknownEncoding();
    void testUnhandledEncoding();
    void testXPath();
    void testZ();
    void testZ_data();
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
  fields.append( QgsField( u"intfield"_s, QMetaType::Type::Int, u"int"_s ) );
  fields.append( QgsField( u"nillablefield"_s, QMetaType::Type::Int, u"nillablefield"_s ) );
  QgsGml gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  Qgis::WkbType wkbType;
  QTemporaryFile tmpFile;
  QVERIFY( tmpFile.open() );
  tmpFile.write( data1.toLatin1() );
  tmpFile.flush();
  QCOMPARE( gmlParser.getFeatures( QUrl::fromLocalFile( tmpFile.fileName() ).toString(), &wkbType ), 0 );
  QCOMPARE( wkbType, Qgis::WkbType::Point );
  QMap<QgsFeatureId, QgsFeature *> featureMaps = gmlParser.featuresMap();
  QCOMPARE( featureMaps.size(), 1 );
  QCOMPARE( gmlParser.idsMap().size(), 1 );
  QCOMPARE( gmlParser.crs().authid(), QString( "EPSG:27700" ) );
  QCOMPARE( featureMaps[0]->attribute( u"intfield"_s ).toInt(), 1 );
  QVERIFY( featureMaps[0]->attribute( u"nillablefield"_s ).isNull() );
  delete featureMaps[0];
}

void TestQgsGML::testFromByteArray()
{
  QgsFields fields;
  fields.append( QgsField( u"intfield"_s, QMetaType::Type::Int, u"int"_s ) );
  fields.append( QgsField( u"nillablefield"_s, QMetaType::Type::Int, u"nillablefield"_s ) );
  QgsGml gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  Qgis::WkbType wkbType;
  QCOMPARE( gmlParser.getFeatures( data1.toLatin1(), &wkbType ), 0 );
  QMap<QgsFeatureId, QgsFeature *> featureMaps = gmlParser.featuresMap();
  QCOMPARE( featureMaps.size(), 1 );
  QVERIFY( featureMaps.constFind( 0 ) != featureMaps.constEnd() );
  QCOMPARE( featureMaps[0]->attributes().size(), 2 );
  QMap<QgsFeatureId, QString> idsMap = gmlParser.idsMap();
  QVERIFY( idsMap.constFind( 0 ) != idsMap.constEnd() );
  QCOMPARE( idsMap[0], QString( "mytypename.1" ) );
  QCOMPARE( featureMaps[0]->attribute( u"intfield"_s ).toInt(), 1 );
  QVERIFY( featureMaps[0]->attribute( u"nillablefield"_s ).isNull() );
  delete featureMaps[0];
}

void TestQgsGML::testStreamingParser()
{
  QgsFields fields;
  fields.append( QgsField( u"intfield"_s, QMetaType::Type::Int, u"int"_s ) );
  fields.append( QgsField( u"nillablefield"_s, QMetaType::Type::Int, u"nillablefield"_s ) );
  fields.append( QgsField( u"longfield"_s, QMetaType::Type::LongLong, u"longlong"_s ) );
  fields.append( QgsField( u"doublefield"_s, QMetaType::Type::Double, u"double"_s ) );
  fields.append( QgsField( u"strfield"_s, QMetaType::Type::QString, u"string"_s ) );
  fields.append( QgsField( u"datetimefield"_s, QMetaType::Type::QDateTime, u"datetime"_s ) );
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.processData( data1.mid( 0, data1.size() / 2 ).toLatin1(), false ), true );
  QCOMPARE( gmlParser.getAndStealReadyFeatures().size(), 0 );
  QCOMPARE( gmlParser.processData( data1.mid( data1.size() / 2 ).toLatin1(), true ), true );
  QCOMPARE( gmlParser.isException(), false );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QCOMPARE( features[0].first->attributes().size(), 6 );
  QCOMPARE( features[0].first->attributes().at( 0 ), QVariant( 1 ) );
  QCOMPARE( features[0].first->attributes().at( 1 ), QVariant() );
  QCOMPARE( features[0].first->attributes().at( 2 ), QVariant( Q_INT64_C( 1234567890123 ) ) );
  QCOMPARE( features[0].first->attributes().at( 3 ), QVariant( 1.23 ) );
  QCOMPARE( features[0].first->attributes().at( 4 ), QVariant( "foo" ) );
  QCOMPARE( features[0].first->attributes().at( 5 ), QVariant( QDateTime( QDate( 2016, 4, 10 ), QTime( 12, 34, 56, 789 ), Qt::UTC ) ) );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( gmlParser.getAndStealReadyFeatures().size(), 0 );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:27700"_s );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  delete features[0].first;
}

void TestQgsGML::testStreamingParserInvalidGML()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.processData( "<FeatureCollection>", true ), false );
  QCOMPARE( gmlParser.getAndStealReadyFeatures().size(), 0 );
}

void TestQgsGML::testPointGML2()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  delete features[0].first;
}

void TestQgsGML::testPointZGML2()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                               "xmlns:myns='http://myns' "
                                               "xmlns:gml='http://www.opengis.net/gml'>"
                                               "<gml:featureMember>"
                                               "<myns:mytypename fid='mytypename.1'>"
                                               "<myns:mygeom>"
                                               "<gml:Point srsName='EPSG:4326'>"
                                               "<gml:coordinates>1,2,3</gml:coordinates>"
                                               "</gml:Point>"
                                               "</myns:mygeom>"
                                               "</myns:mytypename>"
                                               "</gml:featureMember>"
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::PointZ );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::PointZ );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPoint( 1, 2, 3 ) );
  delete features[0].first;
}

void TestQgsGML::testLineStringGML2()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::LineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::LineString );
  QgsPolylineXY line = features[0].first->geometry().asPolyline();
  QCOMPARE( line.size(), 2 );
  QCOMPARE( line[0], QgsPointXY( 10, 20 ) );
  QCOMPARE( line[1], QgsPointXY( 30, 40 ) );
  delete features[0].first;
}

void TestQgsGML::testPolygonGML2()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Polygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Polygon );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 2 );
  QCOMPARE( poly[0].size(), 5 );
  QCOMPARE( poly[1].size(), 4 );
  delete features[0].first;
}

void TestQgsGML::testMultiPointGML2()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::MultiPoint );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::MultiPoint );
  QgsMultiPointXY multi = features[0].first->geometry().asMultiPoint();
  QCOMPARE( multi.size(), 2 );
  QCOMPARE( multi[0], QgsPointXY( 10, 20 ) );
  QCOMPARE( multi[1], QgsPointXY( 30, 40 ) );
  delete features[0].first;
}

void TestQgsGML::testMultiLineStringGML2()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::MultiLineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::MultiLineString );
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
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::MultiPolygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 1 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testPointGML3()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:27700"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_EPSG_4326()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:4326"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_uri_EPSG_4326()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                               "xmlns:myns='http://myns' "
                                               "xmlns:gml='http://www.opengis.net/gml'>"
                                               "<gml:featureMember>"
                                               "<myns:mytypename gml:id='mytypename.1'>"
                                               "<myns:mygeom>"
                                               "<gml:Point srsName='http://www.opengis.net/def/crs/EPSG/0/4326'>"
                                               "<gml:pos>49 2</gml:pos>"
                                               "</gml:Point>"
                                               "</myns:mygeom>"
                                               "</myns:mytypename>"
                                               "</gml:featureMember>"
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:4326"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_urn_EPSG_4326()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:4326"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_EPSG_4326_honour_EPSG()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields, QgsGmlStreamingParser::Honour_EPSG );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:4326"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_EPSG_4326_honour_EPSG_invert()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields, QgsGmlStreamingParser::Honour_EPSG, true );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:4326"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testLineStringGML3()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::LineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::LineString );
  QgsPolylineXY line = features[0].first->geometry().asPolyline();
  QCOMPARE( line.size(), 2 );
  QCOMPARE( line[0], QgsPointXY( 10, 20 ) );
  QCOMPARE( line[1], QgsPointXY( 30, 40 ) );
  delete features[0].first;
}

void TestQgsGML::testLineStringGML3_LineStringSegment()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::LineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::LineString );
  QgsPolylineXY line = features[0].first->geometry().asPolyline();
  QCOMPARE( line.size(), 2 );
  QCOMPARE( line[0], QgsPointXY( 10, 20 ) );
  QCOMPARE( line[1], QgsPointXY( 30, 40 ) );
  delete features[0].first;
}

void TestQgsGML::testLineStringGML3_pos()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                               "xmlns:myns='http://myns' "
                                               "xmlns:gml='http://www.opengis.net/gml'>"
                                               "<gml:featureMember>"
                                               "<myns:mytypename fid='mytypename.1'>"
                                               "<myns:mygeom>"
                                               "<gml:Curve srsName='EPSG:27700'><gml:segments><gml:LineStringSegment><gml:pos>10 20</gml:pos><gml:pos>30 40</gml:pos></gml:LineStringSegment></gml:segments></gml:Curve>"
                                               "</myns:mygeom>"
                                               "</myns:mytypename>"
                                               "</gml:featureMember>"
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::LineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::LineString );
  QgsPolylineXY line = features[0].first->geometry().asPolyline();
  QCOMPARE( line.size(), 2 );
  QCOMPARE( line[0], QgsPointXY( 10, 20 ) );
  QCOMPARE( line[1], QgsPointXY( 30, 40 ) );
  delete features[0].first;
}

void TestQgsGML::testPolygonGML3()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Polygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Polygon );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 2 );
  QCOMPARE( poly[0].size(), 5 );
  QCOMPARE( poly[1].size(), 4 );
  delete features[0].first;
}

void TestQgsGML::testPolygonGML3_srsDimension_on_Polygon()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::PolygonZ );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::PolygonZ );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 1 );
  QCOMPARE( poly[0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testPolygonGML3_srsDimension_on_posList()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                               "xmlns:myns='http://myns' "
                                               "xmlns:gml='http://www.opengis.net/gml'>"
                                               "<gml:featureMember>"
                                               "<myns:mytypename fid='mytypename.1'>"
                                               "<myns:mygeom>"
                                               "<gml:Polygon srsName='EPSG:27700'>"
                                               "<gml:exterior>"
                                               "<gml:LinearRing>"
                                               "<gml:posList srsDimension='3'>0 0 -100 0 10 -100 10 10 -100 10 0 -100 0 0 -100</gml:posList>"
                                               "</gml:LinearRing>"
                                               "</gml:exterior>"
                                               "</gml:Polygon>"
                                               "</myns:mygeom>"
                                               "</myns:mytypename>"
                                               "</gml:featureMember>"
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::PolygonZ );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::PolygonZ );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 1 );
  QCOMPARE( poly[0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testPolygonGML3_pos()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                               "xmlns:myns='http://myns' "
                                               "xmlns:gml='http://www.opengis.net/gml'>"
                                               "<gml:featureMember>"
                                               "<myns:mytypename fid='mytypename.1'>"
                                               "<myns:mygeom>"
                                               "<gml:Polygon srsName='EPSG:27700'>"
                                               "<gml:exterior>"
                                               "<gml:LinearRing>"
                                               "<gml:pos srsDimension='3'>0 0 -100 </gml:pos>"
                                               "<gml:pos srsDimension='3'>0 10 -100</gml:pos>"
                                               "<gml:pos srsDimension='3'>10 10 -100</gml:pos>"
                                               "<gml:pos srsDimension='3'>10 0 -100</gml:pos>"
                                               "<gml:pos srsDimension='3'>0 0 -100</gml:pos>"
                                               "</gml:LinearRing>"
                                               "</gml:exterior>"
                                               "</gml:Polygon>"
                                               "</myns:mygeom>"
                                               "</myns:mytypename>"
                                               "</gml:featureMember>"
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::PolygonZ );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::PolygonZ );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 1 );
  QCOMPARE( poly[0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testMultiLineStringGML3()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::MultiLineString );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::MultiLineString );
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
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::MultiPolygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 2 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testPointGML3_2()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</wfs:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:27700"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mytypename.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  delete features[0].first;
}

void TestQgsGML::testBoundingBoxGML2()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  //QCOMPARE(gmlParser.wkbType(), QgsWkbTypes::Polygon);
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Polygon );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 1 );
  QCOMPARE( poly[0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testBoundingBoxGML3()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  //QCOMPARE(gmlParser.wkbType(), QgsWkbTypes::Polygon);
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Polygon );
  QgsPolygonXY poly = features[0].first->geometry().asPolygon();
  QCOMPARE( poly.size(), 1 );
  QCOMPARE( poly[0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testNumberMatchedNumberReturned()
{
  const QgsFields fields;
  // No attribute
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                                 "xmlns:wfs='http://wfs' "
                                                 "xmlns:gml='http://www.opengis.net/gml'>"
                                                 "</wfs:FeatureCollection>" ),
                                     true ),
              true );
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
                                                 "</wfs:FeatureCollection>" ),
                                     true ),
              true );
    QCOMPARE( gmlParser.numberReturned(), 1 );
  }
  // Invalid numberOfFeatures
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                                 "numberOfFeatures='invalid' "
                                                 "xmlns:wfs='http://wfs' "
                                                 "xmlns:gml='http://www.opengis.net/gml'>"
                                                 "</wfs:FeatureCollection>" ),
                                     true ),
              true );
    QCOMPARE( gmlParser.numberReturned(), -1 );
  }
  // Valid numberReturned
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                                 "numberReturned='1' "
                                                 "xmlns:wfs='http://wfs' "
                                                 "xmlns:gml='http://www.opengis.net/gml'>"
                                                 "</wfs:FeatureCollection>" ),
                                     true ),
              true );
    QCOMPARE( gmlParser.numberReturned(), 1 );
  }
  // Invalid numberReturned
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                                 "numberReturned='invalid' "
                                                 "xmlns:wfs='http://wfs' "
                                                 "xmlns:gml='http://www.opengis.net/gml'>"
                                                 "</wfs:FeatureCollection>" ),
                                     true ),
              true );
    QCOMPARE( gmlParser.numberReturned(), -1 );
  }
  // Valid numberMatched
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                                 "numberMatched='1' "
                                                 "xmlns:wfs='http://wfs' "
                                                 "xmlns:gml='http://www.opengis.net/gml'>"
                                                 "</wfs:FeatureCollection>" ),
                                     true ),
              true );
    QCOMPARE( gmlParser.numberMatched(), 1 );
  }
  // numberMatched=unknown
  {
    QgsGmlStreamingParser gmlParser( QString(), QString(), fields );
    QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection "
                                                 "numberMatched='unknown' "
                                                 "xmlns:wfs='http://wfs' "
                                                 "xmlns:gml='http://www.opengis.net/gml'>"
                                                 "</wfs:FeatureCollection>" ),
                                     true ),
              true );
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
                                               "</ows:ExceptionReport>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.isException(), true );
  QCOMPARE( gmlParser.exceptionText(), QString( "my_exception" ) );
}

void TestQgsGML::testTuple()
{
  QgsFields fields;
  fields.append( QgsField( u"my_first_attr"_s, QMetaType::Type::Int, u"int"_s ) );
  fields.append( QgsField( u"my_second_attr"_s, QMetaType::Type::Int, u"int"_s ) );
  QList<QgsGmlStreamingParser::LayerProperties> layerProperties;
  QgsGmlStreamingParser::LayerProperties prop;
  prop.mName = u"ns:firstlayer"_s;
  prop.mGeometryAttribute = u"geom"_s;
  layerProperties.append( prop );
  prop.mName = u"ns:secondlayer"_s;
  prop.mGeometryAttribute = u"geom"_s;
  layerProperties.append( prop );
  QMap<QString, QPair<QString, QString>> mapFieldNameToSrcLayerNameFieldName;
  mapFieldNameToSrcLayerNameFieldName.insert( u"my_first_attr"_s, QPair<QString, QString>( u"ns:firstlayer"_s, u"a"_s ) );
  mapFieldNameToSrcLayerNameFieldName.insert( u"my_second_attr"_s, QPair<QString, QString>( u"ns:secondlayer"_s, u"a"_s ) );
  QgsGmlStreamingParser gmlParser( layerProperties, fields, mapFieldNameToSrcLayerNameFieldName );
  QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection numberMatched=\"unknown\" numberReturned=\"1\" "
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
                                               "</wfs:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.isException(), false );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:27700"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QCOMPARE( features[0].first->attributes().size(), 2 );
  QCOMPARE( features[0].first->attributes().at( 0 ), QVariant( 1 ) );
  QCOMPARE( features[0].first->attributes().at( 1 ), QVariant( 2 ) );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "firstlayer.1|secondlayer.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( features[0].first->geometry().asPoint(), QgsPointXY( 10, 20 ) );
  delete features[0].first;
}

void TestQgsGML::testRenamedFields()
{
  QgsFields fields;
  fields.append( QgsField( u"my_first_attr"_s, QMetaType::Type::Int, u"int"_s ) );
  QList<QgsGmlStreamingParser::LayerProperties> layerProperties;
  QgsGmlStreamingParser::LayerProperties prop;
  prop.mName = u"ns:mylayer"_s;
  prop.mGeometryAttribute = u"geom"_s;
  layerProperties.append( prop );
  QMap<QString, QPair<QString, QString>> mapFieldNameToSrcLayerNameFieldName;
  mapFieldNameToSrcLayerNameFieldName.insert( u"my_first_attr"_s, QPair<QString, QString>( u"ns:mylayer"_s, u"b"_s ) );
  QgsGmlStreamingParser gmlParser( layerProperties, fields, mapFieldNameToSrcLayerNameFieldName );
  QCOMPARE( gmlParser.processData( QByteArray( "<wfs:FeatureCollection numberMatched=\"unknown\" numberReturned=\"1\" "
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
                                               "</wfs:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.isException(), false );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Point );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:27700"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QCOMPARE( features[0].first->attributes().size(), 1 );
  QCOMPARE( features[0].first->attributes().at( 0 ), QVariant( 2 ) );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].second, QString( "mylayer.1" ) );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::Point );
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
                                               "</wfs:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.isTruncatedResponse(), true );
}

void TestQgsGML::testPartialFeature()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                               "xmlns:myns='http://myns' "
                                               "xmlns:gml='http://www.opengis.net/gml'>"
                                               "<gml:featureMember>"
                                               "<myns:mytypename fid='mytypename.1'>"
                                               "<myns:mygeom>"
                                               "<gml:Point srsName='EPSG:27700'>"
                                               "<gml:coordinates>10,20</gml:coordinates>" ),
                                   true ),
            false );
  const QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 0 );
}

void TestQgsGML::testThroughOGRGeometry()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                               "xmlns:myns='http://myns' "
                                               "xmlns:gml='http://www.opengis.net/gml'>"
                                               "<gml:featureMember>"
                                               "<myns:mytypename fid='mytypename.1'>"
                                               "<myns:mygeom>"
                                               "<gml:CompositeSurface srsName='EPSG:27700'><gml:surfaceMember>"
                                               "<gml:Polygon gml:id='foo' srsName='EPSG:27700'>"
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Polygon );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:27700"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 1 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testThroughOGRGeometry_urn_EPSG_4326()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.processData( QByteArray( "<myns:FeatureCollection "
                                               "xmlns:myns='http://myns' "
                                               "xmlns:gml='http://www.opengis.net/gml/3.2'>"
                                               "<gml:featureMember>"
                                               "<myns:mytypename fid='mytypename.1'>"
                                               "<myns:mygeom>"
                                               "<gml:CompositeSurface srsName='urn:ogc:def:crs:EPSG::4326'><gml:surfaceMember>"
                                               "<gml:Polygon gml:id='foo' srsName='urn:ogc:def:crs:EPSG::4326'>"
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::Polygon );
  QCOMPARE( QgsCoordinateReferenceSystem::fromOgcWmsCrs( gmlParser.srsName() ).authid(), u"EPSG:4326"_s );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 1 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 4 );
  QgsDebugMsgLevel( multi[0][0][0].toString(), 1 );
  QCOMPARE( multi[0][0][0], QgsPointXY( 2, 49 ) );
  delete features[0].first;
}

void TestQgsGML::testAccents()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( QString::fromUtf8( QByteArray( "my\xc3\xa9typename" ) ), QString::fromUtf8( QByteArray( "my\xc3\xa9geom" ) ), fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::MultiPolygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 2 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testSameTypeameAsGeomName()
{
  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( u"foo"_s, u"foo"_s, fields );
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
                                               "</myns:FeatureCollection>" ),
                                   true ),
            true );
  QCOMPARE( gmlParser.wkbType(), Qgis::WkbType::MultiPolygon );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), Qgis::WkbType::MultiPolygon );
  QgsMultiPolygonXY multi = features[0].first->geometry().asMultiPolygon();
  QCOMPARE( multi.size(), 2 );
  QCOMPARE( multi[0].size(), 1 );
  QCOMPARE( multi[0][0].size(), 5 );
  delete features[0].first;
}

void TestQgsGML::testUnknownEncoding_data()
{
  QTest::addColumn<QString>( "xmlHeader" );
  QTest::addColumn<QByteArray>( "encoding" );

  QTest::newRow( "simple quote" ) << u"<?xml version='1.0' encoding='ISO-8859-15'?>"_s << QByteArrayLiteral( "ISO-8859-15" );
  QTest::newRow( "double quote" ) << u"<?xml version='1.0' encoding=\"ISO-8859-15\"?>"_s << QByteArrayLiteral( "ISO-8859-15" );
  QTest::newRow( "UTF-8" ) << u"<?xml version='1.0' encoding=\"UTF-8\"?>"_s << QByteArrayLiteral( "UTF-8" );
  QTest::newRow( "No header" ) << QString() << QByteArrayLiteral( "UTF-8" );
}

void TestQgsGML::testUnknownEncoding()
{
  QFETCH( QString, xmlHeader );
  QFETCH( QByteArray, encoding );

  Qgis::WkbType wkbType;

  QTextCodec *codec = QTextCodec::codecForName( encoding );

  QByteArray data = codec->fromUnicode(
    QStringLiteral(
      "%1<myns:FeatureCollection "
      "xmlns:myns='http://myns' "
      "xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' "
      "xmlns:gml='http://www.opengis.net/gml'>"
      "<gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>"
      "<gml:featureMember>"
      "<myns:mytypename fid='mytypename.1'>"
      "<myns:strfield>price: 10€</myns:strfield>"
      "<myns:mygeom>"
      "<gml:Point srsName='http://www.opengis.net/gml/srs/epsg.xml#27700'>"
      "<gml:coordinates decimal='.' cs=',' ts=' '>10,20</gml:coordinates>"
      "</gml:Point>"
      "</myns:mygeom>"
      "</myns:mytypename>"
      "</gml:featureMember>"
      "</myns:FeatureCollection>"
    )
      .arg( xmlHeader )
  );

  QgsFields fields;
  fields.append( QgsField( u"strfield"_s, QMetaType::Type::QString, u"string"_s ) );

  {
    QgsGml gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
    QCOMPARE( gmlParser.getFeatures( data, &wkbType ), 0 );
    QMap<QgsFeatureId, QgsFeature *> featureMaps = gmlParser.featuresMap();
    QCOMPARE( featureMaps.size(), 1 );
    QVERIFY( featureMaps.constFind( 0 ) != featureMaps.constEnd() );
    QCOMPARE( featureMaps[0]->attributes().size(), 1 );
    QCOMPARE( featureMaps[0]->attribute( u"strfield"_s ).toString(), QString( "price: 10€" ) );
    delete featureMaps[0];
  }

  {
    QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
    QCOMPARE( gmlParser.processData( data.mid( 0, data.size() / 2 ), false ), true );
    QCOMPARE( gmlParser.getAndStealReadyFeatures().size(), 0 );
    QCOMPARE( gmlParser.processData( data.mid( data.size() / 2 ), true ), true );
    QCOMPARE( gmlParser.isException(), false );
    QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
    QCOMPARE( features.size(), 1 );
    QCOMPARE( features[0].first->attributes().size(), 1 );
    QCOMPARE( features[0].first->attribute( u"strfield"_s ).toString(), QString( "price: 10€" ) );
    delete features[0].first;
  }
}

void TestQgsGML::testUnhandledEncoding()
{
  Qgis::WkbType wkbType;

  QString data = QStringLiteral(
    "<?xml version='1.0' encoding='my-unexisting-encoding'?>"
    "<myns:FeatureCollection "
    "xmlns:myns='http://myns' "
    "xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' "
    "xmlns:gml='http://www.opengis.net/gml'>"
    "<gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>"
    "<gml:featureMember>"
    "<myns:mytypename fid='mytypename.1'>"
    "<myns:strfield>price: 10€</myns:strfield>"
    "<myns:mygeom>"
    "<gml:Point srsName='http://www.opengis.net/gml/srs/epsg.xml#27700'>"
    "<gml:coordinates decimal='.' cs=',' ts=' '>10,20</gml:coordinates>"
    "</gml:Point>"
    "</myns:mygeom>"
    "</myns:mytypename>"
    "</gml:featureMember>"
    "</myns:FeatureCollection>"
  );

  QgsFields fields;
  fields.append( QgsField( u"strfield"_s, QMetaType::Type::QString, u"string"_s ) );

  QgsGml gmlParser( u"mytypename"_s, u"mygeom"_s, fields );
  QCOMPARE( gmlParser.getFeatures( data.toUtf8(), &wkbType ), 0 );
  QMap<QgsFeatureId, QgsFeature *> featureMaps = gmlParser.featuresMap();
  QCOMPARE( featureMaps.size(), 0 );
}

void TestQgsGML::testXPath()
{
  QString data = QStringLiteral(
    "<?xml version='1.0' encoding='utf-8'?>"
    "<myns:FeatureCollection "
    "xmlns:myns='http://myns' "
    "xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' "
    "xmlns:gml='http://www.opengis.net/gml'>"
    "<gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>"
    "<gml:featureMember>"
    "<myns:mytypename fid='mytypename.1' myns:my_attr='my_value'>"
    "<myns:strfield>foo</myns:strfield>"
    "<myns:nested>"
    "<myns:strfield2 attr='attr_val'>bar</myns:strfield2>"
    "<myns:strfield3>baz</myns:strfield3>"
    "</myns:nested>"
    "<myns:complex foo='bar'>"
    "<myns:l1>"
    "<myns:a>x</myns:a>"
    "<myns:b k='v'>"
    "y</myns:b>"
    "<c><k1>1234567890123</k1><k2>12</k2><k3>01</k3><k4>1.25</k4><k5>12345678901234567890123456789</k5></c>"
    "<myns:d>a<i>b</i></myns:d>"
    "<myns:d><i>a</i>b</myns:d>"
    "<myns:d>a<i>b</i>c</myns:d>"
    "<myns:d>x</myns:d>"
    "</myns:l1>"
    "</myns:complex>"
    "<myns:complex2/>"
    "<myns:complex_repeated>foo</myns:complex_repeated>"
    "<myns:complex_repeated>bar</myns:complex_repeated>"
    "<myns:mygeom>"
    "<gml:Point srsName='http://www.opengis.net/gml/srs/epsg.xml#27700'>"
    "<gml:coordinates decimal='.' cs=',' ts=' '>10,20</gml:coordinates>"
    "</gml:Point>"
    "</myns:mygeom>"
    "</myns:mytypename>"
    "</gml:featureMember>"
    "</myns:FeatureCollection>"
  );

  QgsFields fields;
  fields.append( QgsField( u"fid"_s, QMetaType::Type::QString, u"fid"_s ) );
  fields.append( QgsField( u"my_attr"_s, QMetaType::Type::QString, u"string"_s ) );
  fields.append( QgsField( u"strfield"_s, QMetaType::Type::QString, u"string"_s ) );
  fields.append( QgsField( u"nested_strfield2"_s, QMetaType::Type::QString, u"string"_s ) );
  fields.append( QgsField( u"nested_strfield2_attr"_s, QMetaType::Type::QString, u"string"_s ) );
  fields.append( QgsField( u"nested_strfield3"_s, QMetaType::Type::QString, u"string"_s ) );
  fields.append( QgsField( u"complex"_s, QMetaType::Type::QString, u"string"_s ) );
  fields.append( QgsField( u"complex2"_s, QMetaType::Type::QString, u"string"_s ) );
  fields.append( QgsField( u"complex_repeated"_s, QMetaType::Type::QString, u"string"_s ) );

  QgsGmlStreamingParser gmlParser( u"mytypename"_s, u"mygeom"_s, fields );

  QMap<QString, QPair<QString, bool>> mapFieldNameToXPathAndIsNestedContent;
  QMap<QString, QString> mapNamespacePrefixToURI;
  mapFieldNameToXPathAndIsNestedContent[u"fid"_s] = QPair<QString, bool>( u"@fid"_s, false );
  mapFieldNameToXPathAndIsNestedContent[u"my_attr"_s] = QPair<QString, bool>( u"@myns:my_attr"_s, false );
  mapFieldNameToXPathAndIsNestedContent[u"strfield"_s] = QPair<QString, bool>( u"myns:strfield"_s, false );
  mapFieldNameToXPathAndIsNestedContent[u"nested_strfield2"_s] = QPair<QString, bool>( u"myns:nested/myns:strfield2"_s, false );
  mapFieldNameToXPathAndIsNestedContent[u"nested_strfield2_attr"_s] = QPair<QString, bool>( u"myns:nested/myns:strfield2/@attr"_s, false );
  mapFieldNameToXPathAndIsNestedContent[u"nested_strfield3"_s] = QPair<QString, bool>( u"myns:nested/myns:strfield3"_s, false );
  mapFieldNameToXPathAndIsNestedContent[u"complex"_s] = QPair<QString, bool>( u"myns:complex"_s, true );
  mapFieldNameToXPathAndIsNestedContent[u"complex2"_s] = QPair<QString, bool>( u"myns:complex2"_s, true );
  mapFieldNameToXPathAndIsNestedContent[u"complex_repeated"_s] = QPair<QString, bool>( u"myns:complex_repeated"_s, true );
  mapNamespacePrefixToURI[u"myns"_s] = u"http://myns"_s;
  gmlParser.setFieldsXPath( mapFieldNameToXPathAndIsNestedContent, mapNamespacePrefixToURI );

  QCOMPARE( gmlParser.processData( data.toUtf8(), true ), true );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  auto &f = *( features[0].first );
  QCOMPARE( f.attributes().size(), 9 );
  QCOMPARE( f.attribute( u"fid"_s ).toString(), u"mytypename.1"_s );
  QCOMPARE( f.attribute( u"my_attr"_s ).toString(), u"my_value"_s );
  QCOMPARE( f.attribute( u"strfield"_s ).toString(), u"foo"_s );
  QCOMPARE( f.attribute( u"nested_strfield2"_s ).toString(), u"bar"_s );
  QCOMPARE( f.attribute( u"nested_strfield2_attr"_s ).toString(), u"attr_val"_s );
  QCOMPARE( f.attribute( u"nested_strfield3"_s ).toString(), u"baz"_s );
  QCOMPARE( f.attribute( u"complex"_s ).toString(), u"{\"@foo\":\"bar\",\"myns:l1\":{\"c\":{\"k1\":1234567890123,\"k2\":12,\"k3\":\"01\",\"k4\":1.25,\"k5\":\"12345678901234567890123456789\"},\"myns:a\":\"x\",\"myns:b\":{\"@k\":\"v\",\"_text\":\"y\"},\"myns:d\":[{\"_text\":\"a\",\"i\":\"b\"},{\"_text\":\"b\",\"i\":\"a\"},{\"_text\":[\"a\",\"c\"],\"i\":\"b\"},\"x\"]}}"_s );
  QCOMPARE( f.attribute( u"complex2"_s ).toString(), u"{}"_s );
  QCOMPARE( f.attribute( u"complex_repeated"_s ).toString(), u"[\"foo\",\"bar\"]"_s );
}

void TestQgsGML::testZ_data()
{
  QTest::addColumn<QString>( "xml" );
  QTest::addColumn<int>( "expectedWkbType" );
  QTest::addColumn<QString>( "expectedWkt" );

  QTest::newRow( "point with z gml 2" ) << QStringLiteral( R"gml(<gml:Point srsName="EPSG:4326"><gml:coordinates>0,1,2</gml:coordinates></gml:Point>)gml" )
                                        << static_cast<int>( Qgis::WkbType::PointZ )
                                        << u"POINT Z (0 1 2)"_s;

  QTest::newRow( "point with z gml 3" ) << QStringLiteral( R"gml(<gml:Point srsName="EPSG:4326"><gml:pos srsDimension="3">0 1 2</gml:pos></gml:Point>)gml" )
                                        << static_cast<int>( Qgis::WkbType::PointZ )
                                        << u"POINT Z (0 1 2)"_s;

  // Note: this is not supported "point with z gml 3 no srsDimension" <gml:Point srsName="EPSG:4326"><gml:pos>0 1 2</gml:pos></gml:Point>

  QTest::newRow( "linestring with z gml 2" ) << QStringLiteral( R"gml(<gml:LineString srsName="EPSG:4326"><gml:coordinates>0,1,2 3,4,5</gml:coordinates></gml:LineString>)gml" )
                                             << static_cast<int>( Qgis::WkbType::LineStringZ )
                                             << u"LINESTRING Z (0 1 2, 3 4 5)"_s;

  QTest::newRow( "linestring with z gml 3" ) << QStringLiteral( R"gml(<gml:Curve srsName="EPSG:4326"><gml:segments><gml:LineStringSegment><gml:posList srsDimension="3">0 1 2 3 4 5</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve>)gml" )
                                             << static_cast<int>( Qgis::WkbType::LineStringZ )
                                             << u"LINESTRING Z (0 1 2, 3 4 5)"_s;

  QTest::newRow( "linestring with z gml 3 with implicit srsDimension" ) << QStringLiteral( R"gml(<gml:LineString srsName="EPSG:4979"><gml:posList>0 1 2 3 4 5</gml:posList></gml:LineString>)gml" )
                                                                        << static_cast<int>( Qgis::WkbType::LineStringZ )
                                                                        << u"LINESTRING Z (0 1 2, 3 4 5)"_s;

  QTest::newRow( "polygon with z gml 2" ) << QStringLiteral( R"gml(<gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,1,2 3,4,5 6,7,8 0,1,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>)gml" )
                                          << static_cast<int>( Qgis::WkbType::PolygonZ )
                                          << u"POLYGON Z ((0 1 2, 3 4 5, 6 7 8, 0 1 2))"_s;

  QTest::newRow( "polygon with z gml 3" ) << QStringLiteral( R"gml(<gml:Polygon srsName="EPSG:4326"><gml:exterior><gml:LinearRing><gml:posList srsDimension="3">0 1 2 3 4 5 6 7 8 0 1 2</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon>)gml" )
                                          << static_cast<int>( Qgis::WkbType::PolygonZ )
                                          << u"POLYGON Z ((0 1 2, 3 4 5, 6 7 8, 0 1 2))"_s;

  // Multi-geometries
  QTest::newRow( "multipoint with z gml 2" ) << QStringLiteral( R"gml(<gml:MultiPoint srsName="EPSG:4326"><gml:pointMember><gml:Point><gml:coordinates>0,1,2</gml:coordinates></gml:Point></gml:pointMember><gml:pointMember><gml:Point><gml:coordinates>3,4,5</gml:coordinates></gml:Point></gml:pointMember></gml:MultiPoint>)gml" )
                                             << static_cast<int>( Qgis::WkbType::MultiPointZ )
                                             << u"MULTIPOINT Z ((0 1 2),(3 4 5))"_s;

  QTest::newRow( "multipoint with z gml 3" ) << QStringLiteral( R"gml(<gml:MultiPoint srsName="EPSG:4326"><gml:pointMember><gml:Point><gml:pos srsDimension="3">0 1 2</gml:pos></gml:Point></gml:pointMember><gml:pointMember><gml:Point><gml:pos srsDimension="3">3 4 5</gml:pos></gml:Point></gml:pointMember></gml:MultiPoint>)gml" )
                                             << static_cast<int>( Qgis::WkbType::MultiPointZ )
                                             << u"MULTIPOINT Z ((0 1 2),(3 4 5))"_s;

  QTest::newRow( "multilinestring with z gml 2" ) << QStringLiteral( R"gml(<gml:MultiLineString srsName="EPSG:4326"><gml:lineStringMember><gml:LineString><gml:coordinates>0,1,2 3,4,5</gml:coordinates></gml:LineString></gml:lineStringMember><gml:lineStringMember><gml:LineString><gml:coordinates>6,7,8 9,10,11</gml:coordinates></gml:LineString></gml:lineStringMember></gml:MultiLineString>)gml" )
                                                  << static_cast<int>( Qgis::WkbType::MultiLineStringZ )
                                                  << u"MULTILINESTRING Z ((0 1 2, 3 4 5),(6 7 8, 9 10 11))"_s;

  QTest::newRow( "multilinestring with z gml 3" ) << QStringLiteral( R"gml(<gml:MultiCurve srsName="EPSG:4326"><gml:curveMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension="3">0 1 2 3 4 5</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:curveMember><gml:curveMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension="3">6 7 8 9 10 11</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:curveMember></gml:MultiCurve>)gml" )
                                                  << static_cast<int>( Qgis::WkbType::MultiLineStringZ )
                                                  << u"MULTILINESTRING Z ((0 1 2, 3 4 5),(6 7 8, 9 10 11))"_s;

  QTest::newRow( "multilinestring with z gml 3 with implicit srsDimension (variant 1)" ) << QStringLiteral( R"gml(<gml:MultiLineString srsName="EPSG:4979"><gml:lineStringMember><gml:LineString srsName="EPSG:4979"><gml:posList>0 1 2 3 4 5</gml:posList></gml:LineString></gml:lineStringMember></gml:MultiLineString>)gml" )
                                                                                         << static_cast<int>( Qgis::WkbType::MultiLineStringZ )
                                                                                         << u"MULTILINESTRING Z ((0 1 2, 3 4 5))"_s;

  QTest::newRow( "multilinestring with z gml 3 with implicit srsDimension (variant 2)" ) << QStringLiteral( R"gml(<gml:MultiLineString><gml:lineStringMember><gml:LineString srsName="EPSG:4979"><gml:posList>0 1 2 3 4 5</gml:posList></gml:LineString></gml:lineStringMember></gml:MultiLineString>)gml" )
                                                                                         << static_cast<int>( Qgis::WkbType::MultiLineStringZ )
                                                                                         << u"MULTILINESTRING Z ((0 1 2, 3 4 5))"_s;

  QTest::newRow( "multilinestring with z gml 3 with implicit srsDimension (variant 3)" ) << QStringLiteral( R"gml(<gml:MultiLineString srsName="EPSG:4979"><gml:lineStringMember><gml:LineString><gml:posList>0 1 2 3 4 5</gml:posList></gml:LineString></gml:lineStringMember></gml:MultiLineString>)gml" )
                                                                                         << static_cast<int>( Qgis::WkbType::MultiLineStringZ )
                                                                                         << u"MULTILINESTRING Z ((0 1 2, 3 4 5))"_s;

  QTest::newRow( "multipolygon with z gml 2" ) << QStringLiteral( R"gml(<gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,1,2 3,4,5 6,7,8 0,1,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>9,10,11 12,13,14 15,16,17 9,10,11</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon>)gml" )
                                               << static_cast<int>( Qgis::WkbType::MultiPolygonZ )
                                               << u"MULTIPOLYGON Z (((0 1 2, 3 4 5, 6 7 8, 0 1 2)),((9 10 11, 12 13 14, 15 16 17, 9 10 11)))"_s;

  QTest::newRow( "multipolygon with z gml 3" ) << QStringLiteral( R"gml(<gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,1,2 3,4,5 6,7,8 0,1,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>9,10,11 12,13,14 15,16,17 9,10,11</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon>)gml" )
                                               << static_cast<int>( Qgis::WkbType::MultiPolygonZ )
                                               << u"MULTIPOLYGON Z (((0 1 2, 3 4 5, 6 7 8, 0 1 2)),((9 10 11, 12 13 14, 15 16 17, 9 10 11)))"_s;
}

void TestQgsGML::testZ()
{
  QFETCH( QString, xml );
  QFETCH( int, expectedWkbType );
  QFETCH( QString, expectedWkt );

  const QString wrappedXml = QStringLiteral( R"gml(
<myns:FeatureCollection xmlns:myns="http://myns" xmlns:gml="http://www.opengis.net/gml">
  <gml:featureMember>
    <myns:mytypename fid="mytypename.1">
      <myns:mygeom>%1</myns:mygeom>
    </myns:mytypename>
  </gml:featureMember>
</myns:FeatureCollection>)gml" )
                               .arg( xml );

  const QgsFields fields;
  QgsGmlStreamingParser gmlParser( QString::fromUtf8( QByteArray( "mytypename" ) ), QString::fromUtf8( QByteArray( "mygeom" ) ), fields );
  QCOMPARE( gmlParser.processData( wrappedXml.toUtf8(), true ), true );
  QCOMPARE( gmlParser.wkbType(), static_cast<Qgis::WkbType>( expectedWkbType ) );
  QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> features = gmlParser.getAndStealReadyFeatures();
  QCOMPARE( features.size(), 1 );
  QVERIFY( features[0].first->hasGeometry() );
  QCOMPARE( features[0].first->geometry().wkbType(), static_cast<Qgis::WkbType>( expectedWkbType ) );
  QCOMPARE( features[0].first->geometry().asWkt().toUpper(), expectedWkt );
}

QGSTEST_MAIN( TestQgsGML )
#include "testqgsgml.moc"
