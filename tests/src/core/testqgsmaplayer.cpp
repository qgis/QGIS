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

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QStringList>

//qgis includes...
#include <qgsmaplayer.h>
#include <qgsrasterlayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include "qgsvectorlayerref.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsmaplayerproxymodel.h"
#include "qgsmaplayerstore.h"
#include "qgsproject.h"
#include "qgsxmlutils.h"
#include "qgsvectortilelayer.h"

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
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void isValid();
    void testId();
    void formatName();

    void generalHtmlMetadata();

    void setBlendMode();

    void isInScaleRange_data();
    void isInScaleRange();
    void isInScaleRange2();

    void layerRef();
    void layerRefListUtils();
    void layerRefResolveByIdOrNameOnly();
    void layerRefResolveWeakly();

    void styleCategories();

    void notify();

    void customEnumFlagProperties();

    void readCustomProperties();

    void publicSourceOnGdalWithCredentials();

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
  const QFileInfo myMapFileInfo( myFileName );
  mpLayer = new QgsVectorLayer( myMapFileInfo.filePath(), myMapFileInfo.completeBaseName(), u"ogr"_s );
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

void TestQgsMapLayer::testId()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Point"_s, u"a"_s, u"memory"_s );
  QSignalSpy spy( layer.get(), &QgsMapLayer::idChanged );
  QVERIFY( layer->setId( u"my forced id"_s ) );
  QCOMPARE( layer->id(), u"my forced id"_s );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).toString(), u"my forced id"_s );

  // same id, should not emit signal
  layer->setId( u"my forced id"_s );
  QCOMPARE( spy.count(), 1 );

  // if layer is owned by QgsMapLayerStore, cannot change ID
  QgsMapLayerStore store;
  QgsVectorLayer *layer2 = new QgsVectorLayer( u"Point"_s, u"a"_s, u"memory"_s );
  QSignalSpy spy2( layer2, &QgsMapLayer::idChanged );
  layer2->setId( u"my forced id"_s );
  QCOMPARE( spy2.count(), 1 );
  store.addMapLayer( layer2 );
  QVERIFY( !layer2->setId( u"aaa"_s ) );
  QCOMPARE( layer2->id(), u"my forced id"_s );
  QCOMPARE( spy2.count(), 1 );

  // if layer is owned by QgsProject, cannot change ID
  QgsProject project;
  QgsVectorLayer *layer3 = new QgsVectorLayer( u"Point"_s, u"a"_s, u"memory"_s );
  QSignalSpy spy3( layer3, &QgsMapLayer::idChanged );
  layer3->setId( u"my forced id"_s );
  QCOMPARE( spy3.count(), 1 );
  project.addMapLayer( layer3 );
  QVERIFY( !layer3->setId( u"aaa"_s ) );
  QCOMPARE( layer3->id(), u"my forced id"_s );
  QCOMPARE( spy3.count(), 1 );
}

void TestQgsMapLayer::generalHtmlMetadata()
{
  {
    QgsDataSourceUri ds;
    ds.setParam( u"type"_s, "xyz" );
    ds.setParam( u"zmax"_s, "1" );
    ds.setParam( u"url"_s, "https://s3.amazonaws.com/elevation-tiles-prod/terrarium/{z}/{x}/{y}.png" );
    auto vl = std::make_unique<QgsVectorTileLayer>( ds.encodedUri(), u"testLayer"_s );
    QVERIFY( vl->dataProvider() );
    QVERIFY( vl->dataProvider()->isValid() );
    QCOMPARE( ds.param( u"url"_s ), "https://s3.amazonaws.com/elevation-tiles-prod/terrarium/{z}/{x}/{y}.png" );
    QVERIFY( vl->generalHtmlMetadata().contains( "URL</td><td><a href=\"https://s3.amazonaws.com/elevation-tiles-prod/terrarium/%7Bz%7D/%7Bx%7D/%7By%7D.png" ) );
  }

  {
    QgsDataSourceUri ds;
    ds.setParam( u"type"_s, "mbtiles" );
    ds.setParam( u"zmax"_s, "1" );
    ds.setParam( u"url"_s, u"%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) );
    auto vl = std::make_unique<QgsVectorTileLayer>( ds.encodedUri(), u"testLayer"_s );
    QVERIFY( vl->dataProvider() );
    QVERIFY( vl->dataProvider()->isValid() );
    QCOMPARE( ds.param( u"url"_s ), u"%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) );
    QVERIFY( vl->generalHtmlMetadata().contains( u"Path</td><td><a href=\"file://%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) ) );
  }
}

void TestQgsMapLayer::formatName()
{
  QCOMPARE( QgsMapLayer::formatLayerName( QString() ), QString() );
  QCOMPARE( QgsMapLayer::formatLayerName( u"layer"_s ), u"Layer"_s );
  QCOMPARE( QgsMapLayer::formatLayerName( u"layer name"_s ), u"Layer Name"_s );
  QCOMPARE( QgsMapLayer::formatLayerName( u"layer_name"_s ), u"Layer Name"_s );
}

void TestQgsMapLayer::setBlendMode()
{
  const QSignalSpy spy( mpLayer, &QgsMapLayer::blendModeChanged );

  mpLayer->setBlendMode( QPainter::CompositionMode_Screen );
  // check the signal has been correctly emitted
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), static_cast<int>( QPainter::CompositionMode_Screen ) );
  // check accessor
  QCOMPARE( mpLayer->blendMode(), QPainter::CompositionMode_Screen );

  mpLayer->setBlendMode( QPainter::CompositionMode_Screen );
  QCOMPARE( spy.count(), 1 );

  mpLayer->setBlendMode( QPainter::CompositionMode_Darken );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).toInt(), static_cast<int>( QPainter::CompositionMode_Darken ) );
  QCOMPARE( mpLayer->blendMode(), QPainter::CompositionMode_Darken );
}

