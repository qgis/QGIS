/***************************************************************************
                qgsvirtuallayerdefinitionutils.cpp
begin                : Jan 2016
copyright            : (C) 2016 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvirtuallayerdefinitionutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmaplayerregistry.h"

QgsVirtualLayerDefinition QgsVirtualLayerDefinitionUtils::fromJoinedLayer( QgsVectorLayer* layer )
{
  QgsVirtualLayerDefinition def;

  QStringList leftJoins;
  QStringList columns;

  // look for the uid
  const QgsFields& fields = layer->dataProvider()->fields();
  {
    QgsAttributeList pk = layer->dataProvider()->pkAttributeIndexes();
    if ( pk.size() == 1 )
    {
      def.setUid( fields.field( pk[0] ).name() );
    }
    else
    {
      // find an uid name
      QString uid = "uid";
      while ( fields.fieldNameIndex( uid ) != -1 )
        uid += "_"; // add "_" each time this name already exists

      // add a column
      columns << "t.rowid AS " + uid;
      def.setUid( uid );
    }
  }
  Q_FOREACH ( const QgsField& f, layer->dataProvider()->fields() )
  {
    columns << "t." + f.name();
  }

  int joinIdx = 0;
  Q_FOREACH ( const QgsVectorJoinInfo& join, layer->vectorJoins() )
  {
    QString joinName = QString( "j%1" ).arg( ++joinIdx );
    QgsVectorLayer* joinedLayer = static_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( join.joinLayerId ) );
    if ( !joinedLayer )
      continue;
    QString prefix = join.prefix.isEmpty() ? joinedLayer->name() + "_" : join.prefix;

    leftJoins << QString( "LEFT JOIN %1 AS %2 ON t.\"%5\"=%2.\"%3\"" ).arg( join.joinLayerId, joinName, join.joinFieldName, join.targetFieldName );
    if ( join.joinFieldNamesSubset() )
    {
      Q_FOREACH ( const QString& f, *join.joinFieldNamesSubset() )
      {
        columns << joinName + "." + f + " AS " + prefix + f;
      }
    }
    else
    {
      Q_FOREACH ( const QgsField& f, joinedLayer->fields() )
      {
        if ( f.name() == join.joinFieldName )
          continue;
        columns << joinName + "." + f.name() + " AS " + prefix + f.name();
      }
    }
  }

  QString query = "SELECT " + columns.join( ", " ) + " FROM " + layer->id() + " AS t " + leftJoins.join( " " );
  def.setQuery( query );

  return def;
}
