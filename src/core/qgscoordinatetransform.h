/***************************************************************************
               qgscoordinatetransform.h  - Coordinate Transforms
               ------------------------
    begin                : Dec 2004
    copyright            : (C) 2004 Tim Sutton
    email                : tim at linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOORDINATETRANSFORM_H
#define QGSCOORDINATETRANSFORM_H

#include <QExplicitlySharedDataPointer>

#include "qgsconfig.h"
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"

class QgsCoordinateTransformPrivate;
class QgsPointXY;
class QgsRectangle;
class QPolygonF;
class QgsProject;

/**
 * \ingroup core
* Class for doing transforms between two map coordinate systems.
*
* This class can convert map coordinates to a different coordinate reference system.
* It is normally associated with a map layer and is used to transform between the
* layer's coordinate system and the coordinate system of the map canvas, although
* it can be used in a more general sense to transform coordinates.
*
* When used to transform between a layer and the map canvas, all references to source
* and destination coordinate systems refer to layer and map canvas respectively. All
* operations are from the perspective of the layer. For example, a forward transformation
* transforms coordinates from the layer's coordinate system to the map canvas.
* \note Since QGIS 3.0 QgsCoordinateReferenceSystem objects are implicitly shared.
*
* \see QgsDatumTransform
* \see QgsCoordinateTransformContext
*/
class CORE_EXPORT QgsCoordinateTransform
{

  public:

    //! Enum used to indicate the direction (forward or inverse) of the transform
    enum TransformDirection
    {
      ForwardTransform,     //!< Transform from source to destination CRS.
      ReverseTransform      //!< Transform from destination to source CRS.
    };

    //! Default constructor, creates an invalid QgsCoordinateTransform.
    QgsCoordinateTransform();

