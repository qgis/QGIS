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

#include <QDomElement>

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
      addLegendLayerToTreeWidget( currentChildElem, root );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      addLegendGroupToTreeWidget( currentChildElem, root );
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



void QgsLayerTreeUtils::addLegendGroupToTreeWidget( const QDomElement& groupElem, QgsLayerTreeGroup* parent )
{
  QDomNodeList groupChildren = groupElem.childNodes();

  QgsLayerTreeGroup* groupNode = new QgsLayerTreeGroup( groupElem.attribute( "name" ) );
  parent->addChildNode( groupNode );

  groupNode->setVisible( checkStateFromXml( groupElem.attribute( "checked" ) ) );
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
      addLegendLayerToTreeWidget( currentChildElem, groupNode );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      addLegendGroupToTreeWidget( currentChildElem, groupNode );
    }
  }
}

void QgsLayerTreeUtils::addLegendLayerToTreeWidget( const QDomElement& layerElem, QgsLayerTreeGroup* parent )
{
  QDomElement layerFileElem = layerElem.firstChildElement( "filegroup" ).firstChildElement( "legendlayerfile" );
  QString layerId = layerFileElem.attribute( "layerid" );
  QgsLayerTreeLayer* layerNode = new QgsLayerTreeLayer( layerId, layerElem.attribute( "name" ) );

  layerNode->setVisible( checkStateFromXml( layerElem.attribute( "checked" ) ) );
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
