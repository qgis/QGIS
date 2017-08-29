/***************************************************************************
                        qgsgeometryengine.h
  -------------------------------------------------------------------
Date                 : 22 Sept 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYENGINE_H
#define QGSGEOMETRYENGINE_H

#include "qgis_core.h"
#include "qgslinestring.h"
#include "qgsgeometry.h"

#include <QList>

class QgsAbstractGeometry;

/** \ingroup core
 * \class QgsGeometryEngine
 * \brief Contains geometry relation and modification algorithms.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsGeometryEngine
{
  public:

    /**
     * Success or failure of a geometry operation.
     * This gives details about cause of failure.
     */
    enum EngineOperationResult
    {
      Success = 0, //!< Operation succeeded
      NothingHappened = 1000, //!< Nothing happened, without any error
      MethodNotImplemented, //!< Method not implemented in geometry engine
      EngineError, //!< Error occurred in the geometry engine
      NodedGeometryError, //!< Error occurred while creating a noded geometry
      InvalidBaseGeometry, //!< The geometry on which the operation occurs is not valid
      InvalidInput, //!< The input is not valid
      /* split */
      SplitCannotSplitPoint, //!< Points cannot be split
    };

    virtual ~QgsGeometryEngine() = default;

    virtual void geometryChanged() = 0;
    virtual void prepareGeometry() = 0;

    /**
     * Calculate the intersection of this and \a geom.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual QgsAbstractGeometry *intersection( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Calculate the difference of this and \a geom.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual QgsAbstractGeometry *difference( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Calculate the combination of this and \a geom.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual QgsAbstractGeometry *combine( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Calculate the combination of this and \a geometries.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual QgsAbstractGeometry *combine( const QList< QgsAbstractGeometry * > &geometries, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Calculate the symmetric difference of this and \a geom.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual QgsAbstractGeometry *symDifference( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;
    virtual QgsAbstractGeometry *buffer( double distance, int segments, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;
    virtual QgsAbstractGeometry *buffer( double distance, int segments, int endCapStyle, int joinStyle, double miterLimit, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;
    virtual QgsAbstractGeometry *simplify( double tolerance, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;
    virtual QgsAbstractGeometry *interpolate( double distance, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;
    virtual QgsAbstractGeometry *envelope( QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Calculates the centroid of this.
     * May return a `nullptr`.
     *
     * \since QGIS 3.0 the centroid is returned
     */
    virtual QgsPoint *centroid( QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Calculate a point that is guaranteed to be on the surface of this.
     * May return a `nullptr`.
     *
     * \since QGIS 3.0 the centroid is returned
     */
    virtual QgsPoint *pointOnSurface( QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Calculate the convex hull of this.
     */
    virtual QgsAbstractGeometry *convexHull( QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Calculates the distance between this and \a geom.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual double distance( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;

    /**
     * Checks if \a geom intersects this.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual bool intersects( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;

    /**
     * Checks if \a geom touches this.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual bool touches( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;

    /**
     * Checks if \a geom crosses this.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual bool crosses( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;

    /**
     * Checks if \a geom is within this.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual bool within( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;

    /**
     * Checks if \a geom overlaps this.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual bool overlaps( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;

    /**
     * Checks if \a geom contains this.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual bool contains( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;

    /**
     * Checks if \a geom is disjoint from this.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual bool disjoint( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;

    /** Returns the Dimensional Extended 9 Intersection Model (DE-9IM) representation of the
     * relationship between the geometries.
     * \param geom geometry to relate to
     * \param errorMsg destination storage for any error message
     * \returns DE-9IM string for relationship, or an empty string if an error occurred
     * \since QGIS 2.12
     */
    virtual QString relate( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;

    /** Tests whether two geometries are related by a specified Dimensional Extended 9 Intersection Model (DE-9IM)
     * pattern.
     * \param geom geometry to relate to
     * \param pattern DE-9IM pattern for match
     * \param errorMsg destination storage for any error message
     * \returns true if geometry relationship matches with pattern
     * \since QGIS 2.14
     */
    virtual bool relatePattern( const QgsAbstractGeometry *geom, const QString &pattern, QString *errorMsg = nullptr ) const = 0;

    virtual double area( QString *errorMsg = nullptr ) const = 0;
    virtual double length( QString *errorMsg = nullptr ) const = 0;
    virtual bool isValid( QString *errorMsg = nullptr ) const = 0;

    /**
     * Checks if this is equal to \a geom.
     * If both are Null geometries, `false` is returned.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual bool isEqual( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;
    virtual bool isEmpty( QString *errorMsg ) const = 0;

    /** Determines whether the geometry is simple (according to OGC definition).
     * \since QGIS 3.0
     */
    virtual bool isSimple( QString *errorMsg = nullptr ) const = 0;

    virtual QgsGeometryEngine::EngineOperationResult splitGeometry( const QgsLineString &splitLine,
        QList<QgsAbstractGeometry *> &newGeometries,
        bool topological,
        QgsPointSequence &topologyTestPoints, QString *errorMsg = nullptr ) const
    {
      Q_UNUSED( splitLine );
      Q_UNUSED( newGeometries );
      Q_UNUSED( topological );
      Q_UNUSED( topologyTestPoints );
      Q_UNUSED( errorMsg );
      return MethodNotImplemented;
    }

    virtual QgsAbstractGeometry *offsetCurve( double distance, int segments, int joinStyle, double miterLimit, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

  protected:
    const QgsAbstractGeometry *mGeometry = nullptr;

    QgsGeometryEngine( const QgsAbstractGeometry *geometry )
      : mGeometry( geometry )
    {}
};

#endif // QGSGEOMETRYENGINE_H
