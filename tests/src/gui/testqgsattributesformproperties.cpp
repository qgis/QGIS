/***************************************************************************
    testqgsattributesformproperties.cpp
     --------------------------------------
    Date                 : 2025-01-21
    Copyright            : (C) 2025 Mathieu Pellerin
    Email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributesformmodel.h"
#include "qgsattributesformproperties.h"
#include "qgsattributesformview.h"
#include "qgsattributetypedialog.h"
#include "qgseditformconfig.h"
#include "qgseditorwidgetregistry.h"
#include "qgsgui.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QItemSelectionModel>
#include <QMimeData>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsAttributesFormProperties : public QObject
{
    Q_OBJECT
  public:
    TestQgsAttributesFormProperties() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testConfigStored();
    void testRemoveDuplicateField();
    void testDropMultipleItems();
    void testMoveContainerIntoContainer();
    void testMoveTab();
};

void TestQgsAttributesFormProperties::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsAttributesFormProperties::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAttributesFormProperties::init()
{}

void TestQgsAttributesFormProperties::cleanup()
{}

void TestQgsAttributesFormProperties::testConfigStored()
{
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?field=col0:integer&field=col1:integer"_s, u"test"_s, u"memory"_s );
  QgsAttributesFormProperties attributeFormProperties( layer );
  attributeFormProperties.init();

  // Get the fields
  QVERIFY( attributeFormProperties.mAvailableWidgetsView );
  const QgsAttributesAvailableWidgetsModel *availableWidgetsModel = static_cast<QgsAttributesAvailableWidgetsView *>( attributeFormProperties.mAvailableWidgetsView )->availableWidgetsModel();
  QVERIFY( availableWidgetsModel );
  const QModelIndex fieldContainer = availableWidgetsModel->fieldContainer();

  QVERIFY( fieldContainer.isValid() );
  const QString name = fieldContainer.data( QgsAttributesFormModel::ItemNameRole ).toString();
  QCOMPARE( name, u"Fields"_s );
  QCOMPARE( availableWidgetsModel->rowCount( fieldContainer ), 2 );

  // Insure that the configuration was stored when switching from one available widgets tree item to another
  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 0, 0, fieldContainer ) ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  attributeFormProperties.mAttributeTypeDialog->setAlias( u"alias0"_s );
  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 1, 0, fieldContainer ) ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  attributeFormProperties.mAttributeTypeDialog->setAlias( u"alias1"_s );

  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 0, 0, fieldContainer ) ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  QCOMPARE( attributeFormProperties.mAttributeTypeDialog->alias(), u"alias0"_s );
  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 1, 0, fieldContainer ) ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  QCOMPARE( attributeFormProperties.mAttributeTypeDialog->alias(), u"alias1"_s );
}

void TestQgsAttributesFormProperties::testRemoveDuplicateField()
{
  /*
    - removing a copy of a duplicated field must remove exactly that copy

    - dropping a duplicate at or above the currently selected row makes
      QItemSelectionModel emit a no-op selectionChanged() signal from within
      rowsAboutToBeInserted(). If the selection-changed handlers react to it
      by writing to the source model, the proxy model mapping gets corrupted:
      it gains a phantom row and all subsequent proxy indexes are shifted,
      leading to wrong items being removed and "QSortFilterProxyModel:
      inconsistent changes reported by source model" warnings

    - the drop-then-remove flow must also work when appending below the
      selection and when the selection is changed before removing.
  */

  auto layer = std::make_unique< QgsVectorLayer >( u"Point?field=a:integer&field=b:integer"_s, u"test"_s, u"memory"_s );

  // tab
  //  ├─ b   (row 0)
  //  ├─ a   (row 1)
  //  ├─ a   (row 2)
  //  └─ a   (row 3)
  QgsAttributeEditorContainer *tab = new QgsAttributeEditorContainer( u"tab"_s, nullptr );
  tab->addChildElement( new QgsAttributeEditorField( u"b"_s, 1, tab ) );
  tab->addChildElement( new QgsAttributeEditorField( u"a"_s, 0, tab ) );
  tab->addChildElement( new QgsAttributeEditorField( u"a"_s, 0, tab ) );
  tab->addChildElement( new QgsAttributeEditorField( u"a"_s, 0, tab ) );

  QgsEditFormConfig cfg = layer->editFormConfig();
  cfg.setLayout( Qgis::AttributeFormLayout::DragAndDrop );
  cfg.clearTabs();
  cfg.addTab( tab );
  layer->setEditFormConfig( cfg );

  QgsAttributesFormProperties attributeFormProperties( layer.get() );
  attributeFormProperties.init();

  // Mutating the model from selection-changed handlers while rows are being
  // inserted or removed corrupts the proxy model mapping
  QTest::failOnWarning( QRegularExpression( u".*inconsistent changes.*"_s ) );

  QgsAttributesFormProxyModel *layoutProxy = attributeFormProperties.mFormLayoutProxyModel;
  QgsAttributesFormLayoutModel *layoutModel = attributeFormProperties.mFormLayoutModel;
  const QModelIndex tabProxyIndex = layoutProxy->index( 0, 0, QModelIndex() );
  const QModelIndex tabSourceIndex = layoutModel->index( 0, 0, QModelIndex() );
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 4 );

  // Select the middle duplicate (row 2) in the form layout view, then trigger
  // the same slot the trash button is connected to. Exactly that copy must be
  // removed.
  attributeFormProperties.mFormLayoutView->selectionModel()->setCurrentIndex( layoutProxy->index( 2, 0, tabProxyIndex ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  QCOMPARE( attributeFormProperties.mFormLayoutView->selectionModel()->selectedRows( 0 ).count(), 1 );
  attributeFormProperties.removeTabOrGroupButton();
  QCOMPARE( layoutModel->rowCount( tabSourceIndex ), 3 );
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 3 );

  // Remove the remaining duplicate the same way, leaving [b, a]
  attributeFormProperties.mFormLayoutView->selectionModel()->setCurrentIndex( layoutProxy->index( 2, 0, tabProxyIndex ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  attributeFormProperties.removeTabOrGroupButton();
  QCOMPARE( layoutModel->rowCount( tabSourceIndex ), 2 );
  QCOMPARE( layoutModel->index( 0, 0, tabSourceIndex ).data( QgsAttributesFormModel::ItemNameRole ).toString(), u"b"_s );
  QCOMPARE( layoutModel->index( 1, 0, tabSourceIndex ).data( QgsAttributesFormModel::ItemNameRole ).toString(), u"a"_s );

  // Select field "a" in the available widgets view, like a user would do before
  // dragging it. This auto-selects the matching layout item (row 1) and loads
  // the attribute type dialog.
  const QgsAttributesAvailableWidgetsModel *availableWidgetsModel = static_cast<QgsAttributesAvailableWidgetsView *>( attributeFormProperties.mAvailableWidgetsView )->availableWidgetsModel();
  const QModelIndex fieldContainer = availableWidgetsModel->fieldContainer();
  const QModelIndex fieldAProxyIndex = attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 0, 0, fieldContainer ) );
  QCOMPARE( fieldAProxyIndex.data( QgsAttributesFormModel::ItemNameRole ).toString(), u"a"_s );
  attributeFormProperties.mAvailableWidgetsView->selectionModel()->setCurrentIndex( fieldAProxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  QCOMPARE( attributeFormProperties.mFormLayoutView->selectionModel()->selectedRows( 0 ).count(), 1 );

  // Drop a duplicate of field "a" at row 0, i.e. above the selected row
  QMimeData *mimeData = attributeFormProperties.mAvailableWidgetsProxyModel->mimeData( QModelIndexList() << fieldAProxyIndex );
  QVERIFY( layoutProxy->dropMimeData( mimeData, Qt::CopyAction, 0, 0, tabProxyIndex ) );
  delete mimeData;

  // The proxy model must stay in sync with the source model
  QCOMPARE( layoutModel->rowCount( tabSourceIndex ), 3 );
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 3 );
  for ( int row = 0; row < 3; row++ )
  {
    QCOMPARE( layoutProxy->mapToSource( layoutProxy->index( row, 0, tabProxyIndex ) ).row(), row );
  }

  // The freshly dropped copy (row 0) gets auto-selected; the trash button must
  // remove exactly that copy
  QCOMPARE( attributeFormProperties.mFormLayoutView->selectionModel()->selectedRows( 0 ).count(), 1 );
  attributeFormProperties.removeTabOrGroupButton();
  QCOMPARE( layoutModel->rowCount( tabSourceIndex ), 2 );
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 2 );
  QCOMPARE( layoutModel->index( 0, 0, tabSourceIndex ).data( QgsAttributesFormModel::ItemNameRole ).toString(), u"b"_s );
  QCOMPARE( layoutModel->index( 1, 0, tabSourceIndex ).data( QgsAttributesFormModel::ItemNameRole ).toString(), u"a"_s );

  // Drop the duplicate once more, this time appending it below the selection;
  // the fresh copy is auto-selected and must be the one removed
  mimeData = attributeFormProperties.mAvailableWidgetsProxyModel->mimeData( QModelIndexList() << fieldAProxyIndex );
  QVERIFY( layoutProxy->dropMimeData( mimeData, Qt::CopyAction, -1, -1, tabProxyIndex ) );
  delete mimeData;
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 3 );
  QCOMPARE( attributeFormProperties.mFormLayoutView->selectionModel()->selectedRows( 0 ).count(), 1 );
  attributeFormProperties.removeTabOrGroupButton();
  QCOMPARE( layoutModel->rowCount( tabSourceIndex ), 2 );
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 2 );

  // Drop it yet again, but this time change the selection to the first copy
  // before removing. Again, exactly one copy must be removed.
  mimeData = attributeFormProperties.mAvailableWidgetsProxyModel->mimeData( QModelIndexList() << fieldAProxyIndex );
  QVERIFY( layoutProxy->dropMimeData( mimeData, Qt::CopyAction, -1, -1, tabProxyIndex ) );
  delete mimeData;
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 3 );

  attributeFormProperties.mFormLayoutView->selectionModel()->setCurrentIndex( layoutProxy->index( 1, 0, tabProxyIndex ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  attributeFormProperties.removeTabOrGroupButton();
  QCOMPARE( layoutModel->rowCount( tabSourceIndex ), 2 );
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 2 );
}

