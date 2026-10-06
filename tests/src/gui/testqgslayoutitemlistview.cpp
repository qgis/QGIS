/***************************************************************************
    testqgslayoutitemlistview.cpp
     --------------------
    Date                 : October 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayout.h"
#include "qgslayoutitem.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutitemslistview.h"
#include "qgslayoutmodel.h"
#include "qgslayoutview.h"
#include "qgslayoutviewrubberband.h"
#include "qgslayoutviewtool.h"
#include "qgsproject.h"
#include "qgstest.h"

#include <QString>
#include <QtTest/QSignalSpy>

using namespace Qt::StringLiterals;

class TestQgsLayoutItemListView : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testSelectionHandling();
    void testGroupSelectionHandling();

  private:
};

void TestQgsLayoutItemListView::initTestCase()
{}

void TestQgsLayoutItemListView::cleanupTestCase()
{}

void TestQgsLayoutItemListView::init()
{}

void TestQgsLayoutItemListView::cleanup()
{}

void TestQgsLayoutItemListView::testSelectionHandling()
{
  QgsProject p;
  QgsLayout layout( &p );

  QgsLayoutView layoutView;
  layoutView.setCurrentLayout( &layout );

  QgsLayoutItemsListView view( nullptr, nullptr );
  view.setCurrentLayout( &layout );
  view.setLayoutView( &layoutView );

  QgsLayoutItemsListViewModel *proxyModel = view.mModel;

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &layout );
  layout.addLayoutItem( label );

  QgsLayoutItemLabel *label2 = new QgsLayoutItemLabel( &layout );
  layout.addLayoutItem( label2 );

  // set selection from view
  view.selectionModel()->select( proxyModel->indexForItem( label ), QItemSelectionModel::SelectionFlag::ClearAndSelect );
  QVERIFY( label->isSelected() );
  QVERIFY( !label2->isSelected() );
  QCOMPARE( layout.selectedItems().size(), 1 );
  QCOMPARE( layout.selectedItems().at( 0 ), label );
  // reselect same
  view.selectionModel()->select( proxyModel->indexForItem( label ), QItemSelectionModel::SelectionFlag::ClearAndSelect );
  QVERIFY( label->isSelected() );
  QVERIFY( !label2->isSelected() );
  QCOMPARE( layout.selectedItems().size(), 1 );
  QCOMPARE( layout.selectedItems().at( 0 ), label );

  // select different
  view.selectionModel()->select( proxyModel->indexForItem( label2 ), QItemSelectionModel::SelectionFlag::ClearAndSelect );
  QVERIFY( !label->isSelected() );
  QVERIFY( label2->isSelected() );
  QCOMPARE( layout.selectedItems().size(), 1 );
  QCOMPARE( layout.selectedItems().at( 0 ), label2 );

  // emulate selection from the layout
  layout.setSelectedItem( label );
  QCOMPARE( view.selectedIndexes().size(), proxyModel->columnCount() );
  QCOMPARE( proxyModel->itemFromIndex( view.selectedIndexes().at( 0 ) ), label );

  layout.setSelectedItem( label );
  QCOMPARE( view.selectedIndexes().size(), proxyModel->columnCount() );
  QCOMPARE( proxyModel->itemFromIndex( view.selectedIndexes().at( 0 ) ), label );

  layout.setSelectedItem( label2 );
  QCOMPARE( view.selectedIndexes().size(), proxyModel->columnCount() );
  QCOMPARE( proxyModel->itemFromIndex( view.selectedIndexes().at( 0 ) ), label2 );
}

void TestQgsLayoutItemListView::testGroupSelectionHandling()
{
  QgsProject p;
  QgsLayout layout( &p );

  QgsLayoutView layoutView;
  layoutView.setCurrentLayout( &layout );

  QgsLayoutItemsListView view( nullptr, nullptr );
  view.setCurrentLayout( &layout );
  view.setLayoutView( &layoutView );

  QgsLayoutItemsListViewModel *proxyModel = view.mModel;

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &layout );
  layout.addLayoutItem( label );

  QgsLayoutItemLabel *label2 = new QgsLayoutItemLabel( &layout );
  layout.addLayoutItem( label2 );

  // set selection from view
  view.selectionModel()->select( proxyModel->indexForItem( label ), QItemSelectionModel::SelectionFlag::ClearAndSelect );
  QVERIFY( label->isSelected() );
  QVERIFY( !label2->isSelected() );
  QCOMPARE( layout.selectedItems().size(), 1 );
  QCOMPARE( layout.selectedItems().at( 0 ), label );

  layoutView.selectAll();
  layoutView.groupSelectedItems();

  QList< QgsLayoutItemGroup * > groups;
  layout.layoutItems( groups );
  QCOMPARE( groups.size(), 1 );
  QCOMPARE( groups.at( 0 )->items().size(), 2 );
  QVERIFY( groups.at( 0 )->items().contains( label ) );
  QVERIFY( groups.at( 0 )->items().contains( label2 ) );

  QVERIFY( groups.at( 0 )->isSelected() );

  layout.deselectAll();
  QVERIFY( !groups.at( 0 )->isSelected() );

  view.selectionModel()->select( proxyModel->indexForItem( label ), QItemSelectionModel::SelectionFlag::ClearAndSelect );
  QVERIFY( groups.at( 0 )->isSelected() );
  QCOMPARE( layout.selectedItems().size(), 1 );
  QCOMPARE( layout.selectedItems().at( 0 ), groups.at( 0 ) );

  layout.deselectAll();

  // selecting a group member should select the group
  view.selectionModel()->select( proxyModel->indexForItem( label ), QItemSelectionModel::SelectionFlag::ClearAndSelect );
  QVERIFY( groups.at( 0 )->isSelected() );
  QCOMPARE( layout.selectedItems().size(), 1 );
  QCOMPARE( layout.selectedItems().at( 0 ), groups.at( 0 ) );

  layout.deselectAll();

  view.selectionModel()->select( proxyModel->indexForItem( label2 ), QItemSelectionModel::SelectionFlag::ClearAndSelect );
  QVERIFY( groups.at( 0 )->isSelected() );
  QCOMPARE( layout.selectedItems().size(), 1 );
  QCOMPARE( layout.selectedItems().at( 0 ), groups.at( 0 ) );
}

QGSTEST_MAIN( TestQgsLayoutItemListView )
#include "testqgslayoutitemlistview.moc"
