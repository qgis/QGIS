/***************************************************************************
  qgslabelthinningsettings.h
  --------------------------
  Date                 : December 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELTHINNINGSETTINGS_H
#define QGSLABELTHINNINGSETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmapunitscale.h"

class QgsPropertyCollection;
class QgsExpressionContext;

/**
 * \ingroup core
 * \class QgsLabelThinningSettings
 *
 * \brief Contains settings related to how the label engine removes candidate label positions and reduces the number
 * of displayed labels.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsLabelThinningSettings
{
  public:

    /**
     * Returns TRUE if the number of labels drawn for the layer should be limited.
     * \see maximumNumberLabels()
     * \see setLimitNumberLabelsEnabled()
     */
    bool limitNumberOfLabelsEnabled() const { return mLimitNumLabels; }

    /**
     * Sets whether the the number of labels drawn for the layer should be limited.
     * \see setMaximumNumberLabels()
     * \see limitNumberOfLabelsEnabled()
     */
    void setLimitNumberLabelsEnabled( bool enabled ) { mLimitNumLabels = enabled; }

    /**
     * Returns the maximum number of labels which should be drawn for this layer.
     * This only has an effect if limitNumberOfLabelsEnabled() is TRUE.
     * \see limitNumberOfLabelsEnabled()
     * \see setMaximumNumberLabels()
     */
    int maximumNumberLabels() const { return mMaxNumLabels; }

    /**
     * Sets the maximum \a number of labels which should be drawn for this layer.
     * This only has an effect if limitNumberOfLabelsEnabled() is TRUE.
     * \see setLimitNumberLabelsEnabled()
     * \see maximumNumberLabels()
     */
    void setMaximumNumberLabels( int number ) { mMaxNumLabels = number; }

    /**
     * Returns the minimum feature size (in millimeters) for a feature to be labelled.
     * \see setMinimumFeatureSize()
     */
    double minimumFeatureSize() const { return mMinFeatureSize; }

    /**
     * Sets the minimum feature \a size (in millimeters) for a feature to be labelled.
     * \see minimumFeatureSize()
     */
    void setMinimumFeatureSize( double size ) { mMinFeatureSize = size; }

    /**
      * Returns the minimum distance to other labels (i.e. the minimum space/margin around labels).
      *
      * Units are specified through labelMarginDistanceUnit().
      *
      * \see setLabelMarginDistance()
      * \see labelMarginDistanceUnit()
      *
      * \since QGIS 3.44
      */
    double labelMarginDistance() const { return mLabelMarginDistance; }

    /**
      * Sets the minimum \a distance to other labels (i.e. the minimum space/margin around labels).
      *
      * Units are specified through setLabelMarginDistanceUnit().
      *
      * \see labelMarginDistance()
      * \see setLabelMarginDistanceUnit()
      *
      * \since QGIS 3.44
      */
    void setLabelMarginDistance( double distance ) { mLabelMarginDistance = distance; }

    /**
      * Sets the \a unit for the minimum distance to other labels.
      *
      * \see labelMarginDistanceUnit()
      * \see setLabelMarginDistance()
      *
      * \since QGIS 3.44
     */
    void setLabelMarginDistanceUnit( Qgis::RenderUnit unit ) { mLabelMarginDistanceUnits = unit; }

    /**
      * Returns the units for the minimum distance to other labels.
      *
      * \see setLabelMarginDistanceUnit()
      * \see labelMarginDistance()
      *
      * \since QGIS 3.44
     */
    Qgis::RenderUnit labelMarginDistanceUnit() const { return mLabelMarginDistanceUnits; }

    /**
      * Sets the map unit \a scale for the minimum distance to other labels.
      *
      * \see labelMarginDistanceMapUnitScale()
      * \see labelMarginDistanceUnit()
      * \see setLabelMarginDistance()
      *
      * \since QGIS 3.44
      */
    void setLabelMarginDistanceMapUnitScale( const QgsMapUnitScale &scale ) { mLabelMarginDistanceScale = scale; }

    /**
      * Returns the map unit scale for the minimum distance to other labels.
      *
      * \see setLabelMarginDistanceMapUnitScale()
      * \see labelMarginDistanceUnit()
      * \see labelMarginDistance()
      *
      * \since QGIS 3.44
      */
    const QgsMapUnitScale &labelMarginDistanceMapUnitScale() const { return mLabelMarginDistanceScale; }

#ifndef SIP_RUN
    //! Default minimum distance to duplicate labels
    static constexpr double DEFAULT_MINIMUM_DISTANCE_TO_DUPLICATE = 20;
