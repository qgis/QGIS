/***************************************************************************
     testqgselevationprofilewidget.cpp
     --------------------------------------
    Date                 : February 2026
    Copyright            : (C) 2026 by Germán Carrillo
    Email                : german@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "elevation/qgselevationprofilewidget.h"
#include "qgisapp.h"
#include "qgsabstractprofilesource.h"
#include "qgselevationprofile.h"
#include "qgselevationprofilecanvas.h"
#include "qgslayertree.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsprofilesourceregistry.h"
#include "qgstest.h"

#include <QSignalSpy>
#include <QString>

using namespace Qt::StringLiterals;

class MyDummyProfileSource : public QgsAbstractProfileSource
{
  public:
    virtual QgsAbstractProfileGenerator *createProfileGenerator( const QgsProfileRequest & ) override { return nullptr; }
    virtual QString profileSourceName() const override { return u"My Dummy Profile"_s; }
    virtual QString profileSourceId() const override { return u"my-dummy-profile"_s; }
};

/**
 * \ingroup UnitTests
 * This is a unit test for the QgisApp elevation profile widget.
 */
class TestQgsAppElevationProfileWidget : public QObject
{
    Q_OBJECT

  public:
    TestQgsAppElevationProfileWidget();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void registerCustomProfileAddsCustomNode();
    void registerCustomProfileAndThenInitializeProfile();
    void registerCustomProfileInSyncModeNoCrash();
    void registerCustomProfileInSyncMode();
    void registerCustomProfileInNonSyncMode();
    void registerCustomProfileAndToggleSyncMode();
    void testRemovedProjectLayers();
    void testAddedProjectLayers();
    void testChangedLayersWithCustomProfile();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsAppElevationProfileWidget::TestQgsAppElevationProfileWidget() = default;

//runs before all tests
void TestQgsAppElevationProfileWidget::initTestCase()
{
  qDebug() << "TestQgsAppElevationProfileWidget::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgsAppElevationProfileWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

//runs after each testfunction
void TestQgsAppElevationProfileWidget::cleanup()
{
  // Unregister all custom profile sources
  const QList< QgsAbstractProfileSource *> sources = QgsApplication::profileSourceRegistry()->profileSources();
  for ( QgsAbstractProfileSource *source : sources )
  {
    QgsApplication::profileSourceRegistry()->unregisterProfileSource( source->profileSourceId() );
  }
  QVERIFY( QgsApplication::profileSourceRegistry()->profileSources().isEmpty() );

  QgsProject::instance()->clear();
}

void TestQgsAppElevationProfileWidget::registerCustomProfileAddsCustomNode()
{
  QgsElevationProfile *profile = new QgsElevationProfile( QgsProject::instance() );
  QgsElevationProfileWidget::applyDefaultSettingsToProfile( profile );
  QgsElevationProfileWidget *profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );
  QCOMPARE( profile->layerTree()->children().size(), 0 );
  QCOMPARE( profile->layerTree()->findCustomNodeIds().size(), 0 );

  // Register a custom profile and check that there is a custom node in the tree view
  MyDummyProfileSource *source = new MyDummyProfileSource();
  QVERIFY( QgsApplication::profileSourceRegistry()->registerProfileSource( source ) );
  QVERIFY( profile->layerTree() );
  QCOMPARE( profile->layerTree()->children().size(), 1 );
  QCOMPARE( profile->layerTree()->findCustomNodeIds().size(), 1 );
  QCOMPARE( profile->layerTree()->findCustomNodeIds().at( 0 ), "my-dummy-profile"_L1 );

  // Unregister the custom profile and check that the custom node was removed
  QVERIFY( QgsApplication::profileSourceRegistry()->unregisterProfileSource( u"my-dummy-profile"_s ) );
  QCOMPARE( profile->layerTree()->children().size(), 0 );
  QCOMPARE( profile->layerTree()->findCustomNodeIds().size(), 0 );
}

void TestQgsAppElevationProfileWidget::registerCustomProfileAndThenInitializeProfile()
{
  // First, register a custom profile
  MyDummyProfileSource *source = new MyDummyProfileSource();
  QVERIFY( QgsApplication::profileSourceRegistry()->registerProfileSource( source ) );

  // Then, create a new profile widget
  QgsElevationProfile *profile = new QgsElevationProfile( QgsProject::instance() );
  QgsElevationProfileWidget::applyDefaultSettingsToProfile( profile );
  QgsElevationProfileWidget *profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );

