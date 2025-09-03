/***************************************************************************
                        qgssfcgalengine.h
  -------------------------------------------------------------------
    begin                : May 2025
    copyright            : (C) 2025 by Oslandia
    email                : benoit dot de dot mezzo at oslandia dot com
    email                : jean dot felder at oslandia dot com
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef WITH_SFCGAL
#ifndef QGSSFCGALENGINE_H
#define QGSSFCGALENGINE_H

#define SIP_NO_FILE

#include <SFCGAL/capi/sfcgal_c.h>

#include "qgspoint.h"
#include "qgsvector3d.h"
#include "qgis_core.h"
#include "qgsgeometry.h"

class QgsLineString;
class QgsPolygon;
class QgsGeometry;
class QgsGeometryCollection;
class QgsSfcgalGeometry;

/// check if \a ptr is not null else add stacktrace entry and return the \a defaultObj
#define CHECK_NOT_NULL( ptr, defaultObj )                                                                                \
  if ( !( ptr ) )                                                                                                        \
  {                                                                                                                      \
    sfcgal::errorHandler()->addText( QString( "Null pointer for '%1'" ).arg( #ptr ), __FILE__, __FUNCTION__, __LINE__ ); \
    return ( defaultObj );                                                                                               \
  }

/// check if no error has been caught else add stacktrace entry and return the \a defaultObj
#define CHECK_SUCCESS( errorMsg, defaultObj )                                                         \
  if ( !sfcgal::errorHandler()->hasSucceedOrStack( ( errorMsg ), __FILE__, __FUNCTION__, __LINE__ ) ) \
  {                                                                                                   \
    return ( defaultObj );                                                                            \
  }

/// check if no error has been caught else add stacktrace entry, log the stacktrace and return the \a defaultObj
#define CHECK_SUCCESS_LOG( errorMsg, defaultObj )                                                     \
  if ( !sfcgal::errorHandler()->hasSucceedOrStack( ( errorMsg ), __FILE__, __FUNCTION__, __LINE__ ) ) \
  {                                                                                                   \
    QgsDebugError( sfcgal::errorHandler()->getFullText() );                                           \
    return ( defaultObj );                                                                            \
  }

/**
 * Contains SFCGAL related utilities and functions.
 * \note not available in Python bindings.
 * \since QGIS 4.0
 */
namespace sfcgal
{
  //! Shortcut to SFCGAL geometry
  using geometry = sfcgal_geometry_t;

  /**
   * Destroys the SFCGAL geometry \a geom, using the static QGIS
   * SFCGAL context.
   */
  struct Deleter
  {

    /**
     * Destroys the SFCGAL geometry \a geom, using the static QGIS
     * SFCGAL context.
     */
    void CORE_EXPORT operator()( sfcgal::geometry *geom ) const;
  };

  /**
   * Scoped SFCGAL pointer.
   */
  using unique_geom = std::unique_ptr< geometry, Deleter >;
  using shared_geom = std::shared_ptr< geometry >; // NO DELETER ==> added with function make_shared_ptr!!!!!

  //! Creates a shared geometry pointer with sfcgal::Deleter
  shared_geom make_shared_geom( geometry *geom );

  //! Callback uses by SFCGAL lib to push error.
  int errorCallback( const char *, ... );

  //! Callback uses by SFCGAL lib to push warning.
  int warningCallback( const char *, ... );

  /**
   * Helper class to handle SFCGAL engine errors.
   *
   * Messages are held in a stacktrace in order to improve context understanding.
   * \ingroup core
   * \since QGIS 4.0
   */
  class CORE_EXPORT ErrorHandler
  {
    public:
      //! Default constructor.
      ErrorHandler();

      /**
      * Returns true if no failure has been caught or returns false and adds a new stacktrace entry.
      *
      * If a failure has already been caught and \a errorMsg is not null then:
      *
      * - a stacktrace entry is added with caller location
      * - \a errorMsg will be updated with failure messages
      */
      bool hasSucceedOrStack( QString *errorMsg = nullptr, const char *fromFile = nullptr, const char *fromFunc = nullptr, int fromLine = 0 );

      /**
       * Clears failure messages and also clear \a errorMsg content if not null.
       */
      void clearText( QString *errorMsg = nullptr );

      //! Returns true if no failure has been caught.
      bool isTextEmpty() const;

