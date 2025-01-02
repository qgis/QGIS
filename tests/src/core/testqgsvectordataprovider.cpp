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
#include "qgstest.h"
#include <QObject>
#include <QString>

#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgsfeaturerequest.h>
#include "qgsfeatureiterator.h"
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

Q_DECLARE_METATYPE( QgsFeatureRequest )

class TestQgsVectorDataProvider : public QObject
{
    Q_OBJECT
  public:
    TestQgsVectorDataProvider() = default;

  private slots:

    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    // test whether QgsFeature content is set up correctly
    void select_checkContents_data();
    void select_checkContents();

    // test whether correct subset is returned
    void select_checkSubset_data();
    void select_checkSubset();

    void featureAtId();

    void sourceExtent();

  private:
    QgsVectorLayer *vlayerPoints = nullptr;
    QgsVectorLayer *vlayerLines = nullptr;
    QgsVectorLayer *vlayerPoints3D = nullptr;
    QgsVectorLayer *vlayerLines3D = nullptr;
};

void TestQgsVectorDataProvider::initTestCase()
{
  vlayerPoints = nullptr;
  vlayerLines = nullptr;
  vlayerPoints3D = nullptr;
  vlayerLines3D = nullptr;

  // load QGIS
  QgsApplication::init();
  QgsApplication::initQgis();

  const QString layerPointsUrl = QStringLiteral( TEST_DATA_DIR ) + "/points.shp";
  const QString layerLinesUrl = QStringLiteral( TEST_DATA_DIR ) + "/lines.shp";
  const QString layerPoints3DUrl = QStringLiteral( TEST_DATA_DIR ) + "/3d/points_with_z.shp";
  const QString layerLines3DUrl = QStringLiteral( TEST_DATA_DIR ) + "/3d/lines_with_z.gpkg.zip";

  // load layers
  const QgsVectorLayer::LayerOptions options { QgsCoordinateTransformContext() };
  vlayerPoints = new QgsVectorLayer( layerPointsUrl, QStringLiteral( "testlayer" ), QStringLiteral( "ogr" ), options );
  QVERIFY( vlayerPoints );
  QVERIFY( vlayerPoints->isValid() );

  vlayerLines = new QgsVectorLayer( layerLinesUrl, QStringLiteral( "testlayer" ), QStringLiteral( "ogr" ), options );
  QVERIFY( vlayerLines );
  QVERIFY( vlayerLines->isValid() );

  vlayerPoints3D = new QgsVectorLayer( layerPoints3DUrl, QStringLiteral( "testlayer" ), QStringLiteral( "ogr" ), options );
  QVERIFY( vlayerPoints3D );
  QVERIFY( vlayerPoints3D->isValid() );

  vlayerLines3D = new QgsVectorLayer( layerLines3DUrl, QStringLiteral( "testlayer" ), QStringLiteral( "ogr" ), options );
  QVERIFY( vlayerLines3D );
  QVERIFY( vlayerLines3D->isValid() );
}

void TestQgsVectorDataProvider::cleanupTestCase()
{
  delete vlayerPoints;
  delete vlayerLines;
  delete vlayerPoints3D;
  delete vlayerLines3D;

  // unload QGIS
  QgsApplication::exitQgis();
}


static double keep6digits( double x )
{
  return std::round( x * 1e6 ) / 1e6;
}

