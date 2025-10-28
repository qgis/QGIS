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
#include "qgsaction.h"
#include "qgsactionmanager.h"
#include "qgsapplication.h"
#include "qgsgrouplayer.h"
#include "qgslayertree.h"
#include "qgsmaplayerutils.h"
#include "qgsnetworkcontentfetcherregistry.h"
#include "qgsproject.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"

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

Qgis::ProjectTrustStatus QgsProjectUtils::checkUserTrust( QgsProject *project, bool *undetermined )
{
  const Qgis::EmbeddedScriptMode embeddedScriptMode = QgsSettingsRegistryCore::settingsCodeExecutionBehaviorUndeterminedProjects->value();
  switch ( embeddedScriptMode )
  {
    case Qgis::EmbeddedScriptMode::Always:
    {
      // A user having changed the behavior to always allow is considered as determined
      return Qgis::ProjectTrustStatus::Trusted;
    }

    case Qgis::EmbeddedScriptMode::Never:
    {
      // A user having changed the behavior to always deny is considered as determined
      return Qgis::ProjectTrustStatus::Untrusted;
    }

    case Qgis::EmbeddedScriptMode::Ask:
    case Qgis::EmbeddedScriptMode::NeverAsk:
    case Qgis::EmbeddedScriptMode::SessionOnly:
    case Qgis::EmbeddedScriptMode::NotForThisSession:
    {
      QString absoluteFilePath;
      QString absolutePath;
      QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromUri( project->fileName() );
      if ( storage )
      {
        if ( !storage->filePath( project->fileName() ).isEmpty() )
        {
          QFileInfo fileInfo( storage->filePath( project->fileName() ) );
          absoluteFilePath = fileInfo.absoluteFilePath();
          absolutePath = fileInfo.absolutePath();
        }
        else
        {
          // Match non-file based URIs too
          absolutePath = project->fileName();
        }
      }
      else
      {
        QFileInfo fileInfo( project->fileName() );
        absoluteFilePath = fileInfo.absoluteFilePath();
        absolutePath = fileInfo.absolutePath();
      }

      const QStringList untrustedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionUntrustedProjectsFolders->value() + QgsApplication::temporarilyUntrustedProjectsFolders();
      for ( const QString &path : untrustedProjectsFolders )
      {
        if ( absoluteFilePath == path || absolutePath == path )
        {
          return Qgis::ProjectTrustStatus::Untrusted;
        }
      }

      const QStringList trustedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionTrustedProjectsFolders->value() + QgsApplication::temporarilyTrustedProjectsFolders();
      for ( const QString &path : trustedProjectsFolders )
      {
        if ( absoluteFilePath == path || absolutePath == path )
        {
          return Qgis::ProjectTrustStatus::Trusted;
        }
      }

      return Qgis::ProjectTrustStatus::Undetermined;
    }
  }

  return Qgis::ProjectTrustStatus::Undetermined;
}

QList<QgsProject::EmbeddedCode> QgsProjectUtils::embeddedCode( QgsProject *project )
{
  QList<QgsProject::EmbeddedCode> code;

  const QString macros = project->readEntry( QStringLiteral( "Macros" ), QStringLiteral( "/pythonCode" ), QString() );
  if ( !macros.isEmpty() )
  {
    QgsProject::EmbeddedCode details;
    details.type = Qgis::EmbeddedScriptType::Macro;
    details.name = QObject::tr( "Macros" );
    details.code = macros;
    code << details;
  }

  const QString expressionFunctions = project->readEntry( QStringLiteral( "ExpressionFunctions" ), QStringLiteral( "/pythonCode" ) );
  if ( !expressionFunctions.isEmpty() )
  {
    QgsProject::EmbeddedCode details;
    details.type = Qgis::EmbeddedScriptType::ExpressionFunction;
    details.name = QObject::tr( "Expression functions" );
    details.code = expressionFunctions;
    code << details;
  }

  const QVector<QgsVectorLayer *> layers = project->layers<QgsVectorLayer *>();
  for ( QgsVectorLayer *layer : layers )
  {
    const QList<QgsAction> actions = layer->actions()->actions();
    for ( const QgsAction &action : actions )
    {
      if ( action.command().isEmpty() )
      {
        continue;
      }

      switch ( action.type() )
      {
        case Qgis::AttributeActionType::GenericPython:
        case Qgis::AttributeActionType::Mac:
        case Qgis::AttributeActionType::Windows:
        case Qgis::AttributeActionType::Unix:
        {
          QgsProject::EmbeddedCode details;
          details.type = Qgis::EmbeddedScriptType::Action;
          details.name = QObject::tr( "%1: Action ’%2’" ).arg( layer->name(), action.name() );
          details.code = action.command();
          code << details;
          break;
        }

        case Qgis::AttributeActionType::Generic:
        case Qgis::AttributeActionType::OpenUrl:
        case Qgis::AttributeActionType::SubmitUrlEncoded:
        case Qgis::AttributeActionType::SubmitUrlMultipart:
        {
          break;
        }
      }
    }

    const QgsEditFormConfig formConfig = layer->editFormConfig();
    QString initCode;
    switch ( formConfig.initCodeSource() )
    {
      case Qgis::AttributeFormPythonInitCodeSource::Dialog:
      {
        initCode = QStringLiteral( "# Calling function ’%1’\n\n%2" ).arg( formConfig.initFunction(), formConfig.initCode() );
        break;
      }

      case Qgis::AttributeFormPythonInitCodeSource::File:
      {
        QFile *inputFile = QgsApplication::networkContentFetcherRegistry()->localFile( formConfig.initFilePath() );
        if ( inputFile && inputFile->open( QFile::ReadOnly ) )
        {
          // Read it into a string
          QTextStream inf( inputFile );
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
          inf.setCodec( "UTF-8" );
#endif
          initCode = inf.readAll();
          inputFile->close();
          initCode = QStringLiteral( "# Calling function ’%1’\n# From file %2\n\n" ).arg( formConfig.initFunction(), formConfig.initFilePath() ) + initCode;
        }
        break;
      }

      case Qgis::AttributeFormPythonInitCodeSource::Environment:
      {
        initCode = QStringLiteral( "# Calling function ’%1’\n# From environment\n\n" ).arg( formConfig.initFunction() );
      }

      case Qgis::AttributeFormPythonInitCodeSource::NoSource:
      {
        break;
      }
    }

    if ( !initCode.isEmpty() )
    {
      QgsProject::EmbeddedCode details;
      details.type = Qgis::EmbeddedScriptType::FormInitCode;
      details.name = QObject::tr( "%1: Attribute form init code" ).arg( layer->name() );
      details.code = initCode;
      code << details;
    }
  }

  return code;
}
