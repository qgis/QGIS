/***************************************************************************
                        qgsmulticurve.h
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

#ifndef QGSMULTICURVE_H
#define QGSMULTICURVE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsgeometrycollection.h"

/**
 * \ingroup core
 * \class QgsMultiCurve
 * \brief Multi curve geometry collection.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsMultiCurve: public QgsGeometryCollection
{
  public:
    QgsMultiCurve();


#ifndef SIP_RUN

    /**
     * Returns the curve with the specified \a index.
     *
     * \since QGIS 3.16
     */
    QgsCurve *curveN( int index );
#else

    /**
     * Returns the curve with the specified \a index.
     *
     * \throws IndexError if no curve with the specified index exists.
     *
     * \since QGIS 3.16
     */
    SIP_PYOBJECT curveN( int index ) SIP_TYPEHINT( QgsCurve );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numGeometries() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( sipCpp->curveN( a0 ), sipType_QgsCurve, NULL );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the curve with the specified \a index.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.16
     */
    const QgsCurve *curveN( int index ) const;
#endif

    QString geometryType() const override SIP_HOLDGIL;
    QgsMultiCurve *clone() const override SIP_FACTORY;
    void clear() override;
    QgsMultiCurve *toCurveType() const override SIP_FACTORY;
    bool fromWkt( const QString &wkt ) override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;
    bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER ) override;
    bool insertGeometry( QgsAbstractGeometry *g SIP_TRANSFER, int index ) override;

    /**
     * Returns a copy of the multi curve, where each component curve has had its line direction reversed.
     * \since QGIS 2.14
     */
    QgsMultiCurve *reversed() const SIP_FACTORY;

    QgsAbstractGeometry *boundary() const override SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsMultiCurve.
     * Should be used by qgsgeometry_cast<QgsMultiCurve *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline static const QgsMultiCurve *cast( const QgsAbstractGeometry *geom )
    {
      if ( !geom )
        return nullptr;

      const QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( geom->wkbType() );
      if ( flatType == QgsWkbTypes::MultiCurve
           || flatType == QgsWkbTypes::MultiLineString )
        return static_cast<const QgsMultiCurve *>( geom );
      return nullptr;
    }
#endif

    QgsMultiCurve *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsMultiCurve: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

};

// clazy:excludeall=qstring-allocations

#endif // QGSMULTICURVE_H
