/***************************************************************************
     testqgsquickutils.cpp
     --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
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
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgspoint.h"
#include "qgspointxy.h"
#include "qgstest.h"
#include "qgis.h"
#include "qgsunittypes.h"

#include "qgsquickutils.h"

class TestQgsQuickUtils: public QObject
{
    Q_OBJECT
  private slots:
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void screen_density();
  private:
    QgsQuickUtils utils;
};

void TestQgsQuickUtils::screen_density()
{
  qreal dp = utils.screenDensity();
  QVERIFY( ( dp > 0 ) && ( dp < 1000 ) );
}

QGSTEST_MAIN( TestQgsQuickUtils )
#include "testqgsquickutils.moc"
