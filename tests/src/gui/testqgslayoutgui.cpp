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




#include <QtTest/QtTest>

#include "qgsmapsettings.h"
#include "qgslayout.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutmodel.h"
#include "qgslayoutitemcombobox.h"

#include <QApplication>
#include <QMainWindow>

class TestQgsLayoutGui: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

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

void TestQgsLayoutGui::testProxyCrash()
{
  // test for a possible crash when using QgsLayoutProxyModel and reordering items
  QgsLayout *layout = new QgsLayout( QgsProject::instance() );

  // create a composer item combobox
  QgsLayoutItemComboBox *cb = new QgsLayoutItemComboBox( nullptr, layout );
  QgsLayoutItemComboBox *cb2 = new QgsLayoutItemComboBox( nullptr, layout );
  QgsLayoutItemComboBox *cb3 = new QgsLayoutItemComboBox( nullptr, layout );
  QgsLayoutItemComboBox *cb4 = new QgsLayoutItemComboBox( nullptr, layout );

  // add some items to composition
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
