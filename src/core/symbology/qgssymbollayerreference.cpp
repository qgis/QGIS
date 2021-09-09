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
#include <QRegularExpression>

QString symbolLayerReferenceListToString( const QgsSymbolLayerReferenceList &lst )
{
  QStringList slst;
  slst.reserve( lst.size() );
  for ( const QgsSymbolLayerReference &ref : lst )
  {
    QStringList indexPathStr;
    const QVector<int> indexPath = ref.symbolLayerId().symbolLayerIndexPath();
    indexPathStr.reserve( indexPath.size() );
    for ( const int index : indexPath )
    {
      indexPathStr.append( QString::number( index ) );
    }
    // this is BAD BAD BAD -- it assumes that the component parts eg the symbolKey has no commas!
    // a more unique string should have been used as a concatenator here, but it's too late to fix that without breaking projects...
    slst.append( QStringLiteral( "%1,%2,%3" ).arg( ref.layerId(), ref.symbolLayerId().symbolKey(), indexPathStr.join( ',' ) ) );
  }
  return slst.join( ';' );
}

QgsSymbolLayerReferenceList stringToSymbolLayerReferenceList( const QString &str )
{
  QgsSymbolLayerReferenceList lst;
  if ( str.isEmpty() )
    return lst;

  // when saving we used ; as a concatenator... but that was silly, cos maybe the symbol keys contain this string!
  // try to handle this gracefully via regex...
  const thread_local QRegularExpression partsRx( QStringLiteral( "((?:.*?),(?:.*?),(?:(?:\\d+,)+)?(?:\\d+);)" ) );
  QRegularExpressionMatchIterator partsIt = partsRx.globalMatch( str + ';' );

  while ( partsIt.hasNext() )
  {
    const QRegularExpressionMatch partMatch = partsIt.next();
    const QString tuple = partMatch.captured( 1 );

    // We should have "layer_id,symbol_key,symbol_layer_index0,symbol_layer_index1,..."
    // EXCEPT that the symbol_key CAN have commas, so this whole logic is extremely broken.
    // Let's see if a messy regex can save the day!
    const thread_local QRegularExpression rx( QStringLiteral( "(.*?),(.*?),((?:\\d+,)+)?(\\d+)" ) );

    const QRegularExpressionMatch match = rx.match( tuple );
    if ( !match.hasMatch() )
      continue;

    const QString layerId = match.captured( 1 );
    const QString symbolKey = match.captured( 2 );
    const QStringList indices = QString( match.captured( 3 ) + match.captured( 4 ) ).split( ',' );

    QVector<int> indexPath;
    indexPath.reserve( indices.size() );
    for ( const QString &index : indices )
    {
      indexPath.append( index.toInt() );
    }
    lst.append( QgsSymbolLayerReference( layerId, QgsSymbolLayerId( symbolKey, indexPath ) ) );
  }
  return lst;
}
