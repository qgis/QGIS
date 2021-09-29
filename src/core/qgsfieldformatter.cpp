/***************************************************************************
  qgsfieldformatter.cpp - QgsFieldFormatter

 ---------------------
 begin                : 2.12.2016
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
#include "qgsfieldformatter.h"

#include "qgsfield.h"
#include "qgsfields.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsapplication.h"

QString QgsFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( config )
  Q_UNUSED( cache )

  if ( ! layer->fields().exists( fieldIndex ) )
    return QString();

  QString defVal;
  if ( layer->fields().fieldOrigin( fieldIndex ) == QgsFields::OriginProvider && layer->dataProvider() )
    defVal = layer->dataProvider()->defaultValueClause( layer->fields().fieldOriginIndex( fieldIndex ) );

  if ( !defVal.isNull() && defVal == value )
  {
    return defVal;
  }
  else if ( value.isNull() )
  {
    return QgsApplication::nullRepresentation();
  }
  else
  {
    return layer->fields().at( fieldIndex ).displayString( value );
  }
}

QVariant QgsFieldFormatter::sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )
  Q_UNUSED( config )
  Q_UNUSED( cache )

  return value;
}

Qt::AlignmentFlag QgsFieldFormatter::alignmentFlag( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const
{
  Q_UNUSED( config )

  const QgsField field = layer->fields().at( fieldIndex );
  if ( field.isNumeric() || field.isDateOrTime() )
    return Qt::AlignRight;
  else
    return Qt::AlignLeft;
}

QVariant QgsFieldFormatter::createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )
  Q_UNUSED( config )

  return QVariant();
}

QList<QgsVectorLayerRef> QgsFieldFormatter::layerDependencies( const QVariantMap &config ) const
{
  Q_UNUSED( config )
  return QList<QgsVectorLayerRef>();
}

QVariantList QgsFieldFormatter::availableValues( const QVariantMap &config, int countLimit, const QgsFieldFormatterContext &context ) const
{
  Q_UNUSED( config )
  Q_UNUSED( countLimit )
  Q_UNUSED( context )

  return QVariantList();
}

void QgsFieldFormatter::setFlags( const Flags &flags )
{
  mFlags = flags;
}
