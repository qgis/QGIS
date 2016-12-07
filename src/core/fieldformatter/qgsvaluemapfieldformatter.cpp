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

const QString QgsValueMapFieldFormatter::NullValue = QStringLiteral( "{2839923C-8B7D-419E-B84B-CA2FE9B80EC7}" );

QString QgsValueMapFieldFormatter::id() const
{
  return QStringLiteral( "ValueMap" );
}

QString QgsValueMapFieldFormatter::representValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( cache )

  QString valueInternalText;
  if ( value.isNull() )
    valueInternalText = NullValue;
  else
    valueInternalText = value.toString();

  QVariantMap map = config.value( QStringLiteral( "map" ) ).toMap();

  return map.key( valueInternalText, QVariant( QStringLiteral( "(%1)" ).arg( layer->fields().at( fieldIndex ).displayString( value ) ) ).toString() );
}

QVariant QgsValueMapFieldFormatter::sortValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  return representValue( layer, fieldIndex, config, cache, value );
}
