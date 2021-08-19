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
     * Updates the thinning settings to respect any data defined properties
     * set within the specified \a properties collection.
     */
    void updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context );

  private:

    bool mLimitNumLabels = false;
    int mMaxNumLabels = 2000;
    double mMinFeatureSize = 0;
};

#endif // QGSLABELTHINNINGSETTINGS_H
