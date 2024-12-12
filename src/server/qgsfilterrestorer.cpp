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

#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsaccesscontrol.h"
#endif

//! Apply filter from AccessControal
#ifdef HAVE_SERVER_PYTHON_PLUGINS
void QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( const QgsAccessControl *accessControl, QgsMapLayer *mapLayer, QHash<QgsMapLayer *, QString> &originalLayerFilters )
{
  if ( QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mapLayer ) )
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
        sql.prepend( ") AND (" );
        sql.append( ")" );
        sql.prepend( layer->subsetString() );
        sql.prepend( "(" );
      }
      if ( !layer->setSubsetString( sql ) )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Layer does not support Subset String" ) );
      }
    }
  }
}

void QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( const QgsAccessControl *accessControl, QgsMapLayer *mapLayer )
{
  if ( QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mapLayer ) )
  {
    QString sql = accessControl->extraSubsetString( layer );
    if ( !sql.isEmpty() )
    {
      if ( !layer->subsetString().isEmpty() )
      {
        sql.prepend( ") AND (" );
        sql.append( ")" );
        sql.prepend( layer->subsetString() );
        sql.prepend( "(" );
      }
      if ( !layer->setSubsetString( sql ) )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Layer does not support Subset String" ) );
      }
    }
  }
}
#endif

//! Restore layer filter as original
void QgsOWSServerFilterRestorer::restoreLayerFilters( const QHash<QgsMapLayer *, QString> &filterMap )
{
  QHash<QgsMapLayer *, QString>::const_iterator filterIt = filterMap.constBegin();
  for ( ; filterIt != filterMap.constEnd(); ++filterIt )
  {
    QgsVectorLayer *filteredLayer = qobject_cast<QgsVectorLayer *>( filterIt.key() );
    if ( filteredLayer )
    {
      if ( !filteredLayer->setSubsetString( filterIt.value() ) )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Layer does not support Subset String" ) );
      }
    }
  }
}
