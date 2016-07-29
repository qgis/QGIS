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
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include "qgsstylev2.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslayertreeregistrybridge.h"
#include "qgssymbolv2selectordialog.h"
#include "qgssinglesymbolrendererv2.h"

QgsAppLayerTreeViewMenuProvider::QgsAppLayerTreeViewMenuProvider( QgsLayerTreeView* view, QgsMapCanvas* canvas )
    : mView( view )
    , mCanvas( canvas )
{
}


QMenu* QgsAppLayerTreeViewMenuProvider::createContextMenu()
{
  QMenu* menu = new QMenu;

  QgsLayerTreeViewDefaultActions* actions = mView->defaultActions();

  QModelIndex idx = mView->currentIndex();
  if ( !idx.isValid() )
  {
    // global menu
    menu->addAction( actions->actionAddGroup( menu ) );

    menu->addAction( QgsApplication::getThemeIcon( "/mActionExpandTree.svg" ), tr( "&Expand All" ), mView, SLOT( expandAll() ) );
    menu->addAction( QgsApplication::getThemeIcon( "/mActionCollapseTree.svg" ), tr( "&Collapse All" ), mView, SLOT( collapseAll() ) );

    // TODO: update drawing order
  }
  else if ( QgsLayerTreeNode* node = mView->layerTreeModel()->index2node( idx ) )
  {
    // layer or group selected
    if ( QgsLayerTree::isGroup( node ) )
    {
      menu->addAction( actions->actionZoomToGroup( mCanvas, menu ) );

      menu->addAction( QgsApplication::getThemeIcon( "/mActionRemoveLayer.svg" ), tr( "&Remove" ), QgisApp::instance(), SLOT( removeLayer() ) );

      menu->addAction( QgsApplication::getThemeIcon( "/mActionSetCRS.png" ),
                       tr( "&Set Group CRS" ), QgisApp::instance(), SLOT( legendGroupSetCRS() ) );

      menu->addAction( actions->actionRenameGroupOrLayer( menu ) );

      menu->addAction( tr( "&Set Group WMS data" ), QgisApp::instance(), SLOT( legendGroupSetWMSData() ) );

      menu->addAction( actions->actionMutuallyExclusiveGroup( menu ) );

      if ( mView->selectedNodes( true ).count() >= 2 )
        menu->addAction( actions->actionGroupSelected( menu ) );

      if ( QgisApp::instance()->clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
      {
        menu->addAction( tr( "Paste Style" ), QgisApp::instance(), SLOT( applyStyleToGroup() ) );
      }

      menu->addAction( tr( "Save As Layer Definition File..." ), QgisApp::instance(), SLOT( saveAsLayerDefinition() ) );

      menu->addAction( actions->actionAddGroup( menu ) );
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );

      menu->addAction( actions->actionZoomToLayer( mCanvas, menu ) );
      menu->addAction( actions->actionShowInOverview( menu ) );

      if ( rlayer )
      {
        menu->addAction( QgsApplication::getThemeIcon( "/mActionZoomActual.svg" ), tr( "&Zoom to Native Resolution (100%)" ), QgisApp::instance(), SLOT( legendLayerZoomNative() ) );

        if ( rlayer->rasterType() != QgsRasterLayer::Palette )
          menu->addAction( tr( "&Stretch Using Current Extent" ), QgisApp::instance(), SLOT( legendLayerStretchUsingCurrentExtent() ) );
      }

      menu->addAction( QgsApplication::getThemeIcon( "/mActionRemoveLayer.svg" ), tr( "&Remove" ), QgisApp::instance(), SLOT( removeLayer() ) );

      // duplicate layer
      QAction* duplicateLayersAction = menu->addAction( QgsApplication::getThemeIcon( "/mActionDuplicateLayer.svg" ), tr( "&Duplicate" ), QgisApp::instance(), SLOT( duplicateLayers() ) );

      if ( layer && layer->isSpatial() )
      {
        // set layer scale visibility
        menu->addAction( tr( "&Set Layer Scale Visibility" ), QgisApp::instance(), SLOT( setLayerScaleVisibility() ) );

        if ( !layer->isInScaleRange( mCanvas->scale() ) )
          menu->addAction( tr( "Zoom to &Visible Scale" ), QgisApp::instance(), SLOT( zoomToLayerScale() ) );

        // set layer crs
        menu->addAction( QgsApplication::getThemeIcon( "/mActionSetCRS.png" ), tr( "Set Layer CRS" ), QgisApp::instance(), SLOT( setLayerCRS() ) );

        // assign layer crs to project
        menu->addAction( QgsApplication::getThemeIcon( "/mActionSetProjectCRS.png" ), tr( "Set &Project CRS from Layer" ), QgisApp::instance(), SLOT( setProjectCRSFromLayer() ) );
      }

      // style-related actions
      if ( layer && mView->selectedLayerNodes().count() == 1 )
      {
        QMenu *menuStyleManager = new QMenu( tr( "Styles" ), menu );

        QgisApp *app = QgisApp::instance();
        menuStyleManager->addAction( tr( "Copy Style" ), app, SLOT( copyStyle() ) );
        if ( app->clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
        {
          menuStyleManager->addAction( tr( "Paste Style" ), app, SLOT( pasteStyle() ) );
        }

        menuStyleManager->addSeparator();
        QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( menuStyleManager, layer );

        if ( vlayer )
        {
          const QgsSingleSymbolRendererV2* singleRenderer = dynamic_cast< const QgsSingleSymbolRendererV2* >( vlayer->rendererV2() );
          if ( !singleRenderer && vlayer->rendererV2() && vlayer->rendererV2()->embeddedRenderer() )
          {
            singleRenderer = dynamic_cast< const QgsSingleSymbolRendererV2* >( vlayer->rendererV2()->embeddedRenderer() );
          }
          if ( singleRenderer && singleRenderer->symbol() )
          {
            //single symbol renderer, so add set color/edit symbol actions
            menuStyleManager->addSeparator();
            QgsColorWheel* colorWheel = new QgsColorWheel( menuStyleManager );
            colorWheel->setColor( singleRenderer->symbol()->color() );
            QgsColorWidgetAction* colorAction = new QgsColorWidgetAction( colorWheel, menuStyleManager, menuStyleManager );
            colorAction->setDismissOnColorSelection( false );
            connect( colorAction, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setVectorSymbolColor( const QColor& ) ) );
            //store the layer id in action, so we can later retrieve the corresponding layer
            colorAction->setProperty( "layerId", vlayer->id() );
            menuStyleManager->addAction( colorAction );

            //add recent colors action
            QList<QgsRecentColorScheme *> recentSchemes;
            QgsColorSchemeRegistry::instance()->schemes( recentSchemes );
            if ( !recentSchemes.isEmpty() )
            {
              QgsColorSwatchGridAction* recentColorAction = new QgsColorSwatchGridAction( recentSchemes.at( 0 ), menuStyleManager, "symbology", menuStyleManager );
              recentColorAction->setProperty( "layerId", vlayer->id() );
              recentColorAction->setDismissOnColorSelection( false );
              menuStyleManager->addAction( recentColorAction );
              connect( recentColorAction, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setVectorSymbolColor( const QColor& ) ) );
            }

            menuStyleManager->addSeparator();
            QAction* editSymbolAction = new QAction( tr( "Edit Symbol..." ), menuStyleManager );
            //store the layer id in action, so we can later retrieve the corresponding layer
            editSymbolAction->setProperty( "layerId", vlayer->id() );
            connect( editSymbolAction, SIGNAL( triggered() ), this, SLOT( editVectorSymbol() ) );
            menuStyleManager->addAction( editSymbolAction );
          }
        }

        menu->addMenu( menuStyleManager );
      }
      else
      {
        if ( QgisApp::instance()->clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
        {
          menu->addAction( tr( "Paste Style" ), QgisApp::instance(), SLOT( applyStyleToGroup() ) );
        }
      }

      menu->addSeparator();

      if ( vlayer )
      {
        QAction *toggleEditingAction = QgisApp::instance()->actionToggleEditing();
        QAction *saveLayerEditsAction = QgisApp::instance()->actionSaveActiveLayerEdits();
        QAction *allEditsAction = QgisApp::instance()->actionAllEdits();

        // attribute table
        menu->addAction( QgsApplication::getThemeIcon( "/mActionOpenTable.svg" ), tr( "&Open Attribute Table" ),
                         QgisApp::instance(), SLOT( attributeTable() ) );

        // allow editing
        int cap = vlayer->dataProvider()->capabilities();
        if ( cap & QgsVectorDataProvider::EditingCapabilities )
        {
          if ( toggleEditingAction )
          {
            menu->addAction( toggleEditingAction );
            toggleEditingAction->setChecked( vlayer->isEditable() );
          }
          if ( saveLayerEditsAction && vlayer->isModified() )
          {
            menu->addAction( saveLayerEditsAction );
          }
        }

        if ( allEditsAction->isEnabled() )
          menu->addAction( allEditsAction );

        // disable duplication of memory layers
        if ( vlayer->storageType() == "Memory storage" && mView->selectedLayerNodes().count() == 1 )
          duplicateLayersAction->setEnabled( false );

        // save as vector file
        menu->addAction( tr( "Save As..." ), QgisApp::instance(), SLOT( saveAsFile() ) );
        menu->addAction( tr( "Save As Layer Definition File..." ), QgisApp::instance(), SLOT( saveAsLayerDefinition() ) );

        if ( vlayer->dataProvider()->supportsSubsetString() )
        {
          QAction *action = menu->addAction( tr( "&Filter..." ), QgisApp::instance(), SLOT( layerSubsetString() ) );
          action->setEnabled( !vlayer->isEditable() );
        }

        menu->addAction( actions->actionShowFeatureCount( menu ) );

        menu->addSeparator();
      }
      else if ( rlayer )
      {
        menu->addAction( tr( "Save As..." ), QgisApp::instance(), SLOT( saveAsRasterFile() ) );
        menu->addAction( tr( "Save As Layer Definition File..." ), QgisApp::instance(), SLOT( saveAsLayerDefinition() ) );
      }
      else if ( layer && layer->type() == QgsMapLayer::PluginLayer && mView->selectedLayerNodes().count() == 1 )
      {
        // disable duplication of plugin layers
        duplicateLayersAction->setEnabled( false );
      }

      addCustomLayerActions( menu, layer );

      if ( layer && QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
        menu->addAction( tr( "&Properties" ), QgisApp::instance(), SLOT( layerProperties() ) );

      if ( node->parent() != mView->layerTreeModel()->rootGroup() )
        menu->addAction( actions->actionMakeTopLevel( menu ) );

      menu->addAction( actions->actionRenameGroupOrLayer( menu ) );

      if ( mView->selectedNodes( true ).count() >= 2 )
        menu->addAction( actions->actionGroupSelected( menu ) );

      if ( layer && layer->type() == QgsMapLayer::VectorLayer && static_cast<QgsVectorLayer*>( layer )->providerType() == "virtual" )
      {
        menu->addAction( tr( "Edit virtual layer settings" ), QgisApp::instance(), SLOT( addVirtualLayer() ) );
      }
    }

  }
  else if ( QgsLayerTreeModelLegendNode* node = mView->layerTreeModel()->index2legendNode( idx ) )
  {
    if ( QgsSymbolV2LegendNode* symbolNode = dynamic_cast< QgsSymbolV2LegendNode* >( node ) )
    {
      // symbology item
      if ( symbolNode->flags() & Qt::ItemIsUserCheckable )
      {
        menu->addAction( QgsApplication::getThemeIcon( "/mActionShowAllLayers.png" ), tr( "&Show All Items" ),
                         symbolNode, SLOT( checkAllItems() ) );
        menu->addAction( QgsApplication::getThemeIcon( "/mActionHideAllLayers.png" ), tr( "&Hide All Items" ),
                         symbolNode, SLOT( uncheckAllItems() ) );
        menu->addSeparator();
      }

      if ( symbolNode->symbol() )
      {
        QgsColorWheel* colorWheel = new QgsColorWheel( menu );
        colorWheel->setColor( symbolNode->symbol()->color() );
        QgsColorWidgetAction* colorAction = new QgsColorWidgetAction( colorWheel, menu, menu );
        colorAction->setDismissOnColorSelection( false );
        connect( colorAction, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setSymbolLegendNodeColor( const QColor& ) ) );
        //store the layer id and rule key in action, so we can later retrieve the corresponding
        //legend node, if it still exists
        colorAction->setProperty( "layerId", symbolNode->layerNode()->layerId() );
        colorAction->setProperty( "ruleKey", symbolNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString() );
        menu->addAction( colorAction );

        //add recent colors action
        QList<QgsRecentColorScheme *> recentSchemes;
        QgsColorSchemeRegistry::instance()->schemes( recentSchemes );
        if ( !recentSchemes.isEmpty() )
        {
          QgsColorSwatchGridAction* recentColorAction = new QgsColorSwatchGridAction( recentSchemes.at( 0 ), menu, "symbology", menu );
          recentColorAction->setProperty( "layerId", symbolNode->layerNode()->layerId() );
          recentColorAction->setProperty( "ruleKey", symbolNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString() );
          recentColorAction->setDismissOnColorSelection( false );
          menu->addAction( recentColorAction );
          connect( recentColorAction, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setSymbolLegendNodeColor( const QColor& ) ) );
        }

        menu->addSeparator();
      }

      QAction* editSymbolAction = new QAction( tr( "Edit Symbol..." ), menu );
      //store the layer id and rule key in action, so we can later retrieve the corresponding
      //legend node, if it still exists
      editSymbolAction->setProperty( "layerId", symbolNode->layerNode()->layerId() );
      editSymbolAction->setProperty( "ruleKey", symbolNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString() );
      connect( editSymbolAction, SIGNAL( triggered() ), this, SLOT( editSymbolLegendNodeSymbol() ) );
      menu->addAction( editSymbolAction );
    }
  }

  return menu;
}



