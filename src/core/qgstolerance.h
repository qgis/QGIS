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
#include "qgsmaptopixel.h"
#include "qgsmaprenderer.h"
#include "qgsmaplayer.h"
#include "qgspoint.h"

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
    * Static function to get vertex tolerance value for a layer.
    * The value is read from settings and transformed if necessary.
    * @return value of vertex tolerance in map units
    */
    static double vertexSearchRadius( QgsMapLayer* layer, const QgsMapSettings& mapSettings );

    /**
    * Static function to get vertex tolerance value for a layer.
    * The value is read from settings and transformed if necessary.
    * @return value of vertex tolerance in map units
    */
    //! @deprecated since 2.4 - use override with QgsMapSettings
    Q_DECL_DEPRECATED static double vertexSearchRadius( QgsMapLayer* layer, QgsMapRenderer* renderer );

    /**
    * Static function to get default tolerance value for a layer.
    * The value is read from settings and transformed if necessary.
    * @return value of default tolerance in map units
    */
    static double defaultTolerance( QgsMapLayer* layer, const QgsMapSettings& mapSettings );

    /**
    * Static function to get default tolerance value for a layer.
    * The value is read from settings and transformed if necessary.
    * @return value of default tolerance in map units
    */
    //! @deprecated since 2.4 - use override with QgsMapSettings
    Q_DECL_DEPRECATED static double defaultTolerance( QgsMapLayer* layer, QgsMapRenderer* renderer );

    /**
    * Static function to translate tolerance value into current map unit value
    * @param tolerance tolerance value to be translated
    * @param layer reference layer
    * @param mapSettings settings of the map
    * @param units type of units to be translated
    * @return value of tolerance in map units
    */
    static double toleranceInMapUnits( double tolerance, QgsMapLayer* layer, const QgsMapSettings& mapSettings, UnitType units = MapUnits );

    /**
    * Static function to translate tolerance value into current map unit value
    * @param tolerance tolerance value to be translated
    * @param layer reference layer
    * @param renderer renderer
    * @param units type of units to be translated
    * @return value of tolerance in map units
    */
    //! @deprecated since 2.4 - use the override with QgsMapSettings
    Q_DECL_DEPRECATED static double toleranceInMapUnits( double tolerance, QgsMapLayer* layer, QgsMapRenderer* renderer, UnitType units = MapUnits );

  private:
    static double computeMapUnitPerPixel( QgsMapLayer* layer, const QgsMapSettings& mapSettings );
    static QgsPoint toLayerCoordinates( QgsMapLayer* layer, const QgsMapSettings& mapSettings, const QPoint& point );

};

#endif
