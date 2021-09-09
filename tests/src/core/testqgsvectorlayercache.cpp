/***************************************************************************
    testqgsvectorlayercache.cpp
     --------------------------------------
    Date                 : 20.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
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
#include <QTemporaryFile>

//qgis includes...
#include "qgsfeatureiterator.h"
#include "qgsvectorlayercache.h"
#include "qgsvectordataprovider.h"
#include "qgsapplication.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgscacheindexfeatureid.h"
#include "qgsvectorlayer.h"

#include <QDebug>

/**
 * @ingroup UnitTests
 * This is a unit test for the vector layer cache
 *
 * \see QgsVectorLayerCache
 */
class TestVectorLayerCache : public QObject
{
    Q_OBJECT
  public:
    TestVectorLayerCache() = default;

  private slots:
    void initTestCase();      // will be called before the first testfunction is executed.
    void cleanupTestCase();   // will be called after the last testfunction was executed.
    void init();              // will be called before each testfunction is executed.
    void cleanup();           // will be called after every testfunction.

    void testCacheOverflow();    // Test cache will work if too many features to cache them all are present
    void testCacheAttrActions(); // Test attribute add/ attribute delete
    void testFeatureActions();   // Test adding/removing features works
    void testSubsetRequest();
    void testFullCache();
    void testFullCacheThroughRequest();
    void testCanUseCacheForRequest();
    void testCacheGeom();
    void testFullCacheWithRect(); // Test that if rect is set then no full cache can exist, see #19468

    void onCommittedFeaturesAdded( const QString &, const QgsFeatureList & );

  private:
    QgsVectorLayerCache           *mVectorLayerCache = nullptr;
    QgsCacheIndexFeatureId        *mFeatureIdIndex = nullptr;
    QgsVectorLayer                *mPointsLayer = nullptr;
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
  backupFiles << QStringLiteral( "points.shp" ) << QStringLiteral( "points.shx" ) << QStringLiteral( "points.dbf" ) << QStringLiteral( "points.prj" );

  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTestDataDir = myDataDir + '/';

  for ( const QString &f : backupFiles )
  {
    const QString origFileName = myTestDataDir + f;
    const QFileInfo origFileInfo( origFileName );

    const QString tmpFileName = QDir::tempPath() + '/' + origFileInfo.baseName() + '_' + QString::number( qApp->applicationPid() ) + '.' + origFileInfo.completeSuffix();

    qDebug() << "Copy " << origFileName << " " << tmpFileName;

    qDebug() << QFile::copy( origFileName, tmpFileName );
    mTmpFiles.insert( origFileName, tmpFileName );
  }

  //
  // load a vector layer
  //
  const QString myPointsFileName = mTmpFiles.value( myTestDataDir + "points.shp" );
  const QFileInfo myPointFileInfo( myPointsFileName );
  mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                     myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
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
  delete mPointsLayer;

  // Clean tmp files
  QMap<QString, QString>::const_iterator it;

  for ( it = mTmpFiles.constBegin(); it != mTmpFiles.constEnd(); ++it )
  {
    const QString tmpFileName = it.value();
    qDebug() << "Remove " << tmpFileName;
    QFile::remove( tmpFileName );
  }

  // also clean up newly created .qix file
  QFile::remove( QStringLiteral( TEST_DATA_DIR ) + "/points.qix" );

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
  const QVariant::Type attrType = QVariant::Int;
  mPointsLayer->addAttribute( QgsField( QStringLiteral( "newAttr" ), attrType, QStringLiteral( "Int" ), 5, 0 ) );
  mPointsLayer->commitChanges();

  QVERIFY( mVectorLayerCache->featureAtId( 15, f ) );
  QVERIFY( f.attribute( "newAttr" ).isValid() );

  const QgsFields allFields = mPointsLayer->fields();
  const int idx = allFields.indexFromName( QStringLiteral( "newAttr" ) );

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

  const QgsFeatureId fid = mAddedFeatures.last().id();

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

  const QgsFields fields = mPointsLayer->fields();
  QStringList requiredFields;
  requiredFields << QStringLiteral( "Class" ) << QStringLiteral( "Cabin Crew" );

  mVectorLayerCache->featureAtId( 16, f );
  const QVariant a = f.attribute( 3 );

