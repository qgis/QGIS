/***************************************************************************
                         qgsprocessingmodelparameter.cpp
                         ------------------------------
    begin                : June 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
  map.insert( QStringLiteral( "name" ), mParameterName );
  saveCommonProperties( map );
  return map;
}

bool QgsProcessingModelParameter::loadVariant( const QVariantMap &map )
{
  mParameterName = map.value( QStringLiteral( "name" ) ).toString();
  restoreCommonProperties( map );
  return true;
}

///@endcond
