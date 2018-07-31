/***************************************************************************
    qgshighlight.cpp - widget to highlight features on the map
     --------------------------------------
    Date                 : 02-03-2011
    Copyright            : (C) 2011 by Juergen E. Fischer, norBIT GmbH
    Email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QImage>

#include "qgsmarkersymbollayer.h"
#include "qgslinesymbollayer.h"

#include "qgscoordinatetransform.h"
#include "qgsfillsymbollayer.h"
#include "qgsgeometry.h"
#include "qgshighlight.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsrendercontext.h"
#include "qgssymbollayer.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"
#include "qgsrenderer.h"

/* Few notes about highlighting (RB):
 - The highlight fill must always be partially transparent because above highlighted layer
   may be another layer which must remain partially visible.
 - Because single highlight color does not work well with layers using similar layer color
   there were considered various possibilities but no optimal solution was found.
   What does not work:
   - lighter/darker color: it would work more or less for fully opaque highlight, but
     overlaying transparent lighter color over original has small visual effect.
   - complemetary color: mixing transparent (128) complement color with original color
     results in grey for all colors
   - contrast line style/ fill pattern: impression is not highligh but just different style
   - line buffer with contrast (or 2 contrast) color: the same as with patterns, no highlight impression
   - fill with highlight or contrast color but opaque and using pattern
     (e.g. Qt::Dense7Pattern): again no highlight impression
*/

QgsHighlight::QgsHighlight( QgsMapCanvas *mapCanvas, const QgsGeometry &geom, QgsMapLayer *layer )
  : QgsMapCanvasItem( mapCanvas )
  , mLayer( layer )
{
  mGeometry = !geom.isNull() ? new QgsGeometry( geom ) : nullptr;
  init();
}

QgsHighlight::QgsHighlight( QgsMapCanvas *mapCanvas, const QgsFeature &feature, QgsVectorLayer *layer )
  : QgsMapCanvasItem( mapCanvas )
  , mLayer( layer )
  , mFeature( feature )
{
  init();
}

void QgsHighlight::init()
{
  QgsCoordinateTransform ct = mMapCanvas->mapSettings().layerTransform( mLayer );
  if ( ct.isValid() )
  {
    if ( mGeometry )
    {
      mGeometry->transform( ct );
    }
    else if ( mFeature.hasGeometry() )
    {
      QgsGeometry g = mFeature.geometry();
      g.transform( ct );
      mFeature.setGeometry( g );
    }
  }

  updateRect();
  update();
  setColor( QColor( Qt::lightGray ) );
}

QgsHighlight::~QgsHighlight()
{
  delete mGeometry;
}


void QgsHighlight::setColor( const QColor &color )
{
  mColor = color;
  mPen.setColor( color );
  QColor fillColor( color.red(), color.green(), color.blue(), 63 );
  mBrush.setColor( fillColor );
  mBrush.setStyle( Qt::SolidPattern );
}

void QgsHighlight::setFillColor( const QColor &fillColor )
{
  mFillColor = fillColor;
  mBrush.setColor( fillColor );
  mBrush.setStyle( Qt::SolidPattern );
}

std::unique_ptr<QgsFeatureRenderer> QgsHighlight::createRenderer( QgsRenderContext &context, const QColor &color, const QColor &fillColor )
{
  std::unique_ptr<QgsFeatureRenderer> renderer;
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mLayer );
  if ( layer && layer->renderer() )
  {
    renderer.reset( layer->renderer()->clone() );
  }
  if ( renderer )
  {
    Q_FOREACH ( QgsSymbol *symbol, renderer->symbols( context ) )
    {
      if ( !symbol ) continue;
      setSymbol( symbol, context, color, fillColor );
    }
  }
  return renderer;
}