  QgsFeatureIterator itSubset = mVectorLayerCache->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( requiredFields, fields ) );
  while ( itSubset.nextFeature( f ) ) {}
  itSubset.close();

  mVectorLayerCache->featureAtId( 16, f );
  QVERIFY( a == f.attribute( 3 ) );
}

void TestVectorLayerCache::testFullCache()
{
  // cache is too small to fit all features
  QgsVectorLayerCache cache( mPointsLayer, 2 );
  QVERIFY( !cache.hasFullCache() );
  QVERIFY( cache.cacheSize() < mPointsLayer->featureCount() );
  // but we set it to full cache
  cache.setFullCache( true );
  // so now it should have sufficient size for all features
  QVERIFY( cache.cacheSize() >= mPointsLayer->featureCount() );
  QVERIFY( cache.hasFullCache() );

  // double check that everything is indeed in the cache
  QgsFeatureIterator it = mPointsLayer->getFeatures();
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    QVERIFY( cache.isFidCached( f.id() ) );
  }

  // add a feature to the layer
  mPointsLayer->startEditing();
  QgsFeature f2( mPointsLayer->fields() );
  QVERIFY( mPointsLayer->addFeature( f2 ) );
  QVERIFY( cache.hasFullCache() );
  QVERIFY( cache.isFidCached( f2.id() ) );

  mPointsLayer->rollBack();
}

void TestVectorLayerCache::testFullCacheThroughRequest()
{
  // make sure cache is sufficient size for all features
  QgsVectorLayerCache cache( mPointsLayer, mPointsLayer->featureCount() * 2 );
  QVERIFY( !cache.hasFullCache() );

  // now request all features from cache
  QgsFeatureIterator it = cache.getFeatures( QgsFeatureRequest() );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    // suck in all features
  }

  // cache should now contain all features, and should iterate through in the same order as the non-cached feature ordering
  it = mPointsLayer->getFeatures();
  QgsFeatureIterator itCached = cache.getFeatures( QgsFeatureRequest() );
  QgsFeature fCached;
  while ( it.nextFeature( f ) )
  {
    QVERIFY( cache.isFidCached( f.id() ) );
    itCached.nextFeature( fCached );
    QCOMPARE( f.id(), fCached.id() );
  }

  // so it should be a full cache!
  QVERIFY( cache.hasFullCache() );
}

void TestVectorLayerCache::testCanUseCacheForRequest()
{
  //first get some feature ids from layer
  QgsFeature f;
  QgsFeatureIterator it = mPointsLayer->getFeatures();
  it.nextFeature( f );
  const QgsFeatureId id1 = f.id();
  it.nextFeature( f );
  const QgsFeatureId id2 = f.id();

  QgsVectorLayerCache cache( mPointsLayer, 10 );
  // initially nothing in cache, so can't use it to fulfill the request
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFid( id1 ), it ) );
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFid( id2 ), it ) );
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFids( QgsFeatureIds() << id1 << id2 ), it ) );
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterRect( QgsRectangle( 1, 2, 3, 4 ) ), it ) );
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterExpression( "$x<5" ), it ) );

  // get just the first feature into the cache
  it = cache.getFeatures( QgsFeatureRequest().setFilterFid( id1 ) );
  while ( it.nextFeature( f ) ) { }
  QCOMPARE( cache.cachedFeatureIds(), QgsFeatureIds() << id1 );
  QVERIFY( cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFid( id1 ), it ) );
  //verify that the returned iterator was correct
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.id(), id1 );
  QVERIFY( !it.nextFeature( f ) );
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFid( id2 ), it ) );
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFids( QgsFeatureIds() << id1 << id2 ), it ) );
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterRect( QgsRectangle( 1, 2, 3, 4 ) ), it ) );
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterExpression( "$x<5" ), it ) );

  // get feature 2 into cache
  it = cache.getFeatures( QgsFeatureRequest().setFilterFid( id2 ) );
  while ( it.nextFeature( f ) ) { }
  QCOMPARE( cache.cachedFeatureIds(), QgsFeatureIds() << id1 << id2 );
  QVERIFY( cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFid( id1 ), it ) );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.id(), id1 );
  QVERIFY( !it.nextFeature( f ) );
  QVERIFY( cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFid( id2 ), it ) );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.id(), id2 );
  QVERIFY( !it.nextFeature( f ) );
  QVERIFY( cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFids( QgsFeatureIds() << id1 << id2 ), it ) );
  QVERIFY( it.nextFeature( f ) );
  QgsFeatureIds result;
  result << f.id();
  QVERIFY( it.nextFeature( f ) );
  result << f.id();
  QCOMPARE( result, QgsFeatureIds() << id1 << id2 );
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterRect( QgsRectangle( 1, 2, 3, 4 ) ), it ) );
  QVERIFY( !cache.canUseCacheForRequest( QgsFeatureRequest().setFilterExpression( "$x<5" ), it ) );

  // can only use rect/expression requests if cache has everything
  cache.setFullCache( true );
  QVERIFY( cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFid( id1 ), it ) );
  QVERIFY( cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFid( id2 ), it ) );
  QVERIFY( cache.canUseCacheForRequest( QgsFeatureRequest().setFilterFids( QgsFeatureIds() << id1 << id2 ), it ) );
  QVERIFY( cache.canUseCacheForRequest( QgsFeatureRequest().setFilterRect( QgsRectangle( 1, 2, 3, 4 ) ), it ) );
  QVERIFY( cache.canUseCacheForRequest( QgsFeatureRequest().setFilterExpression( "$x<5" ), it ) );
}

