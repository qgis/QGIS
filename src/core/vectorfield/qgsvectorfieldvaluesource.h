/***************************************************************************
    qgsvectorfieldvaluesource.h
    ---------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORFIELDVALUESOURCE_H
#define QGSVECTORFIELDVALUESOURCE_H

#include <memory>

#include "qgis_core.h"
#include "qgspointxy.h"
#include "qgsrectangle.h"
#include "qgsvector.h"

#include <QSize>
#include <QVector>

#define SIP_NO_FILE

class QgsRasterInterface;
class QgsRenderContext;

/**
 * \ingroup core
 *
 * \brief Abstract source of vector (u/v) values sampled at arbitrary map positions.
 *
 * This is the data abstraction used by the streamline and particle trace symbologies of
 * QgsVectorFieldEngine, which walk the vector field rather than being handed one value at a
 * time. Implementations exist for mesh layers (interpolating from vertices or faces) and for
 * raster layers (sampling two bands).
 *
 * \note not available in Python bindings
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVectorFieldValueSource
{
  public:
    QgsVectorFieldValueSource() = default;
    virtual ~QgsVectorFieldValueSource();

    //! Returns a copy of this source
    virtual QgsVectorFieldValueSource *clone() const = 0;

    /**
     * Returns the vector value at \a point, expressed in map coordinates.
     *
     * Returns a vector with NaN components when no data is available at that position, which
     * is how the callers detect that a trace has left the data.
     */
    virtual QgsVector vectorValue( const QgsPointXY &point ) const = 0;

    //! Returns the extent of the data, in map coordinates
    virtual QgsRectangle extent() const = 0;

    /**
     * Returns the maximum magnitude over the whole data.
     *
     * This is used to nondimensionalize the time a trace spends in a field pixel, so it must be
     * stable for the whole rendering, not recomputed per tile or per extent.
     */
    virtual double maximumMagnitude() const = 0;

    /**
     * Returns the natural seeding positions of the data within \a extent, in map coordinates.
     *
     * These are the mesh vertices for a mesh source, or the pixel centers for a raster one.
     * The default implementation returns an empty list, for sources which have no natural
     * seeding positions.
     */
    virtual QVector<QgsPointXY> seedPoints( const QgsRectangle &extent ) const;

    /**
     * Returns a raster interface exposing the magnitude of the vector field as a single band,
     * used to build the color ramp background image of streamlines, or NULLPTR if the source
     * cannot provide one.
     *
     * \a context is a render context whose map to pixel is set to the field, and \a size is the
     * size in pixels of the image which will be requested from the returned interface.
     */
    virtual std::unique_ptr<QgsRasterInterface> magnitudeSource( const QgsRenderContext &context, QSize size ) const;
};

#endif // QGSVECTORFIELDVALUESOURCE_H
