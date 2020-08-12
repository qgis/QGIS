/***************************************************************************
 qgssymbollayerreference.cpp
 ---------------------
 begin                : July 2019
 copyright            : (C) 2019 by Hugo Mercier / Oslandia
 email                : infos at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbollayerreference.h"
#include "qgis.h"

QString symbolLayerReferenceListToString( const QgsSymbolLayerReferenceList &lst )
{
  QStringList slst;
  for ( const QgsSymbolLayerReference &ref : lst )
  {
    QStringList indexPathStr;
    for ( int index : ref.symbolLayerId().symbolLayerIndexPath() )
    {
      indexPathStr.append( QString::number( index ) );
    }
    slst.append( QStringLiteral( "%1,%2,%3" ).arg( ref.layerId(), ref.symbolLayerId().symbolKey(), indexPathStr.join( ',' ) ) );
  }
  return slst.join( ';' );
}

QgsSymbolLayerReferenceList stringToSymbolLayerReferenceList( const QString &str )
{
  QgsSymbolLayerReferenceList lst;
  QStringList slst;
  const QStringList split = str.split( ';' );
  for ( QString tuple : split )
  {
    // We should have "layer_id,symbol_key,symbol_layer_index0,symbol_layer_index1,..."
    QStringList elements = tuple.split( ',' );
    if ( elements.size() >= 3 )
    {
      QVector<int> indexPath;
      for ( int i = 2; i < elements.size(); i++ )
      {
        indexPath.append( elements[i].toInt() );
      }
      lst.append( QgsSymbolLayerReference( elements[0], QgsSymbolLayerId( elements[1], indexPath ) ) );
    }
  }
  return lst;
}
