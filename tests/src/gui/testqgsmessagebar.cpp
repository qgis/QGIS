/***************************************************************************
    testqgsmessagebar.cpp
     --------------------------------------
    Date                 : October 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include <memory>

class TestQgsMessageBar: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void dismiss();
    void pushPop();
    void autoDelete();

};

void TestQgsMessageBar::initTestCase()
{

}

void TestQgsMessageBar::cleanupTestCase()
{
}

void TestQgsMessageBar::init()
{
}

void TestQgsMessageBar::cleanup()
{
}

void TestQgsMessageBar::dismiss()
{
  QgsMessageBar bar;
  bar.show();
  QVERIFY( !bar.currentItem() );

  QgsMessageBarItem *item = new QgsMessageBarItem( QStringLiteral( "test" ) );
  const QPointer< QgsMessageBarItem > pItem( item );
  item->dismiss(); // should do nothing, not in a bar yet
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  for ( int i = 1; i < 100; ++i )
  {
    QApplication::processEvents();
  }
  QCOMPARE( pItem.data(), item );


  bar.pushItem( item );
  QCOMPARE( bar.currentItem(), item );

  item->dismiss();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );

  for ( int i = 1; i < 100; ++i )
  {
    QApplication::processEvents();
  }
  QVERIFY( !bar.currentItem() );
  QVERIFY( !pItem.data() );
}

void TestQgsMessageBar::pushPop()
{
  // test pushing/popping message logic
  QgsMessageBar bar;
  QCOMPARE( bar.items().size(), 0 );
  QVERIFY( !bar.currentItem() );
  bar.pushMessage( QStringLiteral( "1" ) );
  QCOMPARE( bar.items().size(), 1 );
  QCOMPARE( bar.items().at( 0 )->text(), QStringLiteral( "1" ) );
  QCOMPARE( bar.currentItem()->text(), QStringLiteral( "1" ) );
  const QPointer< QgsMessageBarItem > item1 = bar.currentItem();
  // make sure correct item is the visible one
  QCOMPARE( qobject_cast< QgsMessageBarItem * >( qgis::down_cast< QGridLayout * >( bar.layout() )->itemAt( 3 )->widget() )->text(), QStringLiteral( "1" ) );

  bar.pushMessage( QStringLiteral( "2" ) );
  QCOMPARE( bar.items().size(), 2 );
  QCOMPARE( bar.items().at( 0 )->text(), QStringLiteral( "2" ) );
  QCOMPARE( bar.items().at( 1 )->text(), QStringLiteral( "1" ) );
  QCOMPARE( bar.currentItem()->text(), QStringLiteral( "2" ) );
  const QPointer< QgsMessageBarItem > item2 = bar.currentItem();
  QCOMPARE( qobject_cast< QgsMessageBarItem * >( qgis::down_cast< QGridLayout * >( bar.layout() )->itemAt( 3 )->widget() )->text(), QStringLiteral( "2" ) );

  bar.pushMessage( QStringLiteral( "3" ) );
  QCOMPARE( bar.items().size(), 3 );
  QCOMPARE( bar.items().at( 0 )->text(), QStringLiteral( "3" ) );
  QCOMPARE( bar.items().at( 1 )->text(), QStringLiteral( "2" ) );
  QCOMPARE( bar.items().at( 2 )->text(), QStringLiteral( "1" ) );
  QCOMPARE( bar.currentItem()->text(), QStringLiteral( "3" ) );
  const QPointer< QgsMessageBarItem > item3 = bar.currentItem();
  QCOMPARE( qobject_cast< QgsMessageBarItem * >( qgis::down_cast< QGridLayout * >( bar.layout() )->itemAt( 3 )->widget() )->text(), QStringLiteral( "3" ) );

  const int childCount = bar.children().count();
  QVERIFY( bar.popWidget() );
  QCOMPARE( bar.items().size(), 2 );
  QCOMPARE( bar.items().at( 0 )->text(), QStringLiteral( "2" ) );
  QCOMPARE( bar.items().at( 1 )->text(), QStringLiteral( "1" ) );
  QCOMPARE( bar.currentItem()->text(), QStringLiteral( "2" ) );
  QCOMPARE( qobject_cast< QgsMessageBarItem * >( qgis::down_cast< QGridLayout * >( bar.layout() )->itemAt( 3 )->widget() )->text(), QStringLiteral( "2" ) );
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QCOMPARE( bar.children().count(), childCount - 1 );
  QVERIFY( !item3 );

  QVERIFY( bar.popWidget() );
  QCOMPARE( bar.items().size(), 1 );
  QCOMPARE( bar.items().at( 0 )->text(), QStringLiteral( "1" ) );
  QCOMPARE( bar.currentItem()->text(), QStringLiteral( "1" ) );
  QCOMPARE( qobject_cast< QgsMessageBarItem * >( qgis::down_cast< QGridLayout * >( bar.layout() )->itemAt( 3 )->widget() )->text(), QStringLiteral( "1" ) );
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QCOMPARE( bar.children().count(), childCount - 2 );
  QVERIFY( !item2 );

  QVERIFY( bar.popWidget() );
  QCOMPARE( bar.items().size(), 0 );
  QVERIFY( !bar.currentItem() );
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QCOMPARE( bar.children().count(), childCount - 3 );
  QVERIFY( !item1 );

  QVERIFY( !bar.popWidget() );
  QCOMPARE( bar.items().size(), 0 );
  QVERIFY( !bar.currentItem() );
}

void TestQgsMessageBar::autoDelete()
{
  // ensure that items are automatically deleted when queue grows too large
  QgsMessageBar bar;
  for ( int i = 0; i < QgsMessageBar::MAX_ITEMS; ++i )
  {
    bar.pushMessage( QString::number( i ), Qgis::MessageLevel::Warning );
  }
  QCOMPARE( bar.items().size(), 100 );
  QCOMPARE( bar.items().at( 0 )->text(), QStringLiteral( "99" ) );
  QCOMPARE( bar.items().at( 99 )->text(), QStringLiteral( "0" ) );
  const QPointer< QgsMessageBarItem > oldest = bar.items().at( 99 );

  // push one more item, oldest one should be auto-removed
  bar.pushMessage( QStringLiteral( "100" ), Qgis::MessageLevel::Warning );
  QCOMPARE( bar.items().size(), 100 );
  QCOMPARE( bar.items().at( 0 )->text(), QStringLiteral( "100" ) );
  QCOMPARE( bar.items().at( 99 )->text(), QStringLiteral( "1" ) );
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !oldest );

  // but if we have a lower priority message we can pop, then do that instead
  bar.pushMessage( QStringLiteral( "101" ), Qgis::MessageLevel::Info );
  QCOMPARE( bar.items().size(), 100 );
  QCOMPARE( bar.items().at( 0 )->text(), QStringLiteral( "101" ) );
  QCOMPARE( bar.items().at( 1 )->text(), QStringLiteral( "100" ) );
  QCOMPARE( bar.items().at( 99 )->text(), QStringLiteral( "2" ) );
  bar.pushMessage( QStringLiteral( "102" ), Qgis::MessageLevel::Info );
  QCOMPARE( bar.items().size(), 100 );
  QCOMPARE( bar.items().at( 0 )->text(), QStringLiteral( "102" ) );
  QCOMPARE( bar.items().at( 1 )->text(), QStringLiteral( "100" ) );
  QCOMPARE( bar.items().at( 99 )->text(), QStringLiteral( "2" ) );
}

QGSTEST_MAIN( TestQgsMessageBar )
#include "testqgsmessagebar.moc"
