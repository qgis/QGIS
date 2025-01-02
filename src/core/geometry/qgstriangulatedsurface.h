/***************************************************************************
                         qgstriangulatedsurface.h
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

#ifndef QGSTRIANGULATEDSURFACE_H
#define QGSTRIANGULATEDSURFACE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspolyhedralsurface.h"

class QgsTriangle;


/**
 * \ingroup core
 * \class QgsTriangulatedSurface
 * \brief Triangulated surface geometry type.
 *
 * A TIN (triangulated irregular network) is a PolyhedralSurface consisting only of triangle patches.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsTriangulatedSurface: public QgsPolyhedralSurface
{
  public:
    QgsTriangulatedSurface();
    QgsTriangulatedSurface( const QgsTriangulatedSurface &p );
    QgsTriangulatedSurface &operator=( const QgsTriangulatedSurface &p );

#ifndef SIP_RUN
  private:
    bool fuzzyHelper( const QgsAbstractGeometry &other, double epsilon, bool useDistance ) const
    {
      const QgsTriangulatedSurface *otherTriangulatedSurface = qgsgeometry_cast< const QgsTriangulatedSurface * >( &other );
      if ( !otherTriangulatedSurface )
        return false;

      //run cheap checks first
      if ( mWkbType != otherTriangulatedSurface->mWkbType )
        return false;

      if ( mPatches.count() != otherTriangulatedSurface->mPatches.count() )
        return false;

      for ( int i = 0; i < mPatches.count(); ++i )
      {
        if ( ( !mPatches.at( i ) && otherTriangulatedSurface->mPatches.at( i ) ) ||
             ( mPatches.at( i ) && !otherTriangulatedSurface->mPatches.at( i ) ) )
          return false;

        if ( useDistance )
        {
          if ( mPatches.at( i ) && otherTriangulatedSurface->mPatches.at( i ) &&
               !( *mPatches.at( i ) ).fuzzyDistanceEqual( *otherTriangulatedSurface->mPatches.at( i ), epsilon ) )
            return false;
        }
        else
        {
          if ( mPatches.at( i ) && otherTriangulatedSurface->mPatches.at( i ) &&
               !( *mPatches.at( i ) ).fuzzyEqual( *otherTriangulatedSurface->mPatches.at( i ), epsilon ) )
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

    ~QgsTriangulatedSurface() override;

    QString geometryType() const override SIP_HOLDGIL;
    QgsTriangulatedSurface *clone() const override SIP_FACTORY;
    void clear() override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QString asKml( int precision = 17 ) const override;
    void normalize() override SIP_HOLDGIL;

    //surface interface
    QgsTriangulatedSurface *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0, bool removeRedundantPoints = false ) const override SIP_FACTORY;

    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool deleteVertex( QgsVertexId position ) override;

    /**
     * Sets all triangles, transferring ownership to the polyhedral surface.
     */
    void setTriangles( const QVector<QgsTriangle *> &triangles SIP_TRANSFER );

    /**
     * Adds a patch to the geometry, transferring ownership to the polyhedral surface.
     */
    void addPatch( QgsPolygon *patch SIP_TRANSFER ) override;

    /**
     * Adds a triangle to the geometry, transferring ownership to the polyhedral surface.
     */
    void addTriangle( QgsTriangle *triangle SIP_TRANSFER );

#ifndef SIP_RUN

    /**
     * Returns the triangle with the specified \a index.
     */
    QgsTriangle *triangleN( int index );
#else

    /**
     * Returns the triangle with the specified \a index.
     *
     * \throws IndexError if no polygon with the specified index exists.
     */
    SIP_PYOBJECT triangleN( int index ) SIP_TYPEHINT( QgsPolygon );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numPatches() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( sipCpp->triangleN( a0 ), sipType_QgsTriangle, NULL );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the triangle with the specified \a index.
     *
     * \note Not available in Python bindings
     */
    const QgsTriangle *triangleN( int index ) const;
#endif

#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsTriangulatedSurface.
     * Should be used by qgsgeometry_cast<QgsTriangulatedSurface *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     */
    inline static const QgsTriangulatedSurface *cast( const QgsAbstractGeometry *geom ) // cppcheck-suppress duplInheritedMember
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::TIN )
        return static_cast<const QgsTriangulatedSurface *>( geom );

      return nullptr;
    }
#endif

    QgsTriangulatedSurface *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsTriangulatedSurface: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:

    int compareToSameClass( const QgsAbstractGeometry *other ) const final;
};

// clazy:excludeall=qstring-allocations

#endif // QGSTRIANGULATEDSURFACE_H
