/***************************************************************************
                         qgsbilinearrasterresampler.cpp
                         ------------------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsbilinearrasterresampler.h"
#include "qgsgdalutils.h"
#include <QImage>
#include <cmath>

QgsBilinearRasterResampler *QgsBilinearRasterResampler::clone() const
{
  return new QgsBilinearRasterResampler();
}

int QgsBilinearRasterResampler::tileBufferPixels() const
{
  return 1;
}

Q_NOWARN_DEPRECATED_PUSH
void QgsBilinearRasterResampler::resample( const QImage &srcImage, QImage &dstImage )
{
  dstImage = QgsGdalUtils::resampleImage( srcImage, dstImage.size(), GRIORA_Bilinear );
}
Q_NOWARN_DEPRECATED_POP

QImage QgsBilinearRasterResampler::resampleV2( const QImage &source, const QSize &size )
{
  return source.scaled( size.width(), size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}

QString QgsBilinearRasterResampler::type() const
{
  return QStringLiteral( "bilinear" );
}