void TestQgsMapLayer::isInScaleRange_data()
{
  QTest::addColumn<double>( "scale" );
  QTest::addColumn<bool>( "isInScale" );

  QTest::newRow( "in the middle" ) << 3000.0 << true;
  QTest::newRow( "too low" ) << 1000.0 << false;
  QTest::newRow( "too high" ) << 6000.0 << false;
  QTest::newRow( "min is not inclusive" ) << 5000.0 << false;
  QTest::newRow( "max is inclusive" ) << 2500.0 << true;
  QTest::newRow( "max is inclusive even with conversion errors" ) << static_cast<double>( 1.0f / ( ( float ) 1.0 / 2500.0 ) ) << true;
  QTest::newRow( "max is inclusive even with non-round scales (below)" ) << 2499.9999999966526 << true;
  QTest::newRow( "max is inclusive even with non-round scales (above)" ) << 2500.0000000027226 << true;
  QTest::newRow( "min is exclusive even with non-round scales (below)" ) << 4999.999999997278 << false;
  QTest::newRow( "min is exclusive even with non-round scales (above)" ) << 5000.000000003348 << false;
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
  const QgsVectorLayerRef ref( mpLayer );
  QCOMPARE( ref.get(), mpLayer );
  QCOMPARE( ref.layer.data(), mpLayer );
  QCOMPARE( ref.layerId, mpLayer->id() );
  QCOMPARE( ref.name, u"points"_s );
  QCOMPARE( ref.source, mpLayer->publicSource() );
  QCOMPARE( ref.provider, u"ogr"_s );

  // bool operator
  QVERIFY( ref );
  // -> operator
  QCOMPARE( ref->id(), mpLayer->id() );

  // verify that layer matches layer
  QVERIFY( ref.layerMatchesSource( mpLayer ) );

  // create a weak reference
  QgsVectorLayerRef ref2( mpLayer->id(), u"points"_s, mpLayer->publicSource(), u"ogr"_s );
  QVERIFY( !ref2 );
  QVERIFY( !ref2.get() );
  QVERIFY( !ref2.layer.data() );
  QCOMPARE( ref2.layerId, mpLayer->id() );
  QCOMPARE( ref2.name, u"points"_s );
  QCOMPARE( ref2.source, mpLayer->publicSource() );
  QCOMPARE( ref2.provider, u"ogr"_s );

  // verify that weak reference matches layer
  QVERIFY( ref2.layerMatchesSource( mpLayer ) );

  // resolve layer using project
  QCOMPARE( ref2.resolve( QgsProject::instance() ), mpLayer );
  QVERIFY( ref2 );
  QCOMPARE( ref2.get(), mpLayer );
  QCOMPARE( ref2.layer.data(), mpLayer );
  QCOMPARE( ref2.layerId, mpLayer->id() );
  QCOMPARE( ref2.name, u"points"_s );
  QCOMPARE( ref2.source, mpLayer->publicSource() );
  QCOMPARE( ref2.provider, u"ogr"_s );

  // setLayer
  QgsVectorLayerRef ref3;
  QVERIFY( !ref3.get() );
  ref3.setLayer( mpLayer );
  QCOMPARE( ref3.get(), mpLayer );
  QCOMPARE( ref3.layer.data(), mpLayer );
  QCOMPARE( ref3.layerId, mpLayer->id() );
  QCOMPARE( ref3.name, u"points"_s );
  QCOMPARE( ref3.source, mpLayer->publicSource() );
  QCOMPARE( ref3.provider, u"ogr"_s );

  // weak resolve
  QgsVectorLayerRef ref4( u"badid"_s, u"points"_s, mpLayer->publicSource(), u"ogr"_s );
  QVERIFY( !ref4 );
  QVERIFY( !ref4.resolve( QgsProject::instance() ) );
  QCOMPARE( ref4.resolveWeakly( QgsProject::instance() ), mpLayer );
  QCOMPARE( ref4.get(), mpLayer );
  QCOMPARE( ref4.layer.data(), mpLayer );
  QCOMPARE( ref4.layerId, mpLayer->id() );
  QCOMPARE( ref4.name, u"points"_s );
  QCOMPARE( ref4.source, mpLayer->publicSource() );
  QCOMPARE( ref4.provider, u"ogr"_s );

  // try resolving a bad reference
  QgsVectorLayerRef ref5( u"badid"_s, u"points"_s, mpLayer->publicSource(), u"xxx"_s );
  QVERIFY( !ref5.get() );
  QVERIFY( !ref5.resolve( QgsProject::instance() ) );
  QVERIFY( !ref5.resolveWeakly( QgsProject::instance() ) );
}