static void checkFid4( QgsFeature &f, bool hasGeometry, bool hasAttrs, int onlyOneAttribute )
{
  const QgsAttributes &attrs = f.attributes();

  QCOMPARE( f.id(), ( QgsFeatureId ) 4 );

  QCOMPARE( f.attributes().count(), 6 );
  if ( hasAttrs )
  {
    QCOMPARE( attrs[0].toString(), ( onlyOneAttribute == -1 || onlyOneAttribute == 0 ) ? QString( "Jet" ) : QString() );
    QCOMPARE( attrs[1].toInt(), ( onlyOneAttribute == -1 || onlyOneAttribute == 1 ) ? 90 : 0 );
    QCOMPARE( attrs[2].toInt(), ( onlyOneAttribute == -1 || onlyOneAttribute == 2 ) ? 3 : 0 );
  }
  else
  {
    QCOMPARE( static_cast<QMetaType::Type>( attrs[0].userType() ), QMetaType::Type::UnknownType );
    QCOMPARE( static_cast<QMetaType::Type>( attrs[1].userType() ), QMetaType::Type::UnknownType );
    QCOMPARE( static_cast<QMetaType::Type>( attrs[2].userType() ), QMetaType::Type::UnknownType );
  }

  if ( hasGeometry )
  {
    QVERIFY( f.hasGeometry() );
    QVERIFY( f.geometry().wkbType() == Qgis::WkbType::Point );
    QCOMPARE( keep6digits( f.geometry().asPoint().x() ), -88.302277 );
    QCOMPARE( keep6digits( f.geometry().asPoint().y() ), 33.731884 );
  }
  else
  {
    QVERIFY( !f.hasGeometry() );
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
  QTest::newRow( "no geom" ) << QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry ) << 17 << false << true << -1;
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
  QgsVectorDataProvider *pr = vlayerPoints->dataProvider();
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

  const QgsRectangle rect1( -98, 31, -95, 34 ); // bounding box -> 2 feats, exact intersect -> 0 feats
  const QgsRectangle rect2( -90, 37, -86, 39 ); // bounding box -> 4 feats, exact intersect -> 2 feats

  QTest::newRow( "all" ) << QgsFeatureRequest() << 6;
  // OGR always does exact intersection test
  //QTest::newRow("rect1") << QgsFeatureRequest().setExtent(rect1) << 2;
  //QTest::newRow("rect2") << QgsFeatureRequest().setExtent(rect2) << 4;
  QTest::newRow( "rect1 + exact intersect" ) << QgsFeatureRequest().setFilterRect( rect1 ).setFlags( Qgis::FeatureRequestFlag::ExactIntersect ) << 0;
  QTest::newRow( "rect2 + exact intersect" ) << QgsFeatureRequest().setFilterRect( rect2 ).setFlags( Qgis::FeatureRequestFlag::ExactIntersect ) << 2;
}

void TestQgsVectorDataProvider::select_checkSubset()
{
  QFETCH( QgsFeatureRequest, request );
  QFETCH( int, count );

  // base select: check num. features, check geometry, check attrs
  QgsVectorDataProvider *pr = vlayerLines->dataProvider();
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
  QgsVectorDataProvider *pr = vlayerLines->dataProvider();
  QgsFeatureRequest request;
  request.setFilterFid( 4 );
  QCOMPARE( request.filterType(), Qgis::FeatureRequestFilterType::Fid );
  QCOMPARE( request.filterFid(), 4LL );

  QgsFeatureIterator fi = pr->getFeatures( request );
  QgsFeature feature;

  QVERIFY( fi.nextFeature( feature ) );
  QVERIFY( feature.isValid() );
  qDebug( "FID: %lld", feature.id() );
  QCOMPARE( feature.id(), 4LL );

  // further invocations are not valid
  QVERIFY( !fi.nextFeature( feature ) );
  QVERIFY( !feature.isValid() );
}

