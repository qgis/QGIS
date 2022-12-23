/***************************************************************************
                         qgsprocessingmodeloutput.cpp
                         ----------------------------
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

#include "qgsprocessingmodeloutput.h"

///@cond NOT_STABLE

QgsProcessingModelOutput::QgsProcessingModelOutput( const QString &name, const QString &description )
  : QgsProcessingModelComponent( description )
  , mName( name )
{}

QgsProcessingModelOutput *QgsProcessingModelOutput::clone() const
{
  return new QgsProcessingModelOutput( *this );
}

QVariant QgsProcessingModelOutput::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "name" ), mName );

  if ( mDefaultValue.userType() == QMetaType::type( "QgsProcessingOutputLayerDefinition" ) )
  {
    QVariantMap defaultMap = mDefaultValue.value<QgsProcessingOutputLayerDefinition>().toVariant().toMap();
    defaultMap.insert( QStringLiteral( "class" ), QStringLiteral( "QgsProcessingOutputLayerDefinition" ) );
    map.insert( QStringLiteral( "default_value" ), defaultMap );
  }
  else
  {
    map.insert( QStringLiteral( "default_value" ), mDefaultValue );
  }

  map.insert( QStringLiteral( "child_id" ), mChildId );
  map.insert( QStringLiteral( "output_name" ), mOutputName );
  map.insert( QStringLiteral( "mandatory" ), mMandatory );
  saveCommonProperties( map );
  return map;
}

bool QgsProcessingModelOutput::loadVariant( const QVariantMap &map )
{
  mName = map.value( QStringLiteral( "name" ) ).toString();

  const QVariant defaultValue = map.value( QStringLiteral( "default_value" ) );
  if ( defaultValue.type() == QVariant::Map )
  {
    QVariantMap defaultMap = defaultValue.toMap();
    if ( defaultMap["class"] == QLatin1String( "QgsProcessingOutputLayerDefinition" ) )
    {
      QgsProcessingOutputLayerDefinition value;
      value.loadVariant( defaultMap );
      mDefaultValue = QVariant( value );
    }
    else
    {
      mDefaultValue = QVariant();
    }
  }
  else
  {
    mDefaultValue = map.value( QStringLiteral( "default_value" ) );
  }

  mChildId = map.value( QStringLiteral( "child_id" ) ).toString();
  mOutputName = map.value( QStringLiteral( "output_name" ) ).toString();
  mMandatory = map.value( QStringLiteral( "mandatory" ), false ).toBool();
  restoreCommonProperties( map );
  return true;
}


///@endcond
