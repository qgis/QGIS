/***************************************************************************
                         qgsprocessingparameterfieldmap.cpp
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

#include "qgsprocessingparameterfieldmap.h"

QgsProcessingParameterFieldMapping::QgsProcessingParameterFieldMapping( const QString &name, const QString &description, const QString &parentLayerParameterName, bool optional )
  : QgsProcessingParameterDefinition( name, description, QVariant(), optional )
  , mParentLayerParameterName( parentLayerParameterName )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterFieldMapping::clone() const
{
  return new QgsProcessingParameterFieldMapping( *this );
}

QString QgsProcessingParameterFieldMapping::type() const
{
  return typeName();
}

bool QgsProcessingParameterFieldMapping::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  if ( input.userType() != QMetaType::Type::QVariantList )
    return false;

  const QVariantList inputList = input.toList();
  for ( const QVariant &inputItem : inputList )
  {
    if ( inputItem.userType() != QMetaType::Type::QVariantMap )
      return false;

    const QVariantMap inputItemMap = inputItem.toMap();

    if ( !inputItemMap.contains( "name" ) )
      return false;
    if ( !inputItemMap.contains( "type" ) )
      return false;
    if ( !inputItemMap.contains( "expression" ) )
      return false;
  }

  return true;
}

QString QgsProcessingParameterFieldMapping::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  return QgsProcessingUtils::variantToPythonLiteral( value );
}

QString QgsProcessingParameterFieldMapping::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterFieldMapping('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( !mParentLayerParameterName.isEmpty() )
        code += u", parentLayerParameterName=%1"_s.arg( QgsProcessingUtils::stringToPythonLiteral( mParentLayerParameterName ) );

      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;
      code += ')';
      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterFieldMapping::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( u"parent_layer"_s, mParentLayerParameterName );
  return map;
}

bool QgsProcessingParameterFieldMapping::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( u"parent_layer"_s ).toString();
  return true;
}

QStringList QgsProcessingParameterFieldMapping::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentLayerParameterName.isEmpty() )
    depends << mParentLayerParameterName;
  return depends;
}

QString QgsProcessingParameterFieldMapping::parentLayerParameterName() const
{
  return mParentLayerParameterName;
}

void QgsProcessingParameterFieldMapping::setParentLayerParameterName( const QString &name )
{
  mParentLayerParameterName = name;
}

