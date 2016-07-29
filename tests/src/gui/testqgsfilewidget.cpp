/***************************************************************************
    testqgsdoublespinbox.cpp
     --------------------------------------
    Date                 : December 2014
    Copyright            : (C) 2014 Nyall Dawson
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

#include "qgsfilewidget.h"

class TestQgsFileWidget: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void relativePath();
    void toUrl();

};

void TestQgsFileWidget::initTestCase()
{

}

void TestQgsFileWidget::cleanupTestCase()
{
}

void TestQgsFileWidget::init()
{
}

void TestQgsFileWidget::cleanup()
{
}

void TestQgsFileWidget::relativePath()
{
  QgsFileWidget* w = new QgsFileWidget();
  w->setDefaultRoot( "/home/test" );
  w->setRelativeStorage( QgsFileWidget::Absolute );
  QCOMPARE( w->relativePath( "/home/test2/file1.ext", true ), QString( "/home/test2/file1.ext" ) );
  QCOMPARE( w->relativePath( "/home/test2/file2.ext", false ), QString( "/home/test2/file2.ext" ) );
  w->setRelativeStorage( QgsFileWidget::RelativeDefaultPath );
  QCOMPARE( w->relativePath( "/home/test2/file3.ext", true ), QString( "../test2/file3.ext" ) );
  QCOMPARE( w->relativePath( "../test2/file4.ext", true ), QString( "../test2/file4.ext" ) );
  QCOMPARE( w->relativePath( "/home/test2/file5.ext", false ), QString( "/home/test2/file5.ext" ) );
  QCOMPARE( w->relativePath( "../test2/file6.ext", false ), QString( "/home/test2/file6.ext" ) );
}

void TestQgsFileWidget::toUrl()
{
  QgsFileWidget* w = new QgsFileWidget();
  w->setDefaultRoot( "/home/test" );
  w->setRelativeStorage( QgsFileWidget::Absolute );
  w->setFullUrl( true );
  QCOMPARE( w->toUrl( "/home/test2/file1.ext" ), QString( "<a href=\"file:///home/test2/file1.ext\">/home/test2/file1.ext</a>" ) );
  w->setFullUrl( false );
  QCOMPARE( w->toUrl( "/home/test2/file2.ext" ), QString( "<a href=\"file:///home/test2/file2.ext\">file2.ext</a>" ) );
  w->setRelativeStorage( QgsFileWidget::RelativeDefaultPath );
  w->setFullUrl( true );
  QCOMPARE( w->toUrl( "/home/test2/file3.ext" ), QString( "<a href=\"file:///home/test2/file3.ext\">/home/test2/file3.ext</a>" ) );
  QCOMPARE( w->toUrl( "../test2/file4.ext" ), QString( "<a href=\"file:///home/test2/file4.ext\">../test2/file4.ext</a>" ) );
  w->setFullUrl( false );
  QCOMPARE( w->toUrl( "/home/test2/file5.ext" ), QString( "<a href=\"file:///home/test2/file5.ext\">file5.ext</a>" ) );
  QCOMPARE( w->toUrl( "../test2/file6.ext" ), QString( "<a href=\"file:///home/test2/file6.ext\">file6.ext</a>" ) );
}



QTEST_MAIN( TestQgsFileWidget )
#include "testqgsfilewidget.moc"
