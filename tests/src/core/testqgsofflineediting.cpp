/***************************************************************************
  testqgsofflineediting.cpp

 ---------------------
 begin                : 3.7.2018
 copyright            : (C) 2018 by david signer
 email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsjsonutils.h"
#include "qgslayertree.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsofflineediting.h"
#include "qgstest.h"
#include "qgsunittypes.h"
#include "qgsvectorlayerref.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * \ingroup UnitTests
 */
class TestQgsOfflineEditing : public QObject
{
    Q_OBJECT

  private:
    QgsOfflineEditing *mOfflineEditing = nullptr;
    QgsVectorLayer *mpLayer = nullptr;
    QgsVectorLayer *gpkgLayer = nullptr;
    QString offlineDataPath;
    QString offlineDbFile;
    QStringList layerIds;
    long numberOfFeatures;
    int numberOfFields;
    QTemporaryDir tempDir;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void createSpatialiteAndSynchronizeBack_data();
    void createGeopackageAndSynchronizeBack_data();

    void createSpatialiteAndSynchronizeBack();
    void createGeopackageAndSynchronizeBack();
    void removeConstraintsOnDefaultValues();
};

void TestQgsOfflineEditing::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //OfflineEditing
  mOfflineEditing = new QgsOfflineEditing();
  offlineDataPath = ".";
}

void TestQgsOfflineEditing::cleanupTestCase()
{
  delete mOfflineEditing;
  QgsApplication::exitQgis();
}

void TestQgsOfflineEditing::init()
{
  const QString myFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTempDirName = tempDir.path();
  QFile::copy( myFileName + "/points.geojson", myTempDirName + "/points.geojson" );
  const QString myTempFileName = myTempDirName + "/points.geojson";
  const QFileInfo myMapFileInfo( myTempFileName );
  mpLayer = new QgsVectorLayer( myMapFileInfo.filePath(), myMapFileInfo.completeBaseName(), u"ogr"_s );
  QgsProject::instance()->addMapLayer( mpLayer );

  numberOfFeatures = mpLayer->featureCount();
  numberOfFields = mpLayer->fields().size();

  layerIds.append( mpLayer->id() );

  //same with gpkg
  QFile::copy( myFileName + "/points_gpkg.gpkg", myTempDirName + "/points_gpkg.gpkg" );
  const QString myTempFileNameGpgk = myTempDirName + "/points_gpkg.gpkg";
  const QFileInfo myMapFileInfoGpkg( myTempFileNameGpgk );
  gpkgLayer = new QgsVectorLayer( myMapFileInfoGpkg.filePath() + "|layername=points_gpkg", "points_gpkg", u"ogr"_s );

  QgsProject::instance()->addMapLayer( gpkgLayer );
  layerIds.append( gpkgLayer->id() );
}

void TestQgsOfflineEditing::cleanup()
{
  QgsProject::instance()->removeAllMapLayers();
  layerIds.clear();
  QDir dir( offlineDataPath );
  dir.remove( offlineDbFile );
}

void TestQgsOfflineEditing::createSpatialiteAndSynchronizeBack_data()
{
  QTest::addColumn<QString>( "suffix_input" );
  QTest::addColumn<QString>( "suffix_result" );

  QTest::newRow( "no suffix" ) << QString( "no suffix" ) << u" (offline)"_s; //default value expected
  QTest::newRow( "null suffix" ) << QString() << QString();
  QTest::newRow( "empty suffix" ) << QString() << QString();
  QTest::newRow( "part of name suffix" ) << u"point"_s << u"point"_s;
  QTest::newRow( "another suffix" ) << u"another suffix"_s << u"another suffix"_s;
}

void TestQgsOfflineEditing::createGeopackageAndSynchronizeBack_data()
{
  QTest::addColumn<QString>( "suffix_input" );
  QTest::addColumn<QString>( "suffix_result" );

  QTest::newRow( "no suffix" ) << u"no suffix"_s << u" (offline)"_s; //default value expected
  QTest::newRow( "null suffix" ) << QString() << QString();

  QTest::newRow( "empty suffix" ) << QString() << QString();
  QTest::newRow( "part of name suffix" ) << u"point"_s << u"point"_s;
  QTest::newRow( "another suffix" ) << u"another suffix"_s << u"another suffix"_s;
}