void QgsAppLayerTreeViewMenuProvider::addLegendLayerAction( QAction* action, const QString& menu, const QString& id,
    QgsMapLayer::LayerType type, bool allLayers )
{
  mLegendLayerActionMap[type].append( LegendLayerAction( action, menu, id, allLayers ) );
}

bool QgsAppLayerTreeViewMenuProvider::removeLegendLayerAction( QAction* action )
{
  QMap< QgsMapLayer::LayerType, QList< LegendLayerAction > >::iterator it;
  for ( it = mLegendLayerActionMap.begin();
        it != mLegendLayerActionMap.end(); ++it )
  {
    for ( int i = 0; i < it->count(); i++ )
    {
      if (( *it )[i].action == action )
      {
        ( *it ).removeAt( i );
        return true;
      }
    }
  }
  return false;
}

void QgsAppLayerTreeViewMenuProvider::addLegendLayerActionForLayer( QAction* action, QgsMapLayer* layer )
{
  if ( !action || !layer )
    return;

  legendLayerActions( layer->type() );
  if ( !mLegendLayerActionMap.contains( layer->type() ) )
    return;

  QMap< QgsMapLayer::LayerType, QList< LegendLayerAction > >::iterator it
  = mLegendLayerActionMap.find( layer->type() );
  for ( int i = 0; i < it->count(); i++ )
  {
    if (( *it )[i].action == action )
    {
      ( *it )[i].layers.append( layer );
      return;
    }
  }
}

