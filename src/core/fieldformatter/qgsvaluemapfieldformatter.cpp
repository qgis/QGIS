/***************************************************************************
  qgsvaluemapfieldformatter.cpp - QgsValueMapFieldFormatter

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
#include "qgsvaluemapfieldformatter.h"

#include "qgsvectorlayer.h"

const QString QgsValueMapFieldFormatter::NULL_VALUE = QStringLiteral( "{2839923C-8B7D-419E-B84B-CA2FE9B80EC7}" );

QgsValueMapFieldFormatter::QgsValueMapFieldFormatter()
{
  setFlags( flags() | QgsFieldFormatter::CanProvideAvailableValues );
}

QString QgsValueMapFieldFormatter::id() const
{
  return QStringLiteral( "ValueMap" );
}

QString QgsValueMapFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( cache )

  QString valueInternalText;
  if ( value.isNull() )
    valueInternalText = NULL_VALUE;
  else
    valueInternalText = value.toString();

  const QVariant v = config.value( QStringLiteral( "map" ) );
  const QVariantList list = v.toList();
  if ( !list.empty() )
  {
    for ( const QVariant &item : list )
    {
      const QVariantMap map = item.toMap();
      // no built-in Qt way to check if a map contains a value, so iterate through each value
      for ( auto it = map.constBegin(); it != map.constEnd(); ++it )
      {
        if ( it.value().toString() == valueInternalText )
          return it.key();
      }
    }
    return QStringLiteral( "(%1)" ).arg( layer->fields().at( fieldIndex ).displayString( value ) );
  }
  else
  {
    // old style config
    const QVariantMap map = v.toMap();
    return map.key( valueInternalText, QVariant( QStringLiteral( "(%1)" ).arg( layer->fields().at( fieldIndex ).displayString( value ) ) ).toString() );
  }
}

QVariant QgsValueMapFieldFormatter::sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  return representValue( layer, fieldIndex, config, cache, value );
}

QVariantList QgsValueMapFieldFormatter::availableValues( const QVariantMap &config, int countLimit, const QgsFieldFormatterContext &context ) const
{
  Q_UNUSED( context )

  QVariantList values;
  const QList<QVariant> valueList = config.value( QStringLiteral( "map" ) ).toList();
  for ( const QVariant &item : valueList )
  {
    values.append( item.toMap().constBegin().value() );
    if ( values.count() == countLimit )
      break;
  }

  return values;
}
