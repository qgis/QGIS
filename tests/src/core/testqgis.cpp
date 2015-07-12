/***************************************************************************
     testqgis.cpp
     ------------
    Date                 : March 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QApplication>

//qgis includes...
#include <qgis.h>

/** \ingroup UnitTests
 * Includes unit tests for the QGis namespace
 */
class TestQGis : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void permissiveToDouble();
    void permissiveToInt();
    void doubleToString();

  private:
    QString mReport;
};

//runs before all tests
void TestQGis::initTestCase()
{
  mReport = "<h1>QGis Tests</h1>\n";
}

//runs after all tests
void TestQGis::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQGis::permissiveToDouble()
{
  //good inputs
  bool ok = false;
  double result = QGis::permissiveToDouble( QString( "1000" ), ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000.0 );
  ok = false;
  result = QGis::permissiveToDouble( QString( "1" ) + QLocale::system().groupSeparator() + "000", ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000.0 );
  ok = false;
  result = QGis::permissiveToDouble( QString( "5" ) + QLocale::system().decimalPoint() + "5", ok );
  QVERIFY( ok );
  QCOMPARE( result, 5.5 );
  ok = false;
  result = QGis::permissiveToDouble( QString( "1" ) + QLocale::system().groupSeparator() + "000" + QLocale::system().decimalPoint() + "5", ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000.5 );

  //bad input
  ok = false;
  result = QGis::permissiveToDouble( QString( "a" ), ok );
  QVERIFY( !ok );

  //messy input (invalid thousand separator position), should still be converted
  ok = false;
  result = QGis::permissiveToDouble( QString( "10" ) + QLocale::system().groupSeparator() + "00", ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000.0 );
  ok = false;
  result = QGis::permissiveToDouble( QString( "10" ) + QLocale::system().groupSeparator() + "00" + QLocale::system().decimalPoint() + "5", ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000.5 );
}

void TestQGis::permissiveToInt()
{
  //good inputs
  bool ok = false;
  int result = QGis::permissiveToInt( QString( "1000" ), ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000 );
  ok = false;
  result = QGis::permissiveToInt( QString( "1%01000" ).arg( QLocale::system().groupSeparator() ), ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000 );

  //bad input
  ok = false;
  result = QGis::permissiveToInt( QString( "a" ), ok );
  QVERIFY( !ok );

  //messy input (invalid thousand separator position), should still be converted
  ok = false;
  result = QGis::permissiveToInt( QString( "10%0100" ).arg( QLocale::system().groupSeparator() ), ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000 );
}

void TestQGis::doubleToString()
{
  QCOMPARE( qgsDoubleToString( 5.6783212, 5 ), QString( "5.67832" ) );
  QCOMPARE( qgsDoubleToString( 5.5555555, 5 ), QString( "5.55556" ) );
  QCOMPARE( qgsDoubleToString( 12.2, 1 ), QString( "12.2" ) );
  QCOMPARE( qgsDoubleToString( 12.2, 2 ), QString( "12.2" ) );
  QCOMPARE( qgsDoubleToString( 12.2, 10 ), QString( "12.2" ) );
  QCOMPARE( qgsDoubleToString( 12.234333, 1 ), QString( "12.2" ) );
  QCOMPARE( qgsDoubleToString( 12, 1 ), QString( "12" ) );
  QCOMPARE( qgsDoubleToString( 12, 0 ), QString( "12" ) );
  QCOMPARE( qgsDoubleToString( 12000, 0 ), QString( "12000" ) );
  QCOMPARE( qgsDoubleToString( 12000, 1 ), QString( "12000" ) );
  QCOMPARE( qgsDoubleToString( 12000, 10 ), QString( "12000" ) );
  QCOMPARE( qgsDoubleToString( 12345, -1 ), QString( "12345" ) );
}

QTEST_MAIN( TestQGis )
#include "testqgis.moc"
