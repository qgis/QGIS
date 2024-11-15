/***************************************************************************
  qgsmapthemecollection.cpp
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

#include "qgsmapthemecollection.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "moc_qgsmapthemecollection.cpp"

QgsMapThemeCollection::QgsMapThemeCollection( QgsProject *project )
  : mProject( project )
{
  connect( project, static_cast<void ( QgsProject::* )( const QStringList & )>( &QgsProject::layersWillBeRemoved ), this, &QgsMapThemeCollection::registryLayersRemoved );
}

QgsMapThemeCollection::MapThemeLayerRecord QgsMapThemeCollection::createThemeLayerRecord( QgsLayerTreeLayer *nodeLayer, QgsLayerTreeModel *model )
{
  MapThemeLayerRecord layerRec( nodeLayer->layer() );
  layerRec.isVisible = nodeLayer->isVisible();
  layerRec.usingCurrentStyle = true;
  layerRec.currentStyle = nodeLayer->layer()->styleManager()->currentStyle();
  layerRec.expandedLayerNode = nodeLayer->isExpanded();
  const QStringList expandedLegendNodes = nodeLayer->customProperty( QStringLiteral( "expandedLegendNodes" ) ).toStringList();
  layerRec.expandedLegendItems = QSet<QString>( expandedLegendNodes.begin(), expandedLegendNodes.end() );

  // get checked legend items
  bool hasCheckableItems = false;
  bool someItemsUnchecked = false;
  QSet<QString> checkedItems;
  const QList<QgsLayerTreeModelLegendNode *> layerLegendNodes = model->layerLegendNodes( nodeLayer, true );
  for ( QgsLayerTreeModelLegendNode *legendNode : layerLegendNodes )
  {
    if ( legendNode->flags() & Qt::ItemIsUserCheckable )
    {
      hasCheckableItems = true;

      if ( legendNode->data( Qt::CheckStateRole ).toInt() == Qt::Checked )
        checkedItems << legendNode->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString();
      else
        someItemsUnchecked = true;
    }
  }

  if ( hasCheckableItems && someItemsUnchecked )
  {
    layerRec.usingLegendItems = true;
    layerRec.checkedLegendItems = checkedItems;
  }
  return layerRec;
}

static QString _groupId( QgsLayerTreeNode *node )
{
  QStringList lst;
  while ( node->parent() )
  {
    lst.prepend( node->name() );
    node = node->parent();
  }
  return lst.join( '/' );
}

void QgsMapThemeCollection::createThemeFromCurrentState( QgsLayerTreeGroup *parent, QgsLayerTreeModel *model, QgsMapThemeCollection::MapThemeRecord &rec )
{
  const QList<QgsLayerTreeNode *> constChildren = parent->children();
  for ( QgsLayerTreeNode *node : constChildren )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      createThemeFromCurrentState( QgsLayerTree::toGroup( node ), model, rec );
      if ( node->isExpanded() )
        rec.mExpandedGroupNodes.insert( _groupId( node ) );
      if ( node->itemVisibilityChecked() != Qt::Unchecked )
        rec.mCheckedGroupNodes.insert( _groupId( node ) );
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
      if ( node->itemVisibilityChecked() != Qt::Unchecked && nodeLayer->layer() )
        rec.mLayerRecords << createThemeLayerRecord( nodeLayer, model );
    }
  }
}

QgsMapThemeCollection::MapThemeRecord QgsMapThemeCollection::createThemeFromCurrentState( QgsLayerTreeGroup *root, QgsLayerTreeModel *model )
{
  QgsMapThemeCollection::MapThemeRecord rec;
  rec.setHasExpandedStateInfo( true );  // all newly created theme records have expanded state info
  rec.setHasCheckedStateInfo( true );  // all newly created theme records have checked state info
  createThemeFromCurrentState( root, model, rec );
  return rec;
}

bool QgsMapThemeCollection::findRecordForLayer( QgsMapLayer *layer, const QgsMapThemeCollection::MapThemeRecord &rec, QgsMapThemeCollection::MapThemeLayerRecord &layerRec )
{
  for ( const QgsMapThemeCollection::MapThemeLayerRecord &lr : std::as_const( rec.mLayerRecords ) )
  {
    if ( lr.layer() == layer )
    {
      layerRec = lr;
      return true;
    }
  }
  return false;
}

void QgsMapThemeCollection::applyThemeToLayer( QgsLayerTreeLayer *nodeLayer, QgsLayerTreeModel *model, const QgsMapThemeCollection::MapThemeRecord &rec )
{
  MapThemeLayerRecord layerRec;
  const bool recordExists = findRecordForLayer( nodeLayer->layer(), rec, layerRec );

  // Make sure the whole tree is visible
  if ( recordExists )
  {
    if ( rec.hasCheckedStateInfo() )
      nodeLayer->setItemVisibilityChecked( true );
    else
      nodeLayer->setItemVisibilityCheckedParentRecursive( true );
  }
  else
    nodeLayer->setItemVisibilityChecked( false );

  if ( !recordExists )
    return;

  if ( layerRec.usingCurrentStyle )
  {
    // apply desired style first
    nodeLayer->layer()->styleManager()->setCurrentStyle( layerRec.currentStyle );
  }

  if ( layerRec.usingLegendItems )
  {
    // some nodes are not checked
    const QList<QgsLayerTreeModelLegendNode *> constLayerLegendNodes = model->layerLegendNodes( nodeLayer, true );
    for ( QgsLayerTreeModelLegendNode *legendNode : constLayerLegendNodes )
    {
      QString ruleKey = legendNode->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString();
      Qt::CheckState shouldHaveState = layerRec.checkedLegendItems.contains( ruleKey ) ? Qt::Checked : Qt::Unchecked;
      if ( ( legendNode->flags() & Qt::ItemIsUserCheckable ) &&
           legendNode->data( Qt::CheckStateRole ).toInt() != shouldHaveState )
        legendNode->setData( shouldHaveState, Qt::CheckStateRole );
    }
  }
  else
  {
    // all nodes should be checked
    const QList<QgsLayerTreeModelLegendNode *> constLayerLegendNodes = model->layerLegendNodes( nodeLayer, true );
    for ( QgsLayerTreeModelLegendNode *legendNode : constLayerLegendNodes )
    {
      if ( ( legendNode->flags() & Qt::ItemIsUserCheckable ) &&
           legendNode->data( Qt::CheckStateRole ).toInt() != Qt::Checked )
        legendNode->setData( Qt::Checked, Qt::CheckStateRole );
    }
  }

  // apply expanded/collapsed state to the layer and its legend nodes
  if ( rec.hasExpandedStateInfo() )
  {
    nodeLayer->setExpanded( layerRec.expandedLayerNode );
    nodeLayer->setCustomProperty( QStringLiteral( "expandedLegendNodes" ), QStringList( layerRec.expandedLegendItems.constBegin(), layerRec.expandedLegendItems.constEnd() ) );
  }
}


void QgsMapThemeCollection::applyThemeToGroup( QgsLayerTreeGroup *parent, QgsLayerTreeModel *model, const QgsMapThemeCollection::MapThemeRecord &rec )
{
  const QList<QgsLayerTreeNode *> constChildren = parent->children();
  for ( QgsLayerTreeNode *node : constChildren )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      applyThemeToGroup( QgsLayerTree::toGroup( node ), model, rec );
      if ( rec.hasExpandedStateInfo() )
        node->setExpanded( rec.expandedGroupNodes().contains( _groupId( node ) ) );
      if ( rec.hasCheckedStateInfo() )
        node->setItemVisibilityChecked( rec.checkedGroupNodes().contains( _groupId( node ) ) );
    }
    else if ( QgsLayerTree::isLayer( node ) )
      applyThemeToLayer( QgsLayerTree::toLayer( node ), model, rec );
  }
}


void QgsMapThemeCollection::applyTheme( const QString &name, QgsLayerTreeGroup *root, QgsLayerTreeModel *model )
{
  applyThemeToGroup( root, model, mapThemeState( name ) );

  // also make sure that the preset is up-to-date (not containing any non-existent legend items)
  update( name, createThemeFromCurrentState( root, model ) );
}

QgsProject *QgsMapThemeCollection::project()
{
  return mProject;
}

void QgsMapThemeCollection::setProject( QgsProject *project )
{
  if ( project == mProject )
    return;

  disconnect( mProject, static_cast<void ( QgsProject::* )( const QStringList & )>( &QgsProject::layersWillBeRemoved ), this, &QgsMapThemeCollection::registryLayersRemoved );
  mProject = project;
  connect( mProject, static_cast<void ( QgsProject::* )( const QStringList & )>( &QgsProject::layersWillBeRemoved ), this, &QgsMapThemeCollection::registryLayersRemoved );
  emit projectChanged();
}

QList<QgsMapLayer *> QgsMapThemeCollection::masterLayerOrder() const
{
  if ( !mProject )
    return QList< QgsMapLayer * >();

  return mProject->layerTreeRoot()->layerOrder();
}

QList<QgsMapLayer *> QgsMapThemeCollection::masterVisibleLayers() const
{
  const QList< QgsMapLayer *> allLayers = masterLayerOrder();
  const QList< QgsMapLayer * > visibleLayers = mProject->layerTreeRoot()->checkedLayers();

  if ( allLayers.isEmpty() )
  {
    // no project layer order set
    return visibleLayers;
  }
  else
  {
    QList< QgsMapLayer * > orderedVisibleLayers;
    for ( QgsMapLayer *layer : allLayers )
    {
      if ( visibleLayers.contains( layer ) )
        orderedVisibleLayers << layer;
    }
    return orderedVisibleLayers;
  }
}


bool QgsMapThemeCollection::hasMapTheme( const QString &name ) const
{
  return mMapThemes.contains( name );
}

void QgsMapThemeCollection::insert( const QString &name, const QgsMapThemeCollection::MapThemeRecord &state )
{
  mMapThemes.insert( name, state );

  reconnectToLayersStyleManager();
  emit mapThemeChanged( name );
  emit mapThemesChanged();
}

void QgsMapThemeCollection::update( const QString &name, const MapThemeRecord &state )
{
  if ( !mMapThemes.contains( name ) )
    return;

  mMapThemes[name] = state;

  reconnectToLayersStyleManager();
  emit mapThemeChanged( name );
  emit mapThemesChanged();
}

bool QgsMapThemeCollection::renameMapTheme( const QString &name,  const QString &newName )
{
  if ( !mMapThemes.contains( name ) || mMapThemes.contains( newName ) )
    return false;

  const MapThemeRecord state = mMapThemes[name];
  const MapThemeRecord newState = state;
  insert( newName, newState );
  emit mapThemeRenamed( name, newName );
  removeMapTheme( name );
  return true;
}

void QgsMapThemeCollection::removeMapTheme( const QString &name )
{
  if ( !mMapThemes.contains( name ) )
    return;

  mMapThemes.remove( name );

  reconnectToLayersStyleManager();
  emit mapThemesChanged();
}

void QgsMapThemeCollection::clear()
{
  mMapThemes.clear();

  reconnectToLayersStyleManager();
  emit mapThemesChanged();
}

QStringList QgsMapThemeCollection::mapThemes() const
{
  return mMapThemes.keys();
}

QStringList QgsMapThemeCollection::mapThemeVisibleLayerIds( const QString &name ) const
{
  QStringList layerIds;
  const QList<QgsMapLayer *> constMapThemeVisibleLayers = mapThemeVisibleLayers( name );
  for ( QgsMapLayer *layer : constMapThemeVisibleLayers )
  {
    layerIds << layer->id();
  }
  return layerIds;
}

QList<QgsMapLayer *> QgsMapThemeCollection::mapThemeVisibleLayers( const QString &name ) const
{
  QList<QgsMapLayer *> layers;
  const QList<MapThemeLayerRecord> recs = mMapThemes.value( name ).mLayerRecords;
  const QList<QgsMapLayer *> layerOrder = masterLayerOrder();
  if ( layerOrder.isEmpty() )
  {
    // no master layer order - so we have to just use the stored theme layer order as a fallback
    const QList<MapThemeLayerRecord> records { mMapThemes.value( name ).mLayerRecords };
    for ( const MapThemeLayerRecord &layerRec : records )
    {
      if ( layerRec.isVisible && layerRec.layer() )
        layers << layerRec.layer();
    }
  }
  else
  {
    for ( QgsMapLayer *layer : layerOrder )
    {
      for ( const MapThemeLayerRecord &layerRec : recs )
      {
        if ( layerRec.isVisible && layerRec.layer() == layer )
          layers << layerRec.layer();
      }
    }
  }

  return layers;
}


void QgsMapThemeCollection::applyMapThemeCheckedLegendNodesToLayer( const MapThemeLayerRecord &layerRec, QgsMapLayer *layer )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer || !vlayer->renderer() )
    return;

  QgsFeatureRenderer *renderer = vlayer->renderer();
  if ( !renderer->legendSymbolItemsCheckable() )
    return; // no need to do anything

  bool someNodesUnchecked = layerRec.usingLegendItems;

  const auto constLegendSymbolItems = vlayer->renderer()->legendSymbolItems();
  for ( const QgsLegendSymbolItem &item : constLegendSymbolItems )
  {
    bool checked = renderer->legendSymbolItemChecked( item.ruleKey() );
    bool shouldBeChecked = someNodesUnchecked ? layerRec.checkedLegendItems.contains( item.ruleKey() ) : true;
    if ( checked != shouldBeChecked )
      renderer->checkLegendSymbolItem( item.ruleKey(), shouldBeChecked );
  }
}


QMap<QString, QString> QgsMapThemeCollection::mapThemeStyleOverrides( const QString &presetName )
{
  QMap<QString, QString> styleOverrides;
  if ( !mMapThemes.contains( presetName ) )
    return styleOverrides;

  const QList<MapThemeLayerRecord> records {mMapThemes.value( presetName ).mLayerRecords};
  for ( const MapThemeLayerRecord &layerRec : records )
  {
    if ( !layerRec.layer() )
      continue;

    if ( layerRec.usingCurrentStyle )
    {
      QgsMapLayer *layer = layerRec.layer();
      QgsMapLayerStyleOverride styleOverride( layer );
      styleOverride.setOverrideStyle( layerRec.currentStyle );

      // set the checked legend nodes
      applyMapThemeCheckedLegendNodesToLayer( layerRec, layer );

      // save to overrides
      QgsMapLayerStyle layerStyle;
      layerStyle.readFromLayer( layer );
      styleOverrides[layer->id()] = layerStyle.xmlData();
    }
  }
  return styleOverrides;
}

void QgsMapThemeCollection::reconnectToLayersStyleManager()
{
  // disconnect( 0, 0, this, SLOT( layerStyleRenamed( QString, QString ) ) );

  QSet<QgsMapLayer *> layers;
  for ( const MapThemeRecord &rec : std::as_const( mMapThemes ) )
  {
    for ( const MapThemeLayerRecord &layerRec : std::as_const( rec.mLayerRecords ) )
    {
      if ( auto *lLayer = layerRec.layer() )
        layers << lLayer;
    }
  }

  const QSet<QgsMapLayer *> constLayers = layers;
  for ( QgsMapLayer *ml : constLayers )
  {
    connect( ml->styleManager(), &QgsMapLayerStyleManager::styleRenamed, this, &QgsMapThemeCollection::layerStyleRenamed );
  }
}

void QgsMapThemeCollection::readXml( const QDomDocument &doc )
{
  clear();

  QDomElement visPresetsElem = doc.firstChildElement( QStringLiteral( "qgis" ) ).firstChildElement( QStringLiteral( "visibility-presets" ) );
  if ( visPresetsElem.isNull() )
    return;

  QDomElement visPresetElem = visPresetsElem.firstChildElement( QStringLiteral( "visibility-preset" ) );
  while ( !visPresetElem.isNull() )
  {
    QString presetName = visPresetElem.attribute( QStringLiteral( "name" ) );
    QgsMapThemeCollection::MapThemeRecord rec = QgsMapThemeCollection::MapThemeRecord::readXml( visPresetElem, mProject );
    mMapThemes.insert( presetName, rec );
    emit mapThemeChanged( presetName );
    visPresetElem = visPresetElem.nextSiblingElement( QStringLiteral( "visibility-preset" ) );
  }

  reconnectToLayersStyleManager();
  emit mapThemesChanged();
}

void QgsMapThemeCollection::writeXml( QDomDocument &doc ) const
{
  QDomElement visPresetsElem = doc.createElement( QStringLiteral( "visibility-presets" ) );

  QList< QString > keys = mMapThemes.keys();

  std::sort( keys.begin(), keys.end() );

  for ( const QString &grpName : std::as_const( keys ) )
  {
    const MapThemeRecord &rec = mMapThemes.value( grpName );
    QDomElement visPresetElem = doc.createElement( QStringLiteral( "visibility-preset" ) );

    visPresetElem.setAttribute( QStringLiteral( "name" ), grpName );
    rec.writeXml( visPresetElem, doc );

    visPresetsElem.appendChild( visPresetElem );
  }

  doc.firstChildElement( QStringLiteral( "qgis" ) ).appendChild( visPresetsElem );
}

void QgsMapThemeCollection::registryLayersRemoved( const QStringList &layerIDs )
{
  // while layers are stored as weak pointers, this triggers the mapThemeChanged signal for
  // affected themes
  QSet< QString > changedThemes;
  MapThemeRecordMap::iterator it = mMapThemes.begin();
  for ( ; it != mMapThemes.end(); ++it )
  {
    MapThemeRecord &rec = it.value();
    for ( int i = 0; i < rec.mLayerRecords.count(); ++i )
    {
      MapThemeLayerRecord &layerRec = rec.mLayerRecords[i];
      if ( layerRec.layer() && layerIDs.contains( layerRec.layer()->id() ) )
      {
        rec.mLayerRecords.removeAt( i-- );
        changedThemes << it.key();
      }
    }
  }

  for ( const QString &theme : std::as_const( changedThemes ) )
  {
    emit mapThemeChanged( theme );
  }
  emit mapThemesChanged();
}

void QgsMapThemeCollection::layerStyleRenamed( const QString &oldName, const QString &newName )
{
  QgsMapLayerStyleManager *styleMgr = qobject_cast<QgsMapLayerStyleManager *>( sender() );
  if ( !styleMgr )
    return;

  QSet< QString > changedThemes;

  MapThemeRecordMap::iterator it = mMapThemes.begin();
  for ( ; it != mMapThemes.end(); ++it )
  {
    MapThemeRecord &rec = it.value();
    for ( int i = 0; i < rec.mLayerRecords.count(); ++i )
    {
      MapThemeLayerRecord &layerRec = rec.mLayerRecords[i];
      if ( layerRec.layer() == styleMgr->layer() )
      {
        if ( layerRec.currentStyle == oldName )
        {
          layerRec.currentStyle = newName;
          changedThemes << it.key();
        }
      }
    }
  }

  for ( const QString &theme : std::as_const( changedThemes ) )
  {
    emit mapThemeChanged( theme );
  }
  emit mapThemesChanged();
}

void QgsMapThemeCollection::MapThemeRecord::removeLayerRecord( QgsMapLayer *layer )
{
  for ( int i = 0; i < mLayerRecords.length(); ++i )
  {
    if ( mLayerRecords.at( i ).layer() == layer )
      mLayerRecords.removeAt( i );
  }
}

void QgsMapThemeCollection::MapThemeRecord::addLayerRecord( const QgsMapThemeCollection::MapThemeLayerRecord &record )
{
  mLayerRecords.append( record );
}

QHash<QgsMapLayer *, QgsMapThemeCollection::MapThemeLayerRecord> QgsMapThemeCollection::MapThemeRecord::validLayerRecords() const
{
  QHash<QgsMapLayer *, MapThemeLayerRecord> validSet;
  for ( const MapThemeLayerRecord &layerRec : mLayerRecords )
  {
    if ( auto *lLayer = layerRec.layer() )
      validSet.insert( lLayer, layerRec );
  }
  return validSet;
}

QgsMapThemeCollection::MapThemeRecord QgsMapThemeCollection::MapThemeRecord::readXml( const QDomElement &element, const QgsProject *project )
{
  QHash<QString, MapThemeLayerRecord> layerRecords; // key = layer ID

  bool expandedStateInfo = false;
  if ( element.hasAttribute( QStringLiteral( "has-expanded-info" ) ) )
    expandedStateInfo = element.attribute( QStringLiteral( "has-expanded-info" ) ).toInt();

  bool checkedStateInfo = false;
  if ( element.hasAttribute( QStringLiteral( "has-checked-group-info" ) ) )
    checkedStateInfo = element.attribute( QStringLiteral( "has-checked-group-info" ) ).toInt();

  QDomElement visPresetLayerElem = element.firstChildElement( QStringLiteral( "layer" ) );
  while ( !visPresetLayerElem.isNull() )
  {
    QString layerID = visPresetLayerElem.attribute( QStringLiteral( "id" ) );
    if ( QgsMapLayer *layer = project->mapLayer( layerID ) )
    {
      layerRecords[layerID] = MapThemeLayerRecord( layer );
      layerRecords[layerID].isVisible = visPresetLayerElem.attribute( QStringLiteral( "visible" ), QStringLiteral( "1" ) ).toInt();

      if ( visPresetLayerElem.hasAttribute( QStringLiteral( "style" ) ) )
      {
        layerRecords[layerID].usingCurrentStyle = true;
        layerRecords[layerID].currentStyle = visPresetLayerElem.attribute( QStringLiteral( "style" ) );
      }

      if ( visPresetLayerElem.hasAttribute( QStringLiteral( "expanded" ) ) )
        layerRecords[layerID].expandedLayerNode = visPresetLayerElem.attribute( QStringLiteral( "expanded" ) ).toInt();
    }
    visPresetLayerElem = visPresetLayerElem.nextSiblingElement( QStringLiteral( "layer" ) );
  }

  QDomElement checkedLegendNodesElem = element.firstChildElement( QStringLiteral( "checked-legend-nodes" ) );
  while ( !checkedLegendNodesElem.isNull() )
  {
    QSet<QString> checkedLegendNodes;

    QDomElement checkedLegendNodeElem = checkedLegendNodesElem.firstChildElement( QStringLiteral( "checked-legend-node" ) );
    while ( !checkedLegendNodeElem.isNull() )
    {
      checkedLegendNodes << checkedLegendNodeElem.attribute( QStringLiteral( "id" ) );
      checkedLegendNodeElem = checkedLegendNodeElem.nextSiblingElement( QStringLiteral( "checked-legend-node" ) );
    }

    QString layerID = checkedLegendNodesElem.attribute( QStringLiteral( "id" ) );
    if ( project->mapLayer( layerID ) ) // only use valid IDs
    {
      layerRecords[layerID].usingLegendItems = true;
      layerRecords[layerID].checkedLegendItems = checkedLegendNodes;
    }
    checkedLegendNodesElem = checkedLegendNodesElem.nextSiblingElement( QStringLiteral( "checked-legend-nodes" ) );
  }

  QSet<QString> expandedGroupNodes;
  if ( expandedStateInfo )
  {
    // expanded state of legend nodes
    QDomElement expandedLegendNodesElem = element.firstChildElement( QStringLiteral( "expanded-legend-nodes" ) );
    while ( !expandedLegendNodesElem.isNull() )
    {
      QSet<QString> expandedLegendNodes;

      QDomElement expandedLegendNodeElem = expandedLegendNodesElem.firstChildElement( QStringLiteral( "expanded-legend-node" ) );
      while ( !expandedLegendNodeElem.isNull() )
      {
        expandedLegendNodes << expandedLegendNodeElem.attribute( QStringLiteral( "id" ) );
        expandedLegendNodeElem = expandedLegendNodeElem.nextSiblingElement( QStringLiteral( "expanded-legend-node" ) );
      }

      QString layerID = expandedLegendNodesElem.attribute( QStringLiteral( "id" ) );
      if ( project->mapLayer( layerID ) ) // only use valid IDs
      {
        layerRecords[layerID].expandedLegendItems = expandedLegendNodes;
      }
      expandedLegendNodesElem = expandedLegendNodesElem.nextSiblingElement( QStringLiteral( "expanded-legend-nodes" ) );
    }

    // expanded state of group nodes
    QDomElement expandedGroupNodesElem = element.firstChildElement( QStringLiteral( "expanded-group-nodes" ) );
    if ( !expandedGroupNodesElem.isNull() )
    {
      QDomElement expandedGroupNodeElem = expandedGroupNodesElem.firstChildElement( QStringLiteral( "expanded-group-node" ) );
      while ( !expandedGroupNodeElem.isNull() )
      {
        expandedGroupNodes << expandedGroupNodeElem.attribute( QStringLiteral( "id" ) );
        expandedGroupNodeElem = expandedGroupNodeElem.nextSiblingElement( QStringLiteral( "expanded-group-node" ) );
      }
    }
  }

  QSet<QString> checkedGroupNodes;
  if ( checkedStateInfo )
  {
    // expanded state of legend nodes
    QDomElement checkedGroupNodesElem = element.firstChildElement( QStringLiteral( "checked-group-nodes" ) );
    if ( !checkedGroupNodesElem.isNull() )
    {
      QDomElement checkedGroupNodeElem = checkedGroupNodesElem.firstChildElement( QStringLiteral( "checked-group-node" ) );
      while ( !checkedGroupNodeElem.isNull() )
      {
        checkedGroupNodes << checkedGroupNodeElem.attribute( QStringLiteral( "id" ) );
        checkedGroupNodeElem = checkedGroupNodeElem.nextSiblingElement( QStringLiteral( "checked-group-node" ) );
      }
    }
  }

  MapThemeRecord rec;
  rec.setLayerRecords( layerRecords.values() );
  rec.setHasExpandedStateInfo( expandedStateInfo );
  rec.setExpandedGroupNodes( expandedGroupNodes );
  rec.setHasCheckedStateInfo( checkedStateInfo );
  rec.setCheckedGroupNodes( checkedGroupNodes );

  return rec;
}

void QgsMapThemeCollection::MapThemeRecord::writeXml( QDomElement element, QDomDocument &document ) const
{
  if ( hasExpandedStateInfo() )
    element.setAttribute( QStringLiteral( "has-expanded-info" ), QStringLiteral( "1" ) );
  if ( hasCheckedStateInfo() )
    element.setAttribute( QStringLiteral( "has-checked-group-info" ), QStringLiteral( "1" ) );
  for ( const MapThemeLayerRecord &layerRec : std::as_const( mLayerRecords ) )
  {
    if ( !layerRec.layer() )
      continue;
    QString layerID = layerRec.layer()->id();
    QDomElement layerElem = document.createElement( QStringLiteral( "layer" ) );
    layerElem.setAttribute( QStringLiteral( "id" ), layerID );
    layerElem.setAttribute( QStringLiteral( "visible" ), layerRec.isVisible ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
    if ( layerRec.usingCurrentStyle )
      layerElem.setAttribute( QStringLiteral( "style" ), layerRec.currentStyle );
    element.appendChild( layerElem );

    if ( layerRec.usingLegendItems )
    {
      QDomElement checkedLegendNodesElem = document.createElement( QStringLiteral( "checked-legend-nodes" ) );
      checkedLegendNodesElem.setAttribute( QStringLiteral( "id" ), layerID );
      for ( const QString &checkedLegendNode : std::as_const( layerRec.checkedLegendItems ) )
      {
        QDomElement checkedLegendNodeElem = document.createElement( QStringLiteral( "checked-legend-node" ) );
        checkedLegendNodeElem.setAttribute( QStringLiteral( "id" ), checkedLegendNode );
        checkedLegendNodesElem.appendChild( checkedLegendNodeElem );
      }
      element.appendChild( checkedLegendNodesElem );
    }

    if ( hasExpandedStateInfo() )
    {
      layerElem.setAttribute( QStringLiteral( "expanded" ), layerRec.expandedLayerNode ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

      QDomElement expandedLegendNodesElem = document.createElement( QStringLiteral( "expanded-legend-nodes" ) );
      expandedLegendNodesElem.setAttribute( QStringLiteral( "id" ), layerID );
      for ( const QString &expandedLegendNode : std::as_const( layerRec.expandedLegendItems ) )
      {
        QDomElement expandedLegendNodeElem = document.createElement( QStringLiteral( "expanded-legend-node" ) );
        expandedLegendNodeElem.setAttribute( QStringLiteral( "id" ), expandedLegendNode );
        expandedLegendNodesElem.appendChild( expandedLegendNodeElem );
      }
      element.appendChild( expandedLegendNodesElem );
    }
  }

  if ( hasCheckedStateInfo() )
  {
    QDomElement checkedGroupElems = document.createElement( QStringLiteral( "checked-group-nodes" ) );
    const QSet<QString> _checkedGroupNodes = checkedGroupNodes();
    for ( const QString &groupId : _checkedGroupNodes )
    {
      QDomElement checkedGroupElem = document.createElement( QStringLiteral( "checked-group-node" ) );
      checkedGroupElem.setAttribute( QStringLiteral( "id" ), groupId );
      checkedGroupElems.appendChild( checkedGroupElem );
    }
    element.appendChild( checkedGroupElems );
  }

  if ( hasExpandedStateInfo() )
  {
    QDomElement expandedGroupElems = document.createElement( QStringLiteral( "expanded-group-nodes" ) );
    const QSet<QString> _expandedGroupNodes = expandedGroupNodes();
    for ( const QString &groupId : _expandedGroupNodes )
    {
      QDomElement expandedGroupElem = document.createElement( QStringLiteral( "expanded-group-node" ) );
      expandedGroupElem.setAttribute( QStringLiteral( "id" ), groupId );
      expandedGroupElems.appendChild( expandedGroupElem );
    }
    element.appendChild( expandedGroupElems );
  }
}

void QgsMapThemeCollection::MapThemeLayerRecord::setLayer( QgsMapLayer *layer )
{
  mLayer = layer;
}
