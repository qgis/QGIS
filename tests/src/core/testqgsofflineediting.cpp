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
#include <QObject>

#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

#include "qgsunittypes.h"
#include "qgsofflineediting.h"
#include "qgstest.h"
#include "qgsvectorlayerref.h"
#include "qgslayertree.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsjsonutils.h"

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
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

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
  mpLayer = new QgsVectorLayer( myMapFileInfo.filePath(),
                                myMapFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayer( mpLayer );

  numberOfFeatures = mpLayer->featureCount();
  numberOfFields = mpLayer->fields().size();

  layerIds.append( mpLayer->id() );

  //same with gpkg
  QFile::copy( myFileName + "/points_gpkg.gpkg", myTempDirName + "/points_gpkg.gpkg" );
  const QString myTempFileNameGpgk = myTempDirName + "/points_gpkg.gpkg";
  const QFileInfo myMapFileInfoGpkg( myTempFileNameGpgk );
  gpkgLayer = new QgsVectorLayer( myMapFileInfoGpkg.filePath() + "|layername=points_gpkg", "points_gpkg", QStringLiteral( "ogr" ) );

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

  QTest::newRow( "no suffix" ) << QString( "no suffix" ) << QStringLiteral( " (offline)" ); //default value expected
  QTest::newRow( "null suffix" ) << QString() << QString();
  QTest::newRow( "empty suffix" ) << QStringLiteral( "" ) << QStringLiteral( "" );
  QTest::newRow( "part of name suffix" ) << QStringLiteral( "point" ) << QStringLiteral( "point" );
  QTest::newRow( "another suffix" ) << QStringLiteral( "another suffix" ) << QStringLiteral( "another suffix" );
}

void TestQgsOfflineEditing::createGeopackageAndSynchronizeBack_data()
{
  QTest::addColumn<QString>( "suffix_input" );
  QTest::addColumn<QString>( "suffix_result" );

  QTest::newRow( "no suffix" ) << QStringLiteral( "no suffix" ) << QStringLiteral( " (offline)" ); //default value expected
  QTest::newRow( "null suffix" ) << QString() << QString();
  QTest::newRow( "empty suffix" ) << QStringLiteral( "" ) << QStringLiteral( "" );
  QTest::newRow( "part of name suffix" ) << QStringLiteral( "point" ) << QStringLiteral( "point" );
  QTest::newRow( "another suffix" ) << QStringLiteral( "another suffix" ) << QStringLiteral( "another suffix" );
}

void TestQgsOfflineEditing::createSpatialiteAndSynchronizeBack()
{

  QFETCH( QString, suffix_input );
  QFETCH( QString, suffix_result );

  offlineDbFile = "TestQgsOfflineEditing.sqlite";
  QCOMPARE( mpLayer->name(), QStringLiteral( "points" ) );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );

  //set on LayerTreeNode showFeatureCount property
  QgsLayerTreeLayer *layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  layerTreelayer->setCustomProperty( QStringLiteral( "showFeatureCount" ), 1 );

  //convert
  if ( suffix_input.compare( QLatin1String( "no suffix" ) ) == 0 )
    mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::SpatiaLite );
  else
    mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::SpatiaLite, suffix_input );

  const QString layerName = QStringLiteral( "points%1" ).arg( suffix_result );

  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayersByName( layerName ).first() );
  QCOMPARE( mpLayer->name(), layerName );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  //check LayerTreeNode showFeatureCount property
  layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  QCOMPARE( layerTreelayer->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toInt(), 1 );
  //unset on LayerTreeNode showFeatureCount property
  layerTreelayer->setCustomProperty( QStringLiteral( "showFeatureCount" ), 0 );

  //synchronize back
  mOfflineEditing->synchronize();

  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayersByName( QStringLiteral( "points" ) ).first() );
  QCOMPARE( mpLayer->name(), QStringLiteral( "points" ) );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );

  //check LayerTreeNode showFeatureCount property
  layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  QCOMPARE( layerTreelayer->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toInt(), 0 );
}

