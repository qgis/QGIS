/***************************************************************************
                         qgsprocessingparameterfieldform.cpp
                         ----------------------
    begin                : September 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparameterfieldform.h"


QgsProcessingParameterFieldForm::QgsProcessingParameterFieldForm( const QString &name, const QString &description, const QString &parentLayerParameterName, bool optional )
  : QgsProcessingParameterDefinition( name, description, QVariant(), optional )
  , mParentLayerParameterName( parentLayerParameterName )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterFieldForm::clone() const
{
  return new QgsProcessingParameterFieldForm( *this );
}

QString QgsProcessingParameterFieldForm::type() const
{
  return typeName();
}


bool QgsProcessingParameterFieldForm::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.type() != QVariant::Map )
    return false;

  const QVariantMap inputMap = input.toMap();

  if ( inputMap.value( QStringLiteral( "name" ) ).toString().size() == 0 )
    return false;
  if ( inputMap.value( QStringLiteral( "type" ) ).toString().size() == 0 )
    return false;
  if ( inputMap.value( QStringLiteral( "length" ) ).toInt() < 0 )
    return false;
  if ( inputMap.value( QStringLiteral( "precision" ) ).toInt() < 0 )
    return false;

  return true;
}

QString QgsProcessingParameterFieldForm::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  return QgsProcessingUtils::variantToPythonLiteral( value );
}

// TODO finish this
QString QgsProcessingParameterFieldForm::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterFieldForm('%1', '%2'" ).arg( name(), description() );
      if ( !mParentLayerParameterName.isEmpty() )
        code += QStringLiteral( ", parentLayerParameterName=%1" ).arg( QgsProcessingUtils::stringToPythonLiteral( mParentLayerParameterName ) );

      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );
      code += ')';
      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterFieldForm::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "parent_layer" ), mParentLayerParameterName );
  return map;
}

bool QgsProcessingParameterFieldForm::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( QStringLiteral( "parent_layer" ) ).toString();
  return true;
}

QStringList QgsProcessingParameterFieldForm::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentLayerParameterName.isEmpty() )
    depends << mParentLayerParameterName;
  return depends;
}

QString QgsProcessingParameterFieldForm::parentLayerParameterName() const
{
  return mParentLayerParameterName;
}

void QgsProcessingParameterFieldForm::setParentLayerParameterName( const QString &name )
{
  mParentLayerParameterName = name;
}
