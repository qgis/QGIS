/***************************************************************************
                        qgssfcgalengine.h
  -------------------------------------------------------------------
    begin                : May 2025
    copyright            : (C) 2024 by Benoit De Mezzo
    email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSFCGALENGINE_H
#define QGSSFCGALENGINE_H

#define SIP_NO_FILE

#include <functional>
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

/// check if \a ptr is not null else add stacktrace entry and return the \a default_obj
#define CHECK_NOT_NULL( ptr, default_obj )                                                                               \
  if ( !( ptr ) )                                                                                                        \
  {                                                                                                                      \
    sfcgal::errorHandler()->addText( QString( "Null pointer for '%1'" ).arg( #ptr ), __FILE__, __FUNCTION__, __LINE__ ); \
    return default_obj;                                                                                                  \
  }

/// check if no error has been caught else add stacktrace entry and return the \a default_obj
#define CHECK_SUCCESS( errorMsg, default_obj )                                                    \
  if ( !sfcgal::errorHandler()->hasSucceedOrStack( errorMsg, __FILE__, __FUNCTION__, __LINE__ ) ) \
  {                                                                                               \
    return default_obj;                                                                           \
  }

/// check if no error has been caught else add stacktrace entry, log the stacktrace and return the \a default_obj
#define CHECK_SUCCESS_LOG( errorMsg, default_obj )                                                \
  if ( !sfcgal::errorHandler()->hasSucceedOrStack( errorMsg, __FILE__, __FUNCTION__, __LINE__ ) ) \
  {                                                                                               \
    QgsDebugError( sfcgal::errorHandler()->getFullText() );                                       \
    return default_obj;                                                                           \
  }

/**
 * Contains SFCGAL related utilities and functions.
 * \note not available in Python bindings.
 * \since QGIS 3.44
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
   * Messages are hold in a stacktrace way in order to improve context understanding.
   */
  class CORE_EXPORT ErrorHandler
  {
    public:
      //! Default constructor.
      ErrorHandler();

      /**
      * Returns true if no failure has been caught or returns false and add new stacktrace entry.
      *
      * If a failure has already been caught and \a errorMsg is not null then:
      *
      * - a stacktrace entry is added with caller location
      * - \a errorMsg will be updated with failure messages
      */
      bool hasSucceedOrStack( QString *errorMsg = nullptr, const char *fromFile = nullptr, const char *fromFunc = nullptr, int fromLine = 0 );

      /**
       * Clears failure messages and also clear \a errorMsg if not null.
       */
      void clearText( QString *errorMsg = nullptr );

      //! Returns true if failure has been caught.
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
 * \brief Does vector analysis using the SFCGAL library and handles import, export, exception handling*
 * \note not available in Python bindings
 * \since QGIS 3.44
 */
class CORE_EXPORT QgsSfcgalEngine
{
  public:

    /**
     * Creates a SFGAL geometry (inherit QgsAbstractGeometry) from an internal SFCGAL geometry (from SFCGAL library).
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static std::unique_ptr< QgsAbstractGeometry > toAbstractGeometry( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Creates a SFGAL geometry (inherit QgsAbstractGeometry) from an internal SFCGAL geometry (from SFCGAL library).
     *
     * Same as `toAbstractGeometry` but returned object is casted to QgsSfcgalGeometry.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static std::unique_ptr< QgsSfcgalGeometry > toSfcgalGeometry( sfcgal::shared_geom &geom, QString *errorMsg = nullptr );

    /**
     * Creates internal SFCGAL geometry (from SFCGAL library) from a QGIS geometry (inherit QgsAbstractGeometry).
     *
     * \param geometry geometry to convert to SFCGAL representation
     * \param errorMsg pointer to QString to receive the error message if any
     */
    static sfcgal::shared_geom fromAbstractGeometry( const QgsAbstractGeometry *geometry, QString *errorMsg = nullptr );

    /**
     * Clones \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    static sfcgal::shared_geom cloneGeometry( const sfcgal::geometry *geom, QString *errorMsg = nullptr );

    /**
     * Returns the type of a given geometry as a string
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
    static sfcgal::shared_geom translate( const sfcgal::geometry *geom, const QgsPoint &translation, QString *errorMsg = nullptr );

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
