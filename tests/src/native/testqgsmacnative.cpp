/***************************************************************************
     testqgsmacnative.cpp
     --------------------------------------
    Date                 : January 2014
    Copyright            : (C) 2014 by Larry Shaffer
    Email                : larrys at dakotacarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest>
#include <QObject>
#include <QString>

//header for class being tested
#include <qgsmacappkit.h>

class TestQgsMacNative: public QObject
{
  Q_OBJECT

  private slots:
    void testGetRunningAppName();
};

void TestQgsMacNative::testGetRunningAppName()
{
  QgsNSRunningApplication* nsrapp = new QgsNSRunningApplication();
  QString nsrapp_name( nsrapp->currentAppLocalizedName() );
  delete nsrapp;

  QCOMPARE( QString( "qgis_macnativetest" ), nsrapp_name.trimmed() );
}

QTEST_MAIN( TestQgsMacNative )
#include "moc_testqgsmacnative.cxx"

