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


QgsPreviewEffect::QgsPreviewEffect( QObject *parent )
  : QGraphicsEffect( parent )
  , mMode( PreviewGrayscale )
{
  //effect is disabled by default
  setEnabled( false );
}

void QgsPreviewEffect::setMode( QgsPreviewEffect::PreviewMode mode )
{
  mMode = mode;
  update();
}

void QgsPreviewEffect::draw( QPainter *painter )
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
    case QgsPreviewEffect::PreviewMono:
    {
      const QImage bwImage = image.convertToFormat( QImage::Format_Mono );
      painter->drawImage( offset, bwImage );
      break;
    }
    case QgsPreviewEffect::PreviewGrayscale:
    case QgsPreviewEffect::PreviewProtanope:
    case QgsPreviewEffect::PreviewDeuteranope:
    case QgsPreviewEffect::PreviewTritanope:
    {
      QRgb *line = nullptr;

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

QRgb QgsPreviewEffect::simulateColorBlindness( QRgb &originalColor, QgsPreviewEffect::PreviewMode mode )
{
  int red = qRed( originalColor );
  int green = qGreen( originalColor );
  int blue = qBlue( originalColor );

  int r = red;
  int g = green;
  int b = blue;

  //simulate color blindness
  //matrix values taken from Machado et al. (2009), https://doi.org/10.1109/TVCG.2009.113:
  //https://www.inf.ufrgs.br/~oliveira/pubs_files/CVD_Simulation/CVD_Simulation.html
  switch ( mode )
  {
    case PreviewGrayscale:
      simulateGrayscale( r, g, b, red, green, blue );
      break;
    case PreviewProtanope:
      simulateProtanope( r, g, b, red, green, blue );
      break;
    case PreviewDeuteranope:
      simulateDeuteranope( r, g, b, red, green, blue );
      break;
    case PreviewTritanope:
      simulateTritanope( r, g, b, red, green, blue );
      break;
    default:
      break;
  }

  //restrict values to 0-255
  r = std::max( std::min( 255, r ), 0 );
  g = std::max( std::min( 255, g ), 0 );
  b = std::max( std::min( 255, b ), 0 );

  return qRgb( r, g, b );
}

void QgsPreviewEffect::simulateGrayscale( int &r, int &g, int &b, int &red, int &green, int &blue )
{
  r = ( 0.2126 * red ) + ( 0.7152 * green ) + ( 0.0722 * blue );
  g = r;
  b = r;
}

void QgsPreviewEffect::simulateProtanope( int &r, int &g, int &b, int &red, int &green, int &blue )
{
  r = ( 0.152286 * red ) + ( 1.052583 * green ) + ( -0.204868 * blue );
  g = ( 0.114503 * red ) + ( 0.786281 * green ) + ( 0.099216 * blue );
  b = ( -0.003882 * red ) + ( -0.048116 * green ) + ( 1.051998 * blue );
}

void QgsPreviewEffect::simulateDeuteranope( int &r, int &g, int &b, int &red, int &green, int &blue )
{
  r = ( 0.367322 * red ) + ( 0.860646 * green ) + ( -0.227968 * blue );
  g = ( 0.280085 * red ) + ( 0.672501 * green ) + ( 0.047413 * blue );
  b = ( -0.011820 * red ) + ( 0.042940 * green ) + ( 0.968881 * blue );
}

void QgsPreviewEffect::simulateTritanope( int &r, int &g, int &b, int &red, int &green, int &blue )
{
  r = ( 1.255528 * red ) + ( -0.076749 * green ) + ( -0.178779 * blue );
  g = ( -0.078411 * red ) + ( 0.930809 * green ) + ( 0.147602 * blue );
  b = ( 0.004733 * red ) + ( 0.691367 * green ) + ( 0.303900 * blue );
}
