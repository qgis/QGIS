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
#include <QObject>
#include <QLineEdit>
#include <QComboBox>
#include <QtTest/QSignalSpy>
#include "qgstest.h"

class TestQgsScaleComboBox : public QObject
{
    Q_OBJECT
  public:
    TestQgsScaleComboBox() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void basic();
    void slot_test();
    void min_test();
    void toString();
    void toDouble();

  private:
    void enterScale( const QString &scale );
    void enterScale( double scale );
    QgsScaleComboBox *s = nullptr;
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
  // Create a combobox, and init with predefined scales.
  s = new QgsScaleComboBox();
  QgsDebugMsg( QString( "Initial scale is %1" ).arg( s->scaleString() ) );
}

void TestQgsScaleComboBox::basic()
{
  // Testing conversion from "1:nnn".
  enterScale( QStringLiteral( "1:2345" ) );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 2345 ) ) );
  QCOMPARE( s->scale(), 2345.0 );

  // Testing conversion from number to "1:x"
  enterScale( 0.02 );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 50 ) ) );
  QCOMPARE( s->scale(), 1.0 / 0.02 );

  // Testing conversion from number to "1:x"
  enterScale( 42 );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 42 ) ) );
  QCOMPARE( s->scale(), 42.0 );

  // Testing conversion from number to "1:x,000"
  QString str = QStringLiteral( "1%01000%01000" ).arg( QLocale::system().groupSeparator() );
  enterScale( str );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( str ) );
  QCOMPARE( s->scale(), 1000000.0 );

  // Testing conversion from number to "1:x,000" with wonky separators
  //(e.g., four digits between thousands, which should be fixed automatically)
  str = QStringLiteral( "1%010000%01000" ).arg( QLocale::system().groupSeparator() );
  QString fixedStr = QStringLiteral( "10%01000%01000" ).arg( QLocale::system().groupSeparator() );
  enterScale( str );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( fixedStr ) );
  QCOMPARE( s->scale(), 10000000.0 );

  // Testing rounding and conversion from illegal

  enterScale( 0.24 );

  enterScale( QStringLiteral( "1:x:2" ) );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 4 ) ) );
  QCOMPARE( s->scale(), 4.0 );

  // Test setting programmatically
  s->setScale( 1.0 / 0.19 );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 5 ) ) );
  QCOMPARE( s->scale(), 5.0 );

  // Test setting programmatically
  s->setScaleString( QStringLiteral( "1:240" ) );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 240 ) ) );
  QCOMPARE( s->scale(), 240.0 );

  // Test setting programmatically illegal string
  s->setScaleString( QStringLiteral( "1:2" ) + QLocale::system().decimalPoint() + "4" );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 240 ) ) );
  QCOMPARE( s->scale(), 240.0 );

}

void TestQgsScaleComboBox::slot_test()
{
  QLineEdit *l = s->lineEdit();

  QSignalSpy spyScaleChanged( s, SIGNAL( scaleChanged( double ) ) );
  QSignalSpy spyFixup( l, SIGNAL( editingFinished() ) );

  enterScale( 0.02 );
  QCOMPARE( spyFixup.count(), 2 ); // Qt emits twice!?
  QCOMPARE( spyScaleChanged.count(), 1 );
}

void TestQgsScaleComboBox::min_test()
{
  s->setMinScale( 100.0 );

  enterScale( 0.02 );
  QCOMPARE( s->scale(), 1.0 / 0.02 );

  enterScale( 0.002 );
  QCOMPARE( s->scale(), 100.0 );

  s->setMinScale( 1.0 / 0.015 );
  QCOMPARE( s->scale(), 1.0 / 0.015 );

  s->setScale( 2.0 );
  QCOMPARE( s->scale(), 2.0 );
}

void TestQgsScaleComboBox::toString()
{
  QCOMPARE( QgsScaleComboBox::toString( 100 ), QStringLiteral( "1:100" ) );
  QCOMPARE( QgsScaleComboBox::toString( 100.02134234 ), QStringLiteral( "1:100" ) );
  QCOMPARE( QgsScaleComboBox::toString( 1 ), QStringLiteral( "1:1" ) );
  QCOMPARE( QgsScaleComboBox::toString( 1.0 / 100 ), QStringLiteral( "100:1" ) );
}

void TestQgsScaleComboBox::toDouble()
{
  bool ok = false;
  QCOMPARE( QgsScaleComboBox::toDouble( QStringLiteral( "1:100" ), &ok ), 100.0 );
  QVERIFY( ok );
  QCOMPARE( QgsScaleComboBox::toDouble( QStringLiteral( "1:1" ), &ok ), 1.0 );
  QVERIFY( ok );
  QCOMPARE( QgsScaleComboBox::toDouble( QStringLiteral( "100:1" ), &ok ), 1.0 / 100 );
  QVERIFY( ok );
  QCOMPARE( QgsScaleComboBox::toDouble( QStringLiteral( "0.01" ), &ok ), 100.0 );
  QVERIFY( ok );
  QCOMPARE( QgsScaleComboBox::toDouble( QStringLiteral( "100" ), &ok ), 1.0 / 100.0 );
  QVERIFY( ok );

  //bad
  QgsScaleComboBox::toDouble( QStringLiteral( "abc" ), &ok );
  QVERIFY( !ok );
  QgsScaleComboBox::toDouble( QStringLiteral( "" ), &ok );
  QVERIFY( !ok );
  QgsScaleComboBox::toDouble( QStringLiteral( "1:" ), &ok );
  QVERIFY( !ok );
  QgsScaleComboBox::toDouble( QStringLiteral( "1:a" ), &ok );
  QVERIFY( !ok );
  QgsScaleComboBox::toDouble( QStringLiteral( "a:1" ), &ok );
  QVERIFY( !ok );
}

void TestQgsScaleComboBox::enterScale( const QString &scale )
{
  QLineEdit *l = s->lineEdit();
  l->clear();
  QTest::keyClicks( l, scale );
  QTest::keyClick( l, Qt::Key_Return );
}

void TestQgsScaleComboBox::enterScale( double scale )
{
  enterScale( QLocale::system().toString( scale ) );
}

void TestQgsScaleComboBox::cleanup()
{
  delete s;
}

QGSTEST_MAIN( TestQgsScaleComboBox )
#include "testqgsscalecombobox.moc"
