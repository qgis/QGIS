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
#include "qgis.h"
#include "qgsunittypes.h"

class QString;
class QgsRectangle;

/**
 * \ingroup core
 * Calculates scale for a given combination of canvas size, map extent,
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
                        QgsUnitTypes::DistanceUnit mapUnits = QgsUnitTypes::DistanceMeters );

    /**
     * Sets the \a dpi (dots per inch) for the output resolution, to be used in scale calculations.
     * \see dpi()
     */
    void setDpi( double dpi );

    /**
     * Returns the DPI (dots per inch) used in scale calculations.
     * \see setDpi()
     */
    double dpi();

    /**
     * Set the map units
     * \param mapUnits Units of the data on the map. Must match a value from the
     */
    void setMapUnits( QgsUnitTypes::DistanceUnit mapUnits );

    //! Returns current map units
    QgsUnitTypes::DistanceUnit mapUnits() const;

    /**
     * Calculate the scale denominator
     * \param mapExtent QgsRectangle containing the current map extent
     * \param canvasWidth Width of the map canvas in pixel (physical) units
     * \returns scale denominator of current map view, e.g. 1000.0 for a 1:1000 map.
     */
    double calculate( const QgsRectangle &mapExtent, double canvasWidth );

    /**
     * Calculate the distance between two points in geographic coordinates.
     * Used to calculate scale for map views with geographic (decimal degree)
     * data.
     * \param mapExtent QgsRectangle containing the current map extent
     */
    double calculateGeographicDistance( const QgsRectangle &mapExtent );

  private:

    //! dpi member
    double mDpi;

    //! map unit member
    QgsUnitTypes::DistanceUnit mMapUnits;
};

#endif // #ifndef QGSSCALECALCULATOR_H
