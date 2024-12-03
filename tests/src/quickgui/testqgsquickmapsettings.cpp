/***************************************************************************
     testqgsquickmapsettings.cpp
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

#include "qgis.h"
#include "qgsapplication.h"
#include "qgstest.h"
#include "qgspoint.h"
#include "qgsunittypes.h"

#include "qgsquickmapsettings.h"
#include "qgsquickmaptoscreen.h"

class TestQgsQuickMapSettings : public QObject
{
    Q_OBJECT
  private slots:
    void init() {}    // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_project_existency();
    void test_map_to_screen();
};

void TestQgsQuickMapSettings::test_project_existency()
{
  QgsQuickMapSettings *settings = new QgsQuickMapSettings();
  QVERIFY( !settings->project() );
  delete settings;
}

void TestQgsQuickMapSettings::test_map_to_screen()
{
  QgsQuickMapSettings *settings = new QgsQuickMapSettings();
  settings->setOutputSize( QSize( 10, 10 ) );
  settings->setExtent( QgsRectangle( 0, 0, 10, 10 ) );

  QgsPoint point( 5, 5 );

  QgsQuickMapToScreen *mapToScreen = new QgsQuickMapToScreen();
  mapToScreen->setMapSettings( settings );
  mapToScreen->setMapPoint( point );

  QCOMPARE( mapToScreen->screenPoint(), QPointF( 5, 5 ) );
}

QGSTEST_MAIN( TestQgsQuickMapSettings )
#include "testqgsquickmapsettings.moc"
