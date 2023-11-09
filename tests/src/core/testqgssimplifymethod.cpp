/***************************************************************************
     testqgssimplifymethod.cpp
     --------------------------
    Date                 : Oct 2021
    Copyright            : (C) 2021 Sandro Santilli
    Email                : strk at kbt dot io
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
#include <QStringList>
#include <QLocale>

#include <memory>

#include "qgssimplifymethod.h"

class TestQgsSimplifyMethod: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testCreate();//test creating a simplify method
    void testEqualityInequality();//test equality operator

  private:
};

void TestQgsSimplifyMethod::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
}

void TestQgsSimplifyMethod::cleanupTestCase()
{

}

void TestQgsSimplifyMethod::init()
{
  QLocale::setDefault( QLocale::English );
}

void TestQgsSimplifyMethod::cleanup()
{
  QLocale::setDefault( QLocale::English );
}

void TestQgsSimplifyMethod::testCreate()
{
  QgsSimplifyMethod method0;
  QCOMPARE( method0.methodType(), QgsSimplifyMethod::NoSimplification );
}

void TestQgsSimplifyMethod::testEqualityInequality()
{
  QgsSimplifyMethod method0;
  QgsSimplifyMethod method1;
  method1.setMethodType( QgsSimplifyMethod::OptimizeForRendering );

  QVERIFY( method0 == method0 );
  QVERIFY( method0 != method1 );
}


QGSTEST_MAIN( TestQgsSimplifyMethod )
#include "testqgssimplifymethod.moc"
