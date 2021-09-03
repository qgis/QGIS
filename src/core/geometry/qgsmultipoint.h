/***************************************************************************
                        qgsmultipoint.h
  -------------------------------------------------------------------
Date                 : 29 Oct 2014
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

#ifndef QGSMULTIPOINT_H
#define QGSMULTIPOINT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsgeometrycollection.h"

/**
 * \ingroup core
 * \class QgsMultiPoint
 * \brief Multi point geometry collection.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsMultiPoint: public QgsGeometryCollection
{
  public:

    /**
     * Constructor for an empty multipoint geometry.
     */
    QgsMultiPoint() SIP_HOLDGIL;

#ifndef SIP_RUN

    /**
     * Returns the point with the specified \a index.
     *
     * \since QGIS 3.16
     */
    QgsPoint *pointN( int index );
#else

    /**
     * Returns the point with the specified \a index.
     *
     * \throws IndexError if no point with the specified index exists.
     *
     * \since QGIS 3.16
     */
    SIP_PYOBJECT pointN( int index ) SIP_TYPEHINT( QgsPoint );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numGeometries() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( sipCpp->pointN( a0 ), sipType_QgsPoint, NULL );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the point with the specified \a index.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.16
     */
    const QgsPoint *pointN( int index ) const;
#endif

    QString geometryType() const override;
    QgsMultiPoint *clone() const override SIP_FACTORY;
    QgsMultiPoint *toCurveType() const override SIP_FACTORY;
    bool fromWkt( const QString &wkt ) override;
    void clear() override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;
    int nCoordinates() const override SIP_HOLDGIL;
    bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER ) override;
    bool insertGeometry( QgsAbstractGeometry *g SIP_TRANSFER, int index ) override;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;
    bool isValid( QString &error SIP_OUT, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const override SIP_HOLDGIL;

#ifndef SIP_RUN
    void filterVertices( const std::function< bool( const QgsPoint & ) > &filter ) override;

    /**
     * Cast the \a geom to a QgsLineString.
     * Should be used by qgsgeometry_cast<QgsLineString *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline static const QgsMultiPoint *cast( const QgsAbstractGeometry *geom )
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::MultiPoint )
        return static_cast<const QgsMultiPoint *>( geom );
      return nullptr;
    }
#endif

    QgsMultiPoint *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsMultiPoint: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:

    bool wktOmitChildType() const override;

};

// clazy:excludeall=qstring-allocations

#endif // QGSMULTIPOINT_H
