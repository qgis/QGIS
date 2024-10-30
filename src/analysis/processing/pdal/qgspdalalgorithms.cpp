/***************************************************************************
                         qgspdalalgorithms.cpp
                         ---------------------
    begin                : February 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspdalalgorithms.h"
#include "moc_qgspdalalgorithms.cpp"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"

#include "qgsalgorithmpdalassignprojection.h"
#include "qgsalgorithmpdalboundary.h"
#include "qgsalgorithmpdalbuildvpc.h"
#include "qgsalgorithmpdalclip.h"
#include "qgsalgorithmpdalconvertformat.h"
#include "qgsalgorithmpdalcreatecopc.h"
#include "qgsalgorithmpdaldensity.h"
#include "qgsalgorithmpdalexportraster.h"
#include "qgsalgorithmpdalexportrastertin.h"
#include "qgsalgorithmpdalexportvector.h"
#include "qgsalgorithmpdalfilter.h"
#include "qgsalgorithmpdalinformation.h"
#include "qgsalgorithmpdalmerge.h"
#include "qgsalgorithmpdalreproject.h"
#include "qgsalgorithmpdalthinbydecimate.h"
#include "qgsalgorithmpdalthinbyradius.h"
#include "qgsalgorithmpdaltile.h"

///@cond PRIVATE

QgsPdalAlgorithms::QgsPdalAlgorithms( QObject *parent )
  : QgsProcessingProvider( parent )
{}

QIcon QgsPdalAlgorithms::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/providerQgis.svg" ) );
}

QString QgsPdalAlgorithms::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "providerQgis.svg" ) );
}

QString QgsPdalAlgorithms::id() const
{
  return QStringLiteral( "pdal" );
}

QString QgsPdalAlgorithms::helpId() const
{
  return QStringLiteral( "qgis" );
}

QString QgsPdalAlgorithms::name() const
{
  return tr( "QGIS (PDAL)" );
}

bool QgsPdalAlgorithms::supportsNonFileBasedOutput() const
{
  return false;
}

QStringList QgsPdalAlgorithms::supportedOutputVectorLayerExtensions() const
{
  return QStringList() << QStringLiteral( "gpkg" );
}

QStringList QgsPdalAlgorithms::supportedOutputRasterLayerExtensions() const
{
  return QStringList() << QStringLiteral( "tif" );
}

QStringList QgsPdalAlgorithms::supportedOutputPointCloudLayerExtensions() const
{
  return QStringList() << QStringLiteral( "las" ) << QStringLiteral( "laz" ) << QStringLiteral( "copc.laz" ) << QStringLiteral( "vpc" );
}

void QgsPdalAlgorithms::loadAlgorithms()
{
  const QgsScopedRuntimeProfile profile( QObject::tr( "QGIS PDAL provider" ) );

  addAlgorithm( new QgsPdalAssignProjectionAlgorithm() );
  addAlgorithm( new QgsPdalBoundaryAlgorithm() );
  addAlgorithm( new QgsPdalBuildVpcAlgorithm() );
  addAlgorithm( new QgsPdalClipAlgorithm() );
  addAlgorithm( new QgsPdalConvertFormatAlgorithm() );
  addAlgorithm( new QgsPdalCreateCopcAlgorithm() );
  addAlgorithm( new QgsPdalDensityAlgorithm() );
  addAlgorithm( new QgsPdalExportRasterAlgorithm() );
  addAlgorithm( new QgsPdalExportRasterTinAlgorithm() );
  addAlgorithm( new QgsPdalExportVectorAlgorithm() );
  addAlgorithm( new QgsPdalFilterAlgorithm() );
  addAlgorithm( new QgsPdalInformationAlgorithm() );
  addAlgorithm( new QgsPdalMergeAlgorithm() );
  addAlgorithm( new QgsPdalReprojectAlgorithm() );
  addAlgorithm( new QgsPdalThinByDecimateAlgorithm() );
  addAlgorithm( new QgsPdalThinByRadiusAlgorithm() );
  addAlgorithm( new QgsPdalTileAlgorithm() );
}

///@endcond
