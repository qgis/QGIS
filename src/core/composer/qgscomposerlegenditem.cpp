/***************************************************************************
                         qgscomposerlegenditem.cpp  -  description
                         -------------------------
    begin                : May 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerlegendstyle.h"
#include "qgscomposerlegenditem.h"
#include "qgscomposerlegend.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include <QDomDocument>
#include <QDomElement>

QgsComposerLegendItem::QgsComposerLegendItem( QgsComposerLegendStyle::Style s ): QStandardItem()
    , mStyle( s )
{
}

QgsComposerLegendItem::QgsComposerLegendItem( const QString& text, QgsComposerLegendStyle::Style s ): QStandardItem( text )
    , mStyle( s )
{
}

QgsComposerLegendItem::QgsComposerLegendItem( const QIcon& icon, const QString& text, QgsComposerLegendStyle::Style s ): QStandardItem( icon, text )
    , mStyle( s )
{
}

QgsComposerLegendItem::~QgsComposerLegendItem()
{
}

void QgsComposerLegendItem::writeXMLChildren( QDomElement& elem, QDomDocument& doc ) const
{
  int numRows = rowCount();
  QgsComposerLegendItem* currentItem = 0;
  for ( int i = 0; i < numRows; ++i )
  {
    currentItem = dynamic_cast<QgsComposerLegendItem*>( child( i, 0 ) );
    if ( currentItem )
    {
      currentItem->writeXML( elem, doc );
    }
  }
}

////////////////QgsComposerBaseSymbolItem

QgsComposerBaseSymbolItem::QgsComposerBaseSymbolItem()
    : QgsComposerLegendItem( QgsComposerLegendStyle::Symbol )
{

}


QgsComposerLayerItem* QgsComposerBaseSymbolItem::parentLayerItem() const
{
  return dynamic_cast<QgsComposerLayerItem*>( parent() );
}

QgsMapLayer* QgsComposerBaseSymbolItem::parentMapLayer() const
{
  QgsComposerLayerItem* lItem = parentLayerItem();
  if ( !lItem ) return 0;

  return QgsMapLayerRegistry::instance()->mapLayer( lItem->layerID() );

}

////////////////QgsComposerSymbolV2Item

#include "qgssymbolv2.h"

QgsComposerSymbolV2Item::QgsComposerSymbolV2Item()
{
}

QgsComposerSymbolV2Item::QgsComposerSymbolV2Item( const QgsLegendSymbolItemV2& item )
    : mItem( item )
{
  setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
}

QgsComposerSymbolV2Item::QgsComposerSymbolV2Item( const QString& text )
{
  setText( text );
}

QgsComposerSymbolV2Item::QgsComposerSymbolV2Item( const QIcon& icon, const QString& text )
{
  setIcon( icon );
  setText( text );
}

QgsComposerSymbolV2Item::~QgsComposerSymbolV2Item()
{
}



QVariant QgsComposerSymbolV2Item::data( int role ) const
{
  if ( role == Qt::DecorationRole )
  {
    if ( mIcon.isNull() )
      mIcon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mItem.symbol(), QSize( 30, 30 ) );
    return mIcon;
  }
  else if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    QString lbl = label();

    if ( parentLayerItem()->showFeatureCount() )
    {
      // Add counts to multi symbols layers only or labeled single symbols,
      // so that single symbol layers are still drawn on single line
      if ( parentLayerItem()->rowCount() > 1 || !lbl.isEmpty() )
      {
        lbl += QString( " [%1]" ).arg( parentVectorLayer()->featureCount( mItem.legacyRuleKey() ) );
      }
    }
    return lbl;
  }
  else
    return QgsComposerBaseSymbolItem::data( role );
}

QStandardItem* QgsComposerSymbolV2Item::clone() const
{
  return new QgsComposerSymbolV2Item( *this );
}

void QgsComposerSymbolV2Item::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  QDomElement vectorClassElem = doc.createElement( "VectorClassificationItemNg" );
  if ( mItem.symbol() )
  {
    QgsSymbolV2Map saveSymbolMap;
    saveSymbolMap.insert( "classificationSymbol", mItem.symbol() );
    QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( saveSymbolMap, "symbols", doc );
    vectorClassElem.appendChild( symbolsElem );
  }
  vectorClassElem.setAttribute( "text", mItem.label() );
  vectorClassElem.setAttribute( "userText", userText() );
  elem.appendChild( vectorClassElem );
}

void QgsComposerSymbolV2Item::readXML( const QDomElement& itemElem, bool xServerAvailable )
{
  Q_UNUSED( xServerAvailable );

  if ( itemElem.isNull() )
  {
    return;
  }

  setUserText( itemElem.attribute( "userText", "" ) );
  QDomElement symbolsElem = itemElem.firstChildElement( "symbols" );
}

void QgsComposerSymbolV2Item::setSymbolV2( QgsSymbolV2* s )
{
  Q_UNUSED( s );
}


QgsComposerSymbolV2Item* QgsComposerSymbolV2Item::findItemByRuleKey( QgsComposerLayerItem* parentLayerItem, QString ruleKey )
{
  for ( int i = 0; i < parentLayerItem->rowCount(); ++i )
  {
    if ( QgsComposerSymbolV2Item* sItem = dynamic_cast<QgsComposerSymbolV2Item*>( parentLayerItem->child( 0 ) ) )
    {
      if ( sItem->ruleKey() == ruleKey )
        return sItem;
    }
  }
  return 0;
}


QgsVectorLayer* QgsComposerSymbolV2Item::parentVectorLayer() const
{
  return qobject_cast<QgsVectorLayer*>( parentMapLayer() );
}

QString QgsComposerSymbolV2Item::label() const
{
  if ( !mUserText.isEmpty() )
  {
    return mUserText;
  }
  else
  {
    QgsVectorLayer* vLayer = parentVectorLayer();

    if ( vLayer && vLayer->rendererV2() && vLayer->rendererV2()->type() == "singleSymbol" )
    {
      if ( !parentLayerItem()->userText().isEmpty() )
      {
        return parentLayerItem()->userText();
      }
      else if ( !vLayer->title().isEmpty() )
      {
        return vLayer->title();
      }
      else
      {
        return vLayer->name();
      }
    }
    else
    {
      return mItem.label();
    }
  }
}


////////////////////QgsComposerRasterSymbolItem

QgsComposerRasterSymbolItem::QgsComposerRasterSymbolItem()
{
}

QgsComposerRasterSymbolItem::QgsComposerRasterSymbolItem( const QColor& color, const QString& label )
    : mColor( color )
    , mLabel( label )
{
}

QgsComposerRasterSymbolItem::QgsComposerRasterSymbolItem( const QString& text )
{
  setText( text );
}

QgsComposerRasterSymbolItem::QgsComposerRasterSymbolItem( const QIcon& icon, const QString& text )
{
  setIcon( icon );
  setText( text );
}

QgsComposerRasterSymbolItem::~QgsComposerRasterSymbolItem()
{
}

QVariant QgsComposerRasterSymbolItem::data( int role ) const
{
  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    return mUserText.isEmpty() ? mLabel : mUserText;
  }
  else if ( role == Qt::DecorationRole )
  {
    QPixmap itemPixmap( 20, 20 );
    itemPixmap.fill( mColor );
    return QIcon( itemPixmap );
  }
  return QgsComposerBaseSymbolItem::data( role );
}

QStandardItem* QgsComposerRasterSymbolItem::clone() const
{
  QgsComposerRasterSymbolItem* cloneItem  = new QgsComposerRasterSymbolItem();
  *cloneItem = *this;
  return cloneItem;
}

void QgsComposerRasterSymbolItem::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  QDomElement rasterClassElem = doc.createElement( "RasterClassificationItem" );
  rasterClassElem.setAttribute( "text", mLabel );
  rasterClassElem.setAttribute( "userText", userText() );
  rasterClassElem.setAttribute( "color", mColor.name() );
  elem.appendChild( rasterClassElem );
}

void QgsComposerRasterSymbolItem::readXML( const QDomElement& itemElem, bool xServerAvailable )
{
  Q_UNUSED( xServerAvailable );

  if ( itemElem.isNull() )
  {
    return;
  }
  mLabel = itemElem.attribute( "text", "" );
  setUserText( itemElem.attribute( "userText", "" ) );
  setColor( QColor( itemElem.attribute( "color" ) ) );
}


////////////////////QgsComposerRasterImageItem


QgsComposerRasterImageItem::QgsComposerRasterImageItem()
{
}

QgsComposerRasterImageItem::QgsComposerRasterImageItem( const QImage& image )
    : mImage( image )
{

}

QStandardItem* QgsComposerRasterImageItem::clone() const
{
  QgsComposerRasterImageItem* cloneItem  = new QgsComposerRasterImageItem( mImage );
  return cloneItem;
}

void QgsComposerRasterImageItem::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  QDomElement rasterImageElem = doc.createElement( "RasterImageItem" );
  // TODO: also save the image???
  elem.appendChild( rasterImageElem );
}

void QgsComposerRasterImageItem::readXML( const QDomElement& itemElem, bool xServerAvailable )
{
  Q_UNUSED( itemElem );
  Q_UNUSED( xServerAvailable );
}



////////////////////QgsComposerLayerItem

QgsComposerLayerItem::QgsComposerLayerItem(): QgsComposerLegendItem( QgsComposerLegendStyle::Subgroup )
    , mShowFeatureCount( false )
{
}

QgsComposerLayerItem::QgsComposerLayerItem( const QString& text ): QgsComposerLegendItem( text, QgsComposerLegendStyle::Subgroup )
    , mShowFeatureCount( false )
{
}

QgsComposerLayerItem::~QgsComposerLayerItem()
{
}

QVariant QgsComposerLayerItem::data( int role ) const
{
  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    QgsMapLayer* ml = mapLayer();
    if ( !ml ) return QVariant();

    QString label = mUserText.isEmpty() ? ml->name() : mUserText;

    if ( QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer*>( ml ) )
    {
      if ( showFeatureCount() )
      {
        label += QString( " [%1]" ).arg( vLayer->featureCount() );
      }
    }
    return label;
  }
  else
    return QgsComposerLegendItem::data( role );
}

QStandardItem* QgsComposerLayerItem::clone() const
{
  QgsComposerLayerItem* cloneItem  = new QgsComposerLayerItem();
  *cloneItem = *this;
  cloneItem->setLayerID( mLayerID );
  return cloneItem;
}

void QgsComposerLayerItem::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  QDomElement layerItemElem = doc.createElement( "LayerItem" );
  layerItemElem.setAttribute( "layerId", mLayerID );
  layerItemElem.setAttribute( "text", data( Qt::DisplayRole ).toString() );
  layerItemElem.setAttribute( "userText", userText() );
  layerItemElem.setAttribute( "showFeatureCount", showFeatureCount() );
  layerItemElem.setAttribute( "style", QgsComposerLegendStyle::styleName( mStyle ) );
  writeXMLChildren( layerItemElem, doc );
  elem.appendChild( layerItemElem );
}

void QgsComposerLayerItem::readXML( const QDomElement& itemElem, bool xServerAvailable )
{
  if ( itemElem.isNull() )
  {
    return;
  }
  setText( itemElem.attribute( "text", "" ) );
  setUserText( itemElem.attribute( "userText", "" ) );
  setLayerID( itemElem.attribute( "layerId", "" ) );
  setShowFeatureCount( itemElem.attribute( "showFeatureCount", "" ) == "1" ? true : false );
  setStyle( QgsComposerLegendStyle::styleFromName( itemElem.attribute( "style", "subgroup" ) ) );

  //now call readXML for all the child items
  QDomNodeList childList = itemElem.childNodes();
  QDomNode currentNode;
  QDomElement currentElem;
  QgsComposerLegendItem* currentChildItem = 0;

  int nChildItems = childList.count();
  for ( int i = 0; i < nChildItems; ++i )
  {
    currentNode = childList.at( i );
    if ( !currentNode.isElement() )
    {
      continue;
    }

    currentElem = currentNode.toElement();
    QString elemTag = currentElem.tagName();
    if ( elemTag == "VectorClassificationItem" )
    {
      continue; // legacy - unsupported
    }
    else if ( elemTag == "VectorClassificationItemNg" )
    {
      currentChildItem = new QgsComposerSymbolV2Item();
    }
    else if ( elemTag == "RasterClassificationItem" )
    {
      currentChildItem = new QgsComposerRasterSymbolItem();
    }
    else if ( elemTag == "RasterImageItem" )
    {
      currentChildItem = new QgsComposerRasterImageItem();
    }
    else
    {
      continue; //unsupported child type
    }
    currentChildItem->readXML( currentElem, xServerAvailable );
    appendRow( currentChildItem );
  }
}

void QgsComposerLayerItem::setDefaultStyle( double scaleDenominator, QString rule )
{
  Q_UNUSED( scaleDenominator );
  Q_UNUSED( rule );
}



QgsMapLayer* QgsComposerLayerItem::mapLayer() const
{
  return QgsMapLayerRegistry::instance()->mapLayer( mLayerID );
}

////////////////////QgsComposerGroupItem

QgsComposerGroupItem::QgsComposerGroupItem(): QgsComposerLegendItem( QgsComposerLegendStyle::Group )
{
}

QgsComposerGroupItem::QgsComposerGroupItem( const QString& text ): QgsComposerLegendItem( text, QgsComposerLegendStyle::Group )
{
}

QgsComposerGroupItem::~QgsComposerGroupItem()
{
}

QStandardItem* QgsComposerGroupItem::clone() const
{
  QgsComposerGroupItem* cloneItem = new QgsComposerGroupItem();
  *cloneItem = *this;
  return cloneItem;
}

void QgsComposerGroupItem::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  QDomElement layerGroupElem = doc.createElement( "GroupItem" );
  // text is always user text, but for forward compatibility for now write both
  layerGroupElem.setAttribute( "text", data( Qt::DisplayRole ).toString() );
  layerGroupElem.setAttribute( "userText", userText() );
  layerGroupElem.setAttribute( "style", QgsComposerLegendStyle::styleName( mStyle ) );
  writeXMLChildren( layerGroupElem, doc );
  elem.appendChild( layerGroupElem );
}

void QgsComposerGroupItem::readXML( const QDomElement& itemElem, bool xServerAvailable )
{
  if ( itemElem.isNull() )
  {
    return;
  }
  // text is always user text but for backward compatibility we read also text
  QString userText = itemElem.attribute( "userText", "" );
  if ( userText.isEmpty() )
  {
    userText = itemElem.attribute( "text", "" );
  }
  setText( userText );
  setUserText( userText );

  setStyle( QgsComposerLegendStyle::styleFromName( itemElem.attribute( "style", "group" ) ) );

  //now call readXML for all the child items
  QDomNodeList childList = itemElem.childNodes();
  QDomNode currentNode;
  QDomElement currentElem;
  QgsComposerLegendItem* currentChildItem = 0;

  int nChildItems = childList.count();
  for ( int i = 0; i < nChildItems; ++i )
  {
    currentNode = childList.at( i );
    if ( !currentNode.isElement() )
    {
      continue;
    }

    currentElem = currentNode.toElement();
    QString elemTag = currentElem.tagName();

    if ( elemTag == "GroupItem" )
    {
      currentChildItem = new QgsComposerGroupItem();
    }
    else if ( elemTag == "LayerItem" )
    {
      currentChildItem = new QgsComposerLayerItem();
    }
    else
    {
      continue; //unsupported child item type
    }
    currentChildItem->readXML( currentElem, xServerAvailable );

    QList<QStandardItem *> itemsList;
    itemsList << currentChildItem << new QgsComposerStyleItem( currentChildItem );
    appendRow( itemsList );
  }
}



QgsComposerStyleItem::QgsComposerStyleItem(): QStandardItem()
{
}

QgsComposerStyleItem::QgsComposerStyleItem( QgsComposerLegendItem *item ): QStandardItem()
{
  setData( QgsComposerLegendStyle::styleLabel( item->style() ) , Qt::DisplayRole );
}

QgsComposerStyleItem::~QgsComposerStyleItem()
{
}
