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


#include <memory>

#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgstest.h"

class TestQgsMessageBar : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
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

  QgsMessageBarItem *item = new QgsMessageBarItem( u"test"_s );
  const QPointer<QgsMessageBarItem> pItem( item );
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
  bar.pushMessage( u"1"_s );
  QCOMPARE( bar.items().size(), 1 );
  QCOMPARE( bar.items().at( 0 )->text(), u"1"_s );
  QCOMPARE( bar.currentItem()->text(), u"1"_s );
  const QPointer<QgsMessageBarItem> item1 = bar.currentItem();
  // make sure correct item is the visible one
  QCOMPARE( qobject_cast<QgsMessageBarItem *>( qgis::down_cast<QGridLayout *>( bar.layout() )->itemAt( 3 )->widget() )->text(), u"1"_s );

  bar.pushMessage( u"2"_s );
  QCOMPARE( bar.items().size(), 2 );
  QCOMPARE( bar.items().at( 0 )->text(), u"2"_s );
  QCOMPARE( bar.items().at( 1 )->text(), u"1"_s );
  QCOMPARE( bar.currentItem()->text(), u"2"_s );
  const QPointer<QgsMessageBarItem> item2 = bar.currentItem();
  QCOMPARE( qobject_cast<QgsMessageBarItem *>( qgis::down_cast<QGridLayout *>( bar.layout() )->itemAt( 3 )->widget() )->text(), u"2"_s );

  bar.pushMessage( u"3"_s );
  QCOMPARE( bar.items().size(), 3 );
  QCOMPARE( bar.items().at( 0 )->text(), u"3"_s );
  QCOMPARE( bar.items().at( 1 )->text(), u"2"_s );
  QCOMPARE( bar.items().at( 2 )->text(), u"1"_s );
  QCOMPARE( bar.currentItem()->text(), u"3"_s );
  const QPointer<QgsMessageBarItem> item3 = bar.currentItem();
  QCOMPARE( qobject_cast<QgsMessageBarItem *>( qgis::down_cast<QGridLayout *>( bar.layout() )->itemAt( 3 )->widget() )->text(), u"3"_s );

  const int childCount = bar.children().count();
  QVERIFY( bar.popWidget() );
  QCOMPARE( bar.items().size(), 2 );
  QCOMPARE( bar.items().at( 0 )->text(), u"2"_s );
  QCOMPARE( bar.items().at( 1 )->text(), u"1"_s );
  QCOMPARE( bar.currentItem()->text(), u"2"_s );
  QCOMPARE( qobject_cast<QgsMessageBarItem *>( qgis::down_cast<QGridLayout *>( bar.layout() )->itemAt( 3 )->widget() )->text(), u"2"_s );
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QCOMPARE( bar.children().count(), childCount - 1 );
  QVERIFY( !item3 );

  QVERIFY( bar.popWidget() );
  QCOMPARE( bar.items().size(), 1 );
  QCOMPARE( bar.items().at( 0 )->text(), u"1"_s );
  QCOMPARE( bar.currentItem()->text(), u"1"_s );
  QCOMPARE( qobject_cast<QgsMessageBarItem *>( qgis::down_cast<QGridLayout *>( bar.layout() )->itemAt( 3 )->widget() )->text(), u"1"_s );
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
  QCOMPARE( bar.items().at( 0 )->text(), u"99"_s );
  QCOMPARE( bar.items().at( 99 )->text(), u"0"_s );
  const QPointer<QgsMessageBarItem> oldest = bar.items().at( 99 );

  // push one more item, oldest one should be auto-removed
  bar.pushMessage( u"100"_s, Qgis::MessageLevel::Warning );
  QCOMPARE( bar.items().size(), 100 );
  QCOMPARE( bar.items().at( 0 )->text(), u"100"_s );
  QCOMPARE( bar.items().at( 99 )->text(), u"1"_s );
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !oldest );

  // but if we have a lower priority message we can pop, then do that instead
  bar.pushMessage( u"101"_s, Qgis::MessageLevel::Info );
  QCOMPARE( bar.items().size(), 100 );
  QCOMPARE( bar.items().at( 0 )->text(), u"101"_s );
  QCOMPARE( bar.items().at( 1 )->text(), u"100"_s );
  QCOMPARE( bar.items().at( 99 )->text(), u"2"_s );
  bar.pushMessage( u"102"_s, Qgis::MessageLevel::Info );
  QCOMPARE( bar.items().size(), 100 );
  QCOMPARE( bar.items().at( 0 )->text(), u"102"_s );
  QCOMPARE( bar.items().at( 1 )->text(), u"100"_s );
  QCOMPARE( bar.items().at( 99 )->text(), u"2"_s );
}

QGSTEST_MAIN( TestQgsMessageBar )
#include "testqgsmessagebar.moc"
