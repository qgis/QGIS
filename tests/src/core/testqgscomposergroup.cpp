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
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include <QObject>
#include <QtTest>

class TestQgsComposerGroup: public QObject
{
    Q_OBJECT;
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
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QgsComposerLabel* mItem1;
    QgsComposerLabel* mItem2;
    QString mReport;
};

void TestQgsComposerGroup::initTestCase()
{
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  //create some items
  mItem1 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem1 );
  mItem2 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem2 );

  mReport = "<h1>Composer Grouped Item Tests</h1>\n";
}

void TestQgsComposerGroup::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
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
  QgsComposerItemGroup* group = mComposition->groupItems( items );

  //check result
  QVERIFY( group );
  QCOMPARE( group->items().size(), 2 );
  QVERIFY( group->items().contains( mItem1 ) );
  QVERIFY( group->items().contains( mItem2 ) );
  QVERIFY( mItem1->isGroupMember() );
  QVERIFY( mItem2->isGroupMember() );
}

void TestQgsComposerGroup::ungroup()
{

}

void TestQgsComposerGroup::deleteGroup()
{

}

void TestQgsComposerGroup::undoRedo()
{

}

QTEST_MAIN( TestQgsComposerGroup )
#include "moc_testqgscomposergroup.cxx"