  // Check that Canvas has custom source
  QVERIFY( profileWidget->mCanvas->sources().contains( source ) );

  // Check that Elevation tree has custom node
  QVERIFY( !profile->layerTree()->findCustomNodeIds().isEmpty() );
  QVERIFY( profile->layerTree()->findCustomNode( source->profileSourceId() ) );

  // Check that QGIS layer tree has no custom node
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findCustomNodeIds().isEmpty() );

  // Unregister the custom profile
  QVERIFY( QgsApplication::profileSourceRegistry()->unregisterProfileSource( u"my-dummy-profile"_s ) );
}

void TestQgsAppElevationProfileWidget::registerCustomProfileInSyncModeNoCrash()
{
  QgsElevationProfile *profile = new QgsElevationProfile( QgsProject::instance() );
  QgsElevationProfileWidget::applyDefaultSettingsToProfile( profile );
  QgsElevationProfileWidget *profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );

  profile->setUseProjectLayerTree( true ); // Synchronize Layers to Project
  QVERIFY( !profile->layerTree() );

  // Register a custom profile and check that QGIS does not crash (issue #65056)
  MyDummyProfileSource *source = new MyDummyProfileSource();
  QVERIFY( QgsApplication::profileSourceRegistry()->registerProfileSource( source ) );

  // Unregister the custom profile
  QVERIFY( QgsApplication::profileSourceRegistry()->unregisterProfileSource( u"my-dummy-profile"_s ) );
}

void TestQgsAppElevationProfileWidget::registerCustomProfileInSyncMode()
{
  QgsElevationProfile *profile = new QgsElevationProfile( QgsProject::instance() );
  QgsElevationProfileWidget::applyDefaultSettingsToProfile( profile );
  QgsElevationProfileWidget *profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );

  profile->setUseProjectLayerTree( true ); // Synchronize Layers to Project
  QVERIFY( !profile->layerTree() );

  // Register a custom profile
  MyDummyProfileSource *source = new MyDummyProfileSource();
  QVERIFY( QgsApplication::profileSourceRegistry()->registerProfileSource( source ) );

  // Check that Canvas has no custom source
  QVERIFY( !profileWidget->mCanvas->sources().contains( source ) );

  // Check that QGIS layer tree has no custom node
  QVERIFY( profileWidget->mLayerTree->findCustomNodeIds().isEmpty() );
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findCustomNodeIds().isEmpty() );

  // Unregister the custom profile
  QVERIFY( QgsApplication::profileSourceRegistry()->unregisterProfileSource( u"my-dummy-profile"_s ) );
}

void TestQgsAppElevationProfileWidget::registerCustomProfileInNonSyncMode()
{
  QgsElevationProfile *profile = new QgsElevationProfile( QgsProject::instance() );
  QgsElevationProfileWidget::applyDefaultSettingsToProfile( profile );
  QgsElevationProfileWidget *profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );

  profile->setUseProjectLayerTree( false ); // Don't Synchronize Layers to Project
  QVERIFY( profile->layerTree() );

  // Register a custom profile
  MyDummyProfileSource *source = new MyDummyProfileSource();
  QVERIFY( QgsApplication::profileSourceRegistry()->registerProfileSource( source ) );

  // Check that Canvas has custom source
  QVERIFY( profileWidget->mCanvas->sources().contains( source ) );

  // Check that Elevation tree has custom node
  QVERIFY( !profile->layerTree()->findCustomNodeIds().isEmpty() );
  QVERIFY( profile->layerTree()->findCustomNode( source->profileSourceId() ) );

  // Check that QGIS layer tree has no custom node
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findCustomNodeIds().isEmpty() );

  // Unregister the custom profile
  QVERIFY( QgsApplication::profileSourceRegistry()->unregisterProfileSource( u"my-dummy-profile"_s ) );

  // Check that Canvas has no custom source
  QVERIFY( !profileWidget->mCanvas->sources().contains( source ) );

  // Check that Elevation tree has no custom node
  QVERIFY( profile->layerTree()->findCustomNodeIds().isEmpty() );

  // Check that QGIS layer tree has no custom node
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findCustomNodeIds().isEmpty() );
}

