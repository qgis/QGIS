/***************************************************************************
                         qgscompoundcurve.h
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

#ifndef QGSCOMPOUNDCURVEV2_H
#define QGSCOMPOUNDCURVEV2_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgscurve.h"

/** \ingroup core
 * \class QgsCompoundCurve
 * \brief Compound curve geometry type
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsCompoundCurve: public QgsCurve
{
  public:
    QgsCompoundCurve();
    QgsCompoundCurve( const QgsCompoundCurve &curve );
    QgsCompoundCurve &operator=( const QgsCompoundCurve &curve );
    ~QgsCompoundCurve();

    virtual bool operator==( const QgsCurve &other ) const override;
    virtual bool operator!=( const QgsCurve &other ) const override;

    virtual QString geometryType() const override { return QStringLiteral( "CompoundCurve" ); }
    virtual int dimension() const override { return 1; }
    virtual QgsCompoundCurve *clone() const override SIP_FACTORY;
    virtual void clear() override;

    virtual bool fromWkb( QgsConstWkbPtr &wkb ) override;
    virtual bool fromWkt( const QString &wkt ) override;

    QByteArray asWkb() const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    //curve interface
    virtual double length() const override;
    virtual QgsPoint startPoint() const override;
    virtual QgsPoint endPoint() const override;
    virtual void points( QgsPointSequence &pts SIP_OUT ) const override;
    virtual int numPoints() const override;
    bool isEmpty() const override;

    /** Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;

    /** Returns the number of curves in the geometry.
     */
    int nCurves() const { return mCurves.size(); }

    /** Returns the curve at the specified index.
     */
    const QgsCurve *curveAt( int i ) const;

    /** Adds a curve to the geometry (takes ownership)
     */
    void addCurve( QgsCurve *c SIP_TRANSFER );

    /** Removes a curve from the geometry.
     * \param i index of curve to remove
     */
    void removeCurve( int i );

    /** Adds a vertex to the end of the geometry.
     */
    void addVertex( const QgsPoint &pt );

    void draw( QPainter &p ) const override;
    void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false ) override;
    void transform( const QTransform &t ) override;
    void addToPainterPath( QPainterPath &path ) const override;
    void drawAsPolygon( QPainter &p ) const override;

    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    virtual bool deleteVertex( QgsVertexId position ) override;

    virtual double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT,
                                   QgsVertexId &vertexAfter SIP_OUT, bool *leftOf SIP_OUT,
                                   double epsilon ) const override;

    bool pointAt( int node, QgsPoint &point, QgsVertexId::VertexType &type ) const override;

    void sumUpArea( double &sum SIP_OUT ) const override;

    //! Appends first point if not already closed.
    void close();

    bool hasCurvedSegments() const override;

    /** Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
        \param vertex the vertex id
        \returns rotation in radians, clockwise from north*/
    double vertexAngle( QgsVertexId vertex ) const override;

    virtual QgsCompoundCurve *reversed() const override SIP_FACTORY;

    virtual bool addZValue( double zValue = 0 ) override;
    virtual bool addMValue( double mValue = 0 ) override;

    virtual bool dropZValue() override;
    virtual bool dropMValue() override;

    double xAt( int index ) const override;
    double yAt( int index ) const override;
#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsCompoundCurve.
     * Should be used by qgsgeometry_cast<QgsCompoundCurve *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsCompoundCurve *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::CompoundCurve )
        return static_cast<const QgsCompoundCurve *>( geom );
      return nullptr;
    }
#endif

  protected:

    virtual QgsRectangle calculateBoundingBox() const override;

  private:
    QList< QgsCurve * > mCurves;

    /** Turns a vertex id for the compound curve into one or more ids for the subcurves
        \returns the index of the subcurve or -1 in case of error*/
    QList< QPair<int, QgsVertexId> > curveVertexId( QgsVertexId id ) const;

};

#endif // QGSCOMPOUNDCURVEV2_H
