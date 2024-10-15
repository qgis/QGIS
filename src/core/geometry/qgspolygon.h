/***************************************************************************
                         qgspolygon.h
                         -------------------
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

#ifndef QGSPOLYGON_H
#define QGSPOLYGON_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscurvepolygon.h"

class QgsLineString;

/**
 * \ingroup core
 * \class QgsPolygon
 * \brief Polygon geometry type.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsPolygon: public QgsCurvePolygon
{
  public:


    /**
     * Constructor for an empty polygon geometry.
     */
    QgsPolygon() SIP_HOLDGIL;

    /**
     * Constructor for QgsPolygon, with the specified \a exterior ring and interior \a rings.
     *
     * Ownership of \a exterior and \a rings is transferred to the polygon.
     *
     * \since QGIS 3.14
     */
    QgsPolygon( QgsLineString *exterior SIP_TRANSFER, const QList< QgsLineString * > &rings SIP_TRANSFER = QList< QgsLineString * >() ) SIP_HOLDGIL;

    QString geometryType() const override SIP_HOLDGIL;
    QgsPolygon *clone() const override SIP_FACTORY;
    void clear() override;
    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QgsPolygon *surfaceToPolygon() const override SIP_FACTORY;

    /**
     * Returns the geometry converted to the more generic curve type QgsCurvePolygon
     * \returns the converted geometry. Caller takes ownership
    */
    QgsCurvePolygon *toCurveType() const override SIP_FACTORY;

    void addInteriorRing( QgsCurve *ring SIP_TRANSFER ) override;
    //overridden to handle LineString25D rings
    void setExteriorRing( QgsCurve *ring SIP_TRANSFER ) override;

    QgsAbstractGeometry *boundary() const override SIP_FACTORY;

    /**
     * Returns the distance from a point to the boundary of the polygon (either the
     * exterior ring or any closer interior rings). The returned distance will be
     * negative if the point lies outside the polygon.
     * \since QGIS 3.0
     */
    double pointDistanceToBoundary( double x, double y ) const;

#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsPolygonV2.
     * Should be used by qgsgeometry_cast<QgsPolygon *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline static const QgsPolygon *cast( const QgsAbstractGeometry *geom ) // cppcheck-suppress duplInheritedMember
    {
      if ( !geom )
        return nullptr;

      const Qgis::WkbType flatType = QgsWkbTypes::flatType( geom->wkbType() );

      if ( flatType == Qgis::WkbType::Polygon
           || flatType == Qgis::WkbType::Triangle )
        return static_cast<const QgsPolygon *>( geom );
      return nullptr;
    }
#endif

    QgsPolygon *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsPolygon: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:

    friend class QgsCurvePolygon;

};
#endif // QGSPOLYGON_H
