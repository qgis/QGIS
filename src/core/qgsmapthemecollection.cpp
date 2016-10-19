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
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"

#include <QInputDialog>

QgsMapThemeCollection::QgsMapThemeCollection()
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersRemoved( QStringList ) ),
           this, SLOT( registryLayersRemoved( QStringList ) ) );
}

void QgsMapThemeCollection::addVisibleLayersToMapTheme( QgsLayerTreeGroup* parent, QgsMapThemeCollection::MapThemeRecord& rec )
{
  Q_FOREACH ( QgsLayerTreeNode* node, parent->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      addVisibleLayersToMapTheme( QgsLayerTree::toGroup( node ), rec );
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->isVisible() )
        rec.mVisibleLayerIds << nodeLayer->layerId();
    }
  }
}

bool QgsMapThemeCollection::hasMapTheme( const QString& name ) const
{
  return mMapThemes.contains( name );
}

void QgsMapThemeCollection::insert( const QString& name, const QgsMapThemeCollection::MapThemeRecord& state )
{
  mMapThemes.insert( name, state );

  reconnectToLayersStyleManager();
  emit mapThemesChanged();
}

void QgsMapThemeCollection::update( const QString& name, const MapThemeRecord& state )
{
  if ( !mMapThemes.contains( name ) )
    return;

  mMapThemes[name] = state;

  reconnectToLayersStyleManager();
  emit mapThemesChanged();
}

void QgsMapThemeCollection::removeMapTheme( const QString& name )
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

QStringList QgsMapThemeCollection::mapThemeVisibleLayers( const QString& name ) const
{
  return mMapThemes.value( name ).mVisibleLayerIds;
}


void QgsMapThemeCollection::applyMapThemeCheckedLegendNodesToLayer( const QString& name, const QString& layerID )
{
  if ( !mMapThemes.contains( name ) )
    return;

  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerID );
  if ( !layer )
    return;

  const MapThemeRecord& rec = mMapThemes[name];

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( layer );
  if ( !vlayer || !vlayer->renderer() )
    return;

  if ( !vlayer->renderer()->legendSymbolItemsCheckable() )
    return; // no need to do anything

  bool someNodesUnchecked = rec.mPerLayerCheckedLegendSymbols.contains( layerID );

  Q_FOREACH ( const QgsLegendSymbolItem& item, vlayer->renderer()->legendSymbolItemsV2() )
  {
    bool checked = vlayer->renderer()->legendSymbolItemChecked( item.ruleKey() );
    bool shouldBeChecked = someNodesUnchecked ? rec.mPerLayerCheckedLegendSymbols[layerID].contains( item.ruleKey() ) : true;
    if ( checked != shouldBeChecked )
      vlayer->renderer()->checkLegendSymbolItem( item.ruleKey(), shouldBeChecked );
  }
}


QMap<QString, QString> QgsMapThemeCollection::mapThemeStyleOverride( const QString& presetName )
{
  QMap<QString, QString> styleOverrides;
  if ( !mMapThemes.contains( presetName ) )
    return styleOverrides;

  QStringList lst = mapThemeVisibleLayers( presetName );
  const QgsMapThemeCollection::MapThemeRecord& rec = mMapThemes[presetName];
  Q_FOREACH ( const QString& layerID, lst )
  {
    QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerID );
    if ( !layer )
      continue;

    // use either the stored style name or the current one if none has been stored
    QString overrideStyleName = rec.mPerLayerCurrentStyle.value( layerID, layer->styleManager()->currentStyle() );

    // store original style and temporarily apply a style
    layer->styleManager()->setOverrideStyle( overrideStyleName );

    // set the checked legend nodes
    applyMapThemeCheckedLegendNodesToLayer( presetName, layerID );

    // save to overrides
    QgsMapLayerStyle layerStyle;
    layerStyle.readFromLayer( layer );
    styleOverrides[layerID] = layerStyle.xmlData();

    layer->styleManager()->restoreOverrideStyle();
  }
  return styleOverrides;
}

void QgsMapThemeCollection::reconnectToLayersStyleManager()
{
  // disconnect( 0, 0, this, SLOT( layerStyleRenamed( QString, QString ) ) );

  QSet<QString> layerIDs;
  MapThemeRecordMap::const_iterator it = mMapThemes.constBegin();
  for ( ; it != mMapThemes.constEnd(); ++it )
  {
    const MapThemeRecord& rec = it.value();
    QMap<QString, QString>::const_iterator layerIt = rec.mPerLayerCurrentStyle.constBegin();
    for ( ; layerIt != rec.mPerLayerCurrentStyle.constEnd(); ++layerIt )
      layerIDs << layerIt.key();
  }

  Q_FOREACH ( const QString& layerID, layerIDs )
  {
    if ( QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerID ) )
      connect( ml->styleManager(), SIGNAL( styleRenamed( QString, QString ) ), this, SLOT( layerStyleRenamed( QString, QString ) ) );
  }
}

