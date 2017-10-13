/***************************************************************************
  qgsmapunitscale.h
  Struct for storing maximum and minimum scales for measurements in map units
  -------------------
   begin                : April 2014
   copyright            : (C) Sandro Mani
   email                : smani at sourcepole dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPUNITSCALE_H
#define QGSMAPUNITSCALE_H

#include "qgis_core.h"
#include "qgis.h"
#include <QtCore>

class QgsRenderContext;

/**
 * \ingroup core
 * \class QgsMapUnitScale
 * \brief Struct for storing maximum and minimum scales for measurements in map units
 *
 * For measurements in map units, a minimum and a maximum scale can be defined.
 * Outside this range, the measurements aren't scaled anymore proportionally to
 * the map scale.
 */

class CORE_EXPORT QgsMapUnitScale
{
  public:

    /**
     * Constructor for QgsMapUnitScale
     * \param minScale minimum allowed scale, or 0.0 if no minimum scale set
     * \param maxScale maximum allowed scale, or 0.0 if no maximum scale set
     * The scale values indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     */
    explicit QgsMapUnitScale( double minScale = 0.0, double maxScale = 0.0 )
      : minScale( minScale )
      , maxScale( maxScale )
    {}

    /**
     * The minimum scale, or 0.0 if unset.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     */
    double minScale;

    /**
     * The maximum scale, or 0.0 if unset.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     */
    double maxScale;

    //! Whether the minimum size in mm should be respected
    bool minSizeMMEnabled = false;
    //! The minimum size in millimeters, or 0.0 if unset
    double minSizeMM = 0.0;
    //! Whether the maximum size in mm should be respected
    bool maxSizeMMEnabled = false;
    //! The maximum size in millimeters, or 0.0 if unset
    double maxSizeMM = 0.0;

    /**
     * Computes a map units per pixel scaling factor, respecting the minimum and maximum scales
     * set for the object.
     * \param c render context
     * \returns map units per pixel, limited between minimum and maximum scales
     */
    double computeMapUnitsPerPixel( const QgsRenderContext &c ) const;

    bool operator==( const QgsMapUnitScale &other ) const
    {
      return qgsDoubleNear( minScale, other.minScale )
             && qgsDoubleNear( maxScale, other.maxScale )
             && minSizeMMEnabled == other.minSizeMMEnabled
             && qgsDoubleNear( minSizeMM, other.minSizeMM )
             && maxSizeMMEnabled == other.maxSizeMMEnabled
             && qgsDoubleNear( maxSizeMM, other.maxSizeMM );
    }

    bool operator!=( const QgsMapUnitScale &other ) const
    {
      return !operator==( other );
    }
};


#endif // QGSMAPUNITSCALE_H