    /**
     * Constructs a QgsCoordinateTransform to transform from the \a source
     * to \a destination coordinate reference system.
     *
     * The \a context argument specifies the context under which the transform
     * will be applied, and is used for calculating necessary datum transforms
     * to utilize.
     *
     * Python scripts should generally use the constructor variant which accepts
     * a QgsProject instance instead of this constructor.
     *
     * \warning Do NOT use an empty/default constructed QgsCoordinateTransformContext()
     * object when creating QgsCoordinateTransform objects. This prevents correct
     * datum transform handling and may result in inaccurate transformations. Always
     * ensure that the QgsCoordinateTransformContext object is correctly retrieved
     * based on the current code context, or use the constructor variant which
     * accepts a QgsProject argument instead.
     *
     * \since QGIS 3.0
     */
    explicit QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source,
                                     const QgsCoordinateReferenceSystem &destination,
                                     const QgsCoordinateTransformContext &context );

    /**
     * Constructs a QgsCoordinateTransform to transform from the \a source
     * to \a destination coordinate reference system, when used with the
     * given \a project.
     *
     * No reference to \a project is stored or utilized outside of the constructor,
     * and it is used to retrieve the project's transform context only.
     *
     * Python scripts should utilize the QgsProject.instance() project
     * instance when creating QgsCoordinateTransform. This will ensure
     * that any datum transforms defined in the project will be
     * correctly respected during coordinate transforms. E.g.
     *
     * \code{.py}
     *   transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem("EPSG:3111"),
     *                                      QgsCoordinateReferenceSystem("EPSG:4326"), QgsProject.instance())
     * \endcode
     *
     * \since QGIS 3.0
     */
    explicit QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source,
                                     const QgsCoordinateReferenceSystem &destination,
                                     const QgsProject *project );

    /**
     * Constructs a QgsCoordinateTransform to transform from the \a source
     * to \a destination coordinate reference system, with the specified
     * datum transforms (see QgsDatumTransform).
     *
     * \since QGIS 3.0
     */
    explicit QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source,
                                     const QgsCoordinateReferenceSystem &destination,
                                     int sourceDatumTransformId,
                                     int destinationDatumTransformId );

    /**
     * Copy constructor
     */
    QgsCoordinateTransform( const QgsCoordinateTransform &o );

    /**
     * Assignment operator
     */
    QgsCoordinateTransform &operator=( const QgsCoordinateTransform &o );

    ~QgsCoordinateTransform();

    /**
     * Returns TRUE if the coordinate transform is valid, ie both the source and destination
     * CRS have been set and are valid.
     * \since QGIS 3.0
     */
    bool isValid() const;

    /**
     * Sets the source coordinate reference system.
     * \param crs CRS to transform coordinates from
     * \see sourceCrs()
     * \see setDestinationCrs()
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the destination coordinate reference system.
     * \param crs CRS to transform coordinates to
     * \see destinationCrs()
     * \see setSourceCrs()
     */
    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the \a context in which the coordinate transform should be
     * calculated.
     * \see context()
     * \since QGIS 3.0
     */
    void setContext( const QgsCoordinateTransformContext &context );

    /**
     * Returns the context in which the coordinate transform will be
     * calculated.
     * \see setContext()
     * \since QGIS 3.4
     */
    QgsCoordinateTransformContext context() const;

    /**
     * Returns the source coordinate reference system, which the transform will
     * transform coordinates from.
     * \see setSourceCrs()
     * \see destinationCrs()
     */
    QgsCoordinateReferenceSystem sourceCrs() const;

    /**
     * Returns the destination coordinate reference system, which the transform will
     * transform coordinates to.
     * \see setDestinationCrs()
     * \see sourceCrs()
     */
    QgsCoordinateReferenceSystem destinationCrs() const;

    /**
     * Transform the point from the source CRS to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param point point to transform
     * \param direction transform direction (defaults to ForwardTransform)
     * \returns transformed point
     */
    QgsPointXY transform( const QgsPointXY &point, TransformDirection direction = ForwardTransform ) const SIP_THROW( QgsCsException );

    /**
     * Transform the point specified by x,y from the source CRS to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param x x coordinate of point to transform
     * \param y y coordinate of point to transform
     * \param direction transform direction (defaults to ForwardTransform)
     * \returns transformed point
     */
    QgsPointXY transform( double x, double y, TransformDirection direction = ForwardTransform ) const;

    /**
     * Transforms a rectangle from the source CRS to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * This method assumes that the rectangle is a bounding box, and creates a bounding box
     * in the projected CRS, such that all points from the source rectangle are within
     * the returned rectangle.
     * \param rectangle rectangle to transform
     * \param direction transform direction (defaults to ForwardTransform)
     * \param handle180Crossover set to TRUE if destination CRS is geographic and handling of extents
     * crossing the 180 degree longitude line is required
     * \returns rectangle in destination CRS
     */
    QgsRectangle transformBoundingBox( const QgsRectangle &rectangle, TransformDirection direction = ForwardTransform, bool handle180Crossover = false ) const SIP_THROW( QgsCsException );

    /**
     * Transforms an array of x, y and z double coordinates in place, from the source CRS to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param x array of x coordinates of points to transform
     * \param y array of y coordinates of points to transform
     * \param z array of z coordinates of points to transform. The z coordinates of the points
     * must represent height relative to the vertical datum of the source CRS (generally ellipsoidal
     * heights) and must be expressed in its vertical units (generally meters)
     * \param direction transform direction (defaults to ForwardTransform)
     */
    void transformInPlace( double &x, double &y, double &z, TransformDirection direction = ForwardTransform ) const SIP_THROW( QgsCsException );

    /**
     * Transforms an array of x, y and z float coordinates in place, from the source CRS to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param x array of x coordinates of points to transform
     * \param y array of y coordinates of points to transform
     * \param z array of z coordinates of points to transform. The z coordinates of the points
     * must represent height relative to the vertical datum of the source CRS (generally ellipsoidal
     * heights) and must be expressed in its vertical units (generally meters)
     * \param direction transform direction (defaults to ForwardTransform)
     * \note not available in Python bindings
     */
    void transformInPlace( float &x, float &y, double &z, TransformDirection direction = ForwardTransform ) const SIP_SKIP;

    /**
     * Transforms an array of x, y and z float coordinates in place, from the source CRS to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param x array of x coordinates of points to transform
     * \param y array of y coordinates of points to transform
     * \param z array of z coordinates of points to transform. The z coordinates of the points
     * must represent height relative to the vertical datum of the source CRS (generally ellipsoidal
     * heights) and must be expressed in its vertical units (generally meters)
     * \param direction transform direction (defaults to ForwardTransform)
     * \note not available in Python bindings
     */
    void transformInPlace( float &x, float &y, float &z, TransformDirection direction = ForwardTransform ) const SIP_SKIP;

    /**
     * Transforms a vector of x, y and z float coordinates in place, from the source CRS to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param x vector of x coordinates of points to transform
     * \param y vector of y coordinates of points to transform
     * \param z vector of z coordinates of points to transform. The z coordinates of the points
     * must represent height relative to the vertical datum of the source CRS (generally ellipsoidal
     * heights) and must be expressed in its vertical units (generally meters)
     * \param direction transform direction (defaults to ForwardTransform)
     * \note not available in Python bindings
     */
    void transformInPlace( QVector<float> &x, QVector<float> &y, QVector<float> &z,
                           TransformDirection direction = ForwardTransform ) const SIP_SKIP;

    /**
     * Transforms a vector of x, y and z double coordinates in place, from the source CRS to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param x vector of x coordinates of points to transform
     * \param y vector of y coordinates of points to transform
     * \param z vector of z coordinates of points to transform. The z coordinates of the points
     * must represent height relative to the vertical datum of the source CRS (generally ellipsoidal
     * heights) and must be expressed in its vertical units (generally meters)
     * \param direction transform direction (defaults to ForwardTransform)
     * \note not available in Python bindings
     */
    void transformInPlace( QVector<double> &x, QVector<double> &y, QVector<double> &z,
                           TransformDirection direction = ForwardTransform ) const SIP_SKIP;

    /**
     * Transforms a polygon to the destination coordinate system.
     * \param polygon polygon to transform (occurs in place)
     * \param direction transform direction (defaults to forward transformation)
     */
    void transformPolygon( QPolygonF &polygon, TransformDirection direction = ForwardTransform ) const SIP_THROW( QgsCsException );

    /**
     * Transforms a rectangle to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param rectangle rectangle to transform
     * \param direction transform direction (defaults to ForwardTransform)
     * \returns transformed rectangle
     */
    QgsRectangle transform( const QgsRectangle &rectangle, TransformDirection direction = ForwardTransform ) const SIP_THROW( QgsCsException );

    /**
     * Transform an array of coordinates to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param numPoint number of coordinates in arrays
     * \param x array of x coordinates to transform
     * \param y array of y coordinates to transform
     * \param z array of z coordinates to transform
     * \param direction transform direction (defaults to ForwardTransform)
     */
    void transformCoords( int numPoint, double *x, double *y, double *z, TransformDirection direction = ForwardTransform ) const SIP_THROW( QgsCsException );

    /**
     * Returns TRUE if the transform short circuits because the source and destination are equivalent.
     */
    bool isShortCircuited() const;

    /**
     * Returns the ID of the datum transform to use when projecting from the source
     * CRS.
     *
     * This is usually calculated automatically from the transform's QgsCoordinateTransformContext,
     * but can be manually overwritten by a call to setSourceDatumTransformId().
     *
     * \see QgsDatumTransform
     * \see setSourceDatumTransformId()
     * \see destinationDatumTransformId()
     */
    int sourceDatumTransformId() const;

    /**
     * Sets the \a datumId ID of the datum transform to use when projecting from the source
     * CRS.
     *
     * This is usually calculated automatically from the transform's QgsCoordinateTransformContext.
     * Calling this method will overwrite any automatically calculated datum transform.
     *
     * \see QgsDatumTransform
     * \see sourceDatumTransformId()
     * \see setDestinationDatumTransformId()
     */
    void setSourceDatumTransformId( int datumId );

    /**
     * Returns the ID of the datum transform to use when projecting to the destination
     * CRS.
     *
     * This is usually calculated automatically from the transform's QgsCoordinateTransformContext,
     * but can be manually overwritten by a call to setDestinationDatumTransformId().
     *
     * \see QgsDatumTransform
     * \see setDestinationDatumTransformId()
     * \see sourceDatumTransformId()
     */
    int destinationDatumTransformId() const;

    /**
     * Sets the \a datumId ID of the datum transform to use when projecting to the destination
     * CRS.
     *
     * This is usually calculated automatically from the transform's QgsCoordinateTransformContext.
     * Calling this method will overwrite any automatically calculated datum transform.
     *
     * \see QgsDatumTransform
     * \see destinationDatumTransformId()
     * \see setSourceDatumTransformId()
     */
    void setDestinationDatumTransformId( int datumId );

    /**
     * Clears the internal cache used to initialize QgsCoordinateTransform objects.
     * This should be called whenever the srs database has
     * been modified in order to ensure that outdated CRS transforms are not created.
     * \since QGIS 3.0
     */
    static void invalidateCache();

    /**
     * Computes an *estimated* conversion factor between source and destination units:
     *
     *   sourceUnits * scaleFactor = destinationUnits
     *
     * \param referenceExtent A reference extent based on which to perform the computation
     *
     * \since QGIS 3.4
     */
    double scaleFactor( const QgsRectangle &referenceExtent ) const;

  private:

    mutable QExplicitlySharedDataPointer<QgsCoordinateTransformPrivate> d;

    //! Transform context
    QgsCoordinateTransformContext mContext;

