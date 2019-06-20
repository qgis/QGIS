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

/**
 * \class QgsReferencedGeometryBase
 * \ingroup core
 * A base class for geometry primitives which are stored with an associated reference system.
 *
 * QgsReferencedGeometryBase classes represent some form of geometry primitive
 * (such as rectangles) which have an optional coordinate reference system
 * associated with them.
 *
 * \see QgsReferencedRectangle
 * \since QGIS 3.0
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
 * A QgsRectangle with associated coordinate reference system.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsReferencedRectangle : public QgsRectangle, public QgsReferencedGeometryBase
{
  public:

    /**
     * Constructor for QgsReferencedRectangle, with the specified initial \a rectangle
     * and \a crs.
     */
    QgsReferencedRectangle( const QgsRectangle &rectangle, const QgsCoordinateReferenceSystem &crs );

    /**
     * Constructor for QgsReferencedRectangle.
     */
    QgsReferencedRectangle() = default;

    //! Allows direct construction of QVariants from rectangle.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

};

Q_DECLARE_METATYPE( QgsReferencedRectangle )

/**
 * \ingroup core
 * A QgsPointXY with associated coordinate reference system.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsReferencedPointXY : public QgsPointXY, public QgsReferencedGeometryBase
{
  public:

    /**
     * Constructor for QgsReferencedPointXY, with the specified initial \a point
     * and \a crs.
     */
    QgsReferencedPointXY( const QgsPointXY &point, const QgsCoordinateReferenceSystem &crs );

    /**
     * Constructor for QgsReferencedPointXY.
     */
    QgsReferencedPointXY() = default;

    //! Allows direct construction of QVariants from point.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

};

Q_DECLARE_METATYPE( QgsReferencedPointXY )

#endif // QGSREFERENCEDGEOMETRY_H
