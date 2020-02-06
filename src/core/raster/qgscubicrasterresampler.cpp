/***************************************************************************
                         qgscubicrasterresampler.cpp
                         ----------------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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

