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
#include "qgsdoublespinbox.h"

Q_DECLARE_METATYPE( QgsColorWidget::ColorComponent )

void compareFloat( float a, float b )
{
  QVERIFY2( qgsDoubleNear( a, b, 0.00001 ), QString( "%1 != %2" ).arg( a ).arg( b ).toLatin1() );
}

class TestQgsCompoundColorWidget : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsCompoundColorWidget()
      : QgsTest( QStringLiteral( "Compound color widget Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testCmykConversion();
    void testComponentChange();
    void testComponentSettings_data();
    void testComponentSettings();
    void testModelChange();
    void testTabChange();
    void testInvalidColor();
    void testSliderWidgets();
    void testSliderWidgetsCmyk();
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
  w.mColorWheel->setColor( QColor( 10, 20, 30, 50 ), true );
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

  QgsSettings().setValue( QgsColorWidget::colorSpec( expectedComponent ) == QColor::Cmyk ? QStringLiteral( "Windows/ColorDialog/activeCmykComponent" ) : QStringLiteral( "Windows/ColorDialog/activeComponent" ), settingsComponent );

  QgsCompoundColorWidget w( nullptr, QgsColorWidget::colorSpec( expectedComponent ) == QColor::Cmyk ? QColor::fromCmyk( 1, 2, 3, 4 ) : QColor( 10, 20, 30, 50 ) );
  w.setVisible( true );

  QCOMPARE( w.mColorBox->component(), expectedComponent );
  QCOMPARE( w.mVerticalRamp->component(), expectedComponent );

  ( QgsColorWidget::colorSpec( expectedComponent ) == QColor::Cmyk ? w.mCmykRadios : w.mRgbRadios ).at( newSettingsComponent ).first->setChecked( true );
  QCOMPARE( w.mColorBox->component(), newComponent );
  QCOMPARE( w.mVerticalRamp->component(), newComponent );

  w.saveSettings();
  const int newValue = QgsSettings().value( QgsColorWidget::colorSpec( expectedComponent ) == QColor::Cmyk ? QStringLiteral( "Windows/ColorDialog/activeCmykComponent" ) : QStringLiteral( "Windows/ColorDialog/activeComponent" ), -1 ).toInt();
  QCOMPARE( newValue, newSettingsComponent );
}

void TestQgsCompoundColorWidget::testComponentChange()
{
  QgsSettings().setValue( QStringLiteral( "Windows/ColorDialog/activeComponent" ), 3 );

  QgsCompoundColorWidget w( nullptr, QColor( 10, 20, 30, 50 ) );
  w.setVisible( true );

  QCOMPARE( w.mColorBox->component(), QgsColorWidget::Red );
  QCOMPARE( w.mVerticalRamp->component(), QgsColorWidget::Red );

  const QList<QPair<QRadioButton *, QgsColorWidget::ColorComponent>> colors = {
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
    QCOMPARE( w.mVerticalRamp->component(), color.second );
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

  w.setColor( QColor::fromCmykF( 0.5, 0.1, 0.2, 0.3 ) );
  QCOMPARE( w.mColorModel->currentData(), QColor::Cmyk );

  w.setColor( QColor::fromHsvF( 0.5, 0.1, 0.2 ) );
  QCOMPARE( w.mColorModel->currentData(), QColor::Rgb );

  QVERIFY( w.mColorModel->isVisible() );
  w.setColorModelEditable( false );
  QVERIFY( !w.mColorModel->isVisible() );
  w.setColorModelEditable( true );
  QVERIFY( w.mColorModel->isVisible() );
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

void TestQgsCompoundColorWidget::testSliderWidgets()
{
  QgsCompoundColorWidget w( nullptr, QColor::fromRgbF( 0.12f, 0.34f, 0.56f, 0.78f ) );
  w.setVisible( true );

  // TODO QGIS 4 remove the nolint instructions, QColor was qreal (double) and is now float
  // NOLINTBEGIN(bugprone-narrowing-conversions)
  QCOMPARE( w.mRedSlider->mSpinBox->value(), 30.6 );
  compareFloat( w.mRedSlider->mRampWidget->color().redF(), 0.12f );
  QCOMPARE( w.mGreenSlider->mSpinBox->value(), 86.7 );
  compareFloat( w.mGreenSlider->mRampWidget->color().greenF(), 0.34f );
  QCOMPARE( w.mBlueSlider->mSpinBox->value(), 142.8 );
  compareFloat( w.mBlueSlider->mRampWidget->color().blueF(), 0.56f );
  QCOMPARE( w.mAlphaSlider->mSpinBox->value(), 78 );
  compareFloat( w.mAlphaSlider->mRampWidget->color().alphaF(), 0.78f );

  w.mRedSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mRedSlider->mRampWidget->width() ) / 2.f, 0.f ) );
  w.mBlueSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mBlueSlider->mRampWidget->width() ) / 2.f, 0.f ) );
  w.mGreenSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mGreenSlider->mRampWidget->width() ) / 2.f, 0.f ) );
  w.mAlphaSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mAlphaSlider->mRampWidget->width() ) / 2.f, 0.f ) );

  QCOMPARE( w.mRedSlider->mSpinBox->value(), 127.5 );
  compareFloat( w.mRedSlider->mRampWidget->color().redF(), 0.5f );
  QCOMPARE( w.mBlueSlider->mSpinBox->value(), 127.5 );
  compareFloat( w.mBlueSlider->mRampWidget->color().blueF(), 0.5f );
  QCOMPARE( w.mGreenSlider->mSpinBox->value(), 127.5 );
  compareFloat( w.mGreenSlider->mRampWidget->color().greenF(), 0.5f );
  QCOMPARE( w.mAlphaSlider->mSpinBox->value(), 50 );
  compareFloat( w.mAlphaSlider->mRampWidget->color().greenF(), 0.5f );

  w.mHueSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mHueSlider->mRampWidget->width() ) / 2.f, 0.f ) );
  w.mSaturationSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mSaturationSlider->mRampWidget->width() ) / 2.f, 0.f ) );
  w.mValueSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mValueSlider->mRampWidget->width() ) / 2.f, 0.f ) );

  QCOMPARE( w.mHueSlider->mSpinBox->value(), 179.5 );
  compareFloat( w.mHueSlider->mRampWidget->color().hueF(), 0.5f );
  QCOMPARE( w.mSaturationSlider->mSpinBox->value(), 50 );
  compareFloat( w.mSaturationSlider->mRampWidget->color().saturationF(), 0.5f );
  QCOMPARE( w.mValueSlider->mSpinBox->value(), 50 );
  compareFloat( w.mValueSlider->mRampWidget->color().greenF(), 0.5f );
  // NOLINTEND(bugprone-narrowing-conversions)
}

