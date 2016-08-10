/***************************************************************************
  qgsmapthemes.cpp
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

#include "qgsmapthemes.h"
#include "qgsmapthemecollection.h"

#include "qgslayertree.h"
#include "qgslayertreemapcanvasbridge.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeview.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"
#include "qgsnewnamedialog.h"

#include <QInputDialog>


QgsMapThemes* QgsMapThemes::sInstance;


QgsMapThemes::QgsMapThemes()
    : mMenu( new QMenu )
{

  mMenu->addAction( QgisApp::instance()->actionShowAllLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideAllLayers() );
  mMenu->addAction( QgisApp::instance()->actionShowSelectedLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideSelectedLayers() );
  mMenu->addSeparator();

  mReplaceMenu = new QMenu( tr( "Replace Theme" ) );
  mMenu->addMenu( mReplaceMenu );
  mActionAddPreset = mMenu->addAction( tr( "Add Theme..." ), this, SLOT( addPreset() ) );
  mMenuSeparator = mMenu->addSeparator();

  mActionRemoveCurrentPreset = mMenu->addAction( tr( "Remove Current Theme" ), this, SLOT( removeCurrentPreset() ) );

  connect( mMenu, SIGNAL( aboutToShow() ), this, SLOT( menuAboutToShow() ) );
}

void QgsMapThemes::addPerLayerCheckedLegendSymbols( QgsMapThemeCollection::PresetRecord& rec )
{
  QgsLayerTreeModel* model = QgisApp::instance()->layerTreeView()->layerTreeModel();

  Q_FOREACH ( const QString& layerID, rec.mVisibleLayerIDs )
  {
    QgsLayerTreeLayer* nodeLayer = model->rootGroup()->findLayer( layerID );
    if ( !nodeLayer )
      continue;

    bool hasCheckableItems = false;
    bool someItemsUnchecked = false;
    QSet<QString> checkedItems;
    Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, model->layerLegendNodes( nodeLayer ) )
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

void QgsMapThemes::addPerLayerCurrentStyle( QgsMapThemeCollection::PresetRecord& rec )
{
  QgsLayerTreeModel* model = QgisApp::instance()->layerTreeView()->layerTreeModel();

  Q_FOREACH ( const QString& layerID, rec.mVisibleLayerIDs )
  {
    QgsLayerTreeLayer* nodeLayer = model->rootGroup()->findLayer( layerID );
    if ( !nodeLayer )
      continue;

    rec.mPerLayerCurrentStyle[layerID] = nodeLayer->layer()->styleManager()->currentStyle();
  }
}

QgsMapThemeCollection::PresetRecord QgsMapThemes::currentState()
{
  QgsMapThemeCollection::PresetRecord rec;
  QgsLayerTreeGroup* root = QgsProject::instance()->layerTreeRoot();
  QgsMapThemeCollection::addVisibleLayersToPreset( root, rec );
  addPerLayerCheckedLegendSymbols( rec );
  addPerLayerCurrentStyle( rec );
  return rec;
}

QgsMapThemes* QgsMapThemes::instance()
{
  if ( !sInstance )
    sInstance = new QgsMapThemes();

  return sInstance;
}

void QgsMapThemes::addPreset( const QString& name )
{
  QgsProject::instance()->mapThemeCollection()->insert( name, currentState() );
}

void QgsMapThemes::updatePreset( const QString& name )
{
  QgsProject::instance()->mapThemeCollection()->update( name, currentState() );
}

QStringList QgsMapThemes::orderedPresetVisibleLayers( const QString& name ) const
{
  QStringList visibleIds = QgsProject::instance()->mapThemeCollection()->presetVisibleLayers( name );

  // also make sure to order the layers according to map canvas order
  QgsLayerTreeMapCanvasBridge* bridge = QgisApp::instance()->layerTreeCanvasBridge();
  QStringList order = bridge->hasCustomLayerOrder() ? bridge->customLayerOrder() : bridge->defaultLayerOrder();
  QStringList order2;
  Q_FOREACH ( const QString& layerID, order )
  {
    if ( visibleIds.contains( layerID ) )
      order2 << layerID;
  }

  return order2;
}

QMenu* QgsMapThemes::menu()
{
  return mMenu;
}


void QgsMapThemes::addPreset()
{
  QStringList existingNames = QgsProject::instance()->mapThemeCollection()->presets();
  QgsNewNameDialog dlg( tr( "theme" ) , tr( "Theme" ), QStringList(), existingNames, QRegExp(), Qt::CaseInsensitive, mMenu );
  dlg.setWindowTitle( tr( "Map Themes" ) );
  dlg.setHintString( tr( "Name of the new theme" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setConflictingNameWarning( tr( "A theme with this name already exists" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name().isEmpty() )
    return;

  addPreset( dlg.name() );
}


void QgsMapThemes::presetTriggered()
{
  QAction* actionPreset = qobject_cast<QAction*>( sender() );
  if ( !actionPreset )
    return;

  applyState( actionPreset->text() );
}

void QgsMapThemes::replaceTriggered()
{
  QAction* actionPreset = qobject_cast<QAction*>( sender() );
  if ( !actionPreset )
    return;

  //adding preset with same name is effectively a replace
  addPreset( actionPreset->text() );
}

void QgsMapThemes::applyStateToLayerTreeGroup( QgsLayerTreeGroup* parent, const QgsMapThemeCollection::PresetRecord& rec )
{
  Q_FOREACH ( QgsLayerTreeNode* node, parent->children() )
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
          Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, model->layerLegendNodes( nodeLayer ) )
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
          Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, model->layerLegendNodes( nodeLayer ) )
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


void QgsMapThemes::applyState( const QString& presetName )
{
  if ( !QgsProject::instance()->mapThemeCollection()->hasPreset( presetName ) )
    return;

  applyStateToLayerTreeGroup( QgsProject::instance()->layerTreeRoot(), QgsProject::instance()->mapThemeCollection()->presetState( presetName ) );

  // also make sure that the preset is up-to-date (not containing any non-existent legend items)
  QgsProject::instance()->mapThemeCollection()->update( presetName, currentState() );
}

void QgsMapThemes::removeCurrentPreset()
{
  Q_FOREACH ( QAction* a, mMenuPresetActions )
  {
    if ( a->isChecked() )
    {
      QgsProject::instance()->mapThemeCollection()->removePreset( a->text() );
      break;
    }
  }
}

void QgsMapThemes::menuAboutToShow()
{
  qDeleteAll( mMenuPresetActions );
  mMenuPresetActions.clear();
  mReplaceMenu->clear();
  qDeleteAll( mMenuReplaceActions );
  mMenuReplaceActions.clear();

  QgsMapThemeCollection::PresetRecord rec = currentState();
  bool hasCurrent = false;

  Q_FOREACH ( const QString& grpName, QgsProject::instance()->mapThemeCollection()->presets() )
  {
    QAction* a = new QAction( grpName, mMenu );
    a->setCheckable( true );
    if ( !hasCurrent && rec == QgsProject::instance()->mapThemeCollection()->presetState( grpName ) )
    {
      a->setChecked( true );
      hasCurrent = true;
    }
    connect( a, SIGNAL( triggered() ), this, SLOT( presetTriggered() ) );
    mMenuPresetActions.append( a );

    QAction* replaceAction = new QAction( grpName, mReplaceMenu );
    connect( replaceAction, SIGNAL( triggered() ), this, SLOT( replaceTriggered() ) );
    mReplaceMenu->addAction( replaceAction );
  }
  mMenu->insertActions( mMenuSeparator, mMenuPresetActions );
  mReplaceMenu->addActions( mMenuReplaceActions );

  mActionAddPreset->setEnabled( !hasCurrent );
  mActionRemoveCurrentPreset->setEnabled( hasCurrent );
}
