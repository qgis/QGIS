/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:54 AKDT 2007
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
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

//qgis includes...
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include "qgsvectorlayerref.h"
#include "qgsmaplayerlistutils.h"

class TestSignalReceiver : public QObject
{
    Q_OBJECT

  public:
    TestSignalReceiver()
      : QObject( nullptr )
    {}
    QPainter::CompositionMode blendMode =  QPainter::CompositionMode_SourceOver ;
  public slots:
    void onBlendModeChanged( const QPainter::CompositionMode blendMode )
    {
      this->blendMode = blendMode;
    }
};

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsMapLayer class.
 */
class TestQgsMapLayer : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapLayer() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void isValid();
    void formatName();

    void setBlendMode();

    void isInScaleRange_data();
    void isInScaleRange();
    void isInScaleRange2();

    void layerRef();
    void layerRefListUtils();
    void layerRefResolveByIdOrNameOnly();
    void layerRefResolveWeakly();

    void styleCategories();


  private:
    QgsVectorLayer *mpLayer = nullptr;
};

void TestQgsMapLayer::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

}

void TestQgsMapLayer::init()
{
  //create some objects that will be used in all tests...
  //create a map layer that will be used in all tests...
  QString myFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
  myFileName = myFileName + "/points.shp";
  QFileInfo myMapFileInfo( myFileName );
  mpLayer = new QgsVectorLayer( myMapFileInfo.filePath(),
                                myMapFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayer( mpLayer );
}

void TestQgsMapLayer::cleanup()
{
  QgsProject::instance()->removeAllMapLayers();
}

void TestQgsMapLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapLayer::isValid()
{
  QVERIFY( mpLayer->isValid() );
}

void TestQgsMapLayer::formatName()
{
  QCOMPARE( QgsMapLayer::formatLayerName( QString() ), QString() );
  QCOMPARE( QgsMapLayer::formatLayerName( QStringLiteral( "layer" ) ), QStringLiteral( "Layer" ) );
  QCOMPARE( QgsMapLayer::formatLayerName( QStringLiteral( "layer name" ) ), QStringLiteral( "Layer Name" ) );
  QCOMPARE( QgsMapLayer::formatLayerName( QStringLiteral( "layer_name" ) ), QStringLiteral( "Layer Name" ) );
}

void TestQgsMapLayer::setBlendMode()
{
  TestSignalReceiver receiver;
  QObject::connect( mpLayer, SIGNAL( blendModeChanged( const QPainter::CompositionMode ) ),
                    &receiver, SLOT( onBlendModeChanged( const QPainter::CompositionMode ) ) );
  QCOMPARE( int( receiver.blendMode ), 0 );
  mpLayer->setBlendMode( QPainter::CompositionMode_Screen );
  // check the signal has been correctly emitted
  QCOMPARE( receiver.blendMode, QPainter::CompositionMode_Screen );
  // check accessor
  QCOMPARE( mpLayer->blendMode(), QPainter::CompositionMode_Screen );
}

void TestQgsMapLayer::isInScaleRange_data()
{
  QTest::addColumn<double>( "scale" );
  QTest::addColumn<bool>( "isInScale" );

  QTest::newRow( "in the middle" ) << 3000.0 << true;
  QTest::newRow( "too low" ) << 1000.0 << false;
  QTest::newRow( "too high" ) << 6000.0 << false;
  QTest::newRow( "max is not inclusive" ) << 5000.0 << false;
  QTest::newRow( "min is inclusive" ) << 2500.0 << true;
  QTest::newRow( "min is inclusive even with conversion errors" ) << static_cast< double >( 1.0f / ( ( float )1.0 / 2500.0 ) ) << true;
}

void TestQgsMapLayer::isInScaleRange()
{
  QFETCH( double, scale );
  QFETCH( bool, isInScale );

  mpLayer->setMaximumScale( 2500.0 );
  mpLayer->setMinimumScale( 5000.0 );
  mpLayer->setScaleBasedVisibility( true );
  QCOMPARE( mpLayer->isInScaleRange( scale ), isInScale );
  //always in scale range if scale based visibility is false
  mpLayer->setScaleBasedVisibility( false );
  QCOMPARE( mpLayer->isInScaleRange( scale ), true );
}

void TestQgsMapLayer::isInScaleRange2()
{
  mpLayer->setMaximumScale( 5000.0 );
  mpLayer->setMinimumScale( 0.0 );
  mpLayer->setScaleBasedVisibility( true );
  QVERIFY( !mpLayer->isInScaleRange( 1000 ) );
  QVERIFY( !mpLayer->isInScaleRange( 1 ) );
  QVERIFY( !mpLayer->isInScaleRange( 4999 ) );
  QVERIFY( mpLayer->isInScaleRange( 5001 ) );
  QVERIFY( mpLayer->isInScaleRange( 15000 ) );

  mpLayer->setMaximumScale( 0.0 );
  mpLayer->setMinimumScale( 5000.0 );
  mpLayer->setScaleBasedVisibility( true );
  QVERIFY( mpLayer->isInScaleRange( 1000 ) );
  QVERIFY( mpLayer->isInScaleRange( 1 ) );
  QVERIFY( mpLayer->isInScaleRange( 4999 ) );
  QVERIFY( !mpLayer->isInScaleRange( 5001 ) );
  QVERIFY( !mpLayer->isInScaleRange( 15000 ) );
}

void TestQgsMapLayer::layerRef()
{
  // construct from layer
  QgsVectorLayerRef ref( mpLayer );
  QCOMPARE( ref.get(), mpLayer );
  QCOMPARE( ref.layer.data(), mpLayer );
  QCOMPARE( ref.layerId, mpLayer->id() );
  QCOMPARE( ref.name, QStringLiteral( "points" ) );
  QCOMPARE( ref.source, mpLayer->publicSource() );
  QCOMPARE( ref.provider, QStringLiteral( "ogr" ) );

  // bool operator
  QVERIFY( ref );
  // -> operator
  QCOMPARE( ref->id(), mpLayer->id() );

  // verify that layer matches layer
  QVERIFY( ref.layerMatchesSource( mpLayer ) );

  // create a weak reference
  QgsVectorLayerRef ref2( mpLayer->id(), QStringLiteral( "points" ), mpLayer->publicSource(), QStringLiteral( "ogr" ) );
  QVERIFY( !ref2 );
  QVERIFY( !ref2.get() );
  QVERIFY( !ref2.layer.data() );
  QCOMPARE( ref2.layerId, mpLayer->id() );
  QCOMPARE( ref2.name, QStringLiteral( "points" ) );
  QCOMPARE( ref2.source, mpLayer->publicSource() );
  QCOMPARE( ref2.provider, QStringLiteral( "ogr" ) );

  // verify that weak reference matches layer
  QVERIFY( ref2.layerMatchesSource( mpLayer ) );

  // resolve layer using project
  QCOMPARE( ref2.resolve( QgsProject::instance() ), mpLayer );
  QVERIFY( ref2 );
  QCOMPARE( ref2.get(), mpLayer );
  QCOMPARE( ref2.layer.data(), mpLayer );
  QCOMPARE( ref2.layerId, mpLayer->id() );
  QCOMPARE( ref2.name, QStringLiteral( "points" ) );
  QCOMPARE( ref2.source, mpLayer->publicSource() );
  QCOMPARE( ref2.provider, QStringLiteral( "ogr" ) );

  // setLayer
  QgsVectorLayerRef ref3;
  QVERIFY( !ref3.get() );
  ref3.setLayer( mpLayer );
  QCOMPARE( ref3.get(), mpLayer );
  QCOMPARE( ref3.layer.data(), mpLayer );
  QCOMPARE( ref3.layerId, mpLayer->id() );
  QCOMPARE( ref3.name, QStringLiteral( "points" ) );
  QCOMPARE( ref3.source, mpLayer->publicSource() );
  QCOMPARE( ref3.provider, QStringLiteral( "ogr" ) );

  // weak resolve
  QgsVectorLayerRef ref4( QStringLiteral( "badid" ), QStringLiteral( "points" ), mpLayer->publicSource(), QStringLiteral( "ogr" ) );
  QVERIFY( !ref4 );
  QVERIFY( !ref4.resolve( QgsProject::instance() ) );
  QCOMPARE( ref4.resolveWeakly( QgsProject::instance() ), mpLayer );
  QCOMPARE( ref4.get(), mpLayer );
  QCOMPARE( ref4.layer.data(), mpLayer );
  QCOMPARE( ref4.layerId, mpLayer->id() );
  QCOMPARE( ref4.name, QStringLiteral( "points" ) );
  QCOMPARE( ref4.source, mpLayer->publicSource() );
  QCOMPARE( ref4.provider, QStringLiteral( "ogr" ) );

  // try resolving a bad reference
  QgsVectorLayerRef ref5( QStringLiteral( "badid" ), QStringLiteral( "points" ), mpLayer->publicSource(), QStringLiteral( "xxx" ) );
  QVERIFY( !ref5.get() );
  QVERIFY( !ref5.resolve( QgsProject::instance() ) );
  QVERIFY( !ref5.resolveWeakly( QgsProject::instance() ) );
}

void TestQgsMapLayer::layerRefListUtils()
{
  // conversion utils
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "a" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "b" ), QStringLiteral( "memory" ) );

  QList<QgsMapLayer *> listRawSource;
  listRawSource << vlA << vlB;

  QList< QgsMapLayerRef > refs = _qgis_listRawToRef( listRawSource );
  QCOMPARE( refs.at( 0 ).get(), vlA );
  QCOMPARE( refs.at( 1 ).get(), vlB );

  QList<QgsMapLayer *> raw = _qgis_listRefToRaw( refs );
  QCOMPARE( raw, QList< QgsMapLayer *>() << vlA << vlB );

  //remove layers
  QgsVectorLayer *vlC = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "c" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *vlD = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "d" ), QStringLiteral( "memory" ) );
  refs << QgsMapLayerRef( vlC ) << QgsMapLayerRef( vlD );

  _qgis_removeLayers( refs, QList< QgsMapLayer *>() << vlB << vlD );
  QCOMPARE( refs.size(), 2 );
  QCOMPARE( refs.at( 0 ).get(), vlA );
  QCOMPARE( refs.at( 1 ).get(), vlC );
}

