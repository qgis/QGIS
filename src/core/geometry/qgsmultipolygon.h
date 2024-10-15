/***************************************************************************
                        qgsmultipolygon.h
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMULTIPOLYGON_H
#define QGSMULTIPOLYGON_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmultisurface.h"

class QgsPolygon;

/**
 * \ingroup core
 * \class QgsMultiPolygon
 * \brief Multi polygon geometry collection.
 */
class CORE_EXPORT QgsMultiPolygon: public QgsMultiSurface
{
  public:

    /**
     * Constructor for an empty multipolygon geometry.
     */
    QgsMultiPolygon() SIP_HOLDGIL;

    /**
     * Constructor for a multipolygon containing the specified \a polygons.
     *
     * The \a polygons will be internally cloned.
     *
     * \since QGIS 3.38
     */
    QgsMultiPolygon( const QList< QgsPolygon > &polygons ) SIP_HOLDGIL;

    /**
     * Constructor for a multipolygon containing the specified \a polygons.
     *
     * Ownership of the \a polygons will be transferred to the multipolygon.
     *
     * \since QGIS 3.38
     */
    QgsMultiPolygon( const QList< QgsPolygon * > &polygons SIP_TRANSFER ) SIP_HOLDGIL;

#ifndef SIP_RUN

    /**
     * Returns the polygon with the specified \a index.
     *
     * \since QGIS 3.16
     */
    QgsPolygon *polygonN( int index );
#else

    /**
     * Returns the polygon with the specified \a index.
     *
     * \throws IndexError if no polygon with the specified index exists.
     *
     * \since QGIS 3.16
     */
    SIP_PYOBJECT polygonN( int index ) SIP_TYPEHINT( QgsPolygon );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numGeometries() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( sipCpp->polygonN( a0 ), sipType_QgsPolygon, NULL );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the polygon with the specified \a index.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.16
     */
    const QgsPolygon *polygonN( int index ) const;
#endif

    QString geometryType() const override SIP_HOLDGIL;
    void clear() override;
    QgsMultiPolygon *clone() const override SIP_FACTORY;
    bool fromWkt( const QString &wkt ) override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision  = 17 ) const override SIP_SKIP;
    bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER ) override;
    bool addGeometries( const QVector< QgsAbstractGeometry * > &geometries SIP_TRANSFER ) final;
    bool insertGeometry( QgsAbstractGeometry *g SIP_TRANSFER, int index ) override;
    QgsMultiPolygon *simplifyByDistance( double tolerance ) const override SIP_FACTORY;

    /**
     * Returns the geometry converted to the more generic curve type QgsMultiSurface
     * \returns the converted geometry. Caller takes ownership
    */
    QgsMultiSurface *toCurveType() const override SIP_FACTORY;

    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsMultiPolygonV2.
     * Should be used by qgsgeometry_cast<QgsMultiPolygon *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     */
    inline static const QgsMultiPolygon *cast( const QgsAbstractGeometry *geom ) // cppcheck-suppress duplInheritedMember
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::MultiPolygon )
        return static_cast<const QgsMultiPolygon *>( geom );
      return nullptr;
    }
#endif

    QgsMultiPolygon *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsMultiPolygon: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:

    bool wktOmitChildType() const override;
};

// clazy:excludeall=qstring-allocations

#endif // QGSMULTIPOLYGON_H
