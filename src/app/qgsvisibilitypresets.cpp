/***************************************************************************
  qgsvisibilitypresets.cpp
  --------------------------------------
  Date                 : September 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvisibilitypresets.h"
#include "qgsvisibilitypresetcollection.h"

#include "qgslayertree.h"
#include "qgslayertreemapcanvasbridge.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeview.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsproject.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"
#include "qgsnewnamedialog.h"

#include <QInputDialog>


QgsVisibilityPresets* QgsVisibilityPresets::sInstance;


QgsVisibilityPresets::QgsVisibilityPresets()
    : mMenu( new QMenu )
{

  mMenu->addAction( QgisApp::instance()->actionShowAllLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideAllLayers() );
  mMenu->addAction( QgisApp::instance()->actionShowSelectedLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideSelectedLayers() );
  mMenu->addSeparator();

  mReplaceMenu = new QMenu( tr( "Replace Preset" ) );
  mMenu->addMenu( mReplaceMenu );
  mActionAddPreset = mMenu->addAction( tr( "Add Preset..." ), this, SLOT( addPreset() ) );
  mMenuSeparator = mMenu->addSeparator();

  mActionRemoveCurrentPreset = mMenu->addAction( tr( "Remove Current Preset" ), this, SLOT( removeCurrentPreset() ) );

  connect( mMenu, SIGNAL( aboutToShow() ), this, SLOT( menuAboutToShow() ) );
}

void QgsVisibilityPresets::addPerLayerCheckedLegendSymbols( QgsVisibilityPresetCollection::PresetRecord& rec )
{
  QgsLayerTreeModel* model = QgisApp::instance()->layerTreeView()->layerTreeModel();

  foreach ( QString layerID, rec.mVisibleLayerIDs )
  {
    QgsLayerTreeLayer* nodeLayer = model->rootGroup()->findLayer( layerID );
    if ( !nodeLayer )
      continue;

    bool hasCheckableItems = false;
    bool someItemsUnchecked = false;
    QSet<QString> checkedItems;
    foreach ( QgsLayerTreeModelLegendNode* legendNode, model->layerLegendNodes( nodeLayer ) )
    {
      if ( legendNode->flags() & Qt::ItemIsUserCheckable )
      {
        hasCheckableItems = true;

        if ( legendNode->data( Qt::CheckStateRole ).toInt() == Qt::Checked )
          checkedItems << legendNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString();
        else
          someItemsUnchecked = true;
      }
    }

    if ( hasCheckableItems && someItemsUnchecked )
      rec.mPerLayerCheckedLegendSymbols.insert( nodeLayer->layerId(), checkedItems );
  }
}

void QgsVisibilityPresets::addPerLayerCurrentStyle( QgsVisibilityPresetCollection::PresetRecord& rec )
{
  QgsLayerTreeModel* model = QgisApp::instance()->layerTreeView()->layerTreeModel();

  foreach ( QString layerID, rec.mVisibleLayerIDs )
  {
    QgsLayerTreeLayer* nodeLayer = model->rootGroup()->findLayer( layerID );
    if ( !nodeLayer )
      continue;

    rec.mPerLayerCurrentStyle[layerID] = nodeLayer->layer()->styleManager()->currentStyle();
  }
}

QgsVisibilityPresetCollection::PresetRecord QgsVisibilityPresets::currentState()
{
  QgsVisibilityPresetCollection::PresetRecord rec;
  QgsLayerTreeGroup* root = QgsProject::instance()->layerTreeRoot();
  QgsVisibilityPresetCollection::addVisibleLayersToPreset( root, rec );
  addPerLayerCheckedLegendSymbols( rec );
  addPerLayerCurrentStyle( rec );
  return rec;
}

QgsVisibilityPresets* QgsVisibilityPresets::instance()
{
  if ( !sInstance )
    sInstance = new QgsVisibilityPresets();

  return sInstance;
}

void QgsVisibilityPresets::addPreset( const QString& name )
{
  QgsProject::instance()->visibilityPresetCollection()->insert( name, currentState() );
}

void QgsVisibilityPresets::updatePreset( const QString& name )
{
  QgsProject::instance()->visibilityPresetCollection()->update( name, currentState() );
}

QStringList QgsVisibilityPresets::orderedPresetVisibleLayers( const QString& name ) const
{
  QStringList visibleIds = QgsProject::instance()->visibilityPresetCollection()->presetVisibleLayers( name );

  // also make sure to order the layers according to map canvas order
  QgsLayerTreeMapCanvasBridge* bridge = QgisApp::instance()->layerTreeCanvasBridge();
  QStringList order = bridge->hasCustomLayerOrder() ? bridge->customLayerOrder() : bridge->defaultLayerOrder();
  QStringList order2;
  foreach ( QString layerID, order )
  {
    if ( visibleIds.contains( layerID ) )
      order2 << layerID;
  }

  return order2;
}

QMenu* QgsVisibilityPresets::menu()
{
  return mMenu;
}


void QgsVisibilityPresets::addPreset()
{
  QStringList existingNames = QgsProject::instance()->visibilityPresetCollection()->presets();
  QgsNewNameDialog dlg( tr( "preset" ) , tr( "Preset" ), QStringList(), existingNames, QRegExp(), Qt::CaseInsensitive, mMenu );
  dlg.setWindowTitle( tr( "Visibility Presets" ) );
  dlg.setHintString( tr( "Name of the new preset" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setConflictingNameWarning( tr( "A preset with this name already exists" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name().isEmpty() )
    return;

  addPreset( dlg.name() );
}


void QgsVisibilityPresets::presetTriggered()
{
  QAction* actionPreset = qobject_cast<QAction*>( sender() );
  if ( !actionPreset )
    return;

  applyState( actionPreset->text() );
}

void QgsVisibilityPresets::replaceTriggered()
{
  QAction* actionPreset = qobject_cast<QAction*>( sender() );
  if ( !actionPreset )
    return;

  //adding preset with same name is effectively a replace
  addPreset( actionPreset->text() );
}

void QgsVisibilityPresets::applyStateToLayerTreeGroup( QgsLayerTreeGroup* parent, const QgsVisibilityPresetCollection::PresetRecord& rec )
{
  foreach ( QgsLayerTreeNode* node, parent->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      applyStateToLayerTreeGroup( QgsLayerTree::toGroup( node ), rec );
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
      bool isVisible = rec.mVisibleLayerIDs.contains( nodeLayer->layerId() );
      nodeLayer->setVisible( isVisible ? Qt::Checked : Qt::Unchecked );

      if ( isVisible )
      {
        if ( rec.mPerLayerCurrentStyle.contains( nodeLayer->layerId() ) )
        {
          // apply desired style first
          nodeLayer->layer()->styleManager()->setCurrentStyle( rec.mPerLayerCurrentStyle[nodeLayer->layerId()] );
        }

        QgsLayerTreeModel* model = QgisApp::instance()->layerTreeView()->layerTreeModel();
        if ( rec.mPerLayerCheckedLegendSymbols.contains( nodeLayer->layerId() ) )
        {
          const QSet<QString>& checkedNodes = rec.mPerLayerCheckedLegendSymbols[nodeLayer->layerId()];
          // some nodes are not checked
          foreach ( QgsLayerTreeModelLegendNode* legendNode, model->layerLegendNodes( nodeLayer ) )
          {
            Qt::CheckState shouldHaveState = checkedNodes.contains( legendNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString() ) ? Qt::Checked : Qt::Unchecked;
            if (( legendNode->flags() & Qt::ItemIsUserCheckable ) &&
                legendNode->data( Qt::CheckStateRole ).toInt() != shouldHaveState )
              legendNode->setData( shouldHaveState, Qt::CheckStateRole );
          }
        }
        else
        {
          // all nodes should be checked
          foreach ( QgsLayerTreeModelLegendNode* legendNode, model->layerLegendNodes( nodeLayer ) )
          {
            if (( legendNode->flags() & Qt::ItemIsUserCheckable ) &&
                legendNode->data( Qt::CheckStateRole ).toInt() != Qt::Checked )
              legendNode->setData( Qt::Checked, Qt::CheckStateRole );
          }
        }
      }
    }
  }
}


void QgsVisibilityPresets::applyState( const QString& presetName )
{
  if ( !QgsProject::instance()->visibilityPresetCollection()->hasPreset( presetName ) )
    return;

  applyStateToLayerTreeGroup( QgsProject::instance()->layerTreeRoot(), QgsProject::instance()->visibilityPresetCollection()->presetState( presetName ) );

  // also make sure that the preset is up-to-date (not containing any non-existent legend items)
  QgsProject::instance()->visibilityPresetCollection()->update( presetName, currentState() );
}

void QgsVisibilityPresets::removeCurrentPreset()
{
  foreach ( QAction* a, mMenuPresetActions )
  {
    if ( a->isChecked() )
    {
      QgsProject::instance()->visibilityPresetCollection()->removePreset( a->text() );
      break;
    }
  }
}

void QgsVisibilityPresets::menuAboutToShow()
{
  qDeleteAll( mMenuPresetActions );
  mMenuPresetActions.clear();
  mReplaceMenu->clear();
  qDeleteAll( mMenuReplaceActions );
  mMenuReplaceActions.clear();

  QgsVisibilityPresetCollection::PresetRecord rec = currentState();
  bool hasCurrent = false;

  foreach ( const QString& grpName, QgsProject::instance()->visibilityPresetCollection()->presets() )
  {
    QAction* a = new QAction( grpName, mMenu );
    a->setCheckable( true );
    if ( !hasCurrent && rec == QgsProject::instance()->visibilityPresetCollection()->presetState( grpName ) )
    {
      a->setChecked( true );
      hasCurrent = true;
    }
    connect( a, SIGNAL( triggered() ), this, SLOT( presetTriggered() ) );
    mMenuPresetActions.append( a );

    QAction* replaceAction = new QAction( grpName, mReplaceMenu );
    replaceAction->setEnabled( !a->isChecked() ); //can't replace current preset
    connect( replaceAction, SIGNAL( triggered() ), this, SLOT( replaceTriggered() ) );
    mReplaceMenu->addAction( replaceAction );
  }
  mMenu->insertActions( mMenuSeparator, mMenuPresetActions );
  mReplaceMenu->addActions( mMenuReplaceActions );

  mActionAddPreset->setEnabled( !hasCurrent );
  mActionRemoveCurrentPreset->setEnabled( hasCurrent );
}
