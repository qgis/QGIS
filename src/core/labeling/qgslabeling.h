/***************************************************************************
  qgslabeling.h
  --------------------------
  Date                 : January 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELING_H
#define QGSLABELING_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QFlags>

/**
 * \ingroup core
 * \class QgsLabeling
 *
 * \brief Contains constants and enums relating to labeling.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsLabeling
{
  public:

    /**
     * Line placement flags, which control how candidates are generated for a linear feature.
     */
    enum LinePlacementFlag
    {
      OnLine    = 1,      //!< Labels can be placed directly over a line feature.
      AboveLine = 2,      //!< Labels can be placed above a line feature. Unless MapOrientation is also specified this mode respects the direction of the line feature, so a line from right to left labels will have labels placed placed below the line feature.
      BelowLine = 4,      //!< Labels can be placed below a line feature. Unless MapOrientation is also specified this mode respects the direction of the line feature, so a line from right to left labels will have labels placed placed above the line feature.
      MapOrientation = 8, //!< Signifies that the AboveLine and BelowLine flags should respect the map's orientation rather than the feature's orientation. For example, AboveLine will always result in label's being placed above a line, regardless of the line's direction.
    };
    Q_DECLARE_FLAGS( LinePlacementFlags, LinePlacementFlag )

    /**
     * Polygon placement flags, which control how candidates are generated for a polygon feature.
     *
     * \since QGIS 3.14
     */
    enum PolygonPlacementFlag
    {
      AllowPlacementOutsideOfPolygon = 1 << 0, //!< Labels can be placed outside of a polygon feature
      AllowPlacementInsideOfPolygon = 1 << 1, //!< Labels can be placed inside a polygon feature
    };
    Q_DECLARE_FLAGS( PolygonPlacementFlags, PolygonPlacementFlag )

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLabeling::LinePlacementFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLabeling::PolygonPlacementFlags )

#endif // QGSLABELING_H
