/***************************************************************************
                         qgslinestring.h
                         -----------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLINESTRINGV2_H
#define QGSLINESTRINGV2_H


#include <QPolygonF>

#include "qgis_core.h"
#include "qgis.h"
#include "qgscurve.h"
#include "qgscompoundcurve.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsgeometry.cpp.
 * See details in QEP #17
 ****************************************************************************/

/**
 * \ingroup core
 * \class QgsLineString
 * \brief Line string geometry type, with support for z-dimension and m-values.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsLineString: public QgsCurve
{
  public:
    QgsLineString();

    /**
     * Construct a linestring from a vector of points.
     * Z and M type will be set based on the type of the first point
     * in the vector.
     * \since QGIS 3.0
     */
    QgsLineString( const QVector<QgsPoint> &points );

    /**
     * Construct a linestring from arrays of coordinates. If the z or m
     * arrays are non-empty then the resultant linestring will have
     * z and m types accordingly.
     * This constructor is more efficient then calling setPoints()
     * or repeatedly calling addVertex()
     * \since QGIS 3.0
     */
    QgsLineString( const QVector<double> &x, const QVector<double> &y,
                   const QVector<double> &z = QVector<double>(),
                   const QVector<double> &m = QVector<double>() );

    /**
     * Construct a linestring from list of points.
     * This constructor is more efficient then calling setPoints()
     * or repeatedly calling addVertex()
     * \since QGIS 3.0
     */
    QgsLineString( const QVector<QgsPointXY> &points );

    bool operator==( const QgsCurve &other ) const override;
    bool operator!=( const QgsCurve &other ) const override;

    /**
     * Returns the specified point from inside the line string.
     * \param i index of point, starting at 0 for the first point
     */
    QgsPoint pointN( int i ) const;

    double xAt( int index ) const override;
    double yAt( int index ) const override;

    /**
     * Returns the z-coordinate of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0
     * \returns z-coordinate of node, or ``nan`` if index is out of bounds or the line
     * does not have a z dimension
     * \see setZAt()
     */
    double zAt( int index ) const;

    /**
     * Returns the m value of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0
     * \returns m value of node, or ``nan`` if index is out of bounds or the line
     * does not have m values
     * \see setMAt()
     */
    double mAt( int index ) const;

    /**
     * Sets the x-coordinate of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string.
     * \param x x-coordinate of node
     * \see xAt()
     */
    void setXAt( int index, double x );

    /**
     * Sets the y-coordinate of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string.
     * \param y y-coordinate of node
     * \see yAt()
     */
    void setYAt( int index, double y );

    /**
     * Sets the z-coordinate of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string, and the line string must have z-dimension.
     * \param z z-coordinate of node
     * \see zAt()
     */
    void setZAt( int index, double z );

    /**
     * Sets the m value of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string, and the line string must have m values.
     * \param m m value of node
     * \see mAt()
     */
    void setMAt( int index, double m );

    /**
     * Resets the line string to match the specified list of points. The line string will
     * inherit the dimensionality of the first point in the list.
     * \param points new points for line string. If empty, line string will be cleared.
     */
    void setPoints( const QgsPointSequence &points );

    /**
     * Appends the contents of another line string to the end of this line string.
     * \param line line to append. Ownership is not transferred.
     */
    void append( const QgsLineString *line );

    /**
     * Adds a new vertex to the end of the line string.
     * \param pt vertex to add
     */
    void addVertex( const QgsPoint &pt );

    //! Closes the line string by appending the first point to the end of the line, if it is not already closed.
    void close();

    /**
     * Returns the geometry converted to the more generic curve type QgsCompoundCurve
        \returns the converted geometry. Caller takes ownership*/
    QgsCompoundCurve *toCurveType() const override SIP_FACTORY;

    /**
     * Extends the line geometry by extrapolating out the start or end of the line
     * by a specified distance. Lines are extended using the bearing of the first or last
     * segment in the line.
     * \since QGIS 3.0
     */
    void extend( double startDistance, double endDistance );

    //reimplemented methods

    QString geometryType() const override;
    int dimension() const override;
    QgsLineString *clone() const override SIP_FACTORY;
    void clear() override;
    bool isEmpty() const override;
    QgsLineString *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const override SIP_FACTORY;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    QByteArray asWkb() const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QString asJson( int precision = 17 ) const override;

    //curve interface
    double length() const override;
    QgsPoint startPoint() const override;
    QgsPoint endPoint() const override;

    /**
     * Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override  SIP_FACTORY;

    int numPoints() const override;
    int nCoordinates() const override;
    void points( QgsPointSequence &pt SIP_OUT ) const override;

    void draw( QPainter &p ) const override;

    void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false ) override;
    void transform( const QTransform &t ) override;

    void addToPainterPath( QPainterPath &path ) const override;
    void drawAsPolygon( QPainter &p ) const override;

    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;

    QgsLineString *reversed() const override SIP_FACTORY;

    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, bool *leftOf SIP_OUT = nullptr, double epsilon = 4 * DBL_EPSILON ) const override;
    bool pointAt( int node, QgsPoint &point, QgsVertexId::VertexType &type ) const override;

    QgsPoint centroid() const override;

    void sumUpArea( double &sum SIP_OUT ) const override;
    double vertexAngle( QgsVertexId vertex ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;
    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;

    bool dropZValue() override;
    bool dropMValue() override;

    bool convertTo( QgsWkbTypes::Type type ) override;

#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsLineString.
     * Should be used by qgsgeometry_cast<QgsLineString *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsLineString *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::LineString )
        return static_cast<const QgsLineString *>( geom );
      return nullptr;
    }
#endif
  protected:
    QgsLineString *createEmptyWithSameType() const override SIP_FACTORY;
    QgsRectangle calculateBoundingBox() const override;

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;

    void importVerticesFromWkb( const QgsConstWkbPtr &wkb );

    /**
     * Resets the line string to match the line string in a WKB geometry.
     * \param type WKB type
     * \param wkb WKB representation of line geometry
     */
    void fromWkbPoints( QgsWkbTypes::Type type, const QgsConstWkbPtr &wkb );

    friend class QgsPolygon;
    friend class QgsTriangle;

};

// clazy:excludeall=qstring-allocations

#endif // QGSLINESTRINGV2_H
