/***************************************************************************
     testqgssearchstring.cpp
     --------------------------------------
    Date                 : March 28, 2010
    Copyright            : (C) 2010 Martin Dobias
    Email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest>

#include <qgssearchstring.h>
#include <qgssearchtreenode.h>
#include <qgsfeature.h>

class TestQgsSearchString : public QObject
{
    Q_OBJECT;
  private slots:
    //void initTestCase();// will be called before the first testfunction is executed.
    //void cleanupTestCase();// will be called after the last testfunction was executed.
    //void init();// will be called before each testfunction is executed.
    //void cleanup();// will be called after every testfunction.

    void testLike();
    void testRegexp();

  private:
    QString mReport;
};

static bool evalString( QString str )
{
  QgsFeature f;
  QgsSearchString ss;
  ss.setString( str );
  return ss.tree()->checkAgainst( QgsFieldMap(), f );
}

void TestQgsSearchString::testLike()
{
  QVERIFY( evalString( "'a' LIKE 'a'" ) );
  QVERIFY( ! evalString( "'aa' LIKE 'a'" ) );
  QVERIFY( ! evalString( "'a' LIKE 'b'" ) );

  QVERIFY( evalString( "'abba' LIKE 'a%'" ) );
  QVERIFY( ! evalString( "'abba' LIKE 'b%'" ) );
  QVERIFY( evalString( "'abba' LIKE '%a'" ) );
  QVERIFY( ! evalString( "'abba' LIKE '%b'" ) );

  QVERIFY( evalString( "'abba' LIKE '%bb%'" ) );
  QVERIFY( evalString( "'abba' LIKE 'a%a'" ) );
  QVERIFY( ! evalString( "'abba' LIKE 'b%b'" ) );
}

void TestQgsSearchString::testRegexp()
{
  QVERIFY( evalString( "'a' ~ 'a'" ) );
  QVERIFY( ! evalString( "'b' ~ 'a'" ) );

  QVERIFY( evalString( "'abba' ~ 'a'" ) );
  QVERIFY( ! evalString( "'abba' ~ 'aba'" ) );
  QVERIFY( evalString( "'abba' ~ 'a.*a'" ) );
  QVERIFY( evalString( "'abba' ~ 'a[b]+a'" ) );
}

QTEST_MAIN( TestQgsSearchString )
#include "moc_testqgssearchstring.cxx"
