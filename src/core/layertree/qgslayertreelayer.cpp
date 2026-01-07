/***************************************************************************
  qgslayertreelayer.cpp
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

#include "qgslayertreelayer.h"

#include "qgslayertreeutils.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgssymbollayerutils.h"

#include "moc_qgslayertreelayer.cpp"

QgsLayerTreeLayer::QgsLayerTreeLayer( QgsMapLayer *layer )
  : QgsLayerTreeNode( NodeLayer, true )
  , mRef( layer )
  , mLayerName( layer->name() )
{
  attachToLayer();
}

QgsLayerTreeLayer::QgsLayerTreeLayer( const QString &layerId, const QString &name, const QString &source, const QString &provider )
  : QgsLayerTreeNode( NodeLayer, true )
  , mRef( layerId, name, source, provider )
  , mLayerName( name.isEmpty() ? u"(?)"_s : name )
{
}

QgsLayerTreeLayer::QgsLayerTreeLayer( const QgsLayerTreeLayer &other )
  : QgsLayerTreeNode( other )
  , mRef( other.mRef )
  , mLayerName( other.mLayerName )
  , mPatchShape( other.mPatchShape )
  , mPatchSize( other.mPatchSize )
  , mSplitBehavior( other.mSplitBehavior )
{
  attachToLayer();
}

void QgsLayerTreeLayer::resolveReferences( const QgsProject *project, bool looseMatching )
{
  if ( mRef )
    return;  // already assigned

  if ( !looseMatching )
  {
    mRef.resolve( project );
  }
  else
  {
    mRef.resolveWeakly( project );
  }

  if ( !mRef )
    return;

  attachToLayer();
  emit layerLoaded();
}

void QgsLayerTreeLayer::attachToLayer()
{
  if ( !mRef )
    return;

  connect( mRef.layer, &QgsMapLayer::nameChanged, this, &QgsLayerTreeLayer::layerNameChanged );
  connect( mRef.layer, &QgsMapLayer::willBeDeleted, this, &QgsLayerTreeLayer::layerWillBeDeleted );
}


QString QgsLayerTreeLayer::name() const
{
  return ( mRef && mUseLayerName ) ? mRef->name() : mLayerName;
}

void QgsLayerTreeLayer::setName( const QString &n )
{
  if ( mRef && mUseLayerName )
  {
    if ( mRef->name() == n )
      return;
    mRef->setName( n );
    // no need to emit signal: we will be notified from layer's nameChanged() signal
  }
  else
  {
    if ( mLayerName == n )
      return;
    mLayerName = n;
    emit nameChanged( this, n );
  }
}

QgsLayerTreeLayer *QgsLayerTreeLayer::readXml( QDomElement &element, const QgsReadWriteContext &context ) // cppcheck-suppress duplInheritedMember
{
  if ( element.tagName() != "layer-tree-layer"_L1 )
    return nullptr;

  const QString layerID = element.attribute( u"id"_s );
  const QString layerName = element.attribute( u"name"_s );

  const QString providerKey = element.attribute( u"providerKey"_s );
  const QString sourceRaw = element.attribute( u"source"_s );
  const QString source = providerKey.isEmpty() ? sourceRaw : QgsProviderRegistry::instance()->relativeToAbsoluteUri( providerKey, sourceRaw, context );

  const Qt::CheckState checked = QgsLayerTreeUtils::checkStateFromXml( element.attribute( u"checked"_s ) );
  const bool isExpanded = ( element.attribute( u"expanded"_s, u"1"_s ) == "1"_L1 );
  const QString labelExpression = element.attribute( u"legend_exp"_s );

  // needs to have the layer reference resolved later
  QgsLayerTreeLayer *nodeLayer = new QgsLayerTreeLayer( layerID, layerName, source, providerKey );

  nodeLayer->readCommonXml( element );

  nodeLayer->setItemVisibilityChecked( checked != Qt::Unchecked );
  nodeLayer->setExpanded( isExpanded );
  nodeLayer->setLabelExpression( labelExpression );

  const QDomElement patchElem = element.firstChildElement( u"patch"_s );
  if ( !patchElem.isNull() )
  {
    QgsLegendPatchShape patch;
    patch.readXml( patchElem, context );
    nodeLayer->setPatchShape( patch );
  }

  nodeLayer->setPatchSize( QgsSymbolLayerUtils::decodeSize( element.attribute( u"patch_size"_s ) ) );

  nodeLayer->setLegendSplitBehavior( static_cast< LegendNodesSplitBehavior >( element.attribute( u"legend_split_behavior"_s, u"0"_s ).toInt() ) );

  return nodeLayer;
}

QgsLayerTreeLayer *QgsLayerTreeLayer::readXml( QDomElement &element, const QgsProject *project, const QgsReadWriteContext &context )
{
  QgsLayerTreeLayer *node = readXml( element, context );
  if ( node )
    node->resolveReferences( project );
  return node;
}

void QgsLayerTreeLayer::writeXml( QDomElement &parentElement, const QgsReadWriteContext &context )
{
  QDomDocument doc = parentElement.ownerDocument();
  QDomElement elem = doc.createElement( u"layer-tree-layer"_s );
  elem.setAttribute( u"id"_s, layerId() );
  elem.setAttribute( u"name"_s, name() );

  if ( mRef )
  {
    const QString providerKey = mRef->dataProvider() ? mRef->dataProvider()->name() : QString();
    const QString source = providerKey.isEmpty() ? mRef->publicSource() : QgsProviderRegistry::instance()->absoluteToRelativeUri( providerKey, mRef->publicSource(), context );
    elem.setAttribute( u"source"_s, source );
    elem.setAttribute( u"providerKey"_s, providerKey );
  }

  elem.setAttribute( u"checked"_s, mChecked ? u"Qt::Checked"_s : u"Qt::Unchecked"_s );
  elem.setAttribute( u"expanded"_s, mExpanded ? "1" : "0" );
  elem.setAttribute( u"legend_exp"_s, mLabelExpression );

  if ( !mPatchShape.isNull() )
  {
    QDomElement patchElem = doc.createElement( u"patch"_s );
    mPatchShape.writeXml( patchElem, doc, context );
    elem.appendChild( patchElem );
  }
  elem.setAttribute( u"patch_size"_s, QgsSymbolLayerUtils::encodeSize( mPatchSize ) );

  elem.setAttribute( u"legend_split_behavior"_s, mSplitBehavior );

  writeCommonXml( elem );

  parentElement.appendChild( elem );
}

QString QgsLayerTreeLayer::dump() const
{
  return u"LAYER: %1 checked=%2 expanded=%3 id=%4\n"_s.arg( name() ).arg( mChecked ).arg( mExpanded ).arg( layerId() );
}

QgsLayerTreeLayer *QgsLayerTreeLayer::clone() const
{
  return new QgsLayerTreeLayer( *this );
}

void QgsLayerTreeLayer::layerWillBeDeleted()
{
  Q_ASSERT( mRef );

  emit layerWillBeUnloaded();

  mLayerName = mRef->name();
  // in theory we do not even need to do this - the weak ref should clear itself
  mRef.layer.clear();
  // layerId stays in the reference

}

void QgsLayerTreeLayer::setUseLayerName( const bool use )
{
  mUseLayerName = use;
}

bool QgsLayerTreeLayer::useLayerName() const
{
  return mUseLayerName;
}

void QgsLayerTreeLayer::layerNameChanged()
{
  Q_ASSERT( mRef );
  emit nameChanged( this, mRef->name() );
}

void QgsLayerTreeLayer::setLabelExpression( const QString &expression )
{
  mLabelExpression = expression;
}

QgsLegendPatchShape QgsLayerTreeLayer::patchShape() const
{
  return mPatchShape;
}

void QgsLayerTreeLayer::setPatchShape( const QgsLegendPatchShape &shape )
{
  mPatchShape = shape;
}

