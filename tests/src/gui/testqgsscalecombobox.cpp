/***************************************************************************
                         testqgsscalecombobox.cpp
                         ---------------------------
    begin                : September 2012
    copyright            : (C) 2012 by Magnus Homann
    email                : magnus at homann dot se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsscalecombobox.h"
#include "qgssettingsregistrycore.h"
#include "qgstest.h"

#include <QComboBox>
#include <QLineEdit>
#include <QObject>
#include <QtTest/QSignalSpy>

class TestQgsScaleComboBox : public QObject
{
    Q_OBJECT
  public:
    TestQgsScaleComboBox() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void basic();
    void flexible();
    void slot_test();
    void min_test();
    void toString_data();
    void toString();
    void toDouble();
    void allowNull();
    void testLocale();

  private:
    void enterScale( const QString &scale, QgsScaleComboBox *widget );
    void enterScale( double scale, QgsScaleComboBox *widget );
};

void TestQgsScaleComboBox::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsScaleComboBox::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsScaleComboBox::init()
{
}

void TestQgsScaleComboBox::basic()
{
  // Create a combobox, and init with predefined scales.
  QgsScaleComboBox s;
  QgsDebugMsgLevel( u"Initial scale is %1"_s.arg( s.scaleString() ), 1 );

  const QStringList scales = QgsSettingsRegistryCore::settingsMapScales->value();
  QCOMPARE( scales.count(), s.count() );
  for ( int i = 0; i < s.count(); i++ )
  {
    int denominator = QLocale().toInt( scales[i].split( ':' )[1] );
    QCOMPARE( s.itemText( i ), QString( "1:%1" ).arg( QLocale().toString( denominator ) ) );
  }

  // Testing conversion from "1:nnn".
  enterScale( u"1:2345"_s, &s );
  QCOMPARE( s.scaleString(), QString( "1:%1" ).arg( QLocale().toString( 2345 ) ) );
  QCOMPARE( s.scale(), 2345.0 );

  // Testing conversion from number to "1:x"
  enterScale( 0.02, &s );
  QCOMPARE( s.scaleString(), QString( "1:%1" ).arg( QLocale().toString( 50 ) ) );
  QCOMPARE( s.scale(), 1.0 / 0.02 );

  // Testing conversion from number to "1:x"
  enterScale( 42, &s );
  QCOMPARE( s.scaleString(), QString( "1:%1" ).arg( QLocale().toString( 42 ) ) );
  QCOMPARE( s.scale(), 42.0 );

  // Testing conversion from number to "1:x,000"
  QString str = u"1%01000%01000"_s.arg( QLocale().groupSeparator() );
  enterScale( str, &s );
  QCOMPARE( s.scaleString(), QString( "1:%1" ).arg( str ) );
  QCOMPARE( s.scale(), 1000000.0 );

  // Testing conversion from number to "1:x,000" with wonky separators
  //(e.g., four digits between thousands, which should be fixed automatically)
  str = u"1%010000%01000"_s.arg( QLocale().groupSeparator() );
  const QString fixedStr = u"10%01000%01000"_s.arg( QLocale().groupSeparator() );
  enterScale( str, &s );
  QCOMPARE( s.scaleString(), QString( "1:%1" ).arg( fixedStr ) );
  QCOMPARE( s.scale(), 10000000.0 );

  // Testing rounding and conversion from illegal

  enterScale( 0.24, &s );

  enterScale( u"1:x:2"_s, &s );
  QCOMPARE( s.scaleString(), QString( "1:%1" ).arg( QLocale().toString( 4 ) ) );
  QCOMPARE( s.scale(), 4.0 );

  // Test setting programmatically
  s.setScale( 1.0 / 0.19 );
  QCOMPARE( s.scaleString(), QString( "1:%1" ).arg( QLocale().toString( 5 ) ) );
  QCOMPARE( s.scale(), 5.0 );

  // Test setting programmatically
  s.setScaleString( u"1:240"_s );
  QCOMPARE( s.scaleString(), QString( "1:%1" ).arg( QLocale().toString( 240 ) ) );
  QCOMPARE( s.scale(), 240.0 );

  // Test setting programmatically illegal string
  s.setScaleString( u"1:2"_s + QLocale().decimalPoint() + "4" );
  QCOMPARE( s.scaleString(), QString( "1:%1" ).arg( QLocale().toString( 240 ) ) );
  QCOMPARE( s.scale(), 240.0 );
}

void TestQgsScaleComboBox::flexible()
{
  auto combo = std::make_unique< QgsScaleComboBox >();
  combo->setRatioMode( QgsScaleComboBox::RatioMode::Flexible );

  const QStringList scales = QgsSettingsRegistryCore::settingsMapScales->value();
  QCOMPARE( scales.count(), combo->count() );
  for ( int i = 0; i < combo->count(); i++ )
  {
    int denominator = QLocale().toInt( scales[i].split( ':' )[1] );
    QCOMPARE( combo->itemText( i ), QString( "1:%1" ).arg( QLocale().toString( denominator ) ) );
  }

  // Testing conversion from "1:nnn".
  enterScale( u"1:2345"_s, combo.get() );
  QCOMPARE( combo->scaleString(), QString( "1:%1" ).arg( QLocale().toString( 2345 ) ) );
  QCOMPARE( combo->scale(), 2345.0 );

  // Testing conversion from number to "1:x"
  enterScale( 0.02, combo.get() );
  QCOMPARE( combo->scaleString(), u"1:50"_s );
  QCOMPARE( combo->scale(), 1.0 / 0.02 );

  // Testing conversion from number to "1:x"
  enterScale( 42, combo.get() );
  QCOMPARE( combo->scaleString(), u"42:1"_s );
  QGSCOMPARENEAR( combo->scale(), 1.0 / 42.0, 0.0001 );

  enterScale( u"2:3"_s, combo.get() );
  QCOMPARE( combo->scaleString(), u"2:3"_s );
  QCOMPARE( combo->scale(), 3.0 / 2.0 );

  enterScale( u"3:2"_s, combo.get() );
  QCOMPARE( combo->scaleString(), u"3:2"_s );
  QCOMPARE( combo->scale(), 2.0 / 3.0 );

  // Testing conversion from number to rational fraction
  enterScale( 2.0 / 3.0, combo.get() );
  QCOMPARE( combo->scaleString(), u"2:3"_s );
  QCOMPARE( combo->scale(), 3.0 / 2.0 );

  enterScale( 3.0 / 2.0, combo.get() );
  QCOMPARE( combo->scaleString(), u"3:2"_s );
  QCOMPARE( combo->scale(), 2.0 / 3.0 );

  // Test setting programmatically
  combo->setScale( 5.263 );
  QCOMPARE( combo->scaleString(), u"4:21"_s );
  // note that the combo internally rounds to 2 decimal places:
  QCOMPARE( combo->scale(), 5.25 );

  // Test setting programmatically
  combo->setScaleString( u"6:7"_s );
  QCOMPARE( combo->scaleString(), u"6:7"_s );
  QGSCOMPARENEAR( combo->scale(), 1.16666666667, 0.00001 );

  combo->setScaleString( u"7:6"_s );
  QCOMPARE( combo->scaleString(), u"7:6"_s );
  QGSCOMPARENEAR( combo->scale(), 0.8571428571, 0.00001 );
}

void TestQgsScaleComboBox::slot_test()
{
  // Create a combobox, and init with predefined scales.
  QgsScaleComboBox s;
  QgsDebugMsgLevel( u"Initial scale is %1"_s.arg( s.scaleString() ), 1 );

  QLineEdit *l = s.lineEdit();

  const QSignalSpy spyScaleChanged( &s, SIGNAL( scaleChanged( double ) ) );
  const QSignalSpy spyFixup( l, SIGNAL( editingFinished() ) );

  enterScale( 0.02, &s );
  QCOMPARE( spyFixup.count(), 2 ); // Qt emits twice!?
  QCOMPARE( spyScaleChanged.count(), 1 );
}

void TestQgsScaleComboBox::min_test()
{
  // Create a combobox, and init with predefined scales.
  QgsScaleComboBox s;
  QgsDebugMsgLevel( u"Initial scale is %1"_s.arg( s.scaleString() ), 1 );

  s.setMinScale( 100.0 );

  enterScale( 0.02, &s );
  QCOMPARE( s.scale(), 1.0 / 0.02 );

  enterScale( 0.002, &s );
  QCOMPARE( s.scale(), 100.0 );

  s.setMinScale( 1.0 / 0.015 );
  QCOMPARE( s.scale(), 1.0 / 0.015 );

  s.setScale( 2.0 );
  QCOMPARE( s.scale(), 2.0 );
}

void TestQgsScaleComboBox::toString_data()
{
  QTest::addColumn<double>( "scale" );
  QTest::addColumn<QgsScaleComboBox::RatioMode>( "ratioMode" );
  QTest::addColumn<QString>( "expected" );

  // ForceUnitNumerator mode
  QTest::newRow( "ForceUnitNumerator 100.0" ) << 100.0 << QgsScaleComboBox::RatioMode::ForceUnitNumerator << u"1:100"_s;
  QTest::newRow( "ForceUnitNumerator 100.02134234" ) << 100.02134234 << QgsScaleComboBox::RatioMode::ForceUnitNumerator << u"1:100"_s;
  QTest::newRow( "ForceUnitNumerator 1.0" ) << 1.0 << QgsScaleComboBox::RatioMode::ForceUnitNumerator << u"1:1"_s;
  QTest::newRow( "ForceUnitNumerator 1.0 / 100" ) << 1.0 / 100 << QgsScaleComboBox::RatioMode::ForceUnitNumerator << u"100:1"_s;
  QTest::newRow( "ForceUnitNumerator nan" ) << std::numeric_limits<double>::quiet_NaN() << QgsScaleComboBox::RatioMode::ForceUnitNumerator << QString();
  QTest::newRow( "ForceUnitNumerator 0.5" ) << 0.5 << QgsScaleComboBox::RatioMode::ForceUnitNumerator << u"2:1"_s;
  QTest::newRow( "ForceUnitNumerator 2.0" ) << 2.0 << QgsScaleComboBox::RatioMode::ForceUnitNumerator << u"1:2"_s;
  QTest::newRow( "ForceUnitNumerator 2.0 / 3.0" ) << 2.0 / 3.0 << QgsScaleComboBox::RatioMode::ForceUnitNumerator << u"2:1"_s;
  QTest::newRow( "ForceUnitNumerator 3.0 / 2.0" ) << 3.0 / 2.0 << QgsScaleComboBox::RatioMode::ForceUnitNumerator << u"1:2"_s;

  // Flexible mode
  QTest::newRow( "Flexible 100.0" ) << 100.0 << QgsScaleComboBox::RatioMode::Flexible << u"1:100"_s;
  QTest::newRow( "Flexible 100.02134234" ) << 100.02134234 << QgsScaleComboBox::RatioMode::Flexible << u"1:100"_s;
  QTest::newRow( "Flexible 1.0" ) << 1.0 << QgsScaleComboBox::RatioMode::Flexible << u"1:1"_s;
  QTest::newRow( "Flexible 1.0 / 100" ) << 1.0 / 100 << QgsScaleComboBox::RatioMode::Flexible << u"100:1"_s;
  QTest::newRow( "Flexible nan" ) << std::numeric_limits<double>::quiet_NaN() << QgsScaleComboBox::RatioMode::Flexible << QString();
  QTest::newRow( "Flexible 0.5" ) << 0.5 << QgsScaleComboBox::RatioMode::Flexible << u"2:1"_s;
  QTest::newRow( "Flexible 2.0" ) << 2.0 << QgsScaleComboBox::RatioMode::Flexible << u"1:2"_s;
  QTest::newRow( "Flexible 2.0 / 3.0" ) << 2.0 / 3.0 << QgsScaleComboBox::RatioMode::Flexible << u"3:2"_s;
  QTest::newRow( "Flexible 3.0 / 2.0" ) << 3.0 / 2.0 << QgsScaleComboBox::RatioMode::Flexible << u"2:3"_s;
  QTest::newRow( "Flexible 4.0 / 3.0" ) << 4.0 / 3.0 << QgsScaleComboBox::RatioMode::Flexible << u"3:4"_s;
  QTest::newRow( "Flexible 3.0 / 4.0" ) << 3.0 / 4.0 << QgsScaleComboBox::RatioMode::Flexible << u"4:3"_s;
  QTest::newRow( "Flexible 16.0 / 9.0" ) << 16.0 / 9.0 << QgsScaleComboBox::RatioMode::Flexible << u"9:16"_s;
  QTest::newRow( "Flexible 9.0 / 16.0" ) << 9.0 / 16.0 << QgsScaleComboBox::RatioMode::Flexible << u"16:9"_s;
}

void TestQgsScaleComboBox::toString()
{
  QFETCH( double, scale );
  QFETCH( QgsScaleComboBox::RatioMode, ratioMode );
  QFETCH( QString, expected );

  QCOMPARE( QgsScaleComboBox::toString( scale, ratioMode ), expected );
}

void TestQgsScaleComboBox::toDouble()
{
  bool ok = false;
  QCOMPARE( QgsScaleComboBox::toDouble( u"1:100"_s, &ok ), 100.0 );
  QVERIFY( ok );
  QCOMPARE( QgsScaleComboBox::toDouble( u"1:1"_s, &ok ), 1.0 );
  QVERIFY( ok );
  QCOMPARE( QgsScaleComboBox::toDouble( u"100:1"_s, &ok ), 1.0 / 100 );
  QVERIFY( ok );
  QCOMPARE( QgsScaleComboBox::toDouble( u"0.01"_s, &ok ), 100.0 );
  QVERIFY( ok );
  QCOMPARE( QgsScaleComboBox::toDouble( u"100"_s, &ok ), 1.0 / 100.0 );
  QVERIFY( ok );

  //bad
  QgsScaleComboBox::toDouble( u"abc"_s, &ok );
  QVERIFY( !ok );
  QgsScaleComboBox::toDouble( QString(), &ok );
  QVERIFY( !ok );
  QgsScaleComboBox::toDouble( u"1:"_s, &ok );
  QVERIFY( !ok );
  QgsScaleComboBox::toDouble( u"1:a"_s, &ok );
  QVERIFY( !ok );
  QgsScaleComboBox::toDouble( u"a:1"_s, &ok );
  QVERIFY( !ok );
}

void TestQgsScaleComboBox::allowNull()
{
  // Create a combobox, and init with predefined scales.
  QgsScaleComboBox s;
  QgsDebugMsgLevel( u"Initial scale is %1"_s.arg( s.scaleString() ), 1 );

  s.setScale( 50 );
  QVERIFY( !s.allowNull() );
  s.setNull(); // no effect
  QCOMPARE( s.scale(), 50.0 );
  QVERIFY( !s.isNull() );

  const QSignalSpy spyScaleChanged( &s, &QgsScaleComboBox::scaleChanged );
  s.setAllowNull( true );
  QVERIFY( s.allowNull() );

  QVERIFY( s.lineEdit()->isClearButtonEnabled() );

  s.setScaleString( QString() );
  QCOMPARE( spyScaleChanged.count(), 1 );
  QVERIFY( std::isnan( s.scale() ) );
  QVERIFY( s.isNull() );
  s.setScaleString( u"    "_s );
  QVERIFY( std::isnan( s.scale() ) );
  QVERIFY( s.lineEdit()->text().isEmpty() );
  QCOMPARE( spyScaleChanged.count(), 1 );
  QVERIFY( s.isNull() );

  enterScale( 0.02, &s );
  QCOMPARE( s.scale(), 50.0 );
  QCOMPARE( spyScaleChanged.count(), 2 );
  QCOMPARE( s.lineEdit()->text(), u"1:50"_s );
  QVERIFY( !s.isNull() );

  enterScale( QString(), &s );
  QVERIFY( std::isnan( s.scale() ) );
  QCOMPARE( spyScaleChanged.count(), 3 );
  QVERIFY( s.lineEdit()->text().isEmpty() );
  QVERIFY( s.isNull() );

  enterScale( 0.02, &s );
  QCOMPARE( s.scale(), 50.0 );
  QCOMPARE( spyScaleChanged.count(), 4 );
  s.setNull();
  QVERIFY( std::isnan( s.scale() ) );
  QCOMPARE( spyScaleChanged.count(), 5 );
  QVERIFY( s.lineEdit()->text().isEmpty() );
  QVERIFY( s.isNull() );

  s.setAllowNull( false );
  QVERIFY( !s.allowNull() );
  QVERIFY( !s.lineEdit()->isClearButtonEnabled() );
}

void TestQgsScaleComboBox::enterScale( const QString &scale, QgsScaleComboBox *widget )
{
  QLineEdit *l = widget->lineEdit();
  l->clear();
  QTest::keyClicks( l, scale );
  QTest::keyClick( l, Qt::Key_Return );
}

void TestQgsScaleComboBox::enterScale( double scale, QgsScaleComboBox *widget )
{
  enterScale( QLocale().toString( scale ), widget );
}

void TestQgsScaleComboBox::cleanup()
{
}

void TestQgsScaleComboBox::testLocale()
{
  // Create a combobox, and init with predefined scales.
  QgsScaleComboBox s;
  QgsDebugMsgLevel( u"Initial scale is %1"_s.arg( s.scaleString() ), 1 );

  QLocale::setDefault( QLocale::English );
  QCOMPARE( s.toString( 1e8 ), QString( "1:100,000,000" ) );
  QLocale customEnglish( QLocale::English );
  customEnglish.setNumberOptions( QLocale::NumberOption::OmitGroupSeparator );
  QLocale::setDefault( customEnglish );
  QCOMPARE( s.toString( 1e8 ), QString( "1:100000000" ) );

  QLocale::setDefault( QLocale::French );
  QCOMPARE( s.toString( 1e8 ), QString( "1:100 000 000" ) );
  QLocale customFrench( QLocale::French );
  customFrench.setNumberOptions( QLocale::NumberOption::OmitGroupSeparator );
  QLocale::setDefault( customFrench );
  QCOMPARE( s.toString( 1e8 ), QString( "1:100000000" ) );

  QLocale::setDefault( QLocale::German );
  QCOMPARE( s.toString( 1e8 ), QString( "1:100.000.000" ) );
  QLocale customGerman( QLocale::German );
  customGerman.setNumberOptions( QLocale::NumberOption::OmitGroupSeparator );
  QLocale::setDefault( customGerman );
  QCOMPARE( s.toString( 1e8 ), QString( "1:100000000" ) );
}

QGSTEST_MAIN( TestQgsScaleComboBox )
#include "testqgsscalecombobox.moc"
