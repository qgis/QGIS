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
#include "qgspathresolver.h"
#include "qgsreadwritecontext.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgscomposermodel.h"
#include "qgsmapsettings.h"
#include "qgscomposerutils.h"
#include <QPainter>

QgsComposerShape::QgsComposerShape( QgsComposition *composition )
  : QgsComposerItem( composition )
  , mShape( Ellipse )
  , mCornerRadius( 0 )
  , mUseSymbol( false ) //default to not using symbol for shapes, to preserve 2.0 api
  , mMaxSymbolBleed( 0 )
{
  setFrameEnabled( true );
  createDefaultShapeStyleSymbol();

  if ( mComposition )
  {
    //connect to atlas feature changes
    //to update symbol style (in case of data-defined symbology)
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::featureChanged, this, &QgsComposerItem::repaint );
  }
}

QgsComposerShape::QgsComposerShape( qreal x, qreal y, qreal width, qreal height, QgsComposition *composition )
  : QgsComposerItem( x, y, width, height, composition )
  , mShape( Ellipse )
  , mCornerRadius( 0 )
  , mUseSymbol( false ) //default to not using Symbol for shapes, to preserve 2.0 api
  , mMaxSymbolBleed( 0 )
{
  setSceneRect( QRectF( x, y, width, height ) );
  setFrameEnabled( true );
  createDefaultShapeStyleSymbol();

  if ( mComposition )
  {
    //connect to atlas feature changes
    //to update symbol style (in case of data-defined symbology)
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::featureChanged, this, &QgsComposerItem::repaint );
  }
}

QgsComposerShape::~QgsComposerShape()
{
  delete mShapeStyleSymbol;
}

void QgsComposerShape::setUseSymbol( bool useSymbol )
{
  mUseSymbol = useSymbol;
  setFrameEnabled( !useSymbol );
}

void QgsComposerShape::setShapeStyleSymbol( QgsFillSymbol *symbol )
{
  delete mShapeStyleSymbol;
  mShapeStyleSymbol = static_cast<QgsFillSymbol *>( symbol->clone() );
  refreshSymbol();
}

void QgsComposerShape::refreshSymbol()
{
  QgsRenderContext rc = QgsComposerUtils::createRenderContextForMap( mComposition->referenceMap(), nullptr, mComposition->printResolution() );
  mMaxSymbolBleed = ( 25.4 / mComposition->printResolution() ) * QgsSymbolLayerUtils::estimateMaxSymbolBleed( mShapeStyleSymbol, rc );

  updateBoundingRect();

  update();
  emit frameChanged();
}

void QgsComposerShape::createDefaultShapeStyleSymbol()
{
  delete mShapeStyleSymbol;
  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
  mShapeStyleSymbol = QgsFillSymbol::createSimple( properties );

  if ( mComposition )
  {
    QgsRenderContext rc = QgsComposerUtils::createRenderContextForMap( mComposition->referenceMap(), nullptr, mComposition->printResolution() );
    mMaxSymbolBleed = ( 25.4 / mComposition->printResolution() ) * QgsSymbolLayerUtils::estimateMaxSymbolBleed( mShapeStyleSymbol, rc );
  }

  updateBoundingRect();

  emit frameChanged();
}

void QgsComposerShape::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter )
  {
    return;
  }
  if ( !shouldDrawItem() )
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


void QgsComposerShape::drawShape( QPainter *p )
{
  if ( mUseSymbol )
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
      p->drawEllipse( QRectF( 0, 0, rect().width(), rect().height() ) );
      break;
    case Rectangle:
      //if corner radius set, then draw a rounded rectangle
      if ( mCornerRadius > 0 )
      {
        p->drawRoundedRect( QRectF( 0, 0, rect().width(), rect().height() ), mCornerRadius, mCornerRadius );
      }
      else
      {
        p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
      }
      break;
    case Triangle:
      QPolygonF triangle;
      triangle << QPointF( 0, rect().height() );
      triangle << QPointF( rect().width(), rect().height() );
      triangle << QPointF( rect().width() / 2.0, 0 );
      p->drawPolygon( triangle );
      break;
  }
  p->restore();
}

