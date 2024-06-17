/***************************************************************************
  qgslabelpointsettings.h
  --------------------------
  Date                 : May 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELPOINTSETTINGS_H
#define QGSLABELPOINTSETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmapunitscale.h"
#include <QString>

class QgsPropertyCollection;
class QgsExpressionContext;

/**
 * \ingroup core
 * \class QgsLabelPointSettings
 *
 * \brief Contains settings related to how the label engine places and formats
 * labels for point features, or polygon features which are labeled in
 * the "around" or "over" centroid placement modes.
 *
 * \since QGIS 3.38
 */
class CORE_EXPORT QgsLabelPointSettings
{
    Q_GADGET

  public:

    /**
     * Updates the point settings to respect any data defined properties
     * set within the specified \a properties collection.
     */
    void updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context );

    /**
     * Returns the quadrant in which to offset labels from the point.
     *
     * \see setQuadrant()
    */
    Qgis::LabelQuadrantPosition quadrant() const { return mQuadrant; }

    /**
     * Sets the \a quadrant in which to offset labels from the point.
     *
     * \see quadrant()
    */
    void setQuadrant( Qgis::LabelQuadrantPosition quadrant ) { mQuadrant = quadrant; }

    /**
     * Returns the ordered list of predefined label positions for points.
     *
     * Positions earlier in the list will be prioritized over later positions.
     * Only used when the placement is set to Qgis::LabelPlacement::OrderedPositionsAroundPoint.
     *
     * \see setPredefinedPositionOrder()
    */
    QVector< Qgis::LabelPredefinedPointPosition > predefinedPositionOrder() const { return mPredefinedPositionOrder; }

    /**
     * Sets the ordered list of predefined label positions for points.
     *
     * Positions earlier in the list will be prioritized over later positions.
     * Only used when the placement is set to Qgis::LabelPlacement::OrderedPositionsAroundPoint.
     *
     * \see predefinedPositionOrder()
    */
    void setPredefinedPositionOrder( const QVector< Qgis::LabelPredefinedPointPosition > &order ) { mPredefinedPositionOrder = order; }

    /**
     * Returns the maximum distance which labels are allowed to be from their corresponding points.
     *
     * This setting works alongside the standard label offset distance properties to define a permissible
     * range of distances at which labels can be placed from their points.
     *
     * The default value is 0, which indicates that no maximum is set and the label's usual distance
     * from point will always be respected.
     *
     * \see setMaximumDistance()
     * \see maximumDistanceUnit()
     * \see maximumDistanceMapUnitScale()
     */
    double maximumDistance() const { return mMaximumDistance; }

    /**
     * Sets the maximum \a distance which labels are allowed to be from their corresponding points.
     *
     * This setting works alongside the standard label offset distance properties to define a permissible
     * range of distances at which labels can be placed from their points.
     *
     * Setting \a distance to 0 indicates that no maximum is set and the label's usual distance
     * from point will always be respected.
     *
     * \see maximumDistance()
     * \see maximumDistanceUnit()
     * \see maximumDistanceMapUnitScale()
     */
    void setMaximumDistance( double distance ) { mMaximumDistance = distance; }

    /**
     * Returns the units for label maximum distance.
     *
     * \see setMaximumDistanceUnit()
     * \see maximumDistance()
     * \see maximumDistanceMapUnitScale()
     */
    Qgis::RenderUnit maximumDistanceUnit() const { return mMaximumDistanceUnit; }

    /**
     * Sets the \a unit for label maximum distance.
     *
     * \see maximumDistanceUnit()
     * \see maximumDistance()
     * \see maximumDistanceMapUnitScale()
     */
    void setMaximumDistanceUnit( Qgis::RenderUnit unit ) { mMaximumDistanceUnit = unit;}

    /**
     * Returns the map unit scale for label maximum distance.
     *
     * \see setMaximumDistanceMapUnitScale()
     * \see maximumDistance()
     * \see maximumDistanceUnit()
     */
    QgsMapUnitScale maximumDistanceMapUnitScale() const { return mMaximumDistanceMapUnitScale; }

    /**
     * Sets the map unit \a scale for label maximum distance.
     *
     * \see maximumDistanceMapUnitScale()
     * \see maximumDistance()
     * \see maximumDistanceUnit()
     */
    void setMaximumDistanceMapUnitScale( const QgsMapUnitScale &scale ) { mMaximumDistanceMapUnitScale = scale; }

  private:

    Qgis::LabelQuadrantPosition mQuadrant = Qgis::LabelQuadrantPosition::Over;

    QVector< Qgis::LabelPredefinedPointPosition > mPredefinedPositionOrder;

    double mMaximumDistance = 0;
    Qgis::RenderUnit mMaximumDistanceUnit = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mMaximumDistanceMapUnitScale;

};

#endif // QGSLABELPOINTSETTINGS_H
