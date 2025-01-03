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


#include "qgstest.h"

#include <editorwidgets/qgsdoublespinbox.h>

#include <QSignalSpy>

class TestQgsDoubleSpinBox : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void clear();
    void expression();
    void step();
    void editingTimeout();

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
  QgsDoubleSpinBox *spinBox = new QgsDoubleSpinBox();
  spinBox->setMaximum( 10.0 );
  spinBox->setMinimum( 1.0 );
  spinBox->setValue( 5.0 );
  spinBox->setClearValueMode( QgsDoubleSpinBox::MinimumValue );
  QVERIFY( !spinBox->isCleared() );
  spinBox->clear();
  QVERIFY( spinBox->isCleared() );
  QCOMPARE( spinBox->value(), 1.0 );
  QCOMPARE( spinBox->clearValue(), 1.0 );
  spinBox->setClearValueMode( QgsDoubleSpinBox::MaximumValue );
  QVERIFY( !spinBox->isCleared() );
  spinBox->clear();
  QVERIFY( spinBox->isCleared() );
  QCOMPARE( spinBox->value(), 10.0 );
  QCOMPARE( spinBox->clearValue(), 10.0 );
  spinBox->setClearValue( 7.0 );
  QVERIFY( !spinBox->isCleared() );
  spinBox->clear();
  QVERIFY( spinBox->isCleared() );
  QCOMPARE( spinBox->value(), 7.0 );
  QCOMPARE( spinBox->clearValue(), 7.0 );
  delete spinBox;
}

void TestQgsDoubleSpinBox::expression()
{
  QgsDoubleSpinBox *spinBox = new QgsDoubleSpinBox();
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
  spinBox->setSuffix( QStringLiteral( "mm" ) );
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
  spinBox->setPrefix( QStringLiteral( "mm" ) );
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
  spinBox->setSuffix( QStringLiteral( "ll" ) );
  spinBox->setPrefix( QStringLiteral( "mm" ) );
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

void TestQgsDoubleSpinBox::step()
{
  // test step logic

  QgsDoubleSpinBox spin;
  spin.setMinimum( -1000 );
  spin.setMaximum( 1000 );
  spin.setSingleStep( 1 );

  // no clear value
  spin.setValue( 0 );
  spin.stepBy( 1 );
  QCOMPARE( spin.value(), 1 );
  spin.stepBy( -1 );
  QCOMPARE( spin.value(), 0 );
  spin.stepBy( -1 );
  QCOMPARE( spin.value(), -1 );

  // with clear value
  spin.setClearValue( -1000, QStringLiteral( "NULL" ) );
  spin.setValue( 0 );
  spin.stepBy( 1 );
  QCOMPARE( spin.value(), 1 );
  spin.stepBy( -1 );
  QCOMPARE( spin.value(), 0 );
  spin.stepBy( -1 );
  QCOMPARE( spin.value(), -1 );
  spin.clear();
  QCOMPARE( spin.value(), -1000 );
  // when cleared, a step should NOT go to -999 (which is annoying for users), but rather pretend that the initial value was 0, not NULL
  spin.stepBy( 1 );
  QCOMPARE( spin.value(), 1 );
  spin.clear();
  QCOMPARE( spin.value(), -1000 );
  spin.stepBy( -1 );
  QCOMPARE( spin.value(), -1 );

  // with clear value, but no special value text. In this case we should NOT reset to 0 when incrementing up from the clear value
  spin.setSpecialValueText( QString() );
  spin.setClearValue( -1000 );
  spin.setValue( 0 );
  spin.stepBy( 1 );
  QCOMPARE( spin.value(), 1 );
  spin.stepBy( -1 );
  QCOMPARE( spin.value(), 0 );
  spin.stepBy( -1 );
  QCOMPARE( spin.value(), -1 );
  spin.clear();
  QCOMPARE( spin.value(), -1000 );
  spin.stepBy( 1 );
  QCOMPARE( spin.value(), -999 );
  spin.clear();
  QCOMPARE( spin.value(), -1000 );
  spin.stepBy( -1 );
  QCOMPARE( spin.value(), -1000 );
}

void TestQgsDoubleSpinBox::editingTimeout()
{
  QgsDoubleSpinBox spin;
  spin.setMinimum( -1000 );
  spin.setMaximum( 1000 );
  spin.setSingleStep( 1 );
  spin.setFocus();
  QCOMPARE( spin.editingTimeoutInterval(), 1000 );
  spin.setEditingTimeoutInterval( 300 );
  QCOMPARE( spin.editingTimeoutInterval(), 300 );

  QSignalSpy spy( &spin, &QgsDoubleSpinBox::editingTimeout );
  spin.selectAll();
  QTest::keyClicks( &spin, QStringLiteral( "3" ) );
  QTest::qWait( 100 );
  // too short, should not be signal
  QCOMPARE( spy.count(), 0 );
  QTest::qWait( 400 );
  // long enough, signal should have been emitted
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), 3 );

  QTest::keyClicks( &spin, QStringLiteral( "2" ) );
  QCOMPARE( spy.count(), 1 );
  QTest::qWait( 400 );
  // long enough, signal should have been emitted
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).toInt(), 32 );

  // no signal if value not changed
  QTest::keyClicks( &spin, QStringLiteral( "4" ) );
  QTest::qWait( 100 );
  QCOMPARE( spy.count(), 2 );
  QTest::keyPress( &spin, Qt::Key_Backspace );
  QTest::qWait( 400 );
  // no signal, value did not change
  QCOMPARE( spy.count(), 2 );
}

QGSTEST_MAIN( TestQgsDoubleSpinBox )
#include "testqgsdoublespinbox.moc"
