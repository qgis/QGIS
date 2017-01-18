/***************************************************************************
                              qgsowsserver.cpp
                              -------------------
  begin                : February 27, 2012
  copyright            : (C) 2012 by René-Luc D'Hont & Marco Hugentobler
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfilterrestorer.h"
#include "qgsmessagelog.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

//! Apply filter from AccessControl
void QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( const QgsAccessControl* accessControl, QgsMapLayer* mapLayer,
    QHash<QgsMapLayer*, QString>& originalLayerFilters )
{
  if ( QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( mapLayer ) )
  {
    QString sql = accessControl->extraSubsetString( layer );
    if ( !sql.isEmpty() )
    {
      if ( !originalLayerFilters.contains( layer ) )
      {
        originalLayerFilters.insert( layer, layer->subsetString() );
      }
      if ( !layer->subsetString().isEmpty() )
      {
        sql.prepend( " AND " );
        sql.prepend( layer->subsetString() );
      }
      if ( !layer->setSubsetString( sql ) )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Layer does not support Subset String" ) );
      }
    }
  }
}

//! Restore layer filter as original
void QgsOWSServerFilterRestorer::restoreLayerFilters( const QHash<QgsMapLayer*, QString>& filterMap )
{
  QHash<QgsMapLayer*, QString>::const_iterator filterIt = filterMap.constBegin();
  for ( ; filterIt != filterMap.constEnd(); ++filterIt )
  {
    QgsVectorLayer* filteredLayer = qobject_cast<QgsVectorLayer*>( filterIt.key() );
    if ( filteredLayer )
    {
      QgsVectorDataProvider* dp = filteredLayer->dataProvider();
      if ( dp )
      {
        dp->setSubsetString( filterIt.value() );
      }
    }
  }
}
