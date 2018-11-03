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
#include "qgslogger.h"

#include <QDomElement>


static void _readOldLegendGroup( const QDomElement &groupElem, QgsLayerTreeGroup *parent );
static void _readOldLegendLayer( const QDomElement &layerElem, QgsLayerTreeGroup *parent );

bool QgsLayerTreeUtils::readOldLegend( QgsLayerTreeGroup *root, const QDomElement &legendElem )
{
  if ( legendElem.isNull() )
    return false;

  QDomNodeList legendChildren = legendElem.childNodes();

  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == QLatin1String( "legendlayer" ) )
    {
      _readOldLegendLayer( currentChildElem, root );
    }
    else if ( currentChildElem.tagName() == QLatin1String( "legendgroup" ) )
    {
      _readOldLegendGroup( currentChildElem, root );
    }
  }

  return true;
}



static bool _readOldLegendLayerOrderGroup( const QDomElement &groupElem, QMap<int, QString> &layerIndexes )
{
  QDomNodeList legendChildren = groupElem.childNodes();

  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == QLatin1String( "legendlayer" ) )
    {
      QDomElement layerFileElem = currentChildElem.firstChildElement( QStringLiteral( "filegroup" ) ).firstChildElement( QStringLiteral( "legendlayerfile" ) );

      int layerIndex = currentChildElem.attribute( QStringLiteral( "drawingOrder" ) ).toInt();
      if ( layerIndex == -1 )
        return false; // order undefined
      layerIndexes.insert( layerIndex, layerFileElem.attribute( QStringLiteral( "layerid" ) ) );
    }
    else if ( currentChildElem.tagName() == QLatin1String( "legendgroup" ) )
    {
      if ( !_readOldLegendLayerOrderGroup( currentChildElem, layerIndexes ) )
        return false;
    }
  }

  return true;
}


bool QgsLayerTreeUtils::readOldLegendLayerOrder( const QDomElement &legendElem, bool &hasCustomOrder, QStringList &order )
{
  if ( legendElem.isNull() )
    return false;

  hasCustomOrder = legendElem.attribute( QStringLiteral( "updateDrawingOrder" ) ) == QLatin1String( "false" );
  order.clear();

  QMap<int, QString> layerIndexes;

  // try to read the order. may be undefined (order = -1) for some or all items
  bool res = _readOldLegendLayerOrderGroup( legendElem, layerIndexes );

  if ( !res && hasCustomOrder )
    return false; // invalid state

  Q_FOREACH ( const QString &layerId, layerIndexes )
  {
    QgsDebugMsg( layerId );
    order.append( layerId );
  }

  return true;
}


