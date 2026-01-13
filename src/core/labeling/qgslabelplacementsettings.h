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

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"

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
     * Returns the label prioritization technique.
     *
     * \see setPrioritization()
     *
     * \since QGIS 3.38
     */
    Qgis::LabelPrioritization prioritization() const { return mPrioritization; }

    /**
     * Sets the technique used to prioritize labels.
     *
     * \see prioritization()
     *
     * \since QGIS 3.38
     */
    void setPrioritization( Qgis::LabelPrioritization prioritization ) { mPrioritization = prioritization; }

    /**
     * Returns the multipart labeling behavior.
     *
     * \see setMultiPartBehavior()
     *
     * \since QGIS 4.0
     */
    Qgis::MultiPartLabelingBehavior multiPartBehavior() const { return mMultiPartBehavior; }

    /**
     * Sets the multipart labeling \a behavior.
     *
     * \see multiPartBehavior()
     *
     * \since QGIS 4.0
     */
    void setMultiPartBehavior( Qgis::MultiPartLabelingBehavior behavior ) { mMultiPartBehavior = behavior; }

    /**
     * Updates the placement settings to respect any data defined properties
     * set within the specified \a properties collection.
     */
    void updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context );

  private:

    Qgis::LabelOverlapHandling mOverlapHandling = Qgis::LabelOverlapHandling::PreventOverlap;
    Qgis::LabelPrioritization mPrioritization = Qgis::LabelPrioritization::PreferCloser;

    bool mAllowDegradedPlacement = false;

    Qgis::MultiPartLabelingBehavior mMultiPartBehavior = Qgis::MultiPartLabelingBehavior::LabelLargestPartOnly;

};

#endif // QGSLABELPLACEMENTSETTINGS_H
