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
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"

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
  return QStringList() << QStringLiteral( "las" ) << QStringLiteral( "laz" );
}

void QgsPdalAlgorithms::loadAlgorithms()
{
  const QgsScopedRuntimeProfile profile( QObject::tr( "QGIS PDAL provider" ) );
}

///@endcond
