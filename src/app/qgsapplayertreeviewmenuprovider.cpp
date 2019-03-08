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

#include "qgsapplayertreeviewmenuprovider.h"


#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsclipboard.h"
#include "qgscolorwidgets.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorswatchgrid.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeviewdefaultactions.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"
#include "qgsstyle.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslayertreeregistrybridge.h"
#include "qgssymbolselectordialog.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsmaplayerstylecategoriesmodel.h"
#include "qgsxmlutils.h"



QgsAppLayerTreeViewMenuProvider::QgsAppLayerTreeViewMenuProvider( QgsLayerTreeView *view, QgsMapCanvas *canvas )
  : mView( view )
  , mCanvas( canvas )
{
}


QMenu *QgsAppLayerTreeViewMenuProvider::createContextMenu()
{
  QMenu *menu = new QMenu;

  QgsLayerTreeViewDefaultActions *actions = mView->defaultActions();

  QModelIndex idx = mView->currentIndex();
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
  else if ( QgsLayerTreeNode *node = mView->layerTreeModel()->index2node( idx ) )
  {
    // layer or group selected
    if ( QgsLayerTree::isGroup( node ) )
    {
      menu->addAction( actions->actionZoomToGroup( mCanvas, menu ) );

      menu->addAction( tr( "Copy Group" ), QgisApp::instance(), &QgisApp::copyLayer );
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
                       tr( "&Set Group CRS…" ), QgisApp::instance(), &QgisApp::legendGroupSetCrs );
      menu->addAction( tr( "&Set Group WMS Data…" ), QgisApp::instance(), &QgisApp::legendGroupSetWmsData );

      menu->addSeparator();

      menu->addAction( actions->actionMutuallyExclusiveGroup( menu ) );

      menu->addAction( actions->actionCheckAndAllChildren( menu ) );

      menu->addAction( actions->actionUncheckAndAllChildren( menu ) );

      if ( !( mView->selectedNodes( true ).count() == 1 && idx.row() == 0 ) )
      {
        menu->addAction( actions->actionMoveToTop( menu ) );
      }

      menu->addSeparator();

      if ( mView->selectedNodes( true ).count() >= 2 )
        menu->addAction( actions->actionGroupSelected( menu ) );

      if ( QgisApp::instance()->clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
      {
        menu->addAction( tr( "Paste Style" ), QgisApp::instance(), &QgisApp::applyStyleToGroup );
      }

      menu->addSeparator();

      QMenu *menuExportGroup = new QMenu( tr( "Export" ), menu );
      QAction *actionSaveAsDefinitionGroup = new QAction( tr( "Save as Layer Definition File…" ), menuExportGroup );
      connect( actionSaveAsDefinitionGroup, &QAction::triggered, QgisApp::instance(), &QgisApp::saveAsLayerDefinition );
      menuExportGroup->addAction( actionSaveAsDefinitionGroup );

      menu->addMenu( menuExportGroup );
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );

      if ( layer && layer->isSpatial() )
      {
        menu->addAction( actions->actionZoomToLayer( mCanvas, menu ) );
        if ( vlayer )
        {
          QAction *actionZoomSelected = actions->actionZoomToSelection( mCanvas, menu );
          actionZoomSelected->setEnabled( !vlayer->selectedFeatureIds().isEmpty() );
          menu->addAction( actionZoomSelected );
        }
        menu->addAction( actions->actionShowInOverview( menu ) );
      }

      if ( vlayer )
        menu->addAction( actions->actionShowFeatureCount( menu ) );

      QAction *actionCopyLayer = new QAction( tr( "Copy Layer" ), menu );
      connect( actionCopyLayer, &QAction::triggered, QgisApp::instance(), &QgisApp::copyLayer );
      menu->addAction( actionCopyLayer );

      menu->addAction( actions->actionRenameGroupOrLayer( menu ) );

      if ( rlayer )
      {
        menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomActual.svg" ) ), tr( "&Zoom to Native Resolution (100%)" ), QgisApp::instance(), &QgisApp::legendLayerZoomNative );

        if ( rlayer->rasterType() != QgsRasterLayer::Palette )
          menu->addAction( tr( "&Stretch Using Current Extent" ), QgisApp::instance(), &QgisApp::legendLayerStretchUsingCurrentExtent );
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
        QgsAttributeTableFilterModel::FilterMode initialMode = settings.enumValue( QStringLiteral( "qgis/attributeTableBehavior" ),  QgsAttributeTableFilterModel::ShowAll );
        menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ), tr( "&Open Attribute Table" ),
                         QgisApp::instance(), [ = ] { QgisApp::instance()->attributeTable( initialMode ); } );

        // allow editing
        unsigned int cap = vlayer->dataProvider()->capabilities();
        if ( cap & QgsVectorDataProvider::EditingCapabilities )
        {
          if ( toggleEditingAction )
          {
            menu->addAction( toggleEditingAction );
            toggleEditingAction->setChecked( vlayer->isEditable() );
            toggleEditingAction->setEnabled( true );
          }
          if ( saveLayerEditsAction && vlayer->isModified() )
          {
            menu->addAction( saveLayerEditsAction );
          }
        }

        if ( allEditsAction->isEnabled() )
          menu->addAction( allEditsAction );

        // disable duplication of memory layers
        if ( vlayer->storageType() == QLatin1String( "Memory storage" ) && mView->selectedLayerNodes().count() == 1 )
          duplicateLayersAction->setEnabled( false );

        if ( vlayer->dataProvider()->supportsSubsetString() )
        {
          QAction *action = menu->addAction( tr( "&Filter…" ), QgisApp::instance(), &QgisApp::layerSubsetString );
          action->setEnabled( !vlayer->isEditable() );
        }
      }

      // change data source is only supported for vectors and rasters
      if ( vlayer || rlayer )
      {

        QAction *a = new QAction( tr( "Change Data Source…" ), menu );
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

      menu->addSeparator();

      if ( layer && layer->isSpatial() )
      {
        // set layer scale visibility
        menu->addAction( tr( "&Set Layer Scale Visibility…" ), QgisApp::instance(), &QgisApp::setLayerScaleVisibility );

        if ( !layer->isInScaleRange( mCanvas->scale() ) )
          menu->addAction( tr( "Zoom to &Visible Scale" ), QgisApp::instance(), &QgisApp::zoomToLayerScale );

        QMenu *menuSetCRS = new QMenu( tr( "Set CRS" ), menu );
        // set layer crs
        QAction *actionSetLayerCrs = new QAction( tr( "Set Layer CRS…" ), menuSetCRS );
        connect( actionSetLayerCrs, &QAction::triggered, QgisApp::instance(), &QgisApp::setLayerCrs );
        menuSetCRS->addAction( actionSetLayerCrs );
        // assign layer crs to project
        QAction *actionSetProjectCrs = new QAction( tr( "Set &Project CRS from Layer" ), menuSetCRS );
        connect( actionSetProjectCrs, &QAction::triggered, QgisApp::instance(), &QgisApp::setProjectCrsFromLayer );
        menuSetCRS->addAction( actionSetProjectCrs );

        menu->addMenu( menuSetCRS );
      }

      menu->addSeparator();

      if ( vlayer )
      {
        if ( vlayer->dataProvider()->name() == QLatin1String( "memory" ) )
        {
          QAction *actionMakePermanent = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionFileSave.svg" ) ), tr( "Make Permanent…" ), menu );
          connect( actionMakePermanent, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->makeMemoryLayerPermanent( vlayer ); } );
          menu->addAction( actionMakePermanent );
        }
        // save as vector file
        QMenu *menuExportVector = new QMenu( tr( "Export" ), menu );
        QAction *actionSaveAs = new QAction( tr( "Save Features As…" ), menuExportVector );
        connect( actionSaveAs, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveAsFile(); } );
        menuExportVector->addAction( actionSaveAs );
        QAction *actionSaveSelectedFeaturesAs = new QAction( tr( "Save Selected Features As…" ), menuExportVector );
        connect( actionSaveSelectedFeaturesAs, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveAsFile( nullptr, true ); } );
        actionSaveSelectedFeaturesAs->setEnabled( vlayer->selectedFeatureCount() > 0 );
        menuExportVector->addAction( actionSaveSelectedFeaturesAs );
        QAction *actionSaveAsDefinitionLayer = new QAction( tr( "Save as Layer Definition File…" ), menuExportVector );
        connect( actionSaveAsDefinitionLayer, &QAction::triggered, QgisApp::instance(), &QgisApp::saveAsLayerDefinition );
        menuExportVector->addAction( actionSaveAsDefinitionLayer );
        if ( vlayer->isSpatial() )
        {
          QAction *actionSaveStyle = new QAction( tr( "Save as QGIS Layer Style File…" ), menuExportVector );
          connect( actionSaveStyle, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveStyleFile(); } );
          menuExportVector->addAction( actionSaveStyle );
        }
        menu->addMenu( menuExportVector );
      }
      else if ( rlayer )
      {
        QMenu *menuExportRaster = new QMenu( tr( "Export" ), menu );
        QAction *actionSaveAs = new QAction( tr( "Save As…" ), menuExportRaster );
        QAction *actionSaveAsDefinitionLayer = new QAction( tr( "Save as Layer Definition File…" ), menuExportRaster );
        QAction *actionSaveStyle = new QAction( tr( "Save as QGIS Layer Style File…" ), menuExportRaster );
        connect( actionSaveAs, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveAsFile(); } );
        menuExportRaster->addAction( actionSaveAs );
        connect( actionSaveAsDefinitionLayer, &QAction::triggered, QgisApp::instance(), &QgisApp::saveAsLayerDefinition );
        menuExportRaster->addAction( actionSaveAsDefinitionLayer );
        connect( actionSaveStyle, &QAction::triggered, QgisApp::instance(), [ = ] { QgisApp::instance()->saveStyleFile(); } );
        menuExportRaster->addAction( actionSaveStyle );
        menu->addMenu( menuExportRaster );
      }
      else if ( layer && layer->type() == QgsMapLayerType::PluginLayer && mView->selectedLayerNodes().count() == 1 )
      {
        // disable duplication of plugin layers
        duplicateLayersAction->setEnabled( false );
      }

      menu->addSeparator();

      // style-related actions
      if ( layer && mView->selectedLayerNodes().count() == 1 )
      {
        menu->addSeparator();
        QMenu *menuStyleManager = new QMenu( tr( "Styles" ), menu );

        QgisApp *app = QgisApp::instance();
        if ( layer->type() == QgsMapLayerType::VectorLayer )
        {
          QMenu *copyStyleMenu = menuStyleManager->addMenu( tr( "Copy Style" ) );
          copyStyleMenu->setToolTipsVisible( true );
          QgsMapLayerStyleCategoriesModel *model = new QgsMapLayerStyleCategoriesModel( copyStyleMenu );
          model->setShowAllCategories( true );
          for ( int row = 0; row < model->rowCount(); ++row )
          {
            QModelIndex index = model->index( row, 0 );
            QgsMapLayer::StyleCategory category = model->data( index, Qt::UserRole ).value<QgsMapLayer::StyleCategory>();
            QString name = model->data( index, Qt::DisplayRole ).toString();
            QString tooltip = model->data( index, Qt::ToolTipRole ).toString();
            QIcon icon = model->data( index, Qt::DecorationRole ).value<QIcon>();
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
              QDomElement myRoot = doc.firstChildElement( QStringLiteral( "qgis" ) );
              if ( !myRoot.isNull() )
              {
                QMenu *pasteStyleMenu = menuStyleManager->addMenu( tr( "Paste Style" ) );
                pasteStyleMenu->setToolTipsVisible( true );

                QgsMapLayer::StyleCategories sourceCategories = QgsXmlUtils::readFlagAttribute( myRoot, QStringLiteral( "styleCategories" ), QgsMapLayer::AllStyleCategories );

                QgsMapLayerStyleCategoriesModel *model = new QgsMapLayerStyleCategoriesModel( pasteStyleMenu );
                model->setShowAllCategories( true );
                for ( int row = 0; row < model->rowCount(); ++row )
                {
                  QModelIndex index = model->index( row, 0 );
                  QgsMapLayer::StyleCategory category = model->data( index, Qt::UserRole ).value<QgsMapLayer::StyleCategory>();
                  QString name = model->data( index, Qt::DisplayRole ).toString();
                  QString tooltip = model->data( index, Qt::ToolTipRole ).toString();
                  QIcon icon = model->data( index, Qt::DecorationRole ).value<QIcon>();
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
            QAction *editSymbolAction = new QAction( tr( "Edit Symbol…" ), menuStyleManager );
            //store the layer id in action, so we can later retrieve the corresponding layer
            editSymbolAction->setProperty( "layerId", vlayer->id() );
            connect( editSymbolAction, &QAction::triggered, this, &QgsAppLayerTreeViewMenuProvider::editVectorSymbol );
            menuStyleManager->addAction( editSymbolAction );
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

      if ( layer && QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
        menu->addAction( tr( "&Properties…" ), QgisApp::instance(), &QgisApp::layerProperties );
    }
  }
  else if ( QgsLayerTreeModelLegendNode *node = mView->layerTreeModel()->index2legendNode( idx ) )
  {
    if ( QgsSymbolLegendNode *symbolNode = dynamic_cast< QgsSymbolLegendNode * >( node ) )
    {
      // symbology item
      if ( symbolNode->flags() & Qt::ItemIsUserCheckable )
      {
        menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleAllLayers.svg" ) ), tr( "&Toggle Items" ),
                         symbolNode, &QgsSymbolLegendNode::toggleAllItems );
        menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ), tr( "&Show All Items" ),
                         symbolNode, &QgsSymbolLegendNode::checkAllItems );
        menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHideAllLayers.svg" ) ), tr( "&Hide All Items" ),
                         symbolNode, &QgsSymbolLegendNode::uncheckAllItems );
        menu->addSeparator();
      }

      if ( symbolNode->symbol() )
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

      QAction *editSymbolAction = new QAction( tr( "Edit Symbol…" ), menu );
      //store the layer id and rule key in action, so we can later retrieve the corresponding
      //legend node, if it still exists
      editSymbolAction->setProperty( "layerId", symbolNode->layerNode()->layerId() );
      editSymbolAction->setProperty( "ruleKey", symbolNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString() );
      connect( editSymbolAction, &QAction::triggered, this, &QgsAppLayerTreeViewMenuProvider::editSymbolLegendNodeSymbol );
      menu->addAction( editSymbolAction );
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

  QMap< QgsMapLayerType, QList< LegendLayerAction > >::iterator it
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

  QMap< QgsMapLayerType, QList< LegendLayerAction > >::iterator it
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
    QgsDebugMsg( QStringLiteral( "legendLayerActions for layers of type %1:" ).arg( static_cast<int>( type ) ) );

    Q_FOREACH ( const LegendLayerAction &lyrAction, mLegendLayerActionMap[ type ] )
    {
      Q_UNUSED( lyrAction );
      QgsDebugMsg( QStringLiteral( "%1/%2 - %3 layers" ).arg( lyrAction.menu, lyrAction.action->text() ).arg( lyrAction.layers.count() ) );
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
          Q_FOREACH ( QMenu *menu, menus )
          {
            QString src = menu->title();
            src.remove( QChar( '&' ) );
            int comp = dst.localeAwareCompare( src );
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

void QgsAppLayerTreeViewMenuProvider::editVectorSymbol()
{
  QAction *action = qobject_cast< QAction *>( sender() );
  if ( !action )
    return;

  QString layerId = action->property( "layerId" ).toString();
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
  }
}

void QgsAppLayerTreeViewMenuProvider::setVectorSymbolColor( const QColor &color )
{
  QAction *action = qobject_cast< QAction *>( sender() );
  if ( !action )
    return;

  QString layerId = action->property( "layerId" ).toString();
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
}

void QgsAppLayerTreeViewMenuProvider::editSymbolLegendNodeSymbol()
{
  QAction *action = qobject_cast< QAction *>( sender() );
  if ( !action )
    return;

  QString layerId = action->property( "layerId" ).toString();
  QString ruleKey = action->property( "ruleKey" ).toString();

  QgsSymbolLegendNode *node = dynamic_cast<QgsSymbolLegendNode *>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbol *originalSymbol = node->symbol();
  if ( !originalSymbol )
    return;

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
  }
}

void QgsAppLayerTreeViewMenuProvider::setSymbolLegendNodeColor( const QColor &color )
{
  QAction *action = qobject_cast< QAction *>( sender() );
  if ( !action )
    return;

  QString layerId = action->property( "layerId" ).toString();
  QString ruleKey = action->property( "ruleKey" ).toString();

  QgsSymbolLegendNode *node = dynamic_cast<QgsSymbolLegendNode *>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
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
