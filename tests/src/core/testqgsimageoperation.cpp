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

#include "qgsapplication.h"
#include "qgscolorrampimpl.h"
#include "qgsimageoperation.h"
#include "qgsrenderchecker.h"
#include "qgstest.h"

#include <QObject>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsImageOperation : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsImageOperation()
      : QgsTest( u"Image Operation Tests"_s, u"image_operations"_s )
    {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void init();         // will be called before each testfunction is executed.
    void cleanup();      // will be called after every testfunction.
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

    // flood fill
    void floodFill();
    void floodFill2();
    void floodFill3();

  private:
    QString mSampleImage;
    QString mTransparentSampleImage;

    static QImage renderImageForCheck( const QImage &image );
};

void TestQgsImageOperation::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mSampleImage = QStringLiteral( TEST_DATA_DIR ) + "/sample_image.png";
  mTransparentSampleImage = QStringLiteral( TEST_DATA_DIR ) + "/sample_alpha_image.png";
}

void TestQgsImageOperation::init()
{}

void TestQgsImageOperation::cleanup()
{}

void TestQgsImageOperation::smallImageOp()
{
  QImage image( QStringLiteral( TEST_DATA_DIR ) + "/small_sample_image.png" );
  QgsImageOperation::convertToGrayscale( image, QgsImageOperation::GrayscaleLightness );

  QGSVERIFYIMAGECHECK( u"imageop_smallimage"_s, u"expected_imageop_smallimage"_s, renderImageForCheck( image ), u"expected_imageop_smallimage"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::grayscaleLightness()
{
  QImage image( mSampleImage );
  QgsImageOperation::convertToGrayscale( image, QgsImageOperation::GrayscaleLightness );

  QGSVERIFYIMAGECHECK( u"imageop_graylightness"_s, u"expected_imageop_graylightness"_s, renderImageForCheck( image ), u"expected_imageop_graylightness"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::grayscaleLuminosity()
{
  QImage image( mSampleImage );
  QgsImageOperation::convertToGrayscale( image, QgsImageOperation::GrayscaleLuminosity );

  QGSVERIFYIMAGECHECK( u"imageop_grayluminosity"_s, u"expected_imageop_grayluminosity"_s, renderImageForCheck( image ), u"expected_imageop_grayluminosity"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::grayscaleAverage()
{
  QImage image( mSampleImage );
  QgsImageOperation::convertToGrayscale( image, QgsImageOperation::GrayscaleAverage );

  QGSVERIFYIMAGECHECK( u"imageop_grayaverage"_s, u"expected_imageop_grayaverage"_s, renderImageForCheck( image ), u"expected_imageop_grayaverage"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::brightnessContrastNoChange()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustBrightnessContrast( image, 0, 1.0 );

  QGSVERIFYIMAGECHECK( u"imageop_bcnochange"_s, u"expected_imageop_bcnochange"_s, renderImageForCheck( image ), u"expected_imageop_bcnochange"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::increaseBrightness()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustBrightnessContrast( image, 50, 1.0 );

  QGSVERIFYIMAGECHECK( u"imageop_increasebright"_s, u"expected_imageop_increasebright"_s, renderImageForCheck( image ), u"expected_imageop_increasebright"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::decreaseBrightness()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustBrightnessContrast( image, -50, 1.0 );

  QGSVERIFYIMAGECHECK( u"imageop_decreasebright"_s, u"expected_imageop_decreasebright"_s, renderImageForCheck( image ), u"expected_imageop_decreasebright"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::increaseContrast()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustBrightnessContrast( image, 0, 30.0 );

  QGSVERIFYIMAGECHECK( u"imageop_increasecontrast"_s, u"expected_imageop_increasecontrast"_s, renderImageForCheck( image ), u"expected_imageop_increasecontrast"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::decreaseContrast()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustBrightnessContrast( image, 0, 0.1 );

  QGSVERIFYIMAGECHECK( u"imageop_decreasecontrast"_s, u"expected_imageop_decreasecontrast"_s, renderImageForCheck( image ), u"expected_imageop_decreasecontrast"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::hueSaturationNoChange()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustHueSaturation( image, 1.0 );

  QGSVERIFYIMAGECHECK( u"imageop_satnochange"_s, u"expected_imageop_satnochange"_s, renderImageForCheck( image ), u"expected_imageop_satnochange"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::increaseSaturation()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustHueSaturation( image, 5.0 );

  QGSVERIFYIMAGECHECK( u"imageop_increasesat"_s, u"expected_imageop_increasesat"_s, renderImageForCheck( image ), u"expected_imageop_increasesat"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::decreaseSaturation()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustHueSaturation( image, 0.5 );

  QGSVERIFYIMAGECHECK( u"imageop_decreasesat"_s, u"expected_imageop_decreasesat"_s, renderImageForCheck( image ), u"expected_imageop_decreasesat"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::colorizeFull()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustHueSaturation( image, 1.0, QColor( 255, 255, 0 ), 1.0 );

  QGSVERIFYIMAGECHECK( u"imageop_colorizefull"_s, u"expected_imageop_colorizefull"_s, renderImageForCheck( image ), u"expected_imageop_colorizefull"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::colorizePartial()
{
  QImage image( mSampleImage );
  QgsImageOperation::adjustHueSaturation( image, 1.0, QColor( 255, 255, 0 ), 0.5 );

  QGSVERIFYIMAGECHECK( u"imageop_colorizepartial"_s, u"expected_imageop_colorizepartial"_s, renderImageForCheck( image ), u"expected_imageop_colorizepartial"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::opacityNoChange()
{
  QImage image( mSampleImage );
  QgsImageOperation::multiplyOpacity( image, 1.0 );

  QGSVERIFYIMAGECHECK( u"imageop_opacitynochange"_s, u"expected_imageop_opacitynochange"_s, renderImageForCheck( image ), u"expected_imageop_opacitynochange"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::opacityIncrease()
{
  QImage image( mSampleImage );
  QgsImageOperation::multiplyOpacity( image, 2.0 );

  QGSVERIFYIMAGECHECK( u"imageop_opacityincrease"_s, u"expected_imageop_opacityincrease"_s, renderImageForCheck( image ), u"expected_imageop_opacityincrease"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::opacityDecrease()
{
  QImage image( mSampleImage );
  QgsImageOperation::multiplyOpacity( image, 0.5 );

  QGSVERIFYIMAGECHECK( u"imageop_opacitydecrease"_s, u"expected_imageop_opacitydecrease"_s, renderImageForCheck( image ), u"expected_imageop_opacitydecrease"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::overlayColor()
{
  QImage image( mSampleImage );
  QgsImageOperation::overlayColor( image, QColor( 0, 255, 255 ) );

  QGSVERIFYIMAGECHECK( u"imageop_overlaycolor"_s, u"expected_imageop_overlaycolor"_s, renderImageForCheck( image ), u"expected_imageop_overlaycolor"_s, 0, QSize(), 2 );
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

  QGSVERIFYIMAGECHECK( u"imageop_dt_max"_s, u"expected_imageop_dt_max"_s, renderImageForCheck( image ), u"expected_imageop_dt_max"_s, 0, QSize(), 2 );
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

  QGSVERIFYIMAGECHECK( u"imageop_dt_spread"_s, u"expected_imageop_dt_spread"_s, renderImageForCheck( image ), u"expected_imageop_dt_spread"_s, 0, QSize(), 2 );
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

  QGSVERIFYIMAGECHECK( u"imageop_dt_interior"_s, u"expected_imageop_dt_interior"_s, renderImageForCheck( image ), u"expected_imageop_dt_interior"_s, 0, QSize(), 2 );
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
  QGSVERIFYIMAGECHECK( u"imageop_nochange"_s, u"expected_imageop_nochange"_s, renderImageForCheck( image ), u"expected_imageop_nochange"_s, 0, QSize(), 2 );

  //zero spread
  QImage image2( mSampleImage );
  QgsImageOperation::DistanceTransformProperties props2;
  QgsGradientColorRamp ramp( QColor( 0, 0, 255 ), QColor( 0, 255, 0 ) );
  props2.useMaxDistance = false;
  props2.spread = 0;
  props2.ramp = &ramp;
  props2.shadeExterior = false;
  QgsImageOperation::distanceTransform( image2, props2 );
  QGSVERIFYIMAGECHECK( u"imageop_zerospread"_s, u"expected_imageop_zerospread"_s, renderImageForCheck( image2 ), u"expected_imageop_zerospread"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::stackBlur()
{
  QImage image( mSampleImage );
  QgsImageOperation::stackBlur( image, 10 );

  QGSVERIFYIMAGECHECK( u"imageop_stackblur"_s, u"expected_imageop_stackblur"_s, renderImageForCheck( image ), u"expected_imageop_stackblur"_s, 0, QSize(), 2 );
  QCOMPARE( image.format(), QImage::Format_ARGB32 );
}

void TestQgsImageOperation::stackBlurPremultiplied()
{
  QImage image( mSampleImage );
  image = image.convertToFormat( QImage::Format_ARGB32_Premultiplied );
  QgsImageOperation::stackBlur( image, 10 );

  QGSVERIFYIMAGECHECK( u"imageop_stackblur"_s, u"expected_imageop_stackblur"_s, renderImageForCheck( image ), u"expected_imageop_stackblur"_s, 0, QSize(), 2 );
  QCOMPARE( image.format(), QImage::Format_ARGB32_Premultiplied );
}

void TestQgsImageOperation::alphaOnlyBlur()
{
  QImage image( QStringLiteral( TEST_DATA_DIR ) + "/small_sample_image.png" );
  QgsImageOperation::stackBlur( image, 10, true );

  QGSVERIFYIMAGECHECK( u"imageop_stackblur_alphaonly"_s, u"expected_imageop_stackblur_alphaonly"_s, renderImageForCheck( image ), u"expected_imageop_stackblur_alphaonly"_s, 0, QSize(), 2 );
  QCOMPARE( image.format(), QImage::Format_ARGB32 );

  QImage premultImage( QStringLiteral( TEST_DATA_DIR ) + "/small_sample_image.png" );
  premultImage = premultImage.convertToFormat( QImage::Format_ARGB32_Premultiplied );
  QgsImageOperation::stackBlur( premultImage, 10, true );

  QGSVERIFYIMAGECHECK( u"imageop_stackblur_alphaonly"_s, u"expected_imageop_stackblur_alphaonly"_s, renderImageForCheck( image ), u"expected_imageop_stackblur_alphaonly"_s, 0, QSize(), 2 );
  QCOMPARE( premultImage.format(), QImage::Format_ARGB32_Premultiplied );
}

void TestQgsImageOperation::gaussianBlur()
{
  QImage image( mSampleImage );
  QImage *blurredImage = QgsImageOperation::gaussianBlur( image, 30 );

  QGSVERIFYIMAGECHECK( u"imageop_gaussianblur"_s, u"expected_imageop_gaussianblur"_s, renderImageForCheck( *blurredImage ), u"expected_imageop_gaussianblur"_s, 0, QSize(), 2 );
  QCOMPARE( blurredImage->format(), QImage::Format_ARGB32 );
  delete blurredImage;
}

//todo small, zero radius
void TestQgsImageOperation::gaussianBlurSmall()
{
  QImage image( QStringLiteral( TEST_DATA_DIR ) + "/small_sample_image.png" );
  image = image.convertToFormat( QImage::Format_ARGB32_Premultiplied );

  QImage *blurredImage = QgsImageOperation::gaussianBlur( image, 10 );

  QCOMPARE( blurredImage->format(), QImage::Format_ARGB32_Premultiplied );
  QGSVERIFYIMAGECHECK( u"imageop_gaussianblur_small"_s, u"expected_imageop_gaussianblur_small"_s, renderImageForCheck( *blurredImage ), u"expected_imageop_gaussianblur_small"_s, 0, QSize(), 2 );
  delete blurredImage;
}

void TestQgsImageOperation::gaussianBlurNoChange()
{
  QImage image( mSampleImage );
  QImage *blurredImage = QgsImageOperation::gaussianBlur( image, 0 );

  QGSVERIFYIMAGECHECK( u"imageop_nochange"_s, u"expected_imageop_nochange"_s, renderImageForCheck( *blurredImage ), u"expected_imageop_nochange"_s, 0, QSize(), 2 );
  delete blurredImage;
}

void TestQgsImageOperation::flipHorizontal()
{
  QImage image( mSampleImage );
  QgsImageOperation::flipImage( image, QgsImageOperation::FlipHorizontal );

  QGSVERIFYIMAGECHECK( u"imageop_fliphoz"_s, u"expected_imageop_fliphoz"_s, renderImageForCheck( image ), u"expected_imageop_fliphoz"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::flipVertical()
{
  QImage image( mSampleImage );
  QgsImageOperation::flipImage( image, QgsImageOperation::FlipVertical );

  QGSVERIFYIMAGECHECK( u"imageop_flipvert"_s, u"expected_imageop_flipvert"_s, renderImageForCheck( image ), u"expected_imageop_flipvert"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::floodFill()
{
  const QImage image( mSampleImage );
  const QImage filled = QgsImageOperation::floodFill( image, QPoint( 0, 0 ), QColor( 0, 255, 0, 150 ), 0 );

  QGSVERIFYIMAGECHECK( u"imageop_floodfill1"_s, u"expected_imageop_floodfill1"_s, renderImageForCheck( filled ), u"expected_imageop_floodfill1"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::floodFill2()
{
  const QImage image( mSampleImage );
  const QImage filled = QgsImageOperation::floodFill( image, QPoint( 320, 276 ), QColor( 0, 255, 0, 150 ), 0 );

  QGSVERIFYIMAGECHECK( u"imageop_floodfill2"_s, u"expected_imageop_floodfill2"_s, renderImageForCheck( filled ), u"expected_imageop_floodfill2"_s, 0, QSize(), 2 );
}

void TestQgsImageOperation::floodFill3()
{
  const QImage image( mSampleImage );
  const QImage filled = QgsImageOperation::floodFill( image, QPoint( 320, 276 ), QColor( 0, 255, 0, 150 ), 40 );

  QGSVERIFYIMAGECHECK( u"imageop_floodfill3"_s, u"expected_imageop_floodfill3"_s, renderImageForCheck( filled ), u"expected_imageop_floodfill3"_s, 0, QSize(), 2 );
}

//
// Private helper functions not called directly by CTest
//

QImage TestQgsImageOperation::renderImageForCheck( const QImage &image )
{
  //draw background
  QImage imageWithBackground( image.width(), image.height(), QImage::Format_RGB32 );
  QgsRenderChecker::drawBackground( &imageWithBackground );
  QPainter painter( &imageWithBackground );
  painter.drawImage( 0, 0, image );
  painter.end();
  return imageWithBackground;
}

QGSTEST_MAIN( TestQgsImageOperation )
#include "testqgsimageoperation.moc"
