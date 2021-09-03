/***************************************************************************
                         testqgslayoutmodel.cpp
                         -----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayout.h"
#include "qgslayoutmodel.h"
#include "qgslayoutitemmap.h"
#include "qgsapplication.h"
#include "qgsmapsettings.h"
#include "qgsproject.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutitemshape.h"
#include <QObject>
#include "qgstest.h"
#include <QList>
#include <QSignalSpy>

class TestQgsLayoutModel : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutModel() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void creation(); //check that model has been created
    void addItems(); //add some items to the composition and test model
    void zList(); //check model z order list
    void clear(); //check clearing the model
    void addLayoutItemDirectly(); //add an item directly to the model
    void rebuildZList(); //test rebuilding the z list from the current composer stacking
    void removeItem(); //test removing an item from the model
    void reorderUp(); //test reordering an item up
    void reorderDown(); //test reordering an item down
    void reorderTop(); //test reordering an item to top
    void reorderBottom(); //test reordering an item to bottom
    void moveItem(); //test move an item in the item tree
    void findItemAbove(); //test getting composer item above
    void findItemBelow(); //test getting composer item below
    void setItemRemoved(); //test setting an item as removed
    void rebuildZListWithRemoved(); //test rebuilding z list with removed items
    void reorderUpWithRemoved(); //test reordering up with removed items
    void reorderDownWithRemoved(); //test reordering down with removed items
    void reorderToTopWithRemoved(); //test reordering to top with removed items
    void reorderToBottomWithRemoved(); //test reordering to bottom with removed items
    void groupSelection();

    void proxy();
    void proxyCrash();

};

void TestQgsLayoutModel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsLayoutModel::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayoutModel::init()
{

}

void TestQgsLayoutModel::cleanup()
{

}

void TestQgsLayoutModel::creation()
{
  QgsLayout layout( QgsProject::instance() );
  QVERIFY( layout.itemsModel() );
  //check some basic things
  QCOMPARE( layout.itemsModel()->columnCount(), 3 );
  // models are initially shown with a "null" item
  QCOMPARE( layout.itemsModel()->rowCount(), 1 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 0 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 0 );
}

void TestQgsLayoutModel::addItems()
{
  QgsLayout layout( QgsProject::instance() );
  //add some items to the composition
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 0 );
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  item1->setId( QStringLiteral( "item1" ) );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  item2->setId( QStringLiteral( "item2" ) );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  item3->setId( QStringLiteral( "item3" ) );
  //check that these items have been added to the model
  QCOMPARE( layout.itemsModel()->rowCount(), 4 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "item3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "item2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "item1" ) );
  //and the scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 3 );
}

void TestQgsLayoutModel::zList()
{
  QgsLayout layout( QgsProject::instance() );
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );

  //check z list for items added by TestQgsLayoutModel::addLayoutItems()
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );
  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 2 ), item1 );
}

void TestQgsLayoutModel::clear()
{
  QgsLayout layout( QgsProject::instance() );
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );

  layout.itemsModel()->clear();
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 0 );
  QCOMPARE( layout.itemsModel()->rowCount(), 1 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 0 );
}

void TestQgsLayoutModel::addLayoutItemDirectly()
{
  //create some items not attached to the layout
  QgsLayoutItemMap *bottomItem = new QgsLayoutItemMap( nullptr );
  bottomItem->setId( QStringLiteral( "bottom" ) );
  QgsLayoutItemMap *topItem = new QgsLayoutItemMap( nullptr );
  topItem->setId( QStringLiteral( "top" ) );

  QgsLayout layout( QgsProject::instance() );
  layout.addItem( bottomItem );
  layout.addItem( topItem );

  layout.itemsModel()->addItemAtTop( bottomItem );
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 1 );
  QCOMPARE( layout.itemsModel()->rowCount(), 2 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "bottom" ) );

  layout.itemsModel()->addItemAtTop( topItem );
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( layout.itemsModel()->rowCount(), 3 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "top" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "bottom" ) );

  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), topItem );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), bottomItem );

  //also check scene list (these items are treated by the model as belonging to the scene)
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), topItem );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), bottomItem );

  layout.itemsModel()->clear();
}

void TestQgsLayoutModel::rebuildZList()
{
  //start with an empty model
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  item1->setId( QStringLiteral( "i1" ) );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  item2->setId( QStringLiteral( "i2" ) );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  item3->setId( QStringLiteral( "i3" ) );

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check that these items have been added to the model
  QCOMPARE( layout.itemsModel()->rowCount(), 4 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );

  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 2 ), item1 );
}

void TestQgsLayoutModel::removeItem()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  item1->setId( QStringLiteral( "i1" ) );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  item2->setId( QStringLiteral( "i2" ) );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  item3->setId( QStringLiteral( "i3" ) );

  // start with an empty model
  layout.itemsModel()->clear();

  item1 = new QgsLayoutItemMap( nullptr );
  item1->setId( QStringLiteral( "map1" ) );
  item2 = new QgsLayoutItemMap( nullptr );
  layout.addItem( item1 );

  //add one item to the model
  layout.itemsModel()->addItemAtTop( item1 );
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 1 );
  QCOMPARE( layout.itemsModel()->rowCount(), 2 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "map1" ) );

  //also check scene list (this item is treated by the model as belonging to the scene)
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 1 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item1 );

  //try removing a missing item
  layout.itemsModel()->removeItem( nullptr );
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 1 );
  QCOMPARE( layout.itemsModel()->rowCount(), 2 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "map1" ) );

  //try removing an item not in the model
  layout.itemsModel()->removeItem( item2 );
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 1 );
  QCOMPARE( layout.itemsModel()->rowCount(), 2 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "map1" ) );

  //remove the item which is in the model
  layout.itemsModel()->removeItem( item1 );
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 0 );
  QCOMPARE( layout.itemsModel()->rowCount(), 1 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 0 );

  delete item1;
  delete item2;
}

void TestQgsLayoutModel::reorderUp()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  item1->setId( QStringLiteral( "i1" ) );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  item2->setId( QStringLiteral( "i2" ) );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  item3->setId( QStringLiteral( "i3" ) );

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //try reordering some bad items
  QVERIFY( ! layout.itemsModel()->reorderItemUp( nullptr ) );
  QgsLayoutItemMap *label = new QgsLayoutItemMap( nullptr );
  QVERIFY( ! layout.itemsModel()->reorderItemUp( label ) );

  //trying to reorder up the topmost item should fail
  QVERIFY( ! layout.itemsModel()->reorderItemUp( item3 ) );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //try reorder a good item, should succeed
  QVERIFY( layout.itemsModel()->reorderItemUp( item2 ) );

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );
  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 2 ), item1 );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  delete label;
}

void TestQgsLayoutModel::reorderDown()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  item1->setId( QStringLiteral( "i1" ) );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  item2->setId( QStringLiteral( "i2" ) );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  item3->setId( QStringLiteral( "i3" ) );

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //try reordering some bad items
  QVERIFY( ! layout.itemsModel()->reorderItemDown( nullptr ) );
  QgsLayoutItemMap *label = new QgsLayoutItemMap( nullptr );
  QVERIFY( ! layout.itemsModel()->reorderItemDown( label ) );

  //trying to reorder down the bottommost item should fail
  QVERIFY( ! layout.itemsModel()->reorderItemDown( item1 ) );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //try reorder a good item, should succeed
  QVERIFY( layout.itemsModel()->reorderItemDown( item2 ) );

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item1 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item2 );
  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item1 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 2 ), item2 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );

  delete label;
}

void TestQgsLayoutModel::reorderTop()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  item1->setId( QStringLiteral( "i1" ) );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  item2->setId( QStringLiteral( "i2" ) );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  item3->setId( QStringLiteral( "i3" ) );

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //try reordering some bad items
  QVERIFY( ! layout.itemsModel()->reorderItemToTop( nullptr ) );
  QgsLayoutItemMap *label = new QgsLayoutItemMap( nullptr );
  QVERIFY( ! layout.itemsModel()->reorderItemToTop( label ) );

  //trying to reorder up the topmost item should fail
  QVERIFY( ! layout.itemsModel()->reorderItemToTop( item3 ) );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //try reorder a good item, should succeed
  QVERIFY( layout.itemsModel()->reorderItemToTop( item1 ) );

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item1 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item2 );
  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item1 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 2 ), item2 );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );

  delete label;
}

void TestQgsLayoutModel::reorderBottom()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  item1->setId( QStringLiteral( "i1" ) );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  item2->setId( QStringLiteral( "i2" ) );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  item3->setId( QStringLiteral( "i3" ) );

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //try reordering some bad items
  QVERIFY( ! layout.itemsModel()->reorderItemToBottom( nullptr ) );
  QgsLayoutItemMap *label = new QgsLayoutItemMap( nullptr );
  QVERIFY( ! layout.itemsModel()->reorderItemToBottom( label ) );

  //trying to reorder down the bottommost item should fail
  QVERIFY( ! layout.itemsModel()->reorderItemToBottom( item1 ) );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //try reorder a good item, should succeed
  QVERIFY( layout.itemsModel()->reorderItemToBottom( item3 ) );

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item1 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item3 );
  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item1 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 2 ), item3 );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );

  delete label;
}

void TestQgsLayoutModel::moveItem()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  item1->setId( QStringLiteral( "i1" ) );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  item2->setId( QStringLiteral( "i2" ) );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  item3->setId( QStringLiteral( "i3" ) );

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  QgsLayoutModel *model = layout.itemsModel();
  QMimeData *mimedata = model->mimeData( QModelIndexList() << model->index( 2, 2 ) ); // get i2
  model->dropMimeData( mimedata, Qt::MoveAction, 1, 2, QModelIndex() ); // move i2 at the top

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  mimedata = model->mimeData( QModelIndexList() << model->index( 1, 2 ) ); // get i2
  model->dropMimeData( mimedata, Qt::MoveAction, -1, -1, QModelIndex() ); // move i2 at the bottom

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );

  mimedata = model->mimeData( QModelIndexList() << model->index( 3, 2 ) ); // get i2
  model->dropMimeData( mimedata, Qt::MoveAction, 2, 2, QModelIndex() ); // move i2 between i3 and i1

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );
}

void TestQgsLayoutModel::findItemAbove()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );

  //try getting item above some bad items
  QVERIFY( ! layout.itemsModel()->findItemAbove( nullptr ) );
  QgsLayoutItemMap *label = new QgsLayoutItemMap( nullptr );
  QVERIFY( ! layout.itemsModel()->findItemAbove( label ) );

  //trying to get item above topmost item should fail
  QVERIFY( ! layout.itemsModel()->findItemAbove( item3 ) );

  //try using a good item
  QCOMPARE( layout.itemsModel()->findItemAbove( item2 ), item3 );
  QCOMPARE( layout.itemsModel()->findItemAbove( item1 ), item2 );

  delete label;
}

void TestQgsLayoutModel::findItemBelow()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );

  //try getting item below some bad items
  QVERIFY( ! layout.itemsModel()->findItemBelow( nullptr ) );
  QgsLayoutItemMap *label = new QgsLayoutItemMap( nullptr );
  QVERIFY( ! layout.itemsModel()->findItemBelow( label ) );

  //trying to get item below bottom most item should fail
  QVERIFY( ! layout.itemsModel()->findItemBelow( item1 ) );

  //try using a good item
  QCOMPARE( layout.itemsModel()->findItemBelow( item3 ), item2 );
  QCOMPARE( layout.itemsModel()->findItemBelow( item2 ), item1 );

  delete label;
}

void TestQgsLayoutModel::setItemRemoved()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  item1->setId( QStringLiteral( "i1" ) );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  item2->setId( QStringLiteral( "i2" ) );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  item3->setId( QStringLiteral( "i3" ) );

  layout.itemsModel()->clear();
  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 3 );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //try marking some bad items as removed
  layout.itemsModel()->setItemRemoved( nullptr );
  QgsLayoutItemMap *label = new QgsLayoutItemMap( nullptr );
  layout.itemsModel()->setItemRemoved( label );
  QVERIFY( !label->scene() );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i3" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 3, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //try using a good item
  layout.itemsModel()->setItemRemoved( item3 );
  QVERIFY( !item3->scene() );
  QVERIFY( !layout.items().contains( item3 ) );

  //item should still be in z-list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item3 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 2 ), item1 );

  //but not in scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item1 );

  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  delete label;
  delete item3;
}

void TestQgsLayoutModel::rebuildZListWithRemoved()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  item1->setId( QStringLiteral( "i1" ) );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  item2->setId( QStringLiteral( "i2" ) );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  item3->setId( QStringLiteral( "i3" ) );

  layout.itemsModel()->setItemRemoved( item3 );

  //start with an empty model
  layout.itemsModel()->clear();

  //rebuild z list
  layout.itemsModel()->rebuildZList();

  //check that only items in the scene are shown by the model
  QCOMPARE( layout.itemsModel()->rowCount(), 3 );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 0, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QString() );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 1, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i2" ) );
  QCOMPARE( layout.itemsModel()->data( layout.itemsModel()->index( 2, 2, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "i1" ) );

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item1 );

  //check that scene list is missing removed item
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item1 );
  delete item3;
}

void TestQgsLayoutModel::reorderUpWithRemoved()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );

  delete item3;

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item1 );

  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item1 );

  //try reorder a good item, should succeed
  QVERIFY( layout.itemsModel()->reorderItemUp( item1 ) );

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item1 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item1 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item2 );
}

void TestQgsLayoutModel::reorderDownWithRemoved()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );

  delete item3;

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item1 );

  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item1 );

  //try reorder a good item, should succeed
  QVERIFY( layout.itemsModel()->reorderItemDown( item2 ) );

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item1 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item1 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item2 );
}

void TestQgsLayoutModel::reorderToTopWithRemoved()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );

  delete item3;

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item1 );

  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item1 );

  //try reorder a good item, should succeed
  QVERIFY( layout.itemsModel()->reorderItemToTop( item1 ) );

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item1 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item1 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item2 );
}

void TestQgsLayoutModel::reorderToBottomWithRemoved()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );
  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );

  delete item3;

  // start with an empty model
  layout.itemsModel()->clear();

  layout.itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item1 );

  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item1 );

  //try reorder a good item, should succeed
  QVERIFY( layout.itemsModel()->reorderItemToBottom( item2 ) );

  //check z list
  QCOMPARE( layout.itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 0 ), item1 );
  QCOMPARE( layout.itemsModel()->zOrderList().at( 1 ), item2 );
  //also check scene list
  QCOMPARE( layout.itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 0 ), item1 );
  QCOMPARE( layout.itemsModel()->mItemsInScene.at( 1 ), item2 );
}

void TestQgsLayoutModel::groupSelection()
{
  QgsLayout layout( QgsProject::instance() );

  //some items in layout
  QgsLayoutItem *item1 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item1 );
  QgsLayoutItem *item2 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item2 );

  QgsLayoutItemGroup *group = new QgsLayoutItemGroup( &layout );
  group->addItem( item1 );
  group->addItem( item2 );
  layout.addLayoutItem( group );

  QgsLayoutItem *item3 = new QgsLayoutItemMap( &layout );
  layout.addLayoutItem( item3 );
  QgsLayoutItemGroup *group2 = new QgsLayoutItemGroup( &layout );
  group2->addItem( group );
  group2->addItem( item3 );
  layout.addLayoutItem( group2 );

  // selecting an item in a group should actually select the topmost parent group
  const QSignalSpy spy( &layout, &QgsLayout::selectedItemChanged );
  layout.itemsModel()->setSelected( layout.itemsModel()->indexForItem( item3 ) );
  QVERIFY( !item3->isSelected() );
  QVERIFY( group2->isSelected() );
  QCOMPARE( spy.count(), 1 );
  // but the actual selected item signal should be the originally selected item, so
  // that it can be tweaked in the properties dialog
  QCOMPARE( spy.at( 0 ).at( 0 ).value< QObject * >(), item3 );

  layout.itemsModel()->setSelected( layout.itemsModel()->indexForItem( item1 ) );
  QVERIFY( !item1->isSelected() );
  QVERIFY( !group->isSelected() );
  QVERIFY( group2->isSelected() );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).value< QObject * >(), item1 );

  layout.itemsModel()->setSelected( layout.itemsModel()->indexForItem( item2 ) );
  QVERIFY( !item2->isSelected() );
  QVERIFY( !group->isSelected() );
  QVERIFY( group2->isSelected() );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.at( 2 ).at( 0 ).value< QObject * >(), item2 );
}

void TestQgsLayoutModel::proxy()
{
  QgsLayout *layout = new QgsLayout( QgsProject::instance() );
  QgsLayoutProxyModel *proxy = new QgsLayoutProxyModel( layout );
  // add some items to composition
  QgsLayoutItemMap *item1 = new QgsLayoutItemMap( layout );
  item1->setId( QStringLiteral( "c" ) );
  layout->addLayoutItem( item1 );
  QgsLayoutItemMap *item2 = new QgsLayoutItemMap( layout );
  item2->setId( QStringLiteral( "b" ) );
  layout->addLayoutItem( item2 );
  QgsLayoutItemLabel *item3 = new QgsLayoutItemLabel( layout );
  item3->setId( QStringLiteral( "a" ) );
  layout->addLayoutItem( item3 );
  QgsLayoutItemShape *item4 = new QgsLayoutItemShape( layout );
  item4->setId( QStringLiteral( "d" ) );
  layout->addLayoutItem( item4 );
  QgsLayoutItemShape *item5 = new QgsLayoutItemShape( layout );
  item5->setId( QStringLiteral( "e" ) );
  layout->addLayoutItem( item5 );
  QCOMPARE( proxy->rowCount( QModelIndex() ), 5 );
  QCOMPARE( proxy->data( proxy->index( 0, 2, QModelIndex() ) ).toString(), QStringLiteral( "a" ) );
  QCOMPARE( proxy->data( proxy->index( 1, 2, QModelIndex() ) ).toString(), QStringLiteral( "b" ) );
  QCOMPARE( proxy->data( proxy->index( 2, 2, QModelIndex() ) ).toString(), QStringLiteral( "c" ) );
  QCOMPARE( proxy->data( proxy->index( 3, 2, QModelIndex() ) ).toString(), QStringLiteral( "d" ) );
  QCOMPARE( proxy->data( proxy->index( 4, 2, QModelIndex() ) ).toString(), QStringLiteral( "e" ) );

  proxy->setAllowEmptyItem( true );
  QCOMPARE( proxy->rowCount( QModelIndex() ), 6 );
  QCOMPARE( proxy->data( proxy->index( 0, 2, QModelIndex() ) ).toString(), QString() );
  QCOMPARE( proxy->data( proxy->index( 1, 2, QModelIndex() ) ).toString(), QStringLiteral( "a" ) );
  QCOMPARE( proxy->data( proxy->index( 2, 2, QModelIndex() ) ).toString(), QStringLiteral( "b" ) );
  QCOMPARE( proxy->data( proxy->index( 3, 2, QModelIndex() ) ).toString(), QStringLiteral( "c" ) );
  QCOMPARE( proxy->data( proxy->index( 4, 2, QModelIndex() ) ).toString(), QStringLiteral( "d" ) );
  QCOMPARE( proxy->data( proxy->index( 5, 2, QModelIndex() ) ).toString(), QStringLiteral( "e" ) );

  proxy->setFilterType( QgsLayoutItemRegistry::LayoutMap );
  QCOMPARE( proxy->rowCount( QModelIndex() ), 3 );
  QCOMPARE( proxy->data( proxy->index( 0, 2, QModelIndex() ) ).toString(), QString() );
  QCOMPARE( proxy->data( proxy->index( 1, 2, QModelIndex() ) ).toString(), QStringLiteral( "b" ) );
  QCOMPARE( proxy->data( proxy->index( 2, 2, QModelIndex() ) ).toString(), QStringLiteral( "c" ) );

  proxy->setFilterType( QgsLayoutItemRegistry::LayoutLabel );
  QCOMPARE( proxy->rowCount( QModelIndex() ), 2 );
  QCOMPARE( proxy->data( proxy->index( 0, 2, QModelIndex() ) ).toString(), QString() );
  QCOMPARE( proxy->data( proxy->index( 1, 2, QModelIndex() ) ).toString(), QStringLiteral( "a" ) );

  proxy->setFilterType( QgsLayoutItemRegistry::LayoutScaleBar );
  QCOMPARE( proxy->rowCount( QModelIndex() ), 1 );
  QCOMPARE( proxy->data( proxy->index( 0, 2, QModelIndex() ) ).toString(), QString() );

  proxy->setAllowEmptyItem( false );
  proxy->setFilterType( QgsLayoutItemRegistry::LayoutItem );
  QCOMPARE( proxy->rowCount( QModelIndex() ), 5 );
  proxy->setItemFlags( QgsLayoutItem::FlagProvidesClipPath );
  QCOMPARE( proxy->rowCount( QModelIndex() ), 2 );
  QCOMPARE( proxy->data( proxy->index( 0, 2, QModelIndex() ) ).toString(), QStringLiteral( "d" ) );
  QCOMPARE( proxy->data( proxy->index( 1, 2, QModelIndex() ) ).toString(), QStringLiteral( "e" ) );
  proxy->setItemFlags( QgsLayoutItem::Flags() );
  QCOMPARE( proxy->rowCount( QModelIndex() ), 5 );
}

void TestQgsLayoutModel::proxyCrash()
{
  // test for a possible crash when using QgsComposerProxyModel and reordering items
  QgsLayout *layout = new QgsLayout( QgsProject::instance() );

  // create a proxy - it's not used, but will be watching...
  QgsLayoutProxyModel *proxy = new QgsLayoutProxyModel( layout );
  Q_UNUSED( proxy );

  // add some items to composition
  QgsLayoutItemMap *item1 = new QgsLayoutItemMap( layout );
  layout->addLayoutItem( item1 );
  QgsLayoutItemMap *item2 = new QgsLayoutItemMap( layout );
  layout->addLayoutItem( item2 );
  QgsLayoutItemMap *item3 = new QgsLayoutItemMap( layout );
  layout->addLayoutItem( item3 );

  // reorder items - expect no crash!
  ( void )layout->itemsModel()->reorderItemUp( item1 );
}

QGSTEST_MAIN( TestQgsLayoutModel )
#include "testqgslayoutmodel.moc"
