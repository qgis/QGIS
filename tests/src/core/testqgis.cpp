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
#include <memory>

#include "qgstest.h"

#include <QApplication>
#include <QCheckBox>
#include <QObject>
#include <QSignalSpy>
#include <QString>

//qgis includes...
#include "qgis.h"
#include "qgsmaplayermodel.h"
#include "qgsfieldproxymodel.h"

/**
 * \ingroup UnitTests
 * Includes unit tests for the Qgis namespace
 */
class TestQgis : public QgsTest
{
    Q_OBJECT

  public:
    enum class TestEnum : int
    {
      TestEnum1 = 1,
      TestEnum2 = 2,
      TestEnum3 = 6,
    };
    Q_ENUM( TestEnum )

  public:
    TestQgis()
      : QgsTest( u"Qgis Tests"_s ) {}

  private slots:
    void init() {}    // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void permissiveToDouble();
    void permissiveToInt();
    void permissiveToLongLong();
    void doubleToString();
    void signalBlocker();
    void qVariantOperators_data();
    void qVariantOperators();
    void qVariantCompare_data();
    void qVariantCompare();
    void testNanCompatibleEquals_data();
    void testNanCompatibleEquals();
    void testQgsAsConst();
    void testQgsRound();
    void testQgsVariantEqual();
    void testQgsEnumMapList();
    void testQgsEnumValueToKey();
    void testQgsEnumKeyToValue();
    void testQgsFlagValueToKeys();
    void testQgsFlagKeysToValue();
    void testQMapQVariantList();
    void testQgsMapJoin();
    void testQgsSetJoin();
};

void TestQgis::permissiveToDouble()
{
  //good inputs
  bool ok = false;
  double result = qgsPermissiveToDouble( u"1000"_s, ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000.0 );
  ok = false;
  result = qgsPermissiveToDouble( u"1"_s + QLocale().groupSeparator() + "000", ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000.0 );
  ok = false;
  result = qgsPermissiveToDouble( u"5"_s + QLocale().decimalPoint() + "5", ok );
  QVERIFY( ok );
  QCOMPARE( result, 5.5 );
  ok = false;
  result = qgsPermissiveToDouble( u"1"_s + QLocale().groupSeparator() + "000" + QLocale().decimalPoint() + "5", ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000.5 );

  //bad input
  ok = false;
  ( void ) qgsPermissiveToDouble( u"a"_s, ok );
  QVERIFY( !ok );

  //messy input (invalid thousand separator position), should still be converted
  ok = false;
  result = qgsPermissiveToDouble( u"10"_s + QLocale().groupSeparator() + "00", ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000.0 );
  ok = false;
  result = qgsPermissiveToDouble( u"10"_s + QLocale().groupSeparator() + "00" + QLocale().decimalPoint() + "5", ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000.5 );
}

void TestQgis::permissiveToInt()
{
  //good inputs
  bool ok = false;
  int result = qgsPermissiveToInt( u"1000"_s, ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000 );
  ok = false;
  result = qgsPermissiveToInt( u"1%01000"_s.arg( QLocale().groupSeparator() ), ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000 );

  //bad input
  ok = false;
  ( void ) qgsPermissiveToInt( u"a"_s, ok );
  QVERIFY( !ok );

  //messy input (invalid thousand separator position), should still be converted
  ok = false;
  result = qgsPermissiveToInt( u"10%0100"_s.arg( QLocale().groupSeparator() ), ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000 );
}

void TestQgis::permissiveToLongLong()
{
  //good inputs
  bool ok = false;
  qlonglong result = qgsPermissiveToLongLong( u"1000"_s, ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000 );
  ok = false;
  result = qgsPermissiveToLongLong( u"1%01000"_s.arg( QLocale().groupSeparator() ), ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000 );

  //bad input
  ok = false;
  ( void ) qgsPermissiveToLongLong( u"a"_s, ok );
  QVERIFY( !ok );

  //messy input (invalid thousand separator position), should still be converted
  ok = false;
  result = qgsPermissiveToLongLong( u"10%0100"_s.arg( QLocale().groupSeparator() ), ok );
  QVERIFY( ok );
  QCOMPARE( result, 1000 );
}

void TestQgis::doubleToString()
{
  QCOMPARE( qgsDoubleToString( 5.6783212, 5 ), u"5.67832"_s );
  QCOMPARE( qgsDoubleToString( 5.5555555, 5 ), u"5.55556"_s );
  QCOMPARE( qgsDoubleToString( 12.2, 1 ), u"12.2"_s );
  QCOMPARE( qgsDoubleToString( 12.2, 2 ), u"12.2"_s );
  QCOMPARE( qgsDoubleToString( 12.2, 10 ), u"12.2"_s );
  QCOMPARE( qgsDoubleToString( 12.234333, 1 ), u"12.2"_s );
  QCOMPARE( qgsDoubleToString( 12, 1 ), u"12"_s );
  QCOMPARE( qgsDoubleToString( 12, 0 ), u"12"_s );
  QCOMPARE( qgsDoubleToString( 12000, 0 ), u"12000"_s );
  QCOMPARE( qgsDoubleToString( 12000, 1 ), u"12000"_s );
  QCOMPARE( qgsDoubleToString( 12000, 10 ), u"12000"_s );
  QCOMPARE( qgsDoubleToString( 12345, -1 ), u"12350"_s );
  QCOMPARE( qgsDoubleToString( 12345.0111, -1 ), u"12350"_s );
  QCOMPARE( qgsDoubleToString( 12345.0111, -2 ), u"12300"_s );
  QCOMPARE( qgsDoubleToString( 12345.0111, -3 ), u"12000"_s );
  QCOMPARE( qgsDoubleToString( 12345.0111, -4 ), u"10000"_s );
  QCOMPARE( qgsDoubleToString( 12345.0111, -5 ), u"0"_s );
  QCOMPARE( qgsDoubleToString( 62345.0111, -5 ), u"100000"_s );
  QCOMPARE( qgsDoubleToString( 12345.0111, -6 ), u"0"_s );
  QCOMPARE( qgsDoubleToString( -12345.0111, -1 ), u"-12350"_s );
  QCOMPARE( qgsDoubleToString( -12345.0111, -2 ), u"-12300"_s );
  QCOMPARE( qgsDoubleToString( -12345.0111, -3 ), u"-12000"_s );
  QCOMPARE( qgsDoubleToString( -12345.0111, -4 ), u"-10000"_s );
  QCOMPARE( qgsDoubleToString( -12345.0111, -5 ), u"0"_s );
  QCOMPARE( qgsDoubleToString( -62345.0111, -5 ), u"-100000"_s );
  QCOMPARE( qgsDoubleToString( -12345.0111, -6 ), u"0"_s );
  QCOMPARE( qgsDoubleToString( 12345.12300000, 7 ), u"12345.123"_s );
  QCOMPARE( qgsDoubleToString( 12345.00011111, 2 ), u"12345"_s );
  QCOMPARE( qgsDoubleToString( -0.000000000708115, 0 ), u"0"_s );
}

