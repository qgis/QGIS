#include "qgsapplayertreeviewmenuprovider.h"


#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsclipboard.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeviewdefaultactions.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"


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

    // TODO: expand all, collapse all
    // TODO: update drawing order
  }
  else if ( QgsLayerTreeNode* node = mView->layerTreeModel()->index2node( idx ) )
  {
    // layer or group selected
    if ( QgsLayerTree::isGroup( node ) )
    {
      menu->addAction( actions->actionZoomToGroup( mCanvas, menu ) );
      menu->addAction( actions->actionRemoveGroupOrLayer( menu ) );

      menu->addAction( QgsApplication::getThemeIcon( "/mActionSetCRS.png" ),
                       tr( "&Set Group CRS" ), QgisApp::instance(), SLOT( legendGroupSetCRS() ) );

      menu->addAction( actions->actionRenameGroupOrLayer( menu ) );

      if ( mView->selectedNodes( true ).count() >= 2 )
        menu->addAction( actions->actionGroupSelected( menu ) );

      menu->addAction( actions->actionAddGroup( menu ) );
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsMapLayer* layer = QgsLayerTree::toLayer( node )->layer();

      menu->addAction( actions->actionZoomToLayer( mCanvas, menu ) );
      menu->addAction( actions->actionShowInOverview( menu ) );

      if ( layer && layer->type() == QgsMapLayer::RasterLayer )
      {
        menu->addAction( tr( "&Zoom to Best Scale (100%)" ), QgisApp::instance(), SLOT( legendLayerZoomNative() ) );

        QgsRasterLayer* rasterLayer =  qobject_cast<QgsRasterLayer *>( layer );
        if ( rasterLayer && rasterLayer->rasterType() != QgsRasterLayer::Palette )
          menu->addAction( tr( "&Stretch Using Current Extent" ), QgisApp::instance(), SLOT( legendLayerStretchUsingCurrentExtent() ) );
      }

      menu->addAction( actions->actionRemoveGroupOrLayer( menu ) );

      // duplicate layer
      QAction* duplicateLayersAction = menu->addAction( QgsApplication::getThemeIcon( "/mActionDuplicateLayer.svg" ), tr( "&Duplicate" ), QgisApp::instance(), SLOT( duplicateLayers() ) );

      // set layer scale visibility
      menu->addAction( tr( "&Set Layer Scale Visibility" ), QgisApp::instance(), SLOT( setLayerScaleVisibility() ) );

      // set layer crs
      menu->addAction( QgsApplication::getThemeIcon( "/mActionSetCRS.png" ), tr( "&Set Layer CRS" ), QgisApp::instance(), SLOT( setLayerCRS() ) );

      // assign layer crs to project
      menu->addAction( QgsApplication::getThemeIcon( "/mActionSetProjectCRS.png" ), tr( "Set &Project CRS from Layer" ), QgisApp::instance(), SLOT( setProjectCRSFromLayer() ) );

      menu->addSeparator();

      if ( layer && layer->type() == QgsMapLayer::VectorLayer )
      {
        QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( layer );

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

        if ( !vlayer->isEditable() && vlayer->dataProvider()->supportsSubsetString() && vlayer->vectorJoins().isEmpty() )
          menu->addAction( tr( "&Filter..." ), QgisApp::instance(), SLOT( layerSubsetString() ) );

        menu->addAction( actions->actionShowFeatureCount( menu ) );

        menu->addSeparator();
      }
      else if ( layer && layer->type() == QgsMapLayer::RasterLayer )
      {
        menu->addAction( tr( "Save As..." ), QgisApp::instance(), SLOT( saveAsRasterFile() ) );
        menu->addAction( tr( "Save As Layer Definition File..." ), QgisApp::instance(), SLOT( saveAsLayerDefinition() ) );
      }
      else if ( layer && layer->type() == QgsMapLayer::PluginLayer && mView->selectedLayerNodes().count() == 1 )
      {
        // disable duplication of plugin layers
        duplicateLayersAction->setEnabled( false );
      }

      // TODO: custom actions

      if ( layer && QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
        menu->addAction( tr( "&Properties" ), QgisApp::instance(), SLOT( layerProperties() ) );

      if ( node->parent() != mView->layerTreeModel()->rootGroup() )
        menu->addAction( actions->actionMakeTopLevel( menu ) );

      menu->addAction( actions->actionRenameGroupOrLayer( menu ) );

      if ( mView->selectedNodes( true ).count() >= 2 )
        menu->addAction( actions->actionGroupSelected( menu ) );

      if ( mView->selectedLayerNodes().count() == 1 )
      {
        QgisApp* app = QgisApp::instance();
        menu->addAction( tr( "Copy Style" ), app, SLOT( copyStyle() ) );
        if ( app->clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
        {
          menu->addAction( tr( "Paste Style" ), app, SLOT( pasteStyle() ) );
        }
      }
    }

  }
  else
  {
    // symbology item?
  }

  return menu;
}

