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
class QgsVector3D;

/**
 * \ingroup core
* \brief Class for doing transforms between two map coordinate systems.
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
* \warning Since QGIS 3.20 The QgsCoordinateTransform class can perform time-dependent transformations
* between a static and dynamic CRS based on either the source OR destination CRS coordinate epoch,
* however dynamic CRS to dynamic CRS transformations are not currently supported.
*
* \see QgsDatumTransform
* \see QgsCoordinateTransformContext
*/
class CORE_EXPORT QgsCoordinateTransform
{

  public:

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
     * \warning Since QGIS 3.20 The QgsCoordinateTransform class can perform time-dependent transformations
     * between a static and dynamic CRS based on either the source OR destination CRS coordinate epoch,
     * however dynamic CRS to dynamic CRS transformations are not currently supported.
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
     * \warning Since QGIS 3.20 The QgsCoordinateTransform class can perform time-dependent transformations
     * between a static and dynamic CRS based on either the source OR destination CRS coordinate epoch,
     * however dynamic CRS to dynamic CRS transformations are not currently supported.
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
     * \deprecated will be removed in QGIS 4.0. Use the constructor with a QgsCoordinateTransformContext argument instead.
     * \since QGIS 3.0
     */
    Q_DECL_DEPRECATED explicit QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source,
        const QgsCoordinateReferenceSystem &destination,
        int sourceDatumTransformId,
        int destinationDatumTransformId ) SIP_DEPRECATED;

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
    QgsPointXY transform( const QgsPointXY &point, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const SIP_THROW( QgsCsException );

    /**
     * Transform the point specified by x,y from the source CRS to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param x x coordinate of point to transform
     * \param y y coordinate of point to transform
     * \param direction transform direction (defaults to ForwardTransform)
     * \returns transformed point
     */
    QgsPointXY transform( double x, double y, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const;

    /**
     * Transform the point specified in 3D coordinates from the source CRS to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param point coordinates of point to transform
     * \param direction transform direction (defaults to ForwardTransform)
     * \returns transformed point
     * \since QGIS 3.18
     */
    QgsVector3D transform( const QgsVector3D &point, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const;

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
    QgsRectangle transformBoundingBox( const QgsRectangle &rectangle, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward, bool handle180Crossover = false ) const SIP_THROW( QgsCsException );

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
    void transformInPlace( double &x, double &y, double &z, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const SIP_THROW( QgsCsException );

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
    void transformInPlace( float &x, float &y, double &z, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const SIP_SKIP;

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
    void transformInPlace( float &x, float &y, float &z, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const SIP_SKIP;

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
                           Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const SIP_SKIP;

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
                           Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const SIP_SKIP;

    /**
     * Transforms a polygon to the destination coordinate system.
     * \param polygon polygon to transform (occurs in place)
     * \param direction transform direction (defaults to forward transformation)
     */
    void transformPolygon( QPolygonF &polygon, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const SIP_THROW( QgsCsException );

    /**
     * Transforms a rectangle to the destination CRS.
     * If the direction is ForwardTransform then coordinates are transformed from source to destination,
     * otherwise points are transformed from destination to source CRS.
     * \param rectangle rectangle to transform
     * \param direction transform direction (defaults to ForwardTransform)
     * \returns transformed rectangle
     */
    QgsRectangle transform( const QgsRectangle &rectangle, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const SIP_THROW( QgsCsException );

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
    void transformCoords( int numPoint, double *x, double *y, double *z, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const SIP_THROW( QgsCsException );

    /**
     * Returns TRUE if the transform short circuits because the source and destination are equivalent.
     */
    bool isShortCircuited() const;

    /**
     * Returns a Proj string representing the coordinate operation which will be used to transform
     * coordinates.
     *
     * \note The string returned by this method gives the desired coordinate operation string, based on
     * the state of the QgsCoordinateTransformContext object given in the QgsCoordinateTransform's constructor.
     * It may be an empty string if no explicit coordinate operation is required. In order to determine the
     * ACTUAL coordinate operation which is being used by the transform, use the instantiatedCoordinateOperationDetails() call instead.
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will always return
     * an empty string, and the deprecated sourceDatumTransformId() or destinationDatumTransformId() methods should be used instead.
     *
     * \see instantiatedCoordinateOperationDetails()
     * \see setCoordinateOperation()
     * \since QGIS 3.8
     */
    QString coordinateOperation() const;

    /**
     * Returns the transform details representing the coordinate operation which is being used to transform
     * coordinates.
     *
     * This may differ from the result returned by coordinateOperation() if the desired coordinate
     * operation was not successfully instantiated.
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will always return
     * an empty result, and the deprecated sourceDatumTransformId() or destinationDatumTransformId() methods should be used instead.
     *
     * \see coordinateOperation()
     * \since QGIS 3.10.2
     */
    QgsDatumTransform::TransformDetails instantiatedCoordinateOperationDetails() const;

    /**
     * Sets a Proj string representing the coordinate \a operation which will be used to transform
     * coordinates.
     *
     * \warning It is the caller's responsibility to ensure that \a operation is a valid Proj
     * coordinate operation string.
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will ignore this setting,
     * and the deprecated setSourceDatumTransformId() or setDestinationDatumTransformId() methods should be used instead.
     *
     * \see coordinateOperation()
     * \since QGIS 3.8
     */
    void setCoordinateOperation( const QString &operation ) const;

    /**
     * Sets whether "ballpark" fallback transformations can be used in the case that the specified
     * coordinate operation fails (such as when coordinates from outside a required grid shift file
     * are transformed). See fallbackOperationOccurred() for further details.
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will ignore this setting.
     *
     * \warning If setBallparkTransformsAreAppropriate() is set to TRUE, this setting will be ignored
     * and fallback transformations will always be permitted.
     *
     * \see allowFallbackTransforms()
     * \see setBallparkTransformsAreAppropriate()
     * \since QGIS 3.12
     */
    void setAllowFallbackTransforms( bool allowed );

    /**
     * Returns whether "ballpark" fallback transformations will be used in the case that the specified
     * coordinate operation fails (such as when coordinates from outside a required grid shift file
     * are transformed). See fallbackOperationOccurred() for further details.
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will ignore this setting.
     *
     * \see setAllowFallbackTransforms()
     * \see setBallparkTransformsAreAppropriate()
     * \since QGIS 3.12
     */
    bool allowFallbackTransforms() const;

    /**
     * Sets whether approximate "ballpark" results are appropriate for this coordinate transform.
     *
     * When a coordinate transform is only being used to generate ballpark results then the
     * \a appropriate argument should be set to TRUE. This indicates that its perfectable
     * acceptable (and even expected!) for the transform to use fallback coordinate operations
     * in the case that the preferred or user-specified operation fails (such as when coordinates
     * from outside of a grid shift file's extent are transformed).
     *
     * When \a appropriate is TRUE, then no warnings will be generated when the transform
     * falls back to a default operation, which may introduce inaccuracies when compared to
     * the default/specified coordinate operation.
     *
     * This should be set when a transform expects that coordinates outside of the direct
     * area of use while be transformed, e.g. when transforming from a global extent to a
     * CRS with a localized area of use.
     *
     * If \a appropriate is FALSE (the default behavior), then transforms MAY STILL fallback to default operations
     * when the preferred or user-specified operation fails, however whenever this occurs
     * a user-visible warning will be generated.
     *
     * If \a appropriate is TRUE, then this setting overrides allowFallbackTransforms()
     * and fallback transforms will always be allowed when required.
     *
     * \warning This setting applies to a single instance of a coordinate transform only,
     * and is not copied when a coordinate transform object is copied or assigned.
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will ignore this setting.
     *
     * \since QGIS 3.12
     */
    void setBallparkTransformsAreAppropriate( bool appropriate );

    /**
     * Sets whether the default fallback operation handler is disabled for this transform instance.
     *
     * If the default handler is \a disabled then it is possible to determine whether a fallback
     * operation occurred by testing fallbackOperationOccurred() immediately after a transformation.
     *
     * \warning This setting applies to a single instance of a coordinate transform only,
     * and is not copied when a coordinate transform object is copied or assigned.
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will never perform fallback operations.
     *
     * \see fallbackOperationOccurred()
     * \since QGIS 3.12
     */
    void disableFallbackOperationHandler( bool disabled );

    /**
     * Returns TRUE if a fallback operation occurred for the most recent transform.
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will never perform fallback operations.
     *
     * \see disableFallbackOperationHandler()
     * \since QGIS 3.12
     */
    bool fallbackOperationOccurred() const;

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
     *
     * \deprecated Unused on builds based on Proj 6.0 or later
     */
    Q_DECL_DEPRECATED int sourceDatumTransformId() const SIP_DEPRECATED;

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
     *
     * \deprecated Unused on builds based on Proj 6.0 or later
     */
    Q_DECL_DEPRECATED void setSourceDatumTransformId( int datumId ) SIP_DEPRECATED;

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
     *
     * \deprecated Unused on builds based on Proj 6.0 or later
     */
    Q_DECL_DEPRECATED int destinationDatumTransformId() const SIP_DEPRECATED;

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
     *
     * \deprecated Unused on builds based on Proj 6.0 or later
     */
    Q_DECL_DEPRECATED void setDestinationDatumTransformId( int datumId ) SIP_DEPRECATED;

#ifndef SIP_RUN

    /**
     * Clears the internal cache used to initialize QgsCoordinateTransform objects.
     * This should be called whenever the srs database has
     * been modified in order to ensure that outdated CRS transforms are not created.
     *
     * If \a disableCache is TRUE then the inbuilt cache will be completely disabled. This
     * argument is for internal use only.
     *
     * \since QGIS 3.0
     */
    static void invalidateCache( bool disableCache = false );
#else

    /**
     * Clears the internal cache used to initialize QgsCoordinateTransform objects.
     * This should be called whenever the srs database has
     * been modified in order to ensure that outdated CRS transforms are not created.
     *
     * \since QGIS 3.0
     */
    static void invalidateCache( bool disableCache SIP_PYARGREMOVE = false );
#endif

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

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsCoordinateTransform: %1 to %2>" ).arg( sipCpp->sourceCrs().isValid() ? sipCpp->sourceCrs().authid() : QStringLiteral( "NULL" ),
                  sipCpp->destinationCrs().isValid() ? sipCpp->destinationCrs().authid() : QStringLiteral( "NULL" ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

#ifndef SIP_RUN

    /**
     * Sets a custom handler to use when a coordinate transform is created between \a sourceCrs and
     * \a destinationCrs, yet the coordinate operation requires a transform \a grid which is not present
     * on the system.
     *
     * \see setCustomMissingPreferredGridHandler()
     * \see setCustomCoordinateOperationCreationErrorHandler()
     * \see setCustomMissingGridUsedByContextHandler()
     *
     * \note Not available in Python bindings
     * \since QGIS 3.8
     */
    static void setCustomMissingRequiredGridHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QgsDatumTransform::GridDetails &grid )> &handler );

    /**
     * Sets a custom handler to use when a coordinate transform is created between \a sourceCrs and
     * \a destinationCrs, yet a preferred (more accurate?) operation is available which could not
     * be created on the system (e.g. due to missing transform grids).
     *
     * \a preferredOperation gives the details of the preferred coordinate operation, and
     * \a availableOperation gives the details of the actual operation to be used during the
     * transform.
     *
     * \see setCustomMissingRequiredGridHandler()
     * \see setCustomCoordinateOperationCreationErrorHandler()
     * \see setCustomMissingGridUsedByContextHandler()
     *
     * \note Not available in Python bindings
     * \since QGIS 3.8
     */
    static void setCustomMissingPreferredGridHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QgsDatumTransform::TransformDetails &preferredOperation,
        const QgsDatumTransform::TransformDetails &availableOperation )> &handler );

    /**
     * Sets a custom handler to use when a coordinate transform was required between \a sourceCrs and
     * \a destinationCrs, yet the coordinate operation could not be created. The \a error argument
     * specifies the error message obtained.
     *
     * \see setCustomMissingRequiredGridHandler()
     * \see setCustomMissingPreferredGridHandler()
     * \see setCustomMissingGridUsedByContextHandler()
     *
     * \note Not available in Python bindings
     * \since QGIS 3.8
     */
    static void setCustomCoordinateOperationCreationErrorHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QString &error )> &handler );

    /**
     * Sets a custom handler to use when a coordinate operation was specified for use between \a sourceCrs and
     * \a destinationCrs by the transform context, yet the coordinate operation could not be created. The \a desiredOperation argument
     * specifies the desired transform details as specified by the context.
     *
     * \see setCustomMissingRequiredGridHandler()
     * \see setCustomMissingPreferredGridHandler()
     * \see setCustomCoordinateOperationCreationErrorHandler()
     *
     * \note Not available in Python bindings
     * \since QGIS 3.8
     */
    static void setCustomMissingGridUsedByContextHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QgsDatumTransform::TransformDetails &desiredOperation )> &handler );


    /**
     * Sets a custom \a handler to use when the desired coordinate operation for use between \a sourceCrs and
     * \a destinationCrs failed and an alternative fallback operation was utilized instead.
     *
     * \since QGIS 3.10.3
     */
    static void setFallbackOperationOccurredHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QString &desiredOperation )> &handler );

    /**
     * Sets a custom \a handler to use when the desired coordinate operation for use between \a sourceCrs and
     * \a destinationCrs is a dynamic CRS to dynamic CRS transform, not currently supported by PROJ.
     *
     * \since QGIS 3.20
     */
    static void setDynamicCrsToDynamicCrsWarningHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs )> &handler );

