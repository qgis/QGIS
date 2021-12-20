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
#include <QDesktopWidget>

#include "qgsapplication.h"
#include "qgstest.h"
#include "qgis.h"
#include "qgsunittypes.h"

#include "qgsquickmapsettings.h"

class TestQgsQuickMapSettings: public QObject
{
    Q_OBJECT
  private slots:
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_project_existency();
};

void TestQgsQuickMapSettings::test_project_existency()
{
  QgsQuickMapSettings *settings = new QgsQuickMapSettings();
  QVERIFY( !settings->project() );
  delete settings;
}

QGSTEST_MAIN( TestQgsQuickMapSettings )
#include "testqgsquickmapsettings.moc"
