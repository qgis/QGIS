/***************************************************************************
  qgsvaluerelationfieldformatter.cpp - QgsValueRelationFieldFormatter

 ---------------------
 begin                : 3.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvaluerelationfieldformatter.h"

#include "qgis.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

#include <QSettings>

bool orderByKeyLessThan( const QgsValueRelationFieldFormatter::ValueRelationItem &p1, const QgsValueRelationFieldFormatter::ValueRelationItem &p2 )
{
  return qgsVariantLessThan( p1.key, p2.key );
}

bool orderByValueLessThan( const QgsValueRelationFieldFormatter::ValueRelationItem &p1, const QgsValueRelationFieldFormatter::ValueRelationItem &p2 )
{
  return qgsVariantLessThan( p1.value, p2.value );
}

QString QgsValueRelationFieldFormatter::id() const
{
  return QStringLiteral( "ValueRelation" );
}

QString QgsValueRelationFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )

  ValueRelationCache vrCache;

  if ( cache.isValid() )
  {
    vrCache = cache.value<QgsValueRelationFieldFormatter::ValueRelationCache>();
  }
  else
  {
    vrCache = QgsValueRelationFieldFormatter::createCache( config );
  }

  if ( config.value( QStringLiteral( "AllowMulti" ) ).toBool() )
  {
    QStringList keyList = value.toString().remove( QChar( '{' ) ).remove( QChar( '}' ) ).split( ',' );
    QStringList valueList;

    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &item : qgis::as_const( vrCache ) )
    {
      if ( keyList.contains( item.key.toString() ) )
      {
        valueList << item.value;
      }
    }

    return valueList.join( QStringLiteral( ", " ) ).prepend( '{' ).append( '}' );
  }
  else
  {
    if ( value.isNull() )
    {
      return QgsApplication::nullRepresentation();
    }

    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &item : qgis::as_const( vrCache ) )
    {
      if ( item.key == value )
      {
        return item.value;
      }
    }
  }

  return QStringLiteral( "(%1)" ).arg( value.toString() );
}

QVariant QgsValueRelationFieldFormatter::sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  return representValue( layer, fieldIndex, config, cache, value );
}

QVariant QgsValueRelationFieldFormatter::createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )
  return QVariant::fromValue<ValueRelationCache>( createCache( config ) );

}

QgsValueRelationFieldFormatter::ValueRelationCache QgsValueRelationFieldFormatter::createCache( const QVariantMap &config )
{
  ValueRelationCache cache;

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( config.value( QStringLiteral( "Layer" ) ).toString() ) );

  if ( !layer )
    return cache;

  QgsFields fields = layer->fields();
  int ki = fields.indexOf( config.value( QStringLiteral( "Key" ) ).toString() );
  int vi = fields.indexOf( config.value( QStringLiteral( "Value" ) ).toString() );

  QgsFeatureRequest request;

  request.setFlags( QgsFeatureRequest::NoGeometry );
  request.setSubsetOfAttributes( QgsAttributeList() << ki << vi );
  if ( !config.value( QStringLiteral( "FilterExpression" ) ).toString().isEmpty() )
  {
    request.setFilterExpression( config.value( QStringLiteral( "FilterExpression" ) ).toString() );
  }

  QgsFeatureIterator fit = layer->getFeatures( request );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    cache.append( ValueRelationItem( f.attribute( ki ), f.attribute( vi ).toString() ) );
  }

  if ( config.value( QStringLiteral( "OrderByValue" ) ).toBool() )
  {
    std::sort( cache.begin(), cache.end(), orderByValueLessThan );
  }
  else
  {
    std::sort( cache.begin(), cache.end(), orderByKeyLessThan );
  }

  return cache;
}
