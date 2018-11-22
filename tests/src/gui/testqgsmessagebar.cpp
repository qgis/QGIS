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
  QPointer< QgsMessageBarItem > pItem( item );
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



QGSTEST_MAIN( TestQgsMessageBar )
#include "testqgsmessagebar.moc"
