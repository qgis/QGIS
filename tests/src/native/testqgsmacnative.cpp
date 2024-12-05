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

#include "qgstest.h"
#include <QObject>
#include <QString>

//header for class being tested
#include "qgsmacnative.h"

class TestQgsMacNative : public QObject
{
    Q_OBJECT

  private slots:
    void testGetRunningAppName();
};

void TestQgsMacNative::testGetRunningAppName()
{
  QgsMacNative *macNative = new QgsMacNative();
  QCOMPARE( QStringLiteral( "qgis_macnativetest" ), QString( macNative->currentAppLocalizedName() ) );
  delete macNative;
}

QGSTEST_MAIN( TestQgsMacNative )
#include "testqgsmacnative.moc"
