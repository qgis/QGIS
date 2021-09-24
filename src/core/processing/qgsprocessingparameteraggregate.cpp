/***************************************************************************
                         qgsprocessingparameteraggregate.cpp
                         -------------------------
    begin                : June 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparameteraggregate.h"

#include "qgsvectorlayer.h"


QgsProcessingParameterAggregate::QgsProcessingParameterAggregate( const QString &name, const QString &description, const QString &parentLayerParameterName, bool optional )
  : QgsProcessingParameterDefinition( name, description, QVariant(), optional )
  , mParentLayerParameterName( parentLayerParameterName )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterAggregate::clone() const
{
  return new QgsProcessingParameterAggregate( *this );
}

QString QgsProcessingParameterAggregate::type() const
{
  return typeName();
}

bool QgsProcessingParameterAggregate::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.type() != QVariant::List )
    return false;

  const QVariantList inputList = input.toList();
  for ( const QVariant &inputItem : inputList )
  {
    if ( inputItem.type() != QVariant::Map )
      return false;

    const QVariantMap inputItemMap = inputItem.toMap();

    if ( !inputItemMap.contains( "name" ) )
      return false;
    if ( !inputItemMap.contains( "type" ) )
      return false;
    if ( !inputItemMap.contains( "input" ) )
      return false;
    if ( !inputItemMap.contains( "aggregate" ) )
      return false;
  }

  return true;
}

QString QgsProcessingParameterAggregate::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  return QgsProcessingUtils::variantToPythonLiteral( value );
}

QString QgsProcessingParameterAggregate::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterAggregate('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( !mParentLayerParameterName.isEmpty() )
        code += QStringLiteral( ", parentLayerParameterName=%1" ).arg( QgsProcessingUtils::stringToPythonLiteral( mParentLayerParameterName ) );

      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );
      code += ')';
      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterAggregate::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "parent_layer" ), mParentLayerParameterName );
  return map;
}

bool QgsProcessingParameterAggregate::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( QStringLiteral( "parent_layer" ) ).toString();
  return true;
}

QStringList QgsProcessingParameterAggregate::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentLayerParameterName.isEmpty() )
    depends << mParentLayerParameterName;
  return depends;
}

QString QgsProcessingParameterAggregate::parentLayerParameterName() const
{
  return mParentLayerParameterName;
}

void QgsProcessingParameterAggregate::setParentLayerParameterName( const QString &name )
{
  mParentLayerParameterName = name;
}

