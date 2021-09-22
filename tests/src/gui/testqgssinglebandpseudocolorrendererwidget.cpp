/***************************************************************************
    testqgssinglebandpsueodcolorrendererwidget
     --------------------------------------
    Date                 : May 2020
    Copyright            : (C) 2020 Julien Cabieces
    Email                : julien.cabieces@oslandia.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>

#include <qgsapplication.h>
#include <qgsrasterlayer.h>
#include "qgssinglebandpseudocolorrendererwidget.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"

/**
 * \ingroup UnitTests
 * This is a unit test to verify that raster histogram works
 */
class TestQgsSingleBandPseudoColorRendererWidget : public QObject
{
    Q_OBJECT

  public:

    TestQgsSingleBandPseudoColorRendererWidget() {}

  private:

    QgsRasterLayer *mRasterLayer = nullptr;

  private slots:

    // init / cleanup
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    // tests
    void testEditLabel();
};


// slots

void TestQgsSingleBandPseudoColorRendererWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  const QString mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  const QString myLandsatFileName = mTestDataDir + "landsat.tif";

  mRasterLayer = new QgsRasterLayer( myLandsatFileName, "landsat" );
  QVERIFY( mRasterLayer->isValid() );
}

void TestQgsSingleBandPseudoColorRendererWidget::cleanupTestCase()
{
  delete mRasterLayer;
}

void TestQgsSingleBandPseudoColorRendererWidget::testEditLabel()
{
  // related to  https://github.com/qgis/QGIS/issues/36172

  QgsRasterShader *rasterShader = new QgsRasterShader();
  QgsColorRampShader *colorRampShader = new QgsColorRampShader();
  colorRampShader->setColorRampType( QgsColorRampShader::Interpolated );

  const double min = 122.123456;
  const double max = 129.123456;

  // configure pseudo band color renderer
  QList<QgsColorRampShader::ColorRampItem> colorRampItems;
  QgsColorRampShader::ColorRampItem firstItem;
  firstItem.value = 0.0;
  firstItem.label = "zero";
  firstItem.color = QColor( 0, 0, 255 );
  colorRampItems.append( firstItem );

  colorRampShader->setColorRampItemList( colorRampItems );
  rasterShader->setRasterShaderFunction( colorRampShader );

  QgsSingleBandPseudoColorRenderer *rasterRenderer = new QgsSingleBandPseudoColorRenderer( mRasterLayer->dataProvider(), 1, rasterShader );
  rasterRenderer->setClassificationMin( min );
  rasterRenderer->setClassificationMax( max );
  mRasterLayer->dataProvider()->setNoDataValue( 1, 126 );
  rasterRenderer->setNodataColor( QColor( 255, 0, 255 ) );
  mRasterLayer->setRenderer( rasterRenderer );

  QgsSingleBandPseudoColorRendererWidget widget( mRasterLayer );

  // force loading min/max with the same exact values
  // it should not triggers classification and we should get the initial ramp item
  widget.loadMinMax( 1, min, max );

  QgsSingleBandPseudoColorRenderer *newRasterRenderer = dynamic_cast<QgsSingleBandPseudoColorRenderer *>( widget.renderer() );
  QVERIFY( newRasterRenderer );
  QVERIFY( rasterRenderer->shader() );

  QgsColorRampShader *newColorRampShader = dynamic_cast<QgsColorRampShader *>( newRasterRenderer->shader()->rasterShaderFunction() );
  QVERIFY( newColorRampShader );

  const QList<QgsColorRampShader::ColorRampItem> newColorRampItems = newColorRampShader->colorRampItemList();
  QCOMPARE( newColorRampItems.at( 0 ).label, QStringLiteral( "zero" ) );

  QCOMPARE( widget.mMinLineEdit->text(), widget.displayValueWithMaxPrecision( widget.mColorRampShaderWidget->minimum() ) );
  QCOMPARE( widget.mMaxLineEdit->text(), widget.displayValueWithMaxPrecision( widget.mColorRampShaderWidget->maximum() ) );
  QCOMPARE( widget.mMinLineEdit->text(), widget.displayValueWithMaxPrecision( widget.mColorRampShaderWidget->shader().minimumValue() ) );
  QCOMPARE( widget.mMaxLineEdit->text(), widget.displayValueWithMaxPrecision( widget.mColorRampShaderWidget->shader().maximumValue() ) );

  // change the min/max
  widget.loadMinMax( 1, min + 1.0, max - 1.0 );

  QCOMPARE( widget.mMinLineEdit->text(), widget.displayValueWithMaxPrecision( min + 1.0 ) );
  QCOMPARE( widget.mMaxLineEdit->text(), widget.displayValueWithMaxPrecision( max - 1.0 ) );
  QCOMPARE( widget.mMinLineEdit->text(), widget.displayValueWithMaxPrecision( widget.mColorRampShaderWidget->minimum() ) );
  QCOMPARE( widget.mMaxLineEdit->text(), widget.displayValueWithMaxPrecision( widget.mColorRampShaderWidget->maximum() ) );
  QCOMPARE( widget.mMinLineEdit->text(), widget.displayValueWithMaxPrecision( widget.mColorRampShaderWidget->shader().minimumValue() ) );
  QCOMPARE( widget.mMaxLineEdit->text(), widget.displayValueWithMaxPrecision( widget.mColorRampShaderWidget->shader().maximumValue() ) );
}



QGSTEST_MAIN( TestQgsSingleBandPseudoColorRendererWidget )
#include "testqgssinglebandpseudocolorrendererwidget.moc"
