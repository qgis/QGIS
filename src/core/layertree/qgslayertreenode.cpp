/***************************************************************************
  qgslayertreenode.cpp
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

#include "qgslayertreenode.h"

#include "qgslayertree.h"
#include "qgslayertreeutils.h"

#include <QDomElement>
#include <QStringList>


QgsLayerTreeNode::QgsLayerTreeNode( QgsLayerTreeNode::NodeType t )
    : mNodeType( t )
    , mParent( 0 )
    , mExpanded( true )
{
}

QgsLayerTreeNode::QgsLayerTreeNode( const QgsLayerTreeNode& other )
    : QObject()
    , mNodeType( other.mNodeType )
    , mParent( 0 )
    , mExpanded( other.mExpanded )
    , mProperties( other.mProperties )
{
  QList<QgsLayerTreeNode*> clonedChildren;
  foreach ( QgsLayerTreeNode* child, other.mChildren )
    clonedChildren << child->clone();
  insertChildrenPrivate( -1, clonedChildren );
}

QgsLayerTreeNode::~QgsLayerTreeNode()
{
  qDeleteAll( mChildren );
}

QgsLayerTreeNode* QgsLayerTreeNode::readXML( QDomElement& element )
{
  QgsLayerTreeNode* node = 0;
  if ( element.tagName() == "layer-tree-group" )
    node = QgsLayerTreeGroup::readXML( element );
  else if ( element.tagName() == "layer-tree-layer" )
    node = QgsLayerTreeLayer::readXML( element );

  return node;
}


bool QgsLayerTreeNode::isExpanded() const
{
  return mExpanded;
}


void QgsLayerTreeNode::setExpanded( bool expanded )
{
  if ( mExpanded == expanded )
    return;

  mExpanded = expanded;
  emit expandedChanged( this, expanded );
}


void QgsLayerTreeNode::setCustomProperty( const QString &key, const QVariant &value )
{
  mProperties.setValue( key, value );
  emit customPropertyChanged( this, key );
}

QVariant QgsLayerTreeNode::customProperty( const QString &key, const QVariant &defaultValue ) const
{
  return mProperties.value( key, defaultValue );
}

void QgsLayerTreeNode::removeCustomProperty( const QString &key )
{
  mProperties.remove( key );
  emit customPropertyChanged( this, key );
}

QStringList QgsLayerTreeNode::customProperties() const
{
  return mProperties.keys();
}

void QgsLayerTreeNode::readCommonXML( QDomElement& element )
{
  mProperties.readXml( element );
}

void QgsLayerTreeNode::writeCommonXML( QDomElement& element )
{
  QDomDocument doc( element.ownerDocument() );
  mProperties.writeXml( element, doc );
}

void QgsLayerTreeNode::insertChildrenPrivate( int index, QList<QgsLayerTreeNode*> nodes )
{
  if ( nodes.count() == 0 )
    return;

  foreach ( QgsLayerTreeNode* node, nodes )
  {
    Q_ASSERT( node->mParent == 0 );
    node->mParent = this;
  }

  if ( index < 0 || index >= mChildren.count() )
    index = mChildren.count();

  int indexTo = index + nodes.count() - 1;
  emit willAddChildren( this, index, indexTo );
  for ( int i = 0; i < nodes.count(); ++i )
  {
    mChildren.insert( index + i, nodes[i] );

    // forward the signal towards the root
    connect( nodes[i], SIGNAL( willAddChildren( QgsLayerTreeNode*, int, int ) ), this, SIGNAL( willAddChildren( QgsLayerTreeNode*, int, int ) ) );
    connect( nodes[i], SIGNAL( addedChildren( QgsLayerTreeNode*, int, int ) ), this, SIGNAL( addedChildren( QgsLayerTreeNode*, int, int ) ) );
    connect( nodes[i], SIGNAL( willRemoveChildren( QgsLayerTreeNode*, int, int ) ), this, SIGNAL( willRemoveChildren( QgsLayerTreeNode*, int, int ) ) );
    connect( nodes[i], SIGNAL( removedChildren( QgsLayerTreeNode*, int, int ) ), this, SIGNAL( removedChildren( QgsLayerTreeNode*, int, int ) ) );
    connect( nodes[i], SIGNAL( customPropertyChanged( QgsLayerTreeNode*, QString ) ), this, SIGNAL( customPropertyChanged( QgsLayerTreeNode*, QString ) ) );
    connect( nodes[i], SIGNAL( visibilityChanged( QgsLayerTreeNode*, Qt::CheckState ) ), this, SIGNAL( visibilityChanged( QgsLayerTreeNode*, Qt::CheckState ) ) );
    connect( nodes[i], SIGNAL( expandedChanged( QgsLayerTreeNode*, bool ) ), this, SIGNAL( expandedChanged( QgsLayerTreeNode*, bool ) ) );
  }
  emit addedChildren( this, index, indexTo );
}

void QgsLayerTreeNode::removeChildrenPrivate( int from, int count )
{
  if ( from < 0 || count <= 0 )
    return;

  int to = from + count - 1;
  if ( to >= mChildren.count() )
    return;
  emit willRemoveChildren( this, from, to );
  while ( --count >= 0 )
  {
    QgsLayerTreeNode* node = mChildren.takeAt( from );
    node->mParent = 0;
    delete node;
  }
  emit removedChildren( this, from, to );
}