void TestQgsVectorDataProvider::sourceExtent()
{
  // 2d data
  QgsVectorDataProvider *prLines = vlayerLines->dataProvider();

  QGSCOMPARENEAR( prLines->sourceExtent().xMinimum(), -117.623, 0.001 );
  QGSCOMPARENEAR( prLines->sourceExtent().xMaximum(), -82.3226, 0.001 );
  QGSCOMPARENEAR( prLines->sourceExtent().yMinimum(), 23.2082, 0.001 );
  QGSCOMPARENEAR( prLines->sourceExtent().yMaximum(), 46.1829, 0.001 );

  QGSCOMPARENEAR( prLines->sourceExtent3D().xMinimum(), -117.623, 0.001 );
  QGSCOMPARENEAR( prLines->sourceExtent3D().xMaximum(), -82.3226, 0.001 );
  QGSCOMPARENEAR( prLines->sourceExtent3D().yMinimum(), 23.2082, 0.001 );
  QGSCOMPARENEAR( prLines->sourceExtent3D().yMaximum(), 46.1829, 0.001 );
  QVERIFY( std::isnan( prLines->sourceExtent3D().zMinimum() ) );
  QVERIFY( std::isnan( prLines->sourceExtent3D().zMaximum() ) );


  QgsVectorDataProvider *prPoints = vlayerPoints->dataProvider();

  QGSCOMPARENEAR( prPoints->sourceExtent().xMinimum(), -118.889, 0.001 );
  QGSCOMPARENEAR( prPoints->sourceExtent().xMaximum(), -83.3333, 0.001 );
  QGSCOMPARENEAR( prPoints->sourceExtent().yMinimum(), 22.8002, 0.001 );
  QGSCOMPARENEAR( prPoints->sourceExtent().yMaximum(), 46.872, 0.001 );

  QGSCOMPARENEAR( prPoints->sourceExtent3D().xMinimum(), -118.889, 0.001 );
  QGSCOMPARENEAR( prPoints->sourceExtent3D().xMaximum(), -83.3333, 0.001 );
  QGSCOMPARENEAR( prPoints->sourceExtent3D().yMinimum(), 22.8002, 0.001 );
  QGSCOMPARENEAR( prPoints->sourceExtent3D().yMaximum(), 46.872, 0.001 );
  QVERIFY( std::isnan( prPoints->sourceExtent3D().zMinimum() ) );
  QVERIFY( std::isnan( prPoints->sourceExtent3D().zMaximum() ) );

  // 3d data
  QgsVectorDataProvider *prLines3D = vlayerLines3D->dataProvider();

  QGSCOMPARENEAR( prLines3D->sourceExtent().xMinimum(), 0.0, 0.01 );
  QGSCOMPARENEAR( prLines3D->sourceExtent().xMaximum(), 322355.71, 0.01 );
  QGSCOMPARENEAR( prLines3D->sourceExtent().yMinimum(), 0.0, 0.01 );
  QGSCOMPARENEAR( prLines3D->sourceExtent().yMaximum(), 129791.26, 0.01 );

  QGSCOMPARENEAR( prLines3D->sourceExtent3D().xMinimum(), 0.0, 0.01 );
  QGSCOMPARENEAR( prLines3D->sourceExtent3D().xMaximum(), 322355.71, 0.01 );
  QGSCOMPARENEAR( prLines3D->sourceExtent3D().yMinimum(), 0.0, 0.01 );
  QGSCOMPARENEAR( prLines3D->sourceExtent3D().yMaximum(), 129791.26, 0.01 );
  QGSCOMPARENEAR( prLines3D->sourceExtent3D().zMinimum(), -5.00, 0.01 );
  QGSCOMPARENEAR( prLines3D->sourceExtent3D().zMaximum(), 15.0, 0.01 );


  QgsVectorDataProvider *prPoints3D = vlayerPoints3D->dataProvider();

  QGSCOMPARENEAR( prPoints3D->sourceExtent().xMinimum(), 321384.94, 0.01 );
  QGSCOMPARENEAR( prPoints3D->sourceExtent().xMaximum(), 322342.3, 0.01 );
  QGSCOMPARENEAR( prPoints3D->sourceExtent().yMinimum(), 129147.09, 0.01 );
  QGSCOMPARENEAR( prPoints3D->sourceExtent().yMaximum(), 130554.6, 0.01 );

  QGSCOMPARENEAR( prPoints3D->sourceExtent3D().xMinimum(), 321384.94, 0.01 );
  QGSCOMPARENEAR( prPoints3D->sourceExtent3D().xMaximum(), 322342.3, 0.01 );
  QGSCOMPARENEAR( prPoints3D->sourceExtent3D().yMinimum(), 129147.09, 0.01 );
  QGSCOMPARENEAR( prPoints3D->sourceExtent3D().yMaximum(), 130554.6, 0.01 );
  QGSCOMPARENEAR( prPoints3D->sourceExtent3D().zMinimum(), 64.9, 0.01 );
  QGSCOMPARENEAR( prPoints3D->sourceExtent3D().zMaximum(), 105.6, 0.01 );
}


QGSTEST_MAIN( TestQgsVectorDataProvider )

#include "testqgsvectordataprovider.moc"
