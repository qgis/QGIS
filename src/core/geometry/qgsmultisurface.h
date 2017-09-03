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

#ifndef QGSMULTISURFACEV2_H
#define QGSMULTISURFACEV2_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsgeometrycollection.h"

/** \ingroup core
 * \class QgsMultiSurface
 * \brief Multi surface geometry collection.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsMultiSurface: public QgsGeometryCollection
{
  public:
    QgsMultiSurface();
    virtual QString geometryType() const override { return QStringLiteral( "MultiSurface" ); }
    QgsMultiSurface *clone() const override SIP_FACTORY;

    bool fromWkt( const QString &wkt ) override;

    // inherited: int wkbSize() const;
    // inherited: unsigned char* asWkb( int& binarySize ) const;
    // inherited: QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;


    //! Adds a geometry and takes ownership. Returns true in case of success
    virtual bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER ) override;

    virtual QgsAbstractGeometry *boundary() const override SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsMultiSurface.
     * Should be used by qgsgeometry_cast<QgsMultiSurface *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsMultiSurface *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( !geom )
        return nullptr;

      QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( geom->wkbType() );

      if ( flatType == QgsWkbTypes::MultiSurface
           || flatType == QgsWkbTypes::MultiPolygon )
        return static_cast<const QgsMultiSurface *>( geom );
      return nullptr;
    }
#endif

};

#endif // QGSMULTISURFACEV2_H
