/***************************************************************************
                             qgsprojectutils.h
                             -------------------
    begin                : July 2021
    copyright            : (C) 2021 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectutils.h"
#include "qgsmaplayerutils.h"
#include "qgsproject.h"
#include "qgsgrouplayer.h"

QList<QgsMapLayer *> QgsProjectUtils::layersMatchingPath( const QgsProject *project, const QString &path )
{
  QList<QgsMapLayer *> layersList;
  if ( !project )
    return layersList;

  const QMap<QString, QgsMapLayer *> mapLayers( project->mapLayers() );
  for ( QgsMapLayer *layer : mapLayers )
  {
    if ( QgsMapLayerUtils::layerSourceMatchesPath( layer, path ) )
    {
      layersList << layer;
    }
  }
  return layersList;
}

bool QgsProjectUtils::updateLayerPath( QgsProject *project, const QString &oldPath, const QString &newPath )
{
  if ( !project )
    return false;

  bool res = false;
  const QMap<QString, QgsMapLayer *> mapLayers( project->mapLayers() );
  for ( QgsMapLayer *layer : mapLayers )
  {
    if ( QgsMapLayerUtils::layerSourceMatchesPath( layer, oldPath ) )
    {
      res = QgsMapLayerUtils::updateLayerSourcePath( layer, newPath ) || res;
    }
  }
  return res;
}

bool QgsProjectUtils::layerIsContainedInGroupLayer( QgsProject *project, QgsMapLayer *layer )
{
  const QVector< QgsGroupLayer * > groupLayers = project->layers< QgsGroupLayer * >();
  for ( QgsGroupLayer *groupLayer : groupLayers )
  {
    if ( groupLayer->childLayers().contains( layer ) )
      return true;
  }
  return false;
}
