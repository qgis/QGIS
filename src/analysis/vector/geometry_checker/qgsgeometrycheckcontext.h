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
    QgsGeometryCheckContext( int precision, const QgsCoordinateReferenceSystem &mapCrs, const QMap<QString, QgsFeaturePool *> &featurePools, const QgsCoordinateTransformContext &transformContext );
    const double tolerance;
    const double reducedTolerance;
    const QgsCoordinateReferenceSystem mapCrs;
    const QMap<QString, QgsFeaturePool *> featurePools;
    const QgsCoordinateTransformContext transformContext;
    const QgsCoordinateTransform &layerTransform( const QPointer<QgsVectorLayer> &layer ) SIP_SKIP;
    double layerScaleFactor( const QPointer<QgsVectorLayer> &layer ) SIP_SKIP;

  private:
#ifdef SIP_RUN
    QgsGeometryCheckContext( const QgsGeometryCheckContext &rh )
    {}
#endif
    QMap<QPointer<QgsVectorLayer>, QgsCoordinateTransform> mTransformCache;
    QMap<QPointer<QgsVectorLayer>, double> mScaleFactorCache;
    QReadWriteLock mCacheLock;
};

#endif // QGSGEOMETRYCHECKCONTEXT_H