void QgsHighlight::setSymbol( QgsSymbol *symbol, const QgsRenderContext &context,   const QColor &color, const QColor &fillColor )
{
  if ( !symbol ) return;


  for ( int i = symbol->symbolLayerCount() - 1; i >= 0;  i-- )
  {
    QgsSymbolLayer *symbolLayer = symbol->symbolLayer( i );
    if ( !symbolLayer ) continue;

    if ( symbolLayer->subSymbol() )
    {
      setSymbol( symbolLayer->subSymbol(), context, color, fillColor );
    }
    else
    {
      symbolLayer->setColor( color ); // line symbology layers
      symbolLayer->setStrokeColor( color ); // marker and fill symbology layers
      symbolLayer->setFillColor( fillColor ); // marker and fill symbology layers

      // Data defined widths overwrite what we set here (widths do not work with data defined)
      QgsSimpleMarkerSymbolLayer *simpleMarker = dynamic_cast<QgsSimpleMarkerSymbolLayer *>( symbolLayer );
      if ( simpleMarker )
      {
        simpleMarker->setStrokeWidth( getSymbolWidth( context, simpleMarker->strokeWidth(), simpleMarker->strokeWidthUnit() ) );
      }
      QgsSimpleLineSymbolLayer *simpleLine = dynamic_cast<QgsSimpleLineSymbolLayer *>( symbolLayer );
      if ( simpleLine )
      {
        simpleLine->setWidth( getSymbolWidth( context, simpleLine->width(), simpleLine->widthUnit() ) );
      }
      QgsSimpleFillSymbolLayer *simpleFill = dynamic_cast<QgsSimpleFillSymbolLayer *>( symbolLayer );
      if ( simpleFill )
      {
        simpleFill->setStrokeWidth( getSymbolWidth( context, simpleFill->strokeWidth(), simpleFill->outputUnit() ) );
      }
      symbolLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty() );
      symbolLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty() );
    }
  }
}

double QgsHighlight::getSymbolWidth( const QgsRenderContext &context, double width, QgsUnitTypes::RenderUnit unit )
{
  // if necessary scale mm to map units
  double scale = 1.;
  if ( unit == QgsUnitTypes::RenderMapUnits )
  {
    scale = context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters ) / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMapUnits );
  }
  width = std::max( width + 2 * mBuffer * scale, mMinWidth * scale );
  return width;
}

void QgsHighlight::setWidth( int width )
{
  mWidth = width;
  mPen.setWidth( width );
}

void QgsHighlight::paintPoint( QPainter *p, const QgsPointXY &point )
{
  QPolygonF r( 5 );

  double d = mMapCanvas->extent().width() * 0.005;
  r[0] = toCanvasCoordinates( point + QgsVector( -d, -d ) ) - pos();
  r[1] = toCanvasCoordinates( point + QgsVector( d, -d ) ) - pos();
  r[2] = toCanvasCoordinates( point + QgsVector( d, d ) ) - pos();
  r[3] = toCanvasCoordinates( point + QgsVector( -d, d ) ) - pos();
  r[4] = r[0];

  p->drawPolygon( r );
}

void QgsHighlight::paintLine( QPainter *p, QgsPolylineXY line )
{
  QPolygonF polygon( line.size() );

  for ( int i = 0; i < line.size(); i++ )
  {
    polygon[i] = toCanvasCoordinates( line[i] ) - pos();
  }

  p->drawPolyline( polygon );
}

void QgsHighlight::paintPolygon( QPainter *p, const QgsPolygonXY &polygon )
{
  // OddEven fill rule by default
  QPainterPath path;

  p->setPen( mPen );
  p->setBrush( mBrush );

  for ( const auto &sourceRing : polygon )
  {
    if ( sourceRing.empty() )
      continue;

    QPolygonF ring;
    ring.reserve( sourceRing.size() + 1 );

    QPointF lastVertex;
    for ( const auto &sourceVertex : sourceRing )
    {
      //adding point only if it is more than a pixel apart from the previous one
      const QPointF curVertex = toCanvasCoordinates( sourceVertex ) - pos();
      if ( ring.isEmpty() || std::abs( ring.back().x() - curVertex.x() ) > 1 || std::abs( ring.back().y() - curVertex.y() ) > 1 )
      {
        ring.push_back( curVertex );
      }
      lastVertex = curVertex;
    }

    ring.push_back( ring.at( 0 ) );

    path.addPolygon( ring );
  }

  p->drawPath( path );
}

void QgsHighlight::updatePosition()
{
  if ( isVisible() ) updateRect();
}