void TestQgsCompoundColorWidget::testSliderWidgetsCmyk()
{
  QgsCompoundColorWidget w( nullptr, QColor::fromCmykF( 0.12f, 0.34f, 0.56f, 0.78f, 0.91f ) );
  w.setVisible( true );

  // TODO QGIS 4 remove the nolint instructions, QColor was qreal (double) and is now float
  // NOLINTBEGIN(bugprone-narrowing-conversions)
  QCOMPARE( w.mCyanSlider->mSpinBox->value(), 12 );
  compareFloat( w.mCyanSlider->mRampWidget->color().cyanF(), 0.12f );
  QCOMPARE( w.mMagentaSlider->mSpinBox->value(), 34 );
  compareFloat( w.mMagentaSlider->mRampWidget->color().magentaF(), 0.34f );
  QCOMPARE( w.mYellowSlider->mSpinBox->value(), 56 );
  compareFloat( w.mYellowSlider->mRampWidget->color().yellowF(), 0.56f );
  QCOMPARE( w.mBlackSlider->mSpinBox->value(), 78 );
  compareFloat( w.mBlackSlider->mRampWidget->color().blackF(), 0.78f );
  QCOMPARE( w.mAlphaSlider->mSpinBox->value(), 91 );
  compareFloat( w.mAlphaSlider->mRampWidget->color().alphaF(), 0.91f );

  w.mCyanSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mCyanSlider->mRampWidget->width() ) / 2.f, 0.f ) );
  w.mMagentaSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mMagentaSlider->mRampWidget->width() ) / 2.f, 0.f ) );
  w.mYellowSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mYellowSlider->mRampWidget->width() ) / 2.f, 0.f ) );
  w.mBlackSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mBlackSlider->mRampWidget->width() ) / 2.f, 0.f ) );
  w.mAlphaSlider->mRampWidget->setColorFromPoint( QPointF( static_cast<float>( w.mAlphaSlider->mRampWidget->width() ) / 2.f, 0.f ) );

  QCOMPARE( w.mCyanSlider->mSpinBox->value(), 50 );
  compareFloat( w.mCyanSlider->mRampWidget->color().cyanF(), 0.5f );
  QCOMPARE( w.mMagentaSlider->mSpinBox->value(), 50 );
  compareFloat( w.mMagentaSlider->mRampWidget->color().magentaF(), 0.5f );
  QCOMPARE( w.mYellowSlider->mSpinBox->value(), 50 );
  compareFloat( w.mYellowSlider->mRampWidget->color().yellowF(), 0.5f );
  QCOMPARE( w.mBlackSlider->mSpinBox->value(), 50 );
  compareFloat( w.mBlackSlider->mRampWidget->color().blackF(), 0.5f );
  QCOMPARE( w.mAlphaSlider->mSpinBox->value(), 50 );
  compareFloat( w.mAlphaSlider->mRampWidget->color().alphaF(), 0.5f );
  // NOLINTEND(bugprone-narrowing-conversions)
}


QGSTEST_MAIN( TestQgsCompoundColorWidget )
#include "testqgscompoundcolorwidget.moc"
