/***************************************************************************
                         testqgscomposergroup.cpp
                         -----------------------
    begin                : October 2014
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

#include "qgscomposeritemgroup.h"
#include "qgscomposerlabel.h"
#include "qgscomposerpolygon.h"
#include "qgscomposition.h"
#include "qgsmultirenderchecker.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <QObject>
#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>

class TestQgsComposerGroup : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerGroup()
        : mComposition( 0 )
        , mMapSettings( 0 )
        , mItem1( 0 )
        , mItem2( 0 )
        , mGroup( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void createGroup(); //test grouping items
    void ungroup(); //test ungrouping items
    void deleteGroup(); //test deleting group works
    void undoRedo(); //test that group/ungroup undo/redo commands don't crash

  private:

    void dumpUndoStack( const QUndoStack&, QString prefix = "" ) const;

    QgsComposition *mComposition;
    QgsMapSettings *mMapSettings;
    QgsComposerLabel* mItem1;
    QgsComposerLabel* mItem2;
    QgsComposerItemGroup* mGroup;
    QString mReport;
};

// private
void TestQgsComposerGroup::dumpUndoStack( const QUndoStack& us, QString prefix ) const
{
  if ( ! prefix.isEmpty() ) prefix += ": ";
  for ( int i = 0; i < us.count(); ++i )
  {
    QgsDebugMsg( QString( "%4US %1: %2%3" )
                 .arg( i ). arg( i >= us.index() ? "-" : "" )
                 .arg( us.text( i ) ) .arg( prefix ) );
  }
}

void TestQgsComposerGroup::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mMapSettings = new QgsMapSettings();
  mComposition = new QgsComposition( *mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  //create some items
  mItem1 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem1 );
  mItem2 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem2 );

  mGroup = 0;

  mReport = "<h1>Composer Grouped Item Tests</h1>\n";
}

void TestQgsComposerGroup::cleanupTestCase()
{
  delete mComposition;
  delete mMapSettings;
  QgsApplication::exitQgis();

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsComposerGroup::init()
{

}

void TestQgsComposerGroup::cleanup()
{

}

void TestQgsComposerGroup::createGroup()
{
  //group items
  QList<QgsComposerItem*> items;
  items << mItem1 << mItem2;
  mGroup = mComposition->groupItems( items );

  //check result
  QVERIFY( mGroup );
  QCOMPARE( mGroup->items().size(), 2 );
  QVERIFY( mGroup->items().contains( mItem1 ) );
  QVERIFY( mGroup->items().contains( mItem2 ) );
  QVERIFY( mItem1->isGroupMember() );
  QVERIFY( mItem2->isGroupMember() );
}

void TestQgsComposerGroup::ungroup()
{
  //test ungrouping items

  //simple tests - check that we don't crash
  mComposition->ungroupItems( 0 ); //no item

  //ungroup mGroup
  QList<QgsComposerItem*> ungroupedItems;
  ungroupedItems = mComposition->ungroupItems( mGroup );

  QCOMPARE( ungroupedItems.size(), 2 );
  QVERIFY( ungroupedItems.contains( mItem1 ) );
  QVERIFY( ungroupedItems.contains( mItem2 ) );

  QVERIFY( !mItem1->isGroupMember() );
  QVERIFY( !mItem2->isGroupMember() );

  //should also be no groups left in the composition
  QList<QgsComposerItemGroup*> groups;
  mComposition->composerItems( groups );
  QCOMPARE( groups.size(), 0 );
}

void TestQgsComposerGroup::deleteGroup()
{
  //group items
  QList<QgsComposerItem*> groupItems;
  groupItems << mItem1 << mItem2;
  mGroup = mComposition->groupItems( groupItems );
  QList<QgsComposerItem*> items;
  mComposition->composerItems( items );
  //expect initially 4 items, as paper counts as an item
  QCOMPARE( items.size(), 4 );

  //test that deleting group also removes all grouped items
  mComposition->removeComposerItem( mGroup );
  mComposition->composerItems( items );

  //expect a single item (paper item)
  QCOMPARE( items.size(), 1 );
  QVERIFY( mItem1->isRemoved() );
  QVERIFY( mItem2->isRemoved() );
  QVERIFY( mGroup->isRemoved() );
}

Q_DECLARE_METATYPE( QgsComposerItemGroup * );
Q_DECLARE_METATYPE( QgsComposerPolygon * );
Q_DECLARE_METATYPE( QgsComposerItem * );

void TestQgsComposerGroup::undoRedo()
{
  QgsComposerPolygon *item1, *item2;
  int polygonsAdded = 0;
  int groupsAdded = 0;
  int itemsRemoved = 0;

  qRegisterMetaType<QgsComposerPolygon *>();
  QSignalSpy spyPolygonAdded( mComposition, SIGNAL( composerPolygonAdded( QgsComposerPolygon* ) ) );
  QCOMPARE( spyPolygonAdded.count(), 0 );

  qRegisterMetaType<QgsComposerItemGroup *>();
  QSignalSpy spyGroupAdded( mComposition, SIGNAL( composerItemGroupAdded( QgsComposerItemGroup* ) ) );
  QCOMPARE( spyGroupAdded.count(), 0 );

  qRegisterMetaType<QgsComposerItem *>();
  QSignalSpy spyItemRemoved( mComposition, SIGNAL( itemRemoved( QgsComposerItem* ) ) );
  QCOMPARE( spyItemRemoved.count(), 0 );

  //test for crash when undo/redoing with groups
  // Set initial condition
  QUndoStack *us = mComposition->undoStack();
  QgsDebugMsg( QString( "clearing" ) );
  us->clear();
  QgsDebugMsg( QString( "clearing completed" ) );
  QList<QgsComposerItem*> items;
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
  mGroup->beginCommand( "move group" );
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
}

QTEST_MAIN( TestQgsComposerGroup )
#include "testqgscomposergroup.moc"
