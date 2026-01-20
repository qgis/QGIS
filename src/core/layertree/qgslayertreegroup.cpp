/***************************************************************************
  qgslayertreegroup.cpp
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

#include "qgslayertreegroup.h"

#include "qgsgrouplayer.h"
#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgsmaplayer.h"

#include <QDomElement>
#include <QStringList>

#include "moc_qgslayertreegroup.cpp"

QgsLayerTreeGroup::QgsLayerTreeGroup( const QString &name, bool checked )
  : QgsLayerTreeNode( NodeGroup, checked )
  , mName( name )
  , mServerProperties( std::make_unique<QgsMapLayerServerProperties>() )
{
  init();
}

QgsLayerTreeGroup::QgsLayerTreeGroup( const QgsLayerTreeGroup &other )
  : QgsLayerTreeNode( other )
  , mName( other.mName )
  , mChangingChildVisibility( other.mChangingChildVisibility )
  , mMutuallyExclusive( other.mMutuallyExclusive )
  , mMutuallyExclusiveChildIndex( other.mMutuallyExclusiveChildIndex )
  , mWmsHasTimeDimension( other.mWmsHasTimeDimension )
  , mGroupLayer( other.mGroupLayer )
  , mServerProperties( std::make_unique<QgsMapLayerServerProperties>() )
{
  other.serverProperties()->copyTo( mServerProperties.get() );

  init();
}

void QgsLayerTreeGroup::init()
{
  connect( this, &QgsLayerTreeNode::visibilityChanged, this, &QgsLayerTreeGroup::nodeVisibilityChanged );
  connect( this, &QgsLayerTreeNode::addedChildren, this, &QgsLayerTreeGroup::updateGroupLayers );
  connect( this, &QgsLayerTreeNode::removedChildren, this, &QgsLayerTreeGroup::updateGroupLayers );
  connect( this, &QgsLayerTreeNode::visibilityChanged, this, &QgsLayerTreeGroup::updateGroupLayers );
}

QString QgsLayerTreeGroup::name() const
{
  return mName;
}

void QgsLayerTreeGroup::setName( const QString &n )
{
  if ( mName == n )
    return;

  mName = n;
  emit nameChanged( this, n );
}


QgsLayerTreeGroup *QgsLayerTreeGroup::insertGroup( int index, const QString &name )
{
  QgsLayerTreeGroup *grp = new QgsLayerTreeGroup( name );
  insertChildNode( index, grp );
  return grp;
}

QgsLayerTreeGroup *QgsLayerTreeGroup::addGroup( const QString &name )
{
  QgsLayerTreeGroup *grp = new QgsLayerTreeGroup( name );
  addChildNode( grp );
  return grp;
}

QgsLayerTreeLayer *QgsLayerTreeGroup::insertLayer( int index, QgsMapLayer *layer )
{
  if ( !layer )
    return nullptr;

  QgsLayerTreeLayer *ll = new QgsLayerTreeLayer( layer );
  insertChildNode( index, ll );

  updateGroupLayers();
  return ll;
}

QgsLayerTreeLayer *QgsLayerTreeGroup::addLayer( QgsMapLayer *layer )
{
  if ( !layer )
    return nullptr;

  QgsLayerTreeLayer *ll = new QgsLayerTreeLayer( layer );
  addChildNode( ll );

  updateGroupLayers();
  return ll;
}

QgsLayerTreeCustomNode *QgsLayerTreeGroup::insertCustomNode( int index, const QString &id, const QString &name )
{
  if ( id.trimmed().isEmpty() )
    return nullptr;

  // Avoid registering two custom nodes with the same id
  const QStringList customNodeIds = findCustomNodeIds();
  if ( customNodeIds.contains( id ) )
    return nullptr;

  QgsLayerTreeCustomNode *customNode = new QgsLayerTreeCustomNode( id, name );
  insertChildNode( index, customNode );
  return customNode;
}

QgsLayerTreeCustomNode *QgsLayerTreeGroup::insertCustomNode( int index, QgsLayerTreeCustomNode *node SIP_TRANSFER )
{
  if ( node->nodeId().trimmed().isEmpty() )
    return nullptr;

  // Avoid registering two custom nodes with the same id
  const QStringList customNodeIds = findCustomNodeIds();
  if ( customNodeIds.contains( node->nodeId() ) )
    return nullptr;

  insertChildNode( index, node );
  return node;
}

QgsLayerTreeCustomNode *QgsLayerTreeGroup::addCustomNode( const QString &id, const QString &name )
{
  if ( id.trimmed().isEmpty() )
    return nullptr;

  // Avoid registering two custom nodes with the same id
  const QStringList customNodeIds = findCustomNodeIds();
  if ( customNodeIds.contains( id ) )
    return nullptr;

  QgsLayerTreeCustomNode *customNode = new QgsLayerTreeCustomNode( id, name );
  addChildNode( customNode );
  return customNode;
}

void QgsLayerTreeGroup::insertChildNode( int index, QgsLayerTreeNode *node )
{
  QList<QgsLayerTreeNode *> nodes;
  nodes << node;
  insertChildNodes( index, nodes );
}

void QgsLayerTreeGroup::insertChildNodes( int index, const QList<QgsLayerTreeNode *> &nodes )
{
  QgsLayerTreeNode *meChild = nullptr;
  if ( mMutuallyExclusive && mMutuallyExclusiveChildIndex >= 0 && mMutuallyExclusiveChildIndex < mChildren.count() )
    meChild = mChildren.at( mMutuallyExclusiveChildIndex );

  // low-level insert
  insertChildrenPrivate( index, nodes );

  if ( mMutuallyExclusive )
  {
    if ( meChild )
    {
      // the child could have change its index - or the new children may have been also set as visible
      mMutuallyExclusiveChildIndex = mChildren.indexOf( meChild );
    }
    else if ( mChecked )
    {
      // we have not picked a child index yet, but we should pick one now
      // ... so pick the first one from the newly added
      if ( index == -1 )
        index = mChildren.count() - nodes.count(); // get real insertion index
      mMutuallyExclusiveChildIndex = index;
    }
    updateChildVisibilityMutuallyExclusive();
  }

  updateGroupLayers();
}

void QgsLayerTreeGroup::addChildNode( QgsLayerTreeNode *node )
{
  insertChildNode( -1, node );

  updateGroupLayers();
}

void QgsLayerTreeGroup::removeChildNode( QgsLayerTreeNode *node )
{
  int i = mChildren.indexOf( node );
  if ( i >= 0 )
    removeChildren( i, 1 );

  updateGroupLayers();
}

void QgsLayerTreeGroup::removeLayer( QgsMapLayer *layer )
{
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      QgsLayerTreeLayer *childLayer = QgsLayerTree::toLayer( child );
      if ( childLayer->layer() == layer )
      {
        removeChildren( mChildren.indexOf( child ), 1 );
        break;
      }
    }
  }

  updateGroupLayers();
}

void QgsLayerTreeGroup::removeCustomNode( const QString &id )
{
  QgsLayerTreeCustomNode *node = findCustomNode( id );
  if ( node )
  {
    removeChildNode( node );
  }
}

void QgsLayerTreeGroup::removeChildren( int from, int count )
{
  QgsLayerTreeNode *meChild = nullptr;
  if ( mMutuallyExclusive && mMutuallyExclusiveChildIndex >= 0 && mMutuallyExclusiveChildIndex < mChildren.count() )
    meChild = mChildren.at( mMutuallyExclusiveChildIndex );

  removeChildrenPrivate( from, count );

  if ( meChild )
  {
    // the child could have change its index - or may have been removed completely
    mMutuallyExclusiveChildIndex = mChildren.indexOf( meChild );
    // we need to uncheck this group
    //if ( mMutuallyExclusiveChildIndex == -1 )
    //  setItemVisibilityChecked( false );
  }

  updateGroupLayers();
}

void QgsLayerTreeGroup::removeChildrenGroupWithoutLayers()
{
  // clean the layer tree by removing empty group
  const auto childNodes = children();
  for ( QgsLayerTreeNode *treeNode : childNodes )
  {
    if ( treeNode->nodeType() == QgsLayerTreeNode::NodeGroup )
    {
      QgsLayerTreeGroup *treeGroup = qobject_cast<QgsLayerTreeGroup *>( treeNode );
      if ( treeGroup->findLayerIds().isEmpty() )
        removeChildNode( treeNode );
      else
        treeGroup->removeChildrenGroupWithoutLayers();
    }
  }

  updateGroupLayers();
}

void QgsLayerTreeGroup::removeAllChildren()
{
  removeChildren( 0, mChildren.count() );
}

QgsLayerTreeLayer *QgsLayerTreeGroup::findLayer( QgsMapLayer *layer ) const
{
  if ( !layer )
    return nullptr;

  return findLayer( layer->id() );
}

QgsLayerTreeLayer *QgsLayerTreeGroup::findLayer( const QString &layerId ) const
{
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      QgsLayerTreeLayer *childLayer = QgsLayerTree::toLayer( child );
      if ( childLayer->layerId() == layerId )
        return childLayer;
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeLayer *res = QgsLayerTree::toGroup( child )->findLayer( layerId );
      if ( res )
        return res;
    }
  }
  return nullptr;
}

QList<QgsLayerTreeLayer *> QgsLayerTreeGroup::findLayers() const
{
  QList<QgsLayerTreeLayer *> list;
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    if ( QgsLayerTree::isLayer( child ) )
      list << QgsLayerTree::toLayer( child );
    else if ( QgsLayerTree::isGroup( child ) )
      list << QgsLayerTree::toGroup( child )->findLayers();
  }
  return list;
}

QgsLayerTreeCustomNode *QgsLayerTreeGroup::findCustomNode( const QString &id ) const
{
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    if ( QgsLayerTree::isCustomNode( child ) )
    {
      QgsLayerTreeCustomNode *childCustom = QgsLayerTree::toCustomNode( child );
      if ( childCustom->nodeId() == id )
        return childCustom;
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeCustomNode *res = QgsLayerTree::toGroup( child )->findCustomNode( id );
      if ( res )
        return res;
    }
  }
  return nullptr;
}

QList<QgsLayerTreeNode *> QgsLayerTreeGroup::findLayersAndCustomNodes() const
{
  QList<QgsLayerTreeNode *> list;
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    if ( QgsLayerTree::isLayer( child ) || QgsLayerTree::isCustomNode( child ) )
      list << child;
    else if ( QgsLayerTree::isGroup( child ) )
      list << QgsLayerTree::toGroup( child )->findLayersAndCustomNodes();
  }
  return list;
}

void QgsLayerTreeGroup::reorderGroupLayers( const QList<QgsMapLayer *> &order )
{
  const QList< QgsLayerTreeLayer * > childLayers = findLayers();
  int targetIndex = 0;
  for ( QgsMapLayer *targetLayer : order )
  {
    for ( QgsLayerTreeLayer *layerNode : childLayers )
    {
      if ( layerNode->layer() == targetLayer )
      {
        QgsLayerTreeLayer *cloned = layerNode->clone();
        insertChildNode( targetIndex, cloned );
        removeChildNode( layerNode );
        targetIndex++;
        break;
      }
    }
  }
}

void QgsLayerTreeGroup::reorderGroupLayersAndCustomNodes( const QList<QgsLayerTreeNode *> &order )
{
  const QList< QgsLayerTreeNode * > childNodes = findLayersAndCustomNodes();
  int targetIndex = 0;
  for ( QgsLayerTreeNode *targetNode : order )
  {
    for ( QgsLayerTreeNode *childNode : childNodes )
    {
      if ( childNode == targetNode )
      {
        QgsLayerTreeNode *cloned = childNode->clone();
        insertChildNode( targetIndex, cloned );
        removeChildNode( childNode );
        targetIndex++;
        break;
      }
    }
  }
}

QList<QgsMapLayer *> QgsLayerTreeGroup::layerOrderRespectingGroupLayers() const
{
  QList<QgsMapLayer *> list;
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      QgsMapLayer *layer = QgsLayerTree::toLayer( child )->layer();
      if ( !layer || !layer->isSpatial() )
        continue;
      list << layer;
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeGroup *group = QgsLayerTree::toGroup( child );
      if ( group->groupLayer() )
      {
        list << group->groupLayer();
      }
      else
      {
        list << group->layerOrderRespectingGroupLayers();
      }
    }
  }
  return list;
}

QList<QgsLayerTreeNode *> QgsLayerTreeGroup::layerAndCustomNodeOrderRespectingGroupLayers() const
{
  QList<QgsLayerTreeNode *> list;
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      QgsMapLayer *layer = QgsLayerTree::toLayer( child )->layer();
      if ( !layer || !layer->isSpatial() )
        continue;
      list << child;
    }
    else if ( QgsLayerTree::isCustomNode( child ) )
    {
      list << child;
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeGroup *group = QgsLayerTree::toGroup( child );
      if ( group->groupLayer() )
      {
        list << group;
      }
      else
      {
        list << group->layerAndCustomNodeOrderRespectingGroupLayers();
      }
    }
  }
  return list;
}

QgsLayerTreeGroup *QgsLayerTreeGroup::findGroup( const QString &name )
{
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeGroup *childGroup = QgsLayerTree::toGroup( child );
      if ( childGroup->name() == name )
        return childGroup;
      else
      {
        QgsLayerTreeGroup *grp = childGroup->findGroup( name );
        if ( grp )
          return grp;
      }
    }
  }
  return nullptr;
}

QList<QgsLayerTreeGroup *> QgsLayerTreeGroup::findGroups( bool recursive ) const
{
  QList<QgsLayerTreeGroup *> list;

  for ( QgsLayerTreeNode *child : mChildren )
  {
    if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeGroup *childGroup = QgsLayerTree::toGroup( child );
      list << childGroup;
      if ( recursive )
        list << childGroup->findGroups( recursive );
    }
  }
  return list;
}

QgsLayerTreeGroup *QgsLayerTreeGroup::readXml( const QDomElement &element, const QgsReadWriteContext &context ) // cppcheck-suppress duplInheritedMember
{
  if ( element.tagName() != "layer-tree-group"_L1 )
    return nullptr;

  QString name =  context.projectTranslator()->translate( u"project:layergroups"_s, element.attribute( u"name"_s ) );
  bool isExpanded = ( element.attribute( u"expanded"_s, u"1"_s ) == "1"_L1 );
  bool checked = QgsLayerTreeUtils::checkStateFromXml( element.attribute( u"checked"_s ) ) != Qt::Unchecked;
  bool isMutuallyExclusive = element.attribute( u"mutually-exclusive"_s, u"0"_s ) == "1"_L1;
  int mutuallyExclusiveChildIndex = element.attribute( u"mutually-exclusive-child"_s, u"-1"_s ).toInt();

  QgsLayerTreeGroup *groupNode = new QgsLayerTreeGroup( name, checked );
  groupNode->setExpanded( isExpanded );

  groupNode->readCommonXml( element );

  groupNode->readChildrenFromXml( element, context );

  groupNode->setIsMutuallyExclusive( isMutuallyExclusive, mutuallyExclusiveChildIndex );

  groupNode->mWmsHasTimeDimension = element.attribute( u"wms-has-time-dimension"_s, u"0"_s ) == "1"_L1;

  groupNode->mGroupLayer = QgsMapLayerRef( element.attribute( u"groupLayer"_s ) );

  readLegacyServerProperties( groupNode );

  groupNode->serverProperties()->readXml( element );

  return groupNode;
}

void QgsLayerTreeGroup::readLegacyServerProperties( QgsLayerTreeGroup *groupNode )
{
  const QVariant wmsShortName  = groupNode->customProperty( u"wmsShortName"_s );
  if ( wmsShortName.isValid() )
  {
    groupNode->serverProperties()->setShortName( wmsShortName.toString() );
    groupNode->removeCustomProperty( u"wmsShortName"_s );
  }

  const QVariant wmsTitle = groupNode->customProperty( u"wmsTitle"_s );
  if ( wmsTitle.isValid() )
  {
    groupNode->serverProperties()->setTitle( wmsTitle.toString() );
    groupNode->removeCustomProperty( u"wmsTitle"_s );
  }

  const QVariant wmsAbstract = groupNode->customProperty( u"wmsAbstract"_s );
  if ( wmsAbstract.isValid() )
  {
    groupNode->serverProperties()->setAbstract( wmsAbstract.toString() );
    groupNode->removeCustomProperty( u"wmsAbstract"_s );
  }
}

QgsLayerTreeGroup *QgsLayerTreeGroup::readXml( const QDomElement &element, const QgsProject *project, const QgsReadWriteContext &context )
{
  QgsLayerTreeGroup *node = readXml( element, context );
  if ( !node )
    return nullptr;

  node->resolveReferences( project );

  return node;
}

void QgsLayerTreeGroup::writeXml( QDomElement &parentElement, const QgsReadWriteContext &context )
{
  QDomDocument doc = parentElement.ownerDocument();
  QDomElement elem = doc.createElement( u"layer-tree-group"_s );
  elem.setAttribute( u"name"_s, mName );
  elem.setAttribute( u"expanded"_s, mExpanded ? u"1"_s : u"0"_s );
  elem.setAttribute( u"checked"_s, mChecked ? u"Qt::Checked"_s : u"Qt::Unchecked"_s );
  if ( mMutuallyExclusive )
  {
    elem.setAttribute( u"mutually-exclusive"_s, u"1"_s );
    elem.setAttribute( u"mutually-exclusive-child"_s, mMutuallyExclusiveChildIndex );
  }

  if ( mWmsHasTimeDimension )
  {
    elem.setAttribute( u"wms-has-time-dimension"_s, u"1"_s );
  }

  elem.setAttribute( u"groupLayer"_s, mGroupLayer.layerId );

  writeCommonXml( elem );

  serverProperties()->writeXml( elem, doc );

  for ( QgsLayerTreeNode *node : std::as_const( mChildren ) )
    node->writeXml( elem, context );

  parentElement.appendChild( elem );
}

void QgsLayerTreeGroup::readChildrenFromXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  QList<QgsLayerTreeNode *> nodes;
  QDomElement childElem = element.firstChildElement();
  while ( !childElem.isNull() )
  {
    QgsLayerTreeNode *newNode = QgsLayerTreeNode::readXml( childElem, context );
    if ( newNode )
      nodes << newNode;

    childElem = childElem.nextSiblingElement();
  }

  insertChildNodes( -1, nodes );
}

QString QgsLayerTreeGroup::dump() const
{
  QString header = u"GROUP: %1 checked=%2 expanded=%3\n"_s.arg( name() ).arg( mChecked ).arg( mExpanded );
  QStringList childrenDump;
  for ( QgsLayerTreeNode *node : std::as_const( mChildren ) )
    childrenDump << node->dump().split( '\n' );
  for ( int i = 0; i < childrenDump.count(); ++i )
    childrenDump[i].prepend( "  " );
  return header + childrenDump.join( QLatin1Char( '\n' ) );
}

QgsLayerTreeGroup *QgsLayerTreeGroup::clone() const
{
  return new QgsLayerTreeGroup( *this );
}

void QgsLayerTreeGroup::resolveReferences( const QgsProject *project, bool looseMatching )
{
  for ( QgsLayerTreeNode *node : std::as_const( mChildren ) )
    node->resolveReferences( project, looseMatching );

  mGroupLayer.resolve( project );
}

static bool _nodeIsChecked( QgsLayerTreeNode *node )
{
  return node->itemVisibilityChecked();
}


bool QgsLayerTreeGroup::isMutuallyExclusive() const
{
  return mMutuallyExclusive;
}

void QgsLayerTreeGroup::setIsMutuallyExclusive( bool enabled, int initialChildIndex )
{
  mMutuallyExclusive = enabled;
  mMutuallyExclusiveChildIndex = initialChildIndex;

  if ( !enabled )
  {
    return;
  }

  if ( mMutuallyExclusiveChildIndex < 0 || mMutuallyExclusiveChildIndex >= mChildren.count() )
  {
    // try to use first checked index
    int index = 0;
    for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
    {
      if ( _nodeIsChecked( child ) )
      {
        mMutuallyExclusiveChildIndex = index;
        break;
      }
      index++;
    }
  }

  updateChildVisibilityMutuallyExclusive();
}

QgsGroupLayer *QgsLayerTreeGroup::groupLayer()
{
  return qobject_cast< QgsGroupLayer * >( mGroupLayer.layer );
}

void QgsLayerTreeGroup::setGroupLayer( QgsGroupLayer *layer )
{
  if ( QgsGroupLayer *groupLayer = qobject_cast< QgsGroupLayer * >( mGroupLayer.get() ) )
  {
    if ( !layer )
    {
      groupLayer->prepareLayersForRemovalFromGroup();
    }
  }
  mGroupLayer.setLayer( layer );
  refreshParentGroupLayerMembers();
}

QgsGroupLayer *QgsLayerTreeGroup::convertToGroupLayer( const QgsGroupLayer::LayerOptions &options )
{
  if ( !mGroupLayer.layerId.isEmpty() )
    return nullptr;

  auto res = std::make_unique< QgsGroupLayer >( name(), options );

  mGroupLayer.setLayer( res.get() );
  updateGroupLayers();

  return res.release();
}

void QgsLayerTreeGroup::refreshParentGroupLayerMembers()
{
  QgsLayerTreeGroup *parentGroup = qobject_cast< QgsLayerTreeGroup * >( parent() );
  while ( parentGroup )
  {
    if ( QgsLayerTree *layerTree = qobject_cast< QgsLayerTree * >( parentGroup ) )
      layerTree->emit layerOrderChanged();

    parentGroup->updateGroupLayers();
    parentGroup = qobject_cast< QgsLayerTreeGroup * >( parentGroup->parent() );
  }
}

QStringList QgsLayerTreeGroup::findLayerIds() const
{
  QStringList lst;
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    if ( QgsLayerTree::isGroup( child ) )
      lst << QgsLayerTree::toGroup( child )->findLayerIds();
    else if ( QgsLayerTree::isLayer( child ) )
      lst << QgsLayerTree::toLayer( child )->layerId();
  }
  return lst;
}

QStringList QgsLayerTreeGroup::findCustomNodeIds() const
{
  QStringList lst;
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    if ( QgsLayerTree::isGroup( child ) )
      lst << QgsLayerTree::toGroup( child )->findCustomNodeIds();
    else if ( QgsLayerTree::isCustomNode( child ) )
      lst << QgsLayerTree::toCustomNode( child )->nodeId();
  }
  return lst;
}

void QgsLayerTreeGroup::nodeVisibilityChanged( QgsLayerTreeNode *node )
{
  int childIndex = mChildren.indexOf( node );
  if ( childIndex == -1 )
  {
    updateGroupLayers();
    return; // not a direct child - ignore
  }

  if ( mMutuallyExclusive )
  {
    if ( _nodeIsChecked( node ) )
      mMutuallyExclusiveChildIndex = childIndex;
    else if ( mMutuallyExclusiveChildIndex == childIndex )
      mMutuallyExclusiveChildIndex = -1;

    // we need to make sure there is only one child node checked
    updateChildVisibilityMutuallyExclusive();
  }

  updateGroupLayers();
}

void QgsLayerTreeGroup::updateChildVisibilityMutuallyExclusive()
{
  if ( mChildren.isEmpty() )
    return;

  mChangingChildVisibility = true; // guard against running again setVisible() triggered from children

  int index = 0;
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    child->setItemVisibilityChecked( index == mMutuallyExclusiveChildIndex );
    ++index;
  }

  mChangingChildVisibility = false;

  updateGroupLayers();
}

void QgsLayerTreeGroup::makeOrphan()
{
  QgsLayerTreeNode::makeOrphan();
  // Reconnect internal signals
  connect( this, &QgsLayerTreeNode::visibilityChanged, this, &QgsLayerTreeGroup::nodeVisibilityChanged );
}

void QgsLayerTreeGroup::updateGroupLayers()
{
  QgsGroupLayer *groupLayer = qobject_cast< QgsGroupLayer * >( mGroupLayer.get() );
  if ( !groupLayer )
    return;

  QList< QgsMapLayer * > layers;

  std::function< void( QgsLayerTreeGroup * ) > findGroupLayerChildren;
  findGroupLayerChildren = [&layers, &findGroupLayerChildren]( QgsLayerTreeGroup * group )
  {
    for ( auto it = group->mChildren.crbegin(); it != group->mChildren.crend(); ++it )
    {
      if ( QgsLayerTreeLayer *layerTreeLayer = qobject_cast< QgsLayerTreeLayer * >( *it ) )
      {
        if ( layerTreeLayer->layer() && layerTreeLayer->isVisible() )
          layers << layerTreeLayer->layer();
      }
      else if ( QgsLayerTreeGroup *childGroup = qobject_cast< QgsLayerTreeGroup * >( *it ) )
      {
        if ( childGroup->isVisible() )
        {
          if ( QgsGroupLayer *groupLayer = childGroup->groupLayer() )
            layers << groupLayer;
          else
            findGroupLayerChildren( childGroup );
        }
      }
    }
  };
  findGroupLayerChildren( this );

  groupLayer->setChildLayers( layers );
  refreshParentGroupLayerMembers();
}

void QgsLayerTreeGroup::setItemVisibilityCheckedRecursive( bool checked )
{
  QgsLayerTreeNode::setItemVisibilityChecked( checked );

  mChangingChildVisibility = true; // guard against running again setVisible() triggered from children

  int index = 0;
  for ( QgsLayerTreeNode *child : std::as_const( mChildren ) )
  {
    child->setItemVisibilityCheckedRecursive( checked && ( mMutuallyExclusiveChildIndex < 0 || index == mMutuallyExclusiveChildIndex ) );
    ++index;
  }

  mChangingChildVisibility = false;

  updateGroupLayers();
}

QgsMapLayerServerProperties *QgsLayerTreeGroup::serverProperties()
{
  return mServerProperties.get();
}

const QgsMapLayerServerProperties *QgsLayerTreeGroup::serverProperties() const
{
  return mServerProperties.get();
}

void QgsLayerTreeGroup::setHasWmsTimeDimension( const bool hasWmsTimeDimension )
{
  mWmsHasTimeDimension = hasWmsTimeDimension;
}

bool QgsLayerTreeGroup::hasWmsTimeDimension() const
{
  return mWmsHasTimeDimension;
}
