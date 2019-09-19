/***************************************************************************
                         testqgsmapsettingsutils.cpp
                         -----------------------
    begin                : May 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsmapsettings.h"
#include "qgsmapsettingsutils.h"

#include <QString>

class TestQgsMapSettingsUtils : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase() {} // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after each testfunction was executed.

    void createWorldFileContent(); //test world file content function

  private:

    QgsMapSettings mMapSettings;

};

void TestQgsMapSettingsUtils::initTestCase()
{
  mMapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
  mMapSettings.setOutputSize( QSize( 1, 1 ) );
}

void TestQgsMapSettingsUtils::createWorldFileContent()
{
  double a, b, c, d, e, f;
  QgsMapSettingsUtils::worldFileParameters( mMapSettings, a, b, c, d, e, f );
  QCOMPARE( a, 1.0 );
  QCOMPARE( b, 0.0 );
  QCOMPARE( c, 0.5 );
  QCOMPARE( d, 0.0 );
  QCOMPARE( e, -1.0 );
  QCOMPARE( f, 0.5 );

  QCOMPARE( QgsMapSettingsUtils::worldFileContent( mMapSettings ), QString( "1\r\n0\r\n0\r\n-1\r\n0.5\r\n0.5\r\n" ) );

  mMapSettings.setRotation( 45 );
  QCOMPARE( QgsMapSettingsUtils::worldFileContent( mMapSettings ), QString( "0.70710678118654757\r\n0.70710678118654746\r\n0.70710678118654746\r\n-0.70710678118654757\r\n0.5\r\n0.49999999999999994\r\n" ) );

  mMapSettings.setRotation( 145 );
  QCOMPARE( QgsMapSettingsUtils::worldFileContent( mMapSettings ), QString( "-0.81915204428899191\r\n0.57357643635104594\r\n0.57357643635104594\r\n0.81915204428899191\r\n0.5\r\n0.49999999999999994\r\n" ) );
}

QGSTEST_MAIN( TestQgsMapSettingsUtils )
#include "testqgsmapsettingsutils.moc"
