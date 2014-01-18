/***************************************************************************
                         qgscomposershape.cpp
                         ----------------------
    begin                : November 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco@hugis.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposershape.h"
#include "qgscomposition.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include <QPainter>

QgsComposerShape::QgsComposerShape( QgsComposition* composition ): QgsComposerItem( composition ),
    mShape( Ellipse ),
    mCornerRadius( 0 ),
    mUseSymbolV2( false ), //default to not using SymbolV2 for shapes, to preserve 2.0 api
    mShapeStyleSymbol( 0 )
{
  setFrameEnabled( true );
  createDefaultShapeStyleSymbol();
}

QgsComposerShape::QgsComposerShape( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition ):
    QgsComposerItem( x, y, width, height, composition ),
    mShape( Ellipse ),
    mCornerRadius( 0 ),
    mUseSymbolV2( false ), //default to not using SymbolV2 for shapes, to preserve 2.0 api
    mShapeStyleSymbol( 0 )
{
  setSceneRect( QRectF( x, y, width, height ) );
  setFrameEnabled( true );
  createDefaultShapeStyleSymbol();
}

QgsComposerShape::~QgsComposerShape()
{
  delete mShapeStyleSymbol;
}

void QgsComposerShape::setUseSymbolV2( bool useSymbolV2 )
{
  mUseSymbolV2 = useSymbolV2;
  setFrameEnabled( !useSymbolV2 );
}

void QgsComposerShape::setShapeStyleSymbol( QgsFillSymbolV2* symbol )
{
  delete mShapeStyleSymbol;
  mShapeStyleSymbol = symbol;
  update();
}

void QgsComposerShape::createDefaultShapeStyleSymbol()
{
  delete mShapeStyleSymbol;
  QgsStringMap properties;
  properties.insert( "color", "white" );
  properties.insert( "style", "solid" );
  properties.insert( "style_border", "solid" );
  properties.insert( "color_border", "black" );
  properties.insert( "width_border", "0.3" );
  mShapeStyleSymbol = QgsFillSymbolV2::createSimple( properties );
}

void QgsComposerShape::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter )
  {
    return;
  }
  drawBackground( painter );
  drawFrame( painter );

  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}


void QgsComposerShape::drawShape( QPainter* p )
{
  if ( mUseSymbolV2 )
  {
    drawShapeUsingSymbol( p );
    return;
  }

  //draw using QPainter brush and pen to keep 2.0 api compatibility
  p->save();
  p->setRenderHint( QPainter::Antialiasing );

  switch ( mShape )
  {
    case Ellipse:
      p->drawEllipse( QRectF( 0, 0 , rect().width(), rect().height() ) );
      break;
    case Rectangle:
      //if corner radius set, then draw a rounded rectangle
      if ( mCornerRadius > 0 )
      {
        p->drawRoundedRect( QRectF( 0, 0 , rect().width(), rect().height() ), mCornerRadius, mCornerRadius );
      }
      else
      {
        p->drawRect( QRectF( 0, 0 , rect().width(), rect().height() ) );
      }
      break;
    case Triangle:
      QPolygonF triangle;
      triangle << QPointF( 0, rect().height() );
      triangle << QPointF( rect().width() , rect().height() );
      triangle << QPointF( rect().width() / 2.0, 0 );
      p->drawPolygon( triangle );
      break;
  }
  p->restore();
}

void QgsComposerShape::drawShapeUsingSymbol( QPainter* p )
{
  p->save();
  p->setRenderHint( QPainter::Antialiasing );

  QgsRenderContext context;
  context.setPainter( p );
  context.setScaleFactor( 1.0 );
  if ( mComposition->plotStyle() ==  QgsComposition::Preview )
  {
    context.setRasterScaleFactor( horizontalViewScaleFactor() );
  }
  else
  {
    context.setRasterScaleFactor( mComposition->printResolution() / 25.4 );
  }

  //generate polygon to draw
  QList<QPolygonF> rings; //empty list
  QPolygonF shapePolygon;

  //shapes with curves must be enlarged before conversion to QPolygonF, or
  //the curves are approximated too much and appear jaggy
  QTransform t = QTransform::fromScale( 100, 100 );
  //inverse transform used to scale created polygons back to expected size
  QTransform ti = t.inverted();

  switch ( mShape )
  {
    case Ellipse:
    {
      //create an ellipse
      QPainterPath ellipsePath;
      ellipsePath.addEllipse( QRectF( 0, 0 , rect().width(), rect().height() ) );
      QPolygonF ellipsePoly = ellipsePath.toFillPolygon( t );
      shapePolygon = ti.map( ellipsePoly );
      break;
    }
    case Rectangle:
    {
      //if corner radius set, then draw a rounded rectangle
      if ( mCornerRadius > 0 )
      {
        QPainterPath roundedRectPath;
        roundedRectPath.addRoundedRect( QRectF( 0, 0 , rect().width(), rect().height() ), mCornerRadius, mCornerRadius );
        QPolygonF roundedPoly = roundedRectPath.toFillPolygon( t );
        shapePolygon = ti.map( roundedPoly );
      }
      else
      {
        shapePolygon = QPolygonF( QRectF( 0, 0, rect().width(), rect().height() ) );
      }
      break;
    }
    case Triangle:
    {
      shapePolygon << QPointF( 0, rect().height() );
      shapePolygon << QPointF( rect().width() , rect().height() );
      shapePolygon << QPointF( rect().width() / 2.0, 0 );
      shapePolygon << QPointF( 0, rect().height() );
      break;
    }
  }

  mShapeStyleSymbol->startRender( context );

  double maxBleed = QgsSymbolLayerV2Utils::estimateMaxSymbolBleed( mShapeStyleSymbol );

  //even though we aren't going to use it to draw the shape, set the pen width as 2 * symbol bleed
  //so that the item is fully rendered within it's scene rect
  //(QGraphicsRectItem considers the pen width when calculating an item's scene rect)
  setPen( QPen( QBrush( Qt::NoBrush ),  maxBleed * 2.0 ) );

  //need to render using atlas feature properties?
  if ( mComposition->atlasComposition().enabled() && mComposition->atlasMode() != QgsComposition::AtlasOff )
  {
    //using an atlas, so render using current atlas feature
    //since there may be data defined symbols using atlas feature properties
    mShapeStyleSymbol->renderPolygon( shapePolygon, &rings, mComposition->atlasComposition().currentFeature(), context );
  }
  else
  {
    mShapeStyleSymbol->renderPolygon( shapePolygon, &rings, 0, context );
  }

  mShapeStyleSymbol->stopRender( context );
  p->restore();
}


void QgsComposerShape::drawFrame( QPainter* p )
{
  if ( mFrame && p && !mUseSymbolV2 )
  {
    p->setPen( pen() );
    p->setBrush( Qt::NoBrush );
    p->setRenderHint( QPainter::Antialiasing, true );
    drawShape( p );
  }
}

void QgsComposerShape::drawBackground( QPainter* p )
{
  if ( p && ( mBackground || mUseSymbolV2 ) )
  {
    p->setBrush( brush() );//this causes a problem in atlas generation
    p->setPen( Qt::NoPen );
    p->setRenderHint( QPainter::Antialiasing, true );
    drawShape( p );
  }
}


bool QgsComposerShape::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerShapeElem = doc.createElement( "ComposerShape" );
  composerShapeElem.setAttribute( "shapeType", mShape );
  composerShapeElem.setAttribute( "cornerRadius", mCornerRadius );

  QDomElement shapeStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mShapeStyleSymbol, doc );
  composerShapeElem.appendChild( shapeStyleElem );

  elem.appendChild( composerShapeElem );
  return _writeXML( composerShapeElem, doc );
}

bool QgsComposerShape::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  mShape = QgsComposerShape::Shape( itemElem.attribute( "shapeType", "0" ).toInt() );
  mCornerRadius = itemElem.attribute( "cornerRadius", "0" ).toDouble();

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    //rotation
    if ( composerItemElem.attribute( "rotation", "0" ).toDouble() != 0 )
    {
      //check for old (pre 2.1) rotation attribute
      setItemRotation( composerItemElem.attribute( "rotation", "0" ).toDouble() );
    }

    _readXML( composerItemElem, doc );
  }

  QDomElement shapeStyleSymbolElem = itemElem.firstChildElement( "symbol" );
  if ( !shapeStyleSymbolElem.isNull() )
  {
    delete mShapeStyleSymbol;
    mShapeStyleSymbol = dynamic_cast<QgsFillSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( shapeStyleSymbolElem ) );
  }
  else
  {
    //upgrade project file from 2.0 to use symbolV2 styling
    delete mShapeStyleSymbol;
    QgsStringMap properties;
    properties.insert( "color", QgsSymbolLayerV2Utils::encodeColor( brush().color() ) );
    if ( hasBackground() )
    {
      properties.insert( "style", "solid" );
    }
    else
    {
      properties.insert( "style", "no" );
    }
    if ( hasFrame() )
    {
      properties.insert( "style_border", "solid" );
    }
    else
    {
      properties.insert( "style_border", "no" );
    }
    properties.insert( "color_border", QgsSymbolLayerV2Utils::encodeColor( pen().color() ) );
    properties.insert( "width_border", QString::number( pen().widthF() ) );
    mShapeStyleSymbol = QgsFillSymbolV2::createSimple( properties );
  }
  emit itemChanged();
  return true;
}

void QgsComposerShape::setCornerRadius( double radius )
{
  mCornerRadius = radius;
}