#endif

  private:

#ifndef SIP_RUN
    friend class QgsProjContext;

    // Only meant to be called by QgsProjContext::~QgsProjContext()
    static void removeFromCacheObjectsBelongingToCurrentThread( void *pj_context );
#endif

    mutable QExplicitlySharedDataPointer<QgsCoordinateTransformPrivate> d;

    //! Transform context
    QgsCoordinateTransformContext mContext;

#ifdef QGISDEBUG
    bool mHasContext = false;
#endif

    mutable QString mLastError;
    bool mBallparkTransformsAreAppropriate = false;
    bool mDisableFallbackHandler = false;
    mutable bool mFallbackOperationOccurred = false;

    bool setFromCache( const QgsCoordinateReferenceSystem &src,
                       const QgsCoordinateReferenceSystem &dest,
                       const QString &coordinateOperationProj, bool allowFallback );

    void addToCache();

    // cache
    static QReadWriteLock sCacheLock;

    /**
     * Map of cached transforms. The keys are formed by pairs of strings uniquely identifying the source and
     * destination CRS, using the auth:id were available or a full WKT2 definition where an auth:id is not available.
     *
     * The same auth_id pairs might have multiple transformations, as they can be based on different coordinate
     * operations, allowance of ballpark transforms, and the source or destination coordinate epoch.
     */
    static QMultiHash< QPair< QString, QString >, QgsCoordinateTransform > sTransforms;
    static bool sDisableCache;


    static std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                                const QgsCoordinateReferenceSystem &destinationCrs,
                                const QString &desiredOperation )> sFallbackOperationOccurredHandler;

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
