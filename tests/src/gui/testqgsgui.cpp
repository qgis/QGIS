/***************************************************************************
    testqgsgui.cpp
     --------------------------------------
    Date                 : 26.1.2015
    Copyright            : (C) 2015 Michael Kirk
    Email                : michael at jackpine dot me
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgsguiutils.h"

class TestQgsGui : public QObject
{
    Q_OBJECT
  private slots:
    void createFileFilterForFormat();
    void createFileFilter();
    void displayValueWithMaximumDecimals();
    void displayValueWithMaximumDecimals_data();

};

void TestQgsGui::createFileFilterForFormat()
{
  const QString expected = QStringLiteral( "FOO format (*.foo *.FOO)" );
  const QString actual = QgsGuiUtils::createFileFilter_( QStringLiteral( "foo" ) );

  QCOMPARE( actual, expected );
}

void TestQgsGui::createFileFilter()
{
  const QString expected = QStringLiteral( "My Description (my_regex MY_REGEX)" );
  const QString actual = QgsGuiUtils::createFileFilter_( QStringLiteral( "My Description" ), QStringLiteral( "my_regex" ) );

  QCOMPARE( actual, expected );
}

void TestQgsGui::displayValueWithMaximumDecimals()
{
  QFETCH( QLocale::Language, locale );
  QFETCH( double, value );
  QFETCH( Qgis::DataType, dataType );
  QFETCH( bool, displayTrailingZeroes );
  QFETCH( QString, result );

  QLocale::setDefault( QLocale( locale ) );
  QCOMPARE( QgsGuiUtils::displayValueWithMaximumDecimals( dataType, value, displayTrailingZeroes ), result );
}

void TestQgsGui::displayValueWithMaximumDecimals_data()
{

  QTest::addColumn<QLocale::Language>( "locale" );
  QTest::addColumn<Qgis::DataType>( "dataType" );
  QTest::addColumn<double>( "value" );
  QTest::addColumn<bool>( "displayTrailingZeroes" );
  QTest::addColumn<QString>( "result" );

  // Italian locale ("," as decimal point and "." as thousands separator)
  QTest::newRow( "float_1_it_1" ) << QLocale::Italian << Qgis::DataType::Float32 << 112345.0 << true  << "112.345,0000000" ;
  QTest::newRow( "float_1_it_0" ) << QLocale::Italian << Qgis::DataType::Float32 << 112345.0 << false << "112.345" ;
  QTest::newRow( "float_2_it_1" ) << QLocale::Italian << Qgis::DataType::Float32 << 112345.0102 << true  << "112.345,0102000" ;
  QTest::newRow( "float_2_it_0" ) << QLocale::Italian << Qgis::DataType::Float32 << 112345.0102 << false << "112.345,0102" ;

  QTest::newRow( "int_2_it_1" ) << QLocale::Italian << Qgis::DataType::Int32 << 112345.0102 << true << "112.345" ;
  QTest::newRow( "int_2_it_0" ) << QLocale::Italian << Qgis::DataType::Int32 << 112345.0102 << false << "112.345" ;

  // English locale
  QTest::newRow( "float_1_en_1" ) << QLocale::English << Qgis::DataType::Float32 << 112345.0 << true  << "112,345.0000000" ;
  QTest::newRow( "float_1_en_0" ) << QLocale::English << Qgis::DataType::Float32 << 112345.0 << false << "112,345" ;
  QTest::newRow( "float_2_en_1" ) << QLocale::English << Qgis::DataType::Float32 << 112345.0102 << true  << "112,345.0102000" ;
  QTest::newRow( "float_2_en_0" ) << QLocale::English << Qgis::DataType::Float32 << 112345.0102 << false << "112,345.0102" ;

  QTest::newRow( "int_2_en_1" ) << QLocale::English << Qgis::DataType::Int32 << 112345.0102 << true << "112,345" ;
  QTest::newRow( "int_2_en_0" ) << QLocale::English << Qgis::DataType::Int32 << 112345.0102 << false << "112,345" ;

}

QGSTEST_MAIN( TestQgsGui )
#include "testqgsgui.moc"
