/***************************************************************************
  qgsvisibilitygroups.cpp
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

#include "qgsvisibilitygroups.h"

#include "qgslayertree.h"
#include "qgslayertreemapcanvasbridge.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeview.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"

#include <QInputDialog>


QgsVisibilityGroups* QgsVisibilityGroups::sInstance;


QgsVisibilityGroups::QgsVisibilityGroups()
    : mMenu( new QMenu )
{

  mMenu->addAction( QgisApp::instance()->actionShowAllLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideAllLayers() );
  mMenu->addSeparator();

  mMenu->addAction( tr( "Add group..." ), this, SLOT( addGroup() ) );
  mMenuSeparator = mMenu->addSeparator();

  mActionRemoveCurrentGroup = mMenu->addAction( tr( "Remove current group" ), this, SLOT( removeCurrentGroup() ) );

  connect( mMenu, SIGNAL( aboutToShow() ), this, SLOT( menuAboutToShow() ) );

  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersRemoved( QStringList ) ),
           this, SLOT( registryLayersRemoved( QStringList ) ) );

  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ),
           this, SLOT( readProject( const QDomDocument & ) ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ),
           this, SLOT( writeProject( QDomDocument & ) ) );
}

void QgsVisibilityGroups::addVisibleLayersToGroup( QgsLayerTreeGroup* parent, QgsVisibilityGroups::GroupRecord& rec )
{
  foreach ( QgsLayerTreeNode* node, parent->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      addVisibleLayersToGroup( QgsLayerTree::toGroup( node ), rec );
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->isVisible() )
      {
        rec.mVisibleLayerIDs << nodeLayer->layerId();

        QgsLayerTreeModel* model = QgisApp::instance()->layerTreeView()->layerTreeModel();
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
  }
}

QgsVisibilityGroups::GroupRecord QgsVisibilityGroups::currentState()
{
  GroupRecord rec;
  QgsLayerTreeGroup* root = QgsProject::instance()->layerTreeRoot();
  addVisibleLayersToGroup( root, rec );
  return rec;
}


QgsVisibilityGroups* QgsVisibilityGroups::instance()
{
  if ( !sInstance )
    sInstance = new QgsVisibilityGroups();

  return sInstance;
}

void QgsVisibilityGroups::addGroup( const QString& name )
{
  mGroups.insert( name, currentState() );
}

void QgsVisibilityGroups::updateGroup( const QString& name )
{
  if ( !mGroups.contains( name ) )
    return;

  mGroups[name] = currentState();
}

void QgsVisibilityGroups::removeGroup( const QString& name )
{
  mGroups.remove( name );
}

void QgsVisibilityGroups::clear()
{
  mGroups.clear();
}

QStringList QgsVisibilityGroups::groups() const
{
  return mGroups.keys();
}

QStringList QgsVisibilityGroups::groupVisibleLayers( const QString& name ) const
{
  QSet<QString> visibleIds = mGroups.value( name ).mVisibleLayerIDs;

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


void QgsVisibilityGroups::applyGroupCheckedLegendNodesToLayer( const QString& name, const QString& layerID )
{
  if ( !mGroups.contains( name ) )
    return;

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID ) );
  if ( !vlayer || !vlayer->rendererV2() )
    return;

  if ( !vlayer->rendererV2()->legendSymbolItemsCheckable() )
    return; // no need to do anything

  const GroupRecord& rec = mGroups[name];
  bool someNodesUnchecked = rec.mPerLayerCheckedLegendSymbols.contains( layerID );

  foreach ( const QgsLegendSymbolItemV2& item, vlayer->rendererV2()->legendSymbolItemsV2() )
  {
    bool checked = vlayer->rendererV2()->legendSymbolItemChecked( item.ruleKey() );
    bool shouldBeChecked = someNodesUnchecked ? rec.mPerLayerCheckedLegendSymbols[layerID].contains( item.ruleKey() ) : true;
    if ( checked != shouldBeChecked )
      vlayer->rendererV2()->checkLegendSymbolItem( item.ruleKey(), shouldBeChecked );
  }
}

QMenu* QgsVisibilityGroups::menu()
{
  return mMenu;
}


void QgsVisibilityGroups::addGroup()
{
  bool ok;
  QString name = QInputDialog::getText( 0, tr( "Visibility groups" ), tr( "Name of the new group" ), QLineEdit::Normal, QString(), &ok );
  if ( !ok && name.isEmpty() )
    return;

  addGroup( name );
}


void QgsVisibilityGroups::groupTriggerred()
{
  QAction* actionGroup = qobject_cast<QAction*>( sender() );
  if ( !actionGroup )
    return;

  applyState( actionGroup->text() );
}


void QgsVisibilityGroups::applyStateToLayerTreeGroup( QgsLayerTreeGroup* parent, const GroupRecord& rec )
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


void QgsVisibilityGroups::applyState( const QString& groupName )
{
  if ( !mGroups.contains( groupName ) )
    return;

  applyStateToLayerTreeGroup( QgsProject::instance()->layerTreeRoot(), mGroups[groupName] );

  // also make sure that the group is up-to-date (not containing any non-existant legend items)
  if ( mGroups[groupName] == currentState() )
    return; // no need for update

  GroupRecord& rec = mGroups[groupName];
  foreach ( QString layerID, rec.mPerLayerCheckedLegendSymbols.keys() )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID ) );
    if ( !vl || !vl->rendererV2() )
      continue;

    QSet<QString> validRuleKeys;
    foreach ( const QgsLegendSymbolItemV2& item, vl->rendererV2()->legendSymbolItemsV2() )
      validRuleKeys << item.ruleKey();

    QSet<QString> invalidRuleKeys;
    foreach ( QString ruleKey, rec.mPerLayerCheckedLegendSymbols[layerID] )
      if ( !validRuleKeys.contains( ruleKey ) )
        invalidRuleKeys << ruleKey;

    foreach ( QString invalidRuleKey, invalidRuleKeys )
      rec.mPerLayerCheckedLegendSymbols[layerID].remove( invalidRuleKey );
  }
}


void QgsVisibilityGroups::removeCurrentGroup()
{
  foreach ( QAction* a, mMenuGroupActions )
  {
    if ( a->isChecked() )
    {
      removeGroup( a->text() );
      break;
    }
  }
}


void QgsVisibilityGroups::menuAboutToShow()
{
  qDeleteAll( mMenuGroupActions );
  mMenuGroupActions.clear();

  GroupRecord rec = currentState();
  bool hasCurrent = false;

  foreach ( const QString& grpName, mGroups.keys() )
  {
    QAction* a = new QAction( grpName, mMenu );
    a->setCheckable( true );
    if ( rec == mGroups[grpName] )
    {
      a->setChecked( true );
      hasCurrent = true;
    }
    connect( a, SIGNAL( triggered() ), this, SLOT( groupTriggerred() ) );
    mMenuGroupActions.append( a );
  }
  mMenu->insertActions( mMenuSeparator, mMenuGroupActions );

  mActionRemoveCurrentGroup->setEnabled( hasCurrent );
}


void QgsVisibilityGroups::readProject( const QDomDocument& doc )
{
  clear();

  QDomElement visGroupsElem = doc.firstChildElement( "qgis" ).firstChildElement( "visibility-groups" );
  if ( visGroupsElem.isNull() )
    return;

  QDomElement visGroupElem = visGroupsElem.firstChildElement( "visibility-group" );
  while ( !visGroupElem.isNull() )
  {
    QString groupName = visGroupElem.attribute( "name" );
    GroupRecord rec;
    QDomElement visGroupLayerElem = visGroupElem.firstChildElement( "layer" );
    while ( !visGroupLayerElem.isNull() )
    {
      QString layerID = visGroupLayerElem.attribute( "id" );
      if ( QgsMapLayerRegistry::instance()->mapLayer( layerID ) )
        rec.mVisibleLayerIDs << layerID; // only use valid layer IDs
      visGroupLayerElem = visGroupLayerElem.nextSiblingElement( "layer" );
    }

    QDomElement checkedLegendNodesElem = visGroupElem.firstChildElement( "checked-legend-nodes" );
    while ( !checkedLegendNodesElem.isNull() )
    {
      QSet<QString> checkedLegendNodes;

      QDomElement checkedLegendNodeElem = checkedLegendNodesElem.firstChildElement( "checked-legend-node" );
      while ( !checkedLegendNodeElem.isNull() )
      {
        checkedLegendNodes << checkedLegendNodeElem.attribute( "id" );
        checkedLegendNodeElem = checkedLegendNodeElem.nextSiblingElement( "checked-legend-node" );
      }

      QString layerID = checkedLegendNodesElem.attribute( "id" );
      if ( QgsMapLayerRegistry::instance()->mapLayer( layerID ) ) // only use valid IDs
        rec.mPerLayerCheckedLegendSymbols.insert( layerID, checkedLegendNodes );
      checkedLegendNodesElem = checkedLegendNodesElem.nextSiblingElement( "checked-legend-nodes" );
    }

    mGroups.insert( groupName, rec );

    visGroupElem = visGroupElem.nextSiblingElement( "visibility-group" );
  }
}

void QgsVisibilityGroups::writeProject( QDomDocument& doc )
{
  QDomElement visGroupsElem = doc.createElement( "visibility-groups" );
  foreach ( const QString& grpName, mGroups.keys() )
  {
    const GroupRecord& rec = mGroups[grpName];
    QDomElement visGroupElem = doc.createElement( "visibility-group" );
    visGroupElem.setAttribute( "name", grpName );
    foreach ( QString layerID, rec.mVisibleLayerIDs )
    {
      QDomElement layerElem = doc.createElement( "layer" );
      layerElem.setAttribute( "id", layerID );
      visGroupElem.appendChild( layerElem );
    }

    foreach ( QString layerID, rec.mPerLayerCheckedLegendSymbols.keys() )
    {
      QDomElement checkedLegendNodesElem = doc.createElement( "checked-legend-nodes" );
      checkedLegendNodesElem.setAttribute( "id", layerID );
      foreach ( QString checkedLegendNode, rec.mPerLayerCheckedLegendSymbols[layerID] )
      {
        QDomElement checkedLegendNodeElem = doc.createElement( "checked-legend-node" );
        checkedLegendNodeElem.setAttribute( "id", checkedLegendNode );
        checkedLegendNodesElem.appendChild( checkedLegendNodeElem );
      }
      visGroupElem.appendChild( checkedLegendNodesElem );
    }

    visGroupsElem.appendChild( visGroupElem );
  }

  doc.firstChildElement( "qgis" ).appendChild( visGroupsElem );
}

void QgsVisibilityGroups::registryLayersRemoved( QStringList layerIDs )
{
  foreach ( QString layerID, layerIDs )
  {
    foreach ( QString groupName, mGroups.keys() )
    {
      GroupRecord& rec = mGroups[groupName];
      rec.mVisibleLayerIDs.remove( layerID );
      rec.mPerLayerCheckedLegendSymbols.remove( layerID );
    }
  }
}