static QDomElement _writeOldLegendLayer( QDomDocument &doc, QgsLayerTreeLayer *nodeLayer, bool hasCustomOrder, const QList<QgsMapLayer *> &order )
{
  int drawingOrder = -1;
  if ( hasCustomOrder )
    drawingOrder = order.indexOf( nodeLayer->layer() );

  QDomElement layerElem = doc.createElement( QStringLiteral( "legendlayer" ) );
  layerElem.setAttribute( QStringLiteral( "drawingOrder" ), drawingOrder );
  layerElem.setAttribute( QStringLiteral( "open" ), nodeLayer->isExpanded() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  layerElem.setAttribute( QStringLiteral( "checked" ), QgsLayerTreeUtils::checkStateToXml( nodeLayer->itemVisibilityChecked() ? Qt::Checked : Qt::Unchecked ) );
  layerElem.setAttribute( QStringLiteral( "name" ), nodeLayer->name() );
  layerElem.setAttribute( QStringLiteral( "showFeatureCount" ), nodeLayer->customProperty( QStringLiteral( "showFeatureCount" ) ).toInt() );

  QDomElement fileGroupElem = doc.createElement( QStringLiteral( "filegroup" ) );
  fileGroupElem.setAttribute( QStringLiteral( "open" ), nodeLayer->isExpanded() ? "true" : "false" );
  fileGroupElem.setAttribute( QStringLiteral( "hidden" ), QStringLiteral( "false" ) );

  QDomElement layerFileElem = doc.createElement( QStringLiteral( "legendlayerfile" ) );
  layerFileElem.setAttribute( QStringLiteral( "isInOverview" ), nodeLayer->customProperty( QStringLiteral( "overview" ) ).toInt() );
  layerFileElem.setAttribute( QStringLiteral( "layerid" ), nodeLayer->layerId() );
  layerFileElem.setAttribute( QStringLiteral( "visible" ), nodeLayer->isVisible() ? 1 : 0 );

  layerElem.appendChild( fileGroupElem );
  fileGroupElem.appendChild( layerFileElem );
  return layerElem;
}

// need forward declaration as write[..]Group and write[..]GroupChildren call each other
static void _writeOldLegendGroupChildren( QDomDocument &doc, QDomElement &groupElem, QgsLayerTreeGroup *nodeGroup, bool hasCustomOrder, const QList<QgsMapLayer *> &order );

static QDomElement _writeOldLegendGroup( QDomDocument &doc, QgsLayerTreeGroup *nodeGroup, bool hasCustomOrder, const QList<QgsMapLayer *> &order )
{
  QDomElement groupElem = doc.createElement( QStringLiteral( "legendgroup" ) );
  groupElem.setAttribute( QStringLiteral( "open" ), nodeGroup->isExpanded() ? "true" : "false" );
  groupElem.setAttribute( QStringLiteral( "name" ), nodeGroup->name() );
  groupElem.setAttribute( QStringLiteral( "checked" ), QgsLayerTreeUtils::checkStateToXml( nodeGroup->itemVisibilityChecked() ? Qt::Checked : Qt::Unchecked ) );

  if ( nodeGroup->customProperty( QStringLiteral( "embedded" ) ).toInt() )
  {
    groupElem.setAttribute( QStringLiteral( "embedded" ), 1 );
    groupElem.setAttribute( QStringLiteral( "project" ), nodeGroup->customProperty( QStringLiteral( "embedded_project" ) ).toString() );
  }

  _writeOldLegendGroupChildren( doc, groupElem, nodeGroup, hasCustomOrder, order );
  return groupElem;
}


static void _writeOldLegendGroupChildren( QDomDocument &doc, QDomElement &groupElem, QgsLayerTreeGroup *nodeGroup, bool hasCustomOrder, const QList<QgsMapLayer *> &order )
{
  Q_FOREACH ( QgsLayerTreeNode *node, nodeGroup->children() )
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


QDomElement QgsLayerTreeUtils::writeOldLegend( QDomDocument &doc, QgsLayerTreeGroup *root, bool hasCustomOrder, const QList<QgsMapLayer *> &order )
{
  QDomElement legendElem = doc.createElement( QStringLiteral( "legend" ) );
  legendElem.setAttribute( QStringLiteral( "updateDrawingOrder" ), hasCustomOrder ? QStringLiteral( "false" ) : QStringLiteral( "true" ) );

  _writeOldLegendGroupChildren( doc, legendElem, root, hasCustomOrder, order );

  return legendElem;
}


QString QgsLayerTreeUtils::checkStateToXml( Qt::CheckState state )
{
  switch ( state )
  {
    case Qt::Unchecked:
      return QStringLiteral( "Qt::Unchecked" );
    case Qt::PartiallyChecked:
      return QStringLiteral( "Qt::PartiallyChecked" );
    case Qt::Checked:
    default:
      return QStringLiteral( "Qt::Checked" );
  }
}

Qt::CheckState QgsLayerTreeUtils::checkStateFromXml( const QString &txt )
{
  if ( txt == QLatin1String( "Qt::Unchecked" ) )
    return Qt::Unchecked;
  else if ( txt == QLatin1String( "Qt::PartiallyChecked" ) )
    return Qt::PartiallyChecked;
  else // "Qt::Checked"
    return Qt::Checked;
}



static void _readOldLegendGroup( const QDomElement &groupElem, QgsLayerTreeGroup *parent )
{
  QDomNodeList groupChildren = groupElem.childNodes();

  QgsLayerTreeGroup *groupNode = new QgsLayerTreeGroup( groupElem.attribute( QStringLiteral( "name" ) ) );

  groupNode->setItemVisibilityChecked( QgsLayerTreeUtils::checkStateFromXml( groupElem.attribute( QStringLiteral( "checked" ) ) ) != Qt::Unchecked );
  groupNode->setExpanded( groupElem.attribute( QStringLiteral( "open" ) ) == QLatin1String( "true" ) );

  if ( groupElem.attribute( QStringLiteral( "embedded" ) ) == QLatin1String( "1" ) )
  {
    groupNode->setCustomProperty( QStringLiteral( "embedded" ), 1 );
    groupNode->setCustomProperty( QStringLiteral( "embedded_project" ), groupElem.attribute( QStringLiteral( "project" ) ) );
  }

  for ( int i = 0; i < groupChildren.size(); ++i )
  {
    QDomElement currentChildElem = groupChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == QLatin1String( "legendlayer" ) )
    {
      _readOldLegendLayer( currentChildElem, groupNode );
    }
    else if ( currentChildElem.tagName() == QLatin1String( "legendgroup" ) )
    {
      _readOldLegendGroup( currentChildElem, groupNode );
    }
  }

  parent->addChildNode( groupNode );
}

static void _readOldLegendLayer( const QDomElement &layerElem, QgsLayerTreeGroup *parent )
{
  QDomElement layerFileElem = layerElem.firstChildElement( QStringLiteral( "filegroup" ) ).firstChildElement( QStringLiteral( "legendlayerfile" ) );
  QString layerId = layerFileElem.attribute( QStringLiteral( "layerid" ) );
  QgsLayerTreeLayer *layerNode = new QgsLayerTreeLayer( layerId, layerElem.attribute( QStringLiteral( "name" ) ) );

  layerNode->setItemVisibilityChecked( QgsLayerTreeUtils::checkStateFromXml( layerElem.attribute( QStringLiteral( "checked" ) ) ) != Qt::Unchecked );
  layerNode->setExpanded( layerElem.attribute( QStringLiteral( "open" ) ) == QLatin1String( "true" ) );

  if ( layerFileElem.attribute( QStringLiteral( "isInOverview" ) ) == QLatin1String( "1" ) )
    layerNode->setCustomProperty( QStringLiteral( "overview" ), 1 );

  if ( layerElem.attribute( QStringLiteral( "embedded" ) ) == QLatin1String( "1" ) )
    layerNode->setCustomProperty( QStringLiteral( "embedded" ), 1 );

  if ( layerElem.attribute( QStringLiteral( "showFeatureCount" ) ) == QLatin1String( "1" ) )
    layerNode->setCustomProperty( QStringLiteral( "showFeatureCount" ), 1 );

  // drawing order is handled by readOldLegendLayerOrder()

  parent->addChildNode( layerNode );
}



bool QgsLayerTreeUtils::layersEditable( const QList<QgsLayerTreeLayer *> &layerNodes )
{
  Q_FOREACH ( QgsLayerTreeLayer *layerNode, layerNodes )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layerNode->layer() );
    if ( !vl )
      continue;

    if ( vl->isEditable() )
      return true;
  }
  return false;
}