void QgsAppLayerTreeViewMenuProvider::removeLegendLayerActionsForLayer( QgsMapLayer* layer )
{
  if ( ! layer || ! mLegendLayerActionMap.contains( layer->type() ) )
    return;

  QMap< QgsMapLayer::LayerType, QList< LegendLayerAction > >::iterator it
  = mLegendLayerActionMap.find( layer->type() );
  for ( int i = 0; i < it->count(); i++ )
  {
    ( *it )[i].layers.removeAll( layer );
  }
}

QList< LegendLayerAction > QgsAppLayerTreeViewMenuProvider::legendLayerActions( QgsMapLayer::LayerType type ) const
{
#ifdef QGISDEBUG
  if ( mLegendLayerActionMap.contains( type ) )
  {
    QgsDebugMsg( QString( "legendLayerActions for layers of type %1:" ).arg( type ) );

    Q_FOREACH ( const LegendLayerAction& lyrAction, mLegendLayerActionMap[ type ] )
    {
      Q_UNUSED( lyrAction );
      QgsDebugMsg( QString( "%1/%2 - %3 layers" ).arg( lyrAction.menu, lyrAction.action->text() ).arg( lyrAction.layers.count() ) );
    }
  }
#endif

  return mLegendLayerActionMap.contains( type ) ? mLegendLayerActionMap.value( type ) : QList< LegendLayerAction >();
}

