/***************************************************************************
     qgscompositionchecker.cpp - check rendering of Qgscomposition against an expected image
                     --------------------------------------
               Date                 : 5 Juli 2012
               Copyright            : (C) 2012 by Marco Hugentobler
               email                : marco@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscompositionchecker.h"
#include "qgscomposition.h"
#include <QImage>
#include <QPainter>

QgsCompositionChecker::QgsCompositionChecker( QgsComposition* composition, const QString& expectedImageFile ): mComposition( composition ), mExpectedImageFile( expectedImageFile )
{
}

QgsCompositionChecker::QgsCompositionChecker()
{
}

QgsCompositionChecker::~QgsCompositionChecker()
{
}

bool QgsCompositionChecker::testComposition()
{
  if ( !mComposition )
  {
    return false;
  }

#if 0 //fake mode to generate expected image
  //assume 300 dpi and size of the control image 3507 * 2480
  QImage outputImage( QSize( 3507, 2480 ), QImage::Format_ARGB32 );
  mComposition->setPlotStyle( QgsComposition::Print );
  outputImage.setDotsPerMeterX( 300 / 25.4 * 1000 );
  outputImage.setDotsPerMeterY( 300 / 25.4 * 1000 );
  outputImage.fill( 0 );
  QPainter p( &outputImage );
  QRectF sourceArea( 0, 0, mComposition->paperWidth(), mComposition->paperHeight() );
  QRectF targetArea( 0, 0, 3507, 2480 );
  mComposition->render( &p, targetArea, sourceArea );
  p.end();
  outputImage.save( "/tmp/composermap_control.png", "PNG" );
  return false;
#endif //0

  //load expected image
  QImage expectedImage( mExpectedImageFile );

  //get width/height, create image and render the composition to it
  int width = expectedImage.width();
  int height = expectedImage.height();
  QImage outputImage( QSize( width, height ), QImage::Format_ARGB32 );

  mComposition->setPlotStyle( QgsComposition::Print );
  outputImage.setDotsPerMeterX( expectedImage.dotsPerMeterX() );
  outputImage.setDotsPerMeterY( expectedImage.dotsPerMeterX() );
  outputImage.fill( 0 );
  QPainter p( &outputImage );
  QRectF sourceArea( 0, 0, mComposition->paperWidth(), mComposition->paperHeight() );
  QRectF targetArea( 0, 0, width, height );
  mComposition->render( &p, targetArea, sourceArea );
  p.end();

  return compareImages( expectedImage, outputImage );
}

bool QgsCompositionChecker::compareImages( const QImage& img1, const QImage& img2 ) const
{
  if ( img1.width() != img2.width() || img1.height() != img2.height() )
  {
    return false;
  }

  int imageWidth = img1.width();
  int imageHeight = img1.height();
  int mismatchCount = 0;

  QRgb pixel1, pixel2;
  for ( int i = 0; i < imageHeight; ++i )
  {
    for ( int j = 0; j < imageWidth; ++j )
    {
      pixel1 = img1.pixel( j, i );
      pixel2 = img2.pixel( j, i );
      if ( pixel1 != pixel2 )
      {
        ++mismatchCount;
      }
    }
  }
  return mismatchCount < 1;
}
