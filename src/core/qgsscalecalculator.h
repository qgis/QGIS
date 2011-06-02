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

#include <qgis.h>

class QString;
class QgsRectangle;

/** \ingroup core
 * Calculates scale for a given combination of canvas size, map extent,
 * and monitor dpi.
 */
class CORE_EXPORT QgsScaleCalculator
{
  public:

    /**
     * Constructor
     * @param dpi Monitor resolution in dots per inch
     * @param mapUnits Units of the data on the map. Must match a value from the
     * QGis::UnitType enum (Meters, Feet, Degrees)
     */
    QgsScaleCalculator( double dpi = 0,
                        QGis::UnitType mapUnits = QGis::Meters );

    //! Destructor
    ~QgsScaleCalculator();

    /**
     * Set the dpi to be used in scale calculations
     * @param dpi Dots per inch of monitor resolution
     */
    void setDpi( double dpi );
    /**
     * Accessor for dpi used in scale calculations
     * @return int the dpi used for scale calculations.
     */
    double dpi();

    /**
     * Set the map units
     * @param mapUnits Units of the data on the map. Must match a value from the
     */
    void setMapUnits( QGis::UnitType mapUnits );

    /** Returns current map units */
    QGis::UnitType mapUnits() const;

    /**
     * Calculate the scale
     * @param mapExtent QgsRectangle containing the current map extent
     * @param canvasWidth Width of the map canvas in pixel (physical) units
     * @return scale of current map view
     */
    double calculate( const QgsRectangle &mapExtent, int canvasWidth );

    /**
     * Calculate the distance between two points in geographic coordinates.
     * Used to calculate scale for map views with geographic (decimal degree)
     * data.
     * @param mapExtent QgsRectangle containing the current map extent
     */
    double calculateGeographicDistance( const QgsRectangle &mapExtent );

  private:

    //! dpi member
    double mDpi;

    //! map unit member
    QGis::UnitType mMapUnits;
};

#endif // #ifndef QGSSCALECALCULATOR_H
