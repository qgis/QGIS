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
#include "qgssymbol.h"
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

//////////////////////////////QgsComposerSymbolItem

QgsComposerSymbolItem::QgsComposerSymbolItem(): QgsComposerLegendItem( QgsComposerLegendStyle::Symbol ), mSymbol( 0 )
{
}

QgsComposerSymbolItem::QgsComposerSymbolItem( const QString& text ): QgsComposerLegendItem( text, QgsComposerLegendStyle::Symbol ), mSymbol( 0 )
{
}

QgsComposerSymbolItem::QgsComposerSymbolItem( const QIcon& icon, const QString& text ): QgsComposerLegendItem( icon, text, QgsComposerLegendStyle::Symbol ), mSymbol( 0 )
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
  vectorClassElem.setAttribute( "layerId", mLayerID );

  elem.appendChild( vectorClassElem );
}

void QgsComposerSymbolItem::readXML( const QDomElement& itemElem, bool xServerAvailable )
{
  if ( itemElem.isNull() )
  {
    return;
  }
  setText( itemElem.attribute( "text", "" ) );
  setLayerID( itemElem.attribute( "layerId", "" ) );

  QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( mLayerID ) );
  if ( vLayer )
  {
    QDomElement symbolElem = itemElem.firstChildElement( "symbol" );
    if ( !symbolElem.isNull() )
    {
      QgsSymbol* symbol = new QgsSymbol( vLayer->geometryType() );
      symbol->readXML( symbolElem, vLayer );
      setSymbol( symbol );
      if ( !xServerAvailable ) //don't read icon without GUI
      {
        return;
      }

      //add icon
      switch ( symbol->type() )
      {
        case QGis::Point:
          setIcon( QIcon( QPixmap::fromImage( symbol->getPointSymbolAsImage() ) ) );
          break;
        case QGis::Line:
          setIcon( QIcon( QPixmap::fromImage( symbol->getLineSymbolAsImage() ) ) );
          break;
        case QGis::Polygon:
          setIcon( QIcon( QPixmap::fromImage( symbol->getPolygonSymbolAsImage() ) ) );
          break;
        case QGis::NoGeometry:
          setIcon( QIcon( QgsApplication::activeThemePath() + "/mIconTableLayer.png" ) );
          break;
        case QGis::UnknownGeometry:
          // should not occur
          break;
      }
    }
  }
}

////////////////QgsComposerSymbolV2Item

#include "qgssymbolv2.h"

QgsComposerSymbolV2Item::QgsComposerSymbolV2Item(): QgsComposerLegendItem( QgsComposerLegendStyle::Symbol ), mSymbolV2( 0 )
{
}

QgsComposerSymbolV2Item::QgsComposerSymbolV2Item( const QString& text ): QgsComposerLegendItem( text, QgsComposerLegendStyle::Symbol ), mSymbolV2( 0 )
{
}

QgsComposerSymbolV2Item::QgsComposerSymbolV2Item( const QIcon& icon, const QString& text ): QgsComposerLegendItem( icon, text, QgsComposerLegendStyle::Symbol ), mSymbolV2( 0 )
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

void QgsComposerSymbolV2Item::readXML( const QDomElement& itemElem, bool xServerAvailable )
{
  if ( itemElem.isNull() )
  {
    return;
  }

  setText( itemElem.attribute( "text", "" ) );
  QDomElement symbolsElem = itemElem.firstChildElement( "symbols" );
  if ( !symbolsElem.isNull() )
  {
    QgsSymbolV2Map loadSymbolMap = QgsSymbolLayerV2Utils::loadSymbols( symbolsElem );
    //we assume there is only one symbol in the map...
    QgsSymbolV2Map::iterator mapIt = loadSymbolMap.begin();
    if ( mapIt != loadSymbolMap.end() )
    {
      QgsSymbolV2* symbolNg = mapIt.value();
      if ( symbolNg )
      {
        setSymbolV2( symbolNg );
        if ( xServerAvailable )
        {
          setIcon( QgsSymbolLayerV2Utils::symbolPreviewIcon( symbolNg, QSize( 30, 30 ) ) );
        }
      }
    }
  }
}

void QgsComposerSymbolV2Item::setSymbolV2( QgsSymbolV2* s )
{
  delete mSymbolV2;
  mSymbolV2 = s;
}

////////////////////QgsComposerRasterSymbolItem

QgsComposerRasterSymbolItem::QgsComposerRasterSymbolItem(): QgsComposerLegendItem( QgsComposerLegendStyle::Symbol )
{
}

QgsComposerRasterSymbolItem::QgsComposerRasterSymbolItem( const QString& text ): QgsComposerLegendItem( text, QgsComposerLegendStyle::Symbol )
{
}

QgsComposerRasterSymbolItem::QgsComposerRasterSymbolItem( const QIcon& icon, const QString& text ): QgsComposerLegendItem( icon, text, QgsComposerLegendStyle::Symbol )
{
}

QgsComposerRasterSymbolItem::~QgsComposerRasterSymbolItem()
{
}

QStandardItem* QgsComposerRasterSymbolItem::clone() const
{
  QgsComposerRasterSymbolItem* cloneItem  = new QgsComposerRasterSymbolItem();
  *cloneItem = *this;
  cloneItem->setLayerID( mLayerID );
  return cloneItem;
}

void QgsComposerRasterSymbolItem::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  QDomElement rasterClassElem = doc.createElement( "RasterClassificationItem" );
  rasterClassElem.setAttribute( "layerId", mLayerID );
  rasterClassElem.setAttribute( "text", text() );
  rasterClassElem.setAttribute( "color", mColor.name() );
  elem.appendChild( rasterClassElem );
}

void QgsComposerRasterSymbolItem::readXML( const QDomElement& itemElem, bool xServerAvailable )
{
  if ( itemElem.isNull() )
  {
    return;
  }
  setText( itemElem.attribute( "text", "" ) );
  setLayerID( itemElem.attribute( "layerId", "" ) );
  setColor( QColor( itemElem.attribute( "color" ) ) );

  if ( xServerAvailable )
  {
    QPixmap itemPixmap( 20, 20 );
    itemPixmap.fill( mColor );
    setIcon( QIcon( itemPixmap ) );
  }
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
      currentChildItem = new QgsComposerSymbolItem();
    }
    else if ( elemTag == "VectorClassificationItemNg" )
    {
      currentChildItem = new QgsComposerSymbolV2Item();
    }
    else if ( elemTag == "RasterClassificationItem" )
    {
      currentChildItem = new QgsComposerRasterSymbolItem();
    }
    else
    {
      continue; //unsupported child type
    }
    currentChildItem->readXML( currentElem, xServerAvailable );
    appendRow( currentChildItem );
  }
}

void QgsComposerLayerItem::setDefaultStyle()
{
  // set default style according to number of symbols
  QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID() ) );
  if ( vLayer )
  {
    QgsFeatureRendererV2* renderer = vLayer->rendererV2();
    if ( renderer )
    {
      QPair<QString, QgsSymbolV2*> symbolItem = renderer->legendSymbolItems().value( 0 );
      if ( renderer->legendSymbolItems().size() > 1 || !symbolItem.first.isEmpty() )
      {
        setStyle( QgsComposerLegendStyle::Subgroup );
      }
      else
      {
        // Hide title by default for single symbol
        setStyle( QgsComposerLegendStyle::Hidden );
      }
    }
  }
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
  layerGroupElem.setAttribute( "text", text() );
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
  setText( itemElem.attribute( "text", "" ) );

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