void TestQgsMapLayer::layerRefListUtils()
{
  // conversion utils
  QgsVectorLayer *vlA = new QgsVectorLayer( u"Point"_s, u"a"_s, u"memory"_s );
  QgsVectorLayer *vlB = new QgsVectorLayer( u"Point"_s, u"b"_s, u"memory"_s );

  QList<QgsMapLayer *> listRawSource;
  listRawSource << vlA << vlB;

  QList<QgsMapLayerRef> refs = _qgis_listRawToRef( listRawSource );
  QCOMPARE( refs.at( 0 ).get(), vlA );
  QCOMPARE( refs.at( 1 ).get(), vlB );

  const QList<QgsMapLayer *> raw = _qgis_listRefToRaw( refs );
  QCOMPARE( raw, QList<QgsMapLayer *>() << vlA << vlB );

  //remove layers
  QgsVectorLayer *vlC = new QgsVectorLayer( u"Point"_s, u"c"_s, u"memory"_s );
  QgsVectorLayer *vlD = new QgsVectorLayer( u"Point"_s, u"d"_s, u"memory"_s );
  refs << QgsMapLayerRef( vlC ) << QgsMapLayerRef( vlD );

  _qgis_removeLayers( refs, QList<QgsMapLayer *>() << vlB << vlD );
  QCOMPARE( refs.size(), 2 );
  QCOMPARE( refs.at( 0 ).get(), vlA );
  QCOMPARE( refs.at( 1 ).get(), vlC );
}

void TestQgsMapLayer::layerRefResolveByIdOrNameOnly()
{
  QgsVectorLayer *vlA = new QgsVectorLayer( u"Point"_s, u"name"_s, u"memory"_s );
  QgsVectorLayerRef ref;
  QgsProject::instance()->addMapLayer( vlA );
  ref.name = vlA->name();
  QCOMPARE( ref.resolveByIdOrNameOnly( QgsProject::instance() ), vlA );
  ref.layerId = vlA->id();
  // Same name, different id
  QgsVectorLayer *vlB = new QgsVectorLayer( u"Point"_s, u"name"_s, u"memory"_s );
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
  QgsVectorLayer *vlA = new QgsVectorLayer( u"Point"_s, u"name"_s, u"memory"_s );
  QgsVectorLayerRef ref;
  QgsProject::instance()->addMapLayer( vlA );
  ref.name = vlA->name();
  QVERIFY( !ref.resolveWeakly( QgsProject::instance() ) );
  QVERIFY( ref.resolveWeakly( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) );

  ref = QgsVectorLayerRef();
  ref.name = u"another name"_s;
  QVERIFY( !ref.resolveWeakly( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) );
  ref.provider = vlA->providerType();
  QVERIFY( ref.resolveWeakly( QgsProject::instance(), QgsVectorLayerRef::MatchType::Provider ) );

  ref = QgsVectorLayerRef();
  ref.name = u"another name"_s;
  QVERIFY( !ref.resolveWeakly( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Provider | QgsVectorLayerRef::MatchType::Name ) ) );
  ref.provider = vlA->providerType();
  QVERIFY( !ref.resolveWeakly( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Provider | QgsVectorLayerRef::MatchType::Name ) ) );
  ref.name = vlA->name();
  QVERIFY( ref.resolveWeakly( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Provider | QgsVectorLayerRef::MatchType::Name ) ) );

  ref = QgsVectorLayerRef();
  QVERIFY( !ref.resolveWeakly( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Source | QgsVectorLayerRef::MatchType::Name ) ) );
  ref.source = vlA->publicSource();
  QVERIFY( !ref.resolveWeakly( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Source | QgsVectorLayerRef::MatchType::Name ) ) );
  ref.name = vlA->name();
  QVERIFY( ref.resolveWeakly( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Source | QgsVectorLayerRef::MatchType::Name ) ) );
}

