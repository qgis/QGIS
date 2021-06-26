/***************************************************************************
                         qgsprocessingparametertype.cpp
                         ------------------------
    begin                : March 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparametertype.h"

QgsProcessingParameterType::ParameterFlags QgsProcessingParameterType::flags() const
{
  return QgsProcessingParameterType::ExposeToModeler;
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
