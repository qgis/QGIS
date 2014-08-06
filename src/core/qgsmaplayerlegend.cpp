/***************************************************************************
  qgsmaplayerlegend.cpp
  --------------------------------------
  Date                 : July 2014
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

#include "qgsmaplayerlegend.h"

#include "qgslayertree.h"
#include "qgspluginlayer.h"
#include "qgsrasterlayer.h"
#include "qgsrendererv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorlayer.h"


QgsMapLayerLegend::QgsMapLayerLegend( QObject *parent ) :
    QObject( parent )
{
}

QgsMapLayerLegend* QgsMapLayerLegend::defaultVectorLegend( QgsVectorLayer* vl )
{
  return new QgsDefaultVectorLayerLegend( vl );
}

QgsMapLayerLegend* QgsMapLayerLegend::defaultRasterLegend( QgsRasterLayer* rl )
{
  return new QgsDefaultRasterLayerLegend( rl );
}

QgsMapLayerLegend* QgsMapLayerLegend::defaultPluginLegend( QgsPluginLayer* pl )
{
  return new QgsDefaultPluginLayerLegend( pl );
}


// -------------------------------------------------------------------------


QgsLayerTreeModelLegendNode::QgsLayerTreeModelLegendNode( QgsLayerTreeLayer* nodeL )
    : mParent( nodeL )
{
}

Qt::ItemFlags QgsLayerTreeModelLegendNode::flags() const
{
  return Qt::ItemIsEnabled;
}

bool QgsLayerTreeModelLegendNode::setData( const QVariant& value, int role )
{
  Q_UNUSED( value );
  Q_UNUSED( role );
  return false;
}


// -------------------------------------------------------------------------


QgsSymbolV2LegendNode::QgsSymbolV2LegendNode( QgsLayerTreeLayer* nodeLayer, QgsSymbolV2* symbol, const QString& label, const QString& ruleKey )
    : QgsLayerTreeModelLegendNode( nodeLayer )
    , mSymbol( symbol ? symbol->clone() : 0 )
    , mLabel( label )
    , mRuleKey( ruleKey )
{
  if ( nodeLayer->customProperty( "showFeatureCount", 0 ).toBool() && symbol )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( nodeLayer->layer() );
    if ( vl )
      mLabel += QString( " [%1]" ).arg( vl->featureCount( symbol ) );
  }
}

QgsSymbolV2LegendNode::~QgsSymbolV2LegendNode()
{
  delete mSymbol;
}

Qt::ItemFlags QgsSymbolV2LegendNode::flags() const
{
  if ( mParent && mParent->childrenCheckable() )
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  else
    return Qt::ItemIsEnabled;
}


QVariant QgsSymbolV2LegendNode::data( int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    return mLabel;
  }
  else if ( role == Qt::DecorationRole )
  {
    QSize iconSize( 16, 16 ); // TODO: configurable
    if ( mIcon.isNull() && mSymbol )
      mIcon = QgsSymbolLayerV2Utils::symbolPreviewPixmap( mSymbol, iconSize );
    return mIcon;
  }
  else if ( role == Qt::CheckStateRole )
  {
    if ( !mParent || !mParent->childrenCheckable() )
      return QVariant();

    if ( !mParent->isVisible() )
      return Qt::PartiallyChecked;

    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( mParent->layer() );
    if ( !vlayer || !vlayer->rendererV2() )
      return QVariant();

    return vlayer->rendererV2()->legendSymbolItemChecked( mRuleKey ) ? Qt::Checked : Qt::Unchecked;
  }

  return QVariant();
}

bool QgsSymbolV2LegendNode::setData( const QVariant& value, int role )
{
  if ( role != Qt::CheckStateRole )
    return false;

  if ( !mParent || !mParent->childrenCheckable() )
    return false;

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( mParent->layer() );
  if ( !vlayer || !vlayer->rendererV2() )
    return false;

  vlayer->rendererV2()->checkLegendSymbolItem( mRuleKey, value == Qt::Checked );

  if ( mParent->isVisible() )
    vlayer->clearCacheImage();

  return true;
}


// -------------------------------------------------------------------------


QgsSimpleLegendNode::QgsSimpleLegendNode( QgsLayerTreeLayer* nodeLayer, const QString& label, const QIcon& icon )
    : QgsLayerTreeModelLegendNode( nodeLayer )
    , mLabel( label )
    , mIcon( icon )
{
}

QVariant QgsSimpleLegendNode::data( int role ) const
{
  if ( role == Qt::DisplayRole )
    return mLabel;
  else if ( role == Qt::DecorationRole )
    return mIcon;
  else
    return QVariant();
}

// -------------------------------------------------------------------------


QgsDefaultVectorLayerLegend::QgsDefaultVectorLayerLegend( QgsVectorLayer* vl )
    : mLayer( vl )
{
  connect( mLayer, SIGNAL( rendererChanged() ), this, SIGNAL( itemsChanged() ) );
}

QList<QgsLayerTreeModelLegendNode*> QgsDefaultVectorLayerLegend::createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode*> nodes;

  QgsFeatureRendererV2* r = mLayer->rendererV2();
  if ( !r )
    return nodes;

  if ( nodeLayer->customProperty( "showFeatureCount", 0 ).toBool() )
    mLayer->countSymbolFeatures();

  nodeLayer->setChildrenCheckable( r->legendSymbolItemsCheckable() );

  foreach ( const QgsLegendSymbolItemV2& i, r->legendSymbolItemsV2() )
  {
    nodes.append( new QgsSymbolV2LegendNode( nodeLayer, i.symbol, i.label, i.key ) );
  }
  return nodes;
}

#include "qgscomposerlegenditem.h"

void QgsDefaultVectorLayerLegend::createLegendModelItems( QgsComposerLayerItem* layerItem )
{
  if ( !layerItem || !mLayer )
    return;

  QgsFeatureRendererV2* renderer = mLayer->rendererV2();
  if ( !renderer )
    return;

  if ( layerItem->showFeatureCount() )
  {
    if ( !mLayer->countSymbolFeatures() )
    {
      QgsDebugMsg( "Cannot get feature counts" );
    }
  }

  // Remember old user texts
  QHash<QString, QString> oldUserTexts;
  for ( int i = 0; i < layerItem->rowCount(); ++i )
  {
    QgsComposerSymbolV2Item* oldSymbolItem = dynamic_cast<QgsComposerSymbolV2Item*>( layerItem->child( i, 0 ) );
    if ( oldSymbolItem && !oldSymbolItem->userText().isEmpty() )
      oldUserTexts.insert( oldSymbolItem->ruleKey(), oldSymbolItem->userText() );
  }

  int row = 0;
  foreach ( const QgsLegendSymbolItemV2& item, renderer->legendSymbolItemsV2() )
  {
    QgsComposerSymbolV2Item* currentSymbolItem = new QgsComposerSymbolV2Item( item );
    layerItem->setChild( row++, 0, currentSymbolItem );
  }

  // Restore previously used user texts
  for ( QHash<QString, QString>::const_iterator it = oldUserTexts.begin(); it != oldUserTexts.end(); ++it )
  {
    QgsComposerSymbolV2Item* item = QgsComposerSymbolV2Item::findItemByRuleKey( layerItem, it.key() );
    if ( item )
      item->setUserText( it.value() );
  }

  // Delete following old items (if current number of items decreased)
  for ( int i = layerItem->rowCount() - 1; i >= row; --i )
  {
    layerItem->removeRow( i );
  }

}


// -------------------------------------------------------------------------


QgsDefaultRasterLayerLegend::QgsDefaultRasterLayerLegend( QgsRasterLayer* rl )
    : mLayer( rl )
{
  connect( mLayer, SIGNAL( rendererChanged() ), this, SIGNAL( itemsChanged() ) );
}

QList<QgsLayerTreeModelLegendNode*> QgsDefaultRasterLayerLegend::createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode*> nodes;

  // temporary solution for WMS. Ideally should be done with a delegate.
  if ( mLayer->providerType() == "wms" )
  {
    QImage img = mLayer->dataProvider()->getLegendGraphic( 1000 ); // dummy scale - should not be required!
    if ( !img.isNull() )
      nodes << new QgsSimpleLegendNode( nodeLayer, tr( "Double-click to view legend" ) );
  }

  QgsLegendColorList rasterItemList = mLayer->legendSymbologyItems();
  if ( rasterItemList.count() == 0 )
    return nodes;

  // Paletted raster may have many colors, for example UInt16 may have 65536 colors
  // and it is very slow, so we limit max count
  QSize iconSize( 16, 16 );
  int count = 0;
  int max_count = 1000;

  for ( QgsLegendColorList::const_iterator itemIt = rasterItemList.constBegin();
        itemIt != rasterItemList.constEnd(); ++itemIt, ++count )
  {
    QPixmap pix( iconSize );
    pix.fill( itemIt->second );
    nodes << new QgsSimpleLegendNode( nodeLayer, itemIt->first, QIcon( pix ) );

    if ( count == max_count )
    {
      pix.fill( Qt::transparent );
      QString label = tr( "following %1 items\nnot displayed" ).arg( rasterItemList.size() - max_count );
      nodes << new QgsSimpleLegendNode( nodeLayer, label, QIcon( pix ) );
      break;
    }
  }

  return nodes;
}

void QgsDefaultRasterLayerLegend::createLegendModelItems( QgsComposerLayerItem* layerItem )
{
  if ( !layerItem || !mLayer )
    return;

  QgsDebugMsg( QString( "layer providertype:: %1" ).arg( mLayer->providerType() ) );
  if ( mLayer->providerType() == "wms" )
  {
    // GetLegendGraphics in case of WMS service... image can return null if GetLegendGraphics
    // is not supported by the server
    // double currentScale = legend()->canvas()->scale();
    // BEWARE getLegendGraphic() COULD BE USED WITHOUT SCALE PARAMETER IF IT WAS ALREADY CALLED WITH
    // THIS PARAMETER FROM A COMPONENT THAT CAN RECOVER CURRENT SCALE => LEGEND IN THE DESKTOP
    // OTHERWISE IT RETURN A INVALID PIXMAP (QPixmap().isNull() == False)
    QImage legendGraphic = mLayer->dataProvider()->getLegendGraphic();
    if ( !legendGraphic.isNull() )
    {
      QgsDebugMsg( QString( "downloaded legend with dimension width:" ) + QString::number( legendGraphic.width() ) + QString( " and Height:" ) + QString::number( legendGraphic.height() ) );
    }

    QgsComposerRasterImageItem* currentSymbolItem = new QgsComposerRasterImageItem( legendGraphic );

    layerItem->removeRows( 0, layerItem->rowCount() );
    layerItem->setChild( layerItem->rowCount(), 0, currentSymbolItem );
  }
  else
  {
    QList< QPair< QString, QColor > > rasterItemList = mLayer->legendSymbologyItems();
    QList< QPair< QString, QColor > >::const_iterator itemIt = rasterItemList.constBegin();
    int row = 0;
    for ( ; itemIt != rasterItemList.constEnd(); ++itemIt )
    {
      //determine raster layer opacity, and adjust item color opacity to match
      QColor itemColor = itemIt->second;
      QgsRasterRenderer* rasterRenderer = mLayer->renderer();
      itemColor.setAlpha( rasterRenderer ? rasterRenderer->opacity() * 255.0 : 255 );

      QgsComposerRasterSymbolItem* currentSymbolItem = new QgsComposerRasterSymbolItem( itemColor, itemIt->first );

      QgsComposerRasterSymbolItem* oldSymbolItem = dynamic_cast<QgsComposerRasterSymbolItem*>( layerItem->child( row, 0 ) );
      if ( oldSymbolItem )
      {
        currentSymbolItem->setUserText( oldSymbolItem->userText() );
      }

      layerItem->setChild( row++, 0, currentSymbolItem );
    }

    // Delete following old items (if current number of items decreased)
    for ( int i = layerItem->rowCount() - 1; i >= row; --i )
    {
      layerItem->removeRow( i );
    }
  }

}


// -------------------------------------------------------------------------


QgsDefaultPluginLayerLegend::QgsDefaultPluginLayerLegend( QgsPluginLayer* pl )
    : mLayer( pl )
{
}

QList<QgsLayerTreeModelLegendNode*> QgsDefaultPluginLayerLegend::createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode*> nodes;

  QSize iconSize( 16, 16 );
  QgsLegendSymbologyList symbologyList = mLayer->legendSymbologyItems( iconSize );

  if ( symbologyList.count() == 0 )
    return nodes;

  typedef QPair<QString, QPixmap> XY;
  foreach ( XY item, symbologyList )
  {
    nodes << new QgsSimpleLegendNode( nodeLayer, item.first, QIcon( item.second ) );
  }

  return nodes;
}
