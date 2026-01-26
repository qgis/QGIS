/***************************************************************************
    qgsgeometrycheckcontext.h
    ---------------------
    begin                : September 2018
    copyright            : (C) 2018 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYCHECKCONTEXT_H
#define QGSGEOMETRYCHECKCONTEXT_H

#include "qgis_analysis.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsfeaturepool.h"

/**
 * \ingroup analysis
 * \brief Base configuration for geometry checks.
 *
 * \note This class is a technology preview and unstable API.
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryCheckContext
{
  public:
    /**
     * Creates a new QgsGeometryCheckContext.
     * \param precision The precision used to define geometry check tolerance. Tolerance is calculated as pow(10, -precision)
     * \param mapCrs The coordinate system in which calculations should be done
     * \param transformContext The coordinate transform context
     * \param mProject The project used to resolve additional layers
     * \param uniqueIdFieldIndex The index of the unique ID field used to identify features. If set to valid field index, geometry checker will fail if
     * this field is not unique (since QGIS 4.0)
     *
     */
    QgsGeometryCheckContext( int precision, const QgsCoordinateReferenceSystem &mapCrs, const QgsCoordinateTransformContext &transformContext, const QgsProject *mProject = nullptr, const int uniqueIdFieldIndex = -1 );

    /**
     * The tolerance to allow for in geometry checks.
     * Will be calculated as pow(10, -precision) in the constructor.
     * I.e. if the precision is 4 (decimal digits), this will be 0.0001.
     */
    const double tolerance;

    /**
     * The tolerance to allow for in geometry checks.
     * Will be calculated as pow(10, -precision/2) in the constructor.
     * I.e. if the precision is 4 (decimal digits), this will be 0.01.
     * Should be used for areas, where the precision is squared.
     */
    const double reducedTolerance;

    /**
     * The coordinate system in which calculations should be done.
     */
    const QgsCoordinateReferenceSystem mapCrs;

    /**
     * The coordinate transform context with which transformations will be done.
     */
    const QgsCoordinateTransformContext transformContext;

    /**
     * The index of the unique ID field used to identify features.
     *
     * \since QGIS 4.0
     */
    const int uniqueIdFieldIndex;

    /**
     * The project can be used to resolve additional layers.
     *
     * This must only be accessed from the main thread (i.e. do not access from the collectError method)
     *
     * \since QGIS 3.10
     */
    const QgsProject *project() const;

  private:
    const QgsProject *mProject;

  private:
#ifdef SIP_RUN
    QgsGeometryCheckContext( const QgsGeometryCheckContext &rh )
    {}
#endif
};

#endif // QGSGEOMETRYCHECKCONTEXT_H
