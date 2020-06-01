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

#include "qgsvectorlayer.h"


QgsProcessingParameterFieldMapping::QgsProcessingParameterFieldMapping( const QString &name, const QString &description, const QString &parentLayerParameterName )
  : QgsProcessingParameterDefinition( name, description, QVariant(), false )
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

bool QgsProcessingParameterFieldMapping::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
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
    if ( !inputItemMap.contains( "expression" ) )
      return false;
  }

  return true;
}

QString QgsProcessingParameterFieldMapping::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  QStringList parts;

  // TODO

  return parts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterFieldMapping::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterVectorTileWriterLayers('%1', '%2')" ).arg( name(), description() );
      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterFieldMapping::toVariantMap() const
{

}

bool QgsProcessingParameterFieldMapping::fromVariantMap( const QVariantMap &map )
{

}

QString QgsProcessingParameterFieldMapping::parentLayerParameterName() const
{
  return mParentLayerParameterName;
}

void QgsProcessingParameterFieldMapping::setParentLayerParameterName( const QString &name )
{
  mParentLayerParameterName = name;
}