void QgsHighlight::paint( QPainter *p )
{
  if ( mGeometry )
  {
    p->setPen( mPen );
    p->setBrush( mBrush );

    switch ( mGeometry->type() )
    {
      case QgsWkbTypes::PointGeometry:
      {
        if ( !mGeometry->isMultipart() )
        {
          paintPoint( p, mGeometry->asPoint() );
        }
        else
        {
          QgsMultiPointXY m = mGeometry->asMultiPoint();
          for ( int i = 0; i < m.size(); i++ )
          {
            paintPoint( p, m[i] );
          }
        }
      }
      break;

      case QgsWkbTypes::LineGeometry:
      {
        if ( !mGeometry->isMultipart() )
        {
          paintLine( p, mGeometry->asPolyline() );
        }
        else
        {
          QgsMultiPolylineXY m = mGeometry->asMultiPolyline();

          for ( int i = 0; i < m.size(); i++ )
          {
            paintLine( p, m[i] );
          }
        }
        break;
      }

      case QgsWkbTypes::PolygonGeometry:
      {
        if ( !mGeometry->isMultipart() )
        {
          paintPolygon( p, mGeometry->asPolygon() );
        }
        else
        {
          QgsMultiPolygonXY m = mGeometry->asMultiPolygon();
          for ( int i = 0; i < m.size(); i++ )
          {
            paintPolygon( p, m[i] );
          }
        }
        break;
      }

      case QgsWkbTypes::UnknownGeometry:
      case QgsWkbTypes::NullGeometry:
        return;
    }
  }
  else if ( mFeature.hasGeometry() )
  {
    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mLayer );
    if ( !layer )
      return;
    QgsMapSettings mapSettings = mMapCanvas->mapSettings();
    QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
    context.expressionContext() << QgsExpressionContextUtils::layerScope( mLayer );


    // Because lower level outlines must be covered by upper level fill color
    // we render first with temporary opaque color, which is then replaced
    // by final transparent fill color.
    QColor tmpColor( 255, 0, 0, 255 );
    QColor tmpFillColor( 0, 255, 0, 255 );

    std::unique_ptr< QgsFeatureRenderer > renderer = createRenderer( context, tmpColor, tmpFillColor );
    if ( layer && renderer )
    {

      QSize imageSize( mMapCanvas->mapSettings().outputSize() );
      QImage image = QImage( imageSize.width(), imageSize.height(), QImage::Format_ARGB32 );
      image.fill( 0 );
      QPainter imagePainter( &image );
      imagePainter.setRenderHint( QPainter::Antialiasing, true );

      context.setPainter( &imagePainter );

      renderer->startRender( context, layer->fields() );
      context.expressionContext().setFeature( mFeature );
      renderer->renderFeature( mFeature, context );
      renderer->stopRender( context );

      imagePainter.end();

      // true output color
      int penRed = mPen.color().red();
      int penGreen = mPen.color().green();
      int penBlue = mPen.color().blue();
      // coefficient to subtract alpha using green (temporary fill)
      double k = ( 255. - mBrush.color().alpha() ) / 255.;
      QRgb *line = nullptr;
      for ( int r = 0; r < image.height(); r++ )
      {
        line = ( QRgb * )image.scanLine( r );
        for ( int c = 0; c < image.width(); c++ )
        {
          int alpha = qAlpha( line[c] );
          if ( alpha > 0 )
          {
            int green = qGreen( line[c] );
            line[c] = qRgba( penRed, penGreen, penBlue, qBound<int>( 0, alpha - ( green * k ), 255 ) );
          }
        }
      }

      p->drawImage( 0, 0, image );
    }
  }
}

void QgsHighlight::updateRect()
{
  if ( mGeometry )
  {
    QgsRectangle r = mGeometry->boundingBox();

    if ( r.isEmpty() )
    {
      double d = mMapCanvas->extent().width() * 0.005;
      r.setXMinimum( r.xMinimum() - d );
      r.setYMinimum( r.yMinimum() - d );
      r.setXMaximum( r.xMaximum() + d );
      r.setYMaximum( r.yMaximum() + d );
    }

    setRect( r );
    setVisible( mGeometry );
  }
  else if ( mFeature.hasGeometry() )
  {
    // We are currently using full map canvas extent for two reasons:
    // 1) currently there is no method in QgsFeatureRenderer to get rendered feature
    //    bounding box
    // 2) using different extent would result in shifted fill patterns

    // This is an hack to pass QgsMapCanvasItem::setRect what it
    // expects (encoding of position and size of the item)
    const QgsMapToPixel &m2p = mMapCanvas->mapSettings().mapToPixel();
    QgsPointXY topLeft = m2p.toMapPoint( 0, 0 );
    double res = m2p.mapUnitsPerPixel();
    QSizeF imageSize = mMapCanvas->mapSettings().outputSize();
    QgsRectangle rect( topLeft.x(), topLeft.y(), topLeft.x() + imageSize.width()*res, topLeft.y() - imageSize.height()*res );
    setRect( rect );

    setVisible( true );
  }
  else
  {
    setRect( QgsRectangle() );
  }
}
