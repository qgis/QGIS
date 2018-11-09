/***************************************************************************
     testqgspositionkit.cpp
     --------------------------------------
  Date                 : May 2018
  Copyright            : (C) 2017 by Viktor Sklencar
  Email                : vsklencar at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QObject>
#include <QApplication>
#include <QDesktopWidget>

#include "qgsapplication.h"
#include "qgstest.h"
#include "qgis.h"

#include "qgsquickpositionkit.h"
#include "qgsquicksimulatedpositionsource.h"
#include "qgsquickutils.h"

class TestQgsQuickUtils: public QObject
{
    Q_OBJECT
  private slots:
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void simulated_position();

  private:
    QgsQuickUtils utils;
    QgsQuickPositionKit positionKit;
};

void TestQgsQuickUtils::simulated_position()
{
  QVERIFY( !positionKit.isSimulated() );
  positionKit.useSimulatedLocation( -92.36, 38.93, -1 );
  QVERIFY( positionKit.isSimulated() );

  QVERIFY( positionKit.hasPosition() );
  QGSCOMPARENEAR( positionKit.position().y(), 38.93, 1e-4 );
  QVERIFY( positionKit.accuracy() > 0 );

  const QVector<double> newPosition( { 90.36, 33.93, -1 } );
  positionKit.setSimulatePositionLongLatRad( newPosition );
  QVERIFY( positionKit.hasPosition() );
  QGSCOMPARENEAR( positionKit.position().y(), newPosition[1], 1e-4 );

  positionKit.setSimulatePositionLongLatRad( QVector<double>() );
  QVERIFY( !positionKit.isSimulated() );
}

QGSTEST_MAIN( TestQgsQuickUtils )
#include "testqgsquickpositionkit.moc"
