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
#include "qgis_sip.h"
#include <QColor>

#include "qgis_core.h"

/**
 * \ingroup core
    Bilinear Raster Resampler
*/
class CORE_EXPORT QgsBilinearRasterResampler: public QgsRasterResampler
{
  public:

    /**
     * Constructor for QgsBilinearRasterResampler.
     */
    QgsBilinearRasterResampler() = default;

    void resample( const QImage &srcImage, QImage &dstImage ) override;
    QString type() const override { return QStringLiteral( "bilinear" ); }
    QgsBilinearRasterResampler *clone() const override SIP_FACTORY;
};

#endif // QGSBILINEARRASTERRESAMPLER_H
