/***************************************************************************
                         qgsbilinearrasterresampler.h
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

#ifndef QGSBILINEARRASTERRESAMPLER_H
#define QGSBILINEARRASTERRESAMPLER_H

#include "qgsrasterresampler.h"
#include <QColor>

/** \ingroup core
    Bilinear Raster Resampler
*/
class CORE_EXPORT QgsBilinearRasterResampler: public QgsRasterResampler
{
  public:
    QgsBilinearRasterResampler();
    ~QgsBilinearRasterResampler();

    void resample( const QImage& srcImage, QImage& dstImage ) override;
    QString type() const override { return "bilinear"; }
    QgsRasterResampler * clone() const override;
};

#endif // QGSBILINEARRASTERRESAMPLER_H
