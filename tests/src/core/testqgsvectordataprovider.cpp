/***************************************************************************
     test_template.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QObject>

#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgsfeaturerequest.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

#if QT_VERSION < 0x40701
// See http://hub.qgis.org/issues/4284
Q_DECLARE_METATYPE( QVariant )
#endif

Q_DECLARE_METATYPE( QgsFeatureRequest );

class TestQgsVectorDataProvider : public QObject
{
    Q_OBJECT
  public:
    TestQgsVectorDataProvider()
        : vlayerPoints( 0 )
        , vlayerLines( 0 )
    {}

  private slots:

    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    // test whether QgsFeature content is set up correctly
    void select_checkContents_data();
    void select_checkContents();

    // test whether correct subset is returned
    void select_checkSubset_data();
    void select_checkSubset();

    void featureAtId();

  private:

    QgsVectorLayer* vlayerPoints;
    QgsVectorLayer* vlayerLines;
};

void TestQgsVectorDataProvider::initTestCase()
{
  vlayerPoints = 0;
  vlayerLines = 0;

  // load QGIS
  QgsApplication::init();
  QgsApplication::initQgis();

  QString layerPointsUrl = QString( TEST_DATA_DIR ) + QDir::separator() + QString( "points.shp" );
  QString layerLinesUrl = QString( TEST_DATA_DIR ) + QDir::separator() + QString( "lines.shp" );

  // load layers

  vlayerPoints = new QgsVectorLayer( layerPointsUrl, "testlayer", "ogr" );
  QVERIFY( vlayerPoints );
  QVERIFY( vlayerPoints->isValid() );

  vlayerLines = new QgsVectorLayer( layerLinesUrl, "testlayer", "ogr" );
  QVERIFY( vlayerLines );
  QVERIFY( vlayerLines->isValid() );
}

void TestQgsVectorDataProvider::cleanupTestCase()
{
  delete vlayerPoints;
  delete vlayerLines;

  // unload QGIS
  QgsApplication::exitQgis();
}


static double keep6digits( double x )
{
  return qRound( x*1e6 ) / 1e6;
}

static void checkFid4( QgsFeature& f, bool hasGeometry, bool hasAttrs, int onlyOneAttribute )
{
  const QgsAttributes& attrs = f.attributes();

  QCOMPARE( f.id(), ( QgsFeatureId )4 );

  QCOMPARE( f.attributes().count(), 6 );
  if ( hasAttrs )
  {
    QCOMPARE( attrs[0].toString(), ( onlyOneAttribute == -1 || onlyOneAttribute == 0 ) ? QString( "Jet" ) : QString() );
    QCOMPARE( attrs[1].toInt(), ( onlyOneAttribute == -1 || onlyOneAttribute == 1 ) ? 90 : 0 );
    QCOMPARE( attrs[2].toInt(), ( onlyOneAttribute == -1 || onlyOneAttribute == 2 ) ? 3 : 0 );
  }
  else
  {
    QCOMPARE( attrs[0].type(), QVariant::Invalid );
    QCOMPARE( attrs[1].type(), QVariant::Invalid );
    QCOMPARE( attrs[2].type(), QVariant::Invalid );
  }

  if ( hasGeometry )
  {
    QVERIFY( f.geometry() );
    QVERIFY( f.geometry()->wkbType() == QGis::WKBPoint );
    QCOMPARE( keep6digits( f.geometry()->asPoint().x() ), -88.302277 );
    QCOMPARE( keep6digits( f.geometry()->asPoint().y() ),  33.731884 );
  }
  else
  {
    QVERIFY( !f.geometry() );
  }
}

void TestQgsVectorDataProvider::select_checkContents_data()
{
  QTest::addColumn<QgsFeatureRequest>( "request" );
  QTest::addColumn<int>( "count" );
  QTest::addColumn<bool>( "hasGeometry" );
  QTest::addColumn<bool>( "hasAttributes" );
  QTest::addColumn<int>( "onlyOneAttribute" ); // -1 if all attributes should be fetched

  QTest::newRow( "all" ) << QgsFeatureRequest() << 17 << true << true << -1;
  QTest::newRow( "no attrs" ) << QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) << 17 << true << false << -1;
  QTest::newRow( "no geom" ) << QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ) << 17 << false << true << -1;
  QTest::newRow( "one attr" ) << QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() << 1 ) << 17 << true << true << 1;
}

void TestQgsVectorDataProvider::select_checkContents()
{
  QFETCH( QgsFeatureRequest, request );
  QFETCH( int, count );
  QFETCH( bool, hasGeometry );
  QFETCH( bool, hasAttributes );
  QFETCH( int, onlyOneAttribute );

  // base select: check num. features, check geometry, check attrs
  QgsVectorDataProvider* pr = vlayerPoints->dataProvider();
  QgsFeatureIterator fi = pr->getFeatures( request );

  bool foundFid4 = false;
  int realCount = 0;
  QgsFeature f;
  while ( fi.nextFeature( f ) )
  {
    if ( f.id() == 4 )
    {
      checkFid4( f, hasGeometry, hasAttributes, onlyOneAttribute );
      foundFid4 = true;
    }
    realCount++;
  }

  QCOMPARE( realCount, count );
  QVERIFY( foundFid4 );
}

void TestQgsVectorDataProvider::select_checkSubset_data()
{
  // test that only a subset is returned ... need line features

  QTest::addColumn<QgsFeatureRequest>( "request" );
  QTest::addColumn<int>( "count" );

  QgsRectangle rect1( -98, 31, -95, 34 ); // bounding box -> 2 feats, exact intersect -> 0 feats
  QgsRectangle rect2( -90, 37, -86, 39 ); // bounding box -> 4 feats, exact intersect -> 2 feats

  QTest::newRow( "all" ) << QgsFeatureRequest() << 6;
  // OGR always does exact intersection test
  //QTest::newRow("rect1") << QgsFeatureRequest().setExtent(rect1) << 2;
  //QTest::newRow("rect2") << QgsFeatureRequest().setExtent(rect2) << 4;
  QTest::newRow( "rect1 + exact intersect" ) << QgsFeatureRequest().setFilterRect( rect1 ).setFlags( QgsFeatureRequest::ExactIntersect ) << 0;
  QTest::newRow( "rect2 + exact intersect" ) << QgsFeatureRequest().setFilterRect( rect2 ).setFlags( QgsFeatureRequest::ExactIntersect ) << 2;
}

void TestQgsVectorDataProvider::select_checkSubset()
{
  QFETCH( QgsFeatureRequest, request );
  QFETCH( int, count );

  // base select: check num. features, check geometry, check attrs
  QgsVectorDataProvider* pr = vlayerLines->dataProvider();
  QgsFeatureIterator fi = pr->getFeatures( request );

  int realCount = 0;
  QgsFeature f;
  while ( fi.nextFeature( f ) )
  {
    realCount++;
  }

  QCOMPARE( realCount, count );
}

void TestQgsVectorDataProvider::featureAtId()
{
  QgsVectorDataProvider* pr = vlayerLines->dataProvider();
  QgsFeatureRequest request;
  request.setFilterFid( 4 );
  QVERIFY( request.filterType() == QgsFeatureRequest::FilterFid );
  QVERIFY( request.filterFid() == 4 );

  QgsFeatureIterator fi = pr->getFeatures( request );
  QgsFeature feature;

  QVERIFY( fi.nextFeature( feature ) );
  QVERIFY( feature.isValid() );
  qDebug( "FID: %lld", feature.id() );
  QVERIFY( feature.id() == 4 );

  // further invocations are not valid
  QVERIFY( !fi.nextFeature( feature ) );
  QVERIFY( !feature.isValid() );
}


QTEST_MAIN( TestQgsVectorDataProvider )

#include "testqgsvectordataprovider.moc"