void TestVectorLayerCache::testCacheGeom()
{
  QgsVectorLayerCache cache( mPointsLayer, 2 );
  // cache geometry
  cache.setCacheGeometry( true );

  //first get some feature ids from layer
  QgsFeature f;
  QgsFeatureIterator it = mPointsLayer->getFeatures();
  it.nextFeature( f );
  const QgsFeatureId id1 = f.id();
  it.nextFeature( f );
  const QgsFeatureId id2 = f.id();

  QgsFeatureRequest req;
  req.setFlags( QgsFeatureRequest::NoGeometry ); // should be ignored by cache
  req.setFilterFids( QgsFeatureIds() << id1 << id2 );

  it = cache.getFeatures( req );
  while ( it.nextFeature( f ) )
  {
    QVERIFY( f.hasGeometry() );
  }

  // disabled geometry caching
  cache.setCacheGeometry( false );
  // we should still have cached features... no need to lose these!
  QCOMPARE( cache.cachedFeatureIds(), QgsFeatureIds() << id1 << id2 );
  it = cache.getFeatures( req );
  while ( it.nextFeature( f ) )
  {
    QVERIFY( f.hasGeometry() );
  }

  // now upgrade cache from no geometry -> geometry, should be cleared since we
  // cannot be confident that features existing in the cache have geometry
  cache.setCacheGeometry( true );
  QVERIFY( cache.cachedFeatureIds().isEmpty() );
  it = cache.getFeatures( req );
  while ( it.nextFeature( f ) )
  {
    QVERIFY( f.hasGeometry() );
  }

  // another test...
  cache.setCacheGeometry( false );
  cache.setFullCache( true );
  QVERIFY( cache.hasFullCache() );
  cache.setCacheGeometry( true );
  QVERIFY( !cache.hasFullCache() );
}

void TestVectorLayerCache::testFullCacheWithRect()
{
  QgsVectorLayerCache cache( mPointsLayer, mPointsLayer->dataProvider()->featureCount() );
  // cache geometry
  cache.setCacheGeometry( true );
  QVERIFY( ! cache.hasFullCache() );
  QgsFeatureRequest req;
  req.setFilterRect( mPointsLayer->dataProvider()->extent().buffered( - mPointsLayer->dataProvider()->extent().width() / 2 ) );
  QgsFeatureIterator it = cache.getFeatures( req );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    QVERIFY( f.hasGeometry() );
  }
  QVERIFY( ! cache.hasFullCache() );

  // Filter rect contains extent
  req.setFilterRect( mPointsLayer->dataProvider()->extent().buffered( 1 ) );
  it = cache.getFeatures( req );
  while ( it.nextFeature( f ) )
  {
    QVERIFY( f.hasGeometry() );
  }
  QVERIFY( cache.hasFullCache() );

}

void TestVectorLayerCache::onCommittedFeaturesAdded( const QString &layerId, const QgsFeatureList &features )
{
  Q_UNUSED( layerId )
  mAddedFeatures.append( features );
}

QGSTEST_MAIN( TestVectorLayerCache )
#include "testqgsvectorlayercache.moc"