void QgsComposerShape::drawShapeUsingSymbol( QPainter *p )
{
  p->save();
  p->setRenderHint( QPainter::Antialiasing );

  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = p->device()->logicalDpiX() / 25.4;

  //setup render context
  QgsRenderContext context = QgsComposerUtils::createRenderContextForComposition( mComposition, p );
  context.setForceVectorOutput( true );
  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  p->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots

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
      ellipsePath.addEllipse( QRectF( 0, 0, rect().width() * dotsPerMM, rect().height() * dotsPerMM ) );
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
        roundedRectPath.addRoundedRect( QRectF( 0, 0, rect().width() * dotsPerMM, rect().height() * dotsPerMM ), mCornerRadius * dotsPerMM, mCornerRadius * dotsPerMM );
        QPolygonF roundedPoly = roundedRectPath.toFillPolygon( t );
        shapePolygon = ti.map( roundedPoly );
      }
      else
      {
        shapePolygon = QPolygonF( QRectF( 0, 0, rect().width() * dotsPerMM, rect().height() * dotsPerMM ) );
      }
      break;
    }
    case Triangle:
    {
      shapePolygon << QPointF( 0, rect().height() * dotsPerMM );
      shapePolygon << QPointF( rect().width() * dotsPerMM, rect().height() * dotsPerMM );
      shapePolygon << QPointF( rect().width() / 2.0 * dotsPerMM, 0 );
      shapePolygon << QPointF( 0, rect().height() * dotsPerMM );
      break;
    }
  }

  mShapeStyleSymbol->startRender( context );
  mShapeStyleSymbol->renderPolygon( shapePolygon, &rings, nullptr, context );
  mShapeStyleSymbol->stopRender( context );

  p->restore();
}


void QgsComposerShape::drawFrame( QPainter *p )
{
  if ( mFrame && p && !mUseSymbol )
  {
    p->setPen( pen() );
    p->setBrush( Qt::NoBrush );
    p->setRenderHint( QPainter::Antialiasing, true );
    drawShape( p );
  }
}

void QgsComposerShape::drawBackground( QPainter *p )
{
  if ( p && ( mBackground || mUseSymbol ) )
  {
    p->setBrush( brush() );//this causes a problem in atlas generation
    p->setPen( Qt::NoPen );
    p->setRenderHint( QPainter::Antialiasing, true );
    drawShape( p );
  }
}

double QgsComposerShape::estimatedFrameBleed() const
{
  return mMaxSymbolBleed;
}

bool QgsComposerShape::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  QDomElement composerShapeElem = doc.createElement( QStringLiteral( "ComposerShape" ) );
  composerShapeElem.setAttribute( QStringLiteral( "shapeType" ), mShape );
  composerShapeElem.setAttribute( QStringLiteral( "cornerRadius" ), mCornerRadius );

  QgsReadWriteContext context;
  context.setPathResolver( mComposition->project()->pathResolver() );

  QDomElement shapeStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mShapeStyleSymbol, doc, context );
  composerShapeElem.appendChild( shapeStyleElem );

  elem.appendChild( composerShapeElem );
  return _writeXml( composerShapeElem, doc );
}

