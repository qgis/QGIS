/***************************************************************************
                         testqgscomposermodel.cpp
                         -----------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgscomposition.h"
#include "qgscomposermodel.h"
#include "qgscomposerlabel.h"
#include <QObject>
#include <QtTest/QtTest>
#include <QList>

class TestQgsComposerModel : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerModel()
        : mComposition( 0 )
        , mItem1( 0 )
        , mItem2( 0 )
        , mItem3( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void creation(); //check that model has been created
    void addItems(); //add some items to the composition and test model
    void zList(); //check model z order list
    void clear(); //check clearing the model
    void addItemDirectly(); //add an item directly to the model
    void rebuildZList(); //test rebuilding the z list from the current composer stacking
    void removeItem(); //test removing an item from the model
    void reorderUp(); //test reordering an item up
    void reorderDown(); //test reordering an item down
    void reorderTop(); //test reordering an item to top
    void reorderBottom(); //test reordering an item to bottom
    void getComposerItemAbove(); //test getting composer item above
    void getComposerItemBelow(); //test getting composer item below
    void setItemRemoved(); //test setting an item as removed
    void rebuildZListWithRemoved(); //test rebuilding z list with removed items
    void reorderUpWithRemoved(); //test reordering up with removed items
    void reorderDownWithRemoved(); //test reordering down with removed items
    void reorderToTopWithRemoved(); //test reordering to top with removed items
    void reorderToBottomWithRemoved(); //test reordering to bottom with removed items

  private:
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QgsComposerLabel* mItem1;
    QgsComposerLabel* mItem2;
    QgsComposerLabel* mItem3;
};

void TestQgsComposerModel::initTestCase()
{
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
}

void TestQgsComposerModel::cleanupTestCase()
{
  delete mComposition;
}

void TestQgsComposerModel::init()
{

}

void TestQgsComposerModel::cleanup()
{

}

void TestQgsComposerModel::creation()
{
  QVERIFY( mComposition->itemsModel() );
  //check some basic things
  QCOMPARE( mComposition->itemsModel()->columnCount(), 3 );
  QCOMPARE( mComposition->itemsModel()->rowCount(), 0 );
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 0 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 0 );
}

void TestQgsComposerModel::addItems()
{
  //add some items to the composition
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 0 );
  mItem1 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem1 );
  mItem2 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem2 );
  mItem3 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem3 );
  //check that these items have been added to the model
  QCOMPARE( mComposition->itemsModel()->rowCount(), 3 );
  //and the scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 3 );
}

void TestQgsComposerModel::zList()
{
  //check z list for items added by TestQgsComposerModel::addItems()
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );
  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 2 ), mItem1 );
}

void TestQgsComposerModel::clear()
{
  mComposition->itemsModel()->clear();
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 0 );
  QCOMPARE( mComposition->itemsModel()->rowCount(), 0 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 0 );
}

void TestQgsComposerModel::addItemDirectly()
{
  //create some items not attached to the composition
  QgsComposerLabel* bottomItem = new QgsComposerLabel( 0 );
  QgsComposerLabel* topItem = new QgsComposerLabel( 0 );

  mComposition->itemsModel()->clear();

  mComposition->itemsModel()->addItemAtTop( bottomItem );
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 1 );
  QCOMPARE( mComposition->itemsModel()->rowCount(), 1 );

  mComposition->itemsModel()->addItemAtTop( topItem );
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 2 );
  QCOMPARE( mComposition->itemsModel()->rowCount(), 2 );

  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), topItem );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), bottomItem );

  //also check scene list (these items are treated by the model as belonging to the scene,
  //as they will have isRemoved() as false
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), topItem );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), bottomItem );

  mComposition->itemsModel()->clear();
  delete bottomItem;
  delete topItem;
}

void TestQgsComposerModel::rebuildZList()
{
  //start with an empty model
  mComposition->itemsModel()->clear();

  //some items are in composition, added by TestQgsComposerModel::addItems
  mComposition->itemsModel()->rebuildZList();

  //check that these items have been added to the model
  QCOMPARE( mComposition->itemsModel()->rowCount(), 3 );
  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 2 ), mItem1 );
}

void TestQgsComposerModel::removeItem()
{
  //start with an empty model
  mComposition->itemsModel()->clear();

  QgsComposerLabel* item1 = new QgsComposerLabel( 0 );
  QgsComposerLabel* item2 = new QgsComposerLabel( 0 );

  //add one item to the model
  mComposition->itemsModel()->addItemAtTop( item1 );
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 1 );
  QCOMPARE( mComposition->itemsModel()->rowCount(), 1 );

  //also check scene list (this item is treated by the model as belonging to the scene,
  //as it has isRemoved() as false
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 1 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), item1 );

  //try removing a missing item
  mComposition->itemsModel()->removeItem( 0 );
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 1 );
  QCOMPARE( mComposition->itemsModel()->rowCount(), 1 );

  //try removing an item not in the model
  mComposition->itemsModel()->removeItem( item2 );
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 1 );
  QCOMPARE( mComposition->itemsModel()->rowCount(), 1 );

  //remove the item which is in the model
  mComposition->itemsModel()->removeItem( item1 );
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 0 );
  QCOMPARE( mComposition->itemsModel()->rowCount(), 0 );
  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 0 );
}

void TestQgsComposerModel::reorderUp()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  //try reordering some bad items
  QVERIFY( ! mComposition->itemsModel()->reorderItemUp( 0 ) );
  QgsComposerLabel* label = new QgsComposerLabel( 0 );
  QVERIFY( ! mComposition->itemsModel()->reorderItemUp( label ) );

  //trying to reorder up the topmost item should fail
  QVERIFY( ! mComposition->itemsModel()->reorderItemUp( mItem3 ) );

  //try reorder a good item, should succeed
  QVERIFY( mComposition->itemsModel()->reorderItemUp( mItem2 ) );

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );
  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 2 ), mItem1 );

  delete label;
}

void TestQgsComposerModel::reorderDown()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  //try reordering some bad items
  QVERIFY( ! mComposition->itemsModel()->reorderItemDown( 0 ) );
  QgsComposerLabel* label = new QgsComposerLabel( 0 );
  QVERIFY( ! mComposition->itemsModel()->reorderItemDown( label ) );

  //trying to reorder down the bottommost item should fail
  QVERIFY( ! mComposition->itemsModel()->reorderItemDown( mItem1 ) );

  //try reorder a good item, should succeed
  QVERIFY( mComposition->itemsModel()->reorderItemDown( mItem2 ) );

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem2 );
  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 2 ), mItem2 );
  delete label;
}

void TestQgsComposerModel::reorderTop()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  //try reordering some bad items
  QVERIFY( ! mComposition->itemsModel()->reorderItemToTop( 0 ) );
  QgsComposerLabel* label = new QgsComposerLabel( 0 );
  QVERIFY( ! mComposition->itemsModel()->reorderItemToTop( label ) );

  //trying to reorder up the topmost item should fail
  QVERIFY( ! mComposition->itemsModel()->reorderItemToTop( mItem3 ) );

  //try reorder a good item, should succeed
  QVERIFY( mComposition->itemsModel()->reorderItemToTop( mItem1 ) );

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem2 );
  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 2 ), mItem2 );
  delete label;
}

void TestQgsComposerModel::reorderBottom()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  //try reordering some bad items
  QVERIFY( ! mComposition->itemsModel()->reorderItemToBottom( 0 ) );
  QgsComposerLabel* label = new QgsComposerLabel( 0 );
  QVERIFY( ! mComposition->itemsModel()->reorderItemToBottom( label ) );

  //trying to reorder down the bottommost item should fail
  QVERIFY( ! mComposition->itemsModel()->reorderItemToBottom( mItem1 ) );

  //try reorder a good item, should succeed
  QVERIFY( mComposition->itemsModel()->reorderItemToBottom( mItem3 ) );

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem3 );
  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 3 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 2 ), mItem3 );
  delete label;
}

void TestQgsComposerModel::getComposerItemAbove()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  //try getting item above some bad items
  QVERIFY( ! mComposition->itemsModel()->getComposerItemAbove( 0 ) );
  QgsComposerLabel* label = new QgsComposerLabel( 0 );
  QVERIFY( ! mComposition->itemsModel()->getComposerItemAbove( label ) );

  //trying to get item above topmost item should fail
  QVERIFY( ! mComposition->itemsModel()->getComposerItemAbove( mItem3 ) );

  //try using a good item
  QCOMPARE( mComposition->itemsModel()->getComposerItemAbove( mItem2 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->getComposerItemAbove( mItem1 ), mItem2 );

  delete label;
}

void TestQgsComposerModel::getComposerItemBelow()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  //try getting item below some bad items
  QVERIFY( ! mComposition->itemsModel()->getComposerItemBelow( 0 ) );
  QgsComposerLabel* label = new QgsComposerLabel( 0 );
  QVERIFY( ! mComposition->itemsModel()->getComposerItemBelow( label ) );

  //trying to get item below bottom most item should fail
  QVERIFY( ! mComposition->itemsModel()->getComposerItemBelow( mItem1 ) );

  //try using a good item
  QCOMPARE( mComposition->itemsModel()->getComposerItemBelow( mItem3 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->getComposerItemBelow( mItem2 ), mItem1 );

  delete label;
}

void TestQgsComposerModel::setItemRemoved()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 3 );

  //try marking some bad items as removed
  mComposition->itemsModel()->setItemRemoved( 0 );
  QgsComposerLabel* label = new QgsComposerLabel( 0 );
  mComposition->itemsModel()->setItemRemoved( label );
  QVERIFY( !label->isRemoved() );

  //try using a good item
  mComposition->itemsModel()->setItemRemoved( mItem3 );
  QVERIFY( mItem3->isRemoved() );

  //item should still be in z-list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  //but not in scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem1 );
}

void TestQgsComposerModel::rebuildZListWithRemoved()
{
  QVERIFY( mItem3->isRemoved() );

  //start with an empty model
  mComposition->itemsModel()->clear();

  //rebuild z list
  mComposition->itemsModel()->rebuildZList();

  //check that only items in the scene are shown by the model
  QCOMPARE( mComposition->itemsModel()->rowCount(), 2 );

  //check z list contains ALL items
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  //check that scene list is missing removed item
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem1 );
}

void TestQgsComposerModel::reorderUpWithRemoved()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem1 );

  //try reorder a good item, should succeed
  QVERIFY( mComposition->itemsModel()->reorderItemUp( mItem1 ) );

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem2 );
  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem2 );
}

void TestQgsComposerModel::reorderDownWithRemoved()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem1 );

  //try reorder a good item, should succeed
  QVERIFY( mComposition->itemsModel()->reorderItemDown( mItem2 ) );

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem2 );
  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem2 );
}

void TestQgsComposerModel::reorderToTopWithRemoved()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem1 );

  //try reorder a good item, should succeed
  QVERIFY( mComposition->itemsModel()->reorderItemToTop( mItem1 ) );

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem2 );
  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem2 );
}

void TestQgsComposerModel::reorderToBottomWithRemoved()
{
  mComposition->itemsModel()->clear();
  mComposition->itemsModel()->rebuildZList();

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem1 );

  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem1 );

  //try reorder a good item, should succeed
  QVERIFY( mComposition->itemsModel()->reorderItemToBottom( mItem2 ) );

  //check z list
  QCOMPARE( mComposition->itemsModel()->zOrderListSize(), 3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 0 ), mItem3 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 1 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->zOrderList()->at( 2 ), mItem2 );
  //also check scene list
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.size(), 2 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 0 ), mItem1 );
  QCOMPARE( mComposition->itemsModel()->mItemsInScene.at( 1 ), mItem2 );
}

QTEST_MAIN( TestQgsComposerModel )
#include "testqgscomposermodel.moc"