void TestQgsOfflineEditing::createSpatialiteAndSynchronizeBack()
{
  QFETCH( QString, suffix_input );
  QFETCH( QString, suffix_result );

  offlineDbFile = "TestQgsOfflineEditing.sqlite";
  QCOMPARE( mpLayer->name(), u"points"_s );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );

  //set on LayerTreeNode showFeatureCount property
  QgsLayerTreeLayer *layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  layerTreelayer->setCustomProperty( u"showFeatureCount"_s, 1 );

  //convert
  if ( suffix_input.compare( "no suffix"_L1 ) == 0 )
    mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::SpatiaLite );
  else
    mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::SpatiaLite, suffix_input );

  const QString layerName = u"points%1"_s.arg( suffix_result );

  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayersByName( layerName ).first() );
  QCOMPARE( mpLayer->name(), layerName );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  //check LayerTreeNode showFeatureCount property
  layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  QCOMPARE( layerTreelayer->customProperty( u"showFeatureCount"_s, 0 ).toInt(), 1 );
  //unset on LayerTreeNode showFeatureCount property
  layerTreelayer->setCustomProperty( u"showFeatureCount"_s, 0 );

  //synchronize back
  mOfflineEditing->synchronize();

  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayersByName( u"points"_s ).first() );
  QCOMPARE( mpLayer->name(), u"points"_s );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );

  //check LayerTreeNode showFeatureCount property
  layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  QCOMPARE( layerTreelayer->customProperty( u"showFeatureCount"_s, 0 ).toInt(), 0 );
}