bool QgsComposerShape::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  mShape = QgsComposerShape::Shape( itemElem.attribute( QStringLiteral( "shapeType" ), QStringLiteral( "0" ) ).toInt() );
  mCornerRadius = itemElem.attribute( QStringLiteral( "cornerRadius" ), QStringLiteral( "0" ) ).toDouble();

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( QStringLiteral( "ComposerItem" ) );
  if ( !composerItemList.isEmpty() )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    //rotation
    if ( !qgsDoubleNear( composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble(), 0.0 ) )
    {
      //check for old (pre 2.1) rotation attribute
      setItemRotation( composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble() );
    }

    _readXml( composerItemElem, doc );
  }

  QgsReadWriteContext context;
  context.setPathResolver( mComposition->project()->pathResolver() );

  QDomElement shapeStyleSymbolElem = itemElem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !shapeStyleSymbolElem.isNull() )
  {
    delete mShapeStyleSymbol;
    mShapeStyleSymbol = QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( shapeStyleSymbolElem, context );
  }
  else
  {
    //upgrade project file from 2.0 to use symbol styling
    delete mShapeStyleSymbol;
    QgsStringMap properties;
    properties.insert( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( brush().color() ) );
    if ( hasBackground() )
    {
      properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
    }
    else
    {
      properties.insert( QStringLiteral( "style" ), QStringLiteral( "no" ) );
    }
    if ( hasFrame() )
    {
      properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
    }
    else
    {
      properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "no" ) );
    }
    properties.insert( QStringLiteral( "color_border" ), QgsSymbolLayerUtils::encodeColor( pen().color() ) );
    properties.insert( QStringLiteral( "width_border" ), QString::number( pen().widthF() ) );

    //for pre 2.0 projects, shape color and outline were specified in a different element...
    QDomNodeList outlineColorList = itemElem.elementsByTagName( QStringLiteral( "OutlineColor" ) );
    if ( !outlineColorList.isEmpty() )
    {
      QDomElement frameColorElem = outlineColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk, widthOk;
      int penRed, penGreen, penBlue, penAlpha;
      double penWidth;

      penWidth = itemElem.attribute( QStringLiteral( "outlineWidth" ) ).toDouble( &widthOk );
      penRed = frameColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
      penGreen = frameColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
      penBlue = frameColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
      penAlpha = frameColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk && widthOk )
      {
        properties.insert( QStringLiteral( "color_border" ), QgsSymbolLayerUtils::encodeColor( QColor( penRed, penGreen, penBlue, penAlpha ) ) );
        properties.insert( QStringLiteral( "width_border" ), QString::number( penWidth ) );
      }
    }
    QDomNodeList fillColorList = itemElem.elementsByTagName( QStringLiteral( "FillColor" ) );
    if ( !fillColorList.isEmpty() )
    {
      QDomElement fillColorElem = fillColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int fillRed, fillGreen, fillBlue, fillAlpha;

      fillRed = fillColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
      fillGreen = fillColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
      fillBlue = fillColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
      fillAlpha = fillColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        properties.insert( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) ) );
        properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
      }
    }
    if ( itemElem.hasAttribute( QStringLiteral( "transparentFill" ) ) )
    {
      //old style (pre 2.0) of specifying that shapes had no fill
      bool hasOldTransparentFill = itemElem.attribute( QStringLiteral( "transparentFill" ), QStringLiteral( "0" ) ).toInt();
      if ( hasOldTransparentFill )
      {
        properties.insert( QStringLiteral( "style" ), QStringLiteral( "no" ) );
      }
    }

    mShapeStyleSymbol = QgsFillSymbol::createSimple( properties );
  }
  emit itemChanged();
  return true;
}

void QgsComposerShape::setShapeType( QgsComposerShape::Shape s )
{
  if ( s == mShape )
  {
    return;
  }

  mShape = s;

  if ( mComposition && id().isEmpty() )
  {
    //notify the model that the display name has changed
    mComposition->itemsModel()->updateItemDisplayName( this );
  }
}

void QgsComposerShape::setCornerRadius( double radius )
{
  mCornerRadius = radius;
}

QRectF QgsComposerShape::boundingRect() const
{
  return mCurrentRectangle;
}

void QgsComposerShape::updateBoundingRect()
{
  QRectF rectangle = rect();
  rectangle.adjust( -mMaxSymbolBleed, -mMaxSymbolBleed, mMaxSymbolBleed, mMaxSymbolBleed );
  if ( rectangle != mCurrentRectangle )
  {
    prepareGeometryChange();
    mCurrentRectangle = rectangle;
  }
}

void QgsComposerShape::setSceneRect( const QRectF &rectangle )
{
  // Reimplemented from QgsComposerItem as we need to call updateBoundingRect after the shape's size changes

  //update rect for data defined size and position
  QRectF evaluatedRect = evalItemRect( rectangle );
  QgsComposerItem::setSceneRect( evaluatedRect );

  updateBoundingRect();
  update();
}

QString QgsComposerShape::displayName() const
{
  if ( !id().isEmpty() )
  {
    return id();
  }

  switch ( mShape )
  {
    case Ellipse:
      return tr( "<ellipse>" );
    case Rectangle:
      return tr( "<rectangle>" );
    case Triangle:
      return tr( "<triangle>" );
  }

  return tr( "<shape>" );
}
