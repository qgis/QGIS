/***************************************************************************
                         qgsprocessingparametertype.cpp
                         ------------------------
    begin                : March 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparametertype.h"

Qgis::ProcessingParameterTypeFlags QgsProcessingParameterType::flags() const
{
  return Qgis::ProcessingParameterTypeFlag::ExposeToModeler;
}

QVariantMap QgsProcessingParameterType::metadata() const
{
  return QVariantMap();
}

QStringList QgsProcessingParameterType::acceptedPythonTypes() const
{
  return QStringList();
}

QStringList QgsProcessingParameterType::acceptedStringValues() const
{
  return QStringList();
}

QStringList QgsProcessingParameterType::acceptedParameterTypes() const
{
  return QStringList();
}

QStringList QgsProcessingParameterType::acceptedOutputTypes() const
{
  return QStringList();
}

QList<int> QgsProcessingParameterType::acceptedDataTypes( const QgsProcessingParameterDefinition * ) const
{
  return QList<int>();
}