void TestQgsOfflineEditing::createGeopackageAndSynchronizeBack()
{
  QFETCH( QString, suffix_input );
  QFETCH( QString, suffix_result );

  offlineDbFile = "TestQgsOfflineEditing.gpkg";
  QCOMPARE( mpLayer->name(), u"points"_s );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );
  QgsFeature firstFeatureBeforeAction;
  QgsFeatureIterator it = mpLayer->getFeatures();
  it.nextFeature( firstFeatureBeforeAction );

  connect( mOfflineEditing, &QgsOfflineEditing::warning, this, []( const QString &title, const QString &message ) { qDebug() << title << message; } );

  //set on LayerTreeNode showFeatureCount property
  QgsLayerTreeLayer *layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  layerTreelayer->setCustomProperty( u"showFeatureCount"_s, 1 );
  layerTreelayer->setItemVisibilityChecked( false );
  QgsMapLayerStyle style;
  style.readFromLayer( mpLayer );

  mpLayer->styleManager()->addStyle( u"testStyle"_s, style );

  //convert
  if ( suffix_input.compare( "no suffix"_L1 ) == 0 )
    mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::GPKG );
  else
    mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::GPKG, suffix_input );

  const QString layerName = u"points%1"_s.arg( suffix_result );
  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayersByName( layerName ).first() );
  QCOMPARE( mpLayer->name(), layerName );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  //comparing with the number +1 because GPKG created an fid
  QCOMPARE( mpLayer->fields().size(), numberOfFields + 1 );
  //check LayerTreeNode showFeatureCount property
  layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  QCOMPARE( layerTreelayer->customProperty( u"showFeatureCount"_s, 0 ).toInt(), 1 );
  QCOMPARE( layerTreelayer->isVisible(), false );
  QVERIFY( mpLayer->styleManager()->styles().contains( u"testStyle"_s ) );

  QgsFeature firstFeatureInAction;
  it = mpLayer->getFeatures();
  it.nextFeature( firstFeatureInAction );

  //compare some values
  QCOMPARE( firstFeatureInAction.attribute( u"Class"_s ).toString(), firstFeatureBeforeAction.attribute( u"Class"_s ).toString() );
  QCOMPARE( firstFeatureInAction.attribute( u"Heading"_s ).toString(), firstFeatureBeforeAction.attribute( u"Heading"_s ).toString() );
  QCOMPARE( firstFeatureInAction.attribute( u"Cabin Crew"_s ).toString(), firstFeatureBeforeAction.attribute( u"Cabin Crew"_s ).toString() );

  //check converted lists values
  QCOMPARE( firstFeatureInAction.attribute( u"StaffNames"_s ), QVariantList() << u"Bob"_s << u"Alice"_s );
  QCOMPARE( firstFeatureInAction.attribute( u"StaffAges"_s ), QVariantList() << 22 << 33 );

  QgsFeature newFeature( mpLayer->dataProvider()->fields() );
  newFeature.setAttribute( u"Class"_s, u"Superjet"_s );
  newFeature.setAttribute( u"StaffNames"_s, QgsJsonUtils::parseArray( u"[ \"Sebastien\", \"Naomi\", \"And, many, more\" ]"_s ) );
  newFeature.setAttribute( u"StaffAges"_s, QgsJsonUtils::parseArray( u"[ 0, 2 ]"_s ) );
  mpLayer->startEditing();
  mpLayer->addFeature( newFeature );
  mpLayer->commitChanges();
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures + 1 );

  //unset on LayerTreeNode showFeatureCount property
  layerTreelayer->setCustomProperty( u"showFeatureCount"_s, 0 );

  //synchronize back
  mOfflineEditing->synchronize();

  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayersByName( u"points"_s ).first() );
  QCOMPARE( mpLayer->name(), u"points"_s );
  QCOMPARE( mpLayer->dataProvider()->featureCount(), numberOfFeatures + 1 );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );
  //check LayerTreeNode showFeatureCount property
  layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  QCOMPARE( layerTreelayer->customProperty( u"showFeatureCount"_s, 0 ).toInt(), 0 );

  //get last feature
  const QgsFeature f = mpLayer->getFeature( mpLayer->dataProvider()->featureCount() - 1 );
  qDebug() << "FID:" << f.id() << "Class:" << f.attribute( "Class" ).toString();
  QCOMPARE( f.attribute( u"Class"_s ).toString(), u"Superjet"_s );
  QCOMPARE( f.attribute( u"StaffNames"_s ).toStringList(), QStringList() << u"Sebastien"_s << u"Naomi"_s << u"And, many, more"_s );
  QCOMPARE( f.attribute( u"StaffAges"_s ).toList(), QList<QVariant>() << 0 << 2 );

  QgsFeature firstFeatureAfterAction;
  it = mpLayer->getFeatures();
  it.nextFeature( firstFeatureAfterAction );

  QCOMPARE( firstFeatureAfterAction, firstFeatureBeforeAction );

  //and delete the feature again
  QgsFeatureIds idsToClean;
  idsToClean << f.id();
  mpLayer->dataProvider()->deleteFeatures( idsToClean );
  QCOMPARE( mpLayer->dataProvider()->featureCount(), numberOfFeatures );
}

void TestQgsOfflineEditing::removeConstraintsOnDefaultValues()
{
  offlineDbFile = "TestQgsOfflineEditing.gpkg";
  QCOMPARE( gpkgLayer->name(), u"points_gpkg"_s );

  //check constraints (not null and unique)
  QgsFieldConstraints constraintsOfFidField = gpkgLayer->fields().at( gpkgLayer->fields().indexOf( "fid"_L1 ) ).constraints();
  QVERIFY( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintNotNull );
  QVERIFY( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintUnique );

  //convert
  mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::GPKG );

  QCOMPARE( gpkgLayer->name(), u"points_gpkg (offline)"_s );

  //check constraints (not not null)
  constraintsOfFidField = gpkgLayer->fields().at( gpkgLayer->fields().indexOf( "fid"_L1 ) ).constraints();
  QVERIFY( !( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintNotNull ) );
  QVERIFY( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintUnique );

  //synchronize back
  mOfflineEditing->synchronize();

  //check constraints (not null and unique)
  constraintsOfFidField = gpkgLayer->fields().at( gpkgLayer->fields().indexOf( "fid"_L1 ) ).constraints();
  QVERIFY( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintNotNull );
  QVERIFY( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintUnique );
}


QGSTEST_MAIN( TestQgsOfflineEditing )
#include "testqgsofflineediting.moc"
