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
#include "qgslayertree.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"


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
  // two situations we need to catch here -- one is a group layer which isn't present in the layer
  // tree, the other is a group layer associated with the project's layer tree which contains
  // UNCHECKED child layers
  const QVector< QgsGroupLayer * > groupLayers = project->layers< QgsGroupLayer * >();
  for ( QgsGroupLayer *groupLayer : groupLayers )
  {
    if ( groupLayer->childLayers().contains( layer ) )
      return true;
  }

  std::function< bool( QgsLayerTreeGroup *group ) > traverseTree;
  traverseTree = [ &traverseTree, layer ]( QgsLayerTreeGroup * group ) -> bool
  {
    // is the group a layer group containing our target layer?
    if ( group->groupLayer() && group->findLayer( layer ) )
    {
      return true;
    }

    const QList< QgsLayerTreeNode * > children = group->children();
    for ( QgsLayerTreeNode *node : children )
    {
      if ( QgsLayerTreeGroup *childGroup = qobject_cast< QgsLayerTreeGroup * >( node ) )
      {
        if ( traverseTree( childGroup ) )
          return true;
      }
    }
    return false;
  };
  return traverseTree( project->layerTreeRoot() );
}

bool QgsProjectUtils::checkUserTrust( QgsProject *project, bool *undetermined )
{
  const Qgis::PythonEmbeddedMode pythonEmbeddedMode = QgsSettingsRegistryCore::settingsCodeExecutionBehaviorUndeterminedProjects->value();
  if ( pythonEmbeddedMode == Qgis::PythonEmbeddedMode::Always )
  {
    // A user having changed the behavior to always allow is considered as determined
    if ( undetermined )
    {
      *undetermined = false;
    }
    return true;
  }
  else if ( pythonEmbeddedMode == Qgis::PythonEmbeddedMode::Never )
  {
    // A user having changed the behavior to always deny is considered as determined
    if ( undetermined )
    {
      *undetermined = false;
    }
    return false;
  }

  QFileInfo fileInfo( project->absoluteFilePath() );
  const QString absoluteFilePath = fileInfo.absoluteFilePath();
  const QString absolutePath = fileInfo.absolutePath();

  const QStringList deniedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionDeniedProjectsFolders->value() + QgsSettingsRegistryCore::settingsCodeExecutionTemporarilyDeniedProjectsFolders->value();
  for ( const QString &path : deniedProjectsFolders )
  {
    if ( absoluteFilePath == path || absolutePath == path )
    {
      if ( undetermined )
      {
        *undetermined = false;
      }
      return false;
    }
  }

  const QStringList trustedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionTrustedProjectsFolders->value() + QgsSettingsRegistryCore::settingsCodeExecutionTemporarilyTrustedProjectsFolders->value();
  for ( const QString &path : trustedProjectsFolders )
  {
    if ( absoluteFilePath == path || absolutePath == path )
    {
      if ( undetermined )
      {
        *undetermined = false;
      }
      return true;
    }
  }

  if ( undetermined )
  {
    *undetermined = true;
  }
  return false;
}
