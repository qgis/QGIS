/***************************************************************************
    qgsenumerationfieldformatter.cpp
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsenumerationfieldformatter.h"

#include "qgsapplication.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsEnumerationFieldFormatter::QgsEnumerationFieldFormatter()
{}

QString QgsEnumerationFieldFormatter::id() const
{
  return u"Enumeration"_s;
}

QString QgsEnumerationFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )
  Q_UNUSED( config )

  if ( QgsVariantUtils::isNull( value ) )
  {
    return QgsApplication::nullRepresentation();
  }

  const QMap<QString, QString> map( cache.value<QMap<QString, QString>>() );

  return map.value( value.toString() );
}

QVariant QgsEnumerationFieldFormatter::sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  return representValue( layer, fieldIndex, config, cache, value );
}

QVariant QgsEnumerationFieldFormatter::createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const
{
  Q_UNUSED( config )

  const QList<QPair<QString, QString>> values( layer->dataProvider()->codedValues( fieldIndex ) );
  QMap<QString, QString> map;
  for ( const QPair<QString, QString> &pair : values )
  {
    map[pair.first] = pair.second;
  }

  return QVariant::fromValue( map );
}
