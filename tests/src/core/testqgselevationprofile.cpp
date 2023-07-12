/***************************************************************************
  testqgselevationProfile.cpp
  --------------------------------------
Date                 : August 2022
Copyright            : (C) 2022 by Martin Dobias
Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include "qgsapplication.h"
#include "qgsvectorlayerprofilegenerator.h"
#include "qgsprofilerequest.h"
#include "qgscurve.h"
#include "qgsvectorlayer.h"
#include "project/qgsprojectelevationproperties.h"
#include "qgsvectorlayerelevationproperties.h"
#include <qgsproject.h>
#include "qgsterrainprovider.h"

#define DEBUG 0

class TestQgsElevationProfile : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsElevationProfile() : QgsTest( QStringLiteral( "Elevation Profile Tests" ), QStringLiteral( "elevation_Profile" ) ) {}

  private:
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsVectorLayer *mpLinesLayer = nullptr;
    QgsVectorLayer *mpPolygonsLayer = nullptr;
    QgsRasterLayer *mLayerDtm = nullptr;
    QString mTestDataDir;
    QgsPointSequence mProfilePoints;
    QgsVectorLayerProfileResults *mProfileResults = nullptr;
    std::unique_ptr< QgsRasterDemTerrainProvider > mDemTerrain;

    void doCheckPoint( QgsProfileRequest &request, int tolerance, QgsVectorLayer *layer,
                       const QList<QgsFeatureId> &expectedFeatures );
    void doCheckLine( QgsProfileRequest &request, int tolerance, QgsVectorLayer *layer,
                      const QList<QgsFeatureId> &expectedFeatures, const QList<int> &nbSubGeomPerFeature );

    QgsVectorLayer *createVectorLayer( const QString &fileName );

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void testVectorLayerProfileForPoint();
    void testVectorLayerProfileForLine();
};


QgsVectorLayer *TestQgsElevationProfile::createVectorLayer( const QString &fileName )
{
  const QString myFileName = mTestDataDir + fileName;
  const QFileInfo myFileInfo( myFileName );
  QgsVectorLayer *layer = new QgsVectorLayer( myFileInfo.filePath(),
      myFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  dynamic_cast<QgsVectorLayerElevationProperties *>( layer->elevationProperties() )->setClamping( Qgis::AltitudeClamping::Terrain );
  dynamic_cast<QgsVectorLayerElevationProperties *>( layer->elevationProperties() )->setBinding( Qgis::AltitudeBinding::Vertex );

  return layer;
}

void TestQgsElevationProfile::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 3857 ) );

  //create some objects that will be used in all tests...

  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + "/3d/";

  // Create a line layer that will be used in all tests...
  mpLinesLayer = createVectorLayer( "lines.shp" );

  // Create a point layer that will be used in all tests...
  mpPointsLayer = createVectorLayer( "points_with_z.shp" );

  // Create a polygon layer that will be used in all tests...
  mpPolygonsLayer = createVectorLayer( "buildings.shp" );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>()  << mpLinesLayer << mpPointsLayer << mpPolygonsLayer );

  // Create a DEM layer that will be used in all tests...
  const QString myDtmFileName = mTestDataDir + "dtm.tif";
  const QFileInfo myDtmFileInfo( myDtmFileName );
  mLayerDtm = new QgsRasterLayer( myDtmFileInfo.filePath(),
                                  myDtmFileInfo.completeBaseName(), QStringLiteral( "gdal" ) );
  QVERIFY( mLayerDtm->isValid() );

  // set dem as elevation
  mDemTerrain = std::make_unique< QgsRasterDemTerrainProvider >();
  mDemTerrain->setLayer( mLayerDtm );

  QgsProject::instance()->elevationProperties()->setTerrainProvider( mDemTerrain->clone() );

  // profile curve
  mProfilePoints << QgsPoint( Qgis::WkbType::Point, -346120, 6631840 )
                 << QgsPoint( Qgis::WkbType::Point, -346550, 6632030 )
                 << QgsPoint( Qgis::WkbType::Point, -346440, 6632140 )
                 << QgsPoint( Qgis::WkbType::Point, -347830, 6632930 ) ;


}

void TestQgsElevationProfile::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsElevationProfile::doCheckPoint( QgsProfileRequest &request, int tolerance, QgsVectorLayer *layer,
    const QList<QgsFeatureId> &expectedFeatures )
{
  request.setTolerance( tolerance );

  QgsAbstractProfileGenerator *profGen = layer->createProfileGenerator( request );
  QVERIFY( profGen );
  QVERIFY( profGen->generateProfile() );
  mProfileResults = dynamic_cast<QgsVectorLayerProfileResults *>( profGen->takeResults() );
  QVERIFY( mProfileResults );
  QVERIFY( ! mProfileResults->features.empty() );

  QList<QgsFeatureId> expected = expectedFeatures;
  std::sort( expected.begin(), expected.end() );

  QList<QgsFeatureId> actual = mProfileResults->features.keys();
  std::sort( actual.begin(), actual.end() );
#if DEBUG
  qDebug() << "actual sorted fid" << actual;
#endif

  QCOMPARE( actual, expected );

  for ( auto it = mProfileResults->features.constBegin();
        it != mProfileResults->features.constEnd(); ++it )
  {
    for ( const QgsVectorLayerProfileResults::Feature &feat : it.value() )
    {
#if DEBUG
      qDebug() << "feat:" << feat.featureId << "geom:" << feat.geometry.asWkt();
#endif
      if ( QgsWkbTypes::hasZ( feat.geometry.wkbType() ) )
      {
        bool hasValidZ = false;
        for ( QgsAbstractGeometry::vertex_iterator it = feat.geometry.vertices_begin(); it != feat.geometry.vertices_end(); ++it )
        {
          if ( it.operator * ().z() != 0.0 )
          {
            hasValidZ = true;
            break;
          }
        }
        QVERIFY2( hasValidZ, "All vertice are on the ground!" );
      }
      else
      {
        QVERIFY2( false, "Geometry should have z coordinates!" );
      }
    }
  }

}

void TestQgsElevationProfile::doCheckLine( QgsProfileRequest &request, int tolerance, QgsVectorLayer *layer,
    const QList<QgsFeatureId> &expectedFeatures, const QList<int> &nbSubGeomPerFeature )
{
  doCheckPoint( request, tolerance, layer, expectedFeatures );

  // check in how many geometry the feature intersects the profile curve
  int i = 0;
  QList<QgsFeatureId> actual = mProfileResults->features.keys();
  std::sort( actual.begin(), actual.end() );

  for ( auto it = actual.constBegin(); it != actual.constEnd(); ++it, ++i )
  {
    QVector< QgsVectorLayerProfileResults::Feature > feats = mProfileResults->features[*it];
#if DEBUG
    for ( const QgsVectorLayerProfileResults::Feature &feat : feats )
    {
      qDebug() << "feat:" << feat.featureId << "geom:" << feat.geometry.asWkt();
    }
#endif
    QCOMPARE( feats.size(), nbSubGeomPerFeature[i] );
  }
}

void TestQgsElevationProfile::testVectorLayerProfileForPoint()
{
  QgsLineString *profileCurve = new QgsLineString ;
  profileCurve->setPoints( mProfilePoints );

  QgsProfileRequest request( profileCurve );
  request.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 3857 ) );
  request.setTerrainProvider( mDemTerrain->clone() );

  doCheckPoint( request, 15, mpPointsLayer, { 5, 11, 12, 13, 14, 15, 18, 45, 46 } );
  doCheckPoint( request, 70, mpPointsLayer, { 0, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15, 16, 17, 18, 38, 45, 46, 48 } );
}


void TestQgsElevationProfile::testVectorLayerProfileForLine()
{
  QgsLineString *profileCurve = new QgsLineString ;
  profileCurve->setPoints( mProfilePoints );

  QgsProfileRequest request( profileCurve );
  request.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 3857 ) );
  request.setTerrainProvider( mDemTerrain->clone() );

  doCheckLine( request, 1, mpLinesLayer, { 0, 2 }, { 1, 5 } );
  doCheckLine( request, 20, mpLinesLayer, { 0, 2 }, { 1, 3 } );
  doCheckLine( request, 50, mpLinesLayer, { 1, 0, 2 }, { 1, 1, 1 } );
}


QGSTEST_MAIN( TestQgsElevationProfile )
#include "testqgselevationprofile.moc"
