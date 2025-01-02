/***************************************************************************
                             qgsreferencedgeometry.h
                             ----------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSREFERENCEDGEOMETRY_H
#define QGSREFERENCEDGEOMETRY_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"
#include "qgsgeometry.h"

/**
 * \class QgsReferencedGeometryBase
 * \ingroup core
 * \brief A base class for geometry primitives which are stored with an associated reference system.
 *
 * QgsReferencedGeometryBase classes represent some form of geometry primitive
 * (such as rectangles) which have an optional coordinate reference system
 * associated with them.
 *
 * \see QgsReferencedRectangle
 */
class CORE_EXPORT QgsReferencedGeometryBase
{
  public:

    /**
     * Constructor for QgsReferencedGeometryBase, with the specified \a crs.
     */
    QgsReferencedGeometryBase( const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Returns the associated coordinate reference system, or an invalid CRS if
     * no reference system is set.
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    /**
     * Sets the associated \a crs. Set to an invalid CRS if
     * no reference system is required.
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs ) { mCrs = crs; }

  private:

    QgsCoordinateReferenceSystem mCrs;

};

/**
 * \ingroup core
 * \brief A QgsRectangle with associated coordinate reference system.
 */
class CORE_EXPORT QgsReferencedRectangle : public QgsRectangle, public QgsReferencedGeometryBase
{
    Q_GADGET

  public:

    /**
     * Constructor for QgsReferencedRectangle, with the specified initial \a rectangle
     * and \a crs.
     */
    QgsReferencedRectangle( const QgsRectangle &rectangle, const QgsCoordinateReferenceSystem &crs );

    QgsReferencedRectangle() = default;

    //! Allows direct construction of QVariants from rectangle.
    operator QVariant() const // cppcheck-suppress duplInheritedMember
    {
      return QVariant::fromValue( *this );
    }

    bool operator==( const QgsReferencedRectangle &other ) const;
    bool operator!=( const QgsReferencedRectangle &other ) const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsReferencedRectangle: %1 (%2)>" ).arg( sipCpp->asWktCoordinates(), sipCpp->crs().authid() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

};

Q_DECLARE_METATYPE( QgsReferencedRectangle )

/**
 * \ingroup core
 * \brief A QgsPointXY with associated coordinate reference system.
 */
class CORE_EXPORT QgsReferencedPointXY : public QgsPointXY, public QgsReferencedGeometryBase
{
  public:

    /**
     * Constructor for QgsReferencedPointXY, with the specified initial \a point
     * and \a crs.
     */
    QgsReferencedPointXY( const QgsPointXY &point, const QgsCoordinateReferenceSystem &crs );

    QgsReferencedPointXY() = default;

    //! Allows direct construction of QVariants from point.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

    bool operator==( const QgsReferencedPointXY &other );
    bool operator!=( const QgsReferencedPointXY &other );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsReferencedPointXY: %1 (%2)>" ).arg( sipCpp->asWkt(), sipCpp->crs().authid() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

};

Q_DECLARE_METATYPE( QgsReferencedPointXY )

/**
 * \ingroup core
 * \brief A QgsGeometry with associated coordinate reference system.
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsReferencedGeometry : public QgsGeometry, public QgsReferencedGeometryBase
{
  public:

    /**
     * Constructor for QgsReferencedGeometry, with the specified initial \a geometry
     * and \a crs.
     */
    QgsReferencedGeometry( const QgsGeometry &geometry, const QgsCoordinateReferenceSystem &crs );

    QgsReferencedGeometry() = default;

    //! Allows direct construction of QVariants from geometry.
    operator QVariant() const // cppcheck-suppress duplInheritedMember
    {
      return QVariant::fromValue( *this );
    }

    bool operator==( const QgsReferencedGeometry &other ) const;
    bool operator!=( const QgsReferencedGeometry &other ) const;

    /**
     * Construct a new QgsReferencedGeometry from referenced \a point
     */
    static QgsReferencedGeometry fromReferencedPointXY( const QgsReferencedPointXY &point );

    /**
     * Construct a new QgsReferencedGeometry from referenced \a rectangle
     */
    static QgsReferencedGeometry fromReferencedRect( const QgsReferencedRectangle &rectangle );


#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsReferencedGeometry: %1 (%2)>" ).arg( sipCpp->asWkt(), sipCpp->crs().authid() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

};

Q_DECLARE_METATYPE( QgsReferencedGeometry )

#endif // QGSREFERENCEDGEOMETRY_H
