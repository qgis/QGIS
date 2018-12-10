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

/**
 * \ingroup UnitTests
 */
class TestQgsOfflineEditing : public QObject
{
    Q_OBJECT

  private:
    QgsOfflineEditing *mOfflineEditing = nullptr;
    QgsVectorLayer *mpLayer = nullptr;
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

    void createSpatialiteAndSynchronizeBack();
    void createGeopackageAndSynchronizeBack();
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
  QgsApplication::exitQgis();
}

void TestQgsOfflineEditing::init()
{
  QString myFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myTempDirName = tempDir.path();
  QFile::copy( myFileName + "/points.shp", myTempDirName + "/points.shp" );
  QFile::copy( myFileName + "/points.shx", myTempDirName + "/points.shx" );
  QFile::copy( myFileName + "/points.dbf", myTempDirName + "/points.dbf" );
  QString myTempFileName = myTempDirName + "/points.shp";
  QFileInfo myMapFileInfo( myTempFileName );
  mpLayer = new QgsVectorLayer( myMapFileInfo.filePath(),
                                myMapFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayer( mpLayer );

  numberOfFeatures = mpLayer->featureCount();
  numberOfFields = mpLayer->fields().size();

  layerIds.append( mpLayer->id() );
}

void TestQgsOfflineEditing::cleanup()
{
  QgsProject::instance()->removeAllMapLayers();
  layerIds.clear();
  QDir dir( offlineDataPath );
  dir.remove( offlineDbFile );
}

void TestQgsOfflineEditing::createSpatialiteAndSynchronizeBack()
{
  offlineDbFile = "TestQgsOfflineEditing.sqlite";
  QCOMPARE( mpLayer->name(), QStringLiteral( "points" ) );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );

  //convert
  mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::SpatiaLite );

  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayers().first() );
  QCOMPARE( mpLayer->name(), QStringLiteral( "points (offline)" ) );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );

  //synchronize back
  mOfflineEditing->synchronize();

  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayers().first() );
  QCOMPARE( mpLayer->name(), QStringLiteral( "points" ) );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );
}

void TestQgsOfflineEditing::createGeopackageAndSynchronizeBack()
{
  offlineDbFile = "TestQgsOfflineEditing.gpkg";
  QCOMPARE( mpLayer->name(), QStringLiteral( "points" ) );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );
  QgsFeature firstFeatureBeforeAction;
  QgsFeatureIterator it = mpLayer->getFeatures();
  it.nextFeature( firstFeatureBeforeAction );

  connect( mOfflineEditing, &QgsOfflineEditing::warning, this, []( const QString & title, const QString & message ) { qDebug() << title << message; } );
  //convert
  mOfflineEditing->convertToOfflineProject( offlineDataPath, offlineDbFile, layerIds, false, QgsOfflineEditing::GPKG );

  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayers().first() );
  QCOMPARE( mpLayer->name(), QStringLiteral( "points (offline)" ) );
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures );
  //comparing with the number +1 because GPKG created an fid
  QCOMPARE( mpLayer->fields().size(), numberOfFields + 1 );

  QgsFeature firstFeatureInAction;
  it = mpLayer->getFeatures();
  it.nextFeature( firstFeatureInAction );

  //compare some values
  QCOMPARE( firstFeatureInAction.attribute( QStringLiteral( "Class" ) ).toString(), firstFeatureBeforeAction.attribute( QStringLiteral( "Class" ) ).toString() );
  QCOMPARE( firstFeatureInAction.attribute( QStringLiteral( "Heading" ) ).toString(), firstFeatureBeforeAction.attribute( QStringLiteral( "Heading" ) ).toString() );
  QCOMPARE( firstFeatureInAction.attribute( QStringLiteral( "Cabin Crew" ) ).toString(), firstFeatureBeforeAction.attribute( QStringLiteral( "Cabin Crew" ) ).toString() );

  QgsFeature newFeature( mpLayer->dataProvider()->fields() );
  newFeature.setAttribute( QStringLiteral( "Class" ), QStringLiteral( "Superjet" ) );
  mpLayer->startEditing();
  mpLayer->addFeature( newFeature );
  mpLayer->commitChanges();
  QCOMPARE( mpLayer->featureCount(), numberOfFeatures + 1 );

  //synchronize back
  mOfflineEditing->synchronize();

  mpLayer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayers().first() );
  QCOMPARE( mpLayer->name(), QStringLiteral( "points" ) );
  QCOMPARE( mpLayer->dataProvider()->featureCount(), numberOfFeatures + 1 );
  QCOMPARE( mpLayer->fields().size(), numberOfFields );
  //get last feature
  QgsFeature f = mpLayer->getFeature( mpLayer->dataProvider()->featureCount() - 1 );
  qDebug() << "FID:" << f.id() << "Class:" << f.attribute( "Class" ).toString();
  QCOMPARE( f.attribute( QStringLiteral( "Class" ) ).toString(), QStringLiteral( "Superjet" ) );

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

QGSTEST_MAIN( TestQgsOfflineEditing )
#include "testqgsofflineediting.moc"
