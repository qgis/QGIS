#include "qgsapplayertreeviewmenuprovider.h"


#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsclipboard.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeviewdefaultactions.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslayertreeregistrybridge.h"


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

      if ( mView->selectedNodes( true ).count() >= 2 )
        menu->addAction( actions->actionGroupSelected( menu ) );

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
        menu->addAction( tr( "&Zoom to Best Scale (100%)" ), QgisApp::instance(), SLOT( legendLayerZoomNative() ) );

        if ( rlayer->rasterType() != QgsRasterLayer::Palette )
          menu->addAction( tr( "&Stretch Using Current Extent" ), QgisApp::instance(), SLOT( legendLayerStretchUsingCurrentExtent() ) );
      }

      menu->addAction( QgsApplication::getThemeIcon( "/mActionRemoveLayer.svg" ), tr( "&Remove" ), QgisApp::instance(), SLOT( removeLayer() ) );

      // duplicate layer
      QAction* duplicateLayersAction = menu->addAction( QgsApplication::getThemeIcon( "/mActionDuplicateLayer.svg" ), tr( "&Duplicate" ), QgisApp::instance(), SLOT( duplicateLayers() ) );

      if ( !vlayer || vlayer->geometryType() != QGis::NoGeometry )
      {
        // set layer scale visibility
        menu->addAction( tr( "&Set Layer Scale Visibility" ), QgisApp::instance(), SLOT( setLayerScaleVisibility() ) );

        // set layer crs
        menu->addAction( QgsApplication::getThemeIcon( "/mActionSetCRS.png" ), tr( "&Set Layer CRS" ), QgisApp::instance(), SLOT( setLayerCRS() ) );

        // assign layer crs to project
        menu->addAction( QgsApplication::getThemeIcon( "/mActionSetProjectCRS.png" ), tr( "Set &Project CRS from Layer" ), QgisApp::instance(), SLOT( setProjectCRSFromLayer() ) );
      }

      // style-related actions
      if ( layer && mView->selectedLayerNodes().count() == 1 )
      {
        QMenu *menuStyleManager = new QMenu( tr( "Styles" ) );

        QgisApp *app = QgisApp::instance();
        menuStyleManager->addAction( tr( "Copy Style" ), app, SLOT( copyStyle() ) );
        if ( app->clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
        {
          menuStyleManager->addAction( tr( "Paste Style" ), app, SLOT( pasteStyle() ) );
        }

        menuStyleManager->addSeparator();
        QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( menuStyleManager, layer );

        menu->addMenu( menuStyleManager );
      }

      menu->addSeparator();

      if ( vlayer )
      {
        QAction *toggleEditingAction = QgisApp::instance()->actionToggleEditing();
        QAction *saveLayerEditsAction = QgisApp::instance()->actionSaveActiveLayerEdits();
        QAction *allEditsAction = QgisApp::instance()->actionAllEdits();

        // attribute table
        menu->addAction( QgsApplication::getThemeIcon( "/mActionOpenTable.png" ), tr( "&Open Attribute Table" ),
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
    }

  }
  else
  {
    // symbology item?
  }

  return menu;
}



void QgsAppLayerTreeViewMenuProvider::addLegendLayerAction( QAction* action, QString menu, QString id,
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
      QgsDebugMsg( QString( "%1/%2 - %3 layers" ).arg( lyrAction.menu ).arg( lyrAction.action->text() ).arg( lyrAction.layers.count() ) );
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
          QAction* before = 0;
          QMenu* newMenu = 0;
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
