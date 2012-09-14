/***************************************************************************
                         qgsrasterresampler.h
                         --------------------
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

#ifndef QGSRASTERRESAMPLER_H
#define QGSRASTERRESAMPLER_H

#include <QString>

class QImage;

/**Interface for resampling rasters (e.g. to have a smoother appearance)*/
class QgsRasterResampler
{
  public:
    virtual ~QgsRasterResampler() {}
    virtual void resample( const QImage& srcImage, QImage& dstImage ) = 0;
    virtual QString type() const = 0;
    virtual QgsRasterResampler * clone() const = 0;
};

#endif // QGSRASTERRESAMPLER_H
