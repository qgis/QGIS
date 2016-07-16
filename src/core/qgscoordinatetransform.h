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
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform_p.h"
#include "qgscsexception.h"

class QgsCoordinateTransformPrivate;
class QgsPoint;
class QgsRectangle;
class QPolygonF;

/** \ingroup core
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
*/
class CORE_EXPORT QgsCoordinateTransform
{

  public:

    //! Enum used to indicate the direction (forward or inverse) of the transform
    enum TransformDirection
    {
      ForwardTransform,     /*!< Transform from source to destination CRS. */
      ReverseTransform      /*!< Transform from destination to source CRS. */
    };

    /** Default constructor, creates an invalid QgsCoordinateTransform. */
    QgsCoordinateTransform();

    /** Constructs a QgsCoordinateTransform using QgsCoordinateReferenceSystem objects.
     * @param source source CRS, typically of the layer's coordinate system
     * @param destination CRS, typically of the map canvas coordinate system
     */
    QgsCoordinateTransform( const QgsCoordinateReferenceSystem& source,
                            const QgsCoordinateReferenceSystem& destination );

    /** Constructs a QgsCoordinateTransform using CRS ID of source and destination CRS */
    QgsCoordinateTransform( long sourceSrsId, long destinationSrsId );

    /*!
     * Constructs a QgsCoordinateTransform using the Well Known Text representation
     * of the layer and map canvas coordinate systems
     * @param sourceWkt WKT, typically of the layer's coordinate system
     * @param destinationWkt WKT, typically of the map canvas coordinate system
     */
    QgsCoordinateTransform( const QString& sourceWkt, const QString& destinationWkt );

    /*!
     * Constructs a QgsCoordinateTransform using a Spatial Reference Id
     * of the layer and map canvas coordinate system as Wkt
     * @param sourceSrid Spatial Ref Id of the layer's coordinate system
     * @param destinationWkt Wkt of the map canvas coordinate system
     * @param sourceCrsType On of the enum members defined in QgsCoordinateReferenceSystem::CrsType
     */
    QgsCoordinateTransform( long sourceSrid,
                            const QString& destinationWkt,
                            QgsCoordinateReferenceSystem::CrsType sourceCrsType = QgsCoordinateReferenceSystem::PostgisCrsId );


    /*!
     * Returns true if the coordinate transform is valid, ie both the source and destination
     * CRS have been set and are valid.
     * @note added in QGIS 3.0
     */
    bool isValid() const;

    /*!
     * Sets the source coordinate reference system.
     * @param crs CRS to transform coordinates from
     * @see sourceCrs()
     * @see setDestinationCrs()
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem& crs );

    /*!
     * Sets the destination coordinate reference system.
     * @param crs CRS to transform coordinates to
     * @see destinationCrs()
     * @see setSourceCrs()
     */
    void setDestinationCrs( const QgsCoordinateReferenceSystem& crs );

    /** Returns the source coordinate reference system, which the transform will
     * transform coordinates from.
     * @see setSourceCrs()
     * @see destinationCrs()
     */
    QgsCoordinateReferenceSystem sourceCrs() const;

    /** Returns the destination coordinate reference system, which the transform will
     * transform coordinates to.
     * @see setDestinationCrs()
     * @see sourceCrs()
     */
    QgsCoordinateReferenceSystem destinationCrs() const;

    /** Transform the point from Source Coordinate System to Destination Coordinate System
     * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
     * otherwise points are transformed from map canvas CS to layerCS.
     * @param point Point to transform
     * @param direction TransformDirection (defaults to ForwardTransform)
     * @return QgsPoint in Destination Coordinate System
     */
    QgsPoint transform( const QgsPoint& point, TransformDirection direction = ForwardTransform ) const;

    /** Transform the point specified by x,y from Source Coordinate System to Destination Coordinate System
     * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
     * otherwise points are transformed from map canvas CS to layerCS.
     * @param x x cordinate of point to transform
     * @param y y coordinate of point to transform
     * @param direction TransformDirection (defaults to ForwardTransform)
     * @return QgsPoint in Destination Coordinate System
     */
    QgsPoint transform( const double x, const double y, TransformDirection direction = ForwardTransform ) const;

