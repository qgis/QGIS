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
/* $Id$ */
#ifndef QGSSCALECALCULATOR_H
#define QGSSCALECALCULATOR_H
class QString;
class QgsRect;
/**
 * \class QgsScaleCalculator
 * \brief Calculates scale for a given combination of canvas size, map extent,
 * and monitor dpi.
 */
class QgsScaleCalculator{
  public:
    /**
     * Constructor
     * @param dpi Monitor resolution in dots per inch
     * @param mapUnits Units of the data on the map. Must match a value from the
     * QgsScaleCalculator::units enum (METERS, FEET, DEGREES)
     */
    QgsScaleCalculator(int dpi=0, int mapUnits=0);
    //! Destructor
    ~QgsScaleCalculator();
    /**
     * Set the dpi to be used in scale calculations
     * @param dpi Dots per inch of monitor resolution
     */
    void setDpi(int dpi);
    /**
     * Set the map units
     * @param mapUnits Units of the data on the map. Must match a value from the
     */
    void setMapUnits(int mapUnits);
    /**
     * Calculate the scale
     * @param mapExtent QgsRect containing the current map extent
     * @param canvasWidth Width of the map canvas in pixel (physical) units
     * @return scale of current map view
     */
    double calculate(QgsRect &mapExtent, int canvasWidth);
    /**
     * Calculate the distance between to points in geographic coordinates.
     * Used to calculate scale for map views with geographic (decimal degree)
     * data.
     * @param mapExtent QgsRect containing the current map extent
     */
    double calculateGeographicDistance(QgsRect &mapExtent);

    /**
     * Enum for defining map units
     */
    enum units{
      METERS,
      FEET,
      DEGREES
    };

  private:
    //! map unit member
    int mMapUnits;
    //! dpi member
    int mDpi;

};

#endif // #ifndef QGSSCALECALCULATOR_H
