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
#include "qgscomposermodel.h"
#include <QPainter>

QgsComposerShape::QgsComposerShape( QgsComposition* composition ): QgsComposerItem( composition ),
    mShape( Ellipse ),
    mCornerRadius( 0 ),
    mUseSymbolV2( false ), //default to not using SymbolV2 for shapes, to preserve 2.0 api
    mShapeStyleSymbol( 0 ),
    mMaxSymbolBleed( 0 )
{
  setFrameEnabled( true );
  createDefaultShapeStyleSymbol();

  if ( mComposition )
  {
    //connect to atlas feature changes
    //to update symbol style (in case of data-defined symbology)
    connect( &mComposition->atlasComposition(), SIGNAL( featureChanged( QgsFeature* ) ), this, SLOT( repaint() ) );
  }
}

QgsComposerShape::QgsComposerShape( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition ):
    QgsComposerItem( x, y, width, height, composition ),
    mShape( Ellipse ),
    mCornerRadius( 0 ),
    mUseSymbolV2( false ), //default to not using SymbolV2 for shapes, to preserve 2.0 api
    mShapeStyleSymbol( 0 ),
    mMaxSymbolBleed( 0 )
{
  setSceneRect( QRectF( x, y, width, height ) );
  setFrameEnabled( true );
  createDefaultShapeStyleSymbol();

  if ( mComposition )
  {
    //connect to atlas feature changes
    //to update symbol style (in case of data-defined symbology)
    connect( &mComposition->atlasComposition(), SIGNAL( featureChanged( QgsFeature* ) ), this, SLOT( repaint() ) );
  }
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
  refreshSymbol();
}

void QgsComposerShape::refreshSymbol()
{
  mMaxSymbolBleed = QgsSymbolLayerV2Utils::estimateMaxSymbolBleed( mShapeStyleSymbol );
  updateBoundingRect();

  update();
  emit frameChanged();
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
  properties.insert( "joinstyle", "miter" );
  mShapeStyleSymbol = QgsFillSymbolV2::createSimple( properties );

  mMaxSymbolBleed = QgsSymbolLayerV2Utils::estimateMaxSymbolBleed( mShapeStyleSymbol );
  updateBoundingRect();

  emit frameChanged();
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

  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = p->device()->logicalDpiX() / 25.4;

  //setup render context
  QgsMapSettings ms = mComposition->mapSettings();
  //context units should be in dots
  ms.setOutputDpi( p->device()->logicalDpiX() );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
  context.setPainter( p );
  context.setForceVectorOutput( true );
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
      ellipsePath.addEllipse( QRectF( 0, 0 , rect().width() * dotsPerMM, rect().height() * dotsPerMM ) );
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
        roundedRectPath.addRoundedRect( QRectF( 0, 0 , rect().width() * dotsPerMM, rect().height() * dotsPerMM ), mCornerRadius * dotsPerMM, mCornerRadius * dotsPerMM );
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

double QgsComposerShape::estimatedFrameBleed() const
{
  return mMaxSymbolBleed;
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

    //for pre 2.0 projects, shape color and outline were specified in a different element...
    QDomNodeList outlineColorList = itemElem.elementsByTagName( "OutlineColor" );
    if ( outlineColorList.size() > 0 )
    {
      QDomElement frameColorElem = outlineColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk, widthOk;
      int penRed, penGreen, penBlue, penAlpha;
      double penWidth;

      penWidth = itemElem.attribute( "outlineWidth" ).toDouble( &widthOk );
      penRed = frameColorElem.attribute( "red" ).toDouble( &redOk );
      penGreen = frameColorElem.attribute( "green" ).toDouble( &greenOk );
      penBlue = frameColorElem.attribute( "blue" ).toDouble( &blueOk );
      penAlpha = frameColorElem.attribute( "alpha" ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk && widthOk )
      {
        properties.insert( "color_border", QgsSymbolLayerV2Utils::encodeColor( QColor( penRed, penGreen, penBlue, penAlpha ) ) );
        properties.insert( "width_border", QString::number( penWidth ) );
      }
    }
    QDomNodeList fillColorList = itemElem.elementsByTagName( "FillColor" );
    if ( fillColorList.size() > 0 )
    {
      QDomElement fillColorElem = fillColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int fillRed, fillGreen, fillBlue, fillAlpha;

      fillRed = fillColorElem.attribute( "red" ).toDouble( &redOk );
      fillGreen = fillColorElem.attribute( "green" ).toDouble( &greenOk );
      fillBlue = fillColorElem.attribute( "blue" ).toDouble( &blueOk );
      fillAlpha = fillColorElem.attribute( "alpha" ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        properties.insert( "color", QgsSymbolLayerV2Utils::encodeColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) ) );
        properties.insert( "style", "solid" );
      }
    }
    if ( itemElem.hasAttribute( "transparentFill" ) )
    {
      //old style (pre 2.0) of specifying that shapes had no fill
      bool hasOldTransparentFill = itemElem.attribute( "transparentFill", "0" ).toInt();
      if ( hasOldTransparentFill )
      {
        properties.insert( "style", "no" );
      }
    }

    mShapeStyleSymbol = QgsFillSymbolV2::createSimple( properties );
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

void QgsComposerShape::setSceneRect( const QRectF& rectangle )
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
      break;
    case Rectangle:
      return tr( "<rectangle>" );
      break;
    case Triangle:
      return tr( "<triangle>" );
      break;
  }

  return tr( "<shape>" );
}
