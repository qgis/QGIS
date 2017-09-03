/***************************************************************************
                        qgsmultilinestring.h
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

#ifndef QGSMULTILINESTRINGV2_H
#define QGSMULTILINESTRINGV2_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsmulticurve.h"

/** \ingroup core
 * \class QgsMultiLineString
 * \brief Multi line string geometry collection.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsMultiLineString: public QgsMultiCurve
{
  public:
    QgsMultiLineString();
    virtual QString geometryType() const override { return QStringLiteral( "MultiLineString" ); }
    QgsMultiLineString *clone() const override SIP_FACTORY;

    bool fromWkt( const QString &wkt ) override;

    // inherited: int wkbSize() const;
    // inherited: unsigned char* asWkb( int& binarySize ) const;
    // inherited: QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    //! Adds a geometry and takes ownership. Returns true in case of success
    virtual bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER ) override;

    /** Returns the geometry converted to the more generic curve type QgsMultiCurve
    \returns the converted geometry. Caller takes ownership*/
    QgsAbstractGeometry *toCurveType() const override SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsMultiLineString.
     * Should be used by qgsgeometry_cast<QgsMultiLineString *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsMultiLineString *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::MultiLineString )
        return static_cast<const QgsMultiLineString *>( geom );
      return nullptr;
    }
#endif
  protected:

    virtual bool wktOmitChildType() const override { return true; }
};

#endif // QGSMULTILINESTRINGV2_H
