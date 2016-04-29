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
#include <QCheckBox>

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
    void qgsround();
    void signalBlocker();
    void qVariantCompare_data();
    void qVariantCompare();

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
  ( void ) QGis::permissiveToDouble( QString( "a" ), ok );
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
  ( void ) QGis::permissiveToInt( QString( "a" ), ok );
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

void TestQGis::qgsround()
{
  QCOMPARE( qgsRound( 3.141592653589793 ), 3. );
  QCOMPARE( qgsRound( 2.718281828459045 ), 3. );
  QCOMPARE( qgsRound( -3.141592653589793 ), -3. );
  QCOMPARE( qgsRound( -2.718281828459045 ), -3. );
  QCOMPARE( qgsRound( 314159265358979.3 ), 314159265358979. );
  QCOMPARE( qgsRound( 2718281828459.045 ), 2718281828459. );
  QCOMPARE( qgsRound( -314159265358979.3 ), -314159265358979. );
  QCOMPARE( qgsRound( -2718281828459.045 ), -2718281828459. );
  QCOMPARE( qgsRound( 1.5 ), 2. );
  QCOMPARE( qgsRound( -1.5 ), -2. );
}

void TestQGis::signalBlocker()
{
  QScopedPointer< QCheckBox > checkbox( new QCheckBox() );

  QSignalSpy spy( checkbox.data(), SIGNAL( toggled( bool ) ) );

  //first check that signals are not blocked
  QVERIFY( !checkbox->signalsBlocked() );
  checkbox->setChecked( true );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toBool(), true );

  //block signals
  {
    QgsSignalBlocker< QCheckBox > blocker( checkbox.data() );
    QVERIFY( checkbox->signalsBlocked() );

    checkbox->setChecked( false );
    QVERIFY( !checkbox->isChecked() );

    //should be no new signals
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( spy.last().at( 0 ).toBool(), true );
    checkbox->setChecked( true );
  }

  //blocker is out of scope, blocking should be removed
  QVERIFY( !checkbox->signalsBlocked() );
  checkbox->setChecked( false );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toBool(), false );

  // now check that initial blocking state is restored when QgsSignalBlocker goes out of scope
  checkbox->blockSignals( true );
  {
    QgsSignalBlocker< QCheckBox > blocker( checkbox.data() );
    QVERIFY( checkbox->signalsBlocked() );
  }
  // initial blocked state should be restored
  QVERIFY( checkbox->signalsBlocked() );
  checkbox->blockSignals( false );

  // nested signal blockers
  {
    QgsSignalBlocker< QCheckBox > blocker( checkbox.data() );
    QVERIFY( checkbox->signalsBlocked() );
    {
      QgsSignalBlocker< QCheckBox > blocker2( checkbox.data() );
      QVERIFY( checkbox->signalsBlocked() );
    }
    QVERIFY( checkbox->signalsBlocked() );
  }
  QVERIFY( !checkbox->signalsBlocked() );

  // check whileBlocking function
  checkbox->setChecked( true );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.last().at( 0 ).toBool(), true );

  QVERIFY( !checkbox->signalsBlocked() );
  whileBlocking( checkbox.data() )->setChecked( false );
  // should have been no signals emitted
  QCOMPARE( spy.count(), 3 );
  // check that initial state of blocked signals was restored correctly
  QVERIFY( !checkbox->signalsBlocked() );
  checkbox->blockSignals( true );
  QVERIFY( checkbox->signalsBlocked() );
  whileBlocking( checkbox.data() )->setChecked( true );
  QVERIFY( checkbox->signalsBlocked() );
}