void QgsAppLayerTreeViewMenuProvider::addCustomLayerActions( QMenu* menu, QgsMapLayer* layer )
{
  if ( !layer )
    return;

  // add custom layer actions - should this go at end?
  QList< LegendLayerAction > lyrActions = legendLayerActions( layer->type() );

  if ( ! lyrActions.isEmpty() )
  {
    menu->addSeparator();
    QList<QMenu*> theMenus;
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
          QAction* before = nullptr;
          QMenu* newMenu = nullptr;
          QString dst = menuName;
          dst.remove( QChar( '&' ) );
          Q_FOREACH ( QMenu* menu, theMenus )
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
            theMenus.append( newMenu );
            // Where to put it? - we worked that out above...
            menu->insertMenu( before, newMenu );
          }
          // QMenu* menu = getMenu( lyrActions[i].menu, &theBeforeSep, &theAfterSep, &theMenu );
          newMenu->addAction( lyrActions[i].action );
        }
      }
    }
    menu->addSeparator();
  }
}

void QgsAppLayerTreeViewMenuProvider::editVectorSymbol()
{
  QAction* action = qobject_cast< QAction*>( sender() );
  if ( !action )
    return;

  QString layerId = action->property( "layerId" ).toString();
  QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
  if ( !layer )
    return;

  QgsSingleSymbolRendererV2* singleRenderer = dynamic_cast< QgsSingleSymbolRendererV2* >( layer->rendererV2() );
  if ( !singleRenderer )
    return;

  QScopedPointer< QgsSymbolV2 > symbol( singleRenderer->symbol() ? singleRenderer->symbol()->clone() : nullptr );
  QgsSymbolV2SelectorDialog dlg( symbol.data(), QgsStyleV2::defaultStyle(), layer, mView->window() );
  dlg.setWindowTitle( tr( "Symbol selector" ) );
  dlg.setMapCanvas( mCanvas );
  if ( dlg.exec() )
  {
    singleRenderer->setSymbol( symbol.take() );
    layer->triggerRepaint();
    mView->refreshLayerSymbology( layer->id() );
  }
}

