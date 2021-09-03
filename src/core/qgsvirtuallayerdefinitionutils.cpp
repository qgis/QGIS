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
#include "qgsvectorlayerjoininfo.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include "qgsvirtuallayerdefinition.h"

QgsVirtualLayerDefinition QgsVirtualLayerDefinitionUtils::fromJoinedLayer( QgsVectorLayer *layer )
{
  QgsVirtualLayerDefinition def;

  QStringList leftJoins;
  QStringList columns;

  // add the geometry column if the layer is spatial
  if ( layer->isSpatial() )
    columns << "t.geometry";

  // look for the uid
  const QgsFields fields = layer->dataProvider()->fields();
  {
    QgsAttributeList pk = layer->dataProvider()->pkAttributeIndexes();
    if ( pk.size() == 1 )
    {
      def.setUid( fields.field( pk[0] ).name() );
    }
    else
    {
      // find an uid name
      QString uid = QStringLiteral( "uid" );
      while ( fields.lookupField( uid ) != -1 )
        uid += QLatin1Char( '_' ); // add "_" each time this name already exists

      // add a column
      columns << "t.rowid AS " + uid;
      def.setUid( uid );
    }
  }
  const QgsFields providerFields = layer->dataProvider()->fields();
  for ( const auto &f : providerFields )
  {
    columns << "t.\"" + f.name() + "\"";
  }

  int joinIdx = 0;
  const auto constVectorJoins = layer->vectorJoins();
  for ( const QgsVectorLayerJoinInfo &join : constVectorJoins )
  {
    const QString joinName = QStringLiteral( "j%1" ).arg( ++joinIdx );
    QgsVectorLayer *joinedLayer = join.joinLayer();
    if ( !joinedLayer )
      continue;
    const QString prefix = join.prefix().isEmpty() ? joinedLayer->name() + "_" : join.prefix();

    leftJoins << QStringLiteral( "LEFT JOIN \"%1\" AS %2 ON t.\"%5\"=%2.\"%3\"" ).arg( joinedLayer->id(), joinName, join.joinFieldName(), join.targetFieldName() );
    if ( auto *lJoinFieldNamesSubset = join.joinFieldNamesSubset() )
    {
      const QStringList joinFieldNamesSubset { *lJoinFieldNamesSubset };
      for ( const QString &f : joinFieldNamesSubset )
      {
        columns << joinName + ".\"" + f + "\" AS \"" + prefix + f + "\"";
      }
    }
    else
    {
      const QgsFields joinFields = joinedLayer->fields();
      for ( const QgsField &f : joinFields )
      {
        if ( f.name() == join.joinFieldName() )
          continue;
        columns << joinName + ".\"" + f.name() + "\" AS \"" + prefix + f.name() + "\"";
      }
    }
  }

  const QString query = "SELECT " + columns.join( QLatin1String( ", " ) ) + " FROM \"" + layer->id() + "\" AS t " + leftJoins.join( QLatin1Char( ' ' ) );
  def.setQuery( query );

  return def;
}
