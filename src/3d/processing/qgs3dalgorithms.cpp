/***************************************************************************
                         qgs3dalgorithms.cpp
                         ---------------------
    begin                : November 2017
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

#include "qgs3dalgorithms.h"
#include "moc_qgs3dalgorithms.cpp"
#include "qgsalgorithmtessellate.h"
#include "qgsapplication.h"

///@cond PRIVATE

Qgs3DAlgorithms::Qgs3DAlgorithms( QObject *parent )
  : QgsProcessingProvider( parent )
{}

QIcon Qgs3DAlgorithms::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/providerQgis.svg" ) );
}

QString Qgs3DAlgorithms::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "providerQgis.svg" ) );
}

QString Qgs3DAlgorithms::id() const
{
  return QStringLiteral( "3d" );
}

QString Qgs3DAlgorithms::helpId() const
{
  return QStringLiteral( "qgis" );
}

QString Qgs3DAlgorithms::name() const
{
  return tr( "QGIS (3D)" );
}

bool Qgs3DAlgorithms::supportsNonFileBasedOutput() const
{
  return true;
}

void Qgs3DAlgorithms::loadAlgorithms()
{
  addAlgorithm( new QgsTessellateAlgorithm() );
}


///@endcond
