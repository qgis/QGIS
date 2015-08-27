/***************************************************************************
     testqgsmaptoolzoom.cpp
     --------------------------------------
    Date                 : Sat Apr 28th 2012
    Copyright            : (C) 2012 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QWidget>
#include <QMouseEvent>

#include <qgsmaptoolzoom.h>
#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgslogger.h>

class TestQgsMapToolZoom : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolZoom()
        : canvas( 0 )
    {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void zeroDragArea();
  private:
    QgsMapCanvas* canvas;
};

void TestQgsMapToolZoom::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsMapToolZoom::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolZoom::init()
{
  canvas = new QgsMapCanvas();
}

void TestQgsMapToolZoom::cleanup()
{
  delete canvas;
}

/** Zero drag areas can happen on pen based computer when a mouse down,
  * move, and up, all happened at the same spot due to the pen. In this case
  * QGIS thinks it is in dragging mode but it's not really and fails to zoom in.
  **/
void TestQgsMapToolZoom::zeroDragArea()
{
  QPoint point = QPoint( 15, 15 );
  QMouseEvent press( QEvent::MouseButtonPress, point ,
                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QMouseEvent move( QEvent::MouseMove, point,
                    Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QMouseEvent releases( QEvent::MouseButtonRelease, point,
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );

  QgsMapToolZoom* tool = new QgsMapToolZoom( canvas, false );
  // Just set some made up extent so that we can zoom.
  canvas->setExtent( QgsRectangle( 0, 0, 20, 20 ) );

  QgsRectangle before = canvas->extent();
  tool->canvasPressEvent( &press );
  tool->canvasMoveEvent( &move );
  tool->canvasReleaseEvent( &releases );
  QgsRectangle after = canvas->extent();
  // We don't really care if we zoom in or out here just that the extent did
  // change we
  QVERIFY2( before != after, "Extents didn't change" );
}

QTEST_MAIN( TestQgsMapToolZoom )
#include "testqgsmaptoolzoom.moc"




