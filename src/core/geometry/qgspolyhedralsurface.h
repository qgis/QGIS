/***************************************************************************
                         qgspolyhedralsurface.h
                         -------------------
    begin                : August 2024
    copyright            : (C) 2024 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOLYHEDRALSURFACE_H
#define QGSPOLYHEDRALSURFACE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssurface.h"
#include "qgspolygon.h"
#include "qgsmultipolygon.h"


/**
 * \ingroup core
 * \class QgsPolyhedralSurface
 * \brief Polyhedral surface geometry type.
 *
 * A polyhedral surface is a collection of polygons which share common boundary segments.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsPolyhedralSurface: public QgsSurface
{
  public:
    QgsPolyhedralSurface();
    QgsPolyhedralSurface( const QgsPolyhedralSurface &p );


    /**
     * Creates a polyhedral surface from a multiPolygon.
     */
    QgsPolyhedralSurface( const QgsMultiPolygon *multiPolygon );

    QgsPolyhedralSurface &operator=( const QgsPolyhedralSurface &p );

#ifndef SIP_RUN
  private:
    bool fuzzyHelper( const QgsAbstractGeometry &other, double epsilon, bool useDistance ) const
    {
      const QgsPolyhedralSurface *otherPolygon = qgsgeometry_cast< const QgsPolyhedralSurface * >( &other );
      if ( !otherPolygon )
        return false;

      //run cheap checks first
      if ( mWkbType != otherPolygon->mWkbType )
        return false;

      if ( mPatches.count() != otherPolygon->mPatches.count() )
        return false;

      for ( int i = 0; i < mPatches.count(); ++i )
      {
        if ( ( !mPatches.at( i ) && otherPolygon->mPatches.at( i ) ) ||
             ( mPatches.at( i ) && !otherPolygon->mPatches.at( i ) ) )
          return false;

        if ( useDistance )
        {
          if ( mPatches.at( i ) && otherPolygon->mPatches.at( i ) &&
               !( *mPatches.at( i ) ).fuzzyDistanceEqual( *otherPolygon->mPatches.at( i ), epsilon ) )
            return false;
        }
        else
        {
          if ( mPatches.at( i ) && otherPolygon->mPatches.at( i ) &&
               !( *mPatches.at( i ) ).fuzzyEqual( *otherPolygon->mPatches.at( i ), epsilon ) )
            return false;
        }
      }

      return true;
    }
#endif

  public:
    bool fuzzyEqual( const QgsAbstractGeometry &other, double epsilon = 1e-8 ) const override SIP_HOLDGIL
    {
      return fuzzyHelper( other, epsilon, false );
    }
    bool fuzzyDistanceEqual( const QgsAbstractGeometry &other, double epsilon = 1e-8 ) const override SIP_HOLDGIL
    {
      return fuzzyHelper( other, epsilon, true );
    }

    bool operator==( const QgsAbstractGeometry &other ) const override
    {
      return fuzzyEqual( other, 1e-8 );
    }

    bool operator!=( const QgsAbstractGeometry &other ) const override
    {
      return !operator==( other );
    }

    ~QgsPolyhedralSurface() override;

    QString geometryType() const override SIP_HOLDGIL;
    int dimension() const final SIP_HOLDGIL;
    QgsPolyhedralSurface *clone() const override SIP_FACTORY;
    void clear() override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    bool isValid( QString &error SIP_OUT, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const override;

    int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;
    QString asKml( int precision = 17 ) const override;
    void normalize() override SIP_HOLDGIL;

    //surface interface
    double area() const override SIP_HOLDGIL;
    double perimeter() const override SIP_HOLDGIL;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
    QgsPolyhedralSurface *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0, bool removeRedundantPoints = false ) const override SIP_FACTORY;
    QgsPolyhedralSurface *simplifyByDistance( double tolerance ) const override SIP_FACTORY;
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) override;
    bool boundingBoxIntersects( const QgsBox3D &box3d ) const override SIP_HOLDGIL;

    /**
     * Returns the number of patches contained with the polyhedral surface.
     *
     * \see patchN()
     */
    int numPatches() const SIP_HOLDGIL
    {
      return mPatches.size();
    }

#ifndef SIP_RUN

    /**
     * Retrieves a patch from the polyhedral surface. The first patch has index 0.
     *
     * \see numPatches()
     */
    const QgsPolygon *patchN( int i ) const
    {
      if ( i < 0 || i >= mPatches.size() )
      {
        return nullptr;
      }
      return mPatches.at( i );
    }

    /**
     * Retrieves a patch from the polyhedral surface. The first patch has index 0.
     *
     * \see numPatches()
     */
    QgsPolygon *patchN( int i )
    {
      if ( i < 0 || i >= mPatches.size() )
      {
        return nullptr;
      }
      return mPatches.at( i );
    }
#else

    /**
     * Retrieves a patch from the polyhedral surface. The first patch has index 0.
     *
     * \throws IndexError if no patch with the specified index exists.
     *
     * \see numPatches()
     */
    SIP_PYOBJECT patchN( int i ) SIP_HOLDGIL SIP_TYPEHINT( QgsPolygon );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numPatches() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( const_cast< QgsPolygon * >( sipCpp->patchN( a0 ) ), sipType_QgsPolygon, NULL );
    }
    % End
