/***************************************************************************
                        qgsmultisurface.h
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

#ifndef QGSMULTISURFACE_H
#define QGSMULTISURFACE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsgeometrycollection.h"


class QgsSurface;

/**
 * \ingroup core
 * \class QgsMultiSurface
 * \brief Multi surface geometry collection.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsMultiSurface: public QgsGeometryCollection
{
  public:

    /**
     * Constructor for an empty multisurface geometry.
     */
    QgsMultiSurface() SIP_HOLDGIL;

#ifndef SIP_RUN

    /**
     * Returns the surface with the specified \a index.
     *
     * \since QGIS 3.16
     */
    QgsSurface *surfaceN( int index );
#else

    /**
     * Returns the surface with the specified \a index.
     *
     * \throws IndexError if no surface with the specified index exists.
     *
     * \since QGIS 3.16
     */
    SIP_PYOBJECT surfaceN( int index ) SIP_TYPEHINT( QgsSurface );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numGeometries() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( sipCpp->surfaceN( a0 ), sipType_QgsSurface, NULL );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the surface with the specified \a index.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.16
     */
    const QgsSurface *surfaceN( int index ) const;
#endif

    QString geometryType() const override SIP_HOLDGIL;
    void clear() override;
    QgsMultiSurface *clone() const override SIP_FACTORY;
    QgsMultiSurface *toCurveType() const override SIP_FACTORY;
    bool fromWkt( const QString &wkt ) override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;
    bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER ) override;
    bool insertGeometry( QgsAbstractGeometry *g SIP_TRANSFER, int index ) override;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsMultiSurface.
     * Should be used by qgsgeometry_cast<QgsMultiSurface *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline static const QgsMultiSurface *cast( const QgsAbstractGeometry *geom )
    {
      if ( !geom )
        return nullptr;

      const QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( geom->wkbType() );

      if ( flatType == QgsWkbTypes::MultiSurface
           || flatType == QgsWkbTypes::MultiPolygon )
        return static_cast<const QgsMultiSurface *>( geom );
      return nullptr;
    }
#endif

    QgsMultiSurface *createEmptyWithSameType() const override SIP_FACTORY;

};

// clazy:excludeall=qstring-allocations

#endif // QGSMULTISURFACE_H
