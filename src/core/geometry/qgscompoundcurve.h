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

#ifndef QGSCOMPOUNDCURVE_H
#define QGSCOMPOUNDCURVE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscurve.h"

/**
 * \ingroup core
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
    ~QgsCompoundCurve() override;

    bool equals( const QgsCurve &other ) const override;

    QString geometryType() const override SIP_HOLDGIL;
    int dimension() const override SIP_HOLDGIL;
    QgsCompoundCurve *clone() const override SIP_FACTORY;
    void clear() override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;

    //curve interface
    double length() const override SIP_HOLDGIL;
    QgsPoint startPoint() const override SIP_HOLDGIL;
    QgsPoint endPoint() const override SIP_HOLDGIL;
    void points( QgsPointSequence &pts SIP_OUT ) const override;
    int numPoints() const override SIP_HOLDGIL;
    bool isEmpty() const override SIP_HOLDGIL;
    bool isValid( QString &error SIP_OUT, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const override;
    int indexOf( const QgsPoint &point ) const final;

    /**
     * Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve
    */
    QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;

    QgsCompoundCurve *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const override SIP_FACTORY;
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) override;
    bool boundingBoxIntersects( const QgsRectangle &rectangle ) const override SIP_HOLDGIL;
    const QgsAbstractGeometry *simplifiedTypeRef() const override SIP_HOLDGIL;

    /**
     * Returns the number of curves in the geometry.
     */
    int nCurves() const SIP_HOLDGIL { return mCurves.size(); }

    /**
     * Returns the curve at the specified index.
     */
    const QgsCurve *curveAt( int i ) const SIP_HOLDGIL;

    /**
     * Adds a curve to the geometry (takes ownership).
     *
     * Since QGIS 3.20, if \a extendPrevious is TRUE, then adding a LineString when the last existing curve
     * in the compound curve is also a LineString will cause the existing linestring to be
     * extended with the newly added LineString vertices instead of appending a whole new
     * LineString curve to the compound curve. This can result in simplified compound curves with lesser number
     * of component curves while still being topologically identical to the desired result.
     */
    void addCurve( QgsCurve *c SIP_TRANSFER, bool extendPrevious = false );

    /**
     * Removes a curve from the geometry.
     * \param i index of curve to remove
     */
    void removeCurve( int i );

    /**
     * Adds a vertex to the end of the geometry.
     */
    void addVertex( const QgsPoint &pt );

    /**
     * Condenses the curves in this geometry by combining adjacent linestrings a to a single continuous linestring,
     * and combining adjacent circularstrings to a single continuous circularstring.
     *
     * \since QGIS 3.20
     */
    void condenseCurves();

    /**
     * Converts the vertex at the given position from/to circular
     * \returns FALSE if atVertex does not correspond to a valid vertex
     * on this geometry (including if this geometry is a Point),
     * or if the specified vertex can't be converted (e.g. start/end points).
     *
     * \since QGIS 3.20
     */
    bool toggleCircularAtVertex( QgsVertexId position );

    void draw( QPainter &p ) const override;
    void transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d = Qgis::TransformDirection::Forward, bool transformZ = false ) override  SIP_THROW( QgsCsException );
    void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 ) override;
    void addToPainterPath( QPainterPath &path ) const override;
    void drawAsPolygon( QPainter &p ) const override;
    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;
    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, int *leftOf SIP_OUT = nullptr, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const override;
    bool pointAt( int node, QgsPoint &point, Qgis::VertexType &type ) const override;
    void sumUpArea( double &sum SIP_OUT ) const override;

    //! Appends first point if not already closed.
    void close();

    bool hasCurvedSegments() const override;
    double vertexAngle( QgsVertexId vertex ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;
    QgsCompoundCurve *reversed() const override SIP_FACTORY;
    QgsPoint *interpolatePoint( double distance ) const override SIP_FACTORY;
    QgsCompoundCurve *curveSubstring( double startDistance, double endDistance ) const override SIP_FACTORY;

    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;

    bool dropZValue() override;
    bool dropMValue() override;
    void swapXy() override;

    double xAt( int index ) const override SIP_HOLDGIL;
    double yAt( int index ) const override SIP_HOLDGIL;

    bool transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback = nullptr ) override;
    void scroll( int firstVertexIndex ) final;

#ifndef SIP_RUN
    void filterVertices( const std::function< bool( const QgsPoint & ) > &filter ) override;
    void transformVertices( const std::function< QgsPoint( const QgsPoint & ) > &transform ) override;
    std::tuple< std::unique_ptr< QgsCurve >, std::unique_ptr< QgsCurve > > splitCurveAtVertex( int index ) const final;

    /**
     * Cast the \a geom to a QgsCompoundCurve.
     * Should be used by qgsgeometry_cast<QgsCompoundCurve *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline static const QgsCompoundCurve *cast( const QgsAbstractGeometry *geom )
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::CompoundCurve )
        return static_cast<const QgsCompoundCurve *>( geom );
      return nullptr;
    }
#endif

    QgsCompoundCurve *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsCompoundCurve: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:

    int compareToSameClass( const QgsAbstractGeometry *other ) const final;
    QgsRectangle calculateBoundingBox() const override;

  private:
    QVector< QgsCurve * > mCurves;

    /**
     * Turns a vertex id for the compound curve into one or more ids for the subcurves
     * \returns the index of the subcurve or -1 in case of error
    */
    QVector< QPair<int, QgsVertexId> > curveVertexId( QgsVertexId id ) const;

};

// clazy:excludeall=qstring-allocations

#endif // QGSCOMPOUNDCURVE_H
