/***************************************************************************
    testqgscomposergui.cpp
     ----------------------
    Date                 : May 2017
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
#include "qgscomposition.h"
#include "qgscomposerlabel.h"
#include "qgscomposermap.h"
#include "qgscomposermodel.h"
#include "qgscomposeritemcombobox.h"

#include <QApplication>
#include <QMainWindow>

class TestQgsComposerGui: public QObject
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

void TestQgsComposerGui::initTestCase()
{
}

void TestQgsComposerGui::cleanupTestCase()
{
}

void TestQgsComposerGui::init()
{
}

void TestQgsComposerGui::cleanup()
{
}

void TestQgsComposerGui::testProxyCrash()
{
  // test for a possible crash when using QgsComposerProxyModel and reordering items
  QgsMapSettings ms;
  QgsComposition* composition = new QgsComposition( ms );

  // create a composer item combobox
  QgsComposerItemComboBox* cb = new QgsComposerItemComboBox( nullptr, composition );
  QgsComposerItemComboBox* cb2 = new QgsComposerItemComboBox( nullptr, composition );
  QgsComposerItemComboBox* cb3 = new QgsComposerItemComboBox( nullptr, composition );
  QgsComposerItemComboBox* cb4 = new QgsComposerItemComboBox( nullptr, composition );

  cb->show();
  cb2->show();

  // add some items to composition
  QgsComposerMap* item1 = new QgsComposerMap( composition );
  composition->addItem( item1 );
  QgsComposerLabel* item2 = new QgsComposerLabel( composition );
  composition->addItem( item2 );
  QgsComposerMap* item3 = new QgsComposerMap( composition );
  composition->addItem( item3 );

  QCOMPARE( cb->count(), 3 );
  cb->setItemType( QgsComposerItem::ComposerMap );
  QCOMPARE( cb->count(), 2 );

  cb4->setItemType( QgsComposerItem::ComposerLabel );

  cb->setItem( item1 );
  QCOMPARE( cb->currentItem(), item1 );
  cb2->setItem( item3 );
  QCOMPARE( cb2->currentItem(), item3 );
  cb3->setItem( item2 );
  QCOMPARE( cb3->currentItem(), item2 );

  // reorder items - expect no crash!
  // we do this by calling the private members, in order to simulate what
  // happens when a drag and drop reorder occurs
  composition->itemsModel()->mItemZList.removeOne( item1 );
  composition->itemsModel()->mItemZList.insert( 1, item1 );
  composition->itemsModel()->rebuildSceneItemList();

  QCOMPARE( cb->currentItem(), item1 );
  QCOMPARE( cb2->currentItem(), item3 );

  composition->itemsModel()->mItemZList.removeOne( item1 );
  composition->itemsModel()->mItemZList.insert( 0, item1 );
  composition->itemsModel()->rebuildSceneItemList();

  delete composition;
}


QTEST_MAIN( TestQgsComposerGui )
#include "testqgscomposergui.moc"
