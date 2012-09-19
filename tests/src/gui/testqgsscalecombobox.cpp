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
#include "qgsscalecombobox.h"
#include <QObject>
#include <QLineEdit>
#include <QComboBox>
#include <QtTest>

class TestQgsScaleComboBox: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void basic();
    void slot_test();
  private:
    QgsScaleComboBox *s;
};

void TestQgsScaleComboBox::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Create a combobox, and init with predefined scales.
  s = new QgsScaleComboBox();
};

void TestQgsScaleComboBox::cleanupTestCase()
{
  delete s;
};

void TestQgsScaleComboBox::init()
{
};

void TestQgsScaleComboBox::basic()
{
  QLineEdit *l = s->lineEdit();

  // Testing conversion from "1:nnn".
  l->setText( "" );
  QTest::keyClicks( l, "1:2345" );
  QTest::keyClick( l, Qt::Key_Return );
  QCOMPARE( s->scaleString(), QString( "1:2345" ) );
  QCOMPARE( s->scale(), (( double ) 1.0 / ( double ) 2345.0 ) );

  // Testing conversion from number to "1:x"
  l->setText( "" );
  QTest::keyClicks( l, QLocale::system().toString( 0.02 ) );
  QTest::keyClick( l, Qt::Key_Return );
  QCOMPARE( s->scaleString(), QString( "1:50" ) );
  QCOMPARE( s->scale(), ( double ) 0.02 );

  // Testing conversion from number to "x:1"
  l->setText( "" );
  QTest::keyClicks( l, QLocale::system().toString( 42 ) );
  QTest::keyClick( l, Qt::Key_Return );
  QCOMPARE( s->scaleString(), QString( "42:1" ) );
  QCOMPARE( s->scale(), ( double ) 42 );

};

void TestQgsScaleComboBox::slot_test()
{
}
void TestQgsScaleComboBox::cleanup()
{
};

QTEST_MAIN( TestQgsScaleComboBox )
#include "moc_testqgsscalecombobox.cxx"