bool QgsLayerTreeUtils::layersModified( const QList<QgsLayerTreeLayer *> &layerNodes )
{
  Q_FOREACH ( QgsLayerTreeLayer *layerNode, layerNodes )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layerNode->layer() );
    if ( !vl )
      continue;

    if ( vl->isEditable() && vl->isModified() )
      return true;
  }
  return false;
}

void QgsLayerTreeUtils::removeInvalidLayers( QgsLayerTreeGroup *group )
{
  QList<QgsLayerTreeNode *> nodesToRemove;
  Q_FOREACH ( QgsLayerTreeNode *node, group->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      removeInvalidLayers( QgsLayerTree::toGroup( node ) );
    else if ( QgsLayerTree::isLayer( node ) )
    {
      if ( !QgsLayerTree::toLayer( node )->layer() )
        nodesToRemove << node;
    }
  }

  Q_FOREACH ( QgsLayerTreeNode *node, nodesToRemove )
    group->removeChildNode( node );
}

void QgsLayerTreeUtils::storeOriginalLayersProperties( QgsLayerTreeGroup *group,  const QDomDocument *doc )
{
  const QDomNodeList mlNodeList( doc->documentElement()
                                 .firstChildElement( QStringLiteral( "projectlayers" ) )
                                 .elementsByTagName( QStringLiteral( "maplayer" ) ) );
  for ( QgsLayerTreeNode *node : group->children() )
  {
    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsMapLayer *l( QgsLayerTree::toLayer( node )->layer() );
      if ( l )
      {
        for ( int i = 0; i < mlNodeList.count(); i++ )
        {
          QDomNode mlNode( mlNodeList.at( i ) );
          QString id( mlNode.firstChildElement( QStringLiteral( "id" ) ).firstChild().nodeValue() );
          if ( id == l->id() )
          {
            QDomImplementation DomImplementation;
            QDomDocumentType documentType = DomImplementation.createDocumentType( QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
            QDomDocument document( documentType );
            QDomElement element = mlNode.toElement();
            document.appendChild( element );
            QString str;
            QTextStream stream( &str );
            document.save( stream, 4 /*indent*/ );
            l->setOriginalXmlProperties( str );
          }
        }
      }
    }
  }
}

QStringList QgsLayerTreeUtils::invisibleLayerList( QgsLayerTreeNode *node )
{
  QStringList list;

  if ( QgsLayerTree::isGroup( node ) )
  {
    Q_FOREACH ( QgsLayerTreeNode *child, QgsLayerTree::toGroup( node )->children() )
    {
      list << invisibleLayerList( child );
    }
  }
  else if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeLayer *layer = QgsLayerTree::toLayer( node );

    if ( !layer->isVisible() )
      list << layer->layerId();
  }

  return list;
}

