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
#include <QtTest/QtTest>

class TestQgsScaleComboBox : public QObject
{
    Q_OBJECT
  public:
    TestQgsScaleComboBox()
        : s( nullptr )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void basic();
    void slot_test();
    void min_test();
  private:
    void enterScale( const QString& scale );
    void enterScale( double scale );
    QgsScaleComboBox *s;
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
  enterScale( "1:2345" );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 2345 ) ) );
  QCOMPARE( s->scale(), 1.0 / 2345.0 );

  // Testing conversion from number to "1:x"
  enterScale( 0.02 );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 50 ) ) );
  QCOMPARE( s->scale(), 0.02 );

  // Testing conversion from number to "1:x"
  enterScale( 42 );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 42 ) ) );
  QCOMPARE( s->scale(), 1.0 / 42.0 );

  // Testing conversion from number to "1:x,000"
  QString str = QString( "1%01000%01000" ).arg( QLocale::system().groupSeparator() );
  enterScale( str );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( str ) );
  QCOMPARE( s->scale(), 1.0 / 1000000.0 );

  // Testing conversion from number to "1:x,000" with wonky separators
  //(eg four digits between thousands, which should be fixed automatically)
  str = QString( "1%010000%01000" ).arg( QLocale::system().groupSeparator() );
  QString fixedStr = QString( "10%01000%01000" ).arg( QLocale::system().groupSeparator() );
  enterScale( str );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( fixedStr ) );
  QCOMPARE( s->scale(), 1.0 / 10000000.0 );

  // Testing rounding and conversion from illegal

  enterScale( 0.24 );

  enterScale( "1:x:2" );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 4 ) ) );
  QCOMPARE( s->scale(), 0.25 );

  // Test setting programatically
  s->setScale( 0.19 );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 5 ) ) );
  QCOMPARE( s->scale(), 0.2 );

  // Test setting programatically
  s->setScaleString( QString( "1:240" ) );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 240 ) ) );
  QCOMPARE( s->scale(), 1.0 / 240.0 );

  // Test setting programatically illegal string
  s->setScaleString( QString( "1:2" ) + QLocale::system().decimalPoint() + "4" );
  QCOMPARE( s->scaleString(), QString( "1:%1" ).arg( QLocale::system().toString( 240 ) ) );
  QCOMPARE( s->scale(), 1.0 / 240.0 );

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
  s->setMinScale( 0.01 );

  enterScale( 0.02 );
  QCOMPARE( s->scale(), 0.02 );

  enterScale( 0.002 );
  QCOMPARE( s->scale(), 0.01 );

  s->setMinScale( 0.015 );
  QCOMPARE( s->scale(), 0.015 );

  s->setScale( 0.5 );
  QCOMPARE( s->scale(), 0.5 );
}

void TestQgsScaleComboBox::enterScale( const QString& scale )
{
  QLineEdit *l = s->lineEdit();
  l->setText( "" );
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

QTEST_MAIN( TestQgsScaleComboBox )
#include "testqgsscalecombobox.moc"
