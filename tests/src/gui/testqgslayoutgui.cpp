/***************************************************************************
    testqgslayoutgui.cpp
     ----------------------
    Date                 : October 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgselevationprofilecanvas.h"
#include "qgslayertree.h"
#include "qgslayout.h"
#include "qgslayoutelevationprofilewidget.h"
#include "qgslayoutitemcombobox.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutmodel.h"
#include "qgsmapsettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerelevationproperties.h"

#include <QApplication>
#include <QMainWindow>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

class TestQgsLayoutGui : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void itemTypeComboBox();
    void testProxyCrash();
    void testElevationProfileWidgetLegacy();
    void testElevationProfileWidget();

  private:
};

void TestQgsLayoutGui::initTestCase()
{
}

void TestQgsLayoutGui::cleanupTestCase()
{
}

void TestQgsLayoutGui::init()
{
}

void TestQgsLayoutGui::cleanup()
{
}

void TestQgsLayoutGui::itemTypeComboBox()
{
  // test QgsLayoutItemComboBox
  QgsLayout *layout = new QgsLayout( QgsProject::instance() );

  // create a layout item combobox
  QgsLayoutItemComboBox *cb = new QgsLayoutItemComboBox( nullptr, layout );
  QgsLayoutItemComboBox *cb2 = new QgsLayoutItemComboBox( nullptr, layout );
  cb2->setItemType( QgsLayoutItemRegistry::LayoutShape );

  qRegisterMetaType<QgsLayoutItem *>();
  const QSignalSpy spy1( cb, &QgsLayoutItemComboBox::itemChanged );
  const QSignalSpy spy2( cb2, &QgsLayoutItemComboBox::itemChanged );

  // add some items to layout
  QgsLayoutItemMap *item1 = new QgsLayoutItemMap( layout );
  item1->setId( "item 1" );
  layout->addLayoutItem( item1 );

  QCOMPARE( cb->count(), 1 );
  QCOMPARE( cb2->count(), 0 );
  QCOMPARE( cb->itemText( 0 ), u"item 1"_s );
  QCOMPARE( cb->item( 0 ), item1 );
  QCOMPARE( cb->currentItem(), item1 );
  QVERIFY( !cb2->currentItem() );

  int expectedSpy1Count = 2; // ideally only one, but we'll settle for 2
  int expectedSpy2Count = 0;
  QCOMPARE( spy1.count(), 2 );
  QCOMPARE( qvariant_cast<QObject *>( spy1.at( 1 ).at( 0 ) ), item1 );
  QCOMPARE( spy2.count(), 0 );

  QgsLayoutItemShape *item2 = new QgsLayoutItemShape( layout );
  item2->setId( "item 2" );
  layout->addLayoutItem( item2 );

  QCOMPARE( cb->count(), 2 );
  QCOMPARE( cb2->count(), 1 );
  QCOMPARE( cb->itemText( 0 ), u"item 1"_s );
  QCOMPARE( cb->itemText( 1 ), u"item 2"_s );
  QCOMPARE( cb2->itemText( 0 ), u"item 2"_s );
  QCOMPARE( cb->item( 0 ), item1 );
  QCOMPARE( cb->item( 1 ), item2 );
  QCOMPARE( cb2->item( 0 ), item2 );
  QCOMPARE( cb->currentItem(), item1 );
  QCOMPARE( cb2->currentItem(), item2 );

  QCOMPARE( spy1.count(), expectedSpy1Count ); // must be unchanged from earlier
  expectedSpy2Count = 2;                       // ideally only one, but we'll settle for 2
  QCOMPARE( spy2.count(), expectedSpy2Count );
  QCOMPARE( qvariant_cast<QObject *>( spy2.at( 1 ).at( 0 ) ), item2 );

  QgsLayoutItemMap *item3 = new QgsLayoutItemMap( layout );
  item3->setId( "item 3" );
  layout->addLayoutItem( item3 );

  QCOMPARE( cb->count(), 3 );
  QCOMPARE( cb2->count(), 1 );
  QCOMPARE( cb->itemText( 0 ), u"item 1"_s );
  QCOMPARE( cb->itemText( 1 ), u"item 2"_s );
  QCOMPARE( cb->itemText( 2 ), u"item 3"_s );
  QCOMPARE( cb2->itemText( 0 ), u"item 2"_s );
  QCOMPARE( cb->item( 0 ), item1 );
  QCOMPARE( cb->item( 1 ), item2 );
  QCOMPARE( cb->item( 2 ), item3 );
  QCOMPARE( cb2->item( 0 ), item2 );
  QCOMPARE( cb->currentItem(), item1 );
  QCOMPARE( cb2->currentItem(), item2 );
  QCOMPARE( spy1.count(), expectedSpy1Count ); // must be unchanged from earlier
  QCOMPARE( spy2.count(), expectedSpy2Count );

  item3->setId( "a" );
  QCOMPARE( cb->itemText( 0 ), u"a"_s );
  QCOMPARE( cb->itemText( 1 ), u"item 1"_s );
  QCOMPARE( cb->itemText( 2 ), u"item 2"_s );
  item3->setId( "item 3" );
  QCOMPARE( cb->itemText( 0 ), u"item 1"_s );
  QCOMPARE( cb->itemText( 1 ), u"item 2"_s );
  QCOMPARE( cb->itemText( 2 ), u"item 3"_s );

  //manually change item
  cb->setItem( item3 );
  expectedSpy1Count++;
  QCOMPARE( spy1.count(), expectedSpy1Count );
  QCOMPARE( qvariant_cast<QObject *>( spy1.at( expectedSpy1Count - 1 ).at( 0 ) ), item3 );
  QCOMPARE( cb->currentItem(), item3 );

  cb2->setItem( nullptr );
  QVERIFY( !cb2->currentItem() );
  expectedSpy2Count++;
  QCOMPARE( spy2.count(), expectedSpy2Count );
  QVERIFY( !qvariant_cast<QObject *>( spy2.at( expectedSpy2Count - 1 ).at( 0 ) ) );

  // remove item
  layout->removeLayoutItem( item3 );
  expectedSpy1Count++;
  QCOMPARE( spy1.count(), expectedSpy1Count );
  QCOMPARE( qvariant_cast<QObject *>( spy1.at( expectedSpy1Count - 1 ).at( 0 ) ), item2 );
  QCOMPARE( cb->currentItem(), item2 );
  QCOMPARE( spy2.count(), expectedSpy2Count );
  QCOMPARE( cb->count(), 2 );
  QCOMPARE( cb2->count(), 1 );

  layout->removeLayoutItem( item2 );
  expectedSpy1Count += 2; // ideally just one signal, but we want at LEAST 1...
  QCOMPARE( spy1.count(), expectedSpy1Count );
  QCOMPARE( qvariant_cast<QObject *>( spy1.at( expectedSpy1Count - 1 ).at( 0 ) ), item1 );
  QCOMPARE( cb->currentItem(), item1 );
  QCOMPARE( cb->count(), 1 );
  QCOMPARE( cb2->count(), 0 );
  expectedSpy2Count++;
  QCOMPARE( spy2.count(), expectedSpy2Count );
  QVERIFY( !qvariant_cast<QObject *>( spy2.at( expectedSpy2Count - 1 ).at( 0 ) ) );
  QVERIFY( !cb2->currentItem() );

  layout->removeLayoutItem( item1 );
  expectedSpy1Count += 2; // ideally just one signal, but we want at LEAST 1...
  QCOMPARE( spy1.count(), expectedSpy1Count );
  QVERIFY( !qvariant_cast<QObject *>( spy1.at( expectedSpy1Count - 1 ).at( 0 ) ) );
  QVERIFY( !cb->currentItem() );
  QCOMPARE( cb->count(), 0 );
  QCOMPARE( cb2->count(), 0 );
  QCOMPARE( spy2.count(), expectedSpy2Count );
}

void TestQgsLayoutGui::testProxyCrash()
{
  // test for a possible crash when using QgsLayoutProxyModel and reordering items
  QgsLayout *layout = new QgsLayout( QgsProject::instance() );

  // create a layout item combobox
  QgsLayoutItemComboBox *cb = new QgsLayoutItemComboBox( nullptr, layout );
  QgsLayoutItemComboBox *cb2 = new QgsLayoutItemComboBox( nullptr, layout );
  QgsLayoutItemComboBox *cb3 = new QgsLayoutItemComboBox( nullptr, layout );
  QgsLayoutItemComboBox *cb4 = new QgsLayoutItemComboBox( nullptr, layout );

  // add some items to layout
  QgsLayoutItemMap *item1 = new QgsLayoutItemMap( layout );
  layout->addLayoutItem( item1 );
  QCOMPARE( cb->count(), 1 );
  QgsLayoutItemShape *item2 = new QgsLayoutItemShape( layout );
  layout->addLayoutItem( item2 );
  QCOMPARE( cb->count(), 2 );
  QgsLayoutItemMap *item3 = new QgsLayoutItemMap( layout );
  layout->addLayoutItem( item3 );

  QCOMPARE( cb->count(), 3 );
  cb->setItemType( QgsLayoutItemRegistry::LayoutMap );
  QCOMPARE( cb->count(), 2 );

  cb4->setItemType( QgsLayoutItemRegistry::LayoutShape );

  cb->setItem( item1 );
  QCOMPARE( cb->currentItem(), item1 );
  cb2->setItem( item3 );
  QCOMPARE( cb2->currentItem(), item3 );
  cb3->setItem( item2 );
  QCOMPARE( cb3->currentItem(), item2 );

  // reorder items - expect no crash!
  // we do this by calling the private members, in order to simulate what
  // happens when a drag and drop reorder occurs
  layout->itemsModel()->mItemZList.removeOne( item1 );
  layout->itemsModel()->mItemZList.insert( 1, item1 );
  layout->itemsModel()->rebuildSceneItemList();

  QCOMPARE( cb->currentItem(), item1 );
  QCOMPARE( cb2->currentItem(), item3 );

  layout->itemsModel()->mItemZList.removeOne( item1 );
  layout->itemsModel()->mItemZList.insert( 0, item1 );
  layout->itemsModel()->rebuildSceneItemList();

  delete layout;
}

void TestQgsLayoutGui::testElevationProfileWidgetLegacy()
{
  QgsProject project;
  QgsLayout layout( &project );
  QgsVectorLayer *vl1 = new QgsVectorLayer( u"Point?field=intarray:int[]&field=strarray:string[]&field=intf:int"_s, u"vl1"_s, u"memory"_s );
  qobject_cast< QgsVectorLayerElevationProperties * >( vl1->elevationProperties() )->setZOffset( 5 );
  QgsVectorLayer *vl2 = new QgsVectorLayer( u"Point?field=intarray:int[]&field=strarray:string[]&field=intf:int"_s, u"vl2"_s, u"memory"_s );
  qobject_cast< QgsVectorLayerElevationProperties * >( vl2->elevationProperties() )->setZOffset( 5 );
  QgsVectorLayer *vl3 = new QgsVectorLayer( u"Point?field=intarray:int[]&field=strarray:string[]&field=intf:int"_s, u"vl3"_s, u"memory"_s );
  qobject_cast< QgsVectorLayerElevationProperties * >( vl3->elevationProperties() )->setZOffset( 5 );
  project.addMapLayers( { vl1, vl2, vl3 } );

  QgsLayoutItemElevationProfile profile( &layout );
  // layers will be rendered from v1->vl3 from bottom to top
  profile.setLayers( { vl1, vl2, vl3 } );
  QgsLayoutElevationProfileWidget widget( &profile );

  QCOMPARE( widget.mLayerTree->children().size(), 3 );
  // in widget's layer tree the layers should be in order v1->vl3 from bottom to top
  QCOMPARE( qobject_cast< QgsLayerTreeLayer * >( widget.mLayerTree->children().at( 0 ) )->layer()->name(), u"vl3"_s );
  QVERIFY( widget.mLayerTree->children().at( 0 )->isItemVisibilityCheckedRecursive() );
  QCOMPARE( qobject_cast< QgsLayerTreeLayer * >( widget.mLayerTree->children().at( 1 ) )->layer()->name(), u"vl2"_s );
  QVERIFY( widget.mLayerTree->children().at( 1 )->isItemVisibilityCheckedRecursive() );
  QCOMPARE( qobject_cast< QgsLayerTreeLayer * >( widget.mLayerTree->children().at( 2 ) )->layer()->name(), u"vl1"_s );
  QVERIFY( widget.mLayerTree->children().at( 2 )->isItemVisibilityCheckedRecursive() );

  // uncheck a layer
  widget.mLayerTree->children().at( 1 )->setItemVisibilityChecked( false );
  QCOMPARE( profile.layers().size(), 2 );
  // layers should be in order of rendering, ie vl1 before vl3 as vl3 is rendered on top
  QCOMPARE( profile.layers().at( 0 )->name(), u"vl1"_s );
  QCOMPARE( profile.layers().at( 1 )->name(), u"vl3"_s );

  QgsElevationProfileCanvas canvas;
  // layers will be rendered from v1->vl3 from bottom to top
  canvas.setLayers( { vl1, vl2, vl3 } );

  widget.copySettingsFromProfileCanvas( &canvas );
  QCOMPARE( profile.layers().size(), 3 );
  // layers should be in order of rendering, ie vl1 before vl3 as vl3 is rendered on top
  QCOMPARE( profile.layers().at( 0 )->name(), u"vl1"_s );
  QCOMPARE( profile.layers().at( 1 )->name(), u"vl2"_s );
  QCOMPARE( profile.layers().at( 2 )->name(), u"vl3"_s );

  canvas.setLayers( { vl3, vl2, vl1 } );
  widget.copySettingsFromProfileCanvas( &canvas );
  QCOMPARE( profile.layers().size(), 3 );
  QCOMPARE( profile.layers().at( 0 )->name(), u"vl3"_s );
  QCOMPARE( profile.layers().at( 1 )->name(), u"vl2"_s );
  QCOMPARE( profile.layers().at( 2 )->name(), u"vl1"_s );
}

void TestQgsLayoutGui::testElevationProfileWidget()
{
  QgsProject project;
  QgsLayout layout( &project );
  QgsVectorLayer *vl1 = new QgsVectorLayer( u"Point?field=intarray:int[]&field=strarray:string[]&field=intf:int"_s, u"vl1"_s, u"memory"_s );
  qobject_cast< QgsVectorLayerElevationProperties * >( vl1->elevationProperties() )->setZOffset( 5 );
  QgsVectorLayer *vl2 = new QgsVectorLayer( u"Point?field=intarray:int[]&field=strarray:string[]&field=intf:int"_s, u"vl2"_s, u"memory"_s );
  qobject_cast< QgsVectorLayerElevationProperties * >( vl2->elevationProperties() )->setZOffset( 5 );
  QgsVectorLayer *vl3 = new QgsVectorLayer( u"Point?field=intarray:int[]&field=strarray:string[]&field=intf:int"_s, u"vl3"_s, u"memory"_s );
  qobject_cast< QgsVectorLayerElevationProperties * >( vl3->elevationProperties() )->setZOffset( 5 );
  project.addMapLayers( { vl1, vl2, vl3 } );

  QgsLayoutItemElevationProfile profile( &layout );
  // sources will be rendered from vl3->vl1 from bottom to top
  profile.setSources( { vl1, vl2, vl3 } );
  QgsLayoutElevationProfileWidget widget( &profile );

  QCOMPARE( widget.mLayerTree->children().size(), 3 );
  // in widget's layer tree the sources should be in the same order v1->vl3 than the layout item profile
  QCOMPARE( qobject_cast< QgsLayerTreeLayer * >( widget.mLayerTree->children().at( 0 ) )->layer()->name(), u"vl1"_s );
  QVERIFY( widget.mLayerTree->children().at( 0 )->isItemVisibilityCheckedRecursive() );
  QCOMPARE( qobject_cast< QgsLayerTreeLayer * >( widget.mLayerTree->children().at( 1 ) )->layer()->name(), u"vl2"_s );
  QVERIFY( widget.mLayerTree->children().at( 1 )->isItemVisibilityCheckedRecursive() );
  QCOMPARE( qobject_cast< QgsLayerTreeLayer * >( widget.mLayerTree->children().at( 2 ) )->layer()->name(), u"vl3"_s );
  QVERIFY( widget.mLayerTree->children().at( 2 )->isItemVisibilityCheckedRecursive() );

  // uncheck a layer
  widget.mLayerTree->children().at( 1 )->setItemVisibilityChecked( false );
  QCOMPARE( profile.sources().size(), 2 );
  // sources should be in opposite order of rendering, ie vl1 before vl3 as vl1 is rendered on top
  QCOMPARE( profile.sources().at( 0 )->profileSourceName(), u"vl1"_s );
  QCOMPARE( profile.sources().at( 1 )->profileSourceName(), u"vl3"_s );

  QgsElevationProfileCanvas canvas;
  // sources will be rendered from vl3->vl1 from bottom to top
  canvas.setSources( { vl1, vl2, vl3 } );

  widget.copySettingsFromProfileCanvas( &canvas );
  QCOMPARE( profile.sources().size(), 3 );
  // sources should be in opposite order of rendering, ie vl1 before vl3 as vl1 is rendered on top
  QCOMPARE( profile.sources().at( 0 )->profileSourceName(), u"vl1"_s );
  QCOMPARE( profile.sources().at( 1 )->profileSourceName(), u"vl2"_s );
  QCOMPARE( profile.sources().at( 2 )->profileSourceName(), u"vl3"_s );

  canvas.setSources( { vl3, vl2, vl1 } );
  widget.copySettingsFromProfileCanvas( &canvas );
  QCOMPARE( profile.sources().size(), 3 );
  QCOMPARE( profile.sources().at( 0 )->profileSourceName(), u"vl3"_s );
  QCOMPARE( profile.sources().at( 1 )->profileSourceName(), u"vl2"_s );
  QCOMPARE( profile.sources().at( 2 )->profileSourceName(), u"vl1"_s );
}

QTEST_MAIN( TestQgsLayoutGui )
#include "testqgslayoutgui.moc"
