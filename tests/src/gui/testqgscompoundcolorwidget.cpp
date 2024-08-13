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

Q_DECLARE_METATYPE( QgsColorWidget::ColorComponent )

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
    void testComponentSettings_data();
    void testComponentSettings();
    void testModelChange();
    void testTabChange();
    void testInvalidColor();
};

void TestQgsCompoundColorWidget::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
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

void TestQgsCompoundColorWidget::testComponentSettings_data()
{
  QTest::addColumn<int>( "settingsComponent" );
  QTest::addColumn<QgsColorWidget::ColorComponent>( "expectedComponent" );
  QTest::addColumn<QgsColorWidget::ColorComponent>( "newComponent" );
  QTest::addColumn<int>( "newSettingsComponent" );

  QTest::newRow( "hue" ) << 0 << QgsColorWidget::ColorComponent::Hue << QgsColorWidget::ColorComponent::Saturation << 1;
  QTest::newRow( "saturation" ) << 1 << QgsColorWidget::ColorComponent::Saturation << QgsColorWidget::ColorComponent::Value << 2;
  QTest::newRow( "value" ) << 2 << QgsColorWidget::ColorComponent::Value << QgsColorWidget::ColorComponent::Red << 3;
  QTest::newRow( "red" ) << 3 << QgsColorWidget::ColorComponent::Red << QgsColorWidget::ColorComponent::Green << 4;
  QTest::newRow( "green" ) << 4 << QgsColorWidget::ColorComponent::Green << QgsColorWidget::ColorComponent::Blue << 5;
  QTest::newRow( "blue" ) << 5 << QgsColorWidget::ColorComponent::Blue << QgsColorWidget::ColorComponent::Hue << 0;
  QTest::newRow( "cyan" ) << 0 << QgsColorWidget::ColorComponent::Cyan << QgsColorWidget::ColorComponent::Magenta << 1;
  QTest::newRow( "magenta" ) << 1 << QgsColorWidget::ColorComponent::Magenta << QgsColorWidget::ColorComponent::Yellow << 2;
  QTest::newRow( "yellow" ) << 2 << QgsColorWidget::ColorComponent::Yellow << QgsColorWidget::ColorComponent::Black << 3;
  QTest::newRow( "black" ) << 3 << QgsColorWidget::ColorComponent::Black << QgsColorWidget::ColorComponent::Cyan << 0;
}

void TestQgsCompoundColorWidget::testComponentSettings()
{
  QFETCH( int, settingsComponent );
  QFETCH( QgsColorWidget::ColorComponent, expectedComponent );
  QFETCH( QgsColorWidget::ColorComponent, newComponent );
  QFETCH( int, newSettingsComponent );

  QgsSettings().setValue( QgsColorWidget::colorSpec( expectedComponent ) == QColor::Cmyk ?
                          QStringLiteral( "Windows/ColorDialog/activeCmykComponent" ) : QStringLiteral( "Windows/ColorDialog/activeComponent" ), settingsComponent );

  QgsCompoundColorWidget w( nullptr, QgsColorWidget::colorSpec( expectedComponent ) == QColor::Cmyk ?
                            QColor::fromCmyk( 1, 2, 3, 4 ) : QColor( 10, 20, 30, 50 ) );
  w.setVisible( true );

  QCOMPARE( w.mColorBox->component(), expectedComponent );
  QCOMPARE( w.mVerticalRamp->component(), expectedComponent );

  ( QgsColorWidget::colorSpec( expectedComponent ) == QColor::Cmyk ? w.mCmykRadios : w.mRgbRadios ).at( newSettingsComponent ).first->setChecked( true );
  QCOMPARE( w.mColorBox->component(), newComponent );
  QCOMPARE( w.mVerticalRamp->component(), newComponent );

  w.saveSettings();
  const int newValue = QgsSettings().value( QgsColorWidget::colorSpec( expectedComponent ) == QColor::Cmyk ?
                       QStringLiteral( "Windows/ColorDialog/activeCmykComponent" ) : QStringLiteral( "Windows/ColorDialog/activeComponent" ), -1 ).toInt();
  QCOMPARE( newValue, newSettingsComponent );
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
    if ( QgsColorWidget::colorSpec( color.second ) != w.mColorModel->currentData() )
      w.mColorModel->setCurrentIndex( w.mColorModel->findData( QgsColorWidget::colorSpec( color.second ) ) );

    color.first->setChecked( true );
    QCOMPARE( w.mColorBox->component(), color.second );
    QCOMPARE( w.mVerticalRamp->component(),  color.second );
  }
}

void TestQgsCompoundColorWidget::testModelChange()
{
  QgsCompoundColorWidget w( nullptr, QColor( 10, 20, 30, 50 ) );
  w.setVisible( true );

  QCOMPARE( w.mColorModel->currentData(), QColor::Rgb );

  w.mColorModel->setCurrentIndex( w.mColorModel->findData( QColor::Cmyk ) );
  QCOMPARE( w.mColorModel->currentData(), QColor::Cmyk );

  w.setColor( QColor( 1, 2, 3 ) );
  QCOMPARE( w.mColorModel->currentData(), QColor::Rgb );
}

void TestQgsCompoundColorWidget::testTabChange()
{
  QgsCompoundColorWidget w( nullptr, QColor( 10, 20, 30, 50 ) );
  w.setVisible( true );

  QCOMPARE( w.mTabWidget->currentIndex(), 0 );
  QVERIFY( w.mRedRadio->isEnabled() );
  QVERIFY( w.mBlueRadio->isEnabled() );
  QVERIFY( w.mGreenRadio->isEnabled() );
  QVERIFY( w.mHueRadio->isEnabled() );
  QVERIFY( w.mSaturationRadio->isEnabled() );
  QVERIFY( w.mCyanRadio->isEnabled() );
  QVERIFY( w.mMagentaRadio->isEnabled() );
  QVERIFY( w.mYellowRadio->isEnabled() );
  QVERIFY( w.mBlackRadio->isEnabled() );

  w.mTabWidget->setCurrentIndex( 1 );
  QVERIFY( !w.mRedRadio->isEnabled() );
  QVERIFY( !w.mBlueRadio->isEnabled() );
  QVERIFY( !w.mGreenRadio->isEnabled() );
  QVERIFY( !w.mHueRadio->isEnabled() );
  QVERIFY( !w.mSaturationRadio->isEnabled() );
  QVERIFY( !w.mCyanRadio->isEnabled() );
  QVERIFY( !w.mMagentaRadio->isEnabled() );
  QVERIFY( !w.mYellowRadio->isEnabled() );
  QVERIFY( !w.mBlackRadio->isEnabled() );
}

void TestQgsCompoundColorWidget::testInvalidColor()
{
  QgsCompoundColorWidget w( nullptr, QColor() );
  w.setVisible( true );

  // default color is red
  QCOMPARE( w.color(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.mColorModel->currentIndex(), 0 );
  QVERIFY( w.mRGB->isVisible() );
  QVERIFY( !w.mCMYK->isVisible() );
}


QGSTEST_MAIN( TestQgsCompoundColorWidget )
#include "testqgscompoundcolorwidget.moc"
