/***************************************************************************
                         qgsprevieweffect.cpp
                             -------------------
    begin                : March 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include <QPainter>
#include "qgsprevieweffect.h"


QgsPreviewEffect::QgsPreviewEffect( QObject* parent )
    : QGraphicsEffect( parent )
    , mMode( PreviewGrayscale )
{
  //effect is disabled by default
  setEnabled( false );
}

QgsPreviewEffect::~QgsPreviewEffect()
{

}

void QgsPreviewEffect::setMode( QgsPreviewEffect::PreviewMode mode )
{
  mMode = mode;
  update();
}

void QgsPreviewEffect::draw( QPainter* painter )
{
  QPoint offset;
  QPixmap pixmap;

  if ( sourceIsPixmap() )
  {
    // No point in drawing in device coordinates (pixmap will be scaled anyways).
    pixmap = sourcePixmap( Qt::LogicalCoordinates, &offset );
  }
  else
  {
    // Draw pixmap in device coordinates to avoid pixmap scaling;
    pixmap = sourcePixmap( Qt::DeviceCoordinates, &offset );
    painter->setWorldTransform( QTransform() );
  }

  QImage image = pixmap.toImage();

  switch ( mMode )
  {
    case QgsPreviewEffect::PreviewGrayscale:
    {
      QRgb * line;

      for ( int y = 0; y < image.height(); y++ )
      {
        line = ( QRgb * )image.scanLine( y );
        for ( int x = 0; x < image.width(); x++ )
        {
          int gray = 0.21 * qRed( line[x] ) + 0.72 * qGreen( line[x] ) + 0.07 * qBlue( line[x] );
          line[x] = qRgb( gray, gray, gray );
        }
      }

      painter->drawImage( offset, image );
      break;
    }
    case QgsPreviewEffect::PreviewMono:
    {
      QImage bwImage = image.convertToFormat( QImage::Format_Mono );
      painter->drawImage( offset, bwImage );
      break;
    }
    case QgsPreviewEffect::PreviewProtanope:
    case QgsPreviewEffect::PreviewDeuteranope:
    {
      QRgb * line;

      for ( int y = 0; y < image.height(); y++ )
      {
        line = ( QRgb * )image.scanLine( y );
        for ( int x = 0; x < image.width(); x++ )
        {
          line[x] = simulateColorBlindness( line[x], mMode );
        }
      }

      painter->drawImage( offset, image );
      break;
    }
  }

}

QRgb QgsPreviewEffect::simulateColorBlindness( QRgb& originalColor, QgsPreviewEffect::PreviewMode mode )
{
  int red = qRed( originalColor );
  int green = qGreen( originalColor );
  int blue = qBlue( originalColor );

  //convert RGB to LMS color space
  // (http://vision.psychol.cam.ac.uk/jdmollon/papers/colourmaps.pdf p245, equation 4)
  double L = ( 17.8824 * red ) + ( 43.5161 * green ) + ( 4.11935 * blue );
  double M = ( 3.45565 * red ) + ( 27.1554 * green ) + ( 3.86714 * blue );
  double S = ( 0.0299566 * red ) + ( 0.184309 * green ) + ( 1.46709 * blue );

  //simulate color blindness
  switch ( mode )
  {
    case PreviewProtanope:
      simulateProtanopeLMS( L, M, S );
      break;
    case PreviewDeuteranope:
      simulateDeuteranopeLMS( L, M, S );
      break;
    default:
      break;
  }

  //convert LMS back to RGB color space
  //(http://vision.psychol.cam.ac.uk/jdmollon/papers/colourmaps.pdf p248, equation 6)
  red = ( 0.080944 * L ) + ( -0.130504 * M ) + ( 0.116721 * S );
  green = ( -0.0102485 * L ) + ( 0.0540194 * M ) + ( -0.113615 * S );
  blue = ( -0.000365294 * L ) + ( -0.00412163 * M ) + ( 0.693513 * S );

  //restrict values to 0-255
  red = qMax( qMin( 255, red ), 0 );
  green = qMax( qMin( 255, green ), 0 );
  blue = qMax( qMin( 255, blue ), 0 );

  return qRgb( red, green, blue );
}

void QgsPreviewEffect::simulateProtanopeLMS( double& L, double &M, double &S )
{
  //adjust L component to simulate vision of Protanope
  //(http://vision.psychol.cam.ac.uk/jdmollon/papers/colourmaps.pdf p248, equation 5)
  L = ( 2.02344 * M ) + ( -2.52581 * S );
}

void QgsPreviewEffect::simulateDeuteranopeLMS( double& L, double &M, double &S )
{
  //adjust M component to simulate vision of Deuteranope
  //(http://vision.psychol.cam.ac.uk/jdmollon/papers/colourmaps.pdf p248, equation 5)
  M = ( 0.494207 * L ) + ( 1.24827 * S );
}
