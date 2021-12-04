/***************************************************************************
                         testqgsimageoperations.cpp
                         --------------------------
    begin                : January 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsimageoperation.h"
#include "qgscolorrampimpl.h"
#include <QObject>
#include "qgstest.h"
#include "qgsrenderchecker.h"
#include "qgssymbollayerutils.h"
#include "qgsapplication.h"

class TestQgsImageOperation : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void smallImageOp(); //test operation on small image (single threaded op)

    //grayscale
    void grayscaleLightness();
    void grayscaleLuminosity();
    void grayscaleAverage();

    //brightness/contrast
    void brightnessContrastNoChange();
    void increaseBrightness();
    void decreaseBrightness();
    void increaseContrast();
    void decreaseContrast();

    //hue/saturation
    void hueSaturationNoChange();
    void increaseSaturation();
    void decreaseSaturation();
    void colorizeFull();
    void colorizePartial();

    //multiply opacity
    void opacityNoChange();
    void opacityIncrease();
    void opacityDecrease();

    //overlay color
    void overlayColor();

    //distance transform
    void distanceTransformMaxDist();
    void distanceTransformSetSpread();
    void distanceTransformInterior();
    void distanceTransformMisc();

    //stack blur
    void stackBlur();
    void stackBlurPremultiplied();
    void alphaOnlyBlur();

    //gaussian blur
    void gaussianBlur();
    void gaussianBlurSmall();
    void gaussianBlurNoChange();

    //flip
    void flipHorizontal();
    void flipVertical();

  private:

    QString mReport;
    QString mSampleImage;
    QString mTransparentSampleImage;

    bool imageCheck( const QString &testName, QImage &image, int mismatchCount );
};

void TestQgsImageOperation::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mReport += QLatin1String( "<h1>Image Operation Tests</h1>\n" );
  mSampleImage = QStringLiteral( TEST_DATA_DIR ) + "/sample_image.png";
  mTransparentSampleImage = QStringLiteral( TEST_DATA_DIR ) + "/sample_alpha_image.png";
}

void TestQgsImageOperation::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsImageOperation::init()
{
}

void TestQgsImageOperation::cleanup()
{

}

void TestQgsImageOperation::smallImageOp()
{
  QImage image( QStringLiteral( TEST_DATA_DIR ) + "/small_sample_image.png" );
  QgsImageOperation::convertToGrayscale( image, QgsImageOperation::GrayscaleLightness );

  const bool result = imageCheck( QStringLiteral( "imageop_smallimage" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::grayscaleLightness()
{
  QImage image( mSampleImage );
  QgsImageOperation::convertToGrayscale( image, QgsImageOperation::GrayscaleLightness );

  const bool result = imageCheck( QStringLiteral( "imageop_graylightness" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::grayscaleLuminosity()
{
  QImage image( mSampleImage );
  QgsImageOperation::convertToGrayscale( image, QgsImageOperation::GrayscaleLuminosity );

  const bool result = imageCheck( QStringLiteral( "imageop_grayluminosity" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::grayscaleAverage()
{
  QImage image( mSampleImage );
  QgsImageOperation::convertToGrayscale( image, QgsImageOperation::GrayscaleAverage );

  const bool result = imageCheck( QStringLiteral( "imageop_grayaverage" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::brightnessContrastNoChange()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustBrightnessContrast( image, 0, 1.0 );

  const bool result = imageCheck( QStringLiteral( "imageop_bcnochange" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::increaseBrightness()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustBrightnessContrast( image, 50, 1.0 );

  const bool result = imageCheck( QStringLiteral( "imageop_increasebright" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::decreaseBrightness()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustBrightnessContrast( image, -50, 1.0 );

  const bool result = imageCheck( QStringLiteral( "imageop_decreasebright" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::increaseContrast()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustBrightnessContrast( image, 0, 30.0 );

  const bool result = imageCheck( QStringLiteral( "imageop_increasecontrast" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::decreaseContrast()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustBrightnessContrast( image, 0, 0.1 );

  const bool result = imageCheck( QStringLiteral( "imageop_decreasecontrast" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::hueSaturationNoChange()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustHueSaturation( image, 1.0 );

  const bool result = imageCheck( QStringLiteral( "imageop_satnochange" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::increaseSaturation()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustHueSaturation( image, 5.0 );

  const bool result = imageCheck( QStringLiteral( "imageop_increasesat" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::decreaseSaturation()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustHueSaturation( image, 0.5 );

  const bool result = imageCheck( QStringLiteral( "imageop_decreasesat" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::colorizeFull()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustHueSaturation( image, 1.0, QColor( 255, 255, 0 ), 1.0 );

  const bool result = imageCheck( QStringLiteral( "imageop_colorizefull" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::colorizePartial()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustHueSaturation( image, 1.0, QColor( 255, 255, 0 ), 0.5 );

  const bool result = imageCheck( QStringLiteral( "imageop_colorizepartial" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::opacityNoChange()
{
  QImage image( mSampleImage );
  QgsImageOperation::multiplyOpacity( image, 1.0 );

  const bool result = imageCheck( QStringLiteral( "imageop_opacitynochange" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::opacityIncrease()
{
  QImage image( mSampleImage );
  QgsImageOperation::multiplyOpacity( image, 2.0 );

  const bool result = imageCheck( QStringLiteral( "imageop_opacityincrease" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::opacityDecrease()
{
  QImage image( mSampleImage );
  QgsImageOperation::multiplyOpacity( image, 0.5 );

  const bool result = imageCheck( QStringLiteral( "imageop_opacitydecrease" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::overlayColor()
{
  QImage image( mSampleImage );
  QgsImageOperation::overlayColor( image, QColor( 0, 255, 255 ) );

  const bool result = imageCheck( QStringLiteral( "imageop_overlaycolor" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::distanceTransformMaxDist()
{
  QImage image( mTransparentSampleImage );
  QgsGradientColorRamp ramp( QColor( 0, 0, 255 ), QColor( 0, 255, 0 ) );
  QgsImageOperation::DistanceTransformProperties props;
  props.useMaxDistance = true;
  props.ramp = &ramp;
  props.shadeExterior = true;

  QgsImageOperation::distanceTransform( image, props );

  const bool result = imageCheck( QStringLiteral( "imageop_dt_max" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::distanceTransformSetSpread()
{
  QImage image( mTransparentSampleImage );
  QgsGradientColorRamp ramp( QColor( 0, 0, 255 ), QColor( 0, 255, 0 ) );
  QgsImageOperation::DistanceTransformProperties props;
  props.useMaxDistance = false;
  props.spread = 10;
  props.ramp = &ramp;
  props.shadeExterior = true;

  QgsImageOperation::distanceTransform( image, props );

  const bool result = imageCheck( QStringLiteral( "imageop_dt_spread" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::distanceTransformInterior()
{
  QImage image( mTransparentSampleImage );
  QgsGradientColorRamp ramp( QColor( 0, 0, 255 ), QColor( 0, 255, 0 ) );
  QgsImageOperation::DistanceTransformProperties props;
  props.useMaxDistance = true;
  props.ramp = &ramp;
  props.shadeExterior = false;

  QgsImageOperation::distanceTransform( image, props );

  const bool result = imageCheck( QStringLiteral( "imageop_dt_interior" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::distanceTransformMisc()
{
  //no ramp
  QImage image( mSampleImage );
  QgsImageOperation::DistanceTransformProperties props;
  props.useMaxDistance = true;
  props.ramp = nullptr;
  props.shadeExterior = false;
  QgsImageOperation::distanceTransform( image, props );
  bool result = imageCheck( QStringLiteral( "imageop_nochange" ), image, 0 );
  QVERIFY( result );

  //zero spread
  QImage image2( mSampleImage );
  QgsImageOperation::DistanceTransformProperties props2;
  QgsGradientColorRamp ramp( QColor( 0, 0, 255 ), QColor( 0, 255, 0 ) );
  props2.useMaxDistance = false;
  props2.spread = 0;
  props2.ramp = &ramp;
  props2.shadeExterior = false;
  QgsImageOperation::distanceTransform( image2, props2 );
  result = imageCheck( QStringLiteral( "imageop_zerospread" ), image2, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::stackBlur()
{
  QImage image( mSampleImage );
  QgsImageOperation::stackBlur( image, 10 );

  const bool result = imageCheck( QStringLiteral( "imageop_stackblur" ), image, 0 );
  QVERIFY( result );
  QCOMPARE( image.format(), QImage::Format_ARGB32 );
}

void TestQgsImageOperation::stackBlurPremultiplied()
{
  QImage image( mSampleImage );
  image = image.convertToFormat( QImage::Format_ARGB32_Premultiplied );
  QgsImageOperation::stackBlur( image, 10 );

  const bool result = imageCheck( QStringLiteral( "imageop_stackblur" ), image, 0 );
  QVERIFY( result );
  QCOMPARE( image.format(), QImage::Format_ARGB32_Premultiplied );
}

void TestQgsImageOperation::alphaOnlyBlur()
{
  QImage image( QStringLiteral( TEST_DATA_DIR ) + "/small_sample_image.png" );
  QgsImageOperation::stackBlur( image, 10, true );

  bool result = imageCheck( QStringLiteral( "imageop_stackblur_alphaonly" ), image, 0 );
  QVERIFY( result );
  QCOMPARE( image.format(), QImage::Format_ARGB32 );

  QImage premultImage( QStringLiteral( TEST_DATA_DIR ) + "/small_sample_image.png" );
  premultImage = premultImage.convertToFormat( QImage::Format_ARGB32_Premultiplied );
  QgsImageOperation::stackBlur( premultImage, 10, true );

  result = imageCheck( QStringLiteral( "imageop_stackblur_alphaonly" ), premultImage, 0 );
  QVERIFY( result );
  QCOMPARE( premultImage.format(), QImage::Format_ARGB32_Premultiplied );
}

void TestQgsImageOperation::gaussianBlur()
{
  QImage image( mSampleImage );
  QImage *blurredImage = QgsImageOperation::gaussianBlur( image, 30 );

  const bool result = imageCheck( QStringLiteral( "imageop_gaussianblur" ), *blurredImage, 0 );
  QCOMPARE( blurredImage->format(), QImage::Format_ARGB32 );
  delete blurredImage;
  QVERIFY( result );
}

//todo small, zero radius
void TestQgsImageOperation::gaussianBlurSmall()
{
  QImage image( QStringLiteral( TEST_DATA_DIR ) + "/small_sample_image.png" );
  image = image.convertToFormat( QImage::Format_ARGB32_Premultiplied );

  QImage *blurredImage = QgsImageOperation::gaussianBlur( image, 10 );

  QCOMPARE( blurredImage->format(), QImage::Format_ARGB32_Premultiplied );
  const bool result = imageCheck( QStringLiteral( "imageop_gaussianblur_small" ), *blurredImage, 0 );
  delete blurredImage;
  QVERIFY( result );
}

void TestQgsImageOperation::gaussianBlurNoChange()
{
  QImage image( mSampleImage );
  QImage *blurredImage = QgsImageOperation::gaussianBlur( image, 0 );

  const bool result = imageCheck( QStringLiteral( "imageop_nochange" ), *blurredImage, 0 );
  delete blurredImage;
  QVERIFY( result );
}

void TestQgsImageOperation::flipHorizontal()
{
  QImage image( mSampleImage );
  QgsImageOperation::flipImage( image, QgsImageOperation::FlipHorizontal );

  const bool result = imageCheck( QStringLiteral( "imageop_fliphoz" ), image, 0 );
  QVERIFY( result );
}

void TestQgsImageOperation::flipVertical()
{
  QImage image( mSampleImage );
  QgsImageOperation::flipImage( image, QgsImageOperation::FlipVertical );

  const bool result = imageCheck( QStringLiteral( "imageop_flipvert" ), image, 0 );
  QVERIFY( result );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsImageOperation::imageCheck( const QString &testName, QImage &image, int mismatchCount )
{
  //draw background
  QImage imageWithBackground( image.width(), image.height(), QImage::Format_RGB32 );
  QgsRenderChecker::drawBackground( &imageWithBackground );
  QPainter painter( &imageWithBackground );
  painter.drawImage( 0, 0, image );
  painter.end();

  mReport += "<h2>" + testName + "</h2>\n";
  const QString tempDir = QDir::tempPath() + '/';
  const QString fileName = tempDir + testName + ".png";
  imageWithBackground.save( fileName, "PNG" );
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "image_operations" ) );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( fileName );
  checker.setColorTolerance( 2 );
  const bool resultFlag = checker.compareImages( testName, mismatchCount );
  mReport += checker.report();
  return resultFlag;
}

QGSTEST_MAIN( TestQgsImageOperation )
#include "testqgsimageoperation.moc"
