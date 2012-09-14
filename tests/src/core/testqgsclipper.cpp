/***************************************************************************
     testqgsclipper.cpp
     --------------------------------------
    Date                 : Tue 14 Aug 2012
    Copyright            : (C) 2012 by Magnus Homann
    Email                : magnus at homann dot se
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest>
#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QString>
#include <QStringList>
#include <qgsapplication.h>
//header for class being tested
#include <qgsclipper.h>
#include <qgspoint.h>
#include "qgslogger.h"

class TestQgsClipper: public QObject
{

    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase() {};// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.
    void basic();
};

void TestQgsClipper::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  // QgsApplication::init();
  // QgsApplication::initQgis();
  // QgsApplication::showSettings();
}

void TestQgsClipper::basic()
{
  // CQgsClipper is static only
  //QgsClipper snipsnip;

  QPolygonF polygon;
  polygon << QPointF(10.4, 20.5) << QPointF(20.2, 30.2);
  
  QgsRectangle clipRect(10, 10, 25, 30 );
  
  QgsClipper::trimPolygon( polygon, clipRect );
  
  QRectF bBox( polygon.boundingRect() );
  QgsRectangle boundingRect( bBox.bottomLeft().x(), bBox.bottomLeft().y(), bBox.topRight().x(), bBox.topRight().y() );

  QVERIFY( clipRect.contains( boundingRect ) );
};

QTEST_MAIN( TestQgsClipper )
#include "moc_testqgsclipper.cxx"
