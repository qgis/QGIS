/***************************************************************************
    testqgswidgets.cpp
     --------------------------------------
    Date                 : August 2015
    Copyright            : (C) 2015 Nyall Dawson
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

#include "qgsmultibandcolorrendererwidget.h"

class TestQgsWidgets: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void cast();

  private:

};

void TestQgsWidgets::initTestCase()
{

}

void TestQgsWidgets::cleanupTestCase()
{
}

void TestQgsWidgets::init()
{
}

void TestQgsWidgets::cleanup()
{
}

void TestQgsWidgets::cast()
{
  //create new QgsRasterRendererWidget
  QWidget* widget = new QgsMultiBandColorRendererWidget( 0 );

  //try casting as renderer widget
  QgsRasterRendererWidget* rendererWidget = dynamic_cast<QgsRasterRendererWidget*>( widget );
  QVERIFY( rendererWidget );
  delete widget;
}

QTEST_MAIN( TestQgsWidgets )
#include "testqgswidgets.moc"