void TestQgsAttributesFormProperties::testDropMultipleItems()
{
  // Dropping several items at once onto the form layout tree must leave all
  // dropped items selected, not only the last one. This holds both for
  // internal moves and for items dragged from the available widgets tree.

  auto layer = std::make_unique< QgsVectorLayer >( u"Point?field=a:integer&field=b:integer&field=c:integer&field=d:integer"_s, u"test"_s, u"memory"_s );

  // tab
  //  ├─ a
  //  ├─ b
  //  ├─ c
  //  └─ d
  QgsAttributeEditorContainer *tab = new QgsAttributeEditorContainer( u"tab"_s, nullptr );
  tab->addChildElement( new QgsAttributeEditorField( u"a"_s, 0, tab ) );
  tab->addChildElement( new QgsAttributeEditorField( u"b"_s, 1, tab ) );
  tab->addChildElement( new QgsAttributeEditorField( u"c"_s, 2, tab ) );
  tab->addChildElement( new QgsAttributeEditorField( u"d"_s, 3, tab ) );

  QgsEditFormConfig cfg = layer->editFormConfig();
  cfg.setLayout( Qgis::AttributeFormLayout::DragAndDrop );
  cfg.clearTabs();
  cfg.addTab( tab );
  layer->setEditFormConfig( cfg );

  QgsAttributesFormProperties attributeFormProperties( layer.get() );
  attributeFormProperties.init();

  QgsAttributesFormProxyModel *layoutProxy = attributeFormProperties.mFormLayoutProxyModel;
  QModelIndex tabProxyIndex = layoutProxy->index( 0, 0, QModelIndex() );
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 4 );

  // Returns the sorted rows and the names of the currently selected layout items
  auto selectedRowsAndNames = [&attributeFormProperties]( QList< int > &rows, QStringList &names ) {
    rows.clear();
    names.clear();
    const QModelIndexList selectedIndexes = attributeFormProperties.mFormLayoutView->selectionModel()->selectedRows( 0 );
    for ( const QModelIndex &index : selectedIndexes )
      rows << index.row();
    std::sort( rows.begin(), rows.end() );
    const QModelIndex parent = selectedIndexes.isEmpty() ? QModelIndex() : selectedIndexes.at( 0 ).parent();
    for ( const int row : std::as_const( rows ) )
      names << attributeFormProperties.mFormLayoutView->model()->index( row, 0, parent ).data( QgsAttributesFormModel::ItemNameRole ).toString();
  };

  // Internal move: drag "a" and "b" (rows 0 and 1) past the end of the tab
  QMimeData *mimeData = layoutProxy->mimeData( QModelIndexList() << layoutProxy->index( 0, 0, tabProxyIndex ) << layoutProxy->index( 1, 0, tabProxyIndex ) );
  QVERIFY( mimeData );
  QVERIFY( layoutProxy->dropMimeData( mimeData, Qt::MoveAction, 4, 0, tabProxyIndex ) );
  delete mimeData;
  // The internal move is performed from a queued call (see dropMimeData)
  QCoreApplication::processEvents();
  // The move resets the model, so proxy indexes captured earlier are invalid
  tabProxyIndex = layoutProxy->index( 0, 0, QModelIndex() );

  // Resulting order is [c, d, a, b]; both moved items must remain selected
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 4 );
  QList< int > selectedRows;
  QStringList selectedNames;
  selectedRowsAndNames( selectedRows, selectedNames );
  QCOMPARE( selectedRows, QList< int >() << 2 << 3 );
  QCOMPARE( selectedNames, QStringList() << u"a"_s << u"b"_s );

  // External drop: drag fields "a" and "b" from the available widgets tree
  const QgsAttributesAvailableWidgetsModel *availableWidgetsModel = static_cast<QgsAttributesAvailableWidgetsView *>( attributeFormProperties.mAvailableWidgetsView )->availableWidgetsModel();
  const QModelIndex fieldContainer = availableWidgetsModel->fieldContainer();
  QModelIndexList availableWidgetsIndexes;
  availableWidgetsIndexes
    << attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 0, 0, fieldContainer ) )
    << attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 1, 0, fieldContainer ) );

  mimeData = attributeFormProperties.mAvailableWidgetsProxyModel->mimeData( availableWidgetsIndexes );
  QVERIFY( mimeData );
  QVERIFY( layoutProxy->dropMimeData( mimeData, Qt::CopyAction, -1, -1, tabProxyIndex ) );
  delete mimeData;

  // Both dropped copies (appended at rows 4 and 5) must be selected
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 6 );
  selectedRowsAndNames( selectedRows, selectedNames );
  QCOMPARE( selectedRows, QList< int >() << 4 << 5 );
  QCOMPARE( selectedNames, QStringList() << u"a"_s << u"b"_s );
}