void TestQgsAppElevationProfileWidget::registerCustomProfileAndToggleSyncMode()
{
  QgsElevationProfile *profile = new QgsElevationProfile( QgsProject::instance() );
  QgsElevationProfileWidget::applyDefaultSettingsToProfile( profile );
  QgsElevationProfileWidget *profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );

  profile->setUseProjectLayerTree( false ); // Don't Synchronize Layers to Project
  QVERIFY( profile->layerTree() );

  // Register a custom profile
  MyDummyProfileSource *source = new MyDummyProfileSource();
  QVERIFY( QgsApplication::profileSourceRegistry()->registerProfileSource( source ) );

  // Switch to 'Synchronize Layers to Project' mode
  profile->setUseProjectLayerTree( true );
  QVERIFY( !profile->layerTree() );

  // Check that Canvas has no custom source
  QVERIFY( !profileWidget->mCanvas->sources().contains( source ) );

  // Check that QGIS layer tree has no custom node
  QVERIFY( profileWidget->mLayerTree->findCustomNodeIds().isEmpty() );
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findCustomNodeIds().isEmpty() );

  // Switch back to non-'Synchronize Layers to Project' mode
  profile->setUseProjectLayerTree( false );
  QVERIFY( profile->layerTree() );

  // Check that Canvas has custom source
  QVERIFY( profileWidget->mCanvas->sources().contains( source ) );

  // Check that Elevation tree has custom node
  QVERIFY( !profile->layerTree()->findCustomNodeIds().isEmpty() );
  QVERIFY( profile->layerTree()->findCustomNode( source->profileSourceId() ) );

  // Check that QGIS layer tree has no custom node
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findCustomNodeIds().isEmpty() );

  // Unregister the custom profile
  QVERIFY( QgsApplication::profileSourceRegistry()->unregisterProfileSource( u"my-dummy-profile"_s ) );
}

void TestQgsAppElevationProfileWidget::testRemovedProjectLayers()
{
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString layerPath = dataDir + u"/points.shp"_s;

  QgsVectorLayer *layer1 = new QgsVectorLayer( layerPath, u"points 1"_s, u"ogr"_s );
  QgsVectorLayer *layer2 = new QgsVectorLayer( layerPath, u"points 2"_s, u"ogr"_s );
  QgsVectorLayer *layer3 = new QgsVectorLayer( layerPath, u"points 3"_s, u"ogr"_s );

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << layer1 << layer2 << layer3 );

  QgsElevationProfile *profile = new QgsElevationProfile( QgsProject::instance() );
  QgsElevationProfileWidget::applyDefaultSettingsToProfile( profile );
  QgsElevationProfileWidget *profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );
  QVERIFY( profile->layerTree()->findLayer( layer1 ) );
  QVERIFY( profile->layerTree()->findLayer( layer2 ) );
  QVERIFY( profile->layerTree()->findLayer( layer3 ) );

  // close widget and wait for deletion
  QSignalSpy spy( profileWidget, &QObject::destroyed );
  profileWidget->close();
  QVERIFY( spy.wait( 1000 ) );
  profileWidget = nullptr;

  QgsProject::instance()->removeMapLayers( QList<QgsMapLayer *>() << layer1 << layer2 );

  profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );
  QCOMPARE( profile->layerTree()->findLayers().size(), 1 );
  QVERIFY( profile->layerTree()->findLayer( layer3 ) );
}

