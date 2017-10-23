/***************************************************************************
                         qgsnativealgorithms.cpp
                         ---------------------
    begin                : April 2017
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

#include "qgsnativealgorithms.h"
#include "qgsalgorithmaddincrementalfield.h"
#include "qgsalgorithmassignprojection.h"
#include "qgsalgorithmboundary.h"
#include "qgsalgorithmboundingbox.h"
#include "qgsalgorithmbuffer.h"
#include "qgsalgorithmcentroid.h"
#include "qgsalgorithmclip.h"
#include "qgsalgorithmconvexhull.h"
#include "qgsalgorithmdissolve.h"
#include "qgsalgorithmdropgeometry.h"
#include "qgsalgorithmdropmzvalues.h"
#include "qgsalgorithmextenttolayer.h"
#include "qgsalgorithmextractbyattribute.h"
#include "qgsalgorithmextractbyexpression.h"
#include "qgsalgorithmextractbyextent.h"
#include "qgsalgorithmextractbylocation.h"
#include "qgsalgorithmfiledownloader.h"
#include "qgsalgorithmfixgeometries.h"
#include "qgsalgorithmjoinbyattribute.h"
#include "qgsalgorithmjoinwithlines.h"
#include "qgsalgorithmlineintersection.h"
#include "qgsalgorithmmeancoordinates.h"
#include "qgsalgorithmmergelines.h"
#include "qgsalgorithmminimumenclosingcircle.h"
#include "qgsalgorithmmultiparttosinglepart.h"
#include "qgsalgorithmorientedminimumboundingbox.h"
#include "qgsalgorithmrasterlayeruniquevalues.h"
#include "qgsalgorithmremovenullgeometry.h"
#include "qgsalgorithmpromotetomultipart.h"
#include "qgsalgorithmsaveselectedfeatures.h"
#include "qgsalgorithmsimplify.h"
#include "qgsalgorithmsmooth.h"
#include "qgsalgorithmsplitwithlines.h"
#include "qgsalgorithmsubdivide.h"
#include "qgsalgorithmtransform.h"

///@cond PRIVATE

QgsNativeAlgorithms::QgsNativeAlgorithms( QObject *parent )
  : QgsProcessingProvider( parent )
{}

QIcon QgsNativeAlgorithms::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/providerQgis.svg" ) );
}

QString QgsNativeAlgorithms::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "providerQgis.svg" ) );
}

QString QgsNativeAlgorithms::id() const
{
  return QStringLiteral( "native" );
}

QString QgsNativeAlgorithms::name() const
{
  return tr( "QGIS (native c++)" );
}

bool QgsNativeAlgorithms::supportsNonFileBasedOutput() const
{
  return true;
}

void QgsNativeAlgorithms::loadAlgorithms()
{
  addAlgorithm( new QgsBufferAlgorithm() );
  addAlgorithm( new QgsCentroidAlgorithm() );
  addAlgorithm( new QgsClipAlgorithm() );
  addAlgorithm( new QgsDissolveAlgorithm() );
  addAlgorithm( new QgsCollectAlgorithm() );
  addAlgorithm( new QgsExtractByAttributeAlgorithm() );
  addAlgorithm( new QgsExtractByExpressionAlgorithm() );
  addAlgorithm( new QgsFileDownloaderAlgorithm() );
  addAlgorithm( new QgsMultipartToSinglepartAlgorithm() );
  addAlgorithm( new QgsSubdivideAlgorithm() );
  addAlgorithm( new QgsTransformAlgorithm() );
  addAlgorithm( new QgsRemoveNullGeometryAlgorithm() );
  addAlgorithm( new QgsBoundingBoxAlgorithm() );
  addAlgorithm( new QgsOrientedMinimumBoundingBoxAlgorithm() );
  addAlgorithm( new QgsMinimumEnclosingCircleAlgorithm() );
  addAlgorithm( new QgsConvexHullAlgorithm() );
  addAlgorithm( new QgsPromoteToMultipartAlgorithm() );
  addAlgorithm( new QgsSelectByLocationAlgorithm() );
  addAlgorithm( new QgsExtractByLocationAlgorithm() );
  addAlgorithm( new QgsFixGeometriesAlgorithm() );
  addAlgorithm( new QgsMergeLinesAlgorithm() );
  addAlgorithm( new QgsSaveSelectedFeatures() );
  addAlgorithm( new QgsSmoothAlgorithm() );
  addAlgorithm( new QgsSimplifyAlgorithm() );
  addAlgorithm( new QgsExtractByExtentAlgorithm() );
  addAlgorithm( new QgsExtentToLayerAlgorithm() );
  addAlgorithm( new QgsLineIntersectionAlgorithm() );
  addAlgorithm( new QgsSplitWithLinesAlgorithm() );
  addAlgorithm( new QgsMeanCoordinatesAlgorithm() );
  addAlgorithm( new QgsRasterLayerUniqueValuesReportAlgorithm() );
  addAlgorithm( new QgsJoinByAttributeAlgorithm() );
  addAlgorithm( new QgsJoinWithLinesAlgorithm() );
  addAlgorithm( new QgsAssignProjectionAlgorithm() );
  addAlgorithm( new QgsAddIncrementalFieldAlgorithm() );
  addAlgorithm( new QgsBoundaryAlgorithm() );
  addAlgorithm( new QgsDropGeometryAlgorithm() );
  addAlgorithm( new QgsDropMZValuesAlgorithm() );
}


///@endcond



