/***************************************************************************
    qgsapplayerhandling.cpp
    -------------------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplayerhandling.h"

#include "qgsconfig.h"
#include "qgsmaplayer.h"
#include "qgsmeshlayer.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgspointcloudlayer.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"
#ifdef HAVE_3D
#include "qgspointcloudlayer3drenderer.h"
#endif
#include "canvas/qgscanvasrefreshblocker.h"
#include "qgsproviderutils.h"
#include "qgszipitem.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsprovidersublayersdialog.h"
#include "qgslayertreenode.h"
#include "qgslayertree.h"
#include "qgslayertreeview.h"
#include "qgsgui.h"
#include "qgsmbtiles.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include "qgsvectortilelayer.h"
#include "qgsprojectstorageregistry.h"
#include "qgsprojectstorage.h"
#include "qgsmaplayerfactory.h"
#include "qgsrasterlayer.h"
#include "qgsauthguiutils.h"
#include "qgslayerdefinition.h"
#include "qgspluginlayer.h"
#include "qgspluginlayerregistry.h"
#include "qgsmessagebaritem.h"
#include "qgsdockwidget.h"
#include "qgseditorwidgetregistry.h"
#include "qgsweakrelation.h"
#include "qgsfieldformatterregistry.h"
#include "qgsmaplayerutils.h"
#include "qgsfieldformatter.h"
#include "qgsabstractdatabaseproviderconnection.h"

#include <QObject>
#include <QMessageBox>
#include <QFileDialog>
#include <QUrlQuery>

void QgsAppLayerHandling::postProcessAddedLayer( QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    case QgsMapLayerType::RasterLayer:
    {
      bool ok = false;
      layer->loadDefaultStyle( ok );
      layer->loadDefaultMetadata( ok );
      break;
    }

    case QgsMapLayerType::PluginLayer:
      break;

    case QgsMapLayerType::MeshLayer:
    {
      QgsMeshLayer *meshLayer = qobject_cast< QgsMeshLayer *>( layer );
      QDateTime referenceTime = QgsProject::instance()->timeSettings()->temporalRange().begin();
      if ( !referenceTime.isValid() ) // If project reference time is invalid, use current date
        referenceTime = QDateTime( QDate::currentDate(), QTime( 0, 0, 0 ), Qt::UTC );

      if ( meshLayer->dataProvider() && !qobject_cast< QgsMeshLayerTemporalProperties * >( meshLayer->temporalProperties() )->referenceTime().isValid() )
        qobject_cast< QgsMeshLayerTemporalProperties * >( meshLayer->temporalProperties() )->setReferenceTime( referenceTime, meshLayer->dataProvider()->temporalCapabilities() );

      bool ok = false;
      meshLayer->loadDefaultStyle( ok );
      meshLayer->loadDefaultMetadata( ok );
      break;
    }

    case QgsMapLayerType::VectorTileLayer:
    {
      bool ok = false;
      QString error = layer->loadDefaultStyle( ok );
      if ( !ok )
        QgisApp::instance()->visibleMessageBar()->pushMessage( QObject::tr( "Error loading style" ), error, Qgis::MessageLevel::Warning );
      error = layer->loadDefaultMetadata( ok );
      if ( !ok )
        QgisApp::instance()->visibleMessageBar()->pushMessage( QObject::tr( "Error loading layer metadata" ), error, Qgis::MessageLevel::Warning );

      break;
    }

    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::GroupLayer:
      break;

    case QgsMapLayerType::PointCloudLayer:
    {
      bool ok = false;
      layer->loadDefaultStyle( ok );
      layer->loadDefaultMetadata( ok );

#ifdef HAVE_3D
      if ( !layer->renderer3D() )
      {
        QgsPointCloudLayer *pcLayer = qobject_cast< QgsPointCloudLayer * >( layer );
        // If the layer has no 3D renderer and syncing 3D to 2D renderer is enabled, we create a renderer and set it up with the 2D renderer
        if ( pcLayer->sync3DRendererTo2DRenderer() )
        {
          std::unique_ptr< QgsPointCloudLayer3DRenderer > renderer3D = std::make_unique< QgsPointCloudLayer3DRenderer >();
          renderer3D->convertFrom2DRenderer( pcLayer->renderer() );
          layer->setRenderer3D( renderer3D.release() );
        }
      }
#endif
      break;
    }
  }
}

void QgsAppLayerHandling::postProcessAddedLayers( const QList<QgsMapLayer *> & )
{
}

QList< QgsMapLayer * > QgsAppLayerHandling::addVectorLayers( const QStringList &layers, const QString &enc, const QString &dataSourceType, bool &ok, bool guiWarning )
{
  //note: this method ONLY supports vector layers from the OGR provider!
  ok = false;

  QgsCanvasRefreshBlocker refreshBlocker;

  QList<QgsMapLayer *> layersToAdd;
  QList<QgsMapLayer *> addedLayers;
  QgsSettings settings;
  bool userAskedToAddLayers = false;

  for ( const QString &layerUri : layers )
  {
    const QString uri = layerUri.trimmed();
    QString baseName;
    if ( dataSourceType == QLatin1String( "file" ) )
    {
      QString srcWithoutLayername( uri );
      int posPipe = srcWithoutLayername.indexOf( '|' );
      if ( posPipe >= 0 )
        srcWithoutLayername.resize( posPipe );
      baseName = QgsProviderUtils::suggestLayerNameFromFilePath( srcWithoutLayername );

      // if needed prompt for zipitem layers
      QString vsiPrefix = QgsZipItem::vsiPrefix( uri );
      if ( ! uri.startsWith( QLatin1String( "/vsi" ), Qt::CaseInsensitive ) &&
           ( vsiPrefix == QLatin1String( "/vsizip/" ) || vsiPrefix == QLatin1String( "/vsitar/" ) ) )
      {
        if ( askUserForZipItemLayers( uri, { QgsMapLayerType::VectorLayer } ) )
          continue;
      }
    }
    else if ( dataSourceType == QLatin1String( "database" ) )
    {
      // Try to extract the database name and use it as base name
      // sublayers names (if any) will be appended to the layer name
      const QVariantMap parts( QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), uri ) );
      if ( parts.value( QStringLiteral( "databaseName" ) ).isValid() )
        baseName = parts.value( QStringLiteral( "databaseName" ) ).toString();
      else
        baseName = uri;
    }
    else //directory //protocol
    {
      baseName = QgsProviderUtils::suggestLayerNameFromFilePath( uri );
    }

    if ( settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool() )
    {
      baseName = QgsMapLayer::formatLayerName( baseName );
    }

    QgsDebugMsgLevel( "completeBaseName: " + baseName, 2 );
    const bool isVsiCurl { uri.startsWith( QLatin1String( "/vsicurl" ), Qt::CaseInsensitive ) };
    const auto scheme { QUrl( uri ).scheme() };
    const bool isRemoteUrl { scheme.startsWith( QLatin1String( "http" ) ) || scheme == QLatin1String( "ftp" ) };

    std::unique_ptr< QgsTemporaryCursorOverride > cursorOverride;
    if ( isVsiCurl || isRemoteUrl )
    {
      cursorOverride = std::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
      QgisApp::instance()->visibleMessageBar()->pushInfo( QObject::tr( "Remote layer" ), QObject::tr( "loading %1, please wait …" ).arg( uri ) );
      qApp->processEvents();
    }

    QList< QgsProviderSublayerDetails > sublayers = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->querySublayers( uri, Qgis::SublayerQueryFlag::IncludeSystemTables );
    // filter out non-vector sublayers
    sublayers.erase( std::remove_if( sublayers.begin(), sublayers.end(), []( const QgsProviderSublayerDetails & sublayer )
    {
      return sublayer.type() != QgsMapLayerType::VectorLayer;
    } ), sublayers.end() );

    cursorOverride.reset();

    const QVariantMap uriParts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), uri );
    const QString path = uriParts.value( QStringLiteral( "path" ) ).toString();

    if ( !sublayers.empty() )
    {
      userAskedToAddLayers = true;

      const bool detailsAreIncomplete = QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount );
      const bool singleSublayerOnly = sublayers.size() == 1;
      QString groupName;

      if ( !singleSublayerOnly || detailsAreIncomplete )
      {
        // ask user for sublayers (unless user settings dictate otherwise!)
        switch ( shouldAskUserForSublayers( sublayers ) )
        {
          case SublayerHandling::AskUser:
          {
            // prompt user for sublayers
            QgsProviderSublayersDialog dlg( uri, path, sublayers, {QgsMapLayerType::VectorLayer}, QgisApp::instance() );

            if ( dlg.exec() )
              sublayers = dlg.selectedLayers();
            else
              sublayers.clear(); // dialog was canceled, so don't add any sublayers
            groupName = dlg.groupName();
            break;
          }

          case SublayerHandling::LoadAll:
          {
            if ( detailsAreIncomplete )
            {
              // requery sublayers, resolving geometry types
              sublayers = QgsProviderRegistry::instance()->querySublayers( uri, Qgis::SublayerQueryFlag::ResolveGeometryType );
              // filter out non-vector sublayers
              sublayers.erase( std::remove_if( sublayers.begin(), sublayers.end(), []( const QgsProviderSublayerDetails & sublayer )
              {
                return sublayer.type() != QgsMapLayerType::VectorLayer;
              } ), sublayers.end() );
            }
            break;
          }

          case SublayerHandling::AbortLoading:
            sublayers.clear(); // don't add any sublayers
            break;
        };
      }
      else if ( detailsAreIncomplete )
      {
        // requery sublayers, resolving geometry types
        sublayers = QgsProviderRegistry::instance()->querySublayers( uri, Qgis::SublayerQueryFlag::ResolveGeometryType );
        // filter out non-vector sublayers
        sublayers.erase( std::remove_if( sublayers.begin(), sublayers.end(), []( const QgsProviderSublayerDetails & sublayer )
        {
          return sublayer.type() != QgsMapLayerType::VectorLayer;
        } ), sublayers.end() );
      }

      // now add sublayers
      if ( !sublayers.empty() )
      {
        addedLayers << addSublayers( sublayers, baseName, groupName );
      }

    }
    else
    {
      QString msg = QObject::tr( "%1 is not a valid or recognized data source." ).arg( uri );
      // If the failed layer was a vsicurl type, give the user a chance to try the normal download.
      if ( isVsiCurl &&
           QMessageBox::question( QgisApp::instance(), QObject::tr( "Invalid Data Source" ),
                                  QObject::tr( "Download with \"Protocol\" source type has failed, do you want to try the \"File\" source type?" ) ) == QMessageBox::Yes )
      {
        QString fileUri = uri;
        fileUri.replace( QLatin1String( "/vsicurl/" ), " " );
        return addVectorLayers( QStringList() << fileUri, enc, dataSourceType, guiWarning );
      }
      else if ( guiWarning )
      {
        QgisApp::instance()->visibleMessageBar()->pushMessage( QObject::tr( "Invalid Data Source" ), msg, Qgis::MessageLevel::Critical );
      }
    }
  }

  // make sure at least one layer was successfully added
  if ( layersToAdd.isEmpty() )
  {
    // we also return true if we asked the user for sublayers, but they choose none. In this case nothing
    // went wrong, so we shouldn't return false and cause GUI warnings to appear
    ok = userAskedToAddLayers || !addedLayers.isEmpty();
  }

  // Register this layer with the layers registry
  QgsProject::instance()->addMapLayers( layersToAdd );
  for ( QgsMapLayer *l : std::as_const( layersToAdd ) )
  {
    if ( !enc.isEmpty() )
    {
      if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( l ) )
        vl->setProviderEncoding( enc );
    }

    QgisApp::instance()->askUserForDatumTransform( l->crs(), QgsProject::instance()->crs(), l );
    QgsAppLayerHandling::postProcessAddedLayer( l );
  }
  QgisApp::instance()->activateDeactivateLayerRelatedActions( QgisApp::instance()->activeLayer() );

  ok = true;
  addedLayers.append( layersToAdd );
  return addedLayers;
}

QgsPointCloudLayer *QgsAppLayerHandling::addPointCloudLayer( const QString &uri, const QString &baseName, const QString &providerKey, bool guiWarning )
{
  QgsCanvasRefreshBlocker refreshBlocker;
  QgsSettings settings;

  QString base( baseName );

  if ( settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool() )
  {
    base = QgsMapLayer::formatLayerName( base );
  }

  QgsDebugMsgLevel( "completeBaseName: " + base, 2 );

  // create the layer
  std::unique_ptr<QgsPointCloudLayer> layer( new QgsPointCloudLayer( uri, base, providerKey ) );

  if ( !layer || !layer->isValid() )
  {
    if ( guiWarning )
    {
      QString msg = QObject::tr( "%1 is not a valid or recognized data source, error: \"%2\"" ).arg( uri, layer->error().message( QgsErrorMessage::Format::Text ) );
      QgisApp::instance()->visibleMessageBar()->pushMessage( QObject::tr( "Invalid Data Source" ), msg, Qgis::MessageLevel::Critical );
    }

    // since the layer is bad, stomp on it
    return nullptr;
  }

  QgsAppLayerHandling::postProcessAddedLayer( layer.get() );


  QgsProject::instance()->addMapLayer( layer.get() );
  QgisApp::instance()->activateDeactivateLayerRelatedActions( QgisApp::instance()->activeLayer() );

  return layer.release();
}

QgsPluginLayer *QgsAppLayerHandling::addPluginLayer( const QString &uri, const QString &baseName, const QString &providerKey )
{
  QgsPluginLayer *layer = QgsApplication::pluginLayerRegistry()->createLayer( providerKey, uri );
  if ( !layer )
    return nullptr;

  layer->setName( baseName );

  QgsProject::instance()->addMapLayer( layer );

  return layer;
}

QgsVectorTileLayer *QgsAppLayerHandling::addVectorTileLayer( const QString &uri, const QString &baseName, bool guiWarning )
{
  QgsCanvasRefreshBlocker refreshBlocker;
  QgsSettings settings;

  QString base( baseName );

  if ( settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool() )
  {
    base = QgsMapLayer::formatLayerName( base );
  }

  QgsDebugMsgLevel( "completeBaseName: " + base, 2 );

  // create the layer
  const QgsVectorTileLayer::LayerOptions options( QgsProject::instance()->transformContext() );
  std::unique_ptr<QgsVectorTileLayer> layer( new QgsVectorTileLayer( uri, base, options ) );

  if ( !layer || !layer->isValid() )
  {
    if ( guiWarning )
    {
      QString msg = QObject::tr( "%1 is not a valid or recognized data source." ).arg( uri );
      QgisApp::instance()->visibleMessageBar()->pushMessage( QObject::tr( "Invalid Data Source" ), msg, Qgis::MessageLevel::Critical );
    }

    // since the layer is bad, stomp on it
    return nullptr;
  }

  QgsAppLayerHandling::postProcessAddedLayer( layer.get() );

  QgsProject::instance()->addMapLayer( layer.get() );
  QgisApp::instance()->activateDeactivateLayerRelatedActions( QgisApp::instance()->activeLayer() );

  return layer.release();
}

bool QgsAppLayerHandling::askUserForZipItemLayers( const QString &path, const QList<QgsMapLayerType> &acceptableTypes )
{
  // query sublayers
  QList< QgsProviderSublayerDetails > sublayers = QgsProviderRegistry::instance()->querySublayers( path, Qgis::SublayerQueryFlag::IncludeSystemTables );

  // filter out non-matching sublayers
  sublayers.erase( std::remove_if( sublayers.begin(), sublayers.end(), [acceptableTypes]( const QgsProviderSublayerDetails & sublayer )
  {
    return !acceptableTypes.empty() && !acceptableTypes.contains( sublayer.type() );
  } ), sublayers.end() );

  if ( sublayers.empty() )
    return false;

  const bool detailsAreIncomplete = QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount );
  const bool singleSublayerOnly = sublayers.size() == 1;
  QString groupName;

  if ( !singleSublayerOnly || detailsAreIncomplete )
  {
    // ask user for sublayers (unless user settings dictate otherwise!)
    switch ( shouldAskUserForSublayers( sublayers ) )
    {
      case SublayerHandling::AskUser:
      {
        // prompt user for sublayers
        QgsProviderSublayersDialog dlg( path, path, sublayers, acceptableTypes, QgisApp::instance() );

        if ( dlg.exec() )
          sublayers = dlg.selectedLayers();
        else
          sublayers.clear(); // dialog was canceled, so don't add any sublayers
        groupName = dlg.groupName();
        break;
      }

      case SublayerHandling::LoadAll:
      {
        if ( detailsAreIncomplete )
        {
          // requery sublayers, resolving geometry types
          sublayers = QgsProviderRegistry::instance()->querySublayers( path, Qgis::SublayerQueryFlag::ResolveGeometryType );
          sublayers.erase( std::remove_if( sublayers.begin(), sublayers.end(), [acceptableTypes]( const QgsProviderSublayerDetails & sublayer )
          {
            return !acceptableTypes.empty() && !acceptableTypes.contains( sublayer.type() );
          } ), sublayers.end() );
        }
        break;
      }

      case SublayerHandling::AbortLoading:
        sublayers.clear(); // don't add any sublayers
        break;
    };
  }
  else if ( detailsAreIncomplete )
  {
    // requery sublayers, resolving geometry types
    sublayers = QgsProviderRegistry::instance()->querySublayers( path, Qgis::SublayerQueryFlag::ResolveGeometryType );
    sublayers.erase( std::remove_if( sublayers.begin(), sublayers.end(), [acceptableTypes]( const QgsProviderSublayerDetails & sublayer )
    {
      return !acceptableTypes.empty() && !acceptableTypes.contains( sublayer.type() );
    } ), sublayers.end() );
  }

  // now add sublayers
  if ( !sublayers.empty() )
  {
    QgsCanvasRefreshBlocker refreshBlocker;
    QgsSettings settings;

    QString base = QgsProviderUtils::suggestLayerNameFromFilePath( path );
    if ( settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool() )
    {
      base = QgsMapLayer::formatLayerName( base );
    }

    addSublayers( sublayers, base, groupName );
    QgisApp::instance()->activateDeactivateLayerRelatedActions( QgisApp::instance()->activeLayer() );
  }

  return true;
}

QgsAppLayerHandling::SublayerHandling QgsAppLayerHandling::shouldAskUserForSublayers( const QList<QgsProviderSublayerDetails> &layers, bool hasNonLayerItems )
{
  if ( hasNonLayerItems )
    return SublayerHandling::AskUser;

  QgsSettings settings;
  const Qgis::SublayerPromptMode promptLayers = settings.enumValue( QStringLiteral( "qgis/promptForSublayers" ), Qgis::SublayerPromptMode::AlwaysAsk );

  switch ( promptLayers )
  {
    case Qgis::SublayerPromptMode::AlwaysAsk:
      return SublayerHandling::AskUser;

    case Qgis::SublayerPromptMode::AskExcludingRasterBands:
    {
      // if any non-raster layers are found, we ask the user. Otherwise we load all
      for ( const QgsProviderSublayerDetails &sublayer : layers )
      {
        if ( sublayer.type() != QgsMapLayerType::RasterLayer )
          return SublayerHandling::AskUser;
      }
      return SublayerHandling::LoadAll;
    }

    case Qgis::SublayerPromptMode::NeverAskSkip:
      return SublayerHandling::AbortLoading;

    case Qgis::SublayerPromptMode::NeverAskLoadAll:
      return SublayerHandling::LoadAll;
  }

  return SublayerHandling::AskUser;
}

QList<QgsMapLayer *> QgsAppLayerHandling::addSublayers( const QList<QgsProviderSublayerDetails> &layers, const QString &baseName, const QString &groupName )
{
  QgsLayerTreeGroup *group = nullptr;
  if ( !groupName.isEmpty() )
  {
    int index { 0 };
    QgsLayerTreeNode *currentNode { QgisApp::instance()->layerTreeView()->currentNode() };
    if ( currentNode && currentNode->parent() )
    {
      if ( QgsLayerTree::isGroup( currentNode ) )
      {
        group = qobject_cast<QgsLayerTreeGroup *>( currentNode )->insertGroup( 0, groupName );
      }
      else if ( QgsLayerTree::isLayer( currentNode ) )
      {
        const QList<QgsLayerTreeNode *> currentNodeSiblings { currentNode->parent()->children() };
        int nodeIdx { 0 };
        for ( const QgsLayerTreeNode *child : std::as_const( currentNodeSiblings ) )
        {
          nodeIdx++;
          if ( child == currentNode )
          {
            index = nodeIdx;
            break;
          }
        }
        group = qobject_cast<QgsLayerTreeGroup *>( currentNode->parent() )->insertGroup( index, groupName );
      }
      else
      {
        group = QgsProject::instance()->layerTreeRoot()->insertGroup( 0, groupName );
      }
    }
    else
    {
      group = QgsProject::instance()->layerTreeRoot()->insertGroup( 0, groupName );
    }
  }

  QgsSettings settings;
  const bool formatLayerNames = settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool();

  // if we aren't adding to a group, we need to add the layers in reverse order so that they maintain the correct
  // order in the layer tree!
  QList<QgsProviderSublayerDetails> sortedLayers = layers;
  if ( groupName.isEmpty() )
  {
    std::reverse( sortedLayers.begin(), sortedLayers.end() );
  }

  QList< QgsMapLayer * > result;
  result.reserve( sortedLayers.size() );

  for ( const QgsProviderSublayerDetails &sublayer : std::as_const( sortedLayers ) )
  {
    QgsProviderSublayerDetails::LayerOptions options( QgsProject::instance()->transformContext() );
    options.loadDefaultStyle = false;

    std::unique_ptr<QgsMapLayer> layer( sublayer.toLayer( options ) );
    if ( !layer )
      continue;

    QgsMapLayer *ml = layer.get();
    // if we aren't adding to a group, then we're iterating the layers in the reverse order
    // so account for that in the returned list of layers
    if ( groupName.isEmpty() )
      result.insert( 0, ml );
    else
      result << ml;

    QString layerName = layer->name();
    if ( formatLayerNames )
    {
      layerName = QgsMapLayer::formatLayerName( layerName );
    }

    const bool projectWasEmpty = QgsProject::instance()->mapLayers().empty();

    // if user has opted to add sublayers to a group, then we don't need to include the
    // filename in the layer's name, because the group is already titled with the filename.
    // But otherwise, we DO include the file name so that users can differentiate the source
    // when multiple layers are loaded from a GPX file or similar (refs https://github.com/qgis/QGIS/issues/37551)
    if ( group )
    {
      if ( !layerName.isEmpty() )
        layer->setName( layerName );
      else if ( !baseName.isEmpty() )
        layer->setName( baseName );
      QgsProject::instance()->addMapLayer( layer.release(), false );
      group->addLayer( ml );
    }
    else
    {
      if ( layerName != baseName && !layerName.isEmpty() && !baseName.isEmpty() )
        layer->setName( QStringLiteral( "%1 — %2" ).arg( baseName, layerName ) );
      else if ( !layerName.isEmpty() )
        layer->setName( layerName );
      else if ( !baseName.isEmpty() )
        layer->setName( baseName );
      QgsProject::instance()->addMapLayer( layer.release() );
    }

    // Some of the logic relating to matching a new project's CRS to the first layer added CRS is deferred to happen when the event loop
    // next runs -- so in those cases we can't assume that the project's CRS has been matched to the actual desired CRS yet.
    // In these cases we don't need to show the coordinate operation selection choice, so just hardcode an exception in here to avoid that...
    QgsCoordinateReferenceSystem projectCrsAfterLayerAdd = QgsProject::instance()->crs();
    const QgsGui::ProjectCrsBehavior projectCrsBehavior = QgsSettings().enumValue( QStringLiteral( "/projections/newProjectCrsBehavior" ),  QgsGui::UseCrsOfFirstLayerAdded, QgsSettings::App );
    switch ( projectCrsBehavior )
    {
      case QgsGui::UseCrsOfFirstLayerAdded:
      {
        if ( projectWasEmpty )
          projectCrsAfterLayerAdd = ml->crs();
        break;
      }

      case QgsGui::UsePresetCrs:
        break;
    }

    QgisApp::instance()->askUserForDatumTransform( ml->crs(), projectCrsAfterLayerAdd, ml );
    QgsAppLayerHandling::postProcessAddedLayer( ml );
  }

  if ( group )
  {
    // Respect if user don't want the new group of layers visible.
    QgsSettings settings;
    const bool newLayersVisible = settings.value( QStringLiteral( "/qgis/new_layers_visible" ), true ).toBool();
    if ( !newLayersVisible )
      group->setItemVisibilityCheckedRecursive( newLayersVisible );
  }

  return result;
}

QList< QgsMapLayer * > QgsAppLayerHandling::openLayer( const QString &fileName, bool &ok, bool allowInteractive, bool suppressBulkLayerPostProcessing )
{
  QList< QgsMapLayer * > openedLayers;
  auto postProcessAddedLayers = [suppressBulkLayerPostProcessing, &openedLayers]
  {
    if ( !suppressBulkLayerPostProcessing )
      QgsAppLayerHandling::postProcessAddedLayers( openedLayers );
  };

  ok = false;
  const QFileInfo fileInfo( fileName );

  // highest priority = delegate to provider registry to handle
  const QList< QgsProviderRegistry::ProviderCandidateDetails > candidateProviders = QgsProviderRegistry::instance()->preferredProvidersForUri( fileName );
  if ( candidateProviders.size() == 1 && candidateProviders.at( 0 ).layerTypes().size() == 1 )
  {
    // one good candidate provider and possible layer type -- that makes things nice and easy!
    switch ( candidateProviders.at( 0 ).layerTypes().at( 0 ) )
    {
      case QgsMapLayerType::VectorLayer:
      case QgsMapLayerType::RasterLayer:
      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::VectorTileLayer:
      case QgsMapLayerType::GroupLayer:
        // not supported here yet!
        break;

      case QgsMapLayerType::PointCloudLayer:
      {
        if ( QgsPointCloudLayer *layer = addPointCloudLayer( fileName, fileInfo.completeBaseName(), candidateProviders.at( 0 ).metadata()->key(), false ) )
        {
          ok = true;
          openedLayers << layer;
        }
        break;
      }
    }
  }

  if ( ok )
  {
    postProcessAddedLayers();
    return openedLayers;
  }

  CPLPushErrorHandler( CPLQuietErrorHandler );

  // if needed prompt for zipitem layers
  QString vsiPrefix = QgsZipItem::vsiPrefix( fileName );
  if ( vsiPrefix == QLatin1String( "/vsizip/" ) || vsiPrefix == QLatin1String( "/vsitar/" ) )
  {
    if ( askUserForZipItemLayers( fileName, {} ) )
    {
      CPLPopErrorHandler();
      ok = true;
      return openedLayers;
    }
  }

  if ( fileName.endsWith( QStringLiteral( ".mbtiles" ), Qt::CaseInsensitive ) )
  {
    QgsMbTiles reader( fileName );
    if ( reader.open() )
    {
      if ( reader.metadataValue( "format" ) == QLatin1String( "pbf" ) )
      {
        // these are vector tiles
        QUrlQuery uq;
        uq.addQueryItem( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
        uq.addQueryItem( QStringLiteral( "url" ), fileName );
        const QgsVectorTileLayer::LayerOptions options( QgsProject::instance()->transformContext() );
        std::unique_ptr<QgsVectorTileLayer> vtLayer( new QgsVectorTileLayer( uq.toString(), fileInfo.completeBaseName(), options ) );
        if ( vtLayer->isValid() )
        {
          openedLayers << vtLayer.get();
          QgsProject::instance()->addMapLayer( vtLayer.release() );
          postProcessAddedLayers();
          ok = true;
          return openedLayers;
        }
      }
      else // raster tiles
      {
        // prefer to use WMS provider's implementation to open MBTiles rasters
        QUrlQuery uq;
        uq.addQueryItem( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
        uq.addQueryItem( QStringLiteral( "url" ), QUrl::fromLocalFile( fileName ).toString() );
        if ( QgsRasterLayer *rasterLayer = addRasterLayer( uq.toString(), fileInfo.completeBaseName(), QStringLiteral( "wms" ) ) )
        {
          openedLayers << rasterLayer;
          postProcessAddedLayers();
          ok = true;
          return openedLayers;
        }
      }
    }
  }
  else if ( fileName.endsWith( QStringLiteral( ".vtpk" ), Qt::CaseInsensitive ) )
  {
    // these are vector tiles
    QUrlQuery uq;
    uq.addQueryItem( QStringLiteral( "type" ), QStringLiteral( "vtpk" ) );
    uq.addQueryItem( QStringLiteral( "url" ), fileName );
    const QgsVectorTileLayer::LayerOptions options( QgsProject::instance()->transformContext() );
    std::unique_ptr<QgsVectorTileLayer> vtLayer( new QgsVectorTileLayer( uq.toString(), fileInfo.completeBaseName(), options ) );
    if ( vtLayer->isValid() )
    {
      openedLayers << vtLayer.get();
      QgsAppLayerHandling::postProcessAddedLayer( vtLayer.get() );
      QgsProject::instance()->addMapLayer( vtLayer.release() );
      postProcessAddedLayers();
      ok = true;
      return openedLayers;
    }
  }

  QList< QgsProviderSublayerModel::NonLayerItem > nonLayerItems;
  if ( QgsProjectStorage *ps = QgsApplication::projectStorageRegistry()->projectStorageFromUri( fileName ) )
  {
    const QStringList projects = ps->listProjects( fileName );
    for ( const QString &project : projects )
    {
      QgsProviderSublayerModel::NonLayerItem projectItem;
      projectItem.setType( QStringLiteral( "project" ) );
      projectItem.setName( project );
      projectItem.setUri( QStringLiteral( "%1://%2?projectName=%3" ).arg( ps->type(), fileName, project ) );
      projectItem.setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconQgsProjectFile.svg" ) ) );
      nonLayerItems << projectItem;
    }
  }

  // query sublayers
  QList< QgsProviderSublayerDetails > sublayers = QgsProviderRegistry::instance()->querySublayers( fileName, Qgis::SublayerQueryFlag::IncludeSystemTables );

  if ( !sublayers.empty() || !nonLayerItems.empty() )
  {
    const bool detailsAreIncomplete = QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount );
    const bool singleSublayerOnly = sublayers.size() == 1;
    QString groupName;

    if ( allowInteractive && ( !singleSublayerOnly || detailsAreIncomplete || !nonLayerItems.empty() ) )
    {
      // ask user for sublayers (unless user settings dictate otherwise!)
      switch ( shouldAskUserForSublayers( sublayers, !nonLayerItems.empty() ) )
      {
        case SublayerHandling::AskUser:
        {
          // prompt user for sublayers
          QgsProviderSublayersDialog dlg( fileName, fileName, sublayers, {}, QgisApp::instance() );
          dlg.setNonLayerItems( nonLayerItems );

          if ( dlg.exec() )
          {
            sublayers = dlg.selectedLayers();
            nonLayerItems = dlg.selectedNonLayerItems();
          }
          else
          {
            sublayers.clear(); // dialog was canceled, so don't add any sublayers
            nonLayerItems.clear();
          }
          groupName = dlg.groupName();
          break;
        }

        case SublayerHandling::LoadAll:
        {
          if ( detailsAreIncomplete )
          {
            // requery sublayers, resolving geometry types
            sublayers = QgsProviderRegistry::instance()->querySublayers( fileName, Qgis::SublayerQueryFlag::ResolveGeometryType );
          }
          break;
        }

        case SublayerHandling::AbortLoading:
          sublayers.clear(); // don't add any sublayers
          break;
      };
    }
    else if ( detailsAreIncomplete )
    {
      // requery sublayers, resolving geometry types
      sublayers = QgsProviderRegistry::instance()->querySublayers( fileName, Qgis::SublayerQueryFlag::ResolveGeometryType );
    }

    ok = true;

    // now add sublayers
    if ( !sublayers.empty() )
    {
      QgsCanvasRefreshBlocker refreshBlocker;
      QgsSettings settings;

      QString base = QgsProviderUtils::suggestLayerNameFromFilePath( fileName );
      if ( settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool() )
      {
        base = QgsMapLayer::formatLayerName( base );
      }

      openedLayers.append( addSublayers( sublayers, base, groupName ) );
      QgisApp::instance()->activateDeactivateLayerRelatedActions( QgisApp::instance()->activeLayer() );
    }
    else if ( !nonLayerItems.empty() )
    {
      ok = true;
      QgsCanvasRefreshBlocker refreshBlocker;
      if ( QgisApp::instance()->checkTasksDependOnProject() )
        return {};

      // possibly save any pending work before opening a different project
      if ( QgisApp::instance()->checkUnsavedLayerEdits() && QgisApp::instance()->checkMemoryLayers() && QgisApp::instance()->saveDirty() )
      {
        // error handling and reporting is in addProject() function
        QgisApp::instance()->addProject( nonLayerItems.at( 0 ).uri() );
      }
      return {};
    }
  }

  CPLPopErrorHandler();

  if ( !ok )
  {
    // maybe a known file type, which couldn't be opened due to a missing dependency... (eg. las for a non-pdal-enabled build)
    QgsProviderRegistry::UnusableUriDetails details;
    if ( QgsProviderRegistry::instance()->handleUnusableUri( fileName, details ) )
    {
      ok = true;

      if ( details.detailedWarning.isEmpty() )
        QgisApp::instance()->visibleMessageBar()->pushMessage( QString(), details.warning, Qgis::MessageLevel::Critical );
      else
        QgisApp::instance()->visibleMessageBar()->pushMessage( QString(), details.warning, details.detailedWarning, Qgis::MessageLevel::Critical );
    }
  }

  if ( !ok )
  {
    // we have no idea what this file is...
    QgsMessageLog::logMessage( QObject::tr( "Unable to load %1" ).arg( fileName ) );

    const QString msg = QObject::tr( "%1 is not a valid or recognized data source." ).arg( fileName );
    QgisApp::instance()->visibleMessageBar()->pushMessage( QObject::tr( "Invalid Data Source" ), msg, Qgis::MessageLevel::Critical );
  }
  else
  {
    postProcessAddedLayers();
  }

  return openedLayers;
}

QgsVectorLayer *QgsAppLayerHandling::addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey )
{
  return addLayerPrivate< QgsVectorLayer >( QgsMapLayerType::VectorLayer, vectorLayerPath, baseName, !providerKey.isEmpty() ? providerKey : QLatin1String( "ogr" ), true );
}

QgsRasterLayer *QgsAppLayerHandling::addRasterLayer( const QString &uri, const QString &baseName, const QString &providerKey )
{
  return addLayerPrivate< QgsRasterLayer >( QgsMapLayerType::RasterLayer, uri, baseName, !providerKey.isEmpty() ? providerKey : QLatin1String( "gdal" ), true );
}

QgsMeshLayer *QgsAppLayerHandling::addMeshLayer( const QString &url, const QString &baseName, const QString &providerKey )
{
  return addLayerPrivate< QgsMeshLayer >( QgsMapLayerType::MeshLayer, url, baseName, providerKey, true );
}

QList<QgsMapLayer *> QgsAppLayerHandling::addRasterLayers( const QStringList &files, bool &ok, bool guiWarning )
{
  ok = false;
  if ( files.empty() )
  {
    return {};
  }

  QgsCanvasRefreshBlocker refreshBlocker;

  // this is messy since some files in the list may be rasters and others may
  // be ogr layers. We'll set returnValue to false if one or more layers fail
  // to load.

  QList< QgsMapLayer * > res;

  for ( const QString &src : files )
  {
    QString errMsg;

    // if needed prompt for zipitem layers
    QString vsiPrefix = QgsZipItem::vsiPrefix( src );
    if ( ( !src.startsWith( QLatin1String( "/vsi" ), Qt::CaseInsensitive ) || src.endsWith( QLatin1String( ".zip" ) ) || src.endsWith( QLatin1String( ".tar" ) ) ) &&
         ( vsiPrefix == QLatin1String( "/vsizip/" ) || vsiPrefix == QLatin1String( "/vsitar/" ) ) )
    {
      if ( askUserForZipItemLayers( src, { QgsMapLayerType::RasterLayer } ) )
        continue;
    }

    const bool isVsiCurl { src.startsWith( QLatin1String( "/vsicurl" ), Qt::CaseInsensitive ) };
    const bool isRemoteUrl { src.startsWith( QLatin1String( "http" ) ) || src == QLatin1String( "ftp" ) };

    std::unique_ptr< QgsTemporaryCursorOverride > cursorOverride;
    if ( isVsiCurl || isRemoteUrl )
    {
      cursorOverride = std::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
      QgisApp::instance()->visibleMessageBar()->pushInfo( QObject::tr( "Remote layer" ), QObject::tr( "loading %1, please wait …" ).arg( src ) );
      qApp->processEvents();
    }

    if ( QgsRasterLayer::isValidRasterFileName( src, errMsg ) )
    {
      QFileInfo myFileInfo( src );

      // set the layer name to the file base name unless provided explicitly
      QString layerName;
      const QVariantMap uriDetails = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), src );
      if ( !uriDetails[ QStringLiteral( "layerName" ) ].toString().isEmpty() )
      {
        layerName = uriDetails[ QStringLiteral( "layerName" ) ].toString();
      }
      else
      {
        layerName = QgsProviderUtils::suggestLayerNameFromFilePath( src );
      }

      // try to create the layer
      cursorOverride.reset();
      QgsRasterLayer *layer = addLayerPrivate< QgsRasterLayer >( QgsMapLayerType::RasterLayer, src, layerName, QStringLiteral( "gdal" ), guiWarning );
      res << layer;

      if ( layer && layer->isValid() )
      {
        //only allow one copy of a ai grid file to be loaded at a
        //time to prevent the user selecting all adfs in 1 dir which
        //actually represent 1 coverage,

        if ( myFileInfo.fileName().endsWith( QLatin1String( ".adf" ), Qt::CaseInsensitive ) )
        {
          break;
        }
      }
      // if layer is invalid addLayerPrivate() will show the error

    } // valid raster filename
    else
    {
      ok = false;

      // Issue message box warning unless we are loading from cmd line since
      // non-rasters are passed to this function first and then successfully
      // loaded afterwards (see main.cpp)
      if ( guiWarning )
      {
        QString msg = QObject::tr( "%1 is not a supported raster data source" ).arg( src );
        if ( !errMsg.isEmpty() )
          msg += '\n' + errMsg;

        QgisApp::instance()->visibleMessageBar()->pushMessage( QObject::tr( "Unsupported Data Source" ), msg, Qgis::MessageLevel::Critical );
      }
    }
  }
  return res;
}

void QgsAppLayerHandling::addMapLayer( QgsMapLayer *mapLayer )
{
  QgsCanvasRefreshBlocker refreshBlocker;

  if ( mapLayer->isValid() )
  {
    // Register this layer with the layers registry
    QList<QgsMapLayer *> myList;
    myList << mapLayer;
    QgsProject::instance()->addMapLayers( myList );

    QgisApp::instance()->askUserForDatumTransform( mapLayer->crs(), QgsProject::instance()->crs(), mapLayer );
  }
  else
  {
    QString msg = QObject::tr( "The layer is not a valid layer and can not be added to the map" );
    QgisApp::instance()->visibleMessageBar()->pushMessage( QObject::tr( "Layer is not valid" ), msg, Qgis::MessageLevel::Critical );
  }
}

void QgsAppLayerHandling::openLayerDefinition( const QString &filename )
{
  QString errorMessage;
  QgsReadWriteContext context;
  bool loaded = false;

  QFile file( filename );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    errorMessage = QStringLiteral( "Can not open file" );
  }
  else
  {
    QDomDocument doc;
    QString message;
    if ( !doc.setContent( &file, &message ) )
    {
      errorMessage = message;
    }
    else
    {
      QFileInfo fileinfo( file );
      QDir::setCurrent( fileinfo.absoluteDir().path() );

      context.setPathResolver( QgsPathResolver( filename ) );
      context.setProjectTranslator( QgsProject::instance() );

      loaded = QgsLayerDefinition::loadLayerDefinition( doc, QgsProject::instance(), QgsProject::instance()->layerTreeRoot(), errorMessage, context );
    }
  }

  if ( loaded )
  {
    const QList< QgsReadWriteContext::ReadWriteMessage > messages = context.takeMessages();
    QVector< QgsReadWriteContext::ReadWriteMessage > shownMessages;
    for ( const QgsReadWriteContext::ReadWriteMessage &message : messages )
    {
      if ( shownMessages.contains( message ) )
        continue;

      QgisApp::instance()->visibleMessageBar()->pushMessage( QString(), message.message(), message.categories().join( '\n' ), message.level() );

      shownMessages.append( message );
    }
  }
  else if ( !loaded || !errorMessage.isEmpty() )
  {
    QgisApp::instance()->visibleMessageBar()->pushMessage( QObject::tr( "Error loading layer definition" ), errorMessage, Qgis::MessageLevel::Warning );
  }
}

void QgsAppLayerHandling::addLayerDefinition()
{
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastQLRDir" ), QDir::homePath() ).toString();

  QString path = QFileDialog::getOpenFileName( QgisApp::instance(), QStringLiteral( "Add Layer Definition File" ), lastUsedDir, QStringLiteral( "*.qlr" ) );
  if ( path.isEmpty() )
    return;

  QFileInfo fi( path );
  settings.setValue( QStringLiteral( "UI/lastQLRDir" ), fi.path() );

  openLayerDefinition( path );
}

QList< QgsMapLayer * > QgsAppLayerHandling::addDatabaseLayers( const QStringList &layerPathList, const QString &providerKey, bool &ok )
{
  ok = false;
  QList<QgsMapLayer *> myList;

  if ( layerPathList.empty() )
  {
    // no layers to add so bail out, but
    // allow mMapCanvas to handle events
    // first
    return {};
  }

  QgsCanvasRefreshBlocker refreshBlocker;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  const auto constLayerPathList = layerPathList;
  for ( const QString &layerPath : constLayerPathList )
  {
    // create the layer
    QgsDataSourceUri uri( layerPath );

    QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
    options.loadDefaultStyle = false;
    QgsVectorLayer *layer = new QgsVectorLayer( uri.uri( false ), uri.table(), providerKey, options );
    Q_CHECK_PTR( layer );

    if ( ! layer )
    {
      QApplication::restoreOverrideCursor();

      // XXX insert meaningful whine to the user here
      return {};
    }

    if ( layer->isValid() )
    {
      // add to list of layers to register
      //with the central layers registry
      myList << layer;
    }
    else
    {
      QgsMessageLog::logMessage( QObject::tr( "%1 is an invalid layer - not loaded" ).arg( layerPath ) );
      QLabel *msgLabel = new QLabel( QObject::tr( "%1 is an invalid layer and cannot be loaded. Please check the <a href=\"#messageLog\">message log</a> for further info." ).arg( layerPath ), QgisApp::instance()->messageBar() );
      msgLabel->setWordWrap( true );
      QObject::connect( msgLabel, &QLabel::linkActivated, QgisApp::instance()->logDock(), &QWidget::show );
      QgsMessageBarItem *item = new QgsMessageBarItem( msgLabel, Qgis::MessageLevel::Warning );
      QgisApp::instance()->messageBar()->pushItem( item );
      delete layer;
    }
    //qWarning("incrementing iterator");
  }

  QgsProject::instance()->addMapLayers( myList );

  // load default style after adding to process readCustomSymbology signals
  const auto constMyList = myList;
  for ( QgsMapLayer *l : constMyList )
  {
    bool ok;
    l->loadDefaultStyle( ok );
    l->loadDefaultMetadata( ok );
  }

  QApplication::restoreOverrideCursor();

  ok = true;
  return myList;
}

template<typename T>
T *QgsAppLayerHandling::addLayerPrivate( QgsMapLayerType type, const QString &uri, const QString &name, const QString &providerKey, bool guiWarnings )
{
  QgsSettings settings;

  QgsCanvasRefreshBlocker refreshBlocker;

  QString baseName = settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool() ? QgsMapLayer::formatLayerName( name ) : name;

  // if the layer needs authentication, ensure the master password is set
  const thread_local QRegularExpression rx( "authcfg=([a-z]|[A-Z]|[0-9]){7}" );
  if ( rx.match( uri ).hasMatch() )
  {
    if ( !QgsAuthGuiUtils::isDisabled( QgisApp::instance()->messageBar() ) )
    {
      QgsApplication::authManager()->setMasterPassword( true );
    }
  }

  QVariantMap uriElements = QgsProviderRegistry::instance()->decodeUri( providerKey, uri );
  QString path = uri;
  if ( uriElements.contains( QStringLiteral( "path" ) ) )
  {
    // run layer path through QgsPathResolver so that all inbuilt paths and other localised paths are correctly expanded
    path = QgsPathResolver().readPath( uriElements.value( QStringLiteral( "path" ) ).toString() );
    uriElements[ QStringLiteral( "path" ) ] = path;
  }
  // Not all providers implement decodeUri(), so use original uri if uriElements is empty
  const QString updatedUri = uriElements.isEmpty() ? uri : QgsProviderRegistry::instance()->encodeUri( providerKey, uriElements );

  const bool canQuerySublayers = QgsProviderRegistry::instance()->providerMetadata( providerKey ) &&
                                 ( QgsProviderRegistry::instance()->providerMetadata( providerKey )->capabilities() & QgsProviderMetadata::QuerySublayers );

  T *result = nullptr;
  if ( canQuerySublayers )
  {
    // query sublayers
    QList< QgsProviderSublayerDetails > sublayers = QgsProviderRegistry::instance()->providerMetadata( providerKey ) ?
        QgsProviderRegistry::instance()->providerMetadata( providerKey )->querySublayers( updatedUri, Qgis::SublayerQueryFlag::IncludeSystemTables )
        : QgsProviderRegistry::instance()->querySublayers( updatedUri );

    // filter out non-matching sublayers
    sublayers.erase( std::remove_if( sublayers.begin(), sublayers.end(), [type]( const QgsProviderSublayerDetails & sublayer )
    {
      return sublayer.type() != type;
    } ), sublayers.end() );

    if ( sublayers.empty() )
    {
      if ( guiWarnings )
      {
        QString msg = QObject::tr( "%1 is not a valid or recognized data source." ).arg( uri );
        QgisApp::instance()->visibleMessageBar()->pushMessage( QObject::tr( "Invalid Data Source" ), msg, Qgis::MessageLevel::Critical );
      }

      // since the layer is bad, stomp on it
      return nullptr;
    }
    else if ( sublayers.size() > 1 || QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount ) )
    {
      // ask user for sublayers (unless user settings dictate otherwise!)
      switch ( shouldAskUserForSublayers( sublayers ) )
      {
        case SublayerHandling::AskUser:
        {
          QgsProviderSublayersDialog dlg( updatedUri, path, sublayers, {type}, QgisApp::instance() );
          if ( dlg.exec() )
          {
            const QList< QgsProviderSublayerDetails > selectedLayers = dlg.selectedLayers();
            if ( !selectedLayers.isEmpty() )
            {
              result = qobject_cast< T * >( addSublayers( selectedLayers, baseName, dlg.groupName() ).value( 0 ) );
            }
          }
          break;
        }
        case SublayerHandling::LoadAll:
        {
          result = qobject_cast< T * >( addSublayers( sublayers, baseName, QString() ).value( 0 ) );
          break;
        }
        case SublayerHandling::AbortLoading:
          break;
      };
    }
    else
    {
      result = qobject_cast< T * >( addSublayers( sublayers, name, QString() ).value( 0 ) );

      if ( result )
      {
        QString base( baseName );
        if ( settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool() )
        {
          base = QgsMapLayer::formatLayerName( base );
        }
        result->setName( base );
      }
    }
  }
  else
  {
    QgsMapLayerFactory::LayerOptions options( QgsProject::instance()->transformContext() );
    options.loadDefaultStyle = false;
    result = qobject_cast< T * >( QgsMapLayerFactory::createLayer( uri, name, type, options, providerKey ) );
    if ( result )
    {
      QString base( baseName );
      if ( settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool() )
      {
        base = QgsMapLayer::formatLayerName( base );
      }
      result->setName( base );
      QgsProject::instance()->addMapLayer( result );

      QgisApp::instance()->askUserForDatumTransform( result->crs(), QgsProject::instance()->crs(), result );
      QgsAppLayerHandling::postProcessAddedLayer( result );
    }
  }

  QgisApp::instance()->activateDeactivateLayerRelatedActions( QgisApp::instance()->activeLayer() );
  return result;
}

const QList<QgsVectorLayerRef> QgsAppLayerHandling::findBrokenLayerDependencies( QgsVectorLayer *vl, QgsMapLayer::StyleCategories categories )
{
  QList<QgsVectorLayerRef> brokenDependencies;

  if ( categories.testFlag( QgsMapLayer::StyleCategory::Forms ) )
  {
    for ( int i = 0; i < vl->fields().count(); i++ )
    {
      const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( vl, vl->fields().field( i ).name() );
      QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
      if ( fieldFormatter )
      {
        const QList<QgsVectorLayerRef> constDependencies { fieldFormatter->layerDependencies( setup.config() ) };
        for ( const QgsVectorLayerRef &dependency : constDependencies )
        {
          // I guess we need and isNull()/isValid() method for the ref
          if ( dependency.layer ||
               ! dependency.name.isEmpty() ||
               ! dependency.source.isEmpty() ||
               ! dependency.layerId.isEmpty() )
          {
            const QgsVectorLayer *depVl { QgsVectorLayerRef( dependency ).resolveWeakly( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) };
            if ( ! depVl || ! depVl->isValid() )
            {
              brokenDependencies.append( dependency );
            }
          }
        }
      }
    }
  }

  if ( categories.testFlag( QgsMapLayer::StyleCategory::Relations ) )
  {
    // Check for layer weak relations
    const QList<QgsWeakRelation> constWeakRelations { vl->weakRelations() };
    for ( const QgsWeakRelation &rel : constWeakRelations )
    {
      const QList< QgsRelation > relations { rel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) };
      for ( const QgsRelation &relation : relations )
      {
        QgsVectorLayerRef dependency;
        bool found = false;
        if ( ! relation.isValid() )
        {
          // This is the big question: do we really
          // want to automatically load the referencing layer(s) too?
          // This could potentially lead to a cascaded load of a
          // long list of layers.
          // The code is in place but let's leave it disabled for now.
          if ( relation.referencedLayer() == vl )
          {
            // Do nothing because vl is the referenced layer
#if 0
            dependency = rel.referencingLayer();
            found = true;
#endif
          }
          else if ( relation.referencingLayer() == vl )
          {
            dependency = rel.referencedLayer();
            found = true;
          }
          else
          {
            // Something wrong is going on here, maybe this relation
            // does not really apply to this layer?
            QgsMessageLog::logMessage( QObject::tr( "None of the layers in the relation stored in the style match the current layer, skipping relation id: %1." ).arg( relation.id() ) );
          }

          if ( found )
          {
            // Make sure we don't add it twice if it was already added by the form widgets check
            bool refFound = false;
            for ( const QgsVectorLayerRef &otherRef : std::as_const( brokenDependencies ) )
            {
              if ( dependency.layerId == otherRef.layerId || ( dependency.source == otherRef.source && dependency.provider == otherRef.provider ) )
              {
                refFound = true;
                break;
              }
            }
            if ( ! refFound )
            {
              brokenDependencies.append( dependency );
            }
          }
        }
      }
    }
  }
  return brokenDependencies;
}

void QgsAppLayerHandling::resolveVectorLayerDependencies( QgsVectorLayer *vl, QgsMapLayer::StyleCategories categories )
{
  if ( vl && vl->isValid() )
  {
    const auto constDependencies { findBrokenLayerDependencies( vl, categories ) };
    for ( const QgsVectorLayerRef &dependency : constDependencies )
    {
      // Check for projects without layer dependencies (see 7e8c7b3d0e094737336ff4834ea2af625d2921bf)
      if ( QgsProject::instance()->mapLayer( dependency.layerId ) || ( dependency.name.isEmpty() && dependency.source.isEmpty() ) )
      {
        continue;
      }
      // try to aggressively resolve the broken dependencies
      bool loaded = false;
      const QString providerName { vl->dataProvider()->name() };
      QgsProviderMetadata *providerMetadata { QgsProviderRegistry::instance()->providerMetadata( providerName ) };
      if ( providerMetadata )
      {
        // Retrieve the DB connection (if any)

        std::unique_ptr< QgsAbstractDatabaseProviderConnection > conn { QgsMapLayerUtils::databaseConnection( vl ) };
        if ( conn )
        {
          QString tableSchema;
          QString tableName;
          const QVariantMap sourceParts = providerMetadata->decodeUri( dependency.source );

          // This part should really be abstracted out to the connection classes or to the providers directly.
          // Different providers decode the uri differently, for example we don't get the table name out of OGR
          // but the layerName/layerId instead, so let's try different approaches

          // This works for GPKG
          tableName = sourceParts.value( QStringLiteral( "layerName" ) ).toString();

          // This works for PG and spatialite
          if ( tableName.isEmpty() )
          {
            tableName = sourceParts.value( QStringLiteral( "table" ) ).toString();
            tableSchema = sourceParts.value( QStringLiteral( "schema" ) ).toString();
          }

          // Helper to find layers in connections
          auto layerFinder = [ &conn, &dependency, &providerName ]( const QString & tableSchema, const QString & tableName ) -> bool
          {
            // First try the current schema (or no schema if it's not supported from the provider)
            try
            {
              const QString layerUri { conn->tableUri( tableSchema, tableName )};
              // Load it!
              std::unique_ptr< QgsVectorLayer > newVl = std::make_unique< QgsVectorLayer >( layerUri, dependency.name, providerName );
              if ( newVl->isValid() )
              {
                QgsProject::instance()->addMapLayer( newVl.release() );
                return true;
              }
            }
            catch ( QgsProviderConnectionException & )
            {
              // Do nothing!
            }
            return false;
          };

          loaded = layerFinder( tableSchema, tableName );

          // Try different schemas
          if ( ! loaded && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::Schemas ) && ! tableSchema.isEmpty() )
          {
            const QStringList schemas { conn->schemas() };
            for ( const QString &schemaName : schemas )
            {
              if ( schemaName != tableSchema )
              {
                loaded = layerFinder( schemaName, tableName );
              }
              if ( loaded )
              {
                break;
              }
            }
          }
        }
      }
      if ( ! loaded )
      {
        const QString msg { QObject::tr( "layer '%1' requires layer '%2' to be loaded but '%2' could not be found, please load it manually if possible." ).arg( vl->name(), dependency.name ) };
        QgisApp::instance()->messageBar()->pushWarning( QObject::tr( "Missing layer form dependency" ), msg );
      }
      else
      {
        QgisApp::instance()->messageBar()->pushSuccess( QObject::tr( "Missing layer form dependency" ), QObject::tr( "Layer dependency '%2' required by '%1' was automatically loaded." )
            .arg( vl->name(),
                  dependency.name ) );
      }
    }
  }
}

void QgsAppLayerHandling::resolveVectorLayerWeakRelations( QgsVectorLayer *vectorLayer )
{
  if ( vectorLayer && vectorLayer->isValid() )
  {
    const QList<QgsWeakRelation> constWeakRelations { vectorLayer->weakRelations( ) };
    for ( const QgsWeakRelation &rel : constWeakRelations )
    {
      const QList< QgsRelation > relations { rel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) };
      for ( const QgsRelation &relation : relations )
      {
        if ( relation.isValid() )
        {
          // Avoid duplicates
          const QList<QgsRelation> constRelations { QgsProject::instance()->relationManager()->relations().values() };
          for ( const QgsRelation &other : constRelations )
          {
            if ( relation.hasEqualDefinition( other ) )
            {
              continue;
            }
          }
          QgsProject::instance()->relationManager()->addRelation( relation );
        }
      }
    }
  }
}

void QgsAppLayerHandling::onVectorLayerStyleLoaded( QgsVectorLayer *vl, QgsMapLayer::StyleCategories categories )
{
  if ( vl && vl->isValid( ) )
  {

    // Check broken dependencies in forms
    if ( categories.testFlag( QgsMapLayer::StyleCategory::Forms ) )
    {
      resolveVectorLayerDependencies( vl );
    }

    // Check broken relations and try to restore them
    if ( categories.testFlag( QgsMapLayer::StyleCategory::Relations ) )
    {
      resolveVectorLayerWeakRelations( vl );
    }
  }
}

