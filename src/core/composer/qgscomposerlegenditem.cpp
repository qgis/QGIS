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

#include "qgscomposerlegenditem.h"
#include "qgssymbol.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include <QDomDocument>
#include <QDomElement>

QgsComposerLegendItem::QgsComposerLegendItem(): QStandardItem()
{
}

QgsComposerLegendItem::QgsComposerLegendItem( const QString& text ): QStandardItem( text )
{
}

QgsComposerLegendItem::QgsComposerLegendItem( const QIcon& icon, const QString& text ): QStandardItem( icon, text )
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

//////////////////////////////QgsComposerSymbolItem

QgsComposerSymbolItem::QgsComposerSymbolItem(): QgsComposerLegendItem(), mSymbol( 0 )
{
}

QgsComposerSymbolItem::QgsComposerSymbolItem( const QString& text ): QgsComposerLegendItem( text ), mSymbol( 0 )
{
}

QgsComposerSymbolItem::QgsComposerSymbolItem( const QIcon& icon, const QString& text ): QgsComposerLegendItem( icon, text ), mSymbol( 0 )
{
}

QgsComposerSymbolItem::~QgsComposerSymbolItem()
{
  delete mSymbol;
}

void QgsComposerSymbolItem::setSymbol( QgsSymbol* s )
{
  delete mSymbol;
  mSymbol = s;
}

QStandardItem* QgsComposerSymbolItem::clone() const
{
  qWarning( "QgsComposerSymbolItem::clone" );
  QgsComposerSymbolItem* cloneItem = new QgsComposerSymbolItem();
  *cloneItem = *this;
  if ( mSymbol )
  {
    cloneItem->setSymbol( new QgsSymbol( *mSymbol ) );
  }
  return cloneItem;
}

void QgsComposerSymbolItem::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  QDomElement vectorClassElem = doc.createElement( "VectorClassificationItem" );
  if ( mSymbol )
  {
    mSymbol->writeXML( vectorClassElem, doc, 0 );
  }
  vectorClassElem.setAttribute( "text", text() );
  elem.appendChild( vectorClassElem );
}

void QgsComposerSymbolItem::readXML( const QDomElement& itemElem )
{
  //soon...
}

////////////////QgsComposerSymbolV2Item

#include "qgssymbolv2.h"

QgsComposerSymbolV2Item::QgsComposerSymbolV2Item(): QgsComposerLegendItem(), mSymbolV2( 0 )
{
}

QgsComposerSymbolV2Item::QgsComposerSymbolV2Item( const QString& text ): QgsComposerLegendItem( text ), mSymbolV2( 0 )
{
}

QgsComposerSymbolV2Item::QgsComposerSymbolV2Item( const QIcon& icon, const QString& text ): QgsComposerLegendItem( icon, text ), mSymbolV2( 0 )
{
}

QgsComposerSymbolV2Item::~QgsComposerSymbolV2Item()
{
  delete mSymbolV2;
}

QStandardItem* QgsComposerSymbolV2Item::clone() const
{
  QgsComposerSymbolV2Item* cloneItem = new QgsComposerSymbolV2Item();
  *cloneItem = *this;
  if ( mSymbolV2 )
  {
    cloneItem->setSymbolV2( mSymbolV2->clone() );
  }
  return cloneItem;
}

void QgsComposerSymbolV2Item::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  QDomElement vectorClassElem = doc.createElement( "VectorClassificationItemNg" );
  if ( mSymbolV2 )
  {
    QgsSymbolV2Map saveSymbolMap;
    saveSymbolMap.insert( "classificationSymbol", mSymbolV2 );
    QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( saveSymbolMap, "symbols", doc );
    vectorClassElem.appendChild( symbolsElem );
  }
  vectorClassElem.setAttribute( "text", text() );
  elem.appendChild( vectorClassElem );
}

void QgsComposerSymbolV2Item::readXML( const QDomElement& itemElem )
{
  //soon...
}

void QgsComposerSymbolV2Item::setSymbolV2( QgsSymbolV2* s )
{
  delete mSymbolV2;
  mSymbolV2 = s;
}

////////////////////QgsComposerLayerItem

QgsComposerLayerItem::QgsComposerLayerItem(): QgsComposerLegendItem()
{
}

QgsComposerLayerItem::QgsComposerLayerItem( const QString& text ): QgsComposerLegendItem( text )
{
}

QgsComposerLayerItem::~QgsComposerLayerItem()
{
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
  layerItemElem.setAttribute( "text", text() );
  writeXMLChildren( layerItemElem, doc );
  elem.appendChild( layerItemElem );
}

void QgsComposerLayerItem::readXML( const QDomElement& itemElem )
{
  //soon...
}

////////////////////QgsComposerGroupItem

QgsComposerGroupItem::QgsComposerGroupItem(): QgsComposerLegendItem()
{
}

QgsComposerGroupItem::QgsComposerGroupItem( const QString& text ): QgsComposerLegendItem( text )
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
  layerGroupElem.setAttribute( "text", text() );
  writeXMLChildren( layerGroupElem, doc );
  elem.appendChild( layerGroupElem );
}

void QgsComposerGroupItem::readXML( const QDomElement& itemElem )
{
  //soon...
}