      //! Returns the first caught failure message.
      QString getMainText() const;

      //! Returns all failure messages as a stack trace.
      QString getFullText() const;

      //! Adds \a msg to the failure message list.
      void addText( const QString &msg, const char *fromFile = nullptr, const char *fromFunc = nullptr, int fromLine = 0 );

    private:
      QStringList errorMessages;
  };

  //! Returns current error handler.
  CORE_EXPORT ErrorHandler *errorHandler();

  //! Shortcut for SFCGAL function definition.
  using func_geomgeom_to_geom = sfcgal_geometry_t *( * )( const sfcgal_geometry_t *, const sfcgal_geometry_t * );
  //! Shortcut for SFCGAL function definition.
  using func_geom_to_geom = sfcgal_geometry_t *( * )( const sfcgal_geometry_t * );
} // namespace sfcgal

/**
 * \ingroup core
 * \brief Does vector analysis using the SFCGAL library and handles import, export, exception handling
 * \note not available in Python bindings
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsSfcgalEngine
{
  public:

    /**
     * Creates a QgsAbstractGeometry from an internal SFCGAL geometry (from SFCGAL library).
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static std::unique_ptr< QgsAbstractGeometry > toAbstractGeometry( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Creates a SFGAL geometry from a shared SFCGAL geometry (from SFCGAL library).
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static std::unique_ptr< QgsSfcgalGeometry > toSfcgalGeometry( sfcgal::shared_geom &geom, QString *errorMsg = nullptr );

    /**
     * Creates internal SFCGAL geometry (from SFCGAL library) from a QGIS QgsAbstractGeometry.
     *
     * \param geom geometry to convert to SFCGAL representation
     * \param errorMsg pointer to QString to receive the error message if any
     */
    static sfcgal::shared_geom fromAbstractGeometry( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr );

    /**
     * Clones \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom cloneGeometry( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Returns the type of a given geometry as a OGC string in CamelCase
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static QString geometryType( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Creates geometry from WKB data.
     *
     * \param wkbPtr reference onto WKB data.
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom fromWkb( const QgsConstWkbPtr &wkbPtr, QString *errorMsg = nullptr );

    /**
     * Creates geometry from WKT string.
     *
     * \param wkt reference onto WKT string.
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom fromWkt( const QString &wkt, QString *errorMsg = nullptr );

    /**
     * Computes WKB data from \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static QgsConstWkbPtr toWkb( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Computes WKT string from \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param numDecimals Floating point precision for WKT coordinates. Setting to -1 yields rational number WKT (not decimal) f.e. "Point(1/3, 1/6, 1/4)".
     */
    static QString toWkt( const sfcgal::geometry *geom, int numDecimals = -1, QString *errorMsg = nullptr );

    /**
     * Returns the QGIS WTB type from \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg pointer to QString to receive the error message if any
     */
    static Qgis::WkbType wkbType( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Returns the \a geom dimension.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg pointer to QString to receive the error message if any
     */
    static int dimension( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Returns the \a geom part count.
     *
     * - POINT, TRIANGLE, LINESTRING: vertex number
     * - POLYGON, SOLID, POLYHEDRALSURFACE, TRIANGULATEDSURFACE: ring or patch or shell number
     * - MULTIPOINT, MULTILINESTRING, MULTIPOLYGON, MULTISOLID, GEOMETRYCOLLECTION: number of geom in collection
     *
     * \param geom geometry to perform the operation
     * \param errorMsg pointer to QString to receive the error message if any
     */
    static int partCount( const sfcgal::geometry *geom, QString *errorMsg );

    /**
     * Adds a z-dimension to the geometry, initialized to a preset value (existing Z values remains unchanged).
     *
     * \return true if success
     * \param geom geometry to perform the operation
     * \param zValue z value to use
     * \param errorMsg pointer to QString to receive the error message if any
     */
    static bool addZValue( sfcgal::geometry *geom, double zValue = 0, QString *errorMsg = nullptr );

    /**
     * Adds a m-dimension to the geometry, initialized to a preset value (existing M values remains unchanged).
     *
     * \return true if success
     * \param geom geometry to perform the operation
     * \param mValue m value to use
     * \param errorMsg pointer to QString to receive the error message if any
     */
    static bool addMValue( sfcgal::geometry *geom, double mValue = 0, QString *errorMsg = nullptr );

    /**
     * Drops the z coordinate of the geometry
     *
     * \return true if success
     * \param geom geometry to perform the operation
     * \param errorMsg pointer to QString to receive the error message if any
     */
    static bool dropZValue( sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Drops the m coordinate of the geometry
     *
     * \return true if success
     * \param geom geometry to perform the operation
     * \param errorMsg pointer to QString to receive the error message if any
     */
    static bool dropMValue( sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Swaps the x and y coordinates of the geometry
     *
     * \param geom geometry to perform the operation
     * \param errorMsg pointer to QString to receive the error message if any
     */
    static void swapXy( sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Checks if \a geomA and \a geomB are equal.
     *
     * \param geomA first geometry to perform the operation
     * \param geomB second geometry to perform the operation
     * \param tolerance max distance allowed between each point
     * \param errorMsg Error message returned by SFGCAL
     */
    static bool isEqual( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, double tolerance = 0.0, QString *errorMsg = nullptr );

    /**
     * Checks if \a geom is empty.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static bool isEmpty( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Checks if \a geom is valid.
     *
     * If the geometry is invalid, \a errorMsg will be filled with the reported geometry error.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param allowSelfTouchingHoles specifies whether self-touching holes are permitted.
     *        OGC validity states that self-touching holes are NOT permitted, whilst other
     *        vendor validity checks (e.g. ESRI) permit self-touching holes.
     * \param errorLoc if specified, it will be set to the geometry of the error location.
     */
    static bool isValid( const sfcgal::geometry *geom, QString *errorMsg = nullptr, bool allowSelfTouchingHoles = false, QgsGeometry *errorLoc = nullptr );

    /**
     * Checks if \a geom is simple.
     *
     * As OGC specifications. Here are PostGIS (https://postgis.net/docs/using_postgis_dbmanagement.html#OGC_Validity) extract:
     * A POINT is inherently simple as a 0-dimensional geometry object.
     * MULTIPOINTs are simple if no two coordinates (POINTs) are equal (have identical coordinate values).
     * A LINESTRING is simple if it does not pass through the same point twice, except for the endpoints. If the endpoints of a simple LineString are identical it is called closed and referred to as a Linear Ring.
     * A MULTILINESTRING is simple only if all of its elements are simple and the only intersection between any two elements occurs at points that are on the boundaries of both elements.
     * POLYGONs are formed from linear rings, so valid polygonal geometry is always simple.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static bool isSimple( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Calculate the boundary of \a geom
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom boundary( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Calculate the centroid of \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static QgsPoint centroid( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Translate the \a geom by vector \a translation.
     *
     * \param geom geometry to perform the operation
     * \param translation translation vector (2D or 3D)
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom translate( const sfcgal::geometry *geom, const QgsVector3D &translation, QString *errorMsg = nullptr );

    /**
     * Scale the \a geom by vector \a scaleFactor.
     *
     * \param geom geometry to perform the operation
     * \param scaleFactor scale factor vector (2D or 3D)
     * \param center optional parameter. If specified, scaling will be performed relative to this center
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom scale( const sfcgal::geometry *geom, const QgsVector3D &scaleFactor, const QgsPoint &center = QgsPoint(), QString *errorMsg = nullptr );

    /**
     * 2D Rotation of geometry \a geom around point \a center by angle \a angle
     *
     * \param geom geometry to perform the operation
     * \param angle rotation angle in radians
     * \param center rotation center
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom rotate2D( const sfcgal::geometry *geom, double angle, const QgsPoint &center, QString *errorMsg = nullptr );

    /**
     * 3D Rotation of geometry \a geom around axis \a axisVector by angle \a angle
     *
     * \param geom geometry to perform the operation
     * \param angle rotation angle in radians
     * \param axisVector rotation axis
     * \param center optional parameter. If specified, rotation will be applied around axis and center point
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom rotate3D( const sfcgal::geometry *geom, double angle, const QgsVector3D &axisVector, const QgsPoint &center = QgsPoint(), QString *errorMsg = nullptr );

    /**
     * Computes shortest distance between \a geomA and \a geomB.
     *
     * \param geomA first geometry to perform the operation
     * \param geomB second geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static double distance( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg = nullptr );

    /**
     * Checks if \a geomA is within \a maxdistance distance from \a geomB
     *
     * \param geomA first geometry to perform the operation
     * \param geomB second geometry to perform the operation
     * \param maxdistance
     * \param errorMsg Error message returned by SFGCAL
     */
    static bool distanceWithin( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, double maxdistance, QString *errorMsg );

    /**
     * Computes the area of \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static double area( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Computes the max length of \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static double length( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Checks if \a geomA and \a geomB intersect each other
     * A 3D intersection test is performed if at least one geometry is 3D; otherwise, a 2D intersection test is performed.
     *
     * \param geomA first geometry to perform the operation
     * \param geomB second geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static bool intersects( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg = nullptr );

    /**
     * Calculates the intersection of \a geomA and \a geomB
     *
     * \param geomA first geometry to perform the operation
     * \param geomB second geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom intersection( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg = nullptr );

    /**
     * Calculates the difference of \a geomA minus \a geomB
     *
     * \param geomA first geometry to perform the operation
     * \param geomB second geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom difference( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg = nullptr );

    /**
     * Calculate the combination of all geometry in \a geomList.
     *
     * \param geomList list of geometries to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom combine( const QVector< sfcgal::shared_geom > &geomList, QString *errorMsg = nullptr );

    /**
     * Calculate a triangulation of \a geom using constraint 2D Delaunay Triangulation (keep Z if defined).
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom triangulate( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Cover test on 2D or 3D geometries
     * Checks if \a geomA covers \a geomB.
     * A 3D covers test is conducted when at least one geometry is 3D; otherwise, a 2D covers test is carried out.
     *
     * \param geomA first geometry to perform the operation
     * \param geomB second geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static bool covers( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg = nullptr );

    /**
     * Calculate the convex hull of \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom convexHull( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Calculate the envelope (bounding box) of \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom envelope( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Calculate a buffer for the \a geom where all points are at \a distance from the original geometry.
     * A negative distance shrinks the geometry rather than expanding it.
     *
     * \param geom geometry to perform the operation
     * \param distance distance to move each point of the geometry
     * \param segments the number of segments to use for approximating curved
     * \param joinStyle the type of buffer to compute. Only round is supported.
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom offsetCurve( const sfcgal::geometry *geom, double distance, int segments, Qgis::JoinStyle joinStyle, QString *errorMsg = nullptr );

    /**
     * Calculate a 3D buffer for the \a geom where all points are at \a distance from the original geometry.
     * A negative distance shrinks the geometry rather than expanding it.
     * It is limited to Point and LineString.
     *
     * \param geom geometry to perform the operation
     * \param radius the buffer radius
     * \param segments the number of segments to use for approximating curved
     * \param joinStyle3D the type of buffer to compute
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom buffer3D( const sfcgal::geometry *geom, double radius, int segments, Qgis::JoinStyle3D joinStyle3D, QString *errorMsg = nullptr );

    /**
     * Calculate a 2D buffer for the \a geom where all points are at \a distance from the original geometry.
     * A negative distance shrinks the geometry rather than expanding it.
     *
     * \param geom geometry to perform the operation
     * \param radius the buffer radius
     * \param segments the number of segments to use for approximating curved
     * \param joinStyle the type of buffer to compute. Only round is supported.
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom buffer2D( const sfcgal::geometry *geom, double radius, int segments, Qgis::JoinStyle joinStyle, QString *errorMsg = nullptr );

    /**
     * Extrude the \a geom by vector \a extrusion.
     *
     * \param geom geometry to perform the operation
     * \param extrusion translation vector (2D or 3D)
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom extrude( const sfcgal::geometry *geom, const QgsVector3D &extrusion, QString *errorMsg = nullptr );

    /**
     * Calculate the simplified version of \a geom.
     *
     * \param geom geometry to perform the operation
     * \param tolerance The distance (in geometry unit) threshold
     * \param preserveTopology Whether to preserve topology during simplification
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom simplify( const sfcgal::geometry *geom, double tolerance, bool preserveTopology, QString *errorMsg = nullptr );
};

/// @cond PRIVATE


class SFCGALException : public std::runtime_error
{
  public:
    explicit SFCGALException( const QString &message )
      : std::runtime_error( message.toUtf8().constData() )
    {
    }
};

/// @endcond

#endif // QGSSFCGALENGINE_H
#endif
