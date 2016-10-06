/***************************************************************************
    testqgsdoublespinbox.cpp
     --------------------------------------
    Date                 : December 2014
    Copyright            : (C) 2014 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QtTest/QtTest>

#include <editorwidgets/qgsdoublespinbox.h>

class TestQgsDoubleSpinBox: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void clear();
    void expression();

  private:

};

void TestQgsDoubleSpinBox::initTestCase()
{

}

void TestQgsDoubleSpinBox::cleanupTestCase()
{
}

void TestQgsDoubleSpinBox::init()
{
}

void TestQgsDoubleSpinBox::cleanup()
{
}

void TestQgsDoubleSpinBox::clear()
{
  QgsDoubleSpinBox* spinBox = new QgsDoubleSpinBox();
  spinBox->setMaximum( 10.0 );
  spinBox->setMinimum( 1.0 );
  spinBox->setValue( 5.0 );
  spinBox->setClearValueMode( QgsDoubleSpinBox::MinimumValue );
  spinBox->clear();
  QCOMPARE( spinBox->value(), 1.0 );
  QCOMPARE( spinBox->clearValue(), 1.0 );
  spinBox->setClearValueMode( QgsDoubleSpinBox::MaximumValue );
  spinBox->clear();
  QCOMPARE( spinBox->value(), 10.0 );
  QCOMPARE( spinBox->clearValue(), 10.0 );
  spinBox->setClearValue( 7.0 );
  spinBox->clear();
  QCOMPARE( spinBox->value(), 7.0 );
  QCOMPARE( spinBox->clearValue(), 7.0 );
  delete spinBox;
}

void TestQgsDoubleSpinBox::expression()
{
  QgsDoubleSpinBox* spinBox = new QgsDoubleSpinBox();
  spinBox->setMinimum( -10.0 );
  spinBox->setMaximum( 10.0 );
  spinBox->setValue( 1.0 );
  spinBox->setExpressionsEnabled( false );
  QCOMPARE( spinBox->valueFromText( QString( "5" ) ), 5.0 );
  QCOMPARE( spinBox->valueFromText( QString( "5+2" ) ), -10.0 );
  spinBox->setExpressionsEnabled( true );
  QCOMPARE( spinBox->valueFromText( QString( "5" ) ), 5.0 );
  QCOMPARE( spinBox->valueFromText( QString( "5+2" ) ), 7.0 );
  spinBox->setClearValue( 3.0 );
  spinBox->setShowClearButton( true );
  QCOMPARE( spinBox->valueFromText( QString() ), 3.0 ); //clearing should set to clearValue
  spinBox->setShowClearButton( false );
  spinBox->setValue( 8.0 );
  QCOMPARE( spinBox->valueFromText( QString() ), 8.0 ); //if no clear button, clearing should set to previous value
  spinBox->setShowClearButton( true );
  spinBox->setValue( 4.0 );
  QCOMPARE( spinBox->valueFromText( QString( "5/" ) ), 4.0 ); //invalid expression should reset to previous value

  //suffix tests
  spinBox->setSuffix( QString( "mm" ) );
  spinBox->setExpressionsEnabled( false );
  QCOMPARE( spinBox->valueFromText( QString( "5mm" ) ), 5.0 );
  QCOMPARE( spinBox->valueFromText( QString( "5+2mm" ) ), -10.0 );
  spinBox->setExpressionsEnabled( true );
  QCOMPARE( spinBox->valueFromText( QString( "5 mm" ) ), 5.0 );
  QCOMPARE( spinBox->valueFromText( QString( "5+2 mm" ) ), 7.0 );
  QCOMPARE( spinBox->valueFromText( QString( "5mm" ) ), 5.0 );
  QCOMPARE( spinBox->valueFromText( QString( "5+2mm" ) ), 7.0 );
  spinBox->setClearValue( 3.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm" ) ), 3.0 ); //clearing should set to clearValue
  QCOMPARE( spinBox->valueFromText( QString() ), 3.0 );
  spinBox->setValue( 4.0 );
  QCOMPARE( spinBox->valueFromText( QString( "5/mm" ) ), 4.0 ); //invalid expression should reset to previous value

  //prefix tests
  spinBox->setSuffix( QString() );
  spinBox->setPrefix( QString( "mm" ) );
  spinBox->setExpressionsEnabled( false );
  QCOMPARE( spinBox->valueFromText( QString( "mm5" ) ), 5.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm5+2" ) ), -10.0 );
  spinBox->setExpressionsEnabled( true );
  QCOMPARE( spinBox->valueFromText( QString( "mm 5" ) ), 5.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm 5+2" ) ), 7.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm5" ) ), 5.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm5+2" ) ), 7.0 );
  spinBox->setClearValue( 3.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm" ) ), 3.0 ); //clearing should set to clearValue
  QCOMPARE( spinBox->valueFromText( QString() ), 3.0 );
  spinBox->setValue( 4.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm5/" ) ), 4.0 ); //invalid expression should reset to previous value

  //both suffix and prefix
  spinBox->setSuffix( QString( "ll" ) );
  spinBox->setPrefix( QString( "mm" ) );
  spinBox->setExpressionsEnabled( true );
  QCOMPARE( spinBox->valueFromText( QString( "mm 5 ll" ) ), 5.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm 5+2 ll" ) ), 7.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm5ll" ) ), 5.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm5+2ll" ) ), 7.0 );
  spinBox->setClearValue( 3.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mmll" ) ), 3.0 ); //clearing should set to clearValue
  QCOMPARE( spinBox->valueFromText( QString() ), 3.0 );
  spinBox->setValue( 4.0 );
  QCOMPARE( spinBox->valueFromText( QString( "mm5/ll" ) ), 4.0 ); //invalid expression should reset to previous value

  delete spinBox;
}

QTEST_MAIN( TestQgsDoubleSpinBox )
#include "testqgsdoublespinbox.moc"
