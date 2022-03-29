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

#include <QQueue>

#include "qgis_core.h"
#include "qgspointlocator.h"


class QgsSnappingUtils;

/**
 * \ingroup core
 * \brief The QgsCadUtils class provides routines for CAD editing.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsCadUtils
{
  public:

    /**
     * \brief Structure with details of one constraint
     * \ingroup core
     * \since QGIS 3.0
     */
    class AlignMapPointConstraint
    {
      public:

        /**
         * Constructor for AlignMapPointConstraint.
         */
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

    /**
     * \brief Structure returned from alignMapPoint() method
     * \ingroup core
     * \since QGIS 3.0
     */
    class AlignMapPointOutput
    {
      public:
        //! Whether the combination of constraints is actually valid
        bool valid;

        //! map point aligned according to the constraints
        QgsPointXY finalMapPoint;

        /**
         * Snapped point - only valid if actually used for something
         * \since QGIS 3.14
         */
        QgsPointLocator::Match snapMatch;

        /**
         * Snapped segment - only valid if actually used for something
         * \deprecated will be removed in QGIS 4.0 - use snapMatch instead
         */
        QgsPointLocator::Match edgeMatch;

        //! Angle (in degrees) to which we have soft-locked ourselves (if not set it is -1)
        double softLockCommonAngle;

        Qgis::LineExtensionSide softLockLineExtension;
        double softLockX;
        double softLockY;
    };

    /**
     * \ingroup core
     * \brief Defines constraints for the QgsCadUtils::alignMapPoint() method.
     * \since QGIS 3.0
     */
    class AlignMapPointContext
    {
      public:
        //! Snapping utils that will be used to snap point to map. Must not be NULLPTR.
        QgsSnappingUtils *snappingUtils = nullptr;
        //! Map units/pixel ratio from map canvas.
        double mapUnitsPerPixel;

        //! Constraint for X coordinate
        QgsCadUtils::AlignMapPointConstraint xConstraint;
        //! Constraint for Y coordinate
        QgsCadUtils::AlignMapPointConstraint yConstraint;

        /**
         * Constraint for Z coordinate
         * \since QGIS 3.22
         */
        QgsCadUtils::AlignMapPointConstraint zConstraint;

        /**
         * Constraint for M coordinate
         * \since QGIS 3.22
         */
        QgsCadUtils::AlignMapPointConstraint mConstraint;
        //! Constraint for distance
        QgsCadUtils::AlignMapPointConstraint distanceConstraint;
        //! Constraint for angle
        QgsCadUtils::AlignMapPointConstraint angleConstraint;
        //! Constraint for soft lock to a common angle
        QgsCadUtils::AlignMapPointConstraint commonAngleConstraint;

        QgsCadUtils::AlignMapPointConstraint lineExtensionConstraint;
        QgsCadUtils::AlignMapPointConstraint xyVertexConstraint;

        /**
         * Dumps the context's properties, for debugging.
         * \note Not available in Python bindings.
         */
        SIP_SKIP void dump() const;

        /**
         * Returns the list of recent CAD points in map coordinates.
         *
         * These are used to turn relative constraints to absolute. The first
         * point is the most recent point.
         *
         * \see setCadPoints()
         * \since QGIS 3.22
         */
        QList< QgsPoint > cadPoints() const { return mCadPointList; } ;

        /**
         * Sets the list of recent CAD \a points (in map coordinates).
         *
         * \see cadPoints()
         * \since QGIS 3.22
         */
        void setCadPoints( const QList< QgsPoint > &points ) { mCadPointList = points; };

        /**
         * Sets the recent CAD point at the specified \a index to \a point (in map coordinates).
         *
         * \see cadPoint()
         * \since QGIS 3.22
         */
        void setCadPoint( int index, const QgsPoint &point ) { mCadPointList[index] = point; };

        /**
         * Returns the recent CAD point at the specified \a index (in map coordinates).
         *
         * \see setCadPoint()
         * \since QGIS 3.22
         */
        QgsPoint cadPoint( int index ) const { return mCadPointList[index]; };

        /**
         * Sets the queue of locked vertices.
         *
         * Point locator matches are stored instead of vertices to keep more context.
         *
         * \see lockedSnapVertices()
         * \since QGIS 3.26
         */
        void setLockedSnapVertices( const QQueue< QgsPointLocator::Match > &lockedSnapVertices ) { mLockedSnapVertices = lockedSnapVertices; } SIP_SKIP;

        /**
         * Returns the queue of point locator matches that contain the locked vertices.
         *
         * \see setLockedSnapVertices()
         * \since QGIS 3.26
         */
        QQueue< QgsPointLocator::Match > lockedSnapVertices() const { return mLockedSnapVertices; } SIP_SKIP;


#ifdef SIP_RUN
        SIP_PROPERTY( name = cadPointList, get = _cadPointList, set = _setCadPointList )
#endif
        ///@cond PRIVATE
        void _setCadPointList( const QList< QgsPointXY > &list ) { mCadPointList.clear(); for ( const auto &pointxy : list ) { mCadPointList.append( QgsPoint( pointxy ) );} }
        QList< QgsPointXY > _cadPointList() const { QList< QgsPointXY> list; for ( const auto &point : mCadPointList ) { list.append( QgsPointXY( point.x(), point.y() ) ); }; return list; }
        ///@endcond PRIVATE

      private:

        /**
         * List of recent CAD points in map coordinates. These are used to turn relative constraints to absolute.
         * First point is the most recent point. Currently using only "previous" point (index 1) and "penultimate"
         * point (index 2) for alignment purposes.
         */
        QList<QgsPoint> mCadPointList;
        QQueue< QgsPointLocator::Match > mLockedSnapVertices;

    };

    /**
     * Applies X/Y/angle/distance constraints from the given context to a map point.
     * Returns a structure containing aligned map point, whether the constraints are valid and
     * some extra information.
     */
    static QgsCadUtils::AlignMapPointOutput alignMapPoint( const QgsPointXY &originalMapPoint, const QgsCadUtils::AlignMapPointContext &ctx );

};

#endif // QGSCADUTILS_H
