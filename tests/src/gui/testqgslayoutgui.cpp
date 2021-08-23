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




#include <QtTest/QTest>

#include "qgsmapsettings.h"
#include "qgslayout.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutmodel.h"
#include "qgslayoutitemcombobox.h"

#include <QApplication>
#include <QMainWindow>
#include <QtTest/QSignalSpy>


class TestQgsLayoutGui: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void itemTypeComboBox();
    void testProxyCrash();

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
  QCOMPARE( cb->itemText( 0 ), QStringLiteral( "item 1" ) );
  QCOMPARE( cb->item( 0 ), item1 );
  QCOMPARE( cb->currentItem(), item1 );
  QVERIFY( !cb2->currentItem() );

  int expectedSpy1Count = 2;// ideally only one, but we'll settle for 2
  int expectedSpy2Count = 0;
  QCOMPARE( spy1.count(), 2 );
  QCOMPARE( qvariant_cast<QObject *>( spy1.at( 1 ).at( 0 ) ), item1 );
  QCOMPARE( spy2.count(), 0 );

  QgsLayoutItemShape *item2 = new QgsLayoutItemShape( layout );
  item2->setId( "item 2" );
  layout->addLayoutItem( item2 );

  QCOMPARE( cb->count(), 2 );
  QCOMPARE( cb2->count(), 1 );
  QCOMPARE( cb->itemText( 0 ), QStringLiteral( "item 1" ) );
  QCOMPARE( cb->itemText( 1 ), QStringLiteral( "item 2" ) );
  QCOMPARE( cb2->itemText( 0 ), QStringLiteral( "item 2" ) );
  QCOMPARE( cb->item( 0 ), item1 );
  QCOMPARE( cb->item( 1 ), item2 );
  QCOMPARE( cb2->item( 0 ), item2 );
  QCOMPARE( cb->currentItem(), item1 );
  QCOMPARE( cb2->currentItem(), item2 );

  QCOMPARE( spy1.count(), expectedSpy1Count ); // must be unchanged from earlier
  expectedSpy2Count = 2;// ideally only one, but we'll settle for 2
  QCOMPARE( spy2.count(), expectedSpy2Count );
  QCOMPARE( qvariant_cast<QObject *>( spy2.at( 1 ).at( 0 ) ), item2 );

  QgsLayoutItemMap *item3 = new QgsLayoutItemMap( layout );
  item3->setId( "item 3" );
  layout->addLayoutItem( item3 );

  QCOMPARE( cb->count(), 3 );
  QCOMPARE( cb2->count(), 1 );
  QCOMPARE( cb->itemText( 0 ), QStringLiteral( "item 1" ) );
  QCOMPARE( cb->itemText( 1 ), QStringLiteral( "item 2" ) );
  QCOMPARE( cb->itemText( 2 ), QStringLiteral( "item 3" ) );
  QCOMPARE( cb2->itemText( 0 ), QStringLiteral( "item 2" ) );
  QCOMPARE( cb->item( 0 ), item1 );
  QCOMPARE( cb->item( 1 ), item2 );
  QCOMPARE( cb->item( 2 ), item3 );
  QCOMPARE( cb2->item( 0 ), item2 );
  QCOMPARE( cb->currentItem(), item1 );
  QCOMPARE( cb2->currentItem(), item2 );
  QCOMPARE( spy1.count(), expectedSpy1Count ); // must be unchanged from earlier
  QCOMPARE( spy2.count(), expectedSpy2Count );

  item3->setId( "a" );
  QCOMPARE( cb->itemText( 0 ), QStringLiteral( "a" ) );
  QCOMPARE( cb->itemText( 1 ), QStringLiteral( "item 1" ) );
  QCOMPARE( cb->itemText( 2 ), QStringLiteral( "item 2" ) );
  item3->setId( "item 3" );
  QCOMPARE( cb->itemText( 0 ), QStringLiteral( "item 1" ) );
  QCOMPARE( cb->itemText( 1 ), QStringLiteral( "item 2" ) );
  QCOMPARE( cb->itemText( 2 ), QStringLiteral( "item 3" ) );

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


QTEST_MAIN( TestQgsLayoutGui )
#include "testqgslayoutgui.moc"