void QgsAppLayerTreeViewMenuProvider::setVectorSymbolColor( const QColor& color )
{
  QAction* action = qobject_cast< QAction*>( sender() );
  if ( !action )
    return;

  QString layerId = action->property( "layerId" ).toString();
  QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
  if ( !layer )
    return;

  QgsSingleSymbolRendererV2* singleRenderer = dynamic_cast< QgsSingleSymbolRendererV2* >( layer->rendererV2() );
  QgsSymbolV2* newSymbol = nullptr;

  if ( singleRenderer && singleRenderer->symbol() )
    newSymbol = singleRenderer->symbol()->clone();

  const QgsSingleSymbolRendererV2* embeddedRenderer = nullptr;
  if ( !newSymbol && layer->rendererV2()->embeddedRenderer() )
  {
    embeddedRenderer = dynamic_cast< const QgsSingleSymbolRendererV2* >( layer->rendererV2()->embeddedRenderer() );
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
    QgsSingleSymbolRendererV2* newRenderer = embeddedRenderer->clone();
    newRenderer->setSymbol( newSymbol );
    layer->rendererV2()->setEmbeddedRenderer( newRenderer );
  }

  layer->triggerRepaint();
  layer->emitStyleChanged();
  mView->refreshLayerSymbology( layer->id() );
}

void QgsAppLayerTreeViewMenuProvider::editSymbolLegendNodeSymbol()
{
  QAction* action = qobject_cast< QAction*>( sender() );
  if ( !action )
    return;

  QString layerId = action->property( "layerId" ).toString();
  QString ruleKey = action->property( "ruleKey" ).toString();

  QgsSymbolV2LegendNode* node = dynamic_cast<QgsSymbolV2LegendNode*>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbolV2* originalSymbol = node->symbol();
  if ( !originalSymbol )
    return;

  QScopedPointer< QgsSymbolV2 > symbol( originalSymbol->clone() );
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( node->layerNode()->layer() );
  QgsSymbolV2SelectorDialog dlg( symbol.data(), QgsStyleV2::defaultStyle(), vlayer, mView->window() );
  dlg.setWindowTitle( tr( "Symbol selector" ) );
  dlg.setMapCanvas( mCanvas );
  if ( dlg.exec() )
  {
    node->setSymbol( symbol.take() );
    if ( vlayer )
    {
      vlayer->emitStyleChanged();
    }
  }
}

void QgsAppLayerTreeViewMenuProvider::setSymbolLegendNodeColor( const QColor &color )
{
  QAction* action = qobject_cast< QAction*>( sender() );
  if ( !action )
    return;

  QString layerId = action->property( "layerId" ).toString();
  QString ruleKey = action->property( "ruleKey" ).toString();

  QgsSymbolV2LegendNode* node = dynamic_cast<QgsSymbolV2LegendNode*>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbolV2* originalSymbol = node->symbol();
  if ( !originalSymbol )
    return;

  QgsSymbolV2* newSymbol = originalSymbol->clone();
  newSymbol->setColor( color );
  node->setSymbol( newSymbol );
  if ( QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) ) )
  {
    layer->emitStyleChanged();
  }
}
