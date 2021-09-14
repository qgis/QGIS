/***************************************************************************
     testqgshttpheaders.cpp
     --------------------------------------
    Date                 : Tue 14 Sep 2012
    Copyright            : (C) 2012 by Benoit De Mezzo
    Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QString>
#include <QStringList>
#include <qgsapplication.h>
//header for class being tested
#include <qgshttpheaders.h>
#include <qgspoint.h>
#include "qgslogger.h"

class TestQgsHttpheaders: public QObject
{

    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase() {} // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void setFromSettings();
    void updateSettings();
};

void TestQgsHttpheaders::initTestCase()
{

}

void TestQgsHttpheaders::setFromSettings()
{
  QgsSettings settings;
  QString keyBase = "qgis/mytest1/";
  QString outOfHeaderKey = "outofheader";
  settings.remove( keyBase ); // cleanup

  settings.setValue( keyBase + outOfHeaderKey, "value" );
  settings.setValue( keyBase + QgsHttpHeaders::KEY_PREFIX + "key1", "value1" );
  settings.setValue( keyBase + QgsHttpHeaders::KEY_PREFIX + "referer", "valueR" );
  QgsHttpHeaders h( settings, keyBase );
  QVERIFY( ! h.keys().contains( outOfHeaderKey ) );
  QVERIFY( h.keys().contains( "key1" ) );
  QCOMPARE( h [ "key1" ].toString(), "value1" );
  QVERIFY( h.keys().contains( "referer" ) );
  QCOMPARE( h [ "referer" ].toString(), "valueR" );

}

void TestQgsHttpheaders::updateSettings()
{
  QgsSettings settings;
  QString keyBase = "qgis/mytest2/";
  settings.remove( keyBase ); // cleanup

  QgsHttpHeaders h( ( QMap<QString, QVariant> ) { {QStringLiteral( "key1" ), "value1"}} );
  h.updateSettings( settings, keyBase );
  QVERIFY( settings.contains( keyBase + QgsHttpHeaders::KEY_PREFIX + "key1" ) );
  QCOMPARE( settings.value( keyBase + QgsHttpHeaders::KEY_PREFIX + "key1" ).toString(), "value1" );

  QList<QString> keys = settings.allKeys();
  QVERIFY( ! settings.contains( keyBase + "referer" ) );
  QVERIFY( ! settings.contains( keyBase + QgsHttpHeaders::KEY_PREFIX + "referer" ) );

  h [ "referer" ] = "http://gg.com";

  h.updateSettings( settings, keyBase );
  QVERIFY( settings.contains( keyBase + QgsHttpHeaders::KEY_PREFIX + "referer" ) );
  QCOMPARE( settings.value( keyBase + QgsHttpHeaders::KEY_PREFIX + "referer" ).toString(), "http://gg.com" );
  QVERIFY( ! settings.contains( keyBase +  "referer" ) );

  // test backward compability
  settings.setValue( keyBase + "referer", "paf" ) ; // legacy referer, should be overriden
  h.updateSettings( settings, keyBase );
  QVERIFY( settings.contains( keyBase + QgsHttpHeaders::KEY_PREFIX + "referer" ) );
  QCOMPARE( settings.value( keyBase + QgsHttpHeaders::KEY_PREFIX + "referer" ).toString(), "http://gg.com" );
  QVERIFY( settings.contains( keyBase + "referer" ) );
  QCOMPARE( settings.value( keyBase + "referer" ).toString(), "http://gg.com" );
}


QGSTEST_MAIN( TestQgsHttpheaders )
#include "testqgshttpheaders.moc"
