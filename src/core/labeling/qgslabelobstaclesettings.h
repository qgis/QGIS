/***************************************************************************
  qgslabelobstaclesettings.h
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

#ifndef QGSLABELOBSTACLESETTINGS_H
#define QGSLABELOBSTACLESETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsgeometry.h"

class QgsPropertyCollection;
class QgsExpressionContext;

/**
 * \ingroup core
 * \class QgsLabelObstacleSettings
 *
 * \brief Contains settings related to how the label engine treats features as obstacles
 *
 * \since QGIS 3.10.2
 */
class CORE_EXPORT QgsLabelObstacleSettings
{
  public:

    /**
     * Valid obstacle types, which affect how features within the layer will act as obstacles
     * for labels.
     */
    enum class ObstacleType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsLabelObstacleSettings, ObstacleType ) : int
      {
      PolygonInterior, //!< Avoid placing labels over interior of polygon (prefer placing labels totally outside or just slightly inside polygon)
      PolygonBoundary, //!< Avoid placing labels over boundary of polygon (prefer placing outside or completely inside polygon)
      PolygonWhole //!< Avoid placing labels over ANY part of polygon. Where PolygonInterior will prefer to place labels with the smallest area of intersection between the label and the polygon, PolygonWhole will penalise any label which intersects with the polygon by an equal amount, so that placing labels over any part of the polygon is avoided
    };

    /**
     * Returns TRUE if the features are obstacles to labels of other layers.
     * \see setIsObstacle()
     * \see factor()
     * \see type()
     */
    bool isObstacle() const
    {
      return mIsObstacle;
    }

    /**
     * Sets whether features are obstacles to labels of other layers.
     * \see isObstacle()
     * \see factor()
     * \see type()
     */
    void setIsObstacle( bool isObstacle )
    {
      mIsObstacle = isObstacle;
    }

    /**
     * Returns the obstacle factor, where 1.0 = default, < 1.0 more likely to be covered by labels,
     * > 1.0 less likely to be covered
     *
     * \see setFactor()
     * \see isObstacle()
     * \see type()
     */
    double factor() const
    {
      return mObstacleFactor;
    }

    /**
     * Sets the obstacle \a factor, where 1.0 = default, < 1.0 more likely to be covered by labels,
     * > 1.0 less likely to be covered
     *
     * \see factor()
     * \see isObstacle()
     * \see type()
     */
    void setFactor( double factor )
    {
      mObstacleFactor = factor;
    }

    /**
     * Returns how features act as obstacles for labels.
     * \see setType()
     * \see isObstacle()
     * \see factor()
     */
    ObstacleType type() const
    {
      return mObstacleType;
    }

    /**
     * Controls how features act as obstacles for labels.
     * \see type()
     * \see isObstacle()
     * \see factor()
     */
    void setType( ObstacleType type )
    {
      mObstacleType = type;
    }

    /**
     * Sets the label's obstacle geometry, if different to the feature geometry.
     * This can be used to override the shape of the feature for obstacle detection, e.g., to
     * buffer around a point geometry to prevent labels being placed too close to the
     * point itself. It not set, the feature's geometry is used for obstacle detection.
     *
     * \see obstacleGeometry()
     */
    void setObstacleGeometry( const QgsGeometry &obstacleGeom );

    /**
     * Returns the label's obstacle geometry, if different to the feature geometry.
     * \see setObstacleGeometry()
     */
    QgsGeometry obstacleGeometry() const;

    /**
     * Updates the obstacle settings to respect any data defined properties
     * set within the specified \a properties collection.
     */
    void updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context );

  private:

    bool mIsObstacle = true;
    double mObstacleFactor = 1.0;
    ObstacleType mObstacleType = ObstacleType::PolygonBoundary;

    //! Optional geometry to use for label obstacles
    QgsGeometry mObstacleGeometry;

};

#endif // QGSLABELOBSTACLESETTINGS_H
