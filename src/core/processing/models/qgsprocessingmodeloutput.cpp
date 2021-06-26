/***************************************************************************
                         qgsprocessingmodeloutput.cpp
                         ----------------------------
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

  if ( mDefaultValue.canConvert<QgsProcessingOutputLayerDefinition>() )
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

  QVariant defaultValue = map.value( QStringLiteral( "default_value" ) );
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
