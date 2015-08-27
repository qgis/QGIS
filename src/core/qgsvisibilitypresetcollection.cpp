/***************************************************************************
  qgsvisibilitypresetcollection.cpp
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

#include "qgsvisibilitypresetcollection.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsproject.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayer.h"

#include <QInputDialog>

QgsVisibilityPresetCollection::QgsVisibilityPresetCollection()
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersRemoved( QStringList ) ),
           this, SLOT( registryLayersRemoved( QStringList ) ) );
}

void QgsVisibilityPresetCollection::addVisibleLayersToPreset( QgsLayerTreeGroup* parent, QgsVisibilityPresetCollection::PresetRecord& rec )
{
  foreach ( QgsLayerTreeNode* node, parent->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      addVisibleLayersToPreset( QgsLayerTree::toGroup( node ), rec );
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->isVisible() )
        rec.mVisibleLayerIDs << nodeLayer->layerId();
    }
  }
}

bool QgsVisibilityPresetCollection::hasPreset( const QString& name ) const
{
  return mPresets.contains( name );
}

void QgsVisibilityPresetCollection::insert( const QString& name, const QgsVisibilityPresetCollection::PresetRecord& state )
{
  mPresets.insert( name, state );

  reconnectToLayersStyleManager();
}

void QgsVisibilityPresetCollection::update( const QString& name, const PresetRecord& state )
{
  if ( !mPresets.contains( name ) )
    return;

  mPresets[name] = state;

  reconnectToLayersStyleManager();
}

void QgsVisibilityPresetCollection::removePreset( const QString& name )
{
  if ( !mPresets.contains( name ) )
    return;

  mPresets.remove( name );

  reconnectToLayersStyleManager();
}

void QgsVisibilityPresetCollection::clear()
{
  mPresets.clear();

  reconnectToLayersStyleManager();
}

QStringList QgsVisibilityPresetCollection::presets() const
{
  return mPresets.keys();
}

QStringList QgsVisibilityPresetCollection::presetVisibleLayers( const QString& name ) const
{
  QSet<QString> visibleIds = mPresets.value( name ).mVisibleLayerIDs;
  return visibleIds.toList();
}


void QgsVisibilityPresetCollection::applyPresetCheckedLegendNodesToLayer( const QString& name, const QString& layerID )
{
  if ( !mPresets.contains( name ) )
    return;

  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerID );
  if ( !layer )
    return;

  const PresetRecord& rec = mPresets[name];

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( layer );
  if ( !vlayer || !vlayer->rendererV2() )
    return;

  if ( !vlayer->rendererV2()->legendSymbolItemsCheckable() )
    return; // no need to do anything

  bool someNodesUnchecked = rec.mPerLayerCheckedLegendSymbols.contains( layerID );

  foreach ( const QgsLegendSymbolItemV2& item, vlayer->rendererV2()->legendSymbolItemsV2() )
  {
    bool checked = vlayer->rendererV2()->legendSymbolItemChecked( item.ruleKey() );
    bool shouldBeChecked = someNodesUnchecked ? rec.mPerLayerCheckedLegendSymbols[layerID].contains( item.ruleKey() ) : true;
    if ( checked != shouldBeChecked )
      vlayer->rendererV2()->checkLegendSymbolItem( item.ruleKey(), shouldBeChecked );
  }
}


QMap<QString, QString> QgsVisibilityPresetCollection::presetStyleOverrides( const QString& presetName )
{
  QMap<QString, QString> styleOverrides;
  if ( !mPresets.contains( presetName ) )
    return styleOverrides;

  QStringList lst = presetVisibleLayers( presetName );
  const QgsVisibilityPresetCollection::PresetRecord& rec = mPresets[presetName];
  foreach ( const QString& layerID, lst )
  {
    QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerID );
    if ( !layer )
      continue;

    // use either the stored style name or the current one if none has been stored
    QString overrideStyleName = rec.mPerLayerCurrentStyle.value( layerID, layer->styleManager()->currentStyle() );

    // store original style and temporarily apply a style
    layer->styleManager()->setOverrideStyle( overrideStyleName );

    // set the checked legend nodes
    applyPresetCheckedLegendNodesToLayer( presetName, layerID );

    // save to overrides
    QgsMapLayerStyle layerStyle;
    layerStyle.readFromLayer( layer );
    styleOverrides[layerID] = layerStyle.xmlData();

    layer->styleManager()->restoreOverrideStyle();
  }
  return styleOverrides;
}

void QgsVisibilityPresetCollection::reconnectToLayersStyleManager()
{
  // disconnect( 0, 0, this, SLOT( layerStyleRenamed( QString, QString ) ) );

  QSet<QString> layerIDs;
  foreach ( const QString& grpName, mPresets.keys() )
  {
    const PresetRecord& rec = mPresets[grpName];
    foreach ( const QString& layerID, rec.mPerLayerCurrentStyle.keys() )
      layerIDs << layerID;
  }

  foreach ( const QString& layerID, layerIDs )
  {
    if ( QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerID ) )
      connect( ml->styleManager(), SIGNAL( styleRenamed( QString, QString ) ), this, SLOT( layerStyleRenamed( QString, QString ) ) );
  }
}

void QgsVisibilityPresetCollection::readXML( const QDomDocument& doc )
{
  clear();

  QDomElement visPresetsElem = doc.firstChildElement( "qgis" ).firstChildElement( "visibility-presets" );
  if ( visPresetsElem.isNull() )
    return;

  QDomElement visPresetElem = visPresetsElem.firstChildElement( "visibility-preset" );
  while ( !visPresetElem.isNull() )
  {
    QString presetName = visPresetElem.attribute( "name" );
    PresetRecord rec;
    QDomElement visPresetLayerElem = visPresetElem.firstChildElement( "layer" );
    while ( !visPresetLayerElem.isNull() )
    {
      QString layerID = visPresetLayerElem.attribute( "id" );
      if ( QgsMapLayerRegistry::instance()->mapLayer( layerID ) )
      {
        rec.mVisibleLayerIDs << layerID; // only use valid layer IDs
        if ( visPresetLayerElem.hasAttribute( "style" ) )
          rec.mPerLayerCurrentStyle[layerID] = visPresetLayerElem.attribute( "style" );
      }
      visPresetLayerElem = visPresetLayerElem.nextSiblingElement( "layer" );
    }

    QDomElement checkedLegendNodesElem = visPresetElem.firstChildElement( "checked-legend-nodes" );
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

    mPresets.insert( presetName, rec );

    visPresetElem = visPresetElem.nextSiblingElement( "visibility-preset" );
  }

  reconnectToLayersStyleManager();
}

void QgsVisibilityPresetCollection::writeXML( QDomDocument& doc )
{
  QDomElement visPresetsElem = doc.createElement( "visibility-presets" );
  foreach ( const QString& grpName, mPresets.keys() )
  {
    const PresetRecord& rec = mPresets[grpName];
    QDomElement visPresetElem = doc.createElement( "visibility-preset" );
    visPresetElem.setAttribute( "name", grpName );
    foreach ( QString layerID, rec.mVisibleLayerIDs )
    {
      QDomElement layerElem = doc.createElement( "layer" );
      layerElem.setAttribute( "id", layerID );
      if ( rec.mPerLayerCurrentStyle.contains( layerID ) )
        layerElem.setAttribute( "style", rec.mPerLayerCurrentStyle[layerID] );
      visPresetElem.appendChild( layerElem );
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
      visPresetElem.appendChild( checkedLegendNodesElem );
    }

    visPresetsElem.appendChild( visPresetElem );
  }

  doc.firstChildElement( "qgis" ).appendChild( visPresetsElem );
}

void QgsVisibilityPresetCollection::registryLayersRemoved( QStringList layerIDs )
{
  foreach ( QString layerID, layerIDs )
  {
    foreach ( QString presetName, mPresets.keys() )
    {
      PresetRecord& rec = mPresets[presetName];
      rec.mVisibleLayerIDs.remove( layerID );
      rec.mPerLayerCheckedLegendSymbols.remove( layerID );
      rec.mPerLayerCurrentStyle.remove( layerID );
    }
  }
}

void QgsVisibilityPresetCollection::layerStyleRenamed( const QString& oldName, const QString& newName )
{
  QgsMapLayerStyleManager* styleMgr = qobject_cast<QgsMapLayerStyleManager*>( sender() );
  if ( !styleMgr )
    return;

  QString layerID = styleMgr->layer()->id();

  foreach ( QString presetName, mPresets.keys() )
  {
    PresetRecord& rec = mPresets[presetName];

    if ( rec.mPerLayerCurrentStyle.contains( layerID ) )
    {
      QString styleName = rec.mPerLayerCurrentStyle[layerID];
      if ( styleName == oldName )
        rec.mPerLayerCurrentStyle[layerID] = newName;
    }
  }
}
