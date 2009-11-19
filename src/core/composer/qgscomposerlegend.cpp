/***************************************************************************
                         qgscomposerlegend.cpp  -  description
                         ---------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerlegend.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsrenderer.h" //for brush scaling
#include "qgssymbol.h"
#include <QDomDocument>
#include <QDomElement>
#include <QPainter>

QgsComposerLegend::QgsComposerLegend( QgsComposition* composition ): QgsComposerItem( composition ), mTitle( tr( "Legend" ) ), mBoxSpace( 2 ), mLayerSpace( 3 ), mSymbolSpace( 2 ), mIconLabelSpace( 2 )
{
  QStringList idList = layerIdList();
  mLegendModel.setLayerSet( idList );

  mTitleFont.setPointSizeF( 14.0 );
  mLayerFont.setPointSizeF( 12.0 );
  mItemFont.setPointSizeF( 12.0 );

  mSymbolWidth = 7;
  mSymbolHeight = 4;
  adjustBoxSize();

  connect( &mLegendModel, SIGNAL( layersChanged() ), this, SLOT( synchronizeWithModel() ) );
}

QgsComposerLegend::QgsComposerLegend(): QgsComposerItem( 0 )
{

}

QgsComposerLegend::~QgsComposerLegend()
{

}

void QgsComposerLegend::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  paintAndDetermineSize( painter );
}

QSizeF QgsComposerLegend::paintAndDetermineSize( QPainter* painter )
{
  QSizeF size;
  double maxXCoord = 0;



  //go through model...
  QStandardItem* rootItem = mLegendModel.invisibleRootItem();
  if ( !rootItem )
  {
    return size;
  }


  if ( painter )
  {
    painter->save();
    drawBackground( painter );
    painter->setPen( QPen( QColor( 0, 0, 0 ) ) );
  }

  int numLayerItems = rootItem->rowCount();
  QStandardItem* currentLayerItem = 0;
  double currentYCoordinate = mBoxSpace;

  //font metrics

  //draw title
  currentYCoordinate += fontAscentMillimeters( mTitleFont );
  if ( painter )
  {
    drawText( painter, mBoxSpace, currentYCoordinate, mTitle, mTitleFont );
  }

  maxXCoord = 2 * mBoxSpace + textWidthMillimeters( mTitleFont, mTitle );

  //draw only visible layer items
  QgsMapRenderer* theMapRenderer = mComposition->mapRenderer();
  QStringList visibleLayerIds;
  if ( theMapRenderer )
  {
    visibleLayerIds = theMapRenderer->layerSet();
  }


  for ( int i = 0; i < numLayerItems; ++i )
  {
    currentLayerItem = rootItem->child( i );
    if ( currentLayerItem )
    {
      QString currentLayerId = currentLayerItem->data().toString();
      int opacity = 255;
      QgsMapLayer* currentLayer = QgsMapLayerRegistry::instance()->mapLayer( currentLayerId );
      if ( currentLayer )
      {
        opacity = currentLayer->getTransparency();
      }

      if ( visibleLayerIds.contains( currentLayerId ) )
      {
        //Let the user omit the layer title item by having an empty layer title string
        if ( !currentLayerItem->text().isEmpty() )
        {
          currentYCoordinate += mLayerSpace;
          currentYCoordinate += fontAscentMillimeters( mLayerFont );

          //draw layer Item
          if ( painter )
          {
            drawText( painter, mBoxSpace, currentYCoordinate, currentLayerItem->text(), mLayerFont );
          }
        }

        maxXCoord = std::max( maxXCoord, 2 * mBoxSpace + textWidthMillimeters( mLayerFont, currentLayerItem->text() ) );

        //and child items
        drawLayerChildItems( painter, currentLayerItem, currentYCoordinate, maxXCoord, opacity );
      }
    }
  }

  currentYCoordinate += mBoxSpace;

  if ( painter )
  {
    painter->restore();

    //draw frame and selection boxes if necessary
    drawFrame( painter );
    if ( isSelected() )
    {
      drawSelectionBoxes( painter );
    }
  }

  size.setHeight( currentYCoordinate );
  size.setWidth( maxXCoord );

  //adjust box if width or height is to small
  if ( painter && currentYCoordinate > rect().width() )
  {
    setSceneRect( QRectF( transform().dx(), transform().dy(), rect().width(), currentYCoordinate ) );
  }
  if ( painter && maxXCoord > rect().height() )
  {
    setSceneRect( QRectF( transform().dx(), transform().dy(), maxXCoord, rect().height() ) );
  }

  return size;
}

void QgsComposerLegend::adjustBoxSize()
{
  QSizeF size = paintAndDetermineSize( 0 );
  if ( size.isValid() )
  {
    setSceneRect( QRectF( transform().dx(), transform().dy(), size.width(), size.height() ) );
  }
}

void QgsComposerLegend::drawLayerChildItems( QPainter* p, QStandardItem* layerItem, double& currentYCoord, double& maxXCoord, int layerOpacity )
{
  if ( !layerItem )
  {
    return;
  }

  //standerd item height
  double itemHeight = std::max( mSymbolHeight, fontAscentMillimeters( mItemFont ) );

  QStandardItem* currentItem;

  int numChildren = layerItem->rowCount();

  for ( int i = 0; i < numChildren; ++i )
  {
    //real symbol height. Can be different from standard height in case of point symbols
    double realSymbolHeight;
    double realItemHeight = itemHeight; //will be adjusted if realSymbolHeight turns out to be larger

    currentYCoord += mSymbolSpace;
    double currentXCoord = mBoxSpace;

    currentItem = layerItem->child( i, 0 );

    if ( !currentItem )
    {
      continue;
    }

    //take QgsSymbol* from user data
    QVariant symbolVariant = currentItem->data();
    QgsSymbol* symbol = 0;
    if ( symbolVariant.canConvert<void*>() )
    {
      void* symbolData = symbolVariant.value<void*>();
      symbol = ( QgsSymbol* )( symbolData );
    }

    if ( symbol )  //item with symbol?
    {
      //draw symbol
      drawSymbol( p, symbol, currentYCoord + ( itemHeight - mSymbolHeight ) / 2, currentXCoord, realSymbolHeight, layerOpacity );
      realItemHeight = std::max( realSymbolHeight, itemHeight );
      currentXCoord += mIconLabelSpace;
    }
    else //item with icon?
    {
      QIcon symbolIcon = currentItem->icon();
      if ( !symbolIcon.isNull() && p )
      {
        symbolIcon.paint( p, currentXCoord, currentYCoord, mSymbolWidth, mSymbolHeight );
        currentXCoord += mSymbolWidth;
        currentXCoord += mIconLabelSpace;
      }
    }

    //finally draw text
    if ( p )
    {
      drawText( p, currentXCoord, currentYCoord + fontAscentMillimeters( mItemFont ) + ( realItemHeight - fontAscentMillimeters( mItemFont ) ) / 2, currentItem->text(), mItemFont );
    }

    maxXCoord = std::max( maxXCoord, currentXCoord + textWidthMillimeters( mItemFont, currentItem->text() ) + mBoxSpace );

    currentYCoord += realItemHeight;
  }
}

void QgsComposerLegend::drawSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int layerOpacity ) const
{
  if ( !s )
  {
    return;
  }

  QGis::GeometryType symbolType = s->type();
  switch ( symbolType )
  {
    case QGis::Point:
      drawPointSymbol( p, s, currentYCoord, currentXPosition, symbolHeight, layerOpacity );
      break;
    case QGis::Line:
      drawLineSymbol( p, s, currentYCoord, currentXPosition, layerOpacity );
      symbolHeight = mSymbolHeight;
      break;
    case QGis::Polygon:
      drawPolygonSymbol( p, s, currentYCoord, currentXPosition, layerOpacity );
      symbolHeight = mSymbolHeight;
      break;
    case QGis::UnknownGeometry:
      // shouldn't occur
      break;
  }
}

void QgsComposerLegend::drawPointSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int opacity ) const
{
  if ( !s )
  {
    return;
  }

  QImage pointImage;
  double rasterScaleFactor = 1.0;
  if ( p )
  {
    QPaintDevice* paintDevice = p->device();
    if ( !paintDevice )
    {
      return;
    }

    rasterScaleFactor = ( paintDevice->logicalDpiX() + paintDevice->logicalDpiY() ) / 2.0 / 25.4;
  }

  //width scale is 1.0
  pointImage = s->getPointSymbolAsImage( 1.0, false, Qt::yellow, 1.0, 0.0, rasterScaleFactor, opacity / 255.0 );

  if ( p )
  {
    p->save();
    p->scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor );

    QPointF imageTopLeft( currentXPosition * rasterScaleFactor, currentYCoord * rasterScaleFactor );
    p->drawImage( imageTopLeft, pointImage );
    p->restore();
  }

  currentXPosition += s->pointSize(); //pointImage.width() / rasterScaleFactor;
  symbolHeight = s->pointSize(); //pointImage.height() / rasterScaleFactor;
}

void QgsComposerLegend::drawLineSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, int opacity ) const
{
  if ( !s )
  {
    return;
  }

  double yCoord = currentYCoord + mSymbolHeight / 2;

  if ( p )
  {
    p->save();
    QPen symbolPen = s->pen();
    QColor penColor = symbolPen.color();
    penColor.setAlpha( opacity );
    symbolPen.setColor( penColor );
    symbolPen.setCapStyle( Qt::FlatCap );
    p->setPen( symbolPen );
    p->drawLine( QPointF( currentXPosition, yCoord ), QPointF( currentXPosition + mSymbolWidth, yCoord ) );
    p->restore();
  }

  currentXPosition += mSymbolWidth;
}

void QgsComposerLegend::drawPolygonSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, int opacity ) const
{
  if ( !s )
  {
    return;
  }

  if ( p )
  {
    //scale brush and set transparencies
    QBrush symbolBrush = s->brush();
    QColor brushColor = symbolBrush.color();
    brushColor.setAlpha( opacity );
    symbolBrush.setColor( brushColor );
    QPaintDevice* paintDevice = p->device();
    if ( paintDevice )
    {
      double rasterScaleFactor = ( paintDevice->logicalDpiX() + paintDevice->logicalDpiY() ) / 2.0 / 25.4;
      QgsRenderer::scaleBrush( symbolBrush, rasterScaleFactor );
    }
    p->setBrush( symbolBrush );

    QPen symbolPen = s->pen();
    QColor penColor = symbolPen.color();
    penColor.setAlpha( opacity );
    symbolPen.setColor( penColor );
    p->setPen( symbolPen );

    p->drawRect( QRectF( currentXPosition, currentYCoord, mSymbolWidth, mSymbolHeight ) );
  }

  currentXPosition += mSymbolWidth;
}

QStringList QgsComposerLegend::layerIdList() const
{
  QStringList layerIdList;
  QMap<QString, QgsMapLayer*> layerMap =  QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::const_iterator mapIt = layerMap.constBegin();

  for ( ; mapIt != layerMap.constEnd(); ++mapIt )
  {
    layerIdList.push_back( mapIt.key() );
  }

  return layerIdList;
}

void QgsComposerLegend::synchronizeWithModel()
{
  adjustBoxSize();
  update();
}

void QgsComposerLegend::setTitleFont( const QFont& f )
{
  mTitleFont = f;
  adjustBoxSize();
  update();
}

void QgsComposerLegend::setLayerFont( const QFont& f )
{
  mLayerFont = f;
  adjustBoxSize();
  update();
}

void QgsComposerLegend::setItemFont( const QFont& f )
{
  mItemFont = f;
  adjustBoxSize();
  update();
}

QFont QgsComposerLegend::titleFont() const
{
  return mTitleFont;
}

QFont QgsComposerLegend::layerFont() const
{
  return mLayerFont;
}

QFont QgsComposerLegend::itemFont() const
{
  return mItemFont;
}

void QgsComposerLegend::updateLegend()
{
  mLegendModel.setLayerSet( layerIdList() );
  adjustBoxSize();
  update();
}

bool QgsComposerLegend::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement composerLegendElem = doc.createElement( "ComposerLegend" );

  //write general properties
  composerLegendElem.setAttribute( "title", mTitle );
  composerLegendElem.setAttribute( "titleFont", mTitleFont.toString() );
  composerLegendElem.setAttribute( "layerFont", mLayerFont.toString() );
  composerLegendElem.setAttribute( "itemFont", mItemFont.toString() );
  composerLegendElem.setAttribute( "boxSpace", QString::number( mBoxSpace ) );
  composerLegendElem.setAttribute( "layerSpace", QString::number( mLayerSpace ) );
  composerLegendElem.setAttribute( "symbolSpace", QString::number( mSymbolSpace ) );
  composerLegendElem.setAttribute( "iconLabelSpace", QString::number( mIconLabelSpace ) );
  composerLegendElem.setAttribute( "symbolWidth", mSymbolWidth );
  composerLegendElem.setAttribute( "symbolHeight", mSymbolHeight );

  //write model properties
  mLegendModel.writeXML( composerLegendElem, doc );

  elem.appendChild( composerLegendElem );
  return _writeXML( composerLegendElem, doc );
}

bool QgsComposerLegend::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  //read general properties
  mTitle = itemElem.attribute( "title" );
  //title font
  QString titleFontString = itemElem.attribute( "titleFont" );
  if ( !titleFontString.isEmpty() )
  {
    mTitleFont.fromString( titleFontString );
  }
  //layer font
  QString layerFontString = itemElem.attribute( "layerFont" );
  if ( !layerFontString.isEmpty() )
  {
    mLayerFont.fromString( layerFontString );
  }
  //item font
  QString itemFontString = itemElem.attribute( "itemFont" );
  if ( !itemFontString.isEmpty() )
  {
    mItemFont.fromString( itemFontString );
  }

  //spaces
  mBoxSpace = itemElem.attribute( "boxSpace", "2.0" ).toDouble();
  mLayerSpace = itemElem.attribute( "layerSpace", "3.0" ).toDouble();
  mSymbolSpace = itemElem.attribute( "symbolSpace", "2.0" ).toDouble();
  mIconLabelSpace = itemElem.attribute( "iconLabelSpace", "2.0" ).toDouble();
  mSymbolWidth = itemElem.attribute( "symbolWidth", "7.0" ).toDouble();
  mSymbolHeight = itemElem.attribute( "symbolHeight", "14.0" ).toDouble();

  //read model properties
  QDomNodeList modelNodeList = itemElem.elementsByTagName( "Model" );
  if ( modelNodeList.size() > 0 )
  {
    QDomElement modelElem = modelNodeList.at( 0 ).toElement();
    mLegendModel.clear();
    mLegendModel.readXML( modelElem, doc );
  }

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }

  return true;
}
