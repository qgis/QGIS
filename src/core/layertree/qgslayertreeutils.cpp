/***************************************************************************
  qgslayertreeutils.cpp
  --------------------------------------
  Date                 : May 2014
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

#include "qgslayertreeutils.h"

#include "qgslayertree.h"

#include "qgsvectorlayer.h"

#include "qgsproject.h"

#include <QDomElement>


static void _readOldLegendGroup( const QDomElement& groupElem, QgsLayerTreeGroup* parent );
static void _readOldLegendLayer( const QDomElement& layerElem, QgsLayerTreeGroup* parent );

bool QgsLayerTreeUtils::readOldLegend( QgsLayerTreeGroup* root, const QDomElement& legendElem )
{
  if ( legendElem.isNull() )
    return false;

  QDomNodeList legendChildren = legendElem.childNodes();

  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == "legendlayer" )
    {
      _readOldLegendLayer( currentChildElem, root );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      _readOldLegendGroup( currentChildElem, root );
    }
  }

  return true;
}



static bool _readOldLegendLayerOrderGroup( const QDomElement& groupElem, QMap<int, QString>& layerIndexes )
{
  QDomNodeList legendChildren = groupElem.childNodes();

  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == "legendlayer" )
    {
      QDomElement layerFileElem = currentChildElem.firstChildElement( "filegroup" ).firstChildElement( "legendlayerfile" );

      int layerIndex = currentChildElem.attribute( "drawingOrder" ).toInt();
      if ( layerIndex == -1 )
        return false; // order undefined
      layerIndexes.insert( layerIndex, layerFileElem.attribute( "layerid" ) );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      if ( !_readOldLegendLayerOrderGroup( currentChildElem, layerIndexes ) )
        return false;
    }
  }

  return true;
}


bool QgsLayerTreeUtils::readOldLegendLayerOrder( const QDomElement& legendElem, bool& hasCustomOrder, QStringList& order )
{
  if ( legendElem.isNull() )
    return false;

  hasCustomOrder = legendElem.attribute( "updateDrawingOrder" ) == "false";
  order.clear();

  QMap<int, QString> layerIndexes;

  // try to read the order. may be undefined (order = -1) for some or all items
  bool res = _readOldLegendLayerOrderGroup( legendElem, layerIndexes );

  if ( !res && hasCustomOrder )
    return false; // invalid state

  foreach ( QString layerId, layerIndexes )
  {
    QgsDebugMsg( layerId );
    order.append( layerId );
  }

  return true;
}


static QDomElement _writeOldLegendLayer( QDomDocument& doc, QgsLayerTreeLayer* nodeLayer, bool hasCustomOrder, const QStringList& order )
{
  int drawingOrder = -1;
  if ( hasCustomOrder )
    drawingOrder = order.indexOf( nodeLayer->layerId() );

  QDomElement layerElem = doc.createElement( "legendlayer" );
  layerElem.setAttribute( "drawingOrder", drawingOrder );
  layerElem.setAttribute( "open", nodeLayer->isExpanded() ? "true" : "false" );
  layerElem.setAttribute( "checked", QgsLayerTreeUtils::checkStateToXml( nodeLayer->isVisible() ) );
  layerElem.setAttribute( "name", nodeLayer->layerName() );
  layerElem.setAttribute( "showFeatureCount", nodeLayer->customProperty( "showFeatureCount" ).toInt() );

  QDomElement fileGroupElem = doc.createElement( "filegroup" );
  fileGroupElem.setAttribute( "open", nodeLayer->isExpanded() ? "true" : "false" );
  fileGroupElem.setAttribute( "hidden", "false" );

  QDomElement layerFileElem = doc.createElement( "legendlayerfile" );
  layerFileElem.setAttribute( "isInOverview", nodeLayer->customProperty( "overview" ).toInt() );
  layerFileElem.setAttribute( "layerid", nodeLayer->layerId() );
  layerFileElem.setAttribute( "visible", nodeLayer->isVisible() == Qt::Checked ? 1 : 0 );

  layerElem.appendChild( fileGroupElem );
  fileGroupElem.appendChild( layerFileElem );
  return layerElem;
}

// need forward declaration as write[..]Group and write[..]GroupChildren call each other
static void _writeOldLegendGroupChildren( QDomDocument& doc, QDomElement& groupElem, QgsLayerTreeGroup* nodeGroup, bool hasCustomOrder, const QStringList& order );

static QDomElement _writeOldLegendGroup( QDomDocument& doc, QgsLayerTreeGroup* nodeGroup, bool hasCustomOrder, const QStringList& order )
{
  QDomElement groupElem = doc.createElement( "legendgroup" );
  groupElem.setAttribute( "open", nodeGroup->isExpanded() ? "true" : "false" );
  groupElem.setAttribute( "name", nodeGroup->name() );
  groupElem.setAttribute( "checked", QgsLayerTreeUtils::checkStateToXml( nodeGroup->isVisible() ) );

  if ( nodeGroup->customProperty( "embedded" ).toInt() )
  {
    groupElem.setAttribute( "embedded", 1 );
    groupElem.setAttribute( "project", nodeGroup->customProperty( "embedded_project" ).toString() );
  }

  _writeOldLegendGroupChildren( doc, groupElem, nodeGroup, hasCustomOrder, order );
  return groupElem;
}


static void _writeOldLegendGroupChildren( QDomDocument& doc, QDomElement& groupElem, QgsLayerTreeGroup* nodeGroup, bool hasCustomOrder, const QStringList& order )
{
  foreach ( QgsLayerTreeNode* node, nodeGroup->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      groupElem.appendChild( _writeOldLegendGroup( doc, QgsLayerTree::toGroup( node ), hasCustomOrder, order ) );
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      groupElem.appendChild( _writeOldLegendLayer( doc, QgsLayerTree::toLayer( node ), hasCustomOrder, order ) );
    }
  }
}


QDomElement QgsLayerTreeUtils::writeOldLegend( QDomDocument& doc, QgsLayerTreeGroup* root, bool hasCustomOrder, const QStringList& order )
{
  QDomElement legendElem = doc.createElement( "legend" );
  legendElem.setAttribute( "updateDrawingOrder", hasCustomOrder ? "false" : "true" );

  _writeOldLegendGroupChildren( doc, legendElem, root, hasCustomOrder, order );

  return legendElem;
}


QString QgsLayerTreeUtils::checkStateToXml( Qt::CheckState state )
{
  switch ( state )
  {
    case Qt::Unchecked:        return "Qt::Unchecked";
    case Qt::PartiallyChecked: return "Qt::PartiallyChecked";
  case Qt::Checked: default: return "Qt::Checked";
  }
}

Qt::CheckState QgsLayerTreeUtils::checkStateFromXml( QString txt )
{
  if ( txt == "Qt::Unchecked" )
    return Qt::Unchecked;
  else if ( txt == "Qt::PartiallyChecked" )
    return Qt::PartiallyChecked;
  else // "Qt::Checked"
    return Qt::Checked;
}



static void _readOldLegendGroup( const QDomElement& groupElem, QgsLayerTreeGroup* parent )
{
  QDomNodeList groupChildren = groupElem.childNodes();

  QgsLayerTreeGroup* groupNode = new QgsLayerTreeGroup( groupElem.attribute( "name" ) );

  groupNode->setVisible( QgsLayerTreeUtils::checkStateFromXml( groupElem.attribute( "checked" ) ) );
  groupNode->setExpanded( groupElem.attribute( "open" ) == "true" );

  if ( groupElem.attribute( "embedded" ) == "1" )
  {
    groupNode->setCustomProperty( "embedded", 1 );
    groupNode->setCustomProperty( "embedded_project", groupElem.attribute( "project" ) );
  }

  for ( int i = 0; i < groupChildren.size(); ++i )
  {
    QDomElement currentChildElem = groupChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == "legendlayer" )
    {
      _readOldLegendLayer( currentChildElem, groupNode );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      _readOldLegendGroup( currentChildElem, groupNode );
    }
  }

  parent->addChildNode( groupNode );
}

static void _readOldLegendLayer( const QDomElement& layerElem, QgsLayerTreeGroup* parent )
{
  QDomElement layerFileElem = layerElem.firstChildElement( "filegroup" ).firstChildElement( "legendlayerfile" );
  QString layerId = layerFileElem.attribute( "layerid" );
  QgsLayerTreeLayer* layerNode = new QgsLayerTreeLayer( layerId, layerElem.attribute( "name" ) );

  layerNode->setVisible( QgsLayerTreeUtils::checkStateFromXml( layerElem.attribute( "checked" ) ) );
  layerNode->setExpanded( layerElem.attribute( "open" ) == "true" );

  if ( layerFileElem.attribute( "isInOverview" ) == "1" )
    layerNode->setCustomProperty( "overview", 1 );

  if ( layerElem.attribute( "embedded" ) == "1" )
    layerNode->setCustomProperty( "embedded", 1 );

  if ( layerElem.attribute( "showFeatureCount" ) == "1" )
    layerNode->setCustomProperty( "showFeatureCount", 1 );

  // drawing order is handled by readOldLegendLayerOrder()

  parent->addChildNode( layerNode );
}



bool QgsLayerTreeUtils::layersEditable( const QList<QgsLayerTreeLayer*>& layerNodes )
{
  foreach ( QgsLayerTreeLayer* layerNode, layerNodes )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer*>( layerNode->layer() );
    if ( !vl )
      continue;

    if ( vl->isEditable() )
      return true;
  }
  return false;
}

bool QgsLayerTreeUtils::layersModified( const QList<QgsLayerTreeLayer*>& layerNodes )
{
  foreach ( QgsLayerTreeLayer* layerNode, layerNodes )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer*>( layerNode->layer() );
    if ( !vl )
      continue;

    if ( vl->isEditable() && vl->isModified() )
      return true;
  }
  return false;
}

void QgsLayerTreeUtils::removeInvalidLayers( QgsLayerTreeGroup* group )
{
  QList<QgsLayerTreeNode*> nodesToRemove;
  foreach ( QgsLayerTreeNode* node, group->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      removeInvalidLayers( QgsLayerTree::toGroup( node ) );
    else if ( QgsLayerTree::isLayer( node ) )
    {
      if ( !QgsLayerTree::toLayer( node )->layer() )
        nodesToRemove << node;
    }
  }

  foreach ( QgsLayerTreeNode* node, nodesToRemove )
    group->removeChildNode( node );
}

void QgsLayerTreeUtils::removeChildrenOfEmbeddedGroups( QgsLayerTreeGroup* group )
{
  foreach ( QgsLayerTreeNode* child, group->children() )
  {
    if ( QgsLayerTree::isGroup( child ) )
    {
      if ( child->customProperty( "embedded" ).toInt() )
        QgsLayerTree::toGroup( child )->removeAllChildren();
      else
        removeChildrenOfEmbeddedGroups( QgsLayerTree::toGroup( child ) );
    }
  }
}


void QgsLayerTreeUtils::updateEmbeddedGroupsProjectPath( QgsLayerTreeGroup* group )
{
  foreach ( QgsLayerTreeNode* node, group->children() )
  {
    if ( !node->customProperty( "embedded_project" ).toString().isEmpty() )
    {
      // may change from absolute path to relative path
      QString newPath = QgsProject::instance()->writePath( node->customProperty( "embedded_project" ).toString() );
      node->setCustomProperty( "embedded_project", newPath );
    }

    if ( QgsLayerTree::isGroup( node ) )
    {
      updateEmbeddedGroupsProjectPath( QgsLayerTree::toGroup( node ) );
    }
  }
}
