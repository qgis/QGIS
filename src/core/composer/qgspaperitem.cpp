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
#include "qgsstylev2.h"
#include "qgslogger.h"
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QPainter>

//QgsPaperGrid

QgsPaperGrid::QgsPaperGrid( double x, double y, double width, double height, QgsComposition* composition ): QGraphicsRectItem( 0, 0, width, height ), mComposition( composition )
{
  setFlag( QGraphicsItem::ItemIsSelectable, false );
  setFlag( QGraphicsItem::ItemIsMovable, false );
  setZValue( 1000 );
  setPos( x, y );
}

QgsPaperGrid::~QgsPaperGrid()
{
}

void QgsPaperGrid::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );

  //draw grid
  if ( mComposition )
  {
    if ( mComposition->gridVisible() && mComposition->plotStyle() ==  QgsComposition::Preview
         && mComposition->snapGridResolution() > 0 )
    {
      int gridMultiplyX = ( int )( mComposition->snapGridOffsetX() / mComposition->snapGridResolution() );
      int gridMultiplyY = ( int )( mComposition->snapGridOffsetY() / mComposition->snapGridResolution() );
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
            QList<QGraphicsView*> viewList = scene()->views();
            if ( viewList.size() > 0 )
            {
              QGraphicsView* currentView = viewList.at( 0 );
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

QgsPaperItem::QgsPaperItem( QgsComposition* c ): QgsComposerItem( c, false ),
    mPageGrid( 0 )
{
  initialize();
}

QgsPaperItem::QgsPaperItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition ): QgsComposerItem( x, y, width, height, composition, false ),
    mPageGrid( 0 ), mPageMargin( 0 )
{
  initialize();
}

QgsPaperItem::QgsPaperItem(): QgsComposerItem( 0, false ),
    mPageGrid( 0 ), mPageMargin( 0 )
{
  initialize();
}

QgsPaperItem::~QgsPaperItem()
{
  delete mPageGrid;
}

void QgsPaperItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter )
  {
    return;
  }

  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = painter->device()->logicalDpiX() / 25.4;

  //setup render context
  QgsMapSettings ms = mComposition->mapSettings();
  //context units should be in dots
  ms.setOutputDpi( painter->device()->logicalDpiX() );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
  context.setPainter( painter );
  context.setForceVectorOutput( true );

  painter->save();

  if ( mComposition->plotStyle() ==  QgsComposition::Preview )
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
    painter->setPen( QPen( QColor( 100, 100, 100 ) ) );
    painter->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  }

  painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots

  painter->setRenderHint( QPainter::Antialiasing );
  mComposition->pageStyleSymbol()->startRender( context );

  calculatePageMargin();
  QPolygonF pagePolygon = QPolygonF( QRectF( mPageMargin * dotsPerMM, mPageMargin * dotsPerMM,
                                     ( rect().width() - 2 * mPageMargin ) * dotsPerMM, ( rect().height() - 2 * mPageMargin ) * dotsPerMM ) );
  QList<QPolygonF> rings; //empty list

  //need to render using atlas feature properties?
  if ( mComposition->atlasComposition().enabled() && mComposition->atlasMode() != QgsComposition::AtlasOff )
  {
    //using an atlas, so render using current atlas feature
    //since there may be data defined symbols using atlas feature properties
    mComposition->pageStyleSymbol()->renderPolygon( pagePolygon, &rings, mComposition->atlasComposition().currentFeature(), context );
  }
  else
  {
    mComposition->pageStyleSymbol()->renderPolygon( pagePolygon, &rings, 0, context );
  }

  mComposition->pageStyleSymbol()->stopRender( context );
  painter->restore();
}

void QgsPaperItem::calculatePageMargin()
{
  //get max bleed from symbol
  double maxBleed = QgsSymbolLayerV2Utils::estimateMaxSymbolBleed( mComposition->pageStyleSymbol() );

  //Now subtract 1 pixel to prevent semi-transparent borders at edge of solid page caused by
  //anti-aliased painting. This may cause a pixel to be cropped from certain edge lines/symbols,
  //but that can be counteracted by adding a dummy transparent line symbol layer with a wider line width
  mPageMargin = maxBleed - ( 25.4 / mComposition->printResolution() );
}

bool QgsPaperItem::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  Q_UNUSED( elem );
  Q_UNUSED( doc );
  return true;
}

bool QgsPaperItem::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  Q_UNUSED( itemElem );
  Q_UNUSED( doc );
  return true;
}

void QgsPaperItem::setSceneRect( const QRectF& rectangle )
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
    connect( &mComposition->atlasComposition(), SIGNAL( featureChanged( QgsFeature* ) ), this, SLOT( repaint() ) );
  }
}
