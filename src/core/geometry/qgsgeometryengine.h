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

#include <QVector>

class QgsAbstractGeometry;

/**
 * \ingroup core
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

    /**
     * Should be called whenever the geometry associated with the engine
     * has been modified and the engine must be updated to suit.
     */
    virtual void geometryChanged() = 0;

    /**
     * Prepares the geometry, so that subsequent calls to spatial relation methods
     * are much faster.
     *
     * This should be called for any geometry which is used for multiple relation
     * tests against other geometries.
     *
     * \see geometryChanged()
     */
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
    virtual QgsAbstractGeometry *combine( const QVector<QgsAbstractGeometry *> &geomList, QString *errorMsg ) const = 0 SIP_FACTORY;

    /**
     * Calculate the combination of this and \a geometries.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual QgsAbstractGeometry *combine( const QVector< QgsGeometry > &geometries, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

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
     * May return a `NULLPTR`.
     *
     * \since QGIS 3.0 the centroid is returned
     */
    virtual QgsPoint *centroid( QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Calculate a point that is guaranteed to be on the surface of this.
     * May return a `NULLPTR`.
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

    /**
     * Returns the Dimensional Extended 9 Intersection Model (DE-9IM) representation of the
     * relationship between the geometries.
     * \param geom geometry to relate to
     * \param errorMsg destination storage for any error message
     * \returns DE-9IM string for relationship, or an empty string if an error occurred
     * \since QGIS 2.12
     */
    virtual QString relate( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;

    /**
     * Tests whether two geometries are related by a specified Dimensional Extended 9 Intersection Model (DE-9IM)
     * pattern.
     * \param geom geometry to relate to
     * \param pattern DE-9IM pattern for match
     * \param errorMsg destination storage for any error message
     * \returns TRUE if geometry relationship matches with pattern
     * \since QGIS 2.14
     */
    virtual bool relatePattern( const QgsAbstractGeometry *geom, const QString &pattern, QString *errorMsg = nullptr ) const = 0;

    virtual double area( QString *errorMsg = nullptr ) const = 0;
    virtual double length( QString *errorMsg = nullptr ) const = 0;

    /**
     * Returns true if the geometry is valid.
     *
     * If the geometry is invalid, \a errorMsg will be filled with the reported geometry error.
     *
     * The \a allowSelfTouchingHoles argument specifies whether self-touching holes are permitted.
     * OGC validity states that self-touching holes are NOT permitted, whilst other vendor
     * validity checks (e.g. ESRI) permit self-touching holes.
     *
     * If \a errorLoc is specified, it will be set to the geometry of the error location.
     */
    virtual bool isValid( QString *errorMsg = nullptr, bool allowSelfTouchingHoles = false, QgsGeometry *errorLoc = nullptr ) const = 0;

    /**
     * Checks if this is equal to \a geom.
     * If both are Null geometries, `FALSE` is returned.
     *
     * \since QGIS 3.0 \a geom is a pointer
     */
    virtual bool isEqual( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const = 0;
    virtual bool isEmpty( QString *errorMsg ) const = 0;

    /**
     * Determines whether the geometry is simple (according to OGC definition).
     * \since QGIS 3.0
     */
    virtual bool isSimple( QString *errorMsg = nullptr ) const = 0;

    /**
     * Splits this geometry according to a given line.
     * \param splitLine the line that splits the geometry
     * \param[out] newGeometries list of new geometries that have been created with the split
     * \param topological TRUE if topological editing is enabled
     * \param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
     * \param[out] errorMsg error messages emitted, if any
     * \returns 0 in case of success, 1 if geometry has not been split, error else
    */
    virtual QgsGeometryEngine::EngineOperationResult splitGeometry( const QgsLineString &splitLine,
        QVector<QgsGeometry > &newGeometries SIP_OUT,
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
