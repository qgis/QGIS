/***************************************************************************
                         qgsprocessingmodelparameter.cpp
                         ------------------------------
    begin                : June 2017
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

#include "qgsprocessingmodelparameter.h"

///@cond NOT_STABLE

QgsProcessingModelParameter::QgsProcessingModelParameter( const QString &parameterName )
  : mParameterName( parameterName )
{

}

QgsProcessingModelParameter *QgsProcessingModelParameter::clone() const
{
  return new QgsProcessingModelParameter( *this );
}

QVariant QgsProcessingModelParameter::toVariant() const
{
  QVariantMap map;
  map.insert( u"name"_s, mParameterName );
  saveCommonProperties( map );
  return map;
}

bool QgsProcessingModelParameter::loadVariant( const QVariantMap &map )
{
  mParameterName = map.value( u"name"_s ).toString();
  restoreCommonProperties( map );
  return true;
}

///@endcond
