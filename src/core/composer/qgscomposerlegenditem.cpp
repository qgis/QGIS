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

QgsComposerBaseSymbolItem::ItemMetrics QgsComposerBaseSymbolItem::draw( const QgsLegendSettings& settings, ItemContext* ctx ) // QPainter* painter, const QPointF& point, double labelXOffset )
{
  QFont symbolLabelFont = settings.style( QgsComposerLegendStyle::SymbolLabel ).font();

  double textHeight = settings.fontHeightCharacterMM( symbolLabelFont, QChar( '0' ) );
  // itemHeight here is not realy item height, it is only for symbol
  // vertical alignment purpose, i.e. ok take single line height
  // if there are more lines, thos run under the symbol
  double itemHeight = qMax( settings.symbolSize().height(), textHeight );

  ItemMetrics im;
  im.symbolSize = drawSymbol( settings, ctx, itemHeight );
  im.labelSize = drawSymbolText( settings, ctx, im.symbolSize );
  return im;
}

QSizeF QgsComposerBaseSymbolItem::drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const
{
  QIcon symbolIcon = icon();
  if ( symbolIcon.isNull() )
    return QSizeF();

  if ( ctx )
    symbolIcon.paint( ctx->painter, ctx->point.x(), ctx->point.y() + ( itemHeight - settings.symbolSize().height() ) / 2,
                      settings.symbolSize().width(), settings.symbolSize().height() );
  return settings.symbolSize();
}


QSizeF QgsComposerBaseSymbolItem::drawSymbolText( const QgsLegendSettings& settings, ItemContext* ctx, const QSizeF& symbolSize ) const
{
  QSizeF labelSize( 0, 0 );

  QFont symbolLabelFont = settings.style( QgsComposerLegendStyle::SymbolLabel ).font();
  double textHeight = settings.fontHeightCharacterMM( symbolLabelFont, QChar( '0' ) );

  QStringList lines = settings.splitStringForWrapping( data( Qt::DisplayRole ).toString() );

  labelSize.rheight() = lines.count() * textHeight + ( lines.count() - 1 ) * settings.lineSpacing();

  double labelX, labelY;
  if ( ctx )
  {
    ctx->painter->setPen( settings.fontColor() );

    labelX = ctx->point.x() + qMax(( double ) symbolSize.width(), ctx->labelXOffset );
    labelY = ctx->point.y();

    // Vertical alignment of label with symbol
    if ( labelSize.height() < symbolSize.height() )
      labelY += symbolSize.height() / 2 + textHeight / 2;  // label centered with symbol
    else
      labelY += textHeight; // label starts at top and runs under symbol
  }

  for ( QStringList::Iterator itemPart = lines.begin(); itemPart != lines.end(); ++itemPart )
  {
    labelSize.rwidth() = qMax( settings.textWidthMillimeters( symbolLabelFont, *itemPart ), double( labelSize.width() ) );

    if ( ctx )
    {
      settings.drawText( ctx->painter, labelX, labelY, *itemPart , symbolLabelFont );
      if ( itemPart != lines.end() )
        labelY += settings.lineSpacing() + textHeight;
    }
  }

  return labelSize;
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
      mIcon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mItem.symbol, QSize( 30, 30 ) );
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
        lbl += QString( " [%1]" ).arg( parentVectorLayer()->featureCount( mItem.symbol ) );
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
  if ( mItem.symbol )
  {
    QgsSymbolV2Map saveSymbolMap;
    saveSymbolMap.insert( "classificationSymbol", mItem.symbol );
    QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( saveSymbolMap, "symbols", doc );
    vectorClassElem.appendChild( symbolsElem );
  }
  vectorClassElem.setAttribute( "text", mItem.label );
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

  mItem.label = itemElem.attribute( "text", "" );
  setUserText( itemElem.attribute( "userText", "" ) );
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
        delete mItem.symbol;
        mItem.symbol = symbolNg;
      }
    }
  }
}

void QgsComposerSymbolV2Item::setSymbolV2( QgsSymbolV2* s )
{
  Q_UNUSED( s );
}

