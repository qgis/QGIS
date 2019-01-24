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
    QString geometryType() const override;
    QgsMultiCurve *clone() const override SIP_FACTORY;
    void clear() override;
    QgsMultiCurve *toCurveType() const override SIP_FACTORY;
    bool fromWkt( const QString &wkt ) override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QString asJson( int precision = 17 ) const override;
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
    inline const QgsMultiCurve *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( !geom )
        return nullptr;

      QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( geom->wkbType() );
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