#endif

    /**
     * Returns the minimum distance to labels with duplicate text.
     *
     * Units are specified through minimumDistanceToDuplicateUnit().
     *
     * \note This setting is only respected if allowDuplicateRemoval() is TRUE.
     *
     * \see setMinimumDistanceToDuplicate()
     * \see minimumDistanceToDuplicateUnit()
     *
     * \since QGIS 3.44
     */
    double minimumDistanceToDuplicate() const { return mMinDistanceToDuplicate; }

    /**
     * Sets the minimum \a distance to labels with duplicate text.
     *
     * Units are specified through setMinimumDistanceToDuplicateUnit().
     *
     * \note This setting is only respected if allowDuplicateRemoval() is TRUE.
     *
     * \see minimumDistanceToDuplicate()
     * \see setMinimumDistanceToDuplicateUnit()
     *
     * \since QGIS 3.44
     */
    void setMinimumDistanceToDuplicate( double distance ) { mMinDistanceToDuplicate = distance; }

    /**
     * Sets the \a unit for the minimum distance to labels with duplicate text.
     *
     * \note This setting is only respected if allowDuplicateRemoval() is TRUE.
     *
     * \see minimumDistanceToDuplicateUnit()
     * \see setMinimumDistanceToDuplicate()
     *
     * \since QGIS 3.44
    */
    void setMinimumDistanceToDuplicateUnit( Qgis::RenderUnit unit ) { mMinDistanceToDuplicateUnits = unit; }

    /**
     * Returns the units for the minimum distance to labels with duplicate text.
     *
     * \note This setting is only respected if allowDuplicateRemoval() is TRUE.
     *
     * \see setMinimumDistanceToDuplicateUnit()
     * \see minimumDistanceToDuplicate()
     *
     * \since QGIS 3.44
    */
    Qgis::RenderUnit minimumDistanceToDuplicateUnit() const { return mMinDistanceToDuplicateUnits; }

    /**
     * Sets the map unit \a scale for the minimum distance to labels with duplicate text.
     *
     * \note This setting is only respected if allowDuplicateRemoval() is TRUE.
     *
     * \see minimumDistanceToDuplicateMapUnitScale()
     * \see minimumDistanceToDuplicateUnit()
     * \see setMinimumDistanceToDuplicate()
     *
     * \since QGIS 3.44
     */
    void setMinimumDistanceToDuplicateMapUnitScale( const QgsMapUnitScale &scale ) { mMinDistanceToDuplicateScale = scale; }

    /**
     * Returns the map unit scale for the minimum distance to labels with duplicate text.
     *
     * \note This setting is only respected if allowDuplicateRemoval() is TRUE.
     *
     * \see setMinimumDistanceToDuplicateMapUnitScale()
     * \see minimumDistanceToDuplicateUnit()
     * \see minimumDistanceToDuplicate()
     *
     * \since QGIS 3.44
     */
    const QgsMapUnitScale &minimumDistanceToDuplicateMapUnitScale() const { return mMinDistanceToDuplicateScale; }

    /**
     * Returns whether duplicate label removal is permitted for this layer.
     * \see setAllowDuplicateRemoval()
     *
     * \since QGIS 3.44
     */
    bool allowDuplicateRemoval() const { return mAllowDuplicateRemoval; }

    /**
     * Sets whether duplicate label removal is permitted for this layer.
     * \see setAllowDuplicateRemoval()
     *
     * \since QGIS 3.44
     */
    void setAllowDuplicateRemoval( bool allow ) { mAllowDuplicateRemoval = allow; }

    /**
     * Updates the thinning settings to respect any data defined properties
     * set within the specified \a properties collection.
     */
    void updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context );

  private:

    bool mLimitNumLabels = false;
    int mMaxNumLabels = 2000;
    double mMinFeatureSize = 0;

    double mLabelMarginDistance = 0;
    Qgis::RenderUnit mLabelMarginDistanceUnits = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mLabelMarginDistanceScale;

    bool mAllowDuplicateRemoval = false;
    double mMinDistanceToDuplicate = QgsLabelThinningSettings::DEFAULT_MINIMUM_DISTANCE_TO_DUPLICATE;
    Qgis::RenderUnit mMinDistanceToDuplicateUnits = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mMinDistanceToDuplicateScale;
};

#ifndef SIP_RUN

/**
  * \ingroup core
  * \class QgsLabelFeatureThinningSettings
  *
  * Contains settings related to how the label engine removes candidate label positions and reduces the number
  * of displayed labels for one particular label feature.
  *
  * \since QGIS 3.44
  */
class CORE_EXPORT QgsLabelFeatureThinningSettings
{
  public:

    /**
      * Returns the minimum distance (in label units) between labels for this
      * feature and other labels.
      * \see setLabelMarginDistance()
      */
    double labelMarginDistance() const { return mLabelMarginDistance; }

    /**
      * Sets the minimum \a distance (in label units) between labels for this
      * feature and other labels.
      * \see labelMarginDistance()
      */
    void setLabelMarginDistance( double distance ) { mLabelMarginDistance = distance; }

    /**
     * Returns the minimum distance (in label units) between labels for this
     * feature and other labels with the same label text.
     * \see setNoRepeatDistance()
     */
    double noRepeatDistance() const { return mNoRepeatDistance; }

    /**
     * Sets the minimum \a distance (in label units) between labels for this
     * feature and other labels with the same label text.
     * \see noRepeatDistance()
     */
    void setNoRepeatDistance( double distance ) { mNoRepeatDistance = distance; }

  private:

    double mLabelMarginDistance = 0;
    double mNoRepeatDistance = 0;

};

#endif

#endif // QGSLABELTHINNINGSETTINGS_H
