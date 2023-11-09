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
#include "qgssymbollayerutils.h"


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
  , mLayerName( name.isEmpty() ? QStringLiteral( "(?)" ) : name )
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

QgsLayerTreeLayer *QgsLayerTreeLayer::readXml( QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.tagName() != QLatin1String( "layer-tree-layer" ) )
    return nullptr;

  const QString layerID = element.attribute( QStringLiteral( "id" ) );
  const QString layerName = element.attribute( QStringLiteral( "name" ) );

  const QString providerKey = element.attribute( QStringLiteral( "providerKey" ) );
  const QString source = context.pathResolver().readPath( element.attribute( QStringLiteral( "source" ) ) );

  const Qt::CheckState checked = QgsLayerTreeUtils::checkStateFromXml( element.attribute( QStringLiteral( "checked" ) ) );
  const bool isExpanded = ( element.attribute( QStringLiteral( "expanded" ), QStringLiteral( "1" ) ) == QLatin1String( "1" ) );
  const QString labelExpression = element.attribute( QStringLiteral( "legend_exp" ) );

  // needs to have the layer reference resolved later
  QgsLayerTreeLayer *nodeLayer = new QgsLayerTreeLayer( layerID, layerName, source, providerKey );

  nodeLayer->readCommonXml( element );

  nodeLayer->setItemVisibilityChecked( checked != Qt::Unchecked );
  nodeLayer->setExpanded( isExpanded );
  nodeLayer->setLabelExpression( labelExpression );

  const QDomElement patchElem = element.firstChildElement( QStringLiteral( "patch" ) );
  if ( !patchElem.isNull() )
  {
    QgsLegendPatchShape patch;
    patch.readXml( patchElem, context );
    nodeLayer->setPatchShape( patch );
  }

  nodeLayer->setPatchSize( QgsSymbolLayerUtils::decodeSize( element.attribute( QStringLiteral( "patch_size" ) ) ) );

  nodeLayer->setLegendSplitBehavior( static_cast< LegendNodesSplitBehavior >( element.attribute( QStringLiteral( "legend_split_behavior" ), QStringLiteral( "0" ) ).toInt() ) );

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
  QDomElement elem = doc.createElement( QStringLiteral( "layer-tree-layer" ) );
  elem.setAttribute( QStringLiteral( "id" ), layerId() );
  elem.setAttribute( QStringLiteral( "name" ), name() );

  if ( mRef )
  {
    elem.setAttribute( QStringLiteral( "source" ), context.pathResolver().writePath( mRef->publicSource() ) );
    elem.setAttribute( QStringLiteral( "providerKey" ), mRef->dataProvider() ? mRef->dataProvider()->name() : QString() );
  }

  elem.setAttribute( QStringLiteral( "checked" ), mChecked ? QStringLiteral( "Qt::Checked" ) : QStringLiteral( "Qt::Unchecked" ) );
  elem.setAttribute( QStringLiteral( "expanded" ), mExpanded ? "1" : "0" );
  elem.setAttribute( QStringLiteral( "legend_exp" ), mLabelExpression );

  if ( !mPatchShape.isNull() )
  {
    QDomElement patchElem = doc.createElement( QStringLiteral( "patch" ) );
    mPatchShape.writeXml( patchElem, doc, context );
    elem.appendChild( patchElem );
  }
  elem.setAttribute( QStringLiteral( "patch_size" ), QgsSymbolLayerUtils::encodeSize( mPatchSize ) );

  elem.setAttribute( QStringLiteral( "legend_split_behavior" ), mSplitBehavior );

  writeCommonXml( elem );

  parentElement.appendChild( elem );
}

QString QgsLayerTreeLayer::dump() const
{
  return QStringLiteral( "LAYER: %1 checked=%2 expanded=%3 id=%4\n" ).arg( name() ).arg( mChecked ).arg( mExpanded ).arg( layerId() );
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