void TestQgsMapLayer::styleCategories()
{
  // control that AllStyleCategories is actually complete
  const QgsMapLayer::StyleCategories allStyleCategories = QgsMapLayer::AllStyleCategories;

  const QMap<QgsMapLayer::StyleCategory, QString> styleCats = qgsEnumMap<QgsMapLayer::StyleCategory>();
  for ( auto it = styleCats.keyBegin(); it != styleCats.keyEnd(); it++ )
  {
    const QgsMapLayer::StyleCategory category = *it;
    if ( category == QgsMapLayer::AllStyleCategories )
      continue;

    QVERIFY( allStyleCategories.testFlag( category ) );
  }
}

void TestQgsMapLayer::notify()
{
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point"_s, u"name"_s, u"memory"_s );
  QVERIFY( vl->dataProvider() );

  const QSignalSpy spyRepaint( vl, &QgsMapLayer::repaintRequested );
  const QSignalSpy spyDataChanged( vl, &QgsMapLayer::dataChanged );

  vl->setRefreshOnNotifyEnabled( true );
  emit vl->dataProvider()->notify( "test" );
  QCOMPARE( spyRepaint.count(), 1 );
  QCOMPARE( spyDataChanged.count(), 1 );

  vl->setRefreshOnNotifyEnabled( false );
  emit vl->dataProvider()->notify( "test" );
  QCOMPARE( spyRepaint.count(), 1 );
  QCOMPARE( spyDataChanged.count(), 1 );

  vl->setRefreshOnNotifyEnabled( true );
  vl->setRefreshOnNofifyMessage( "test" );
  emit vl->dataProvider()->notify( "test" );
  QCOMPARE( spyRepaint.count(), 2 );
  QCOMPARE( spyDataChanged.count(), 2 );

  vl->setRefreshOnNofifyMessage( "test" );
  emit vl->dataProvider()->notify( "nottest" );
  QCOMPARE( spyRepaint.count(), 2 );
  QCOMPARE( spyDataChanged.count(), 2 );
}

