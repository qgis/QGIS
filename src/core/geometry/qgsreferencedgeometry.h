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

#include "qgis.h"
#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"

/**
 * \class QgsReferencedGeometryPrimitive
 * \ingroup core
 * A template based class for storing geometry primitives with an associated reference system.
 *
 * QgsReferencedGeometryPrimitive classes represent some form of geometry primitive
 * (such as rectangles) which have an optional coordinate reference system
 * associated with them.
 *
 * \since QGIS 3.0
 * \see QgsReferencedRectangle
 * \note Not available in Python bindings (although SIP file is present for specific implementations).
 */
template<typename T>
class CORE_EXPORT QgsReferencedGeometryPrimitive
{
  public:

    /**
     * Constructor for QgsReferencedGeometryPrimitive, for the specified \a primitive and \a crs.
     */
    QgsReferencedGeometryPrimitive( T primitive, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() )
      : mPrimitive( primitive )
      , mCrs( crs )
    {}

    /**
     * Returns the geometry primitive.
     */
    T primitive() const { return mPrimitive; }

    /**
     * Returns the geometry primitive.
     */
    T &primitive() { return mPrimitive; }

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

    T mPrimitive;
    QgsCoordinateReferenceSystem mCrs;

};

/**
 * A QgsRectangle with associated coordinate reference system.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsReferencedRectangle : public QgsReferencedGeometryPrimitive< QgsRectangle >
{
  public:

    /**
     * Construct a default optional expression.
     * It will be disabled and with an empty expression.
     */
    QgsReferencedRectangle();

    QgsRectangle &rect() { return primitive(); }


};

#endif // QGSREFERENCEDGEOMETRY_H




