/***************************************************************************
                         qgsprofilerequest.h
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSPROFILEREQUEST_H
#define QGSPROFILEREQUEST_H

#include "qgis_core.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"

#include <memory>

class QgsCurve;
class QgsAbstractTerrainProvider;

/**
 * \brief Encapsulates properties and constraints relating to fetching elevation profiles from different sources.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProfileRequest
{

  public:

    /**
     * Constructor for QgsProfileRequest.
     *
     * The \a curve argument specifies the line along which the profile should be generated. Ownership is transferred
     * to the request.
     */
    QgsProfileRequest( QgsCurve *curve SIP_TRANSFER );

    /**
     * Copy constructor.
     */
    QgsProfileRequest( const QgsProfileRequest &other );

    ~QgsProfileRequest();

    /**
     * Assignment operator
     */
    QgsProfileRequest &operator=( const QgsProfileRequest &other );

    bool operator==( const QgsProfileRequest &other ) const;
    bool operator!=( const QgsProfileRequest &other ) const;

    /**
     * Sets the cross section profile \a curve, which represents the line along which the profile should be generated.
     *
     * Ownership of \a curve is transferred to the request.
     *
     * The coordinate reference system of the \a curve is set via setCrs().
     *
     * \see profileCurve()
     */
    QgsProfileRequest &setProfileCurve( QgsCurve *curve SIP_TRANSFER );

    /**
     * Returns the cross section profile curve, which represents the line along which the profile should be generated.
     *
     * The coordinate reference system of the curve is retrieved via crs().
     *
     * \see setProfileCurve()
     */
    QgsCurve *profileCurve() const;

    /**
     * Sets the desired Coordinate Reference System (\a crs) for the profile.
     *
     * This also represents the CRS associated with the profileCurve().
     *
     * \see crs()
     */
    QgsProfileRequest &setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the desired Coordinate Reference System for the profile.
     *
     * This also represents the CRS associated with the profileCurve().
     *
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Returns the transform context, for use when transforming coordinates from a source
     * to the request's crs()
     *
     * \see setTransformContext()
     */
    QgsCoordinateTransformContext transformContext() const;

    /**
     * Sets the transform \a context, for use when transforming coordinates from a source
     * to the request's crs()
     *
     * \see transformContext()
     */
    QgsProfileRequest &setTransformContext( const QgsCoordinateTransformContext &context );

    /**
     * Sets the tolerance of the request (in crs() units).
     *
     * This value determines how far from the profileCurve() is appropriate for inclusion of results. For instance,
     * when a profile is generated for a point vector layer this tolerance distance will dictate how far from the
     * actual profile curve a point can reside within to be included in the results. Other sources may completely
     * ignore this tolerance if it is not appropriate for the particular source.
     *
     * \see tolerance()
     */
    QgsProfileRequest &setTolerance( double tolerance );

    /**
     * Returns the tolerance of the request (in crs() units).
     *
     * This value determines how far from the profileCurve() is appropriate for inclusion of results. For instance,
     * when a profile is generated for a point vector layer this tolerance distance will dictate how far from the
     * actual profile curve a point can reside within to be included in the results. Other sources may completely
     * ignore this tolerance if it is not appropriate for the particular source.
     *
     * \see setTolerance()
     */
    double tolerance() const { return mTolerance; }

    /**
     * Sets the terrain \a provider.
     *
     * Ownership of \a provider is transferred to the request.
     *
     * \see terrainProvider()
     */
    QgsProfileRequest &setTerrainProvider( QgsAbstractTerrainProvider *provider SIP_TRANSFER );

    /**
     * Returns the terrain provider.
     *
     * \see setTerrainProvider()
     */
    QgsAbstractTerrainProvider *terrainProvider() const;

  private:

    std::unique_ptr< QgsCurve> mCurve;
    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateTransformContext mTransformContext;

    double mTolerance = 0;

    std::unique_ptr< QgsAbstractTerrainProvider > mTerrainProvider;

};

#endif // QGSPROFILEREQUEST_H