void TestQgsMapLayer::customEnumFlagProperties()
{
  auto ml = std::make_unique<QgsVectorLayer>( u"Point"_s, u"name"_s, u"memory"_s );

  // assign to inexisting property
  ml->setCustomProperty( u"my_property_for_units"_s, -1 );
  ml->setCustomProperty( u"my_property_for_units_as_string"_s, u"myString"_s );
  // just to be sure it really doesn't exist
  QVERIFY( static_cast<int>( Qgis::LayoutUnit::Meters ) != -1 );

  // standard method returns invalid property
  const int v1 = ml->customProperty( u"my_property_for_units"_s, static_cast<int>( Qgis::LayoutUnit::Meters ) ).toInt();
  QCOMPARE( v1, -1 );

  // enum method returns default property if current property is incorrect
  const Qgis::LayoutUnit v2 = ml->customEnumProperty( u"my_property_for_units"_s, Qgis::LayoutUnit::Meters );
  QCOMPARE( v2, Qgis::LayoutUnit::Meters );
  const Qgis::LayoutUnit v2s = ml->customEnumProperty( u"my_property_for_units_as_string"_s, Qgis::LayoutUnit::Meters );
  QCOMPARE( v2s, Qgis::LayoutUnit::Meters );

  // test a different property than default
  ml->setCustomEnumProperty( u"my_property_for_units"_s, Qgis::LayoutUnit::Centimeters );
  const Qgis::LayoutUnit v3 = ml->customEnumProperty( u"my_property_for_units"_s, Qgis::LayoutUnit::Meters );
  QCOMPARE( v3, Qgis::LayoutUnit::Centimeters );
  ml->setCustomEnumProperty( u"my_property_for_units"_s, Qgis::LayoutUnit::Centimeters );
  // auto conversion of old ml (int to str)
  QCOMPARE( ml->customProperty( "my_property_for_units" ).toString(), u"Centimeters"_s );
  const Qgis::LayoutUnit v3s = ml->customEnumProperty( u"my_property_for_units"_s, Qgis::LayoutUnit::Meters );
  QCOMPARE( v3s, Qgis::LayoutUnit::Centimeters );
  const QString v3ss = ml->customProperty( u"my_property_for_units"_s, u"myDummyValue"_s ).toString();
  QCOMPARE( v3ss, u"Centimeters"_s );

  // Flags
  const Qgis::LayerFilters pointAndLine = Qgis::LayerFilters( Qgis::LayerFilter::PointLayer | Qgis::LayerFilter::LineLayer );
  const Qgis::LayerFilters pointAndPolygon = Qgis::LayerFilters( Qgis::LayerFilter::PointLayer | Qgis::LayerFilter::PolygonLayer );
  ml->setCustomProperty( u"my_property_for_a_flag"_s, 1e8 ); // invalid
  const Qgis::LayerFilters v4 = ml->customFlagProperty( u"my_property_for_a_flag"_s, pointAndLine );
  QCOMPARE( v4, pointAndLine );

  ml->setCustomProperty( u"my_property_for_a_flag"_s, static_cast<int>( pointAndPolygon ) );
  const Qgis::LayerFilters v5 = ml->customFlagProperty( u"my_property_for_a_flag"_s, pointAndLine );
  QCOMPARE( v5, pointAndPolygon );
  // auto conversion of old property (int to str)
  QCOMPARE( ml->customProperty( "my_property_for_a_flag" ).toString(), u"PointLayer|PolygonLayer"_s );

  ml->setCustomFlagProperty( u"my_property_for_a_flag_as_string"_s, pointAndPolygon );
  const Qgis::LayerFilters v5s = ml->customFlagProperty( u"my_property_for_a_flag_as_string"_s, pointAndLine );
  QCOMPARE( v5s, pointAndPolygon );
  const QString v5ss = ml->customProperty( u"my_property_for_a_flag_as_string"_s, u"myDummyString"_s ).toString();
  QCOMPARE( v5ss, u"PointLayer|PolygonLayer"_s );
}

void TestQgsMapLayer::readCustomProperties()
{
  auto ml = std::make_unique<QgsVectorLayer>( u"Point"_s, u"name"_s, u"memory"_s );

  // assign to inexisting property
  ml->setCustomProperty( u"my_property_one"_s, 42 );
  ml->setCustomProperty( u"my_property_two"_s, u"test2"_s );
  ml->setCustomProperty( u"my_property_three"_s, u"test3"_s );

  QMap<QString, QVariant> map;
  map["my_property_one"] = 51;
  map["my_property_two"] = u"test2 different"_s;
  map["my_property_three"] = u"test3"_s;

  QDomDocument doc( u"qgis"_s );
  QDomElement rootNode = doc.createElement( u"qgis"_s );
  QDomElement propsElement = doc.createElement( u"customproperties"_s );
  propsElement.appendChild( QgsXmlUtils::writeVariant( map, doc ) );
  rootNode.appendChild( propsElement );

  const QSignalSpy spy( ml.get(), &QgsMapLayer::customPropertyChanged );
  ml->readCustomProperties( rootNode, "group" );

  const QgsObjectCustomProperties &props = ml->customProperties();
  QCOMPARE( props.value( u"my_property_one"_s ), 51 );
  QCOMPARE( props.value( u"my_property_two"_s ), u"test2 different"_s );
  QCOMPARE( props.value( u"my_property_three"_s ), u"test3"_s );

  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 0 ).at( 0 ), "my_property_one" );
  QCOMPARE( spy.at( 1 ).at( 0 ), "my_property_two" );
}

void TestQgsMapLayer::publicSourceOnGdalWithCredentials()
{
  QgsRasterLayer rl(
    u"test.tif|option:AN=OPTION|credential:SOMEKEY=AAAAA|credential:ANOTHER=BBB"_s, QString(), u"gdal"_s
  );
  QCOMPARE( rl.publicSource( true ), u"test.tif|option:AN=OPTION|credential:ANOTHER=XXXXXXXX|credential:SOMEKEY=XXXXXXXX"_s );
  QCOMPARE( rl.publicSource( false ), u"test.tif|option:AN=OPTION"_s );
  QCOMPARE( rl.publicSource(), u"test.tif|option:AN=OPTION"_s );
}

QGSTEST_MAIN( TestQgsMapLayer )
#include "testqgsmaplayer.moc"