#endif

    /**
     * Sets all patches, transferring ownership to the polyhedral surface.
     */
    virtual void setPatches( const QVector<QgsPolygon *> &patches SIP_TRANSFER );

    /**
     * Adds a patch to the geometry, transferring ownership to the polyhedral surface.
     */
    virtual void addPatch( QgsPolygon *patch SIP_TRANSFER );

#ifndef SIP_RUN

    /**
     * Removes a patch from the polyhedral surface. The first patch has index 0.
     * The corresponding patch is removed from the polyhedral surface and deleted. If a patch was successfully removed
     * the function will return TRUE.
     *
     */
    bool removePatch( int patchIndex );
#else

    /**
     * Removes a patch from the polyhedral surface. The first patch has index 0.
     * The corresponding patch is removed from the polyhedral surface and deleted.
     *
     * \throws IndexError if no patch with the specified index exists.
     *
     */
    bool removePatch( int ringIndex );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numPatches() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return PyBool_FromLong( sipCpp->removePatch( a0 ) );
    }
    % End
#endif

    QPainterPath asQPainterPath() const override;
    void draw( QPainter &p ) const override;
    void transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d = Qgis::TransformDirection::Forward, bool transformZ = false ) override SIP_THROW( QgsCsException );
    void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 ) override;

    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;

    QgsCoordinateSequence coordinateSequence() const override;
    int nCoordinates() const override;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;
    bool isEmpty() const override SIP_HOLDGIL;
    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, int *leftOf SIP_OUT = nullptr, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const override;

    bool nextVertex( QgsVertexId &id, QgsPoint &vertex SIP_OUT ) const override;
    void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex SIP_OUT, QgsVertexId &nextVertex SIP_OUT ) const override;
    bool hasCurvedSegments() const final;

    /**
     * Returns a geometry without curves. Caller takes ownership
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve
    */
    QgsAbstractGeometry *segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;

    /**
     * Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
     * \param vertex the vertex id
     * \returns rotation in radians, clockwise from north
     */
    double vertexAngle( QgsVertexId vertex ) const override;

    int vertexCount( int part = 0, int ring = 0 ) const override;
    int ringCount( int part = 0 ) const override SIP_HOLDGIL;
    int partCount() const override SIP_HOLDGIL;
    QgsPoint vertexAt( QgsVertexId id ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;

    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;
    bool dropZValue() override;
    bool dropMValue() override;
    void swapXy() override;

    QgsMultiSurface *toCurveType() const override SIP_FACTORY;

    bool transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback = nullptr ) override;

    /**
     * Converts a polyhedral surface to a multipolygon.
     * Caller takes ownership.
    */
    QgsMultiPolygon *toMultiPolygon() const SIP_FACTORY;

#ifndef SIP_RUN
    void filterVertices( const std::function< bool( const QgsPoint & ) > &filter ) override;
    void transformVertices( const std::function< QgsPoint( const QgsPoint & ) > &transform ) override;

    /**
     * Cast the \a geom to a QgsPolyhedralSurface.
     * Should be used by qgsgeometry_cast<QgsPolyhedralSurface *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     */
    inline static const QgsPolyhedralSurface *cast( const QgsAbstractGeometry *geom ) // cppcheck-suppress duplInheritedMember
    {
      if ( !geom )
        return nullptr;

      const Qgis::WkbType flatType = QgsWkbTypes::flatType( geom->wkbType() );
      if ( flatType == Qgis::WkbType::PolyhedralSurface
           || flatType == Qgis::WkbType::TIN )
        return static_cast<const QgsPolyhedralSurface *>( geom );

      return nullptr;
    }
#endif

    QgsPolyhedralSurface *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsPolyhedralSurface: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End

    /**
     * Returns the number of patches within the polyhedral surface.
     */
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->numPatches();
    % End

    /**
    * Returns the geometry at the specified ``index``.
    *
    * Indexes can be less than 0, in which case they correspond to geometries from the end of the collect. E.g. an index of -1
    * corresponds to the last geometry in the collection.
    *
    * \throws IndexError if no geometry with the specified ``index`` exists.
    */
    SIP_PYOBJECT __getitem__( int index ) SIP_TYPEHINT( QgsPolygon );
    % MethodCode
    const int count = sipCpp->numPatches();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else if ( a0 >= 0 )
    {
      return sipConvertFromType( sipCpp->patchN( a0 ), sipType_QgsPolygon, NULL );
    }
    else
    {
      return sipConvertFromType( sipCpp->patchN( count + a0 ), sipType_QgsPolygon, NULL );
    }
    % End
#endif

  protected:

    int childCount() const override;
    QgsAbstractGeometry *childGeometry( int index ) const override;
    int compareToSameClass( const QgsAbstractGeometry *other ) const override;
    QgsBox3D calculateBoundingBox3D() const override;

    QVector< QgsPolygon * > mPatches;
};

// clazy:excludeall=qstring-allocations

#endif // QGSPOLYHEDRALSURFACE_H