void TestQgsMapLayer::layerRefResolveByIdOrNameOnly()
{
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "name" ), QStringLiteral( "memory" ) );
  QgsVectorLayerRef ref;
  QgsProject::instance()->addMapLayer( vlA );
  ref.name = vlA->name();
  QCOMPARE( ref.resolveByIdOrNameOnly( QgsProject::instance() ), vlA );
  ref.layerId = vlA->id();
  // Same name, different id
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "name" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( vlB );
  QCOMPARE( ref.resolveByIdOrNameOnly( QgsProject::instance() ), vlA );
  // Remove layer A and check if B is returned (because they have the same name)
  QgsProject::instance()->removeMapLayer( vlA );
  QCOMPARE( ref.resolveByIdOrNameOnly( QgsProject::instance() ), vlB );
  // Cleanup
  QgsProject::instance()->removeAllMapLayers();
}

void TestQgsMapLayer::layerRefResolveWeakly()
{
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "name" ), QStringLiteral( "memory" ) );
  QgsVectorLayerRef ref;
  QgsProject::instance()->addMapLayer( vlA );
  ref.name = vlA->name();
  QVERIFY( ! ref.resolveWeakly( QgsProject::instance() ) );
  QVERIFY( ref.resolveWeakly( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) );

  ref = QgsVectorLayerRef();
  ref.name = QStringLiteral( "another name" );
  QVERIFY( ! ref.resolveWeakly( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) );
  ref.provider = vlA->providerType();
  QVERIFY( ref.resolveWeakly( QgsProject::instance(), QgsVectorLayerRef::MatchType::Provider ) );

  ref = QgsVectorLayerRef();
  ref.name = QStringLiteral( "another name" );
  QVERIFY( ! ref.resolveWeakly( QgsProject::instance(),
                                static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Provider |
                                    QgsVectorLayerRef::MatchType::Name ) ) );
  ref.provider = vlA->providerType();
  QVERIFY( ! ref.resolveWeakly( QgsProject::instance(),
                                static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Provider |
                                    QgsVectorLayerRef::MatchType::Name ) ) );
  ref.name = vlA->name();
  QVERIFY( ref.resolveWeakly( QgsProject::instance(),
                              static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Provider |
                                  QgsVectorLayerRef::MatchType::Name ) ) );

  ref = QgsVectorLayerRef();
  QVERIFY( ! ref.resolveWeakly( QgsProject::instance(),
                                static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Source |
                                    QgsVectorLayerRef::MatchType::Name ) ) );
  ref.source = vlA->publicSource();
  QVERIFY( ! ref.resolveWeakly( QgsProject::instance(),
                                static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Source |
                                    QgsVectorLayerRef::MatchType::Name ) ) );
  ref.name = vlA->name();
  QVERIFY( ref.resolveWeakly( QgsProject::instance(),
                              static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Source |
                                  QgsVectorLayerRef::MatchType::Name ) ) );
}

void TestQgsMapLayer::styleCategories()
{
  // control that AllStyleCategories is actually complete
  QgsMapLayer::StyleCategories allStyleCategories = QgsMapLayer::AllStyleCategories;
  for ( QgsMapLayer::StyleCategory category : qgsEnumMap<QgsMapLayer::StyleCategory>().keys() )
  {
    if ( category == QgsMapLayer::AllStyleCategories )
      continue;

    QVERIFY( allStyleCategories.testFlag( category ) );
  }
}

QGSTEST_MAIN( TestQgsMapLayer )
#include "testqgsmaplayer.moc"