void TestQgsAppElevationProfileWidget::testAddedProjectLayers()
{
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString layerPath = dataDir + u"/points.shp"_s;

  QgsVectorLayer *layer1 = new QgsVectorLayer( layerPath, u"points 1"_s, u"ogr"_s );

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << layer1 );

  // enable show by default
  layer1->elevationProperties()->setZOffset( 1.0 );
  QVERIFY( layer1->elevationProperties()->showByDefaultInElevationProfilePlots() );

  QgsElevationProfile *profile = new QgsElevationProfile( QgsProject::instance() );
  QgsElevationProfileWidget::applyDefaultSettingsToProfile( profile );
  QgsElevationProfileWidget *profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );
  QVERIFY( profile->layerTree()->findLayer( layer1 ) );
  QCOMPARE( profile->layerTree()->findLayers().size(), 1 );

  // close widget and wait for deletion
  QSignalSpy spy( profileWidget, &QObject::destroyed );
  profileWidget->close();
  QVERIFY( spy.wait( 1000 ) );

  QgsVectorLayer *layer2 = new QgsVectorLayer( layerPath, u"points 2"_s, u"ogr"_s );
  QgsVectorLayer *layer3 = new QgsVectorLayer( layerPath, u"points 3"_s, u"ogr"_s );

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << layer2 << layer3 );
  // enable show by default
  layer2->elevationProperties()->setZOffset( 1.0 );
  QVERIFY( layer2->elevationProperties()->showByDefaultInElevationProfilePlots() );
  layer3->elevationProperties()->setZOffset( 1.0 );
  QVERIFY( layer3->elevationProperties()->showByDefaultInElevationProfilePlots() );

  profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );
  QgsLayerTreeLayer *l;
  l = profile->layerTree()->findLayer( layer1 );
  QVERIFY( l );
  QVERIFY( l->itemVisibilityChecked() );
  l = profile->layerTree()->findLayer( layer2 );
  QVERIFY( l );
  QVERIFY( !l->itemVisibilityChecked() );
  l = profile->layerTree()->findLayer( layer3 );
  QVERIFY( l );
  QVERIFY( !l->itemVisibilityChecked() );
}

void TestQgsAppElevationProfileWidget::testChangedLayersWithCustomProfile()
{
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString layerPath = dataDir + u"/points.shp"_s;

  QgsVectorLayer *layer1 = new QgsVectorLayer( layerPath, u"points 1"_s, u"ogr"_s );
  QgsVectorLayer *layer2 = new QgsVectorLayer( layerPath, u"points 2"_s, u"ogr"_s );

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << layer1 << layer2 );

  // enable show by default
  layer1->elevationProperties()->setZOffset( 1.0 );
  QVERIFY( layer1->elevationProperties()->showByDefaultInElevationProfilePlots() );
  layer2->elevationProperties()->setZOffset( 1.0 );
  QVERIFY( layer2->elevationProperties()->showByDefaultInElevationProfilePlots() );

  QgsElevationProfile *profile = new QgsElevationProfile( QgsProject::instance() );
  QgsElevationProfileWidget::applyDefaultSettingsToProfile( profile );
  QgsElevationProfileWidget *profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );
  QVERIFY( profile->layerTree()->findLayer( layer1 ) );
  QCOMPARE( profile->layerTree()->children().size(), 2 );

  // Register a custom profile and check that there is a custom node in the tree view
  MyDummyProfileSource *source = new MyDummyProfileSource();
  QVERIFY( QgsApplication::profileSourceRegistry()->registerProfileSource( source ) );
  QVERIFY( profile->layerTree() );
  QCOMPARE( profile->layerTree()->children().size(), 3 );
  QCOMPARE( profile->layerTree()->findCustomNodeIds().size(), 1 );

  // close widget and wait for deletion
  QSignalSpy spy( profileWidget, &QObject::destroyed );
  profileWidget->close();
  QVERIFY( spy.wait( 1000 ) );

  QgsVectorLayer *layer3 = new QgsVectorLayer( layerPath, u"points 3"_s, u"ogr"_s );

  QgsProject::instance()->removeMapLayers( QList<QgsMapLayer *>() << layer2 );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << layer3 );

  // enable show by default
  layer3->elevationProperties()->setZOffset( 1.0 );
  QVERIFY( layer3->elevationProperties()->showByDefaultInElevationProfilePlots() );

  // Unregister the custom profile and check that the custom node persists in the closed profile
  QVERIFY( QgsApplication::profileSourceRegistry()->unregisterProfileSource( u"my-dummy-profile"_s ) );

  profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );

  QVERIFY( profile->layerTree()->findLayer( layer1 ) );
  QVERIFY( profile->layerTree()->findLayer( layer3 ) );

  QCOMPARE( profile->layerTree()->findCustomNodeIds().size(), 1 );
  QCOMPARE( profile->layerTree()->children().size(), 3 );
}

QGSTEST_MAIN( TestQgsAppElevationProfileWidget )
#include "testqgselevationprofilewidget.moc"
