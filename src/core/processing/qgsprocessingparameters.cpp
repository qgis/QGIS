/***************************************************************************
                         qgsprocessingparameters.cpp
                         ---------------------------
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

#include "qgsprocessingparameters.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingutils.h"

bool QgsProcessingParameters::isDynamic( const QVariantMap &parameters, const QString &name )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().propertyType() != QgsProperty::StaticProperty;
  else
    return false;
}

QString QgsProcessingParameters::parameterAsString( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsString( context.expressionContext() );
  else
    return val.toString();
}

double QgsProcessingParameters::parameterAsDouble( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsDouble( context.expressionContext(), -1 );
  else
    return val.toDouble();
}

int QgsProcessingParameters::parameterAsInt( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsInt( context.expressionContext(), -1 );
  else
    return val.toInt();
}

bool QgsProcessingParameters::parameterAsBool( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsBool( context.expressionContext(), -1 );
  else
    return val.toBool();
}

QgsMapLayer *QgsProcessingParameters::parameterAsLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  QString layerRef = parameterAsString( parameters, name, context );
  if ( layerRef.isEmpty() )
    return nullptr;

  return QgsProcessingUtils::mapLayerFromString( layerRef, context );
}
