/***************************************************************************
    qgstolerance.h  -  wrapper for tolerance handling
    ----------------------
    begin                : March 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf.kostej at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTOLERANCE_H
#define QGSTOLERANCE_H


/** \ingroup core
 * This is the class is providing tolerance value in map unit values.
 *
 * \note This class has been added in version 1.1.
 */
class CORE_EXPORT QgsTolerance
{

  public:
    /**Type of unit of tolerance value from settings*/
    enum UnitType 
    { 
      /**Map unit value*/
      MapUnits,
      /**Pixels unit of tolerance*/
      Pixels
    };

    /**
    * Static function to get vertex tolerance value from settings
    * @param mapUnitsPerPixel number of map units per pixel
    * @return value of vertex tolerance in map units
    */
    static double vertexSearchRadius( double mapUnitsPerPixel );

    /**
    * Static function to get default tolerance value from settings
    * @param mapUnitsPerPixel number of map units per pixel
    * @return value of default tolerance in map units
    */
    static double defaultTolerance( double mapUnitsPerPixel );

    /** 
    * Static function to translate tolerance value into current map unit value
    * @param tolerace tolerance value to be translated
    * @param mapUnitsPerPixel number of map units per pixel
    * @param units type of units to be translated
    * @return value of tolerance in map units
    */
    static double toleranceInMapUnits(double tolerance, double mapUnitsPerPixel, UnitType units = MapUnits);

};

#endif
