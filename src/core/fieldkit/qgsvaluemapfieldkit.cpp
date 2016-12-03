/***************************************************************************
  qgsvaluemapfieldkit.cpp - QgsValueMapFieldKit

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
#include "qgsvaluemapfieldkit.h"

#include "qgsvectorlayer.h"

QgsValueMapFieldKit::QgsValueMapFieldKit()
{

}

QString QgsValueMapFieldKit::id() const
{
  return QStringLiteral( "ValueMap" );
}

QString QgsValueMapFieldKit::representValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( cache )

  QString valueInternalText;
  if ( value.isNull() )
    valueInternalText = VALUEMAP_NULL_TEXT;
  else
    valueInternalText = value.toString();

  return config.key( valueInternalText, QVariant( QStringLiteral( "(%1)" ).arg( vl->fields().at( fieldIdx ).displayString( value ) ) ).toString() );
}

QVariant QgsValueMapFieldKit::sortValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  return representValue( vl, fieldIdx, config, cache, value );
}
