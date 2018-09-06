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

struct ANALYSIS_EXPORT QgsGeometryCheckContext
{
    QgsGeometryCheckContext( int precision,
                             const QgsCoordinateReferenceSystem &mapCrs,
                             const QgsCoordinateTransformContext &transformContext );
    const double tolerance;
    const double reducedTolerance;
    const QgsCoordinateReferenceSystem mapCrs;
    const QgsCoordinateTransformContext transformContext;

  private:
#ifdef SIP_RUN
    QgsGeometryCheckContext( const QgsGeometryCheckContext &rh )
    {}
#endif
};

#endif // QGSGEOMETRYCHECKCONTEXT_H
