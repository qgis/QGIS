/***************************************************************************
                         qgsrastervectorfieldrenderer.h
                         ------------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERVECTORFIELDRENDERER_H
#define QGSRASTERVECTORFIELDRENDERER_H

#include <limits>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterrenderer.h"
#include "qgsvectorfieldsettings.h"

/**
 * \ingroup core
 *
 * \brief Raster renderer which draws a vector field stored in raster bands.
 *
 * The way that the raster data is interpreted as vectors (eg. Cartesian component
 * per band, single band with cardinal direction) can be set using setSourceMode()
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsRasterVectorFieldRenderer : public QgsRasterRenderer
{
  public:
    //! Creates a vector field renderer
    explicit QgsRasterVectorFieldRenderer( QgsRasterInterface *input );
    ~QgsRasterVectorFieldRenderer() override;

    //! QgsRasterVectorFieldRenderer cannot be copied. Use clone() instead.
    QgsRasterVectorFieldRenderer( const QgsRasterVectorFieldRenderer & ) = delete;
    //! QgsRasterVectorFieldRenderer cannot be copied. Use clone() instead.
    const QgsRasterVectorFieldRenderer &operator=( const QgsRasterVectorFieldRenderer & ) = delete;

    QgsRasterVectorFieldRenderer *clone() const override SIP_FACTORY;
    Qgis::RasterRendererFlags flags() const override;

    //! Creates an instance of the renderer based on definition from XML (used by renderer registry)
    static std::unique_ptr<QgsRasterRenderer> create( const QDomElement &elem, QgsRasterInterface *input );

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    QList<int> usesBands() const override;
    QList<QPair<QString, QColor>> legendSymbologyItems() const override;

    /**
     * Returns the way the vectors are stored in the raster bands.
     *
     * \see setSourceMode()
     */
    Qgis::RasterVectorFieldSourceMode sourceMode() const { return mSourceMode; }

    /**
     * Sets the way the vectors are stored in the raster bands.
     *
     * \see sourceMode()
     */
    void setSourceMode( Qgis::RasterVectorFieldSourceMode mode ) { mSourceMode = mode; }

    /**
     * Returns the band which holds the coded compass directions, used when the sourceMode() is
     * Qgis::RasterVectorFieldSourceMode::EncodedDirection.
     *
     * \see setDirectionBand()
     */
    int directionBand() const { return mDirectionBand; }

    /**
     * Attempts to set the \a band which holds the coded compass directions.
     *
     * Returns TRUE if the band was successfully set.
     *
     * \see directionBand()
     */
    bool setDirectionBand( int band );

    /**
     * Returns the scheme the directionBand() is coded with.
     *
     * \see setDirectionEncoding()
     */
    Qgis::RasterDirectionEncoding directionEncoding() const { return mDirectionEncoding; }

    /**
     * Sets the scheme the directionBand() is coded with.
     *
     * \see directionEncoding()
     */
    void setDirectionEncoding( Qgis::RasterDirectionEncoding encoding ) { mDirectionEncoding = encoding; }

    /**
     * Returns the band which holds the x component of the vectors.
     *
     * \see setXBand()
     */
    int xBand() const { return mXBand; }

    /**
     * Attempts to set the \a band which holds the x component of the vectors.
     *
     * Returns TRUE if the band was successfully set.
     *
     * \see xBand()
     */
    bool setXBand( int band );

    /**
     * Returns the band which holds the y component of the vectors.
     *
     * \see setYBand()
     */
    int yBand() const { return mYBand; }

    /**
     * Attempts to set the \a band which holds the y component of the vectors.
     *
     * Returns TRUE if the band was successfully set.
     *
     * \see yBand()
     */
    bool setYBand( int band );

    /**
     * Returns the settings used to render the vector field.
     *
     * \see setSettings()
     */
    QgsVectorFieldSettings settings() const { return mSettings; }

    /**
     * Sets the \a settings used to render the vector field.
     *
     * \see settings()
     */
    void setSettings( const QgsVectorFieldSettings &settings ) { mSettings = settings; }

    /**
     * Returns the magnitude the symbology treats as the smallest one of the raster.
     *
     * \note Ignored when the sourceMode() is Qgis::RasterVectorFieldSourceMode::EncodedDirection
     *
     * \see setMinimumMagnitude()
     */
    double minimumMagnitude() const { return mMinimumMagnitude; }

    /**
     * Sets the \a magnitude the symbology treats as the smallest one of the raster.
     *
     * \see minimumMagnitude()
     */
    void setMinimumMagnitude( double magnitude ) { mMinimumMagnitude = magnitude; }

    /**
     * Returns the magnitude the symbology treats as the largest one of the raster.
     *
     * \note Ignored when the sourceMode() is Qgis::RasterVectorFieldSourceMode::EncodedDirection
     *
     * \see setMaximumMagnitude()
     */
    double maximumMagnitude() const { return mMaximumMagnitude; }

    /**
     * Sets the \a magnitude the symbology treats as the largest one of the raster.
     *
     * \see maximumMagnitude()
     */
    void setMaximumMagnitude( double magnitude ) { mMaximumMagnitude = magnitude; }

  private:
#ifdef SIP_RUN
    QgsRasterVectorFieldRenderer( const QgsRasterVectorFieldRenderer & );
    const QgsRasterVectorFieldRenderer &operator=( const QgsRasterVectorFieldRenderer & );
#endif

    bool shouldAcceptBand( int band ) const;

    Qgis::RasterVectorFieldSourceMode mSourceMode = Qgis::RasterVectorFieldSourceMode::CartesianComponents;
    int mXBand = 1;
    int mYBand = 2;
    int mDirectionBand = 1;
    Qgis::RasterDirectionEncoding mDirectionEncoding = Qgis::RasterDirectionEncoding::Esri;
    QgsVectorFieldSettings mSettings;
    double mMinimumMagnitude = std::numeric_limits<double>::quiet_NaN();
    double mMaximumMagnitude = std::numeric_limits<double>::quiet_NaN();
};

#endif // QGSRASTERVECTORFIELDRENDERER_H
