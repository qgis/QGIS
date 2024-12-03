/***************************************************************************
    testqgsdoublespinbox.cpp
     --------------------------------------
    Date                 : June 2020
    Copyright            : (C) 2020 Sebastien Peillet
    Email                : sebastien dot peillet at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include "qgsdoublevalidator.h"
#include <QLineEdit>

class TestQgsDoubleValidator : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void validate();
    void validate_data();
    void toDouble_data();
    void toDouble();

  private:
};

void TestQgsDoubleValidator::initTestCase()
{
}

void TestQgsDoubleValidator::cleanupTestCase()
{
}

void TestQgsDoubleValidator::init()
{
}

void TestQgsDoubleValidator::cleanup()
{
}

void TestQgsDoubleValidator::validate_data()
{
  QTest::addColumn<QString>( "actualState" );
  QTest::addColumn<int>( "expState" );
  QTest::addColumn<bool>( "negative" );

  QTest::newRow( "C decimal" ) << QString( "4cd6" ) << int( QValidator::Acceptable ) << false;
  QTest::newRow( "locale decimal" ) << QString( "4ld6" ) << int( QValidator::Acceptable ) << false;
  QTest::newRow( "locale decimal" ) << QString( "4444ld6" ) << int( QValidator::Acceptable ) << false;

  QTest::newRow( "C negative C decimal" ) << QString( "cn4cd6" ) << int( QValidator::Acceptable ) << true;
  QTest::newRow( "locale negative locale decimal" ) << QString( "ln4ld6" ) << int( QValidator::Acceptable ) << true;
  QTest::newRow( "locale negative locale decimal" ) << QString( "ln4444ld6" ) << int( QValidator::Acceptable ) << true;

  QTest::newRow( "positive sign C decimal" ) << QString( "+4cd6" ) << int( QValidator::Acceptable ) << false;

  QTest::newRow( "exponent <e> C negative" ) << QString( "44446ecn1" ) << int( QValidator::Acceptable ) << false;
  QTest::newRow( "exponent <e> locale negative" ) << QString( "44446eln1" ) << int( QValidator::Acceptable ) << false;

#if ( QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 ) ) || ( QT_VERSION >= QT_VERSION_CHECK( 6, 5, 2 ) ) // https://bugreports.qt.io/browse/QTBUG-113443
  QTest::newRow( "locale decimal exponent <E> positive" ) << QString( "444ld46E1" ) << int( QValidator::Acceptable ) << false;
  QTest::newRow( "locale decimal exponent <E> positive sign" ) << QString( "444ld46E+1" ) << int( QValidator::Acceptable ) << false;
#endif

  QTest::newRow( "locale exponent locale negative" ) << QString( "4446leln1" ) << int( QValidator::Acceptable ) << false;


  // QgsDoubleValidator doesn't expect group separator but it tolerates it,
  // so the result will be QValidator::Intermediate and not QValidator::Acceptable
  QTest::newRow( "locale group separator + locale decimal" ) << QString( "4lg444ld6" ) << int( QValidator::Intermediate ) << false;
  QTest::newRow( "locale group separator misplaced + locale decimal" ) << QString( "44lg44ld6" ) << int( QValidator::Intermediate ) << false;
  QTest::newRow( "locale group separator + c decimal" ) << QString( "4lg444cd6" ) << int( QValidator::Invalid ) << false;
  QTest::newRow( "c group separator + locale decimal" ) << QString( "4cg444ld6" ) << int( QValidator::Invalid ) << false;
  QTest::newRow( "c group separator + c decimal" ) << QString( "4cg444cd6" ) << int( QValidator::Intermediate ) << false;

  QTest::newRow( "outside the range + local decimal" ) << QString( "3ld6" ) << int( QValidator::Intermediate ) << false;
  QTest::newRow( "outside the range + c decimal" ) << QString( "3cd6" ) << int( QValidator::Intermediate ) << false;
  QTest::newRow( "string" ) << QString( "string" ) << int( QValidator::Invalid ) << false;
}

void TestQgsDoubleValidator::toDouble_data()
{
  QTest::addColumn<QString>( "actualValue" );
  QTest::addColumn<double>( "expValue" );

  QTest::newRow( "C decimal" ) << QString( "4cd6" ) << 4.6;
  QTest::newRow( "locale decimal" ) << QString( "4ld6" ) << 4.6;
  QTest::newRow( "locale decimal" ) << QString( "4444ld6" ) << 4444.6;

  QTest::newRow( "C negative C decimal" ) << QString( "cn4cd6" ) << -4.6;
  QTest::newRow( "locale negative locale decimal" ) << QString( "ln4ld6" ) << -4.6;
  QTest::newRow( "locale negative locale decimal" ) << QString( "ln4444ld6" ) << -4444.6;

  QTest::newRow( "positive sign C decimal" ) << QString( "+4cd6" ) << 4.6;

  QTest::newRow( "exponent <e> C negative" ) << QString( "44446ecn1" ) << 4444.6;
  QTest::newRow( "exponent <e> locale negative" ) << QString( "44446eln1" ) << 4444.6;

#if ( QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 ) ) || ( QT_VERSION >= QT_VERSION_CHECK( 6, 5, 2 ) ) // https://bugreports.qt.io/browse/QTBUG-113443
  QTest::newRow( "locale decimal exponent <E> positive" ) << QString( "444ld46E1" ) << 4444.6;
  QTest::newRow( "locale decimal exponent <E> positive sign" ) << QString( "444ld46E+1" ) << 4444.6;
#endif

  QTest::newRow( "locale exponent locale negative" ) << QString( "44446leln1" ) << 4444.6;

  // QgsDoubleValidator doesn't expect group separator but it tolerates it,
  // so the result will be QValidator::Intermediate and not QValidator::Acceptable
  QTest::newRow( "locale group separator + locale decimal" ) << QString( "4lg444ld6" ) << 4444.6;
  QTest::newRow( "locale group separator misplaced + locale decimal" ) << QString( "4lg444ld6" ) << 4444.6;
  QTest::newRow( "locale group separator + c decimal" ) << QString( "4lg444cd6" ) << 0.0;
  QTest::newRow( "c group separator + locale decimal" ) << QString( "4cg444ld6" ) << 0.0;
  QTest::newRow( "c group separator + c decimal" ) << QString( "4cg444cd6" ) << 4444.6;

  QTest::newRow( "outside the range + local decimal" ) << QString( "3ld6" ) << 3.6;
  QTest::newRow( "outside the range + c decimal" ) << QString( "3cd6" ) << 3.6;
  QTest::newRow( "string" ) << QString( "string" ) << 0.0;
}

void TestQgsDoubleValidator::validate()
{
  QLineEdit *lineEdit = new QLineEdit();

  QFETCH( QString, actualState );
  QFETCH( int, expState );
  QFETCH( bool, negative );
  QString value;
  int expectedValue;

  const QVector<QLocale> listLocale( { QLocale::English, QLocale::French, QLocale::German, QLocale::Italian, QLocale::NorwegianBokmal, QLocale::Ukrainian } );
  QLocale loc;
  for ( int i = 0; i < listLocale.count(); ++i )
  {
    loc = listLocale.at( i );
    QLocale::setDefault( loc );

    QgsDoubleValidator *validator = new QgsDoubleValidator( lineEdit );
    if ( negative )
      validator->setRange( -10000, -4 );
    else
      validator->setRange( 4, 10000 );
    validator->setLocale( loc );
    lineEdit->setValidator( validator );

    value = actualState;
    value = value.replace( "ld", QLocale().decimalPoint() )
              .replace( "cd", QLocale( QLocale::C ).decimalPoint() )
              .replace( "lg", QLocale().groupSeparator() )
              .replace( "cg", QLocale( QLocale::C ).groupSeparator() )
              .replace( "ln", QLocale().negativeSign() )
              .replace( "cn", QLocale( QLocale::C ).negativeSign() )
              .replace( "le", QLocale().exponential() );
    expectedValue = expState;
    // if the local group separator / decimal point is equal to the C one,
    // expected result will be different for double with test with mixed
    // local/C characters.
    // Example with lg as local group separator
    //              cg as C group separator
    //              ld as local decimal point
    //              cd as C decimal point
    // for 4cg444ld6 double, if cg == lg then 4cg444ld6 == 4lg444ld6
    //                       and validator->validate(4lg444ld6) == 1 and not 0
    // for 4cg444ld6 double, if cd == ld then 4cg444ld6 == 4cg444cd6
    //                       and validator->validate(4cg444cd6) == 1 and not 0
    if ( ( QLocale( QLocale::C ).groupSeparator() == QLocale().groupSeparator() || QLocale( QLocale::C ).decimalPoint() == QLocale().decimalPoint() )
         && value != "string" && expectedValue == 0 )
      expectedValue = 1;
    // There is another corner case in the test where the group separator is equal
    // to the C decimal point and there is no decimal point,
    // in that case the value is valid, because the fall
    // back check is to test after removing all group separators
    if ( QLocale().groupSeparator() == QLocale( QLocale::C ).decimalPoint()
         && !value.contains( QLocale().decimalPoint() )
         && value != "string" && expectedValue == 0 )
    {
      expectedValue = 1;
    }
    // qDebug() << value << loc << int( validator->validate( value ) ) << expectedValue;
    QCOMPARE( int( validator->validate( value ) ), expectedValue );
  }
}

void TestQgsDoubleValidator::toDouble()
{
  QFETCH( QString, actualValue );
  QFETCH( double, expValue );
  QString value;
  double expectedValue;

  const QVector<QLocale> listLocale( { QLocale::English, QLocale::French, QLocale::German, QLocale::Italian, QLocale::NorwegianBokmal, QLocale::Ukrainian } );
  QLocale loc;
  for ( int i = 0; i < listLocale.count(); ++i )
  {
    loc = listLocale.at( i );
    QLocale::setDefault( loc );
    value = actualValue;
    value = value.replace( "ld", QLocale().decimalPoint() )
              .replace( "cd", QLocale( QLocale::C ).decimalPoint() )
              .replace( "lg", QLocale().groupSeparator() )
              .replace( "cg", QLocale( QLocale::C ).groupSeparator() )
              .replace( "ln", QLocale().negativeSign() )
              .replace( "cn", QLocale( QLocale::C ).negativeSign() )
              .replace( "le", QLocale().exponential() );
    expectedValue = expValue;
    // if the local group separator / decimal point is equal to the C one,
    // expected result will be different for double with test with mixed
    // local/C characters.
    // Example with lg as local group separator
    //              cg as C group separator
    //              ld as local decimal point
    //              cd as C group decimal point
    // for 4cg444ld6 double, if cg == lg then 4cg444ld6 == 4lg444ld6
    //                       and QgsDoubleValidator::toDouble(4lg444ld6) == 4444.6 and not 0.0
    // for 4cg444ld6 double, if cd == ld then 4cg444ld6 == 4cg444cd6
    //                       and QgsDoubleValidator::toDouble(4cg444cd6) == 4444.6 and not 0.0
    if ( ( QLocale( QLocale::C ).groupSeparator() == QLocale().groupSeparator() || QLocale( QLocale::C ).decimalPoint() == QLocale().decimalPoint() )
         && value != "string" && expectedValue == 0.0 )
      expectedValue = 4444.6;
    // There is another corner case in the test where the group separator is equal
    // to the C decimal point and there is no decimal point,
    // in that case the value is valid, because the fall
    // back check is to test after removing all group separators
    if ( QLocale().groupSeparator() == QLocale( QLocale::C ).decimalPoint()
         && !value.contains( QLocale().decimalPoint() )
         && value != "string" && expectedValue == 0 )
    {
      expectedValue = 44446;
    }

    QCOMPARE( QgsDoubleValidator::toDouble( value ), expectedValue );
  }
}

QGSTEST_MAIN( TestQgsDoubleValidator )
#include "testqgsdoublevalidator.moc"