void QgsMapThemeCollection::readXml( const QDomDocument& doc )
{
  clear();

  QDomElement visPresetsElem = doc.firstChildElement( "qgis" ).firstChildElement( "visibility-presets" );
  if ( visPresetsElem.isNull() )
    return;

  QDomElement visPresetElem = visPresetsElem.firstChildElement( "visibility-preset" );
  while ( !visPresetElem.isNull() )
  {
    QString presetName = visPresetElem.attribute( "name" );
    MapThemeRecord rec;
    QDomElement visPresetLayerElem = visPresetElem.firstChildElement( "layer" );
    while ( !visPresetLayerElem.isNull() )
    {
      QString layerID = visPresetLayerElem.attribute( "id" );
      if ( QgsMapLayerRegistry::instance()->mapLayer( layerID ) )
      {
        rec.mVisibleLayerIds << layerID; // only use valid layer IDs
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

    mMapThemes.insert( presetName, rec );

    visPresetElem = visPresetElem.nextSiblingElement( "visibility-preset" );
  }

  reconnectToLayersStyleManager();
  emit mapThemesChanged();
}

void QgsMapThemeCollection::writeXml( QDomDocument& doc )
{
  QDomElement visPresetsElem = doc.createElement( "visibility-presets" );
  MapThemeRecordMap::const_iterator it = mMapThemes.constBegin();
  for ( ; it != mMapThemes.constEnd(); ++ it )
  {
    QString grpName = it.key();
    const MapThemeRecord& rec = it.value();
    QDomElement visPresetElem = doc.createElement( "visibility-preset" );
    visPresetElem.setAttribute( "name", grpName );
    Q_FOREACH ( const QString& layerID, rec.mVisibleLayerIds )
    {
      QDomElement layerElem = doc.createElement( "layer" );
      layerElem.setAttribute( "id", layerID );
      if ( rec.mPerLayerCurrentStyle.contains( layerID ) )
        layerElem.setAttribute( "style", rec.mPerLayerCurrentStyle[layerID] );
      visPresetElem.appendChild( layerElem );
    }

    QMap<QString, QSet<QString> >::const_iterator layerIt = rec.mPerLayerCheckedLegendSymbols.constBegin();
    for ( ; layerIt != rec.mPerLayerCheckedLegendSymbols.constEnd(); ++layerIt )
    {
      QString layerID = layerIt.key();
      QDomElement checkedLegendNodesElem = doc.createElement( "checked-legend-nodes" );
      checkedLegendNodesElem.setAttribute( "id", layerID );
      Q_FOREACH ( const QString& checkedLegendNode, layerIt.value() )
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

void QgsMapThemeCollection::registryLayersRemoved( const QStringList& layerIDs )
{
  Q_FOREACH ( const QString& layerID, layerIDs )
  {
    MapThemeRecordMap::iterator it = mMapThemes.begin();
    for ( ; it != mMapThemes.end(); ++it )
    {
      MapThemeRecord& rec = it.value();
      rec.mVisibleLayerIds.removeAll( layerID );
      rec.mPerLayerCheckedLegendSymbols.remove( layerID );
      rec.mPerLayerCurrentStyle.remove( layerID );
    }
  }
  emit mapThemesChanged();
}

void QgsMapThemeCollection::layerStyleRenamed( const QString& oldName, const QString& newName )
{
  QgsMapLayerStyleManager* styleMgr = qobject_cast<QgsMapLayerStyleManager*>( sender() );
  if ( !styleMgr )
    return;

  QString layerID = styleMgr->layer()->id();

  MapThemeRecordMap::iterator it = mMapThemes.begin();
  for ( ; it != mMapThemes.end(); ++it )
  {
    MapThemeRecord& rec = it.value();

    if ( rec.mPerLayerCurrentStyle.contains( layerID ) )
    {
      QString styleName = rec.mPerLayerCurrentStyle[layerID];
      if ( styleName == oldName )
        rec.mPerLayerCurrentStyle[layerID] = newName;
    }
  }
  emit mapThemesChanged();
}

QStringList QgsMapThemeCollection::MapThemeRecord::visibleLayerIds() const
{
  return mVisibleLayerIds;
}

void QgsMapThemeCollection::MapThemeRecord::setVisibleLayerIds( const QStringList& visibleLayerIds )
{
  mVisibleLayerIds = visibleLayerIds;
}

QMap<QString, QSet<QString> > QgsMapThemeCollection::MapThemeRecord::perLayerCheckedLegendSymbols() const
{
  return mPerLayerCheckedLegendSymbols;
}

void QgsMapThemeCollection::MapThemeRecord::setPerLayerCheckedLegendSymbols( const QMap<QString, QSet<QString> >& perLayerCheckedLegendSymbols )
{
  mPerLayerCheckedLegendSymbols = perLayerCheckedLegendSymbols;
}

QMap<QString, QString> QgsMapThemeCollection::MapThemeRecord::perLayerCurrentStyle() const
{
  return mPerLayerCurrentStyle;
}

void QgsMapThemeCollection::MapThemeRecord::setPerLayerCurrentStyle( const QMap<QString, QString>& perLayerCurrentStyle )
{
  mPerLayerCurrentStyle = perLayerCurrentStyle;
}