void QgsLayerTreeUtils::replaceChildrenOfEmbeddedGroups( QgsLayerTreeGroup *group )
{
  Q_FOREACH ( QgsLayerTreeNode *child, group->children() )
  {
    if ( QgsLayerTree::isGroup( child ) )
    {
      if ( child->customProperty( QStringLiteral( "embedded" ) ).toInt() )
      {
        child->setCustomProperty( QStringLiteral( "embedded-invisible-layers" ), invisibleLayerList( child ) );
        QgsLayerTree::toGroup( child )->removeAllChildren();
      }
      else
      {
        replaceChildrenOfEmbeddedGroups( QgsLayerTree::toGroup( child ) );
      }
    }
  }
}


void QgsLayerTreeUtils::updateEmbeddedGroupsProjectPath( QgsLayerTreeGroup *group, const QgsProject *project )
{
  Q_FOREACH ( QgsLayerTreeNode *node, group->children() )
  {
    if ( !node->customProperty( QStringLiteral( "embedded_project" ) ).toString().isEmpty() )
    {
      // may change from absolute path to relative path
      QString newPath = project->writePath( node->customProperty( QStringLiteral( "embedded_project" ) ).toString() );
      node->setCustomProperty( QStringLiteral( "embedded_project" ), newPath );
    }

    if ( QgsLayerTree::isGroup( node ) )
    {
      updateEmbeddedGroupsProjectPath( QgsLayerTree::toGroup( node ), project );
    }
  }
}

void QgsLayerTreeUtils::setLegendFilterByExpression( QgsLayerTreeLayer &layer, const QString &expr, bool enabled )
{
  layer.setCustomProperty( QStringLiteral( "legend/expressionFilter" ), expr );
  layer.setCustomProperty( QStringLiteral( "legend/expressionFilterEnabled" ), enabled );
}

QString QgsLayerTreeUtils::legendFilterByExpression( const QgsLayerTreeLayer &layer, bool *enabled )
{
  if ( enabled )
    *enabled = layer.customProperty( QStringLiteral( "legend/expressionFilterEnabled" ), "" ).toBool();
  return layer.customProperty( QStringLiteral( "legend/expressionFilter" ), "" ).toString();
}

bool QgsLayerTreeUtils::hasLegendFilterExpression( const QgsLayerTreeGroup &group )
{
  Q_FOREACH ( QgsLayerTreeLayer *l, group.findLayers() )
  {
    bool exprEnabled;
    QString expr = legendFilterByExpression( *l, &exprEnabled );
    if ( exprEnabled && !expr.isEmpty() )
    {
      return true;
    }
  }
  return false;
}

QgsLayerTreeLayer *QgsLayerTreeUtils::insertLayerBelow( QgsLayerTreeGroup *group, const QgsMapLayer *refLayer, QgsMapLayer *layerToInsert )
{
  // get the index of the reflayer
  QgsLayerTreeLayer *inTree = group->findLayer( refLayer->id() );
  if ( !inTree )
    return nullptr;

  int idx = 0;
  Q_FOREACH ( QgsLayerTreeNode *vl, inTree->parent()->children() )
  {
    if ( vl->nodeType() == QgsLayerTreeNode::NodeLayer && static_cast<QgsLayerTreeLayer *>( vl )->layer() == refLayer )
    {
      break;
    }
    idx++;
  }
  // insert the new layer
  QgsLayerTreeGroup *parent = static_cast<QgsLayerTreeGroup *>( inTree->parent() ) ? static_cast<QgsLayerTreeGroup *>( inTree->parent() ) : group;
  return parent->insertLayer( idx, layerToInsert );
}

static void _collectMapLayers( const QList<QgsLayerTreeNode *> &nodes, QSet<QgsMapLayer *> &layersSet )
{
  for ( QgsLayerTreeNode *node : nodes )
  {
    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->layer() )
        layersSet << nodeLayer->layer();
    }
    else if ( QgsLayerTree::isGroup( node ) )
    {
      _collectMapLayers( QgsLayerTree::toGroup( node )->children(), layersSet );
    }
  }
}

QSet<QgsMapLayer *> QgsLayerTreeUtils::collectMapLayersRecursive( const QList<QgsLayerTreeNode *> &nodes )
{
  QSet<QgsMapLayer *> layersSet;
  _collectMapLayers( nodes, layersSet );
  return layersSet;
}

int QgsLayerTreeUtils::countMapLayerInTree( QgsLayerTreeNode *tree, QgsMapLayer *layer )
{
  if ( QgsLayerTree::isLayer( tree ) )
  {
    if ( QgsLayerTree::toLayer( tree )->layer() == layer )
      return 1;
    return 0;
  }

  int cnt = 0;
  const QList<QgsLayerTreeNode *> children = tree->children();
  for ( QgsLayerTreeNode *child : children )
    cnt += countMapLayerInTree( child, layer );
  return cnt;
}
