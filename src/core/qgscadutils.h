/***************************************************************************
                              qgscadutils.h
                             -------------------
    begin                : September 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCADUTILS_H
#define QGSCADUTILS_H

#include "qgis_core.h"

#include "qgspointlocator.h"

class QgsSnappingUtils;

/**
 * \ingroup core
 * The QgsCadUtils class provides routines for CAD editing.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsCadUtils
{
  public:

    //! Structure with details of one constraint
    struct AlignMapPointConstraint
    {
      AlignMapPointConstraint( bool locked = false, bool relative = false, double value = 0 )
        : locked( locked )
        , relative( relative )
        , value( value )
      {}

      //! Whether the constraint is active, i.e. should be considered
      bool locked;
      //! Whether the value is relative to previous value
      bool relative;
      //! Numeric value of the constraint (coordinate/distance in map units or angle in degrees)
      double value;
    };

    //! Structure defining all constraints for alignMapPoint() method
    struct AlignMapPointContext
    {
      //! Snapping utils that will be used to snap point to map. Must not be NULLPTR.
      QgsSnappingUtils *snappingUtils = nullptr;
      //! Map units/pixel ratio from map canvas. Needed for
      double mapUnitsPerPixel;

      //! Constraint for X coordinate
      QgsCadUtils::AlignMapPointConstraint xConstraint;
      //! Constraint for Y coordinate
      QgsCadUtils::AlignMapPointConstraint yConstraint;
      //! Constraint for distance
      QgsCadUtils::AlignMapPointConstraint distanceConstraint;
      //! Constraint for angle
      QgsCadUtils::AlignMapPointConstraint angleConstraint;
      //! Constraint for soft lock to a common angle
      QgsCadUtils::AlignMapPointConstraint commonAngleConstraint;

      /**
       * List of recent CAD points in map coordinates. These are used to turn relative constraints to absolute.
       * First point is the most recent point. Currently using only "previous" point (index 1) and "penultimate"
       * point (index 2) for alignment purposes.
       */
      QList<QgsPointXY> cadPointList;

      /**
       * Dumps the context's properties, for debugging.
       * \note Not available in Python bindings.
       */
      SIP_SKIP void dump() const;
    };

    //! Structure returned from alignMapPoint() method
    struct AlignMapPointOutput
    {
      //! Whether the combination of constraints is actually valid
      bool valid;

      //! map point aligned according to the constraints
      QgsPointXY finalMapPoint;

      //! Snapped segment - only valid if actually used for something
      QgsPointLocator::Match edgeMatch;

      //! Angle (in degrees) to which we have soft-locked ourselves (if not set it is -1)
      double softLockCommonAngle;
    };

    /**
     * Applies X/Y/angle/distance constraints from the given context to a map point.
     * Returns a structure containing aligned map point, whether the constraints are valid and
     * some extra information.
     */
    static QgsCadUtils::AlignMapPointOutput alignMapPoint( const QgsPointXY &originalMapPoint, const QgsCadUtils::AlignMapPointContext &ctx );

};

#endif // QGSCADUTILS_H
