/***************************************************************************
  qgslabellinesettings.h
  --------------------------
  Date                 : August 2020
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

#ifndef QGSLABELLINESETTINGS_H
#define QGSLABELLINESETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslabeling.h"

class QgsPropertyCollection;
class QgsExpressionContext;

/**
 * \ingroup core
 * \class QgsLabelLineSettings
 *
 * Contains settings related to how the label engine places and formats
 * labels for line features (or polygon features which are labeled in
 * a "perimeter" style mode).
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsLabelLineSettings
{
  public:

    /**
     * Returns the line placement flags, which dictate how line labels can be placed
     * above or below the lines.
     *
     * \see setPlacementFlags()
     */
    QgsLabeling::LinePlacementFlags placementFlags() const { return mPlacementFlags; }

    /**
     * Returns the line placement \a flags, which dictate how line labels can be placed
     * above or below the lines.
     *
     * \see placementFlags()
     */
    void setPlacementFlags( QgsLabeling::LinePlacementFlags flags ) { mPlacementFlags = flags; }

    /**
     * Updates the thinning settings to respect any data defined properties
     * set within the specified \a properties collection.
     */
    void updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context );

  private:
    QgsLabeling::LinePlacementFlags mPlacementFlags = QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::MapOrientation;

};

#endif // QGSLABELLINESETTINGS_H
