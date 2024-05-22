/***************************************************************************
    testqgscompoundcolorwidget.cpp
    ---------------------
    begin                : 2024/05/07
    copyright            : (C) 2024 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgscompoundcolorwidget.h"
#include "qgssettings.h"

class TestQgsCompoundColorWidget : public QgsTest
{
    Q_OBJECT

  public:

    TestQgsCompoundColorWidget() : QgsTest( QStringLiteral( "Compound color widget Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testCmykConversion();
    void testComponentChange();
};

void TestQgsCompoundColorWidget::initTestCase()
{
}

void TestQgsCompoundColorWidget::cleanupTestCase()
{
}

void TestQgsCompoundColorWidget::init()
{
}

void TestQgsCompoundColorWidget::cleanup()
{
}

void TestQgsCompoundColorWidget::testCmykConversion()
{
  QgsCompoundColorWidget w( nullptr, QColor( 10, 20, 30, 50 ) );
  w.setVisible( true );

  QCOMPARE( w.color(), QColor( 10, 20, 30, 50 ) );
  QCOMPARE( w.mColorModel->currentIndex(), 0 );
  QVERIFY( w.mRGB->isVisible() );
  QVERIFY( !w.mCMYK->isVisible() );

  // switch to CMYK
  w.mColorModel->setCurrentIndex( 1 );
  QVERIFY( !w.mRGB->isVisible() );
  QVERIFY( w.mCMYK->isVisible() );
  QCOMPARE( w.mColorModel->currentIndex(), 1 );
  QCOMPARE( w.color(), QColor::fromCmyk( 170, 85, 0, 225, 50 ) );

  // switch back to RGB
  w.mColorModel->setCurrentIndex( 0 );
  QVERIFY( w.mRGB->isVisible() );
  QVERIFY( !w.mCMYK->isVisible() );
  QCOMPARE( w.mColorModel->currentIndex(), 0 );
  QCOMPARE( w.color(), QColor( 10, 20, 30, 50 ) );

  // edit color in CMYK mode
  w.mColorModel->setCurrentIndex( 1 );
  QVERIFY( !w.mRGB->isVisible() );
  QVERIFY( w.mCMYK->isVisible() );
  QCOMPARE( w.mColorModel->currentIndex(), 1 );
  QCOMPARE( w.color(), QColor::fromCmyk( 170, 85, 0, 225, 50 ) );

  w.mCyanSlider->setColor( QColor::fromCmyk( 120, 85, 0, 225, 50 ), true );
  QCOMPARE( w.color(), QColor::fromCmyk( 120, 85, 0, 225, 50 ) );

  // edit color in RGB, the returned color is still CMYK
  w.mColorWheel->setColor( QColor( 10, 20, 30, 50 ),  true );
  QCOMPARE( w.color(), QColor::fromCmyk( 170, 85, 0, 225, 50 ) );
}

void TestQgsCompoundColorWidget::testComponentChange()
{
  QgsSettings().setValue( QStringLiteral( "Windows/ColorDialog/activeComponent" ), 3 );

  QgsCompoundColorWidget w( nullptr, QColor( 10, 20, 30, 50 ) );
  w.setVisible( true );

  QCOMPARE( w.mColorBox->component(), QgsColorWidget::Red );
  QCOMPARE( w.mVerticalRamp->component(),  QgsColorWidget::Red );

  const QList<QPair<QRadioButton *, QgsColorWidget::ColorComponent>> colors =
  {
    { w.mHueRadio, QgsColorWidget::Hue },
    { w.mSaturationRadio, QgsColorWidget::Saturation },
    { w.mValueRadio, QgsColorWidget::Value },
    { w.mRedRadio, QgsColorWidget::Red },
    { w.mGreenRadio, QgsColorWidget::Green },
    { w.mBlueRadio, QgsColorWidget::Blue },
    { w.mCyanRadio, QgsColorWidget::Cyan },
    { w.mMagentaRadio, QgsColorWidget::Magenta },
    { w.mYellowRadio, QgsColorWidget::Yellow },
    { w.mBlackRadio, QgsColorWidget::Black }
  };

  for ( QPair<QRadioButton *, QgsColorWidget::ColorComponent> color : colors )
  {
    color.first->setChecked( true );
    QCOMPARE( w.mColorBox->component(), color.second );
    QCOMPARE( w.mVerticalRamp->component(),  color.second );
  }

}

QGSTEST_MAIN( TestQgsCompoundColorWidget )
#include "testqgscompoundcolorwidget.moc"
