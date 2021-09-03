/***************************************************************************
  qgsgeocodercontext.h
  ---------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOCODERCONTEXT_H
#define QGSGEOCODERCONTEXT_H

#include "qgis_core.h"

#include "qgscoordinatetransformcontext.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"

/**
 * \ingroup core
 * \brief Encapsulates the context of a geocoding operation.
 *
 * \since QGIS 3.18
*/
class CORE_EXPORT QgsGeocoderContext
{

  public:

    /**
     * Constructor for QgsGeocoderContext, with the specified \a transformContext.
     */
    QgsGeocoderContext( const QgsCoordinateTransformContext &transformContext );

    /**
     * Returns the coordinate transform context, which should be used whenever the
     * geocoder constructs a coordinate transform.
     *
     * \see setTransformContext()
     */
    QgsCoordinateTransformContext transformContext() const { return mTransformContext; }

    /**
     * Sets the coordinate transform \a context, which should be used whenever the
     * geocoder constructs a coordinate transform.
     *
     * \see transformContext()
     */
    void setTransformContext( const QgsCoordinateTransformContext &context ) { mTransformContext = context; }

    /**
     * Returns the optional area of interest, which can be used to indicate the desired
     * geographic area where geocoding results are desired.
     *
     * The area of interest can be a polygon geometry, in which case it represents the extent
     * to use for filtering candidate results, or a point geometry, in which case it represents
     * a "target point" for prioritizing closer results.
     *
     * The coordinate reference system for the area of interest can be retrieved via
     * areaOfInterestCrs().
     *
     * \see setAreaOfInterest()
     * \see areaOfInterestCrs()
     */
    QgsGeometry areaOfInterest() const { return mAreaOfInterest; }

    /**
     * Sets the optional \a area of interest, which can be used to indicate the desired
     * geographic area where geocoding results are desired.
     *
     * The area of interest can be a polygon geometry, in which case it represents the extent
     * to use for filtering candidate results, or a point geometry, in which case it represents
     * a "target point" for prioritizing closer results.
     *
     * The coordinate reference system for the area of interest can be set via
     * setAreaOfInterestCrs().
     *
     * \see areaOfInterest()
     * \see setAreaOfInterestCrs()
     */
    void setAreaOfInterest( const QgsGeometry &area ) { mAreaOfInterest = area; }

    /**
     * Returns the coordinate reference system for the area of interest, which can be used to indicate the desired
     * geographic area where geocoding results are desired.
     *
     * \see areaOfInterest()
     * \see setAreaOfInterestCrs()
     */
    QgsCoordinateReferenceSystem areaOfInterestCrs() const { return mAreaOfInterestCrs; }

    /**
     * Sets the \a crs for the area of interest, which can be used to indicate the desired
     * geographic area where geocoding results are desired.
     *
     * \see areaOfInterestCrs()
     * \see setAreaOfInterest()
     */
    void setAreaOfInterestCrs( const QgsCoordinateReferenceSystem &crs ) { mAreaOfInterestCrs = crs; }

  private:

    QgsCoordinateTransformContext mTransformContext;
    QgsGeometry mAreaOfInterest;
    QgsCoordinateReferenceSystem mAreaOfInterestCrs;
};

#endif // QGSGEOCODERCONTEXT_H
