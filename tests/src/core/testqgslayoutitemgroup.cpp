/***************************************************************************
                         testqgslayoutitemgroup.cpp
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

#include "qgslayoutitemgroup.h"
#include "qgslayout.h"
#include "qgslayoutitemshape.h"
#include "qgscomposerlabel.h"
#include "qgscomposerpolygon.h"
#include "qgscomposition.h"
#include "qgsmultirenderchecker.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsproject.h"

#include <QObject>
#include <QtTest/QSignalSpy>
#include "qgstest.h"

class TestQgsLayoutItemGroup : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutItemGroup() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void createGroupDirect();
    void createGroup(); //test grouping items
    void ungroup(); //test ungrouping items
    void deleteGroup(); //test deleting group works
    void undoRedo(); //test that group/ungroup undo/redo commands don't crash

  private:

    void dumpUndoStack( const QUndoStack &, QString prefix = QString() ) const;

};

// private
void TestQgsLayoutItemGroup::dumpUndoStack( const QUndoStack &us, QString prefix ) const
{
  if ( ! prefix.isEmpty() ) prefix += QLatin1String( ": " );
  for ( int i = 0; i < us.count(); ++i )
  {
    QgsDebugMsg( QString( "%4US %1: %2%3" )
                 .arg( i ). arg( i >= us.index() ? "-" : "",
                                 us.text( i ), prefix ) );
  }
}

void TestQgsLayoutItemGroup::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsLayoutItemGroup::cleanupTestCase()
{
}

void TestQgsLayoutItemGroup::init()
{

}

void TestQgsLayoutItemGroup::cleanup()
{

}

void TestQgsLayoutItemGroup::createGroupDirect()
{
  // create group manually
  QgsProject proj;
  QgsLayout l( &proj );

  QgsLayoutItemRectangularShape *item = new QgsLayoutItemRectangularShape( &l );
  l.addLayoutItem( item );
  QgsLayoutItemRectangularShape *item2 = new QgsLayoutItemRectangularShape( &l );
  l.addLayoutItem( item2 );
  QVERIFY( !item->parentGroup() );
  QVERIFY( !item->isGroupMember() );
  QVERIFY( !item2->parentGroup() );
  QVERIFY( !item2->isGroupMember() );

  QgsLayoutItemGroup *group = new QgsLayoutItemGroup( &l );
  l.addLayoutItem( group );
  QVERIFY( group->items().empty() );

  // add null item
  group->addItem( nullptr );
  QVERIFY( group->items().empty() );

  group->addItem( item );
  QCOMPARE( item->parentGroup(), group );
  QVERIFY( item->isGroupMember() );
  QVERIFY( !item2->parentGroup() );
  QVERIFY( !item2->isGroupMember() );
  QCOMPARE( group->items().count(), 1 );
  QVERIFY( group->items().contains( item ) );
  QVERIFY( l.items().contains( item ) );
  QVERIFY( l.items().contains( item2 ) );

  group->addItem( item2 );
  QCOMPARE( item->parentGroup(), group );
  QVERIFY( item->isGroupMember() );
  QCOMPARE( item2->parentGroup(), group );
  QVERIFY( item2->isGroupMember() );
  QCOMPARE( group->items().count(), 2 );
  QVERIFY( group->items().contains( item ) );
  QVERIFY( group->items().contains( item2 ) );
  QVERIFY( l.items().contains( item ) );
  QVERIFY( l.items().contains( item2 ) );

  // manually delete an item
  QPointer< QgsLayoutItemRectangularShape > pItem( item ); // for testing deletion
  l.removeLayoutItem( item );
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !pItem );

  QCOMPARE( item2->parentGroup(), group );
  QVERIFY( item2->isGroupMember() );
  QCOMPARE( group->items().count(), 1 );
  QVERIFY( group->items().contains( item2 ) );

  QPointer< QgsLayoutItemRectangularShape > pItem2( item2 ); // for testing deletion
  // remove items
  group->removeItems();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  // should still exist
  QCOMPARE( pItem2.data(), item2 );
  QVERIFY( !item2->isGroupMember() );
  QVERIFY( !item2->parentGroup() );
  QCOMPARE( item2->layout(), &l );
  QVERIFY( l.items().contains( item2 ) );
}

void TestQgsLayoutItemGroup::createGroup()
{
  QgsProject proj;
  QgsLayout l( &proj );

  QgsLayoutItemRectangularShape *item = new QgsLayoutItemRectangularShape( &l );
  QgsLayoutItemRectangularShape *item2 = new QgsLayoutItemRectangularShape( &l );

  //group items
  QList<QgsLayoutItem *> items;
  items << item << item2;
  QgsLayoutItemGroup *group = l.groupItems( items );

  //check result
  QVERIFY( group );
  QCOMPARE( group->layout(), &l );
  QCOMPARE( group->items().size(), 2 );
  QVERIFY( group->items().contains( item ) );
  QVERIFY( group->items().contains( item2 ) );
  QVERIFY( item->isGroupMember() );
  QCOMPARE( item->parentGroup(), group );
  QCOMPARE( item2->parentGroup(), group );
}

void TestQgsLayoutItemGroup::ungroup()
{
  //test ungrouping items

  QgsProject proj;
  QgsLayout l( &proj );

  //simple tests - check that we don't crash
  l.ungroupItems( nullptr ); //no item

  QgsLayoutItemRectangularShape *item = new QgsLayoutItemRectangularShape( &l );
  l.addLayoutItem( item );
  QgsLayoutItemRectangularShape *item2 = new QgsLayoutItemRectangularShape( &l );
  l.addLayoutItem( item2 );

  //group items
  QList<QgsLayoutItem *> items;
  items << item << item2;
  QgsLayoutItemGroup *group = l.groupItems( items );

  // group should be in scene
  QList<QgsLayoutItemGroup *> groups;
  l.layoutItems( groups );
  QCOMPARE( groups.size(), 1 );
  QVERIFY( groups.contains( group ) );
  QCOMPARE( group->layout(), &l );

  QPointer< QgsLayoutItemGroup > pGroup( group ); // for testing deletion
  //ungroup group
  QList<QgsLayoutItem *> ungroupedItems;
  ungroupedItems = l.ungroupItems( group );

  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !pGroup );

  QCOMPARE( ungroupedItems.size(), 2 );
  QVERIFY( ungroupedItems.contains( item ) );
  QVERIFY( ungroupedItems.contains( item2 ) );

  QVERIFY( !item->isGroupMember() );
  QVERIFY( !item2->isGroupMember() );
  QVERIFY( !item->parentGroup() );
  QVERIFY( !item2->parentGroup() );

  // items should still be in scene
  QCOMPARE( item->layout(), &l );
  QVERIFY( l.items().contains( item ) );
  QCOMPARE( item2->layout(), &l );
  QVERIFY( l.items().contains( item2 ) );

  //should also be no groups left in the composition

  l.layoutItems( groups );
  QCOMPARE( groups.size(), 0 );
}

void TestQgsLayoutItemGroup::deleteGroup()
{
  QgsProject proj;
  QgsLayout l( &proj );

  QgsLayoutItemRectangularShape *item = new QgsLayoutItemRectangularShape( &l );
  QPointer< QgsLayoutItemRectangularShape > pItem( item ); // for testing deletion
  l.addLayoutItem( item );
  QgsLayoutItemRectangularShape *item2 = new QgsLayoutItemRectangularShape( &l );
  QPointer< QgsLayoutItemRectangularShape > pItem2( item2 ); // for testing deletion
  l.addLayoutItem( item2 );

  //group items
  QList<QgsLayoutItem *> groupItems;
  groupItems << item << item2;
  QgsLayoutItemGroup *group = l.groupItems( groupItems );
  QPointer< QgsLayoutItemGroup > pGroup( group ); // for testing deletion

  QList<QgsLayoutItem *> items;
  l.layoutItems( items );
  QCOMPARE( items.size(), 3 );

  //test that deleting group also removes all grouped items
  l.removeLayoutItem( group );

  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !pGroup );
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !pItem );
  QVERIFY( !pItem2 );

  l.layoutItems( items );
  QVERIFY( items.empty() );
}

Q_DECLARE_METATYPE( QgsLayoutItemGroup * )
Q_DECLARE_METATYPE( QgsComposerPolygon * )
Q_DECLARE_METATYPE( QgsLayoutItem * )

void TestQgsLayoutItemGroup::undoRedo()
{
#if 0
  QgsComposerPolygon *item1, *item2;
  int polygonsAdded = 0;
  int groupsAdded = 0;
  int itemsRemoved = 0;

  qRegisterMetaType<QgsComposerPolygon *>();
  QSignalSpy spyPolygonAdded( mComposition, &QgsComposition::itemAdded );
  QCOMPARE( spyPolygonAdded.count(), 0 );

  qRegisterMetaType<QgsComposerItemGroup *>();
  QSignalSpy spyGroupAdded( mComposition, &QgsComposition::composerItemGroupAdded );
  QCOMPARE( spyGroupAdded.count(), 0 );

  qRegisterMetaType<QgsComposerItem *>();
  QSignalSpy spyItemRemoved( mComposition, &QgsComposition::itemRemoved );
  QCOMPARE( spyItemRemoved.count(), 0 );

  //test for crash when undo/redoing with groups
  // Set initial condition
  QUndoStack *us = mComposition->undoStack();
  QgsDebugMsg( QString( "clearing" ) );
  us->clear();
  QgsDebugMsg( QString( "clearing completed" ) );
  QList<QgsComposerItem *> items;
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 1 ); // paper only
  QgsDebugMsg( QString( "clear stack count:%1 index:%2" ) .arg( us->count() ) .arg( us->index() ) );

  //create some items
  item1 = new QgsComposerPolygon( QPolygonF( QRectF( 0, 0, 1, 1 ) ), mComposition ); //QgsComposerLabel( mComposition );
  mComposition->addComposerPolygon( item1 );
  QCOMPARE( spyPolygonAdded.count(), ++polygonsAdded );
  item2 = new QgsComposerPolygon( QPolygonF( QRectF( -1, -2, 1, 1 ) ), mComposition ); //QgsComposerLabel( mComposition );
  mComposition->addComposerPolygon( item2 );
  QCOMPARE( spyPolygonAdded.count(), ++polygonsAdded );
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 3 ); // paper, 2 shapes
  QgsDebugMsg( QString( "addedItems stack count:%1 index:%2" ) .arg( us->count() ) .arg( us->index() ) );
  QCOMPARE( item1->pos(), QPointF( 0, 0 ) );
  QCOMPARE( item2->pos(), QPointF( -1, -2 ) );
  //dumpUndoStack(*us, "after initial items addition");

  //group items
  items.clear();
  items << item1 << item2;
  mGroup = mComposition->groupItems( items );
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), ++groupsAdded );
  QCOMPARE( spyItemRemoved.count(), itemsRemoved );
  QCOMPARE( mGroup->items().size(), 2 );
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 4 ); // paper, 2 shapes, 1 group
  QVERIFY( ! item1->isRemoved() );
  QCOMPARE( item1->pos(), QPointF( 0, 0 ) );
  QVERIFY( ! item2->isRemoved() );
  QCOMPARE( item2->pos(), QPointF( -1, -2 ) );
  QVERIFY( ! mGroup->isRemoved() );
  QCOMPARE( mGroup->pos(), QPointF( -1, -2 ) );
  //dumpUndoStack(*us, "after initial items addition");

  //move group
  QgsDebugMsg( QString( "moving group" ) );
  mGroup->beginCommand( QStringLiteral( "move group" ) );
  mGroup->move( 10.0, 20.0 );
  mGroup->endCommand();
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), groupsAdded );
  QCOMPARE( spyItemRemoved.count(), itemsRemoved );
  QgsDebugMsg( QString( "groupItems stack count:%1 index:%2" ) .arg( us->count() ) .arg( us->index() ) );
  QCOMPARE( mGroup->items().size(), 2 );
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 4 ); // paper, 2 shapes, 1 group
  QVERIFY( ! item1->isRemoved() );
  QCOMPARE( item1->pos(), QPointF( 10, 20 ) );
  QVERIFY( ! item2->isRemoved() );
  QCOMPARE( item2->pos(), QPointF( 9, 18 ) );
  QVERIFY( ! mGroup->isRemoved() );
  QCOMPARE( mGroup->pos(), QPointF( 9, 18 ) );

  //ungroup
  QgsDebugMsg( QString( "ungrouping" ) );
  mComposition->ungroupItems( mGroup );
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), groupsAdded );
  QCOMPARE( spyItemRemoved.count(), ++itemsRemoved );
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 3 ); // paper, 2 shapes
  QVERIFY( ! item1->isRemoved() );
  QVERIFY( ! item2->isRemoved() );
  QVERIFY( mGroup->isRemoved() );
  QCOMPARE( mGroup->pos(), QPointF( 9, 18 ) ); // should not rely on this
  //dumpUndoStack(*us, "after ungroup");
  // US 0: Items grouped
  // US 1: move group
  // US 2: Remove item group

  //undo (groups again) -- crashed here before #11371 got fixed
  QgsDebugMsg( QString( "undo ungrouping" ) );
  us->undo();
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), ++groupsAdded );
  QCOMPARE( spyItemRemoved.count(), itemsRemoved );
  QCOMPARE( mGroup->items().size(), 2 ); // WARNING: might not be alive anymore
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 4 ); // paper, 2 shapes, 1 group
  QVERIFY( ! item1->isRemoved() );
  QCOMPARE( item1->pos(), QPointF( 10, 20 ) );
  QVERIFY( ! item2->isRemoved() );
  QCOMPARE( item2->pos(), QPointF( 9, 18 ) );
  QVERIFY( ! mGroup->isRemoved() );
  QCOMPARE( mGroup->pos(), QPointF( 9, 18 ) );
  //dumpUndoStack(*us, "after undo ungroup");
  // US 0: Items grouped
  // US 1: move group
  // US 2: -Remove item group

  //remove group
  QgsDebugMsg( QString( "remove group" ) );
  mComposition->removeComposerItem( mGroup, true, true );
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), groupsAdded );
  itemsRemoved += 3; // the group and the two items
  QCOMPARE( spyItemRemoved.count(), itemsRemoved );
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 1 ); // paper only
  QgsDebugMsg( QString( "remove stack count:%1 index:%2" ) .arg( us->count() ) .arg( us->index() ) );
  //dumpUndoStack(*us, "after remove group");
  // US 0: Items grouped
  // US 1: move group
  // US 2: Remove item group

  //undo remove group
  QgsDebugMsg( QString( "undo remove group" ) );
  us->undo();
  polygonsAdded += 2;
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), ++groupsAdded );
  QCOMPARE( spyItemRemoved.count(), itemsRemoved );
  mComposition->composerItems( items );
  QCOMPARE( mGroup->items().size(), 2 );
  QCOMPARE( items.size(), 4 ); // paper, 2 shapes, 1 group
  QgsDebugMsg( QString( "undo stack count:%1 index:%2" ) .arg( us->count() ) .arg( us->index() ) );
  //dumpUndoStack(*us, "after undo remove group");
  // US 0: Items grouped
  // US 1: move group
  // US 2: -Remove item group

  //undo move group
  QgsDebugMsg( QString( "undo move group" ) );
  us->undo();
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), groupsAdded );
  QCOMPARE( spyItemRemoved.count(), itemsRemoved );
  QCOMPARE( mGroup->items().size(), 2 );
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 4 ); // paper, 2 shapes, 1 group
  QCOMPARE( item1->isGroupMember(), true );
  QCOMPARE( item2->isGroupMember(), true );
  QVERIFY( ! item1->isRemoved() );
  QCOMPARE( item1->pos(), QPointF( 0, 0 ) );
  QVERIFY( ! item2->isRemoved() );
  QCOMPARE( item2->pos(), QPointF( -1, -2 ) );
  QVERIFY( ! mGroup->isRemoved() );
  QCOMPARE( mGroup->pos(), QPointF( -1, -2 ) );
  //dumpUndoStack(*us, "after undo move group");
  // US 0: Items grouped
  // US 1: -move group
  // US 2: -Remove item group

  //undo group
  QgsDebugMsg( QString( "undo group" ) );
  us->undo();
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), groupsAdded );
  QCOMPARE( spyItemRemoved.count(), ++itemsRemoved );
  //QCOMPARE( mGroup->items().size(), 2 ); // not important
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 3 ); // paper, 2 shapes
  QCOMPARE( item1->isGroupMember(), false );
  QCOMPARE( item2->isGroupMember(), false );
  QVERIFY( ! item1->isRemoved() );
  QCOMPARE( item1->pos(), QPointF( 0, 0 ) );
  QVERIFY( ! item2->isRemoved() );
  QCOMPARE( item2->pos(), QPointF( -1, -2 ) );
  QVERIFY( mGroup->isRemoved() );
  //QCOMPARE( mGroup->pos(), QPointF( -1, -2 ) );
  //dumpUndoStack(*us, "after undo group");
  // US 0: -Items grouped
  // US 1: -move group
  // US 2: -Remove item group

  //redo group
  QgsDebugMsg( QString( "redo group" ) );
  us->redo();
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), ++groupsAdded );
  QCOMPARE( spyItemRemoved.count(), itemsRemoved );
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 4 ); // paper, 2 shapes, 1 group
  QCOMPARE( item1->isGroupMember(), true );
  QCOMPARE( item2->isGroupMember(), true );
  //// QCOMPARE( mGroup->pos(), QPointF( 0, 0 ) ); // getting nan,nan here
  //dumpUndoStack(*us, "after redo group");
  // US 0: Items grouped
  // US 1: -move group
  // US 2: -Remove item group

  //redo move group
  QgsDebugMsg( QString( "redo move group" ) );
  us->redo();
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), groupsAdded );
  QCOMPARE( spyItemRemoved.count(), itemsRemoved );
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 4 ); // paper, 2 shapes, 1 group
  QCOMPARE( item1->isGroupMember(), true );
  QCOMPARE( item2->isGroupMember(), true );
  QCOMPARE( mGroup->pos(), QPointF( 9, 18 ) );
  //dumpUndoStack(*us, "after redo move group");
  // US 0: Items grouped
  // US 1: move group
  // US 2: -Remove item group

  //redo remove group
  QgsDebugMsg( QString( "redo remove group" ) );
  us->redo();
  QCOMPARE( spyPolygonAdded.count(), polygonsAdded );
  QCOMPARE( spyGroupAdded.count(), groupsAdded );
  itemsRemoved += 3; // 1 group, 2 contained items
  QCOMPARE( spyItemRemoved.count(), itemsRemoved );
  mComposition->composerItems( items );
  QCOMPARE( items.size(), 1 ); // paper only
  QgsDebugMsg( QString( "undo stack count:%1 index:%2" ) .arg( us->count() ) .arg( us->index() ) );
  //dumpUndoStack(*us, "after redo remove group");
  // US 0: Items grouped
  // US 1: move group
  // US 2: Remove item group

  //unwind the whole stack
  us->clear();

  QgsDebugMsg( QString( "clear stack count:%1 index:%2" ) .arg( us->count() ) .arg( us->index() ) );
#endif
}

QGSTEST_MAIN( TestQgsLayoutItemGroup )
#include "testqgslayoutitemgroup.moc"
