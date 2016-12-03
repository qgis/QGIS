/***************************************************************************
  qgskeyvaluefieldkit.cpp - QgsKeyValueFieldKit

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
#include "qgskeyvaluefieldkit.h"

#include <QSettings>

QgsKeyValueFieldKit::QgsKeyValueFieldKit()
{

}

QString QgsKeyValueFieldKit::id() const
{
  return QStringLiteral( "KeyValue" );
}

QString QgsKeyValueFieldKit::representValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( vl );
  Q_UNUSED( fieldIdx );
  Q_UNUSED( config );
  Q_UNUSED( cache );

  if ( value.isNull() )
  {
    QSettings settings;
    return settings.value( QStringLiteral( "qgis/nullValue" ), "NULL" ).toString();
  }

  QString result;
  const QVariantMap map = value.toMap();
  for ( QVariantMap::const_iterator i = map.constBegin(); i != map.constEnd(); ++i )
  {
    if ( !result.isEmpty() ) result.append( ", " );
    result.append( i.key() ).append( ": " ).append( i.value().toString() );
  }
  return result;
}