    /** Transforms a QgsRectangle to the desttination coordinate system.
     * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
     * otherwise points are transformed from map canvas CS to layerCS.
     * It assumes that rect is a bounding box, and creates a bounding box
     * in the proejcted CS, so that all points in source rectangle is within
     * returned rectangle.
     * @param rectangle rectangle to transform
     * @param direction TransformDirection (defaults to ForwardTransform)
     * @param handle180Crossover set to true if destination crs is geographic and handling of extents crossing the 180 degree
     * longitude line is required
     * @return QgsRectangle in Destination Coordinate System
     */
    QgsRectangle transformBoundingBox( const QgsRectangle& rectangle, TransformDirection direction = ForwardTransform, const bool handle180Crossover = false ) const;

    // Same as for the other transform() functions, but alters the x
    // and y variables in place. The second one works with good old-fashioned
    // C style arrays.
    void transformInPlace( double& x, double& y, double &z, TransformDirection direction = ForwardTransform ) const;

    //! @note not available in python bindings
    void transformInPlace( float& x, float& y, double &z, TransformDirection direction = ForwardTransform ) const;
    //! @note not available in python bindings
    void transformInPlace( float& x, float& y, float& z, TransformDirection direction = ForwardTransform ) const;
    //! @note not available in python bindings
    void transformInPlace( QVector<float>& x, QVector<float>& y, QVector<float>& z,
                           TransformDirection direction = ForwardTransform ) const;

    //! @note not available in python bindings
    void transformInPlace( QVector<double>& x, QVector<double>& y, QVector<double>& z,
                           TransformDirection direction = ForwardTransform ) const;

    void transformPolygon( QPolygonF& polygon, TransformDirection direction = ForwardTransform ) const;

    /** Transform a QgsRectangle to the dest Coordinate system
     * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
     * otherwise points are transformed from map canvas CS to layerCS.
     * @param rectangle rectangle to transform
     * @param direction TransformDirection (defaults to ForwardTransform)
     * @return QgsRectangle in Destination Coordinate System
     */
    QgsRectangle transform( const QgsRectangle &rectangle, TransformDirection direction = ForwardTransform ) const;

    /** Transform an array of coordinates to a different Coordinate System
     * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
     * otherwise points are transformed from map canvas CS to layerCS.
     * @param numPoint number of coordinates in arrays
     * @param x array of x coordinates to transform
     * @param y array of y coordinates to transform
     * @param z array of z coordinates to transform
     * @param direction TransformDirection (defaults to ForwardTransform)
     * @return QgsRectangle in Destination Coordinate System
     */
    void transformCoords( int numPoint, double *x, double *y, double *z, TransformDirection direction = ForwardTransform ) const;

    /** Returns true if the transform short circuits because the source and destination are equivalent.
     */
    bool isShortCircuited() const;

    /** Returns list of datum transformations for the given src and dest CRS
     * @note not available in python bindings
     */
    static QList< QList< int > > datumTransformations( const QgsCoordinateReferenceSystem& srcCRS, const QgsCoordinateReferenceSystem& destinationCrs );
    static QString datumTransformString( int datumTransform );
    /** Gets name of source and dest geographical CRS (to show in a tooltip)
        @return epsgNr epsg code of the transformation (or 0 if not in epsg db)*/
    static bool datumTransformCrsInfo( int datumTransform, int& epsgNr, QString& srcProjection, QString& dstProjection, QString &remarks, QString &scope, bool &preferred, bool &deprecated );

    int sourceDatumTransform() const;
    void setSourceDatumTransform( int dt );
    int destinationDatumTransform() const;
    void setDestinationDatumTransform( int dt );

    //!initialize is used to actually create the Transformer instance
    void initialise();

    /** Restores state from the given Dom node.
     * @param node The node from which state will be restored
     * @return bool True on success, False on failure
     * @see writeXml()
     */
    bool readXml( const QDomNode& node );

    /** Stores state to the given Dom node in the given document
     * @param node The node in which state will be restored
     * @param document The document in which state will be stored
     * @return bool True on success, False on failure
     * @see readXml()
     */
    bool writeXml( QDomNode & node, QDomDocument & document ) const;

  private:

    static void searchDatumTransform( const QString& sql, QList< int >& transforms );

    QExplicitlySharedDataPointer<QgsCoordinateTransformPrivate> d;
};

//! Output stream operator
inline std::ostream& operator << ( std::ostream& os, const QgsCoordinateTransform &r )
{
  QString mySummary( "\n%%%%%%%%%%%%%%%%%%%%%%%%\nCoordinate Transform def begins:" );
  mySummary += "\n\tInitialised? : ";
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

  mySummary += ( "\nCoordinate Transform def ends \n%%%%%%%%%%%%%%%%%%%%%%%%\n" );
  return os << mySummary.toLocal8Bit().data() << std::endl;
}


#endif // QGSCOORDINATETRANSFORM_H