void TestQgsAttributesFormProperties::testMoveContainerIntoContainer()
{
  // Moving a container into another container (through the filter proxy) must
  // relocate it, not lose it. This is exercised with the source container
  // sitting above the destination container under the same parent, so that
  // removing the source shifts the destination's row — a case that a plain
  // (non-persistent) destination parent index would get wrong.

  auto layer = std::make_unique< QgsVectorLayer >( u"Point?field=a:integer&field=b:integer"_s, u"test"_s, u"memory"_s );

  // tab
  //  ├─ groupA
  //  │   └─ a
  //  └─ groupB
  //      └─ b
  QgsAttributeEditorContainer *tab = new QgsAttributeEditorContainer( u"tab"_s, nullptr );
  QgsAttributeEditorContainer *groupA = new QgsAttributeEditorContainer( u"groupA"_s, tab );
  groupA->addChildElement( new QgsAttributeEditorField( u"a"_s, 0, groupA ) );
  tab->addChildElement( groupA );
  QgsAttributeEditorContainer *groupB = new QgsAttributeEditorContainer( u"groupB"_s, tab );
  groupB->addChildElement( new QgsAttributeEditorField( u"b"_s, 1, groupB ) );
  tab->addChildElement( groupB );

  QgsEditFormConfig cfg = layer->editFormConfig();
  cfg.setLayout( Qgis::AttributeFormLayout::DragAndDrop );
  cfg.clearTabs();
  cfg.addTab( tab );
  layer->setEditFormConfig( cfg );

  QgsAttributesFormProperties attributeFormProperties( layer.get() );
  attributeFormProperties.init();

  QgsAttributesFormProxyModel *layoutProxy = attributeFormProperties.mFormLayoutProxyModel;
  QgsAttributesFormLayoutModel *layoutModel = attributeFormProperties.mFormLayoutModel;

  QModelIndex tabProxyIndex = layoutProxy->index( 0, 0, QModelIndex() );
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 2 );
  const QModelIndex groupAProxyIndex = layoutProxy->index( 0, 0, tabProxyIndex );
  const QModelIndex groupBProxyIndex = layoutProxy->index( 1, 0, tabProxyIndex );
  QCOMPARE( groupAProxyIndex.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"groupA"_s );
  QCOMPARE( groupBProxyIndex.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"groupB"_s );

  // Move groupA into groupB (append)
  QMimeData *mimeData = layoutProxy->mimeData( QModelIndexList() << groupAProxyIndex );
  QVERIFY( mimeData );
  QVERIFY( layoutProxy->dropMimeData( mimeData, Qt::MoveAction, -1, -1, groupBProxyIndex ) );
  delete mimeData;
  // The internal move is performed from a queued call (see dropMimeData)
  QCoreApplication::processEvents();
  // The move resets the model, so proxy indexes captured earlier are invalid
  tabProxyIndex = layoutProxy->index( 0, 0, QModelIndex() );

  // The tab now holds only groupB, which in turn holds [b, groupA]; groupA still holds a.
  const QModelIndex tabSourceIndex = layoutModel->index( 0, 0, QModelIndex() );
  QCOMPARE( layoutModel->rowCount( tabSourceIndex ), 1 );
  const QModelIndex movedGroupB = layoutModel->index( 0, 0, tabSourceIndex );
  QCOMPARE( movedGroupB.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"groupB"_s );
  QCOMPARE( layoutModel->rowCount( movedGroupB ), 2 );
  QCOMPARE( layoutModel->index( 0, 0, movedGroupB ).data( QgsAttributesFormModel::ItemIdRole ).toString(), u"b"_s );
  const QModelIndex movedGroupA = layoutModel->index( 1, 0, movedGroupB );
  QCOMPARE( movedGroupA.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"groupA"_s );
  QCOMPARE( layoutModel->rowCount( movedGroupA ), 1 );
  QCOMPARE( layoutModel->index( 0, 0, movedGroupA ).data( QgsAttributesFormModel::ItemIdRole ).toString(), u"a"_s );

  // The proxy must stay consistent with the source model
  QCOMPARE( layoutProxy->rowCount( tabProxyIndex ), 1 );
  const QModelIndex groupBProxyAfter = layoutProxy->index( 0, 0, tabProxyIndex );
  QCOMPARE( groupBProxyAfter.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"groupB"_s );
  QCOMPARE( layoutProxy->rowCount( groupBProxyAfter ), 2 );
  QCOMPARE( layoutProxy->index( 1, 0, groupBProxyAfter ).data( QgsAttributesFormModel::ItemIdRole ).toString(), u"groupA"_s );
}
void TestQgsAttributesFormProperties::testMoveTab()
{
  // Moving a top-level tab container to another position (a same-parent reorder
  // at the root level, with each tab holding children) must not crash the
  // filter proxy and must produce the expected order.

  auto layer = std::make_unique< QgsVectorLayer >( u"Point?field=a:integer&field=b:integer&field=c:integer&field=d:integer"_s, u"test"_s, u"memory"_s );

  // tab1{a}  tab2{b}  tab3{c}  tab4{d}
  auto makeTab = []( const QString &name, const QString &field, int fieldIdx ) {
    QgsAttributeEditorContainer *tab = new QgsAttributeEditorContainer( name, nullptr );
    tab->addChildElement( new QgsAttributeEditorField( field, fieldIdx, tab ) );
    return tab;
  };

  QgsEditFormConfig cfg = layer->editFormConfig();
  cfg.setLayout( Qgis::AttributeFormLayout::DragAndDrop );
  cfg.clearTabs();
  cfg.addTab( makeTab( u"tab1"_s, u"a"_s, 0 ) );
  cfg.addTab( makeTab( u"tab2"_s, u"b"_s, 1 ) );
  cfg.addTab( makeTab( u"tab3"_s, u"c"_s, 2 ) );
  cfg.addTab( makeTab( u"tab4"_s, u"d"_s, 3 ) );
  layer->setEditFormConfig( cfg );

  QgsAttributesFormProperties attributeFormProperties( layer.get() );
  attributeFormProperties.init();

  QgsAttributesFormProxyModel *layoutProxy = attributeFormProperties.mFormLayoutProxyModel;
  QgsAttributesFormLayoutModel *layoutModel = attributeFormProperties.mFormLayoutModel;
  QCOMPARE( layoutProxy->rowCount( QModelIndex() ), 4 );

  // Build the proxy child mappings for every tab (as an expanded view would)
  for ( int row = 0; row < 4; ++row )
  {
    const QModelIndex tabProxy = layoutProxy->index( row, 0, QModelIndex() );
    QCOMPARE( layoutProxy->rowCount( tabProxy ), 1 );
    ( void ) layoutProxy->index( 0, 0, tabProxy );
  }

  // Move tab3 (row 2) to the front (drop at row 0 among the tabs)
  const QModelIndex tab3ProxyIndex = layoutProxy->index( 2, 0, QModelIndex() );
  QCOMPARE( tab3ProxyIndex.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"tab3"_s );
  QMimeData *mimeData = layoutProxy->mimeData( QModelIndexList() << tab3ProxyIndex );
  QVERIFY( mimeData );
  QVERIFY( layoutProxy->dropMimeData( mimeData, Qt::MoveAction, 0, 0, QModelIndex() ) );
  delete mimeData;
  // The internal move is performed from a queued call (see dropMimeData)
  QCoreApplication::processEvents();

  // Order is now tab3, tab1, tab2, tab4 (each keeping its child)
  QCOMPARE( layoutModel->rowCount( QModelIndex() ), 4 );
  const QStringList expected { u"tab3"_s, u"tab1"_s, u"tab2"_s, u"tab4"_s };
  for ( int row = 0; row < 4; ++row )
  {
    const QModelIndex tabSource = layoutModel->index( row, 0, QModelIndex() );
    QCOMPARE( tabSource.data( QgsAttributesFormModel::ItemIdRole ).toString(), expected.at( row ) );
    QCOMPARE( layoutModel->rowCount( tabSource ), 1 );
  }

  // The proxy must agree
  QCOMPARE( layoutProxy->rowCount( QModelIndex() ), 4 );
  for ( int row = 0; row < 4; ++row )
  {
    const QModelIndex tabProxy = layoutProxy->index( row, 0, QModelIndex() );
    QCOMPARE( tabProxy.data( QgsAttributesFormModel::ItemIdRole ).toString(), expected.at( row ) );
    QCOMPARE( layoutProxy->rowCount( tabProxy ), 1 );
  }
}

QGSTEST_MAIN( TestQgsAttributesFormProperties )
#include "testqgsattributesformproperties.moc"
