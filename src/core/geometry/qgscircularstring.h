/***************************************************************************
                         qgscircularstring.h
                         ---------------------
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

#ifndef QGSCIRCULARSTRING_H
#define QGSCIRCULARSTRING_H

#include <QVector>

#include "qgis_core.h"
#include "qgis.h"
#include "qgscurve.h"


/**
 * \ingroup core
 * \class QgsCircularString
 * \brief Circular string geometry type
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsCircularString: public QgsCurve
{
  public:
    QgsCircularString();

    bool operator==( const QgsCurve &other ) const override;
    bool operator!=( const QgsCurve &other ) const override;

    QString geometryType() const override;
    int dimension() const override;
    QgsCircularString *clone() const override SIP_FACTORY;
    void clear() override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    QByteArray asWkb() const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QString asJson( int precision = 17 ) const override;

    bool isEmpty() const override;
    int numPoints() const override;

    /**
     * Returns the point at index i within the circular string.
     */
    QgsPoint pointN( int i ) const;

    void points( QgsPointSequence &pts SIP_OUT ) const override;

    /**
     * Sets the circular string's points
     */
    void setPoints( const QgsPointSequence &points );

    double length() const override;
    QgsPoint startPoint() const override;
    QgsPoint endPoint() const override;
    QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;
    QgsCircularString *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const override SIP_FACTORY;
    void draw( QPainter &p ) const override;
    void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false ) override;
    void transform( const QTransform &t ) override;
    void addToPainterPath( QPainterPath &path ) const override;
    void drawAsPolygon( QPainter &p ) const override;
    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;
    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, bool *leftOf SIP_OUT = nullptr, double epsilon = 4 * DBL_EPSILON ) const override;
    bool pointAt( int node, QgsPoint &point, QgsVertexId::VertexType &type ) const override;
    void sumUpArea( double &sum SIP_OUT ) const override;
    bool hasCurvedSegments() const override;
    double vertexAngle( QgsVertexId vertex ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;
    QgsCircularString *reversed() const override  SIP_FACTORY;
    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;
    bool dropZValue() override;
    bool dropMValue() override;
    double xAt( int index ) const override;
    double yAt( int index ) const override;
#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsCircularString.
     * Should be used by qgsgeometry_cast<QgsCircularString *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsCircularString *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::CircularString )
        return static_cast<const QgsCircularString *>( geom );
      return nullptr;
    }
#endif

  protected:
    QgsCircularString *createEmptyWithSameType() const override SIP_FACTORY;
    QgsRectangle calculateBoundingBox() const override;

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;

#if 0
    static void arcTo( QPainterPath &path, QPointF pt1, QPointF pt2, QPointF pt3 );
#endif
    //bounding box of a single segment
    static QgsRectangle segmentBoundingBox( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3 );
    static QgsPointSequence compassPointsOnSegment( double p1Angle, double p2Angle, double p3Angle, double centerX, double centerY, double radius );
    static double closestPointOnArc( double x1, double y1, double x2, double y2, double x3, double y3,
                                     const QgsPoint &pt, QgsPoint &segmentPt,  QgsVertexId &vertexAfter, bool *leftOf, double epsilon );
    void insertVertexBetween( int after, int before, int pointOnCircle );
    void deleteVertex( int i );

};

// clazy:excludeall=qstring-allocations

#endif // QGSCIRCULARSTRING_H
