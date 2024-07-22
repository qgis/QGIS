/***************************************************************************
                         qgscubicrasterresampler.h
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

#ifndef QGSCUBICRASTERRESAMPLER_H
#define QGSCUBICRASTERRESAMPLER_H

#include "qgsrasterresampler.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QColor>

#include "qgis_core.h"

/**
 * \ingroup core
 * \brief Cubic Raster Resampler.
 */
class CORE_EXPORT QgsCubicRasterResampler: public QgsRasterResamplerV2
{
  public:

    QgsCubicRasterResampler() = default;
    QgsCubicRasterResampler *clone() const override SIP_FACTORY;

    QImage resampleV2( const QImage &source, const QSize &size ) override;
    Q_DECL_DEPRECATED void resample( const QImage &srcImage, QImage &dstImage ) override SIP_DEPRECATED;
    QString type() const override;
    int tileBufferPixels() const override;
};

#endif // QGSCUBICRASTERRESAMPLER_H
