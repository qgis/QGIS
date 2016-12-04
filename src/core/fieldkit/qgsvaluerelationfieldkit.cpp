/***************************************************************************
  qgsvaluerelationfieldkit.cpp - QgsValueRelationFieldKit

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
#include "qgsvaluerelationfieldkit.h"

#include "qgis.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

#include <QSettings>

bool orderByKeyLessThan( const QgsValueRelationFieldKit::ValueRelationItem& p1, const QgsValueRelationFieldKit::ValueRelationItem& p2 )
{
  return qgsVariantLessThan( p1.key, p2.key );
}

bool orderByValueLessThan( const QgsValueRelationFieldKit::ValueRelationItem& p1, const QgsValueRelationFieldKit::ValueRelationItem& p2 )
{
  return qgsVariantLessThan( p1.value, p2.value );
}


QgsValueRelationFieldKit::QgsValueRelationFieldKit()
{

}

QString QgsValueRelationFieldKit::id() const
{
  return QStringLiteral( "ValueRelation" );
}

QString QgsValueRelationFieldKit::representValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )

  QHash<QString, QString> vrCache;

  if ( cache.isValid() )
  {
    vrCache = cache.value<QHash<QString, QString>>();
  }
  else
  {
    vrCache = createCache( layer, fieldIndex, config ).value<QHash<QString, QString>>();
  }

  if ( config.value( QStringLiteral( "AllowMulti" ) ).toBool() )
  {
    QStringList keyList = value.toString().remove( QChar( '{' ) ).remove( QChar( '}' ) ).split( ',' );
    QStringList valueList;

    Q_FOREACH ( const QString& key, keyList )
    {
      auto val = vrCache.constFind( key );
      if ( val != vrCache.constEnd() )
        valueList << val.value();
      else
        valueList << QStringLiteral( "(%1)" ).arg( key );
    }

    return valueList.join( QStringLiteral( ", " ) ).prepend( '{' ).append( '}' );
  }
  else
  {
    if ( value.isNull() )
    {
      QSettings settings;
      return settings.value( QStringLiteral( "qgis/nullValue" ), "NULL" ).toString();
    }

    auto val = vrCache.constFind( value.toString() );
    if ( val != vrCache.constEnd() )
      return val.value();
  }

  return QStringLiteral( "(%1)" ).arg( value.toString() );
}

QVariant QgsValueRelationFieldKit::sortValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  return representValue( layer, fieldIndex, config, cache, value );
}

QVariant QgsValueRelationFieldKit::createCache( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )
  return QVariant::fromValue<ValueRelationCache>( createCache( config ) );

}

QgsValueRelationFieldKit::ValueRelationCache QgsValueRelationFieldKit::createCache( const QVariantMap& config )
{
  ValueRelationCache cache;

  QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( config.value( "Layer" ).toString() ) );

  if ( !layer )
    return cache;

  QgsFields fields = layer->fields();
  int ki = fields.indexOf( config.value( "Key" ).toString() );
  int vi = fields.indexOf( config.value( "Value" ).toString() );

  QgsFeatureRequest request;

  request.setFlags( QgsFeatureRequest::NoGeometry );
  request.setSubsetOfAttributes( QgsAttributeList() << ki << vi );
  if ( !config.value( "FilterExpression" ).toString().isEmpty() )
  {
    request.setFilterExpression( config.value( "FilterExpression" ).toString() );
  }

  QgsFeatureIterator fit = layer->getFeatures( request );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    cache.append( ValueRelationItem( f.attribute( ki ), f.attribute( vi ).toString() ) );
  }

  if ( config.value( "OrderByValue" ).toBool() )
  {
    qSort( cache.begin(), cache.end(), orderByValueLessThan );
  }
  else
  {
    qSort( cache.begin(), cache.end(), orderByKeyLessThan );
  }

  return cache;
}
