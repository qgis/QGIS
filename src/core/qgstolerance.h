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

#include <QObject>

#include "qgis_core.h"
#include "qgis.h"

class QgsMapSettings;
class QgsMapLayer;
class QgsPointXY;

/**
 * \ingroup core
 * \brief Provides calculations for tolerance values in map units.
 */
class CORE_EXPORT QgsTolerance
{
    Q_GADGET
  public:

    /**
     * Static function to get vertex tolerance value.
     * The value is read from settings and transformed if necessary.
     * \returns value of vertex tolerance in map units (not layer units)
     */
    static double vertexSearchRadius( const QgsMapSettings &mapSettings );

    /**
     * Static function to get vertex tolerance value for a layer.
     * The value is read from settings and transformed if necessary.
     * \returns value of vertex tolerance in layer units
     */
    static double vertexSearchRadius( QgsMapLayer *layer, const QgsMapSettings &mapSettings );

    /**
     * Static function to get default tolerance value for a layer.
     * The value is read from settings and transformed if necessary.
     * \returns value of default tolerance in layer units
     */
    static double defaultTolerance( QgsMapLayer *layer, const QgsMapSettings &mapSettings );

    /**
     * Static function to translate tolerance value into map units
     * \param tolerance tolerance value to be translated
     * \param layer source layer necessary in case tolerance is in layer units
     * \param mapSettings settings of the map
     * \param units type of units to be translated
     * \returns value of tolerance in map units
     */
    static double toleranceInProjectUnits( double tolerance, QgsMapLayer *layer, const QgsMapSettings &mapSettings, Qgis::MapToolUnit units );

    /**
     * Static function to translate tolerance value into layer units
     * \param tolerance tolerance value to be translated
     * \param layer reference layer
     * \param mapSettings settings of the map
     * \param units type of units to be translated
     * \returns value of tolerance in layer units
     */
    static double toleranceInMapUnits( double tolerance, QgsMapLayer *layer, const QgsMapSettings &mapSettings, Qgis::MapToolUnit units = Qgis::MapToolUnit::Layer );

  private:
    static double computeMapUnitPerPixel( QgsMapLayer *layer, const QgsMapSettings &mapSettings );
    static QgsPointXY toLayerCoordinates( QgsMapLayer *layer, const QgsMapSettings &mapSettings, QPoint point );

};

#endif
