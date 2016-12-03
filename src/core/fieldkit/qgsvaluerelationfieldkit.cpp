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

#include <QSettings>

QgsValueRelationFieldKit::QgsValueRelationFieldKit()
{

}

QString QgsValueRelationFieldKit::representValue( QgsVectorLayer* layer, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QHash<QString, QString> vrCache;

  if ( cache.isValid() )
  {
    vrCache = cache.value<QHash<QString, QString>>();
  }
  else
  {
    vrCache = createCache( layer, fieldIdx, config ).value<QHash<QString, QString>>();
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

QVariant QgsValueRelationFieldKit::sortValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  return representValue( vl, fieldIdx, config, cache, value );
}
