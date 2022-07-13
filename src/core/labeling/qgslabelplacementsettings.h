/***************************************************************************
  qgslabelplacementsettings.h
  --------------------------
  Date                 : May 2022
  Copyright            : (C) 2022 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELPLACEMENTSETTINGS_H
#define QGSLABELPLACEMENTSETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

class QgsPropertyCollection;
class QgsExpressionContext;

/**
 * \ingroup core
 * \class QgsLabelPlacementSettings
 *
 * \brief Contains general settings related to how labels are placed.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsLabelPlacementSettings
{
  public:

    /**
     * Returns the technique used to handle overlapping labels.
     * \see setOverlapHandling()
     */
    Qgis::LabelOverlapHandling overlapHandling() const { return mOverlapHandling; }

    /**
     * Sets the technique used to handle overlapping labels.
     * \see overlapHandling()
     */
    void setOverlapHandling( Qgis::LabelOverlapHandling handling ) { mOverlapHandling = handling; }

    /**
     * Returns TRUE if labels can be placed in inferior fallback positions if they cannot otherwise
     * be placed.
     *
     * For instance, this will permit a curved line label to fallback to a horizontal label at the end of the line
     * if the label cannot otherwise be placed on the line in a curved manner.
     *
     * \see setAllowDegradedPlacement()
     */
    bool allowDegradedPlacement() const { return mAllowDegradedPlacement; }

    /**
     * Sets whether labels can be placed in inferior fallback positions if they cannot otherwise
     * be placed.
     *
     * For instance, this will permit a curved line label to fallback to a horizontal label at the end of the line
     * if the label cannot otherwise be placed on the line in a curved manner.
     *
     * \see allowDegradedPlacement()
     */
    void setAllowDegradedPlacement( bool allow ) { mAllowDegradedPlacement = allow; }

    /**
     * Updates the placement settings to respect any data defined properties
     * set within the specified \a properties collection.
     */
    void updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context );

  private:

    Qgis::LabelOverlapHandling mOverlapHandling = Qgis::LabelOverlapHandling::PreventOverlap;

    bool mAllowDegradedPlacement = false;

};

#endif // QGSLABELPLACEMENTSETTINGS_H