void TestQgsOfflineEditing::createGeopackageAndSynchronizeBack()
{
  QFETCH( QString, suffix_input );
  QFETCH( QString, suffix_result );

  offlineDbFile = "TestQgsOfflineEditing.gpkg";
  QCOMPARE( mpLayer->name(), QStringLiteral( "points" ) );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );
  QgsFeature firstFeatureBeforeAction;
  QgsFeatureIterator it = mpLayer->getFeatures();
  it.nextFeature( firstFeatureBeforeAction );

  connect( mOfflineEditing, &QgsOfflineEditing::warning, this, []( const QString & title, const QString & message ) { qDebug() << title << message; } );

  //set on LayerTreeNode showFeatureCount property
  QgsLayerTreeLayer *layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  layerTreelayer->setCustomProperty( QStringLiteral( "showFeatureCount" ), 1 );
  layerTreelayer->setItemVisibilityChecked( false );
  QgsMapLayerStyle style;
  style.readFromLayer( mpLayer );

  mpLayer->styleManager()->addStyle( QStringLiteral( "testStyle" ), style );

  //convert
  if ( suffix_input.compare( QLatin1String( "no suffix" ) ) == 0 )
    mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::GPKG );
  else
    mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::GPKG, suffix_input );

  const QString layerName = QStringLiteral( "points%1" ).arg( suffix_result );
  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayersByName( layerName ).first() );
  QCOMPARE( mpLayer->name(), layerName );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  //comparing with the number +1 because GPKG created an fid
  QCOMPARE( mpLayer->fields().size(), numberOfFields + 1 );
  //check LayerTreeNode showFeatureCount property
  layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  QCOMPARE( layerTreelayer->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toInt(), 1 );
  QCOMPARE( layerTreelayer->isVisible(), false );
  QVERIFY( mpLayer->styleManager()->styles().contains( QStringLiteral( "testStyle" ) ) );

  QgsFeature firstFeatureInAction;
  it = mpLayer->getFeatures();
  it.nextFeature( firstFeatureInAction );

  //compare some values
  QCOMPARE( firstFeatureInAction.attribute( QStringLiteral( "Class" ) ).toString(), firstFeatureBeforeAction.attribute( QStringLiteral( "Class" ) ).toString() );
  QCOMPARE( firstFeatureInAction.attribute( QStringLiteral( "Heading" ) ).toString(), firstFeatureBeforeAction.attribute( QStringLiteral( "Heading" ) ).toString() );
  QCOMPARE( firstFeatureInAction.attribute( QStringLiteral( "Cabin Crew" ) ).toString(), firstFeatureBeforeAction.attribute( QStringLiteral( "Cabin Crew" ) ).toString() );

  //check converted lists values
  QCOMPARE( firstFeatureInAction.attribute( QStringLiteral( "StaffNames" ) ), QVariantList() << QStringLiteral( "Bob" ) << QStringLiteral( "Alice" ) );
  QCOMPARE( firstFeatureInAction.attribute( QStringLiteral( "StaffAges" ) ), QVariantList() << 22 << 33 );

  QgsFeature newFeature( mpLayer->dataProvider()->fields() );
  newFeature.setAttribute( QStringLiteral( "Class" ), QStringLiteral( "Superjet" ) );
  newFeature.setAttribute( QStringLiteral( "StaffNames" ), QgsJsonUtils::parseArray( QStringLiteral( "[ \"Sebastien\", \"Naomi\", \"And, many, more\" ]" ) ) );
  newFeature.setAttribute( QStringLiteral( "StaffAges" ), QgsJsonUtils::parseArray( QStringLiteral( "[ 0, 2 ]" ) ) );
  mpLayer->startEditing();
  mpLayer->addFeature( newFeature );
  mpLayer->commitChanges();
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures + 1 );

  //unset on LayerTreeNode showFeatureCount property
  layerTreelayer->setCustomProperty( QStringLiteral( "showFeatureCount" ), 0 );

  //synchronize back
  mOfflineEditing->synchronize();

  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayersByName( QStringLiteral( "points" ) ).first() );
  QCOMPARE( mpLayer->name(), QStringLiteral( "points" ) );
  QCOMPARE( mpLayer->dataProvider()->featureCount(), numberOfFeatures + 1 );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );
  //check LayerTreeNode showFeatureCount property
  layerTreelayer = QgsProject::instance()->layerTreeRoot()->findLayer( mpLayer->id() );
  QCOMPARE( layerTreelayer->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toInt(), 0 );

  //get last feature
  const QgsFeature f = mpLayer->getFeature( mpLayer->dataProvider()->featureCount() - 1 );
  qDebug() << "FID:" << f.id() << "Class:" << f.attribute( "Class" ).toString();
  QCOMPARE( f.attribute( QStringLiteral( "Class" ) ).toString(), QStringLiteral( "Superjet" ) );
  QCOMPARE( f.attribute( QStringLiteral( "StaffNames" ) ).toStringList(), QStringList() << QStringLiteral( "Sebastien" ) << QStringLiteral( "Naomi" ) << QStringLiteral( "And, many, more" ) );
  QCOMPARE( f.attribute( QStringLiteral( "StaffAges" ) ).toList(), QList<QVariant>() << 0 << 2 );

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
  QCOMPARE( gpkgLayer->name(), QStringLiteral( "points_gpkg" ) );

  //check constraints (not null and unique)
  QgsFieldConstraints constraintsOfFidField = gpkgLayer->fields().at( gpkgLayer->fields().indexOf( QLatin1String( "fid" ) ) ).constraints();
  QVERIFY( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintNotNull );
  QVERIFY( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintUnique );

  //convert
  mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::GPKG );

  QCOMPARE( gpkgLayer->name(), QStringLiteral( "points_gpkg (offline)" ) );

  //check constraints (not not null)
  constraintsOfFidField = gpkgLayer->fields().at( gpkgLayer->fields().indexOf( QLatin1String( "fid" ) ) ).constraints();
  QVERIFY( !( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintNotNull ) );
  QVERIFY( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintUnique );

  //synchronize back
  mOfflineEditing->synchronize();

  //check constraints (not null and unique)
  constraintsOfFidField = gpkgLayer->fields().at( gpkgLayer->fields().indexOf( QLatin1String( "fid" ) ) ).constraints();
  QVERIFY( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintNotNull );
  QVERIFY( constraintsOfFidField.constraints() & QgsFieldConstraints::ConstraintUnique );
}


QGSTEST_MAIN( TestQgsOfflineEditing )
#include "testqgsofflineediting.moc"
