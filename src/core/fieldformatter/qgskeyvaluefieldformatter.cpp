/***************************************************************************
  qgskeyvaluefieldformatter.cpp - QgsKeyValueFieldFormatter

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
#include "qgskeyvaluefieldformatter.h"
#include "qgsapplication.h"
#include "qgsvariantutils.h"

#include <QSettings>

QString QgsKeyValueFieldFormatter::id() const
{
  return QStringLiteral( "KeyValue" );
}

QString QgsKeyValueFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )
  Q_UNUSED( config )
  Q_UNUSED( cache )

  if ( QgsVariantUtils::isNull( value ) )
  {
    return QgsApplication::nullRepresentation();
  }

  QString result;
  const QVariantMap map = value.toMap();
  for ( QVariantMap::const_iterator i = map.constBegin(); i != map.constEnd(); ++i )
  {
    if ( !result.isEmpty() )
      result.append( ", " );
    result.append( i.key() ).append( ": " ).append( i.value().toString() );
  }
  return result;
}
