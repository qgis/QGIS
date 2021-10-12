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
#include "qgslogger.h"

#include <QVector>

class QgsAbstractGeometry;

/**
 * \ingroup core
 * \class QgsGeometryEngine
 * \brief A geometry engine is a low-level representation of a QgsAbstractGeometry object, optimised for use with external
 * geometry libraries such as GEOS.
 *
 * QgsGeometryEngine objects provide a mechanism for optimized evaluation of geometric algorithms, including spatial relationships
 * between geometries and operations such as buffers or clipping.
 *
 * QgsGeometryEngine objects are not created directly, but are instead created by calling QgsGeometry::createGeometryEngine().
 *
 * Many methods available in the QgsGeometryEngine class can benefit from pre-preparing geometries. For instance, whenever
 * a large number of spatial relationships will be tested (such as calling intersects(), within(), etc) then the
 * geometry should first be prepared by calling prepareGeometry() before performing the tests.
 *
 * ### Example
 *
 * \code{.py}
 *   # polygon_geometry contains a complex polygon, with many vertices
 *   polygon_geometry = QgsGeometry.fromWkt('Polygon((...))')
 *
 *   # create a QgsGeometryEngine representation of the polygon
 *   polygon_geometry_engine = QgsGeometry.createGeometryEngine(polygon_geometry.constGet())
 *
 *   # since we'll be performing many intersects tests, we can speed up these tests considerably
 *   # by first "preparing" the geometry engine
 *   polygon_geometry_engine.prepareGeometry()
 *
 *   # now we are ready to quickly test intersection against many other objects
 *   for feature in my_layer.getFeatures():
 *       feature_geometry = feature.geometry()
 *       # test whether the feature's geometry intersects our original complex polygon
 *       if polygon_geometry_engine.intersects(feature_geometry.constGet()):
 *           print('feature intersects the polygon!')
 * \endcode
 *
 * QgsGeometryEngine operations are backed by the GEOS library (https://trac.osgeo.org/geos/).
 *
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

    /**
     * Buffers a geometry.
     */
    virtual QgsAbstractGeometry *buffer( double distance, int segments, Qgis::EndCapStyle endCapStyle, Qgis::JoinStyle joinStyle, double miterLimit, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;
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
     * Checks if \a geom is within \a maxdistance distance from this geometry
     *
     * \since QGIS 3.22
     */
    virtual bool distanceWithin( const QgsAbstractGeometry *geom, double maxdistance, QString *errorMsg = nullptr ) const = 0;

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
     * Returns TRUE if the geometry is valid.
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
     * \param skipIntersectionCheck set to TRUE to skip the potentially expensive initial intersection check. Only set this flag if an intersection
     * test has already been performed by the caller!
     * \returns 0 in case of success, 1 if geometry has not been split, error else
    */
    virtual QgsGeometryEngine::EngineOperationResult splitGeometry( const QgsLineString &splitLine,
        QVector<QgsGeometry > &newGeometries SIP_OUT,
        bool topological,
        QgsPointSequence &topologyTestPoints, QString *errorMsg = nullptr, bool skipIntersectionCheck = false ) const
    {
      Q_UNUSED( splitLine )
      Q_UNUSED( newGeometries )
      Q_UNUSED( topological )
      Q_UNUSED( topologyTestPoints )
      Q_UNUSED( errorMsg )
      Q_UNUSED( skipIntersectionCheck )
      return MethodNotImplemented;
    }

    /**
     * Offsets a curve.
     */
    virtual QgsAbstractGeometry *offsetCurve( double distance, int segments, Qgis::JoinStyle joinStyle, double miterLimit, QString *errorMsg = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Sets whether warnings and errors encountered during the geometry operations should be logged.
     *
     * By default these errors are logged to the console and in the QGIS UI. But for some operations errors are expected and logging
     * these just results in noise. In this case setting \a enabled to FALSE will avoid the automatic error reporting.
     *
     * \since QGIS 3.16
     */
    void setLogErrors( bool enabled ) { mLogErrors = enabled; }

  protected:
    const QgsAbstractGeometry *mGeometry = nullptr;
    bool mLogErrors = true;

    /**
     * Logs an error \a message encountered during an operation.
     *
     * \see setLogErrors()
     *
     * \since QGIS 3.16
     */
    void logError( const QString &engineName, const QString &message ) const
    {
      if ( mLogErrors )
      {
        QgsDebugMsg( QStringLiteral( "%1 notice: %2" ).arg( engineName, message ) );
        qWarning( "%s exception: %s", engineName.toLocal8Bit().constData(), message.toLocal8Bit().constData() );
      }
    }

    QgsGeometryEngine( const QgsAbstractGeometry *geometry )
      : mGeometry( geometry )
    {}
};

#endif // QGSGEOMETRYENGINE_H

