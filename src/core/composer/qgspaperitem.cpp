/***************************************************************************
                         qgspaperitem.cpp
                       -------------------
    begin                : September 2008
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

#include "qgspaperitem.h"
#include "qgscomposition.h"
#include "qgsstyle.h"
#include "qgslogger.h"
#include "qgsmapsettings.h"
#include "qgscomposerutils.h"
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QPainter>

//QgsPaperGrid

QgsPaperGrid::QgsPaperGrid( double x, double y, double width, double height, QgsComposition *composition ): QGraphicsRectItem( 0, 0, width, height ), mComposition( composition )
{
  setFlag( QGraphicsItem::ItemIsSelectable, false );
  setFlag( QGraphicsItem::ItemIsMovable, false );
  setZValue( 1000 );
  setPos( x, y );
}

void QgsPaperGrid::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );

  //draw grid
  if ( mComposition )
  {
    if ( mComposition->gridVisible() && mComposition->plotStyle() == QgsComposition::Preview
         && mComposition->snapGridResolution() > 0 )
    {
      int gridMultiplyX = static_cast< int >( mComposition->snapGridOffsetX() / mComposition->snapGridResolution() );
      int gridMultiplyY = static_cast< int >( mComposition->snapGridOffsetY() / mComposition->snapGridResolution() );
      double currentXCoord = mComposition->snapGridOffsetX() - gridMultiplyX * mComposition->snapGridResolution();
      double currentYCoord;
      double minYCoord = mComposition->snapGridOffsetY() - gridMultiplyY * mComposition->snapGridResolution();

      painter->save();
      //turn of antialiasing so grid is nice and sharp
      painter->setRenderHint( QPainter::Antialiasing, false );

      if ( mComposition->gridStyle() == QgsComposition::Solid )
      {
        painter->setPen( mComposition->gridPen() );

        //draw vertical lines
        for ( ; currentXCoord <= rect().width(); currentXCoord += mComposition->snapGridResolution() )
        {
          painter->drawLine( QPointF( currentXCoord, 0 ), QPointF( currentXCoord, rect().height() ) );
        }

        //draw horizontal lines
        currentYCoord = minYCoord;
        for ( ; currentYCoord <= rect().height(); currentYCoord += mComposition->snapGridResolution() )
        {
          painter->drawLine( QPointF( 0, currentYCoord ), QPointF( rect().width(), currentYCoord ) );
        }
      }
      else //'Dots' or 'Crosses'
      {
        QPen gridPen = mComposition->gridPen();
        painter->setPen( gridPen );
        painter->setBrush( QBrush( gridPen.color() ) );
        double halfCrossLength = 1;
        if ( mComposition->gridStyle() == QgsComposition::Dots )
        {
          //dots are actually drawn as tiny crosses a few pixels across
          //check QGraphicsView to get current transform
          if ( scene() )
          {
            QList<QGraphicsView *> viewList = scene()->views();
            if ( !viewList.isEmpty() )
            {
              QGraphicsView *currentView = viewList.at( 0 );
              if ( currentView->isVisible() )
              {
                //set halfCrossLength to equivalent of 1 pixel
                halfCrossLength = 1 / currentView->transform().m11();
              }
            }
          }
        }
        else if ( mComposition->gridStyle() == QgsComposition::Crosses )
        {
          halfCrossLength = mComposition->snapGridResolution() / 6;
        }

        for ( ; currentXCoord <= rect().width(); currentXCoord += mComposition->snapGridResolution() )
        {
          currentYCoord = minYCoord;
          for ( ; currentYCoord <= rect().height(); currentYCoord += mComposition->snapGridResolution() )
          {
            painter->drawLine( QPointF( currentXCoord - halfCrossLength, currentYCoord ), QPointF( currentXCoord + halfCrossLength, currentYCoord ) );
            painter->drawLine( QPointF( currentXCoord, currentYCoord - halfCrossLength ), QPointF( currentXCoord, currentYCoord + halfCrossLength ) );
          }
        }
      }
      painter->restore();
    }
  }
}


//QgsPaperItem

QgsPaperItem::QgsPaperItem( QgsComposition *c )
  : QgsComposerItem( c, false )
{
  initialize();
}

QgsPaperItem::QgsPaperItem( qreal x, qreal y, qreal width, qreal height, QgsComposition *composition )
  : QgsComposerItem( x, y, width, height, composition, false )
{
  initialize();
}

QgsPaperItem::QgsPaperItem()
  : QgsComposerItem( nullptr, false )
{
  initialize();
}

QgsPaperItem::~QgsPaperItem()
{
  delete mPageGrid;
}

void QgsPaperItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter || !mComposition || !mComposition->pagesVisible() )
  {
    return;
  }

  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = painter->device()->logicalDpiX() / 25.4;

  //setup render context
  QgsRenderContext context = QgsComposerUtils::createRenderContextForComposition( mComposition, painter );
  context.setForceVectorOutput( true );

  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  painter->save();

  if ( mComposition->plotStyle() == QgsComposition::Preview )
  {
    //if in preview mode, draw page border and shadow so that it's
    //still possible to tell where pages with a transparent style begin and end
    painter->setRenderHint( QPainter::Antialiasing, false );

    //shadow
    painter->setBrush( QBrush( QColor( 150, 150, 150 ) ) );
    painter->setPen( Qt::NoPen );
    painter->drawRect( QRectF( 1, 1, rect().width() + 1, rect().height() + 1 ) );

    //page area
    painter->setBrush( QColor( 215, 215, 215 ) );
    QPen pagePen = QPen( QColor( 100, 100, 100 ), 0 );
    pagePen.setCosmetic( true );
    painter->setPen( pagePen );
    painter->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  }

  painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots

  painter->setRenderHint( QPainter::Antialiasing );
  mComposition->pageStyleSymbol()->startRender( context );

  calculatePageMargin();
  QPolygonF pagePolygon = QPolygonF( QRectF( mPageMargin * dotsPerMM, mPageMargin * dotsPerMM,
                                     ( rect().width() - 2 * mPageMargin ) * dotsPerMM, ( rect().height() - 2 * mPageMargin ) * dotsPerMM ) );
  QList<QPolygonF> rings; //empty list

  mComposition->pageStyleSymbol()->renderPolygon( pagePolygon, &rings, nullptr, context );
  mComposition->pageStyleSymbol()->stopRender( context );
  painter->restore();
}

void QgsPaperItem::calculatePageMargin()
{
  //get max bleed from symbol
  QgsRenderContext rc = QgsComposerUtils::createRenderContextForMap( mComposition->referenceMap(), nullptr, mComposition->printResolution() );
  double maxBleedPixels = QgsSymbolLayerUtils::estimateMaxSymbolBleed( mComposition->pageStyleSymbol(), rc );

  //Now subtract 1 pixel to prevent semi-transparent borders at edge of solid page caused by
  //anti-aliased painting. This may cause a pixel to be cropped from certain edge lines/symbols,
  //but that can be counteracted by adding a dummy transparent line symbol layer with a wider line width
  maxBleedPixels--;

  double maxBleedMm = ( 25.4 / mComposition->printResolution() ) * maxBleedPixels;
  mPageMargin = maxBleedMm;
}

bool QgsPaperItem::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  Q_UNUSED( elem );
  Q_UNUSED( doc );
  return true;
}

bool QgsPaperItem::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  Q_UNUSED( itemElem );
  Q_UNUSED( doc );
  return true;
}

void QgsPaperItem::setSceneRect( const QRectF &rectangle )
{
  QgsComposerItem::setSceneRect( rectangle );
  //update size and position of attached QgsPaperGrid to reflect new page size and position
  mPageGrid->setRect( 0, 0, rect().width(), rect().height() );
  mPageGrid->setPos( pos().x(), pos().y() );
}

void QgsPaperItem::initialize()
{
  setFlag( QGraphicsItem::ItemIsSelectable, false );
  setFlag( QGraphicsItem::ItemIsMovable, false );
  setZValue( 0 );

  //even though we aren't going to use it to draw the page, set the pen width as 4
  //so that the page border and shadow is fully rendered within its scene rect
  //(QGraphicsRectItem considers the pen width when calculating an item's scene rect)
  setPen( QPen( QBrush( Qt::NoBrush ), 4 ) );

  if ( mComposition )
  {
    //create a new QgsPaperGrid for this page, and add it to the composition
    mPageGrid = new QgsPaperGrid( pos().x(), pos().y(), rect().width(), rect().height(), mComposition );
    mComposition->addItem( mPageGrid );

    //connect to atlas feature changes
    //to update symbol style (in case of data-defined symbology)
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::featureChanged, this, &QgsComposerItem::repaint );
  }
}
