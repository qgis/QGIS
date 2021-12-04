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


QgsLayerTreeNode::QgsLayerTreeNode( QgsLayerTreeNode::NodeType t, bool checked )
  : mNodeType( t )
  , mChecked( checked )
  , mExpanded( true )
{
}

QgsLayerTreeNode::QgsLayerTreeNode( const QgsLayerTreeNode &other )
  : QObject( nullptr )
  , mNodeType( other.mNodeType )
  , mChecked( other.mChecked )
  , mExpanded( other.mExpanded )
  , mProperties( other.mProperties )
{
  QList<QgsLayerTreeNode *> clonedChildren;

  for ( QgsLayerTreeNode *child : std::as_const( other.mChildren ) )
    clonedChildren << child->clone();
  insertChildrenPrivate( -1, clonedChildren );
}

QgsLayerTreeNode::~QgsLayerTreeNode()
{
  qDeleteAll( mChildren );
}

QList<QgsLayerTreeNode *> QgsLayerTreeNode::abandonChildren()
{
  const QList<QgsLayerTreeNode *> orphans { mChildren };
  mChildren.clear();
  for ( auto orphan : std::as_const( orphans ) )
  {
    orphan->makeOrphan( );
  }
  return orphans;
}

void QgsLayerTreeNode::makeOrphan()
{
  disconnect();
  mParent = nullptr;
}

QgsLayerTreeNode *QgsLayerTreeNode::readXml( QDomElement &element, const QgsReadWriteContext &context )
{
  QgsLayerTreeNode *node = nullptr;
  if ( element.tagName() == QLatin1String( "layer-tree-group" ) )
    node = QgsLayerTreeGroup::readXml( element, context );
  else if ( element.tagName() == QLatin1String( "layer-tree-layer" ) )
    node = QgsLayerTreeLayer::readXml( element, context );

  return node;
}

QgsLayerTreeNode *QgsLayerTreeNode::readXml( QDomElement &element, const QgsProject *project )
{
  QgsReadWriteContext context;
  QgsPathResolver resolver;
  if ( project )
    resolver = project->pathResolver();
  context.setPathResolver( resolver );
  context.setProjectTranslator( const_cast<QgsProject *>( project ) );

  QgsLayerTreeNode *node = readXml( element, context );
  if ( node )
    node->resolveReferences( project );
  return node;
}


void QgsLayerTreeNode::setItemVisibilityChecked( bool checked )
{
  if ( mChecked == checked )
    return;
  mChecked = checked;
  emit visibilityChanged( this );
}

void QgsLayerTreeNode::setItemVisibilityCheckedRecursive( bool checked )
{
  setItemVisibilityChecked( checked );
}

void QgsLayerTreeNode::setItemVisibilityCheckedParentRecursive( bool checked )
{
  setItemVisibilityChecked( checked );
  if ( mParent )
    mParent->setItemVisibilityCheckedParentRecursive( checked );
}

bool QgsLayerTreeNode::isVisible() const
{
  return mChecked && ( !mParent || mParent->isVisible() );
}


bool QgsLayerTreeNode::isExpanded() const
{
  return mExpanded;
}

bool QgsLayerTreeNode::isItemVisibilityCheckedRecursive() const
{
  if ( !mChecked )
    return false;
  const auto constMChildren = mChildren;
  for ( QgsLayerTreeNode *child : constMChildren )
  {
    if ( !child->isItemVisibilityCheckedRecursive() )
      return false;
  }

  return true;
}

bool QgsLayerTreeNode::isItemVisibilityUncheckedRecursive() const
{
  if ( mChecked )
    return false;
  const auto constMChildren = mChildren;
  for ( QgsLayerTreeNode *child : constMChildren )
  {
    if ( !child->isItemVisibilityUncheckedRecursive() )
      return false;
  }

  return true;
}

void fetchCheckedLayers( const QgsLayerTreeNode *node, QList<QgsMapLayer *> &layers )
{
  if ( QgsLayerTree::isLayer( node ) )
  {
    const QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
    if ( nodeLayer->isVisible() )
      layers << nodeLayer->layer();
  }

  const auto constChildren = node->children();
  for ( QgsLayerTreeNode *child : constChildren )
  {
    if ( QgsLayerTreeGroup *group = qobject_cast< QgsLayerTreeGroup * >( child ) )
    {
      if ( QgsGroupLayer *groupLayer = group->groupLayer() )
      {
        layers << groupLayer;
        continue;
      }
    }

    fetchCheckedLayers( child, layers );
  }
}

