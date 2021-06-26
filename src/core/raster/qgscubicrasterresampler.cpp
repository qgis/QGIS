/***************************************************************************
                         qgscubicrasterresampler.cpp
                         ----------------------------
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

#include "qgscubicrasterresampler.h"
#include "qgsgdalutils.h"
#include <QImage>
#include <cmath>

QgsCubicRasterResampler *QgsCubicRasterResampler::clone() const
{
  return new QgsCubicRasterResampler();
}

QImage QgsCubicRasterResampler::resampleV2( const QImage &source, const QSize &size )
{
  return QgsGdalUtils::resampleImage( source, size, GRIORA_Cubic );
}

Q_NOWARN_DEPRECATED_PUSH
void QgsCubicRasterResampler::resample( const QImage &srcImage, QImage &dstImage )
{
  dstImage = QgsGdalUtils::resampleImage( srcImage, dstImage.size(), GRIORA_Cubic );
}
Q_NOWARN_DEPRECATED_POP

QString QgsCubicRasterResampler::type() const
{
  return QStringLiteral( "cubic" );
}

int QgsCubicRasterResampler::tileBufferPixels() const
{
  return 2;
}

