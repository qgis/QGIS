/***************************************************************************
                         qgslayoutitempolyline.cpp
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
     email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitempolyline.h"
#include "qgslayoutitemregistry.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgslayout.h"
#include "qgsmapsettings.h"
#include "qgslayoututils.h"
#include "qgsreadwritecontext.h"
#include "qgssvgcache.h"
#include "qgsstyleentityvisitor.h"
#include "qgslinesymbol.h"

#include <QSvgRenderer>
#include <limits>
#include <QGraphicsPathItem>
#include <QVector2D>

QgsLayoutItemPolyline::QgsLayoutItemPolyline( QgsLayout *layout )
  : QgsLayoutNodesItem( layout )
{
  createDefaultPolylineStyleSymbol();
}

QgsLayoutItemPolyline::QgsLayoutItemPolyline( const QPolygonF &polyline, QgsLayout *layout )
  : QgsLayoutNodesItem( polyline, layout )
{
  createDefaultPolylineStyleSymbol();
}

QgsLayoutItemPolyline::~QgsLayoutItemPolyline() = default;

QgsLayoutItemPolyline *QgsLayoutItemPolyline::create( QgsLayout *layout )
{
  return new QgsLayoutItemPolyline( layout );
}

int QgsLayoutItemPolyline::type() const
{
  return QgsLayoutItemRegistry::LayoutPolyline;
}

QIcon QgsLayoutItemPolyline::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemPolyline.svg" ) );
}

bool QgsLayoutItemPolyline::_addNode( const int indexPoint,
                                      QPointF newPoint,
                                      const double radius )
{
  const double distStart = computeDistance( newPoint, mPolygon[0] );
  const double distEnd = computeDistance( newPoint, mPolygon[mPolygon.size() - 1] );

  if ( indexPoint == ( mPolygon.size() - 1 ) )
  {
    if ( distEnd < radius )
      mPolygon.append( newPoint );
    else if ( distStart < radius )
      mPolygon.insert( 0, newPoint );
  }
  else
    mPolygon.insert( indexPoint + 1, newPoint );

  return true;
}

bool QgsLayoutItemPolyline::_removeNode( const int index )
{
  if ( index < 0 || index >= mPolygon.size() )
    return false;

  mPolygon.remove( index );

  if ( mPolygon.size() < 2 )
    mPolygon.clear();
  else
  {
    int newSelectNode = index;
    if ( index >= mPolygon.size() )
      newSelectNode = mPolygon.size() - 1;
    setSelectedNode( newSelectNode );
  }

  return true;
}

void QgsLayoutItemPolyline::createDefaultPolylineStyleSymbol()
{
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "square" ) );

  mPolylineStyleSymbol.reset( QgsLineSymbol::createSimple( properties ) );
  refreshSymbol();
}

void QgsLayoutItemPolyline::refreshSymbol()
{
  if ( auto *lLayout = layout() )
  {
    const QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( lLayout, nullptr, lLayout->renderContext().dpi() );
    mMaxSymbolBleed = ( 25.4 / lLayout->renderContext().dpi() ) * QgsSymbolLayerUtils::estimateMaxSymbolBleed( mPolylineStyleSymbol.get(), rc );
  }

  updateSceneRect();

  emit frameChanged();
}

void QgsLayoutItemPolyline::drawStartMarker( QPainter *painter )
{
  if ( mPolygon.size() < 2 )
    return;

  switch ( mStartMarker )
  {
    case MarkerMode::NoMarker:
      break;

    case MarkerMode::ArrowHead:
    {
      // calculate angle at start of line
      const QLineF startLine( mPolygon.at( 0 ), mPolygon.at( 1 ) );
      const double angle = startLine.angle();
      drawArrow( painter, mPolygon.at( 0 ), angle );
      break;
    }

    case MarkerMode::SvgMarker:
    {
      // calculate angle at start of line
      const QLineF startLine( mPolygon.at( 0 ), mPolygon.at( 1 ) );
      const double angle = startLine.angle();
      drawSvgMarker( painter, mPolygon.at( 0 ), angle, mStartMarkerFile, mStartArrowHeadHeight );
      break;
    }
  }

}

void QgsLayoutItemPolyline::drawEndMarker( QPainter *painter )
{
  if ( mPolygon.size() < 2 )
    return;

  switch ( mEndMarker )
  {
    case MarkerMode::NoMarker:
      break;

    case MarkerMode::ArrowHead:
    {
      // calculate angle at end of line
      const QLineF endLine( mPolygon.at( mPolygon.count() - 2 ), mPolygon.at( mPolygon.count() - 1 ) );
      const double angle = endLine.angle();

      //move end point depending on arrow width
      const QVector2D dir = QVector2D( endLine.dx(), endLine.dy() ).normalized();
      QPointF endPoint = endLine.p2();
      endPoint += ( dir * 0.5 * mArrowHeadWidth ).toPointF();

      drawArrow( painter, endPoint, angle );
      break;
    }
    case MarkerMode::SvgMarker:
    {
      // calculate angle at end of line
      const QLineF endLine( mPolygon.at( mPolygon.count() - 2 ), mPolygon.at( mPolygon.count() - 1 ) );
      const double angle = endLine.angle();
      drawSvgMarker( painter, endLine.p2(), angle, mEndMarkerFile, mEndArrowHeadHeight );
      break;
    }
  }
}

void QgsLayoutItemPolyline::drawArrow( QPainter *painter, QPointF center, double angle )
{
  // translate angle from ccw from axis to cw from north
  angle = 90 - angle;
  QPen p;
  p.setColor( mArrowHeadStrokeColor );
  p.setWidthF( mArrowHeadStrokeWidth );
  painter->setPen( p );
  QBrush b;
  b.setColor( mArrowHeadFillColor );
  painter->setBrush( b );

  drawArrowHead( painter, center.x(), center.y(), angle, mArrowHeadWidth );
}

void QgsLayoutItemPolyline::updateMarkerSvgSizes()
{
  setStartSvgMarkerPath( mStartMarkerFile );
  setEndSvgMarkerPath( mEndMarkerFile );
}

void QgsLayoutItemPolyline::drawArrowHead( QPainter *p, const double x, const double y, const double angle, const double arrowHeadWidth )
{
  if ( !p )
    return;

  const double angleRad = angle / 180.0 * M_PI;
  const QPointF middlePoint( x, y );

  //rotate both arrow points
  const QPointF p1 = QPointF( -arrowHeadWidth / 2.0, arrowHeadWidth );
  const QPointF p2 = QPointF( arrowHeadWidth / 2.0, arrowHeadWidth );

  QPointF p1Rotated, p2Rotated;
  p1Rotated.setX( p1.x() * std::cos( angleRad ) + p1.y() * -std::sin( angleRad ) );
  p1Rotated.setY( p1.x() * std::sin( angleRad ) + p1.y() * std::cos( angleRad ) );
  p2Rotated.setX( p2.x() * std::cos( angleRad ) + p2.y() * -std::sin( angleRad ) );
  p2Rotated.setY( p2.x() * std::sin( angleRad ) + p2.y() * std::cos( angleRad ) );

  QPolygonF arrowHeadPoly;
  arrowHeadPoly << middlePoint;
  arrowHeadPoly << QPointF( middlePoint.x() + p1Rotated.x(), middlePoint.y() + p1Rotated.y() );
  arrowHeadPoly << QPointF( middlePoint.x() + p2Rotated.x(), middlePoint.y() + p2Rotated.y() );
  QPen arrowPen = p->pen();
  arrowPen.setJoinStyle( Qt::RoundJoin );
  QBrush arrowBrush = p->brush();
  arrowBrush.setStyle( Qt::SolidPattern );
  p->setPen( arrowPen );
  p->setBrush( arrowBrush );
  arrowBrush.setStyle( Qt::SolidPattern );
  p->drawPolygon( arrowHeadPoly );
}

void QgsLayoutItemPolyline::drawSvgMarker( QPainter *p, QPointF point, double angle, const QString &markerPath, double height ) const
{
  // translate angle from ccw from axis to cw from north
  angle = 90 - angle;

  if ( mArrowHeadWidth <= 0 || height <= 0 )
  {
    //bad image size
    return;
  }

  if ( markerPath.isEmpty() )
    return;

  QSvgRenderer r;
  const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( markerPath, mArrowHeadWidth, mArrowHeadFillColor, mArrowHeadStrokeColor, mArrowHeadStrokeWidth,
                                 1.0 );
  r.load( svgContent );

  const QgsScopedQPainterState painterState( p );
  p->translate( point.x(), point.y() );
  p->rotate( angle );
  p->translate( -mArrowHeadWidth / 2.0, -height / 2.0 );
  r.render( p, QRectF( 0, 0, mArrowHeadWidth, height ) );
}

QString QgsLayoutItemPolyline::displayName() const
{
  if ( !id().isEmpty() )
    return id();

  return tr( "<Polyline>" );
}

void QgsLayoutItemPolyline::_draw( QgsLayoutItemRenderContext &context, const QStyleOptionGraphicsItem * )
{
  const QgsScopedQPainterState painterState( context.renderContext().painter() );
  //setup painter scaling to dots so that raster symbology is drawn to scale
  const double scale = context.renderContext().convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
  const QTransform t = QTransform::fromScale( scale, scale );

  mPolylineStyleSymbol->startRender( context.renderContext() );
  mPolylineStyleSymbol->renderPolyline( t.map( mPolygon ), nullptr, context.renderContext() );
  mPolylineStyleSymbol->stopRender( context.renderContext() );

  // painter is scaled to dots, so scale back to layout units
  context.renderContext().painter()->scale( context.renderContext().scaleFactor(), context.renderContext().scaleFactor() );

  drawStartMarker( context.renderContext().painter() );
  drawEndMarker( context.renderContext().painter() );
}

void QgsLayoutItemPolyline::_readXmlStyle( const QDomElement &elmt, const QgsReadWriteContext &context )
{
  mPolylineStyleSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( elmt, context ) );
}

void QgsLayoutItemPolyline::setSymbol( QgsLineSymbol *symbol )
{
  mPolylineStyleSymbol.reset( static_cast<QgsLineSymbol *>( symbol->clone() ) );
  refreshSymbol();
}

void QgsLayoutItemPolyline::setStartMarker( QgsLayoutItemPolyline::MarkerMode mode )
{
  mStartMarker = mode;
  update();
}

void QgsLayoutItemPolyline::setEndMarker( QgsLayoutItemPolyline::MarkerMode mode )
{
  mEndMarker = mode;
  update();
}

void QgsLayoutItemPolyline::setArrowHeadWidth( double width )
{
  mArrowHeadWidth = width;
  updateMarkerSvgSizes();
  update();
}

QPainterPath QgsLayoutItemPolyline::shape() const
{
  QPainterPath path;
  path.addPolygon( mPolygon );

  QPainterPathStroker ps;

  ps.setWidth( 2 * mMaxSymbolBleed );
  const QPainterPath strokedOutline = ps.createStroke( path );

  return strokedOutline;
}

QgsLineSymbol *QgsLayoutItemPolyline::symbol()
{
  return mPolylineStyleSymbol.get();
}

void QgsLayoutItemPolyline::setStartSvgMarkerPath( const QString &path )
{
  QSvgRenderer r;
  mStartMarkerFile = path;
  if ( path.isEmpty() || !r.load( path ) )
  {
    mStartArrowHeadHeight = 0;
  }
  else
  {
    //calculate mArrowHeadHeight from svg file and mArrowHeadWidth
    const QRect viewBox = r.viewBox();
    mStartArrowHeadHeight = mArrowHeadWidth / viewBox.width() * viewBox.height();
  }
  updateBoundingRect();
}

void QgsLayoutItemPolyline::setEndSvgMarkerPath( const QString &path )
{
  QSvgRenderer r;
  mEndMarkerFile = path;
  if ( path.isEmpty() || !r.load( path ) )
  {
    mEndArrowHeadHeight = 0;
  }
  else
  {
    //calculate mArrowHeadHeight from svg file and mArrowHeadWidth
    const QRect viewBox = r.viewBox();
    mEndArrowHeadHeight = mArrowHeadWidth / viewBox.width() * viewBox.height();
  }
  updateBoundingRect();
}

void QgsLayoutItemPolyline::setArrowHeadStrokeColor( const QColor &color )
{
  mArrowHeadStrokeColor = color;
  update();
}

void QgsLayoutItemPolyline::setArrowHeadFillColor( const QColor &color )
{
  mArrowHeadFillColor = color;
  update();
}

void QgsLayoutItemPolyline::setArrowHeadStrokeWidth( double width )
{
  mArrowHeadStrokeWidth = width;
  updateBoundingRect();
  update();
}

bool QgsLayoutItemPolyline::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mPolylineStyleSymbol )
  {
    QgsStyleSymbolEntity entity( mPolylineStyleSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, uuid(), displayName() ) ) )
      return false;
  }

  return true;
}

void QgsLayoutItemPolyline::_writeXmlStyle( QDomDocument &doc, QDomElement &elmt, const QgsReadWriteContext &context ) const
{
  const QDomElement pe = QgsSymbolLayerUtils::saveSymbol( QString(),
                         mPolylineStyleSymbol.get(),
                         doc,
                         context );
  elmt.appendChild( pe );
}

bool QgsLayoutItemPolyline::writePropertiesToElement( QDomElement &elmt, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QgsLayoutNodesItem::writePropertiesToElement( elmt, doc, context );

  // absolute paths to relative
  const QString startMarkerPath = QgsSymbolLayerUtils::svgSymbolPathToName( mStartMarkerFile, context.pathResolver() );
  const QString endMarkerPath = QgsSymbolLayerUtils::svgSymbolPathToName( mEndMarkerFile, context.pathResolver() );

  elmt.setAttribute( QStringLiteral( "arrowHeadWidth" ), QString::number( mArrowHeadWidth ) );
  elmt.setAttribute( QStringLiteral( "arrowHeadFillColor" ), QgsSymbolLayerUtils::encodeColor( mArrowHeadFillColor ) );
  elmt.setAttribute( QStringLiteral( "arrowHeadOutlineColor" ), QgsSymbolLayerUtils::encodeColor( mArrowHeadStrokeColor ) );
  elmt.setAttribute( QStringLiteral( "outlineWidth" ), QString::number( mArrowHeadStrokeWidth ) );
  elmt.setAttribute( QStringLiteral( "markerMode" ), mEndMarker );
  elmt.setAttribute( QStringLiteral( "startMarkerMode" ), mStartMarker );
  elmt.setAttribute( QStringLiteral( "startMarkerFile" ), startMarkerPath );
  elmt.setAttribute( QStringLiteral( "endMarkerFile" ), endMarkerPath );

  return true;
}

bool QgsLayoutItemPolyline::readPropertiesFromElement( const QDomElement &elmt, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  mArrowHeadWidth = elmt.attribute( QStringLiteral( "arrowHeadWidth" ), QStringLiteral( "2.0" ) ).toDouble();
  mArrowHeadFillColor = QgsSymbolLayerUtils::decodeColor( elmt.attribute( QStringLiteral( "arrowHeadFillColor" ), QStringLiteral( "0,0,0,255" ) ) );
  mArrowHeadStrokeColor = QgsSymbolLayerUtils::decodeColor( elmt.attribute( QStringLiteral( "arrowHeadOutlineColor" ), QStringLiteral( "0,0,0,255" ) ) );
  mArrowHeadStrokeWidth = elmt.attribute( QStringLiteral( "outlineWidth" ), QStringLiteral( "1.0" ) ).toDouble();
  // relative paths to absolute
  const QString startMarkerPath = elmt.attribute( QStringLiteral( "startMarkerFile" ), QString() );
  const QString endMarkerPath = elmt.attribute( QStringLiteral( "endMarkerFile" ), QString() );
  setStartSvgMarkerPath( QgsSymbolLayerUtils::svgSymbolNameToPath( startMarkerPath, context.pathResolver() ) );
  setEndSvgMarkerPath( QgsSymbolLayerUtils::svgSymbolNameToPath( endMarkerPath, context.pathResolver() ) );
  mEndMarker = static_cast< QgsLayoutItemPolyline::MarkerMode >( elmt.attribute( QStringLiteral( "markerMode" ), QStringLiteral( "0" ) ).toInt() );
  mStartMarker = static_cast< QgsLayoutItemPolyline::MarkerMode >( elmt.attribute( QStringLiteral( "startMarkerMode" ), QStringLiteral( "0" ) ).toInt() );

  QgsLayoutNodesItem::readPropertiesFromElement( elmt, doc, context );

  updateBoundingRect();
  return true;
}

void QgsLayoutItemPolyline::updateBoundingRect()
{
  QRectF br = rect();

  double margin = std::max( mMaxSymbolBleed, computeMarkerMargin() );
  if ( mEndMarker == ArrowHead )
  {
    margin += 0.5 * mArrowHeadWidth;
  }
  br.adjust( -margin, -margin, margin, margin );
  mCurrentRectangle = br;

  // update
  prepareGeometryChange();
  update();
}


double QgsLayoutItemPolyline::computeMarkerMargin() const
{
  double margin = 0;

  if ( mStartMarker == ArrowHead || mEndMarker == ArrowHead )
  {
    margin = mArrowHeadStrokeWidth / 2.0 + mArrowHeadWidth * M_SQRT2;
  }

  if ( mStartMarker == SvgMarker )
  {
    const double startMarkerMargin = std::sqrt( 0.25 * ( mStartArrowHeadHeight * mStartArrowHeadHeight + mArrowHeadWidth * mArrowHeadWidth ) );
    margin = std::max( startMarkerMargin, margin );
  }

  if ( mEndMarker == SvgMarker )
  {
    const double endMarkerMargin = std::sqrt( 0.25 * ( mEndArrowHeadHeight * mEndArrowHeadHeight + mArrowHeadWidth * mArrowHeadWidth ) );
    margin = std::max( endMarkerMargin, margin );
  }

  return margin;
}