QList<QgsMapLayer *> QgsLayerTreeNode::checkedLayers() const
{
  QList<QgsMapLayer *> layers;
  fetchCheckedLayers( this, layers );
  return layers;
}

int QgsLayerTreeNode::depth() const
{
  int depth = 0;
  QgsLayerTreeNode *node = mParent;
  while ( node )
  {
    node = node->parent();
    ++depth;
  }
  return depth;
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
  if ( !mProperties.contains( key ) || mProperties.value( key ) != value )
  {
    mProperties.setValue( key, value );
    emit customPropertyChanged( this, key );
  }
}

QVariant QgsLayerTreeNode::customProperty( const QString &key, const QVariant &defaultValue ) const
{
  return mProperties.value( key, defaultValue );
}

void QgsLayerTreeNode::removeCustomProperty( const QString &key )
{
  if ( mProperties.contains( key ) )
  {
    mProperties.remove( key );
    emit customPropertyChanged( this, key );
  }
}

QStringList QgsLayerTreeNode::customProperties() const
{
  return mProperties.keys();
}

void QgsLayerTreeNode::readCommonXml( QDomElement &element )
{
  mProperties.readXml( element );
}

void QgsLayerTreeNode::writeCommonXml( QDomElement &element )
{
  QDomDocument doc( element.ownerDocument() );
  mProperties.writeXml( element, doc );
}

void QgsLayerTreeNode::insertChildrenPrivate( int index, const QList<QgsLayerTreeNode *> &nodes )
{
  if ( nodes.isEmpty() )
    return;

  for ( QgsLayerTreeNode *node : nodes )
  {
    Q_ASSERT( !node->mParent );
    node->mParent = this;
  }

  if ( index < 0 || index >= mChildren.count() )
    index = mChildren.count();

  for ( int i = 0; i < nodes.count(); ++i )
  {
    QgsLayerTreeNode *node = nodes.at( i );

    const QList<QgsLayerTreeNode *> orphans { node->abandonChildren() };

    emit willAddChildren( this, index + i, index + i );
    mChildren.insert( index + i, node );
    emit addedChildren( this, index + i, index + i );

    // forward the signal towards the root
    connect( node, &QgsLayerTreeNode::willAddChildren, this, &QgsLayerTreeNode::willAddChildren );
    connect( node, &QgsLayerTreeNode::addedChildren, this, &QgsLayerTreeNode::addedChildren );
    connect( node, &QgsLayerTreeNode::willRemoveChildren, this, &QgsLayerTreeNode::willRemoveChildren );
    connect( node, &QgsLayerTreeNode::removedChildren, this, &QgsLayerTreeNode::removedChildren );
    connect( node, &QgsLayerTreeNode::customPropertyChanged, this, &QgsLayerTreeNode::customPropertyChanged );
    connect( node, &QgsLayerTreeNode::visibilityChanged, this, &QgsLayerTreeNode::visibilityChanged );
    connect( node, &QgsLayerTreeNode::expandedChanged, this, &QgsLayerTreeNode::expandedChanged );
    connect( node, &QgsLayerTreeNode::nameChanged, this, &QgsLayerTreeNode::nameChanged );

    // Now add children
    if ( ! orphans.isEmpty() )
    {
      node->insertChildrenPrivate( -1, orphans );
    }

  }
}

void QgsLayerTreeNode::removeChildrenPrivate( int from, int count, bool destroy )
{
  if ( from < 0 || count <= 0 )
    return;

  const int to = from + count - 1;
  if ( to >= mChildren.count() )
    return;

  // Remove in reverse order
  while ( --count >= 0 )
  {
    const int last { from + count };
    Q_ASSERT( last >= 0 && last < mChildren.count( ) );
    QgsLayerTreeNode *node = mChildren.at( last );

    // Remove children first
    if ( ! node->children().isEmpty() )
    {
      node->removeChildrenPrivate( 0, node->children().count( ), destroy );
    }

    emit willRemoveChildren( this, last, last );
    node = mChildren.takeAt( last );
    if ( destroy )
    {
      delete node;
    }
    else
    {
      node->makeOrphan();
    }
    emit removedChildren( this, last, last );
  }
}

bool QgsLayerTreeNode::takeChild( QgsLayerTreeNode *node )
{
  int index = mChildren.indexOf( node );
  if ( index < 0 )
    return false;

  int n = mChildren.size();

  removeChildrenPrivate( index, 1, false );

  return mChildren.size() < n;
}