#ifdef QGISDEBUG
    bool mHasContext = false;
#endif

    bool setFromCache( const QgsCoordinateReferenceSystem &src,
                       const QgsCoordinateReferenceSystem &dest,
                       int srcDatumTransform,
                       int destDatumTransform );
    void addToCache();

    // cache
    static QReadWriteLock sCacheLock;
    static QMultiHash< QPair< QString, QString >, QgsCoordinateTransform > sTransforms; //same auth_id pairs might have different datum transformations

};

//! Output stream operator
#ifndef SIP_RUN
inline std::ostream &operator << ( std::ostream &os, const QgsCoordinateTransform &r )
{
  QString mySummary( QStringLiteral( "\n%%%%%%%%%%%%%%%%%%%%%%%%\nCoordinate Transform def begins:" ) );
  mySummary += QLatin1String( "\n\tInitialized? : " );
  //prevent warnings
  if ( r.isValid() )
  {
    //do nothing this is a dummy
  }

#if 0
  if ( r.isValid() )
  {
    mySummary += "Yes";
  }
  else
  {
    mySummary += "No";
  }
  mySummary += "\n\tShort Circuit?  : ";
  if ( r.isShortCircuited() )
  {
    mySummary += "Yes";
  }
  else
  {
    mySummary += "No";
  }

  mySummary += "\n\tSource Spatial Ref Sys  : ";
  if ( r.sourceCrs() )
  {
    mySummary << r.sourceCrs();
  }
  else
  {
    mySummary += "Undefined";
  }

  mySummary += "\n\tDest Spatial Ref Sys  : ";
  if ( r.destCRS() )
  {
    mySummary << r.destCRS();
  }
  else
  {
    mySummary += "Undefined";
  }
#endif

  mySummary += QLatin1String( "\nCoordinate Transform def ends \n%%%%%%%%%%%%%%%%%%%%%%%%\n" );
  return os << mySummary.toLocal8Bit().data() << std::endl;
}
#endif


#endif // QGSCOORDINATETRANSFORM_H
