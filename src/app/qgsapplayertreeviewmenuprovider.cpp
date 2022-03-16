/***************************************************************************
    qgsapplayertreeviewmenuprovider.cpp
    ---------------------
    begin                : May 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QClipboard>
#include <QMessageBox>

#include "qgisapp.h"
#include "qgsapplayertreeviewmenuprovider.h"
#include "qgsapplication.h"
#include "qgsclipboard.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorswatchgrid.h"
#include "qgscolorwidgets.h"
#include "qgsdialog.h"
#include "qgsgui.h"
#include "qgslayernotesmanager.h"
#include "qgslayernotesutils.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeregistrybridge.h"
#include "qgslayertreeviewdefaultactions.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerstylecategoriesmodel.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsmaplayerutils.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmessagebar.h"
#include "qgspointcloudlayer.h"
#include "qgsproject.h"
#include "qgsqueryresultwidget.h"
#include "qgsrasterlayer.h"
#include "qgsrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgssymbolselectordialog.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsxmlutils.h"
#include "qgsmeshlayer.h"


QgsAppLayerTreeViewMenuProvider::QgsAppLayerTreeViewMenuProvider( QgsLayerTreeView *view, QgsMapCanvas *canvas )
  : mView( view )
  , mCanvas( canvas )
{
}

QMenu *QgsAppLayerTreeViewMenuProvider::createContextMenu()
{
  QMenu *menu = new QMenu;

  QgsLayerTreeViewDefaultActions *actions = mView->defaultActions();

  const QModelIndex idx = mView->currentIndex();
  if ( !idx.isValid() )
  {
    // global menu
    menu->addAction( actions->actionAddGroup( menu ) );
    menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionExpandTree.svg" ) ), tr( "&Expand All" ), mView, &QgsLayerTreeView::expandAll );
    menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCollapseTree.svg" ) ), tr( "&Collapse All" ), mView, &QgsLayerTreeView::collapseAll );
    menu->addSeparator();
    if ( QgisApp::instance()->clipboard()->hasFormat( QGSCLIPBOARD_MAPLAYER_MIME ) )
    {
      QAction *actionPasteLayerOrGroup = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditPaste.svg" ) ), tr( "Paste Layer/Group" ), menu );
      connect( actionPasteLayerOrGroup, &QAction::triggered, QgisApp::instance(), &QgisApp::pasteLayer );
      menu->addAction( actionPasteLayerOrGroup );
    }

    // TODO: update drawing order
  }
  else if ( QgsLayerTreeNode *node = mView->index2node( idx ) )
  {
    // layer or group selected
    if ( QgsLayerTree::isGroup( node ) )
    {
      menu->addAction( actions->actionZoomToGroup( mCanvas, menu ) );

      menu->addAction( tr( "Co&py Group" ), QgisApp::instance(), &QgisApp::copyLayer );
      if ( QgisApp::instance()->clipboard()->hasFormat( QGSCLIPBOARD_MAPLAYER_MIME ) )
      {
        QAction *actionPasteLayerOrGroup = new QAction( tr( "Paste Layer/Group" ), menu );
        connect( actionPasteLayerOrGroup, &QAction::triggered, QgisApp::instance(), &QgisApp::pasteLayer );
        menu->addAction( actionPasteLayerOrGroup );
      }

      menu->addAction( actions->actionRenameGroupOrLayer( menu ) );

      menu->addSeparator();
      menu->addAction( actions->actionAddGroup( menu ) );
      QAction *removeAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemoveLayer.svg" ) ), tr( "&Remove Group…" ), QgisApp::instance(), &QgisApp::removeLayer );
      removeAction->setEnabled( removeActionEnabled() );
      menu->addSeparator();

      menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSetCRS.png" ) ),
                       tr( "Set Group &CRS…" ), QgisApp::instance(), &QgisApp::legendGroupSetCrs );
      menu->addAction( tr( "Set Group &WMS Data…" ), QgisApp::instance(), &QgisApp::legendGroupSetWmsData );

      menu->addSeparator();

      menu->addAction( actions->actionMutuallyExclusiveGroup( menu ) );

      if ( QAction *checkAll = actions->actionCheckAndAllChildren( menu ) )
        menu->addAction( checkAll );

      if ( QAction *unCheckAll = actions->actionUncheckAndAllChildren( menu ) )
        menu->addAction( unCheckAll );

      if ( !( mView->selectedNodes( true ).count() == 1 && idx.row() == 0 ) )
      {
        menu->addAction( actions->actionMoveToTop( menu ) );
      }

      if ( !( mView->selectedNodes( true ).count() == 1 && idx.row() == idx.model()->rowCount() - 1 ) )
      {
        menu->addAction( actions->actionMoveToBottom( menu ) );
      }

      menu->addSeparator();

      if ( !mView->selectedNodes( true ).empty() )
        menu->addAction( actions->actionGroupSelected( menu ) );

      if ( QgisApp::instance()->clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
      {
        menu->addAction( tr( "Paste Style" ), QgisApp::instance(), &QgisApp::applyStyleToGroup );
      }

      menu->addSeparator();

      QMenu *menuExportGroup = new QMenu( tr( "E&xport" ), menu );
      QAction *actionSaveAsDefinitionGroup = new QAction( tr( "Save as Layer &Definition File…" ), menuExportGroup );
      connect( actionSaveAsDefinitionGroup, &QAction::triggered, QgisApp::instance(), &QgisApp::saveAsLayerDefinition );
      menuExportGroup->addAction( actionSaveAsDefinitionGroup );

      menu->addMenu( menuExportGroup );
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      QgsPointCloudLayer *pcLayer = qobject_cast<QgsPointCloudLayer * >( layer );
      QgsMeshLayer *meshLayer = qobject_cast<QgsMeshLayer * >( layer );

      if ( layer && layer->isSpatial() )
      {

        QAction *zoomToLayers = actions->actionZoomToLayers( mCanvas, menu );
        zoomToLayers->setEnabled( layer->isValid() );
        menu->addAction( zoomToLayers );
        if ( vlayer )
        {
          const QList<QgsMapLayer *> selectedLayers = mView->selectedLayers();
          bool hasSelectedFeature = false;
          for ( const QgsMapLayer *layer : selectedLayers )
          {

            if ( const QgsVectorLayer *vLayer = qobject_cast<const QgsVectorLayer *>( layer ) )
            {

              if ( vLayer->selectedFeatureCount() > 0 )
              {
                hasSelectedFeature = true;
                break;
              }
            }

          }
          QAction *actionZoomSelected = actions->actionZoomToSelection( mCanvas, menu );
          actionZoomSelected->setEnabled( vlayer->isValid() && hasSelectedFeature );
          menu->addAction( actionZoomSelected );
        }
        menu->addAction( actions->actionShowInOverview( menu ) );
      }

      if ( vlayer )
      {
        QAction *showFeatureCount = actions->actionShowFeatureCount( menu );
        menu->addAction( showFeatureCount );
        showFeatureCount->setEnabled( vlayer->isValid() );

        const QString iconName = vlayer && vlayer->labeling() && vlayer->labeling()->type() == QLatin1String( "rule-based" )
                                 ? QStringLiteral( "labelingRuleBased.svg" )
                                 : QStringLiteral( "labelingSingle.svg" );
        QAction *actionShowLabels = new QAction( QgsApplication::getThemeIcon( iconName ), tr( "Show &Labels" ), menu );
        actionShowLabels->setCheckable( true );
        actionShowLabels->setChecked( vlayer->labelsEnabled() );
        connect( actionShowLabels, &QAction::toggled, this, &QgsAppLayerTreeViewMenuProvider::toggleLabels );
        menu->addAction( actionShowLabels );
      }

      QAction *actionCopyLayer = new QAction( tr( "Copy Layer" ), menu );
      connect( actionCopyLayer, &QAction::triggered, QgisApp::instance(), &QgisApp::copyLayer );
      menu->addAction( actionCopyLayer );

      menu->addAction( actions->actionRenameGroupOrLayer( menu ) );

      if ( rlayer )
      {
        QAction *zoomToNative = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomActual.svg" ) ), tr( "Zoom to Nat&ive Resolution (100%)" ), QgisApp::instance(), &QgisApp::legendLayerZoomNative );
        zoomToNative->setEnabled( rlayer->isValid() );

        if ( rlayer->rasterType() != QgsRasterLayer::Palette )
        {
          QAction *stretch = menu->addAction( tr( "&Stretch Using Current Extent" ), QgisApp::instance(), &QgisApp::legendLayerStretchUsingCurrentExtent );
          stretch->setEnabled( rlayer->isValid() );
        }
      }

      // No raster support in createSqlVectorLayer (yet)
      if ( vlayer && vlayer->isSqlQuery() )
      {
        const std::unique_ptr< QgsAbstractDatabaseProviderConnection> conn { QgsMapLayerUtils::databaseConnection( layer ) };
        if ( conn )
          menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/dbmanager.svg" ) ), tr( "Update SQL Layer…" ), menu, [ layer, this ]
        {
          std::unique_ptr< QgsAbstractDatabaseProviderConnection> conn2 { QgsMapLayerUtils::databaseConnection( layer ) };
          if ( conn2 )
          {
            QgsDialog dialog;
            dialog.setObjectName( QStringLiteral( "SqlUpdateDialog" ) );
            dialog.setWindowTitle( tr( "%1 — Update SQL" ).arg( layer->name() ) );
            QgsGui::enableAutoGeometryRestore( &dialog );
            QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions options { conn2->sqlOptions( layer->source() ) };
            options.layerName = layer->name();
            QgsQueryResultWidget *queryResultWidget { new QgsQueryResultWidget( &dialog, conn2.release() ) };
            queryResultWidget->setWidgetMode( QgsQueryResultWidget::QueryWidgetMode::QueryLayerUpdateMode );
            queryResultWidget->setSqlVectorLayerOptions( options );
            queryResultWidget->executeQuery();
            queryResultWidget->layout()->setMargin( 0 );
            dialog.layout()->addWidget( queryResultWidget );

            connect( queryResultWidget, &QgsQueryResultWidget::createSqlVectorLayer, queryResultWidget, [queryResultWidget, layer, this ]( const QString &, const QString &, const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions & options )
            {
              ( void )this;
              std::unique_ptr< QgsAbstractDatabaseProviderConnection> conn3 { QgsMapLayerUtils::databaseConnection( layer ) };
              if ( conn3 )
              {
                try
                {
                  std::unique_ptr<QgsMapLayer> sqlLayer { conn3->createSqlVectorLayer( options ) };
                  if ( sqlLayer->isValid() )
                  {
                    layer->setDataSource( sqlLayer->source(), sqlLayer->name(), sqlLayer->dataProvider()->name(), QgsDataProvider::ProviderOptions() );
                    queryResultWidget->notify( QObject::tr( "Layer Update Success" ), QObject::tr( "The SQL layer was updated successfully" ), Qgis::MessageLevel::Success );
                  }
                  else
                  {
                    QString error { sqlLayer->dataProvider()->error().message( QgsErrorMessage::Format::Text ) };
                    if ( error.isEmpty() )
                    {
                      error = QObject::tr( "layer is not valid, check the log messages for more information" );
                    }
                    queryResultWidget->notify( QObject::tr( "Layer Update Error" ), QObject::tr( "Error updating the SQL layer: %1" ).arg( error ), Qgis::MessageLevel::Critical );
                  }
                }
                catch ( QgsProviderConnectionException &ex )
                {
                  queryResultWidget->notify( QObject::tr( "Layer Update Error" ), QObject::tr( "Error updating the SQL layer: %1" ).arg( ex.what() ), Qgis::MessageLevel::Critical );
                }
              }

            } );

            dialog.exec();

          }
        } );
      }

      addCustomLayerActions( menu, layer );
      if ( layer && layer->type() == QgsMapLayerType::VectorLayer && static_cast<QgsVectorLayer *>( layer )->providerType() == QLatin1String( "virtual" ) )
      {
        menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddVirtualLayer.svg" ) ), tr( "Edit Virtual Layer…" ), QgisApp::instance(), &QgisApp::addVirtualLayer );
      }

      menu->addSeparator();

      // duplicate layer
      QAction *duplicateLayersAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDuplicateLayer.svg" ) ), tr( "&Duplicate Layer" ), QgisApp::instance(), [] { QgisApp::instance()->duplicateLayers(); } );
      QAction *removeAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemoveLayer.svg" ) ), tr( "&Remove Layer…" ), QgisApp::instance(), &QgisApp::removeLayer );
      removeAction->setEnabled( removeActionEnabled() );

      menu->addSeparator();

      if ( node->parent() != mView->layerTreeModel()->rootGroup() )
        menu->addAction( actions->actionMoveOutOfGroup( menu ) );

      if ( !( mView->selectedNodes( true ).count() == 1 && idx.row() == 0 ) )
      {
        menu->addAction( actions->actionMoveToTop( menu ) );
      }

      if ( !( mView->selectedNodes( true ).count() == 1 && idx.row() == idx.model()->rowCount() - 1 ) )
      {
        menu->addAction( actions->actionMoveToBottom( menu ) );
      }

      QAction *checkAll = actions->actionCheckAndAllParents( menu );
      if ( checkAll )
        menu->addAction( checkAll );

      if ( mView->selectedNodes( true ).count() >= 2 )
        menu->addAction( actions->actionGroupSelected( menu ) );

      menu->addSeparator();

      if ( vlayer )
      {
        QAction *toggleEditingAction = QgisApp::instance()->actionToggleEditing();
        QAction *saveLayerEditsAction = QgisApp::instance()->actionSaveActiveLayerEdits();
        QAction *allEditsAction = QgisApp::instance()->actionAllEdits();

        // attribute table
        QgsSettings settings;
        const QgsAttributeTableFilterModel::FilterMode initialMode = settings.enumValue( QStringLiteral( "qgis/attributeTableBehavior" ),  QgsAttributeTableFilterModel::ShowAll );
        const auto lambdaOpenAttributeTable = [ = ] { QgisApp::instance()->attributeTable( initialMode ); };
        QAction *attributeTableAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ), tr( "Open &Attribute Table" ),
                                        QgisApp::instance(), lambdaOpenAttributeTable );
        attributeTableAction->setEnabled( vlayer->isValid() );

        // allow editing
        const QgsVectorDataProvider *provider = vlayer->dataProvider();
        if ( vlayer->supportsEditing() )
        {
          if ( toggleEditingAction )
          {
            menu->addAction( toggleEditingAction );
            toggleEditingAction->setChecked( vlayer->isEditable() );
            toggleEditingAction->setEnabled( vlayer->isValid() );
          }
          if ( saveLayerEditsAction && vlayer->isModified() )
          {
            menu->addAction( saveLayerEditsAction );
          }
        }

        if ( allEditsAction->isEnabled() )
          menu->addAction( allEditsAction );

        if ( provider && provider->supportsSubsetString() )
        {
          QAction *action = menu->addAction( tr( "&Filter…" ), QgisApp::instance(), qOverload<>( &QgisApp::layerSubsetString ) );
          action->setEnabled( !vlayer->isEditable() );
        }
      }

      if ( rlayer &&
           rlayer->dataProvider() &&
           rlayer->dataProvider()->supportsSubsetString() )
      {
        menu->addAction( tr( "&Filter…" ), QgisApp::instance(), qOverload<>( &QgisApp::layerSubsetString ) );
      }

      // change data source is only supported for vectors, rasters, point clouds, mesh
      if ( vlayer || rlayer || pcLayer || meshLayer )
      {

        QAction *a = new QAction( layer->isValid() ? tr( "C&hange Data Source…" ) : tr( "Repair Data Source…" ), menu );
        if ( !layer->isValid() )
          a->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconWarning.svg" ) ) );
        // Disable when layer is editable
        if ( layer->isEditable() )
        {
          a->setEnabled( false );
        }
        else
        {
          connect( a, &QAction::triggered, this, [ = ]
          {
            QgisApp::instance()->changeDataSource( layer );
          } );
        }
        menu->addAction( a );
      }

      // actions on the selection
      if ( vlayer && vlayer->selectedFeatureCount() > 0 )
      {
        const int selectionCount = vlayer->selectedFeatureCount();
        QgsMapLayerAction::Target target;
        if ( selectionCount == 1 )
          target = QgsMapLayerAction::Target::SingleFeature;
        else
          target = QgsMapLayerAction::Target::MultipleFeatures;

        const QList<QgsMapLayerAction *> constRegisteredActions = QgsGui::mapLayerActionRegistry()->mapLayerActions( vlayer, target );
        if ( !constRegisteredActions.isEmpty() )
        {
          QMenu *actionMenu = menu->addMenu( tr( "Actions on Selection (%1)" ).arg( selectionCount ) );
          for ( QgsMapLayerAction *action : constRegisteredActions )
          {
            if ( target == QgsMapLayerAction::Target::SingleFeature )
            {
              actionMenu->addAction( action->text(), action, [ = ]() { action->triggerForFeature( vlayer,  vlayer->selectedFeatures().at( 0 ) ); } );
            }
            else if ( target == QgsMapLayerAction::Target::MultipleFeatures )
            {
              actionMenu->addAction( action->text(), action, [ = ]() {action->triggerForFeatures( vlayer, vlayer->selectedFeatures() );} );
            }
          }
        }
      }

      menu->addSeparator();

      if ( layer && layer->isSpatial() )
      {
        // set layer scale visibility
        menu->addAction( tr( "Set Layer Scale &Visibility…" ), QgisApp::instance(), &QgisApp::setLayerScaleVisibility );

        if ( !layer->isInScaleRange( mCanvas->scale() ) )
          menu->addAction( tr( "Zoom to &Visible Scale" ), QgisApp::instance(), &QgisApp::zoomToLayerScale );

        QMenu *menuSetCRS = new QMenu( tr( "Layer CRS" ), menu );

        const QList<QgsLayerTreeNode *> selectedNodes = mView->selectedNodes();
        QgsCoordinateReferenceSystem layerCrs;
        bool firstLayer = true;
        bool allSameCrs = true;
        for ( QgsLayerTreeNode *node : selectedNodes )
        {
          if ( QgsLayerTree::isLayer( node ) )
          {
            QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
            if ( nodeLayer->layer() )
            {
              if ( firstLayer )
              {
                layerCrs = nodeLayer->layer()->crs();
                firstLayer = false;
              }
              else if ( nodeLayer->layer()->crs() != layerCrs )
              {
                allSameCrs = false;
                break;
              }
            }
          }
        }

        QAction *actionCurrentCrs = new QAction( !allSameCrs ? tr( "Mixed CRS" )
            : layer->crs().isValid() ? layer->crs().userFriendlyIdentifier()
            : tr( "No CRS" ), menuSetCRS );
        actionCurrentCrs->setEnabled( false );
        menuSetCRS->addAction( actionCurrentCrs );

        if ( allSameCrs && layerCrs.isValid() )
        {
          // assign layer crs to project
          QAction *actionSetProjectCrs = new QAction( tr( "Set &Project CRS from Layer" ), menuSetCRS );
          connect( actionSetProjectCrs, &QAction::triggered, QgisApp::instance(), &QgisApp::setProjectCrsFromLayer );
          menuSetCRS->addAction( actionSetProjectCrs );
        }

        const QList< QgsCoordinateReferenceSystem> recentProjections = QgsCoordinateReferenceSystem::recentCoordinateReferenceSystems();
        if ( !recentProjections.isEmpty() )
        {
          menuSetCRS->addSeparator();
          int i = 0;
          for ( const QgsCoordinateReferenceSystem &crs : recentProjections )
          {
            if ( crs == layer->crs() )
              continue;

            QAction *action = menuSetCRS->addAction( tr( "Set to %1" ).arg( crs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ) ) );
            connect( action, &QAction::triggered, this, [ = ]
            {
              setLayerCrs( crs );
            } );

            i++;
            if ( i == 5 )
              break;
          }
        }

        // set layer crs
        menuSetCRS->addSeparator();
        QAction *actionSetLayerCrs = new QAction( tr( "Set &Layer CRS…" ), menuSetCRS );
        connect( actionSetLayerCrs, &QAction::triggered, QgisApp::instance(), &QgisApp::setLayerCrs );
        menuSetCRS->addAction( actionSetLayerCrs );

        menu->addMenu( menuSetCRS );
      }

      menu->addSeparator();

      // export menu
      if ( layer )
      {
        switch ( layer->type() )
        {
          case QgsMapLayerType::VectorLayer:
            if ( vlayer )
            {
              if ( vlayer->isTemporary() )
              {
                QAction *actionMakePermanent = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionFileSave.svg" ) ), tr( "Make Permanent…" ), menu );
                connect( actionMakePermanent, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->makeMemoryLayerPermanent( vlayer ); } );
                menu->addAction( actionMakePermanent );
              }
              // save as vector file
              QMenu *menuExportVector = new QMenu( tr( "E&xport" ), menu );
              QAction *actionSaveAs = new QAction( tr( "Save Features &As…" ), menuExportVector );
              connect( actionSaveAs, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveAsFile(); } );
              actionSaveAs->setEnabled( vlayer->isValid() );
              menuExportVector->addAction( actionSaveAs );
              QAction *actionSaveSelectedFeaturesAs = new QAction( tr( "Save &Selected Features As…" ), menuExportVector );
              connect( actionSaveSelectedFeaturesAs, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveAsFile( nullptr, true ); } );
              actionSaveSelectedFeaturesAs->setEnabled( vlayer->isValid() && vlayer->selectedFeatureCount() > 0 );
              menuExportVector->addAction( actionSaveSelectedFeaturesAs );
              QAction *actionSaveAsDefinitionLayer = new QAction( tr( "Save as Layer &Definition File…" ), menuExportVector );
              connect( actionSaveAsDefinitionLayer, &QAction::triggered, QgisApp::instance(), &QgisApp::saveAsLayerDefinition );
              menuExportVector->addAction( actionSaveAsDefinitionLayer );
              if ( vlayer->isSpatial() )
              {
                QAction *actionSaveStyle = new QAction( tr( "Save as &QGIS Layer Style File…" ), menuExportVector );
                connect( actionSaveStyle, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveStyleFile(); } );
                menuExportVector->addAction( actionSaveStyle );
              }
              menu->addMenu( menuExportVector );
            }
            break;

          case QgsMapLayerType::RasterLayer:
            if ( rlayer )
            {
              QMenu *menuExportRaster = new QMenu( tr( "E&xport" ), menu );
              QAction *actionSaveAs = new QAction( tr( "Save &As…" ), menuExportRaster );
              QAction *actionSaveAsDefinitionLayer = new QAction( tr( "Save as Layer &Definition File…" ), menuExportRaster );
              QAction *actionSaveStyle = new QAction( tr( "Save as &QGIS Layer Style File…" ), menuExportRaster );
              connect( actionSaveAs, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveAsFile(); } );
              menuExportRaster->addAction( actionSaveAs );
              actionSaveAs->setEnabled( rlayer->isValid() );
              connect( actionSaveAsDefinitionLayer, &QAction::triggered, QgisApp::instance(), &QgisApp::saveAsLayerDefinition );
              menuExportRaster->addAction( actionSaveAsDefinitionLayer );
              connect( actionSaveStyle, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveStyleFile(); } );
              menuExportRaster->addAction( actionSaveStyle );
              menu->addMenu( menuExportRaster );
            }
            break;

          case QgsMapLayerType::MeshLayer:
          case QgsMapLayerType::VectorTileLayer:
          case QgsMapLayerType::PointCloudLayer:
          {
            QMenu *menuExportRaster = new QMenu( tr( "E&xport" ), menu );
            QAction *actionSaveAsDefinitionLayer = new QAction( tr( "Save as Layer &Definition File…" ), menuExportRaster );
            QAction *actionSaveStyle = new QAction( tr( "Save as &QGIS Layer Style File…" ), menuExportRaster );
            connect( actionSaveAsDefinitionLayer, &QAction::triggered, QgisApp::instance(), &QgisApp::saveAsLayerDefinition );
            menuExportRaster->addAction( actionSaveAsDefinitionLayer );
            connect( actionSaveStyle, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveStyleFile(); } );
            menuExportRaster->addAction( actionSaveStyle );
            menu->addMenu( menuExportRaster );
          }
          break;

          case QgsMapLayerType::AnnotationLayer:
          case QgsMapLayerType::GroupLayer:
            break;

          case QgsMapLayerType::PluginLayer:
            if ( mView->selectedLayerNodes().count() == 1 )
            {
              // disable duplication of plugin layers
              duplicateLayersAction->setEnabled( false );
            }
            break;

        }
        menu->addSeparator();
      }

      // style-related actions
      if ( layer && mView->selectedLayerNodes().count() == 1 )
      {
        menu->addSeparator();
        QMenu *menuStyleManager = new QMenu( tr( "Styles" ), menu );
        QgsMapLayerStyleManager *mgr = layer->styleManager();
        if ( mgr->styles().count() > 1 )
        {
          menuStyleManager->setTitle( tr( "Styles (%1)" ).arg( mgr->styles().count() ) );
        }

        QgisApp *app = QgisApp::instance();
        if ( layer->type() == QgsMapLayerType::VectorLayer )
        {
          QMenu *copyStyleMenu = menuStyleManager->addMenu( tr( "Copy Style" ) );
          copyStyleMenu->setToolTipsVisible( true );
          QgsMapLayerStyleCategoriesModel *model = new QgsMapLayerStyleCategoriesModel( layer->type(), copyStyleMenu );
          model->setShowAllCategories( true );
          for ( int row = 0; row < model->rowCount(); ++row )
          {
            const QModelIndex index = model->index( row, 0 );
            const QgsMapLayer::StyleCategory category = model->data( index, Qt::UserRole ).value<QgsMapLayer::StyleCategory>();
            const QString name = model->data( index, Qt::DisplayRole ).toString();
            const QString tooltip = model->data( index, Qt::ToolTipRole ).toString();
            const QIcon icon = model->data( index, Qt::DecorationRole ).value<QIcon>();
            QAction *copyAction = new QAction( icon, name, copyStyleMenu );
            copyAction->setToolTip( tooltip );
            connect( copyAction, &QAction::triggered, this, [ = ]() {app->copyStyle( layer, category );} );
            copyStyleMenu->addAction( copyAction );
            if ( category == QgsMapLayer::AllStyleCategories )
              copyStyleMenu->addSeparator();
          }
        }
        else
        {
          menuStyleManager->addAction( tr( "Copy Style" ), app, [ = ] { app->copyStyle(); } );
        }

        if ( layer && app->clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
        {
          if ( layer->type() == QgsMapLayerType::VectorLayer )
          {
            QDomDocument doc( QStringLiteral( "qgis" ) );
            QString errorMsg;
            int errorLine, errorColumn;
            if ( doc.setContent( app->clipboard()->data( QGSCLIPBOARD_STYLE_MIME ), false, &errorMsg, &errorLine, &errorColumn ) )
            {
              const QDomElement myRoot = doc.firstChildElement( QStringLiteral( "qgis" ) );
              if ( !myRoot.isNull() )
              {
                QMenu *pasteStyleMenu = menuStyleManager->addMenu( tr( "Paste Style" ) );
                pasteStyleMenu->setToolTipsVisible( true );

                const QgsMapLayer::StyleCategories sourceCategories = QgsXmlUtils::readFlagAttribute( myRoot, QStringLiteral( "styleCategories" ), QgsMapLayer::AllStyleCategories );

                QgsMapLayerStyleCategoriesModel *model = new QgsMapLayerStyleCategoriesModel( layer->type(), pasteStyleMenu );
                model->setShowAllCategories( true );
                for ( int row = 0; row < model->rowCount(); ++row )
                {
                  const QModelIndex index = model->index( row, 0 );
                  const QgsMapLayer::StyleCategory category = model->data( index, Qt::UserRole ).value<QgsMapLayer::StyleCategory>();
                  const QString name = model->data( index, Qt::DisplayRole ).toString();
                  const QString tooltip = model->data( index, Qt::ToolTipRole ).toString();
                  const QIcon icon = model->data( index, Qt::DecorationRole ).value<QIcon>();
                  QAction *pasteAction = new QAction( icon, name, pasteStyleMenu );
                  pasteAction->setToolTip( tooltip );
                  connect( pasteAction, &QAction::triggered, this, [ = ]() {app->pasteStyle( layer, category );} );
                  pasteStyleMenu->addAction( pasteAction );
                  if ( category == QgsMapLayer::AllStyleCategories )
                    pasteStyleMenu->addSeparator();
                  else
                    pasteAction->setEnabled( sourceCategories.testFlag( category ) );
                }
              }
            }
          }
          else
          {
            menuStyleManager->addAction( tr( "Paste Style" ), app, [ = ] { app->pasteStyle(); } );
          }
        }

        menuStyleManager->addSeparator();
        QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( menuStyleManager, layer );

        if ( vlayer )
        {
          const QgsSingleSymbolRenderer *singleRenderer = dynamic_cast< const QgsSingleSymbolRenderer * >( vlayer->renderer() );
          if ( !singleRenderer && vlayer->renderer() && vlayer->renderer()->embeddedRenderer() )
          {
            singleRenderer = dynamic_cast< const QgsSingleSymbolRenderer * >( vlayer->renderer()->embeddedRenderer() );
          }
          if ( singleRenderer && singleRenderer->symbol() )
          {
            //single symbol renderer, so add set color/edit symbol actions
            menuStyleManager->addSeparator();
            QgsColorWheel *colorWheel = new QgsColorWheel( menuStyleManager );
            colorWheel->setColor( singleRenderer->symbol()->color() );
            QgsColorWidgetAction *colorAction = new QgsColorWidgetAction( colorWheel, menuStyleManager, menuStyleManager );
            colorAction->setDismissOnColorSelection( false );
            connect( colorAction, &QgsColorWidgetAction::colorChanged, this, &QgsAppLayerTreeViewMenuProvider::setVectorSymbolColor );
            //store the layer id in action, so we can later retrieve the corresponding layer
            colorAction->setProperty( "layerId", vlayer->id() );
            menuStyleManager->addAction( colorAction );

            //add recent colors action
            QList<QgsRecentColorScheme *> recentSchemes;
            QgsApplication::colorSchemeRegistry()->schemes( recentSchemes );
            if ( !recentSchemes.isEmpty() )
            {
              QgsColorSwatchGridAction *recentColorAction = new QgsColorSwatchGridAction( recentSchemes.at( 0 ), menuStyleManager, QStringLiteral( "symbology" ), menuStyleManager );
              recentColorAction->setProperty( "layerId", vlayer->id() );
              recentColorAction->setDismissOnColorSelection( false );
              menuStyleManager->addAction( recentColorAction );
              connect( recentColorAction, &QgsColorSwatchGridAction::colorChanged, this, &QgsAppLayerTreeViewMenuProvider::setVectorSymbolColor );
            }

            menuStyleManager->addSeparator();
            const QString layerId = vlayer->id();

            QAction *editSymbolAction = new QAction( tr( "Edit Symbol…" ), menuStyleManager );
            connect( editSymbolAction, &QAction::triggered, this, [this, layerId]
            {
              editVectorSymbol( layerId );
            } );
            menuStyleManager->addAction( editSymbolAction );

            QAction *copySymbolAction = new QAction( tr( "Copy Symbol" ), menuStyleManager );
            connect( copySymbolAction, &QAction::triggered, this, [this, layerId]
            {
              copyVectorSymbol( layerId );
            } );
            menuStyleManager->addAction( copySymbolAction );

            bool enablePaste = false;
            const std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
            if ( tempSymbol )
              enablePaste = true;

            QAction *pasteSymbolAction = new QAction( tr( "Paste Symbol" ), menuStyleManager );
            connect( pasteSymbolAction, &QAction::triggered, this, [this, layerId]
            {
              pasteVectorSymbol( layerId );
            } );
            pasteSymbolAction->setEnabled( enablePaste );
            menuStyleManager->addAction( pasteSymbolAction );
          }
        }

        menu->addMenu( menuStyleManager );
      }
      else
      {
        if ( QgisApp::instance()->clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
        {
          menu->addAction( tr( "Paste Style" ), QgisApp::instance(), &QgisApp::applyStyleToGroup );
        }
      }

      // Actions for layer notes
      if ( layer )
      {
        QAction *notes = new QAction( QgsLayerNotesUtils::layerHasNotes( layer ) ? tr( "Edit Layer Notes…" ) : tr( "Add Layer Notes…" ), menu );
        connect( notes, &QAction::triggered, this, [layer ]
        {
          QgsLayerNotesManager::editLayerNotes( layer, QgisApp::instance() );
        } );
        menu->addAction( notes );
        if ( QgsLayerNotesUtils::layerHasNotes( layer ) )
        {
          QAction *notes = new QAction( tr( "Remove Layer Notes" ), menu );
          connect( notes, &QAction::triggered, this, [layer ]
          {
            if ( QMessageBox::question( QgisApp::instance(),
                                        tr( "Remove Layer Notes" ),
                                        tr( "Are you sure you want to remove all notes for the layer “%1”?" ).arg( layer->name() ),
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
            {
              QgsLayerNotesUtils::removeNotes( layer );
              QgsProject::instance()->setDirty( true );
            }
          } );
          menu->addAction( notes );
        }
      }

      if ( layer && QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
        menu->addAction( tr( "&Properties…" ), QgisApp::instance(), &QgisApp::layerProperties );
    }
  }
  else if ( QgsLayerTreeModelLegendNode *node = mView->index2legendNode( idx ) )
  {
    if ( node->flags() & Qt::ItemIsUserCheckable )
    {
      menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleAllLayers.svg" ) ), tr( "&Toggle Items" ),
                       node, &QgsLayerTreeModelLegendNode::toggleAllItems );
      menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ), tr( "&Show All Items" ),
                       node, &QgsLayerTreeModelLegendNode::checkAllItems );
      menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHideAllLayers.svg" ) ), tr( "&Hide All Items" ),
                       node, &QgsLayerTreeModelLegendNode::uncheckAllItems );
      menu->addSeparator();
    }

    if ( QgsSymbolLegendNode *symbolNode = qobject_cast< QgsSymbolLegendNode * >( node ) )
    {
      // symbology item
      QgsMapLayer *layer = QgsLayerTree::toLayer( node->layerNode() )->layer();
      if ( layer && layer->type() == QgsMapLayerType::VectorLayer && symbolNode->symbol() )
      {
        QgsColorWheel *colorWheel = new QgsColorWheel( menu );
        colorWheel->setColor( symbolNode->symbol()->color() );
        QgsColorWidgetAction *colorAction = new QgsColorWidgetAction( colorWheel, menu, menu );
        colorAction->setDismissOnColorSelection( false );
        connect( colorAction, &QgsColorWidgetAction::colorChanged, this, &QgsAppLayerTreeViewMenuProvider::setSymbolLegendNodeColor );
        //store the layer id and rule key in action, so we can later retrieve the corresponding
        //legend node, if it still exists
        colorAction->setProperty( "layerId", symbolNode->layerNode()->layerId() );
        colorAction->setProperty( "ruleKey", symbolNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString() );
        menu->addAction( colorAction );

        //add recent colors action
        QList<QgsRecentColorScheme *> recentSchemes;
        QgsApplication::colorSchemeRegistry()->schemes( recentSchemes );
        if ( !recentSchemes.isEmpty() )
        {
          QgsColorSwatchGridAction *recentColorAction = new QgsColorSwatchGridAction( recentSchemes.at( 0 ), menu, QStringLiteral( "symbology" ), menu );
          recentColorAction->setProperty( "layerId", symbolNode->layerNode()->layerId() );
          recentColorAction->setProperty( "ruleKey", symbolNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString() );
          recentColorAction->setDismissOnColorSelection( false );
          menu->addAction( recentColorAction );
          connect( recentColorAction, &QgsColorSwatchGridAction::colorChanged, this, &QgsAppLayerTreeViewMenuProvider::setSymbolLegendNodeColor );
        }

        menu->addSeparator();
      }

      const QString layerId = symbolNode->layerNode()->layerId();
      const QString ruleKey = symbolNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString();

      if ( layer && layer->type() == QgsMapLayerType::VectorLayer )
      {
        QAction *editSymbolAction = new QAction( tr( "Edit Symbol…" ), menu );
        connect( editSymbolAction, &QAction::triggered, this, [this, layerId, ruleKey ]
        {
          editSymbolLegendNodeSymbol( layerId, ruleKey );
        } );
        menu->addAction( editSymbolAction );
      }

      QAction *copySymbolAction = new QAction( tr( "Copy Symbol" ), menu );
      connect( copySymbolAction, &QAction::triggered, this, [this, layerId, ruleKey ]
      {
        copySymbolLegendNodeSymbol( layerId, ruleKey );
      } );
      menu->addAction( copySymbolAction );

      if ( layer && layer->type() == QgsMapLayerType::VectorLayer )
      {
        bool enablePaste = false;
        const std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
        if ( tempSymbol )
          enablePaste = true;

        QAction *pasteSymbolAction = new QAction( tr( "Paste Symbol" ), menu );
        connect( pasteSymbolAction, &QAction::triggered, this, [this, layerId, ruleKey]
        {
          pasteSymbolLegendNodeSymbol( layerId, ruleKey );
        } );
        pasteSymbolAction->setEnabled( enablePaste );
        menu->addAction( pasteSymbolAction );
      }
    }
  }

  return menu;
}

void QgsAppLayerTreeViewMenuProvider::addLegendLayerAction( QAction *action, const QString &menu,
    QgsMapLayerType type, bool allLayers )
{
  mLegendLayerActionMap[type].append( LegendLayerAction( action, menu, allLayers ) );
}

bool QgsAppLayerTreeViewMenuProvider::removeLegendLayerAction( QAction *action )
{
  QMap< QgsMapLayerType, QList< LegendLayerAction > >::iterator it;
  for ( it = mLegendLayerActionMap.begin();
        it != mLegendLayerActionMap.end(); ++it )
  {
    for ( int i = 0; i < it->count(); i++ )
    {
      if ( ( *it )[i].action == action )
      {
        ( *it ).removeAt( i );
        return true;
      }
    }
  }
  return false;
}

void QgsAppLayerTreeViewMenuProvider::addLegendLayerActionForLayer( QAction *action, QgsMapLayer *layer )
{
  if ( !action || !layer )
    return;

  legendLayerActions( layer->type() );
  if ( !mLegendLayerActionMap.contains( layer->type() ) )
    return;

  const QMap< QgsMapLayerType, QList< LegendLayerAction > >::iterator it
    = mLegendLayerActionMap.find( layer->type() );
  for ( int i = 0; i < it->count(); i++ )
  {
    if ( ( *it )[i].action == action )
    {
      ( *it )[i].layers.append( layer );
      return;
    }
  }
}

void QgsAppLayerTreeViewMenuProvider::removeLegendLayerActionsForLayer( QgsMapLayer *layer )
{
  if ( ! layer || ! mLegendLayerActionMap.contains( layer->type() ) )
    return;

  const QMap< QgsMapLayerType, QList< LegendLayerAction > >::iterator it
    = mLegendLayerActionMap.find( layer->type() );
  for ( int i = 0; i < it->count(); i++ )
  {
    ( *it )[i].layers.removeAll( layer );
  }
}

QList< LegendLayerAction > QgsAppLayerTreeViewMenuProvider::legendLayerActions( QgsMapLayerType type ) const
{
#ifdef QGISDEBUG
  if ( mLegendLayerActionMap.contains( type ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "legendLayerActions for layers of type %1:" ).arg( static_cast<int>( type ) ), 2 );

    const auto legendLayerActions { mLegendLayerActionMap.value( type ) };
    for ( const LegendLayerAction &lyrAction : legendLayerActions )
    {
      Q_UNUSED( lyrAction )
      QgsDebugMsgLevel( QStringLiteral( "%1/%2 - %3 layers" ).arg( lyrAction.menu, lyrAction.action->text() ).arg( lyrAction.layers.count() ), 2 );
    }
  }
#endif

  return mLegendLayerActionMap.contains( type ) ? mLegendLayerActionMap.value( type ) : QList< LegendLayerAction >();
}

void QgsAppLayerTreeViewMenuProvider::addCustomLayerActions( QMenu *menu, QgsMapLayer *layer )
{
  if ( !layer )
    return;

  // add custom layer actions - should this go at end?
  QList< LegendLayerAction > lyrActions = legendLayerActions( layer->type() );

  if ( ! lyrActions.isEmpty() )
  {
    menu->addSeparator();
    QList<QMenu *> menus;
    for ( int i = 0; i < lyrActions.count(); i++ )
    {
      if ( lyrActions[i].allLayers || lyrActions[i].layers.contains( layer ) )
      {
        if ( lyrActions[i].menu.isEmpty() )
        {
          menu->addAction( lyrActions[i].action );
        }
        else
        {
          // find or create menu for given menu name
          // adapted from QgisApp::getPluginMenu( QString menuName )
          QString menuName = lyrActions[i].menu;
#ifdef Q_OS_MAC
          // Mac doesn't have '&' keyboard shortcuts.
          menuName.remove( QChar( '&' ) );
#endif
          QAction *before = nullptr;
          QMenu *newMenu = nullptr;
          QString dst = menuName;
          dst.remove( QChar( '&' ) );
          const auto constMenus = menus;
          for ( QMenu *menu : constMenus )
          {
            QString src = menu->title();
            src.remove( QChar( '&' ) );
            const int comp = dst.localeAwareCompare( src );
            if ( comp < 0 )
            {
              // Add item before this one
              before = menu->menuAction();
              break;
            }
            else if ( comp == 0 )
            {
              // Plugin menu item already exists
              newMenu = menu;
              break;
            }
          }
          if ( ! newMenu )
          {
            // It doesn't exist, so create
            newMenu = new QMenu( menuName );
            menus.append( newMenu );
            // Where to put it? - we worked that out above...
            menu->insertMenu( before, newMenu );
          }
          // QMenu* menu = getMenu( lyrActions[i].menu, &beforeSep, &afterSep, &menu );
          newMenu->addAction( lyrActions[i].action );
        }
      }
    }
    menu->addSeparator();
  }
}

void QgsAppLayerTreeViewMenuProvider::editVectorSymbol( const QString &layerId )
{
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  QgsSingleSymbolRenderer *singleRenderer = dynamic_cast< QgsSingleSymbolRenderer * >( layer->renderer() );
  if ( !singleRenderer )
    return;

  std::unique_ptr< QgsSymbol > symbol( singleRenderer->symbol() ? singleRenderer->symbol()->clone() : nullptr );
  QgsSymbolSelectorDialog dlg( symbol.get(), QgsStyle::defaultStyle(), layer, mView->window() );
  dlg.setWindowTitle( tr( "Symbol Selector" ) );
  QgsSymbolWidgetContext context;
  context.setMapCanvas( mCanvas );
  context.setMessageBar( QgisApp::instance()->messageBar() );
  dlg.setContext( context );
  if ( dlg.exec() )
  {
    singleRenderer->setSymbol( symbol.release() );
    layer->triggerRepaint();
    mView->refreshLayerSymbology( layer->id() );
    layer->emitStyleChanged();
    QgsProject::instance()->setDirty( true );
  }
}

void QgsAppLayerTreeViewMenuProvider::copyVectorSymbol( const QString &layerId )
{
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  QgsSingleSymbolRenderer *singleRenderer = dynamic_cast< QgsSingleSymbolRenderer * >( layer->renderer() );
  if ( !singleRenderer )
    return;

  QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::symbolToMimeData( singleRenderer->symbol() ) );
}

void QgsAppLayerTreeViewMenuProvider::pasteVectorSymbol( const QString &layerId )
{
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  QgsSingleSymbolRenderer *singleRenderer = dynamic_cast< QgsSingleSymbolRenderer * >( layer->renderer() );
  if ( !singleRenderer )
    return;

  std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
  if ( !tempSymbol )
    return;

  if ( !singleRenderer->symbol() || singleRenderer->symbol()->type() != tempSymbol->type() )
    return;

  singleRenderer->setSymbol( tempSymbol.release() );
  layer->triggerRepaint();
  layer->emitStyleChanged();
  mView->refreshLayerSymbology( layer->id() );
  QgsProject::instance()->setDirty( true );
}

void QgsAppLayerTreeViewMenuProvider::setVectorSymbolColor( const QColor &color )
{
  QAction *action = qobject_cast< QAction *>( sender() );
  if ( !action )
    return;

  const QString layerId = action->property( "layerId" ).toString();
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  QgsSingleSymbolRenderer *singleRenderer = dynamic_cast< QgsSingleSymbolRenderer * >( layer->renderer() );
  QgsSymbol *newSymbol = nullptr;

  if ( singleRenderer && singleRenderer->symbol() )
    newSymbol = singleRenderer->symbol()->clone();

  const QgsSingleSymbolRenderer *embeddedRenderer = nullptr;
  if ( !newSymbol && layer->renderer()->embeddedRenderer() )
  {
    embeddedRenderer = dynamic_cast< const QgsSingleSymbolRenderer * >( layer->renderer()->embeddedRenderer() );
    if ( embeddedRenderer && embeddedRenderer->symbol() )
      newSymbol = embeddedRenderer->symbol()->clone();
  }

  if ( !newSymbol )
    return;

  newSymbol->setColor( color );
  if ( singleRenderer )
  {
    singleRenderer->setSymbol( newSymbol );
  }
  else if ( embeddedRenderer )
  {
    QgsSingleSymbolRenderer *newRenderer = embeddedRenderer->clone();
    newRenderer->setSymbol( newSymbol );
    layer->renderer()->setEmbeddedRenderer( newRenderer );
  }

  layer->triggerRepaint();
  layer->emitStyleChanged();
  mView->refreshLayerSymbology( layer->id() );
  QgsProject::instance()->setDirty( true );
}

void QgsAppLayerTreeViewMenuProvider::editSymbolLegendNodeSymbol( const QString &layerId, const QString &ruleKey )
{
  QgsSymbolLegendNode *node = qobject_cast<QgsSymbolLegendNode *>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbol *originalSymbol = node->symbol();
  if ( !originalSymbol )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "No Symbol" ), tr( "There is no symbol associated with the rule." ) );
    return;
  }

  std::unique_ptr< QgsSymbol > symbol( originalSymbol->clone() );
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( node->layerNode()->layer() );
  QgsSymbolSelectorDialog dlg( symbol.get(), QgsStyle::defaultStyle(), vlayer, mView->window() );
  dlg.setWindowTitle( tr( "Symbol Selector" ) );
  QgsSymbolWidgetContext context;
  context.setMapCanvas( mCanvas );
  context.setMessageBar( QgisApp::instance()->messageBar() );
  dlg.setContext( context );
  if ( dlg.exec() )
  {
    node->setSymbol( symbol.release() );
    if ( vlayer )
    {
      vlayer->emitStyleChanged();
    }
    QgsProject::instance()->setDirty( true );
  }
}

void QgsAppLayerTreeViewMenuProvider::copySymbolLegendNodeSymbol( const QString &layerId, const QString &ruleKey )
{
  QgsSymbolLegendNode *node = qobject_cast<QgsSymbolLegendNode *>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbol *originalSymbol = node->symbol();
  if ( !originalSymbol )
    return;

  QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::symbolToMimeData( originalSymbol ) );
}

void QgsAppLayerTreeViewMenuProvider::pasteSymbolLegendNodeSymbol( const QString &layerId, const QString &ruleKey )
{
  QgsSymbolLegendNode *node = qobject_cast<QgsSymbolLegendNode *>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbol *originalSymbol = node->symbol();
  if ( !originalSymbol )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( node->layerNode()->layer() );

  std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
  if ( tempSymbol && tempSymbol->type() == originalSymbol->type() )
  {
    node->setSymbol( tempSymbol.release() );
    if ( vlayer )
    {
      vlayer->emitStyleChanged();
    }
    QgsProject::instance()->setDirty( true );
  }
}

void QgsAppLayerTreeViewMenuProvider::setSymbolLegendNodeColor( const QColor &color )
{
  QAction *action = qobject_cast< QAction *>( sender() );
  if ( !action )
    return;

  const QString layerId = action->property( "layerId" ).toString();
  const QString ruleKey = action->property( "ruleKey" ).toString();

  QgsSymbolLegendNode *node = qobject_cast<QgsSymbolLegendNode *>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbol *originalSymbol = node->symbol();
  if ( !originalSymbol )
    return;

  std::unique_ptr< QgsSymbol > newSymbol( originalSymbol->clone() );
  newSymbol->setColor( color );
  node->setSymbol( newSymbol.release() );
  if ( QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId ) )
  {
    layer->emitStyleChanged();
  }
  QgsProject::instance()->setDirty( true );
}

bool QgsAppLayerTreeViewMenuProvider::removeActionEnabled()
{
  const QList<QgsLayerTreeLayer *> selectedLayers = mView->selectedLayerNodes();
  for ( QgsLayerTreeLayer *nodeLayer : selectedLayers )
  {
    // be careful with the logic here -- if nodeLayer->layer() is false, will still must return true
    // to allow the broken layer to be removed from the project
    if ( nodeLayer->layer() && !nodeLayer->layer()->flags().testFlag( QgsMapLayer::Removable ) )
      return false;
  }
  return true;
}

void QgsAppLayerTreeViewMenuProvider::setLayerCrs( const QgsCoordinateReferenceSystem &crs )
{
  const auto constSelectedNodes = mView->selectedNodes();
  for ( QgsLayerTreeNode *node : constSelectedNodes )
  {
    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->layer() )
      {
        nodeLayer->layer()->setCrs( crs, true );
        nodeLayer->layer()->triggerRepaint();
      }
    }
  }
}

void QgsAppLayerTreeViewMenuProvider::toggleLabels( bool enabled )
{
  const QList<QgsLayerTreeLayer *> selectedLayerNodes = mView->selectedLayerNodes();
  for ( QgsLayerTreeLayer *l : selectedLayerNodes )
  {
    QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( l->layer() );
    if ( !vlayer || !vlayer->isSpatial() )
      continue;

    if ( enabled && !vlayer->labeling() )
    {
      // no labeling setup - create default labeling for layer
      const QgsPalLayerSettings settings = QgsAbstractVectorLayerLabeling::defaultSettingsForLayer( vlayer );
      vlayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
      vlayer->setLabelsEnabled( true );
    }
    else
    {
      vlayer->setLabelsEnabled( enabled );
    }
    vlayer->emitStyleChanged();
    vlayer->triggerRepaint();
  }
}