QSizeF QgsComposerSymbolV2Item::drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const
{
  QgsSymbolV2* s = mItem.symbol;
  if ( !s )
  {
    return QSizeF();
  }

  //consider relation to composer map for symbol sizes in mm
  bool sizeInMapUnits = s->outputUnit() == QgsSymbolV2::MapUnit;
  QgsMarkerSymbolV2* markerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( s );

  //Consider symbol size for point markers
  double height = settings.symbolSize().height();
  double width = settings.symbolSize().width();
  double size = 0;
  //Center small marker symbols
  double widthOffset = 0;
  double heightOffset = 0;

  if ( markerSymbol )
  {
    size = markerSymbol->size();
    height = size;
    width = size;
    if ( sizeInMapUnits )
    {
      height *= settings.mmPerMapUnit();
      width *= settings.mmPerMapUnit();
      markerSymbol->setSize( width );
    }
    if ( width < settings.symbolSize().width() )
    {
      widthOffset = ( settings.symbolSize().width() - width ) / 2.0;
    }
    if ( height < settings.symbolSize().height() )
    {
      heightOffset = ( settings.symbolSize().height() - height ) / 2.0;
    }
  }

  if ( ctx )
  {
    double currentXPosition = ctx->point.x();
    double currentYCoord = ctx->point.y() + ( itemHeight - settings.symbolSize().height() ) / 2;
    QPainter* p = ctx->painter;

    //setup painter scaling to dots so that raster symbology is drawn to scale
    double dotsPerMM = 1.0;
    QPaintDevice* paintDevice = p->device();
    if ( !paintDevice )
    {
      return QSizeF();
    }
    dotsPerMM = paintDevice->logicalDpiX() / 25.4;

    if ( markerSymbol && sizeInMapUnits )
    {
      s->setOutputUnit( QgsSymbolV2::MM );
    }

    int opacity = 255;
    if ( QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem*>( parent() ) )
    {
      if ( QgsMapLayer* currentLayer = QgsMapLayerRegistry::instance()->mapLayer( layerItem->layerID() ) )
      {
        if ( QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( currentLayer ) )
        {
          opacity = 255 - ( 255 * vectorLayer->layerTransparency() / 100 );
        }
      }
    }

    p->save();
    p->setRenderHint( QPainter::Antialiasing );
    if ( opacity != 255 && settings.useAdvancedEffects() )
    {
      //semi transparent layer, so need to draw symbol to an image (to flatten it first)
      //create image which is same size as legend rect, in case symbol bleeds outside its alloted space
      QImage tempImage = QImage( QSize( width * dotsPerMM, height * dotsPerMM ), QImage::Format_ARGB32 );
      QPainter imagePainter( &tempImage );
      tempImage.fill( Qt::transparent );
      imagePainter.translate( dotsPerMM * ( currentXPosition + widthOffset ),
                              dotsPerMM * ( currentYCoord + heightOffset ) );
      s->drawPreviewIcon( &imagePainter, QSize( width * dotsPerMM, height * dotsPerMM ) );
      //reduce opacity of image
      imagePainter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
      imagePainter.fillRect( tempImage.rect(), QColor( 0, 0, 0, opacity ) );
      //draw rendered symbol image
      p->scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );
      p->drawImage( 0, 0, tempImage );
    }
    else
    {
      p->translate( currentXPosition + widthOffset, currentYCoord + heightOffset );
      p->scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );
      s->drawPreviewIcon( p, QSize( width * dotsPerMM, height * dotsPerMM ) );
    }
    p->restore();

    if ( markerSymbol && sizeInMapUnits )
    {
      s->setOutputUnit( QgsSymbolV2::MapUnit );
      markerSymbol->setSize( size );
    }
  }

  return QSizeF( qMax( width + 2 * widthOffset, settings.symbolSize().width() ),
                 qMax( height + 2 * heightOffset, settings.symbolSize().height() ) );
}

QgsComposerSymbolV2Item* QgsComposerSymbolV2Item::findItemByRuleIndex( QgsComposerLayerItem* parentLayerItem, int ruleIndex )
{
  for ( int i = 0; i < parentLayerItem->rowCount(); ++i )
  {
    if ( QgsComposerSymbolV2Item* sItem = dynamic_cast<QgsComposerSymbolV2Item*>( parentLayerItem->child( 0 ) ) )
    {
      if ( sItem->ruleIndex() == ruleIndex )
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
      return mItem.label;
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

QSizeF QgsComposerRasterSymbolItem::drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const
{
  if ( ctx )
  {
    ctx->painter->setBrush( color() );
    ctx->painter->drawRect( QRectF( ctx->point.x(), ctx->point.y() + ( itemHeight - settings.symbolSize().height() ) / 2,
                                    settings.symbolSize().width(), settings.symbolSize().height() ) );
  }
  return settings.symbolSize();
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

QSizeF QgsComposerRasterImageItem::drawSymbol( const QgsLegendSettings& settings, QgsComposerBaseSymbolItem::ItemContext* ctx, double itemHeight ) const
{
  Q_UNUSED( itemHeight );

  if ( ctx )
  {
    ctx->painter->drawImage( QRectF( ctx->point.x(), ctx->point.y(), settings.wmsLegendSize().width(), settings.wmsLegendSize().height() ),
                             mImage, QRectF( 0, 0, mImage.width(), mImage.height() ) );
  }
  return settings.wmsLegendSize();
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



QSizeF QgsComposerLayerItem::draw( const QgsLegendSettings& settings, QPainter* painter, QPointF point )
{
  QSizeF size( 0, 0 );

  //Let the user omit the layer title item by having an empty layer title string
  if ( data( Qt::DisplayRole ).toString().isEmpty() ) return size;

  double y = point.y();

  if ( painter ) painter->setPen( settings.fontColor() );

  QFont layerFont = settings.style( style() ).font();

  QStringList lines = settings.splitStringForWrapping( data( Qt::DisplayRole ).toString() );
  for ( QStringList::Iterator layerItemPart = lines.begin(); layerItemPart != lines.end(); ++layerItemPart )
  {
    y += settings.fontAscentMillimeters( layerFont );
    if ( painter ) settings.drawText( painter, point.x(), y, *layerItemPart , layerFont );
    qreal width = settings.textWidthMillimeters( layerFont, *layerItemPart );
    size.rwidth() = qMax( width, size.width() );
    if ( layerItemPart != lines.end() )
    {
      y += settings.lineSpacing();
    }
  }
  size.rheight() = y - point.y();

  return size;
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


QSizeF QgsComposerGroupItem::draw( const QgsLegendSettings& settings, QPainter* painter, QPointF point )
{
  QSizeF size( 0, 0 );

  double y = point.y();

  if ( painter ) painter->setPen( settings.fontColor() );

  QFont groupFont = settings.style( style() ).font();

  QStringList lines = settings.splitStringForWrapping( data( Qt::DisplayRole ).toString() );
  for ( QStringList::Iterator groupPart = lines.begin(); groupPart != lines.end(); ++groupPart )
  {
    y += settings.fontAscentMillimeters( groupFont );
    if ( painter ) settings.drawText( painter, point.x(), y, *groupPart, groupFont );
    qreal width = settings.textWidthMillimeters( groupFont, *groupPart );
    size.rwidth() = qMax( width, size.width() );
    if ( groupPart != lines.end() )
    {
      y += settings.lineSpacing();
    }
  }
  size.rheight() = y - point.y();
  return size;
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
