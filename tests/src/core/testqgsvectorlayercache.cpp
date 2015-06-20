/***************************************************************************
    testqgsvectorlayercache.cpp
     --------------------------------------
    Date                 : 20.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
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
#include <QTemporaryFile>

//qgis includes...
#include <qgsvectorlayercache.h>
#include <qgsvectordataprovider.h>
#include <qgsapplication.h>
#include <qgsvectorlayereditbuffer.h>
#include <qgscacheindexfeatureid.h>
#include <QDebug>

/** @ingroup UnitTests
 * This is a unit test for the vector layer cache
 *
 * @see QgsVectorLayerCache
 */
class TestVectorLayerCache : public QObject
{
    Q_OBJECT
  public:
    TestVectorLayerCache()
        : mVectorLayerCache( 0 )
        , mFeatureIdIndex( 0 )
        , mPointsLayer( 0 )
    {}

  private slots:
    void initTestCase();      // will be called before the first testfunction is executed.
    void cleanupTestCase();   // will be called after the last testfunction was executed.
    void init();              // will be called before each testfunction is executed.
    void cleanup();           // will be called after every testfunction.

    void testCacheOverflow();    // Test cache will work if too many features to cache them all are present
    void testCacheAttrActions(); // Test attribute add/ attribute delete
    void testFeatureActions();   // Test adding/removing features works
    void testSubsetRequest();

    void onCommittedFeaturesAdded( QString, QgsFeatureList );

  private:
    QgsVectorLayerCache*           mVectorLayerCache;
    QgsCacheIndexFeatureId*        mFeatureIdIndex;
    QgsVectorLayer*                mPointsLayer;
    QgsFeatureList                 mAddedFeatures;
    QMap<QString, QString> mTmpFiles;
};

// runs before all tests
void TestVectorLayerCache::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  // Backup test shape file and attributes
  QStringList backupFiles;
  backupFiles << "points.shp" << "points.shx" << "points.dbf";

  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myTestDataDir = myDataDir + QDir::separator();

  foreach ( QString f, backupFiles )
  {
    QString tmpFileName = QDir::tempPath() + QDir::separator() + f + "_" + QString::number( qApp->applicationPid() );
    QString origFileName = myTestDataDir + f;

    qDebug() << "Copy " << origFileName << " " << tmpFileName;

    qDebug() << QFile::copy( origFileName, tmpFileName );
    mTmpFiles.insert( origFileName, tmpFileName );
  }

  //
  // load a vector layer
  //
  QString myPointsFileName = myTestDataDir + "points.shp";
  QFileInfo myPointFileInfo( myPointsFileName );
  mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                     myPointFileInfo.completeBaseName(), "ogr" );
}

void TestVectorLayerCache::init()
{
  mVectorLayerCache = new QgsVectorLayerCache( mPointsLayer, 10 );
  mFeatureIdIndex = new QgsCacheIndexFeatureId( mVectorLayerCache );
  mVectorLayerCache->addCacheIndex( mFeatureIdIndex );
}

void TestVectorLayerCache::cleanup()
{
  delete mVectorLayerCache;
}

//runs after all tests
void TestVectorLayerCache::cleanupTestCase()
{
  // Clean added features
  if ( mAddedFeatures.length() > 0 )
  {
    mPointsLayer->startEditing();
    foreach ( QgsFeature f, mAddedFeatures )
    {
      mPointsLayer->deleteFeature( f.id() );
    }
    mPointsLayer->commitChanges();
    mAddedFeatures.clear();
  }


  delete mPointsLayer;

  // Clean tmp files
  QMap<QString, QString>::const_iterator it;

  for ( it = mTmpFiles.constBegin(); it != mTmpFiles.constEnd(); ++it )
  {
    QString tmpFileName = it.value();
    QString origFileName = it.key();

    qDebug() << "Copy " << tmpFileName << " " << origFileName;
    QFile( origFileName ).remove();
    qDebug() << QFile::copy( tmpFileName, origFileName );
    QFile::remove( tmpFileName );
  }

  // also clean up newly created .qix file
  QFile::remove( QString( TEST_DATA_DIR ) + QDir::separator() + "points.qix" );

  QgsApplication::exitQgis();
}

void TestVectorLayerCache::testCacheOverflow()
{
  QgsFeature f;

  // Verify we get all features, even if there are too many to fit into the cache
  QgsFeatureIterator it = mVectorLayerCache->getFeatures();

  int i = 0;
  while ( it.nextFeature( f ) )
  {
    i++;
  }
  it.close();

  QVERIFY( i == 17 );
}

void TestVectorLayerCache::testCacheAttrActions()
{
  QgsFeature f;

  // Add an attribute, make sure it is returned also if a cached feature is requested
  mPointsLayer->startEditing();
  QVariant::Type attrType = QVariant::Int;
  mPointsLayer->addAttribute( QgsField( "newAttr", attrType, "Int", 5, 0 ) );
  mPointsLayer->commitChanges();

  QVERIFY( mVectorLayerCache->featureAtId( 15, f ) );
  QVERIFY( f.attribute( "newAttr" ).isValid() );

  QgsFields allFields = mPointsLayer->pendingFields();
  int idx = allFields.indexFromName( "newAttr" );

  mPointsLayer->startEditing();
  mPointsLayer->deleteAttribute( idx );
  mPointsLayer->commitChanges();

  QVERIFY( mVectorLayerCache->featureAtId( 15, f ) );
  QVERIFY( !f.attribute( "newAttr" ).isValid() );
}

void TestVectorLayerCache::testFeatureActions()
{
  QgsFeature f;

  // Get a random feature to clone
  mPointsLayer->getFeatures( QgsFeatureRequest().setFilterFid( 1 ) ).nextFeature( f );

  // Add feature...
  mPointsLayer->startEditing();
  QVERIFY( mPointsLayer->addFeature( f ) );

  connect( mPointsLayer, SIGNAL( committedFeaturesAdded( QString, QgsFeatureList ) ), SLOT( onCommittedFeaturesAdded( QString, QgsFeatureList ) ) );
  mPointsLayer->commitChanges();
  disconnect( mPointsLayer, SIGNAL( committedFeaturesAdded( QString, QgsFeatureList ) ), this, SLOT( onCommittedFeaturesAdded( QString, QgsFeatureList ) ) );

  QgsFeatureId fid = mAddedFeatures.last().id();

  QVERIFY( mVectorLayerCache->featureAtId( fid, f ) );

  // Delete feature...
  mPointsLayer->startEditing();
  QVERIFY( mPointsLayer->deleteFeature( fid ) );

  QVERIFY( !mVectorLayerCache->featureAtId( fid, f ) );
  mPointsLayer->rollBack();
}

void TestVectorLayerCache::testSubsetRequest()
{
  QgsFeature f;

  QgsFields fields = mPointsLayer->pendingFields();
  QStringList requiredFields;
  requiredFields << "Class" << "Cabin Crew";

  mVectorLayerCache->featureAtId( 16, f );
  QVariant a = f.attribute( 3 );

  QgsFeatureIterator itSubset = mVectorLayerCache->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( requiredFields, fields ) );
  while ( itSubset.nextFeature( f ) ) {}
  itSubset.close();

  mVectorLayerCache->featureAtId( 16, f );
  QVERIFY( a == f.attribute( 3 ) );
}

void TestVectorLayerCache::onCommittedFeaturesAdded( QString layerId, QgsFeatureList features )
{
  Q_UNUSED( layerId )
  mAddedFeatures.append( features );
}

QTEST_MAIN( TestVectorLayerCache )
#include "testqgsvectorlayercache.moc"
