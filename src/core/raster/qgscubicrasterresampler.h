/***************************************************************************
                         qgscubicrasterresampler.h
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

#ifndef QGSCUBICRASTERRESAMPLER_H
#define QGSCUBICRASTERRESAMPLER_H

#include "qgsrasterresampler.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QColor>

#include "qgis_core.h"

/**
 * \ingroup core
    Cubic Raster Resampler
*/
class CORE_EXPORT QgsCubicRasterResampler: public QgsRasterResamplerV2
{
  public:

    /**
     * Constructor for QgsCubicRasterResampler.
     */
    QgsCubicRasterResampler() = default;
    QgsCubicRasterResampler *clone() const override SIP_FACTORY;

    QImage resampleV2( const QImage &source, const QSize &size ) override;
    Q_DECL_DEPRECATED void resample( const QImage &srcImage, QImage &dstImage ) override SIP_DEPRECATED;
    QString type() const override;
    int tileBufferPixels() const override;
};

#endif // QGSCUBICRASTERRESAMPLER_H
