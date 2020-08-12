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
#include "qgis_core.h"
#include "qgis_sip.h"

class QString;
class QImage;
class QSize;

/**
 * \ingroup core
  * Interface for resampling rasters (e.g. to have a smoother appearance)
  */
class CORE_EXPORT QgsRasterResampler
{
#ifdef SIP_RUN
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"
#endif


#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsBilinearRasterResampler *>( sipCpp ) != NULL )
      sipType = sipType_QgsBilinearRasterResampler;
    else if ( dynamic_cast<QgsCubicRasterResampler *>( sipCpp ) != NULL )
      sipType = sipType_QgsCubicRasterResampler;
    else
      sipType = 0;
    SIP_END
#endif

  public:
    virtual ~QgsRasterResampler() = default;

    /**
     * Resamples a source image to a destination image.
     *
     * The size of the passed destination image should be respected during the resampling
     * process.
     *
     * \deprecated since QGIS 3.10.1, use the more efficient QgsRasterResamplerV2 interface instead.
     */
    Q_DECL_DEPRECATED virtual void resample( const QImage &srcImage, QImage &dstImage ) = 0 SIP_DEPRECATED;

    /**
     * Gets a descriptive type identifier for this raster resampler.
     * Needs to be implemented by subclasses.
     */
    virtual QString type() const = 0;

    /**
     * Gets a deep copy of this object.
     * Needs to be reimplemented by subclasses.
     * Ownership is transferred to the caller.
     */
    virtual QgsRasterResampler *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the optional tile buffer size in pixels. This represents
     * the size to buffer individual resampled tile requests prior to resampling,
     * in order to avoid rendering artifacts at the edges of raster tile boundaries.
     *
     * \since QGIS 3.10.1
     */
    virtual int tileBufferPixels() const { return 0; }
};


/**
 * \ingroup core
  * Interface for resampling rasters (V2) (e.g. to have a smoother appearance),
  * which provides a more efficient interface vs QgsRasterResampler.
  *
  * \since QGIS 3.10.1
  */
class CORE_EXPORT QgsRasterResamplerV2 : public QgsRasterResampler
{
  public:

    /**
     * Resamples a \a source image to the specified \a size.
     *
     * Returns the resampled image, or a null QImage if the resampling fails.
     */
    virtual QImage resampleV2( const QImage &source, const QSize &size ) = 0;

};

#endif // QGSRASTERRESAMPLER_H
