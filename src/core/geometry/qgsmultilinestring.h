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

#ifndef QGSMULTILINESTRING_H
#define QGSMULTILINESTRING_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmulticurve.h"

class QgsLineString;

/**
 * \ingroup core
 * \class QgsMultiLineString
 * \brief Multi line string geometry collection.
 */
class CORE_EXPORT QgsMultiLineString: public QgsMultiCurve
{
  public:

    /**
     * Constructor for an empty multilinestring geometry.
     */
    QgsMultiLineString() SIP_HOLDGIL;

    /**
     * Constructor for a multilinestring containing the specified \a linestrings.
     *
     * The \a linestrings will be internally cloned.
     *
     * \since QGIS 3.38
     */
    QgsMultiLineString( const QList< QgsLineString > &linestrings ) SIP_HOLDGIL;

    /**
     * Constructor for a multilinestring containing the specified \a linestrings.
     *
     * Ownership of the \a linestrings will be transferred to the multilinestring.
     *
     * \since QGIS 3.38
     */
    QgsMultiLineString( const QList< QgsLineString * > &linestrings SIP_TRANSFER ) SIP_HOLDGIL;

#ifndef SIP_RUN

    /**
     * Returns the line string with the specified \a index.
     *
     * \since QGIS 3.16
     */
    QgsLineString *lineStringN( int index );
#else

    /**
     * Returns the line string with the specified \a index.
     *
     * \throws IndexError if no line string with the specified index exists.
     *
     * \since QGIS 3.16
     */
    SIP_PYOBJECT lineStringN( int index ) SIP_TYPEHINT( QgsLineString );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numGeometries() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( sipCpp->lineStringN( a0 ), sipType_QgsLineString, NULL );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the line string with the specified \a index.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.16
     */
    const QgsLineString *lineStringN( int index ) const;
#endif

    QString geometryType() const override SIP_HOLDGIL;
    QgsMultiLineString *clone() const override SIP_FACTORY;
    void clear() override;
    bool fromWkt( const QString &wkt ) override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;
    bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER ) override;
    bool addGeometries( const QVector< QgsAbstractGeometry * > &geometries SIP_TRANSFER ) final;
    bool insertGeometry( QgsAbstractGeometry *g SIP_TRANSFER, int index ) override;
    QgsMultiLineString *simplifyByDistance( double tolerance ) const override SIP_FACTORY;

    /**
     * Returns the geometry converted to the more generic curve type QgsMultiCurve
     * \returns the converted geometry. Caller takes ownership
    */
    QgsMultiCurve *toCurveType() const override SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsMultiLineString.
     * Should be used by qgsgeometry_cast<QgsMultiLineString *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     */
    inline static const QgsMultiLineString *cast( const QgsAbstractGeometry *geom ) // cppcheck-suppress duplInheritedMember
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::MultiLineString )
        return static_cast<const QgsMultiLineString *>( geom );
      return nullptr;
    }
#endif

    QgsMultiLineString *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsMultiLineString: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Re-write the measure ordinate (or add one, if it isn't already there) interpolating
     * the measure between the supplied \a start and \a end values.
     *
     * \since QGIS 3.36
     */
    QgsMultiLineString *measuredLine( double start, double end ) const SIP_FACTORY;

  protected:

    bool wktOmitChildType() const override;
};

// clazy:excludeall=qstring-allocations

#endif // QGSMULTILINESTRING_H
