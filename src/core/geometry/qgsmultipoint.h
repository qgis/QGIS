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

#ifndef QGSMULTIPOINTV2_H
#define QGSMULTIPOINTV2_H

#include "qgis_core.h"
#include "qgis.h"
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
    QgsMultiPoint();

    QString geometryType() const override;
    QgsMultiPoint *clone() const override SIP_FACTORY;
    QgsMultiPoint *toCurveType() const override SIP_FACTORY;
    bool fromWkt( const QString &wkt ) override;
    void clear() override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QString asJson( int precision = 17 ) const override;
    int nCoordinates() const override;
    bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER ) override;
    bool insertGeometry( QgsAbstractGeometry *g SIP_TRANSFER, int index ) override;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;

#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsLineString.
     * Should be used by qgsgeometry_cast<QgsLineString *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsMultiPoint *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::MultiPoint )
        return static_cast<const QgsMultiPoint *>( geom );
      return nullptr;
    }
#endif
  protected:
    QgsMultiPoint *createEmptyWithSameType() const override SIP_FACTORY;
    bool wktOmitChildType() const override;

};

// clazy:excludeall=qstring-allocations

#endif // QGSMULTIPOINTV2_H
