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

#ifndef QGSMULTIPOLYGONV2_H
#define QGSMULTIPOLYGONV2_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsmultisurface.h"

/** \ingroup core
 * \class QgsMultiPolygonV2
 * \brief Multi polygon geometry collection.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsMultiPolygonV2: public QgsMultiSurface
{
  public:
    QgsMultiPolygonV2();
    virtual QString geometryType() const override { return QStringLiteral( "MultiPolygon" ); }
    QgsMultiPolygonV2 *clone() const override SIP_FACTORY;

    bool fromWkt( const QString &wkt ) override;

    // inherited: int wkbSize() const;
    // inherited: unsigned char* asWkb( int& binarySize ) const;
    // inherited: QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    //! Adds a geometry and takes ownership. Returns true in case of success
    virtual bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER ) override;

    /** Returns the geometry converted to the more generic curve type QgsMultiSurface
    \returns the converted geometry. Caller takes ownership*/
    QgsAbstractGeometry *toCurveType() const override SIP_FACTORY;

    virtual QgsAbstractGeometry *boundary() const override SIP_FACTORY;
#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsMultiPolygonV2.
     * Should be used by qgsgeometry_cast<QgsMultiPolygonV2 *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsMultiPolygonV2 *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::MultiPolygon )
        return static_cast<const QgsMultiPolygonV2 *>( geom );
      return nullptr;
    }
#endif

  protected:

    virtual bool wktOmitChildType() const override { return true; }
};

#endif // QGSMULTIPOLYGONV2_H