void TestQgis::signalBlocker()
{
  auto checkbox = std::make_unique<QCheckBox>();

  QSignalSpy spy( checkbox.get(), &QCheckBox::toggled );

  //first check that signals are not blocked
  QVERIFY( !checkbox->signalsBlocked() );
  checkbox->setChecked( true );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toBool(), true );

  //block signals
  {
    QgsSignalBlocker<QCheckBox> blocker( checkbox.get() );
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
    QgsSignalBlocker<QCheckBox> blocker( checkbox.get() );
    QVERIFY( checkbox->signalsBlocked() );
  }
  // initial blocked state should be restored
  QVERIFY( checkbox->signalsBlocked() );
  checkbox->blockSignals( false );

  // nested signal blockers
  {
    QgsSignalBlocker<QCheckBox> blocker( checkbox.get() );
    QVERIFY( checkbox->signalsBlocked() );
    {
      QgsSignalBlocker<QCheckBox> blocker2( checkbox.get() );
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
  whileBlocking( checkbox.get() )->setChecked( false );
  // should have been no signals emitted
  QCOMPARE( spy.count(), 3 );
  // check that initial state of blocked signals was restored correctly
  QVERIFY( !checkbox->signalsBlocked() );
  checkbox->blockSignals( true );
  QVERIFY( checkbox->signalsBlocked() );
  whileBlocking( checkbox.get() )->setChecked( true );
  QVERIFY( checkbox->signalsBlocked() );
}

void TestQgis::qVariantOperators_data()
{
  QTest::addColumn<QVariant>( "lhs" );
  QTest::addColumn<QVariant>( "rhs" );
  QTest::addColumn<bool>( "lessThan" );
  QTest::addColumn<bool>( "greaterThan" );

  QTest::newRow( "both invalid" ) << QVariant() << QVariant() << false << false;
  QTest::newRow( "invalid to value" ) << QVariant() << QVariant( 2 ) << true << false;
  QTest::newRow( "invalid to value 2" ) << QVariant( 2 ) << QVariant() << false << true;
  QTest::newRow( "invalid to null" ) << QVariant() << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << true << false;
  QTest::newRow( "invalid to null2 " ) << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << QVariant() << false << true;
  QTest::newRow( "both null" ) << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << false << false;
  QTest::newRow( "null to value" ) << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << QVariant( "a" ) << true << false;
  QTest::newRow( "null to value 2" ) << QVariant( "a" ) << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << false << true;

  // type mismatches -- we MUST NOT compare the values here, just the types.
  // otherwise we break use of QMap< QVariant, ... > and incorrectly match different
  // variant types with the same value in the map.
  QTest::newRow( "int vs double" ) << QVariant( 1 ) << QVariant( 2.0 ) << true << false;
  QTest::newRow( "double vs int" ) << QVariant( 1.0 ) << QVariant( 2 ) << false << true;
  QTest::newRow( "int vs double same" ) << QVariant( 1 ) << QVariant( 1.0 ) << true << false;
  QTest::newRow( "double vs int same" ) << QVariant( 1.0 ) << QVariant( 1 ) << false << true;
  QTest::newRow( "int vs string" ) << QVariant( 1 ) << QVariant( "2" ) << true << false;
  QTest::newRow( "string vs int" ) << QVariant( "1" ) << QVariant( 2 ) << false << true;
  QTest::newRow( "int vs string same" ) << QVariant( 1 ) << QVariant( "1" ) << true << false;
  QTest::newRow( "string vs int same" ) << QVariant( "1" ) << QVariant( 1 ) << false << true;

  // matching types
  QTest::newRow( "int" ) << QVariant( 1 ) << QVariant( 2 ) << true << false;
  QTest::newRow( "int 2" ) << QVariant( 1 ) << QVariant( -2 ) << false << true;
  QTest::newRow( "int 3" ) << QVariant( 0 ) << QVariant( 1 ) << true << false;
  QTest::newRow( "int equal" ) << QVariant( 1 ) << QVariant( 1 ) << false << false;
  QTest::newRow( "uint" ) << QVariant( 1u ) << QVariant( 2u ) << true << false;
  QTest::newRow( "uint 2" ) << QVariant( 2u ) << QVariant( 0u ) << false << true;
  QTest::newRow( "uint equal" ) << QVariant( 2u ) << QVariant( 2u ) << false << false;
  QTest::newRow( "long long" ) << QVariant( 1LL ) << QVariant( 2LL ) << true << false;
  QTest::newRow( "long long 2" ) << QVariant( 1LL ) << QVariant( -2LL ) << false << true;
  QTest::newRow( "long long 3" ) << QVariant( 0LL ) << QVariant( 1LL ) << true << false;
  QTest::newRow( "long long equal" ) << QVariant( 1LL ) << QVariant( 1LL ) << false << false;
  QTest::newRow( "ulong long" ) << QVariant( 1uLL ) << QVariant( 2uLL ) << true << false;
  QTest::newRow( "ulong long 2" ) << QVariant( 2uLL ) << QVariant( 0uLL ) << false << true;
  QTest::newRow( "ulong long equal" ) << QVariant( 2uLL ) << QVariant( 2uLL ) << false << false;
  QTest::newRow( "double" ) << QVariant( 1.5 ) << QVariant( 2.5 ) << true << false;
  QTest::newRow( "double 2" ) << QVariant( 1.5 ) << QVariant( -2.5 ) << false << true;
  QTest::newRow( "double 3" ) << QVariant( 0.5 ) << QVariant( 1.5 ) << true << false;
  QTest::newRow( "double equal" ) << QVariant( 1.5 ) << QVariant( 1.5 ) << false << false;
  QTest::newRow( "double both nan" ) << QVariant( std::numeric_limits<double>::quiet_NaN() ) << QVariant( std::numeric_limits<double>::quiet_NaN() ) << false << false;
  QTest::newRow( "double lhs nan" ) << QVariant( 5.5 ) << QVariant( std::numeric_limits<double>::quiet_NaN() ) << false << true;
  QTest::newRow( "double rhs nan" ) << QVariant( std::numeric_limits<double>::quiet_NaN() ) << QVariant( 5.5 ) << true << false;
  QTest::newRow( "float" ) << QVariant( 1.5f ) << QVariant( 2.5f ) << true << false;
  QTest::newRow( "float 2" ) << QVariant( 1.5f ) << QVariant( -2.5f ) << false << true;
  QTest::newRow( "float 3" ) << QVariant( 0.5f ) << QVariant( 1.5f ) << true << false;
  QTest::newRow( "float equal" ) << QVariant( 1.5f ) << QVariant( 1.5f ) << false << false;
  QTest::newRow( "float both nan" ) << QVariant( std::numeric_limits<float>::quiet_NaN() ) << QVariant( std::numeric_limits<float>::quiet_NaN() ) << false << false;
  QTest::newRow( "float lhs nan" ) << QVariant( 5.5f ) << QVariant( std::numeric_limits<float>::quiet_NaN() ) << false << true;
  QTest::newRow( "float rhs nan" ) << QVariant( std::numeric_limits<float>::quiet_NaN() ) << QVariant( 5.5f ) << true << false;
  QTest::newRow( "char" ) << QVariant( 'b' ) << QVariant( 'x' ) << true << false;
  QTest::newRow( "char 2" ) << QVariant( 'x' ) << QVariant( 'b' ) << false << true;
  QTest::newRow( "char equal" ) << QVariant( 'x' ) << QVariant( 'x' ) << false << false;
  QTest::newRow( "date" ) << QVariant( QDate( 2000, 5, 6 ) ) << QVariant( QDate( 2000, 8, 6 ) ) << true << false;
  QTest::newRow( "date 2" ) << QVariant( QDate( 2000, 8, 6 ) ) << QVariant( QDate( 2000, 5, 6 ) ) << false << true;
  QTest::newRow( "date equal" ) << QVariant( QDate( 2000, 8, 6 ) ) << QVariant( QDate( 2000, 8, 6 ) ) << false << false;
  QTest::newRow( "time" ) << QVariant( QTime( 13, 5, 6 ) ) << QVariant( QTime( 13, 8, 6 ) ) << true << false;
  QTest::newRow( "time 2" ) << QVariant( QTime( 18, 8, 6 ) ) << QVariant( QTime( 13, 5, 6 ) ) << false << true;
  QTest::newRow( "time equal" ) << QVariant( QTime( 18, 8, 6 ) ) << QVariant( QTime( 18, 8, 6 ) ) << false << false;
  QTest::newRow( "datetime" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 8, 6 ), QTime( 13, 5, 6 ) ) ) << true << false;
  QTest::newRow( "datetime 2" ) << QVariant( QDateTime( QDate( 2000, 8, 6 ), QTime( 13, 5, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << false << true;
  QTest::newRow( "datetime 3" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 9, 6 ) ) ) << true << false;
  QTest::newRow( "datetime 4" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 9, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << false << true;
  QTest::newRow( "datetime equal" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 9, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 9, 6 ) ) ) << false << false;
  QTest::newRow( "bool" ) << QVariant( false ) << QVariant( true ) << true << false;
  QTest::newRow( "bool 2" ) << QVariant( true ) << QVariant( false ) << false << true;
  QTest::newRow( "bool equal true" ) << QVariant( true ) << QVariant( true ) << false << false;
  QTest::newRow( "bool equal false" ) << QVariant( false ) << QVariant( false ) << false << false;
  QTest::newRow( "qvariantlist both empty" ) << QVariant( QVariantList() ) << QVariant( QVariantList() ) << false << false;
  QTest::newRow( "qvariantlist" ) << QVariant( QVariantList() << QVariant( 5 ) ) << QVariant( QVariantList() << QVariant( 9 ) ) << true << false;
  QTest::newRow( "qvariantlist 2" ) << QVariant( QVariantList() << QVariant( 9 ) ) << QVariant( QVariantList() << QVariant( 5 ) ) << false << true;
  QTest::newRow( "qvariantlist equal one element" ) << QVariant( QVariantList() << QVariant( 9 ) ) << QVariant( QVariantList() << QVariant( 9 ) ) << false << false;
  QTest::newRow( "qvariantlist 3" ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 3 ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << true << false;
  QTest::newRow( "qvariantlist 4" ) << QVariant( QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 3 ) ) << false << true;
  QTest::newRow( "qvariantlist equal two element" ) << QVariant( QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << false << false;
  QTest::newRow( "qvariantlist 5" ) << QVariant( QVariantList() << QVariant( 5 ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << true << false;
  QTest::newRow( "qvariantlist 5" ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << QVariant( QVariantList() << QVariant( 5 ) ) << false << true;
  QTest::newRow( "qstringlist empty" ) << QVariant( QStringList() ) << QVariant( QStringList() ) << false << false;
  QTest::newRow( "qstringlist" ) << QVariant( QStringList() << u"aa"_s ) << QVariant( QStringList() << u"bb"_s ) << true << false;
  QTest::newRow( "qstringlist 2" ) << QVariant( QStringList() << u"bb"_s ) << QVariant( QStringList() << u"aa"_s ) << false << true;
  QTest::newRow( "qstringlist equal one element" ) << QVariant( QStringList() << u"bb"_s ) << QVariant( QStringList() << u"bb"_s ) << false << false;
  QTest::newRow( "qstringlist 3" ) << QVariant( QStringList() << u"aa"_s << u"cc"_s ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << true << false;
  QTest::newRow( "qstringlist 4" ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << QVariant( QStringList() << u"aa"_s << u"cc"_s ) << false << true;
  QTest::newRow( "qstringlist equal two element" ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << false << false;
  QTest::newRow( "qstringlist 5" ) << QVariant( QStringList() << u"aa"_s ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << true << false;
  QTest::newRow( "qstringlist 6" ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << QVariant( QStringList() << u"aa"_s ) << false << true;
  QTest::newRow( "string both empty" ) << QVariant( QString() ) << QVariant( QString() ) << false << false;
  QTest::newRow( "string" ) << QVariant( "a b c" ) << QVariant( "d e f" ) << true << false;
  QTest::newRow( "string 2" ) << QVariant( "d e f" ) << QVariant( "a b c" ) << false << true;
  QTest::newRow( "string equal" ) << QVariant( "a b c" ) << QVariant( "a b c" ) << false << false;
}

void TestQgis::qVariantOperators()
{
  QFETCH( QVariant, lhs );
  QFETCH( QVariant, rhs );
  QFETCH( bool, lessThan );
  QFETCH( bool, greaterThan );

  QCOMPARE( lhs < rhs, lessThan );
  QCOMPARE( lhs > rhs, greaterThan );
}

void TestQgis::qVariantCompare_data()
{
  QTest::addColumn<QVariant>( "lhs" );
  QTest::addColumn<QVariant>( "rhs" );
  QTest::addColumn<bool>( "lessThan" );
  QTest::addColumn<bool>( "greaterThan" );
  QTest::addColumn<int>( "compare" );

  QTest::newRow( "both invalid" ) << QVariant() << QVariant() << false << false << 0;
  QTest::newRow( "invalid to value" ) << QVariant() << QVariant( 2 ) << true << false << -1;
  QTest::newRow( "invalid to value 2" ) << QVariant( 2 ) << QVariant() << false << true << 1;
  QTest::newRow( "invalid to null" ) << QVariant() << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << true << false << -1;
  QTest::newRow( "invalid to null2 " ) << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << QVariant() << false << true << 1;
  QTest::newRow( "both null" ) << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << false << false << 0;
  QTest::newRow( "null to value" ) << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << QVariant( "a" ) << true << false << -1;
  QTest::newRow( "null to value 2" ) << QVariant( "a" ) << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << false << true << 1;

  // type mismatches -- we DO compare the values here, and are tolerant to different variant types!
  QTest::newRow( "int vs double less than" ) << QVariant( 1 ) << QVariant( 2.0 ) << true << false << -1;
  QTest::newRow( "int vs double less than truncation" ) << QVariant( 1 ) << QVariant( 1.1 ) << true << false << -1;
  QTest::newRow( "int vs double greater than" ) << QVariant( 2 ) << QVariant( 1.0 ) << false << true << 1;
  QTest::newRow( "int vs double greater than truncation" ) << QVariant( -2 ) << QVariant( -2.1 ) << false << true << 1;
  QTest::newRow( "int vs double same" ) << QVariant( 1 ) << QVariant( 1.0 ) << false << false << 0;

  QTest::newRow( "int vs float less than" ) << QVariant( 1 ) << QVariant( 2.0f ) << true << false << -1;
  QTest::newRow( "int vs float less than truncation" ) << QVariant( 1 ) << QVariant( 1.1f ) << true << false << -1;
  QTest::newRow( "int vs float greater than" ) << QVariant( 2 ) << QVariant( 1.0f ) << false << true << 1;
  QTest::newRow( "int vs float greater than truncation" ) << QVariant( -2 ) << QVariant( -2.1f ) << false << true << 1;
  QTest::newRow( "int vs float same" ) << QVariant( 1 ) << QVariant( 1.0f ) << false << false << 0;

  QTest::newRow( "long long vs double less than" ) << QVariant( 1LL ) << QVariant( 2.0 ) << true << false << -1;
  QTest::newRow( "long long vs double less than truncation" ) << QVariant( 1LL ) << QVariant( 1.1 ) << true << false << -1;
  QTest::newRow( "long long vs double greater than" ) << QVariant( 2LL ) << QVariant( 1.0 ) << false << true << 1;
  QTest::newRow( "long long vs double greater than truncation" ) << QVariant( -2LL ) << QVariant( -2.1 ) << false << true << 1;
  QTest::newRow( "long long vs double same" ) << QVariant( 1LL ) << QVariant( 1.0 ) << false << false << 0;

  QTest::newRow( "long long vs float less than" ) << QVariant( 1LL ) << QVariant( 2.0f ) << true << false << -1;
  QTest::newRow( "long long vs float less than truncation" ) << QVariant( 1LL ) << QVariant( 1.1f ) << true << false << -1;
  QTest::newRow( "long long vs float greater than" ) << QVariant( 2LL ) << QVariant( 1.0f ) << false << true << 1;
  QTest::newRow( "long long vs float greater than truncation" ) << QVariant( -2LL ) << QVariant( -2.1f ) << false << true << 1;
  QTest::newRow( "long long vs float same" ) << QVariant( 1LL ) << QVariant( 1.0f ) << false << false << 0;

  QTest::newRow( "long long vs int less than" ) << QVariant( 1LL ) << QVariant( 2 ) << true << false << -1;
  QTest::newRow( "long long vs int greater than" ) << QVariant( 2LL ) << QVariant( 1 ) << false << true << 1;
  QTest::newRow( "long long vs int same" ) << QVariant( 1LL ) << QVariant( 1 ) << false << false << 0;
  QTest::newRow( "int vs long long less than" ) << QVariant( 1 ) << QVariant( 2LL ) << true << false << -1;
  QTest::newRow( "int vs long long greater than" ) << QVariant( 2 ) << QVariant( 1LL ) << false << true << 1;
  QTest::newRow( "int vs long long same" ) << QVariant( 1 ) << QVariant( 1LL ) << false << false << 0;

  QTest::newRow( "double vs int less than" ) << QVariant( 1.0 ) << QVariant( 2 ) << true << false << -1;
  QTest::newRow( "double vs int less than truncation" ) << QVariant( -2.1 ) << QVariant( -2 ) << true << false << -1;
  QTest::newRow( "double vs int greater than" ) << QVariant( 2.0 ) << QVariant( 1 ) << false << true << 1;
  QTest::newRow( "double vs int greater than truncation" ) << QVariant( 1.1 ) << QVariant( 1 ) << false << true << 1;
  QTest::newRow( "double vs int same" ) << QVariant( 1.0 ) << QVariant( 1 ) << false << false << 0;

  QTest::newRow( "double vs long long less than" ) << QVariant( 1.0 ) << QVariant( 2LL ) << true << false << -1;
  QTest::newRow( "double vs long long less than truncation" ) << QVariant( -2.1 ) << QVariant( -2LL ) << true << false << -1;
  QTest::newRow( "double vs long long greater than" ) << QVariant( 2.0 ) << QVariant( 1LL ) << false << true << 1;
  QTest::newRow( "double vs long long greater than truncation" ) << QVariant( 1.1 ) << QVariant( 1LL ) << false << true << 1;
  QTest::newRow( "double vs long long same" ) << QVariant( 1.0 ) << QVariant( 1LL ) << false << false << 0;

  QTest::newRow( "float vs int less than" ) << QVariant( 1.0f ) << QVariant( 2 ) << true << false << -1;
  QTest::newRow( "float vs int less than truncation" ) << QVariant( -2.1f ) << QVariant( -2 ) << true << false << -1;
  QTest::newRow( "float vs int greater than" ) << QVariant( 2.0f ) << QVariant( 1 ) << false << true << 1;
  QTest::newRow( "float vs int greater than truncation" ) << QVariant( 1.1f ) << QVariant( 1 ) << false << true << 1;
  QTest::newRow( "float vs int same" ) << QVariant( 1.0f ) << QVariant( 1 ) << false << false << 0;

  QTest::newRow( "float vs long long less than" ) << QVariant( 1.0f ) << QVariant( 2LL ) << true << false << -1;
  QTest::newRow( "float vs long long less than truncation" ) << QVariant( -2.1f ) << QVariant( -2LL ) << true << false << -1;
  QTest::newRow( "float vs long long greater than" ) << QVariant( 2.0f ) << QVariant( 1LL ) << false << true << 1;
  QTest::newRow( "float vs long long greater than truncation" ) << QVariant( 1.1f ) << QVariant( 1LL ) << false << true << 1;
  QTest::newRow( "float vs long long same" ) << QVariant( 1.0f ) << QVariant( 1LL ) << false << false << 0;

  QTest::newRow( "int vs string less than" ) << QVariant( 1 ) << QVariant( "2" ) << true << false << -1;
  QTest::newRow( "int vs string greater than" ) << QVariant( 2 ) << QVariant( "1" ) << false << true << 1;
  QTest::newRow( "int vs string same" ) << QVariant( 2 ) << QVariant( "2" ) << false << false << 0;
  QTest::newRow( "int vs string non numeric" ) << QVariant( 2 ) << QVariant( "aaaa" ) << true << false << -1;
  QTest::newRow( "non numeric string vs int" ) << QVariant( "abc" ) << QVariant( 2 ) << false << true << 1;
  QTest::newRow( "int 0 vs string non numeric" ) << QVariant( 0 ) << QVariant( "aaaa" ) << true << false << -1;
  QTest::newRow( "non numeric string vs int 0" ) << QVariant( "abc" ) << QVariant( 0 ) << false << true << 1;

  QTest::newRow( "long long vs string less than" ) << QVariant( 1LL ) << QVariant( "2" ) << true << false << -1;
  QTest::newRow( "long long vs string greater than" ) << QVariant( 2LL ) << QVariant( "1" ) << false << true << 1;
  QTest::newRow( "long long vs string same" ) << QVariant( 2LL ) << QVariant( "2" ) << false << false << 0;
  QTest::newRow( "long long vs string non numeric" ) << QVariant( 2LL ) << QVariant( "aaaa" ) << true << false << -1;
  QTest::newRow( "non numeric string vs long long" ) << QVariant( "abc" ) << QVariant( 2LL ) << false << true << 1;
  QTest::newRow( "long long 0 vs string non numeric" ) << QVariant( 0LL ) << QVariant( "aaaa" ) << true << false << -1;
  QTest::newRow( "non numeric string vs long long 0" ) << QVariant( "abc" ) << QVariant( 0LL ) << false << true << 1;

  QTest::newRow( "int vs double string less than" ) << QVariant( 1 ) << QVariant( "2.0" ) << true << false << -1;
  QTest::newRow( "int vs double string less than truncation" ) << QVariant( 1 ) << QVariant( "1.1" ) << true << false << -1;
  QTest::newRow( "int vs double string greater than" ) << QVariant( 2 ) << QVariant( "1.0" ) << false << true << 1;
  QTest::newRow( "int vs double string greater than truncation" ) << QVariant( -2 ) << QVariant( "-2.1" ) << false << true << 1;
  QTest::newRow( "int vs double string same" ) << QVariant( 2 ) << QVariant( "2.0" ) << false << false << 0;
  QTest::newRow( "long long vs double string less than" ) << QVariant( 1LL ) << QVariant( "2.0" ) << true << false << -1;
  QTest::newRow( "long long vs double string less than truncation" ) << QVariant( 1LL ) << QVariant( "1.1" ) << true << false << -1;
  QTest::newRow( "long long vs double string greater than" ) << QVariant( 2LL ) << QVariant( "1.0" ) << false << true << 1;
  QTest::newRow( "long long vs double string greater than truncation" ) << QVariant( -2LL ) << QVariant( "-2.1" ) << false << true << 1;
  QTest::newRow( "long long vs double string same" ) << QVariant( 2LL ) << QVariant( "2.0" ) << false << false << 0;
  QTest::newRow( "double vs non numeric string" ) << QVariant( 2.1 ) << QVariant( "abc" ) << true << false << -1;
  QTest::newRow( "double 0 vs non numeric string" ) << QVariant( 0 ) << QVariant( "abc" ) << true << false << -1;
  QTest::newRow( "non numeric string vs double" ) << QVariant( "abc" ) << QVariant( 2.0 ) << false << true << 1;
  QTest::newRow( "float vs non numeric string" ) << QVariant( 2.1f ) << QVariant( "abc" ) << true << false << -1;
  QTest::newRow( "float 0 vs non numeric string" ) << QVariant( 0.f ) << QVariant( "abc" ) << true << false << -1;
  QTest::newRow( "non numeric string vs float" ) << QVariant( "abc" ) << QVariant( 2.0f ) << false << true << 1;

  QTest::newRow( "string vs int less than" ) << QVariant( "1" ) << QVariant( 2 ) << true << false << -1;
  QTest::newRow( "string vs int less than truncation" ) << QVariant( "-2.1" ) << QVariant( -2 ) << true << false << -1;
  QTest::newRow( "string vs int greater than" ) << QVariant( "2" ) << QVariant( 1 ) << false << true << 1;
  QTest::newRow( "string vs int greater than truncation" ) << QVariant( "2.1" ) << QVariant( 2 ) << false << true << 1;
  QTest::newRow( "string vs int same" ) << QVariant( "2" ) << QVariant( 2 ) << false << false << 0;
  QTest::newRow( "string double vs int same" ) << QVariant( "2.0" ) << QVariant( 2 ) << false << false << 0;

  QTest::newRow( "string vs long long less than" ) << QVariant( "1" ) << QVariant( 2LL ) << true << false << -1;
  QTest::newRow( "string vs long long less than truncation" ) << QVariant( "-2.1" ) << QVariant( -2LL ) << true << false << -1;
  QTest::newRow( "string vs long long greater than" ) << QVariant( "2" ) << QVariant( 1LL ) << false << true << 1;
  QTest::newRow( "string vs long long greater than truncation" ) << QVariant( "2.1" ) << QVariant( 2LL ) << false << true << 1;
  QTest::newRow( "string vs long long same" ) << QVariant( "2" ) << QVariant( 2LL ) << false << false << 0;
  QTest::newRow( "string double vs long long same" ) << QVariant( "2.0" ) << QVariant( 2LL ) << false << false << 0;

  QTest::newRow( "string vs double same" ) << QVariant( "2" ) << QVariant( 2.0 ) << false << false << 0;
  QTest::newRow( "string vs double less than" ) << QVariant( "1" ) << QVariant( 2.0 ) << true << false << -1;
  QTest::newRow( "string vs double less than truncation" ) << QVariant( "1" ) << QVariant( 1.1 ) << true << false << -1;
  QTest::newRow( "string vs double greater than" ) << QVariant( "3" ) << QVariant( 2.0 ) << false << true << 1;
  QTest::newRow( "string vs double greater than truncation" ) << QVariant( "-3" ) << QVariant( -3.1 ) << false << true << 1;
  QTest::newRow( "string double vs double same" ) << QVariant( "2.1" ) << QVariant( 2.1 ) << false << false << 0;
  QTest::newRow( "string double vs double less than" ) << QVariant( "2.05" ) << QVariant( 2.1 ) << true << false << -1;
  QTest::newRow( "string double vs double greater than" ) << QVariant( "2.15" ) << QVariant( 2.1 ) << false << true << 1;
  QTest::newRow( "string vs float same" ) << QVariant( "2" ) << QVariant( 2.0f ) << false << false << 0;
  QTest::newRow( "string double vs float less than" ) << QVariant( "2.05" ) << QVariant( 2.1f ) << true << false << -1;
  QTest::newRow( "string double vs float greater than" ) << QVariant( "2.15" ) << QVariant( 2.1f ) << false << true << 1;
  QTest::newRow( "string double vs float" ) << QVariant( "2.1" ) << QVariant( 2.2f ) << true << false << -1;

  QTest::newRow( "int" ) << QVariant( 1 ) << QVariant( 2 ) << true << false << -1;
  QTest::newRow( "int 2" ) << QVariant( 1 ) << QVariant( -2 ) << false << true << 1;
  QTest::newRow( "int 3" ) << QVariant( 0 ) << QVariant( 1 ) << true << false << -1;
  QTest::newRow( "int equal" ) << QVariant( 1 ) << QVariant( 1 ) << false << false << 0;
  QTest::newRow( "uint" ) << QVariant( 1u ) << QVariant( 2u ) << true << false << -1;
  QTest::newRow( "uint 2" ) << QVariant( 2u ) << QVariant( 0u ) << false << true << 1;
  QTest::newRow( "uint equal" ) << QVariant( 2u ) << QVariant( 2u ) << false << false << 0;
  QTest::newRow( "long long" ) << QVariant( 1LL ) << QVariant( 2LL ) << true << false << -1;
  QTest::newRow( "long long 2" ) << QVariant( 1LL ) << QVariant( -2LL ) << false << true << 1;
  QTest::newRow( "long long 3" ) << QVariant( 0LL ) << QVariant( 1LL ) << true << false << -1;
  QTest::newRow( "long long equal" ) << QVariant( 1LL ) << QVariant( 1LL ) << false << false << 0;
  QTest::newRow( "ulong long" ) << QVariant( 1uLL ) << QVariant( 2uLL ) << true << false << -1;
  QTest::newRow( "ulong long 2" ) << QVariant( 2uLL ) << QVariant( 0uLL ) << false << true << 1;
  QTest::newRow( "ulong long equal" ) << QVariant( 2uLL ) << QVariant( 2uLL ) << false << false << 0;
  QTest::newRow( "double" ) << QVariant( 1.5 ) << QVariant( 2.5 ) << true << false << -1;
  QTest::newRow( "double 2" ) << QVariant( 1.5 ) << QVariant( -2.5 ) << false << true << 1;
  QTest::newRow( "double 3" ) << QVariant( 0.5 ) << QVariant( 1.5 ) << true << false << -1;
  QTest::newRow( "double equal" ) << QVariant( 1.5 ) << QVariant( 1.5 ) << false << false << 0;
  QTest::newRow( "double both nan" ) << QVariant( std::numeric_limits<double>::quiet_NaN() ) << QVariant( std::numeric_limits<double>::quiet_NaN() ) << false << false << 0;
  QTest::newRow( "double lhs nan" ) << QVariant( 5.5 ) << QVariant( std::numeric_limits<double>::quiet_NaN() ) << false << true << 1;
  QTest::newRow( "double rhs nan" ) << QVariant( std::numeric_limits<double>::quiet_NaN() ) << QVariant( 5.5 ) << true << false << -1;
  QTest::newRow( "float" ) << QVariant( 1.5f ) << QVariant( 2.5f ) << true << false << -1;
  QTest::newRow( "float 2" ) << QVariant( 1.5f ) << QVariant( -2.5f ) << false << true << 1;
  QTest::newRow( "float 3" ) << QVariant( 0.5f ) << QVariant( 1.5f ) << true << false << -1;
  QTest::newRow( "float equal" ) << QVariant( 1.5f ) << QVariant( 1.5f ) << false << false << 0;
  QTest::newRow( "float both nan" ) << QVariant( std::numeric_limits<float>::quiet_NaN() ) << QVariant( std::numeric_limits<float>::quiet_NaN() ) << false << false << 0;
  QTest::newRow( "float lhs nan" ) << QVariant( 5.5f ) << QVariant( std::numeric_limits<float>::quiet_NaN() ) << false << true << 1;
  QTest::newRow( "float rhs nan" ) << QVariant( std::numeric_limits<float>::quiet_NaN() ) << QVariant( 5.5f ) << true << false << -1;
  QTest::newRow( "char" ) << QVariant( 'b' ) << QVariant( 'x' ) << true << false << -1;
  QTest::newRow( "char 2" ) << QVariant( 'x' ) << QVariant( 'b' ) << false << true << 1;
  QTest::newRow( "char equal" ) << QVariant( 'x' ) << QVariant( 'x' ) << false << false << 0;
  QTest::newRow( "date" ) << QVariant( QDate( 2000, 5, 6 ) ) << QVariant( QDate( 2000, 8, 6 ) ) << true << false << -1;
  QTest::newRow( "date 2" ) << QVariant( QDate( 2000, 8, 6 ) ) << QVariant( QDate( 2000, 5, 6 ) ) << false << true << 1;
  QTest::newRow( "date equal" ) << QVariant( QDate( 2000, 8, 6 ) ) << QVariant( QDate( 2000, 8, 6 ) ) << false << false << 0;
  QTest::newRow( "time" ) << QVariant( QTime( 13, 5, 6 ) ) << QVariant( QTime( 13, 8, 6 ) ) << true << false << -1;
  QTest::newRow( "time 2" ) << QVariant( QTime( 18, 8, 6 ) ) << QVariant( QTime( 13, 5, 6 ) ) << false << true << 1;
  QTest::newRow( "time equal" ) << QVariant( QTime( 18, 8, 6 ) ) << QVariant( QTime( 18, 8, 6 ) ) << false << false << 0;
  QTest::newRow( "datetime" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 8, 6 ), QTime( 13, 5, 6 ) ) ) << true << false << -1;
  QTest::newRow( "datetime 2" ) << QVariant( QDateTime( QDate( 2000, 8, 6 ), QTime( 13, 5, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << false << true << 1;
  QTest::newRow( "datetime 3" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 9, 6 ) ) ) << true << false << -1;
  QTest::newRow( "datetime 4" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 9, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 5, 6 ) ) ) << false << true << 1;
  QTest::newRow( "datetime equal" ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 9, 6 ) ) ) << QVariant( QDateTime( QDate( 2000, 5, 6 ), QTime( 13, 9, 6 ) ) ) << false << false << 0;
  QTest::newRow( "bool" ) << QVariant( false ) << QVariant( true ) << true << false << -1;
  QTest::newRow( "bool 2" ) << QVariant( true ) << QVariant( false ) << false << true << 1;
  QTest::newRow( "bool equal true" ) << QVariant( true ) << QVariant( true ) << false << false << 0;
  QTest::newRow( "bool equal false" ) << QVariant( false ) << QVariant( false ) << false << false << 0;
  QTest::newRow( "qvariantlist both empty" ) << QVariant( QVariantList() ) << QVariant( QVariantList() ) << false << false << 0;
  QTest::newRow( "qvariantlist" ) << QVariant( QVariantList() << QVariant( 5 ) ) << QVariant( QVariantList() << QVariant( 9 ) ) << true << false << -1;
  QTest::newRow( "qvariantlist 2" ) << QVariant( QVariantList() << QVariant( 9 ) ) << QVariant( QVariantList() << QVariant( 5 ) ) << false << true << 1;
  QTest::newRow( "qvariantlist equal one element" ) << QVariant( QVariantList() << QVariant( 9 ) ) << QVariant( QVariantList() << QVariant( 9 ) ) << false << false << 0;
  QTest::newRow( "qvariantlist 3" ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 3 ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << true << false << -1;
  QTest::newRow( "qvariantlist 4" ) << QVariant( QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 3 ) ) << false << true << 1;
  QTest::newRow( "qvariantlist equal two element" ) << QVariant( QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << false << false << 0;
  QTest::newRow( "qvariantlist 5" ) << QVariant( QVariantList() << QVariant( 5 ) ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << true << false << -1;
  QTest::newRow( "qvariantlist 5" ) << QVariant( QVariantList() << QVariant( 5 ) << QVariant( 6 ) ) << QVariant( QVariantList() << QVariant( 5 ) ) << false << true << 1;
  QTest::newRow( "qstringlist empty" ) << QVariant( QStringList() ) << QVariant( QStringList() ) << false << false << 0;
  QTest::newRow( "qstringlist" ) << QVariant( QStringList() << u"aa"_s ) << QVariant( QStringList() << u"bb"_s ) << true << false << -1;
  QTest::newRow( "qstringlist 2" ) << QVariant( QStringList() << u"bb"_s ) << QVariant( QStringList() << u"aa"_s ) << false << true << 1;
  QTest::newRow( "qstringlist equal one element" ) << QVariant( QStringList() << u"bb"_s ) << QVariant( QStringList() << u"bb"_s ) << false << false << 0;
  QTest::newRow( "qstringlist 3" ) << QVariant( QStringList() << u"aa"_s << u"cc"_s ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << true << false << -1;
  QTest::newRow( "qstringlist 4" ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << QVariant( QStringList() << u"aa"_s << u"cc"_s ) << false << true << 1;
  QTest::newRow( "qstringlist equal two element" ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << false << false << 0;
  QTest::newRow( "qstringlist 5" ) << QVariant( QStringList() << u"aa"_s ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << true << false << -1;
  QTest::newRow( "qstringlist 6" ) << QVariant( QStringList() << u"aa"_s << u"xx"_s ) << QVariant( QStringList() << u"aa"_s ) << false << true << 1;
  QTest::newRow( "string both empty" ) << QVariant( QString() ) << QVariant( QString() ) << false << false << 0;
  QTest::newRow( "string" ) << QVariant( "a b c" ) << QVariant( "d e f" ) << true << false << -1;
  QTest::newRow( "string 2" ) << QVariant( "d e f" ) << QVariant( "a b c" ) << false << true << 1;
  QTest::newRow( "string equal" ) << QVariant( "a b c" ) << QVariant( "a b c" ) << false << false << 0;
}

void TestQgis::qVariantCompare()
{
  QFETCH( QVariant, lhs );
  QFETCH( QVariant, rhs );
  QFETCH( bool, lessThan );
  QFETCH( bool, greaterThan );
  QFETCH( int, compare );

  QCOMPARE( qgsVariantLessThan( lhs, rhs ), lessThan );
  QCOMPARE( qgsVariantGreaterThan( lhs, rhs ), greaterThan );
  QCOMPARE( qgsVariantCompare( lhs, rhs ), compare );
}

void TestQgis::testNanCompatibleEquals_data()
{
  QTest::addColumn<double>( "lhs" );
  QTest::addColumn<double>( "rhs" );
  QTest::addColumn<bool>( "expected" );

  QTest::newRow( "both nan" ) << std::numeric_limits<double>::quiet_NaN() << std::numeric_limits<double>::quiet_NaN() << true;
  QTest::newRow( "first is nan" ) << std::numeric_limits<double>::quiet_NaN() << 5.0 << false;
  QTest::newRow( "second is nan" ) << 5.0 << std::numeric_limits<double>::quiet_NaN() << false;
  QTest::newRow( "two numbers, not equal" ) << 5.0 << 6.0 << false;
  QTest::newRow( "two numbers, equal" ) << 5.0 << 5.0 << true;
}

void TestQgis::testNanCompatibleEquals()
{
  QFETCH( double, lhs );
  QFETCH( double, rhs );
  QFETCH( bool, expected );

  QCOMPARE( qgsNanCompatibleEquals( lhs, rhs ), expected );
  QCOMPARE( qgsNanCompatibleEquals( rhs, lhs ), expected );
}

class ConstTester
{
  public:
    void doSomething()
    {
      mVal = 1;
    }

    void doSomething() const
    {
      mVal = 2;
    }

    mutable int mVal = 0;
};

void TestQgis::testQgsAsConst()
{
  ConstTester ct;
  ct.doSomething();
  QCOMPARE( ct.mVal, 1 );
  std::as_const( ct ).doSomething();
  QCOMPARE( ct.mVal, 2 );
}

void TestQgis::testQgsRound()
{
  QGSCOMPARENEAR( qgsRound( 1234.567, 2 ), 1234.57, 0.01 );
  QGSCOMPARENEAR( qgsRound( -1234.567, 2 ), -1234.57, 0.01 );
  QGSCOMPARENEAR( qgsRound( 98765432198, 8 ), 98765432198, 1.0 );
  QGSCOMPARENEAR( qgsRound( 98765432198, 9 ), 98765432198, 1.0 );
  QGSCOMPARENEAR( qgsRound( 98765432198, 10 ), 98765432198, 1.0 );
  QGSCOMPARENEAR( qgsRound( 98765432198, 11 ), 98765432198, 1.0 );
  QGSCOMPARENEAR( qgsRound( 98765432198, 12 ), 98765432198, 1.0 );
  QGSCOMPARENEAR( qgsRound( 98765432198, 13 ), 98765432198, 1.0 );
  QGSCOMPARENEAR( qgsRound( 98765432198, 14 ), 98765432198, 1.0 );
  QGSCOMPARENEAR( qgsRound( 98765432198765, 14 ), 98765432198765, 1.0 );
  QGSCOMPARENEAR( qgsRound( 98765432198765432., 20 ), 98765432198765432., 1.0 );
  QGSCOMPARENEAR( qgsRound( 9.8765432198765, 2 ), 9.88, 0.001 );
  QGSCOMPARENEAR( qgsRound( 9.8765432198765, 3 ), 9.877, 0.0001 );
  QGSCOMPARENEAR( qgsRound( 9.8765432198765, 4 ), 9.8765, 0.00001 );
  QGSCOMPARENEAR( qgsRound( 9.8765432198765, 5 ), 9.87654, 0.000001 );
  QGSCOMPARENEAR( qgsRound( 9.8765432198765, 6 ), 9.876543, 0.0000001 );
  QGSCOMPARENEAR( qgsRound( 9.8765432198765, 7 ), 9.8765432, 0.00000001 );
  QGSCOMPARENEAR( qgsRound( -9.8765432198765, 7 ), -9.8765432, 0.0000001 );
  QGSCOMPARENEAR( qgsRound( 9876543.2198765, 5 ), 9876543.219880, 0.000001 );
  QGSCOMPARENEAR( qgsRound( -9876543.2198765, 5 ), -9876543.219880, 0.000001 );
  QGSCOMPARENEAR( qgsRound( 9.87654321987654321, 13 ), 9.87654321987654, 0.0000000000001 );
  QGSCOMPARENEAR( qgsRound( 9.87654321987654321, 14 ), 9.876543219876543, 0.00000000000001 );
  QGSCOMPARENEAR( qgsRound( 9998.87654321987654321, 14 ), 9998.876543219876543, 0.00000000000001 );
  QGSCOMPARENEAR( qgsRound( 9999999.87654321987654321, 14 ), 9999999.876543219876543, 0.00000000000001 );
}

void TestQgis::testQgsVariantEqual()
{
  // Invalid
  QVERIFY( qgsVariantEqual( QVariant(), QVariant() ) );
  QVERIFY( QVariant() == QVariant() );

  // Zero
  QVERIFY( qgsVariantEqual( QVariant( 0 ), QVariant( 0.0f ) ) );
  QVERIFY( QVariant( 0 ) == QVariant( 0.0f ) );

  // Double
  QVERIFY( qgsVariantEqual( QVariant( 1.234 ), QVariant( 1.234 ) ) );

  // This is what we actually wanted to fix with qgsVariantEqual
  // zero != NULL
  QVERIFY( !qgsVariantEqual( QVariant( 0 ), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) ) );
  QVERIFY( !qgsVariantEqual( QVariant( 0 ), QgsVariantUtils::createNullVariant( QMetaType::Type::Double ) ) );
  QVERIFY( !qgsVariantEqual( QVariant( 0.0f ), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) ) );
  QVERIFY( !qgsVariantEqual( QVariant( 0.0f ), QgsVariantUtils::createNullVariant( QMetaType::Type::Double ) ) );
  QVERIFY( QVariant( 0 ) == QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );

  // NULL identities
  QVERIFY( qgsVariantEqual( QgsVariantUtils::createNullVariant( QMetaType::Type::Int ), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) ) );
  QVERIFY( qgsVariantEqual( QgsVariantUtils::createNullVariant( QMetaType::Type::Double ), QgsVariantUtils::createNullVariant( QMetaType::Type::Double ) ) );
  QVERIFY( qgsVariantEqual( QgsVariantUtils::createNullVariant( QMetaType::Type::Int ), QgsVariantUtils::createNullVariant( QMetaType::Type::Double ) ) );
  QVERIFY( qgsVariantEqual( QgsVariantUtils::createNullVariant( QMetaType::Type::Int ), QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) ) );

  // NULL should not be equal to invalid
  QVERIFY( !qgsVariantEqual( QVariant(), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) ) );

  // string
  QVERIFY( qgsVariantEqual( QString( "" ), QString( "" ) ) );
  QVERIFY( qgsVariantEqual( QString(), QString() ) );
  QVERIFY( !qgsVariantEqual( QString( "" ), QString() ) );
  QVERIFY( !qgsVariantEqual( QString(), QString( "" ) ) );
  QVERIFY( !qgsVariantEqual( QString( "abc" ), QString() ) );
  QVERIFY( !qgsVariantEqual( QString(), QString( "abc" ) ) );
  QVERIFY( !qgsVariantEqual( QString( "abc" ), QString( "" ) ) );
  QVERIFY( !qgsVariantEqual( QString( "" ), QString( "abc" ) ) );
  QVERIFY( !qgsVariantEqual( QString( "def" ), QString( "abc" ) ) );
  QVERIFY( qgsVariantEqual( QString( "abc" ), QString( "abc" ) ) );
}

void TestQgis::testQgsEnumMapList()
{
  QCOMPARE( qgsEnumList<TestEnum>(), QList<TestEnum>( { TestEnum::TestEnum1, TestEnum::TestEnum2, TestEnum::TestEnum3 } ) );
  QCOMPARE( qgsEnumMap<TestEnum>().keys(), QList<TestEnum>( { TestEnum::TestEnum1, TestEnum::TestEnum2, TestEnum::TestEnum3 } ) );
  QCOMPARE( qgsEnumMap<TestEnum>().values(), QStringList( { u"TestEnum1"_s, u"TestEnum2"_s, u"TestEnum3"_s } ) );
}


void TestQgis::testQgsEnumValueToKey()
{
  bool ok = false;
  QgsMapLayerModel::CustomRole value = QgsMapLayerModel::CustomRole::Layer;
  QgsMapLayerModel::CustomRole badValue = static_cast<QgsMapLayerModel::CustomRole>( -1 );
  QMetaEnum metaEnum = QMetaEnum::fromType<QgsMapLayerModel::CustomRole>();
  QVERIFY( !metaEnum.valueToKey( static_cast<int>( badValue ) ) );
  QCOMPARE( qgsEnumValueToKey( value, &ok ), u"Layer"_s );
  QCOMPARE( ok, true );
  QCOMPARE( qgsEnumValueToKey( badValue, &ok ), QString() );
  QCOMPARE( ok, false );
}
void TestQgis::testQgsEnumKeyToValue()
{
  bool ok = false;
  QgsMapLayerModel::CustomRole defaultValue = QgsMapLayerModel::CustomRole::LayerId;
  QCOMPARE( qgsEnumKeyToValue( u"Additional"_s, defaultValue, false, &ok ), QgsMapLayerModel::CustomRole::Additional );
  QCOMPARE( ok, true );
  QCOMPARE( qgsEnumKeyToValue( u"UnknownKey"_s, defaultValue, false, &ok ), defaultValue );
  QCOMPARE( ok, false );
  QCOMPARE( qgsEnumKeyToValue( u"UnknownKey"_s, defaultValue, true, &ok ), defaultValue );
  QCOMPARE( ok, false );

  // try with int values as string keys
  QCOMPARE( qgsEnumKeyToValue( QString::number( static_cast<int>( QgsMapLayerModel::CustomRole::Additional ) ), defaultValue, true, &ok ), QgsMapLayerModel::CustomRole::Additional );
  QCOMPARE( ok, true );
  QCOMPARE( qgsEnumKeyToValue( QString::number( static_cast<int>( QgsMapLayerModel::CustomRole::Additional ) ), defaultValue, false, &ok ), defaultValue );
  QCOMPARE( ok, false );
  // also try with an invalid int value
  QMetaEnum metaEnum = QMetaEnum::fromType<QgsMapLayerModel::CustomRole>();
  int invalidValue = static_cast<int>( defaultValue ) + 7894563;
  QVERIFY( !metaEnum.valueToKey( invalidValue ) );
  QCOMPARE( qgsEnumKeyToValue( QString::number( invalidValue ), defaultValue, true, &ok ), defaultValue );
  QCOMPARE( ok, false );
}

void TestQgis::testQgsFlagValueToKeys()
{
  bool ok = false;
  QgsFieldProxyModel::Filters filters = QgsFieldProxyModel::Filter::String | QgsFieldProxyModel::Filter::Double;
  QCOMPARE( qgsFlagValueToKeys( filters, &ok ), u"String|Double"_s );
  QCOMPARE( ok, true );
  QCOMPARE( qgsFlagValueToKeys( QgsFieldProxyModel::Filters( -10 ), &ok ), QString() );
  QCOMPARE( ok, false );
}

void TestQgis::testQgsFlagKeysToValue()
{
  QgsFieldProxyModel::Filters defaultValue( QgsFieldProxyModel::Filter::AllTypes );
  QgsFieldProxyModel::Filters newValue( QgsFieldProxyModel::Filter::String | QgsFieldProxyModel::Filter::Double );

  bool ok = true;
  QCOMPARE( qgsFlagKeysToValue( QString(), defaultValue, false, &ok ), defaultValue );
  QCOMPARE( ok, false );

  QCOMPARE( qgsFlagKeysToValue( u"String|Double"_s, defaultValue, false, &ok ), newValue );
  QCOMPARE( ok, true );
  QCOMPARE( qgsFlagKeysToValue( u"UnknownKey"_s, defaultValue, false, &ok ), defaultValue );
  QCOMPARE( ok, false );
  QCOMPARE( qgsFlagKeysToValue( u"UnknownKey"_s, defaultValue, true, &ok ), defaultValue );
  QCOMPARE( ok, false );

  // try with int values as string keys
  QCOMPARE( qgsFlagKeysToValue( QString::number( newValue ), defaultValue, false, &ok ), defaultValue );
  QCOMPARE( ok, false );
  QCOMPARE( qgsFlagKeysToValue( QString::number( newValue ), defaultValue, true, &ok ), newValue );
  QCOMPARE( ok, true );
  // also try with an invalid int value
  QCOMPARE( qgsFlagKeysToValue( QString::number( -1 ), defaultValue, true, &ok ), defaultValue );
  QCOMPARE( ok, false );
}

void TestQgis::testQMapQVariantList()
{
  QMap<QVariantList, long> ids;
  ids.insert( QVariantList() << "B" << "c", 5 );
  ids.insert( QVariantList() << "b" << "C", 7 );

  QVariantList v = QVariantList() << "b" << "C";
  QMap<QVariantList, long>::const_iterator it = ids.constFind( v );

  QVERIFY( it != ids.constEnd() );
  QCOMPARE( it.value(), 7L );

  v = QVariantList() << "B" << "c";
  it = ids.constFind( v );

  QVERIFY( it != ids.constEnd() );
  QCOMPARE( it.value(), 5L );
}

void TestQgis::testQgsMapJoin()
{
  QMap<QString, int> map;

  map.insert( "tutu", 3 );
  map.insert( "titi", 4 );
  map.insert( "tata", 5 );

  QString res = qgsMapJoinValues( map, u", "_s );

  QRegularExpression re( "[3|4|5], [3|4|5], [3|4|5]" );
  QVERIFY( re.match( res ).hasMatch() );
  QVERIFY( res.contains( "3" ) );
  QVERIFY( res.contains( "4" ) );
  QVERIFY( res.contains( "5" ) );

  res = qgsMapJoinKeys( map, u", "_s );

  re.setPattern( "(tutu|titi|tata), (tutu|titi|tata), (tutu|titi|tata)" );
  QVERIFY( re.match( res ).hasMatch() );
  QVERIFY( res.contains( "tutu" ) );
  QVERIFY( res.contains( "titi" ) );
  QVERIFY( res.contains( "tata" ) );
}

void TestQgis::testQgsSetJoin()
{
  QSet<int> set;

  set.insert( 3 );
  set.insert( 4 );
  set.insert( 4 );
  set.insert( 5 );

  const QString res = qgsSetJoin( set, u", "_s );

  const thread_local QRegularExpression re( "[3|4|5], [3|4|5], [3|4|5]" );
  QVERIFY( re.match( res ).hasMatch() );
  QVERIFY( res.contains( "3" ) );
  QVERIFY( res.contains( "4" ) );
  QVERIFY( res.contains( "5" ) );
}


QGSTEST_MAIN( TestQgis )
#include "testqgis.moc"