void TestQGis::qVariantCompare_data()
{
  QTest::addColumn<QVariant>( "lhs" );
  QTest::addColumn<QVariant>( "rhs" );
  QTest::addColumn<bool>( "lessThan" );
  QTest::addColumn<bool>( "greaterThan" );

  QTest::newRow( "invalid to value" ) << QVariant() << QVariant( 2 ) << true << false;
  QTest::newRow( "invalid to value 2" ) << QVariant( 2 ) << QVariant() << false << true;
  QTest::newRow( "invalid to null" ) << QVariant() << QVariant( QVariant::String ) << true << false;
  QTest::newRow( "invalid to null2 " ) << QVariant( QVariant::String ) << QVariant() << false << true;
  QTest::newRow( "null to value" ) <<  QVariant( QVariant::String ) << QVariant( "a" ) << true << false;
  QTest::newRow( "null to value 2" ) << QVariant( "a" ) << QVariant( QVariant::String ) << false << true;

  QTest::newRow( "int" ) << QVariant( 1 ) << QVariant( 2 ) << true << false;
  QTest::newRow( "int 2" ) << QVariant( 1 ) << QVariant( -2 ) << false << true;
  QTest::newRow( "int 3" ) << QVariant( 0 ) << QVariant( 1 ) << true << false;
  QTest::newRow( "uint" ) << QVariant( 1u ) << QVariant( 2u ) << true << false;
  QTest::newRow( "uint 2" ) << QVariant( 2u ) << QVariant( 0u ) << false << true;
  QTest::newRow( "long long" ) << QVariant( 1LL ) << QVariant( 2LL ) << true << false;
  QTest::newRow( "long long 2" ) << QVariant( 1LL ) << QVariant( -2LL ) << false << true;
  QTest::newRow( "long long 3" ) << QVariant( 0LL ) << QVariant( 1LL ) << true << false;
  QTest::newRow( "ulong long" ) << QVariant( 1uLL ) << QVariant( 2uLL ) << true << false;
  QTest::newRow( "ulong long 2" ) << QVariant( 2uLL ) << QVariant( 0uLL ) << false << true;
  QTest::newRow( "double" ) << QVariant( 1.5 ) << QVariant( 2.5 ) << true << false;
  QTest::newRow( "double 2" ) << QVariant( 1.5 ) << QVariant( -2.5 ) << false << true;
  QTest::newRow( "double 3" ) << QVariant( 0.5 ) << QVariant( 1.5 ) << true << false;
  QTest::newRow( "char" ) << QVariant( 'b' ) << QVariant( 'x' ) << true << false;
  QTest::newRow( "char 2" ) << QVariant( 'x' ) << QVariant( 'b' ) << false << true;
  QTest::newRow( "date" ) << QVariant( QDate( 2000, 5, 6 ) ) << QVariant( QDate( 2000, 8, 6 ) ) << true << false;
  QTest::newRow( "date 2" ) << QVariant( QDate( 2000, 8, 6 ) ) << QVariant( QDate( 2000, 5, 6 ) ) << false << true;
  QTest::newRow( "time" ) << QVariant( QTime( 13, 5, 6 ) ) << QVariant( QTime( 13, 8, 6 ) ) << true << false;
  QTest::newRow( "time 2" ) << QVariant( QTime( 18, 8, 6 ) ) << QVariant( QTime( 13, 5, 6 ) ) << false << true;
  QTest::newRow( "datetime" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 8, 6 ), QTime( 13, 5, 6 ) ) ) << true << false;
  QTest::newRow( "datetime 2" ) << QVariant( QDateTime( QDate( 2000, 8, 6 ), QTime( 13, 5, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << false << true;
  QTest::newRow( "datetime 3" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 9, 6 ) ) ) << true << false;
  QTest::newRow( "datetime 4" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 9, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << false << true;
  QTest::newRow( "bool" ) << QVariant( false ) << QVariant( true ) << true << false;
  QTest::newRow( "bool 2" ) << QVariant( true ) << QVariant( false ) << false << true;
  QTest::newRow( "qvariantlist" ) << QVariant( QVariantList() << QVariant( 5 ) ) << QVariant( QVariantList() << QVariant( 9 ) ) << true << false;
  QTest::newRow( "qvariantlist 2" ) << QVariant( QVariantList() << QVariant( 9 ) ) << QVariant( QVariantList() << QVariant( 5 ) ) << false << true;
  QTest::newRow( "qvariantlist 3" ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 3 ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << true << false;
  QTest::newRow( "qvariantlist 4" ) << QVariant( QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 3 ) ) << false << true;
  QTest::newRow( "qvariantlist 5" ) << QVariant( QVariantList() << QVariant( 5 ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << true << false;
  QTest::newRow( "qvariantlist 5" ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << QVariant( QVariantList() << QVariant( 5 ) ) << false << true;
  QTest::newRow( "qstringlist" ) << QVariant( QStringList() << "aa" ) << QVariant( QStringList() << "bb" ) << true << false;
  QTest::newRow( "qstringlist 2" ) << QVariant( QStringList() << "bb" ) << QVariant( QStringList() << "aa" ) << false << true;
  QTest::newRow( "qstringlist 3" ) << QVariant( QStringList() << "aa" << "cc" ) << QVariant( QStringList() << "aa" << "xx" ) << true << false;
  QTest::newRow( "qstringlist 4" ) << QVariant( QStringList() << "aa" << "xx" ) << QVariant( QStringList() << "aa" << "cc" ) << false << true;
  QTest::newRow( "qstringlist 5" ) << QVariant( QStringList() << "aa" ) << QVariant( QStringList() << "aa" << "xx" ) << true << false;
  QTest::newRow( "qstringlist 6" ) << QVariant( QStringList() << "aa" << "xx" ) << QVariant( QStringList() << "aa" ) << false << true;
  QTest::newRow( "string" ) << QVariant( "a b c" ) << QVariant( "d e f" ) << true << false;
  QTest::newRow( "string 2" ) << QVariant( "d e f" ) << QVariant( "a b c" ) << false << true;
}

void TestQGis::qVariantCompare()
{
  QFETCH( QVariant, lhs );
  QFETCH( QVariant, rhs );
  QFETCH( bool, lessThan );
  QFETCH( bool, greaterThan );

  QCOMPARE( qgsVariantLessThan( lhs, rhs ), lessThan );
  QCOMPARE( qgsVariantGreaterThan( lhs, rhs ), greaterThan );
}


QTEST_MAIN( TestQGis )
#include "testqgis.moc"
