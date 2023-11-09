/***************************************************************************
                          qgsscalecalculator.h
              Calculates scale based on map extent and units
                             -------------------
    begin                : May 18, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSCALECALCULATOR_H
#define QGSSCALECALCULATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

class QString;
class QgsRectangle;

/**
 * \ingroup core
 * \brief Calculates scale for a given combination of canvas size, map extent,
 * and monitor dpi.
 */
class CORE_EXPORT QgsScaleCalculator
{
  public:

    /**
     * Constructor
     * \param dpi Monitor resolution in dots per inch
     * \param mapUnits Units of the data on the map
     */
    QgsScaleCalculator( double dpi = 0,
                        Qgis::DistanceUnit mapUnits = Qgis::DistanceUnit::Meters );

    /**
     * Sets the \a dpi (dots per inch) for the output resolution, to be used in scale calculations.
     * \see dpi()
     */
    void setDpi( double dpi );

    /**
     * Returns the DPI (dots per inch) used in scale calculations.
     * \see setDpi()
     */
    double dpi() const;

    /**
     * Set the map units.
     *
     * \see mapUnits()
     */
    void setMapUnits( Qgis::DistanceUnit mapUnits );

    /**
     * Returns current map units.
     *
     * \see setMapUnits()
     */
    Qgis::DistanceUnit mapUnits() const;

    /**
     * Calculate the scale denominator.
     *
     * \param mapExtent QgsRectangle containing the current map extent. Units are specified by mapUnits().
     * \param canvasWidth Width of the map canvas in pixel (physical) units
     *
     * \returns scale denominator of current map view, e.g. 1000.0 for a 1:1000 map.
     */
    double calculate( const QgsRectangle &mapExtent, double canvasWidth ) const;

    /**
     * Calculate the image size in pixel (physical) units.
     *
     * \param mapExtent QgsRectangle containing the current map extent. Units are specified by mapUnits()
     * \param scale Scale denominator, e.g. 1000.0 for a 1:1000 map
     *
     * \returns image size
     * \since QGIS 3.24
     */
    QSizeF calculateImageSize( const QgsRectangle &mapExtent, double scale ) const;

    /**
     * Calculate the distance between two points in geographic coordinates.
     * Used to calculate scale for map views with geographic (decimal degree)
     * data.
     * \param mapExtent QgsRectangle containing the current map extent
     */
    double calculateGeographicDistance( const QgsRectangle &mapExtent ) const;

  private:

    //! Calculate the \a delta and \a conversionFactor values based on the provided \a mapExtent.
    void calculateMetrics( const QgsRectangle &mapExtent, double &delta, double &conversionFactor ) const;

    //! dpi member
    double mDpi = 96;

    //! map unit member
    Qgis::DistanceUnit mMapUnits = Qgis::DistanceUnit::Unknown;
};

#endif // #ifndef QGSSCALECALCULATOR_H
