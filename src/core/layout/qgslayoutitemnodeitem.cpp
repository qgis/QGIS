/***************************************************************************
                         qgslayoutitemnodeitem.cpp
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

#include "qgslayoutitemnodeitem.h"

#include <cmath>
#include <limits>

#include "qgslayout.h"
#include "qgslayoutrendercontext.h"
#include "qgsmarkersymbol.h"
#include "qgssymbol.h"

#include <QStyleOptionGraphicsItem>

#include "moc_qgslayoutitemnodeitem.cpp"

void QgsLayoutNodesItem::setNodes( const QPolygonF &nodes )
{
  mPolygon = nodes;
  updateSceneRect();
  emit clipPathChanged();
}

QRectF QgsLayoutNodesItem::boundingRect() const
{
  return mCurrentRectangle;
}

double QgsLayoutNodesItem::estimatedFrameBleed() const
{
  return mMaxSymbolBleed;
}

QgsLayoutNodesItem::QgsLayoutNodesItem( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  init();
}

QgsLayoutNodesItem::QgsLayoutNodesItem( const QPolygonF &polygon,
                                        QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  init();

  const QRectF boundingRect = polygon.boundingRect();
  attemptSetSceneRect( boundingRect );

  const QPointF topLeft = boundingRect.topLeft();
  mPolygon = polygon.translated( -topLeft );
}

void QgsLayoutNodesItem::init()
{
  // no cache - the node based items cannot reliably determine their real bounds (e.g. due to mitred corners).
  // this blocks use of the pixmap based cache for these
  setCacheMode( QGraphicsItem::NoCache );
  setBackgroundEnabled( false );
  setFrameEnabled( false );

  connect( this, &QgsLayoutNodesItem::sizePositionChanged, this, &QgsLayoutNodesItem::updateBoundingRect );
}

void QgsLayoutNodesItem::draw( QgsLayoutItemRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();
  painter->setPen( Qt::NoPen );
  painter->setBrush( Qt::NoBrush );

  if ( context.renderContext().rasterizedRenderingPolicy() == Qgis::RasterizedRenderingPolicy::Default )
    context.renderContext().setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );

  rescaleToFitBoundingBox();
  _draw( context );

  if ( mDrawNodes && layout()->renderContext().isPreviewRender() )
    drawNodes( context );
}

QgsLayoutItem::Flags QgsLayoutNodesItem::itemFlags() const
{
  return QgsLayoutItem::FlagDisableSceneCaching;
}

double QgsLayoutNodesItem::computeDistance( QPointF pt1,
    QPointF pt2 ) const
{
  return std::sqrt( std::pow( pt1.x() - pt2.x(), 2 ) + std::pow( pt1.y() - pt2.y(), 2 ) );
}

bool QgsLayoutNodesItem::addNode( QPointF pt,
                                  const bool checkArea,
                                  const double radius )
{
  const QPointF start = mapFromScene( pt );
  double minDistance = std::numeric_limits<double>::max();
  const double maxDistance = ( checkArea ) ? radius : minDistance;
  bool rc = false;
  int idx = -1;

  for ( int i = 0; i != mPolygon.size(); i++ )
  {
    // get nodes of polyline
    const QPointF pt1 = mPolygon.at( i );
    QPointF pt2 = mPolygon.at( 0 );
    if ( ( i + 1 ) != mPolygon.size() )
      pt2 = mPolygon.at( i + 1 );

    // compute line eq
    const double coef = ( pt2.y() - pt1.y() ) / ( pt2.x() - pt1.x() );
    const double b = pt1.y() - coef * pt1.x();

    double distance = std::numeric_limits<double>::max();
    if ( std::isinf( coef ) )
      distance = std::fabs( pt1.x() - start.x() );
    else
    {
      const double coef2 = ( -1 / coef );
      const double b2 = start.y() - coef2 * start.x();

      QPointF inter;
      if ( std::isinf( coef2 ) )
      {
        distance = std::fabs( pt1.y() - start.y() );
        inter.setX( start.x() );
        inter.setY( pt1.y() );
      }
      else
      {
        const double interx = ( b - b2 ) / ( coef2 - coef );
        const double intery = interx * coef2 + b2;
        inter.setX( interx );
        inter.setY( intery );
      }

      // check if intersection is within the line
      const double length1 = computeDistance( inter, pt1 );
      const double length2 = computeDistance( inter, pt2 );
      const double length3 = computeDistance( pt1, pt2 );
      const double length4 = length1 + length2;

      if ( std::fabs( length3 - length4 ) < std::numeric_limits<float>::epsilon() )
        distance = computeDistance( inter, start );
    }

    if ( distance < minDistance && distance < maxDistance )
    {
      minDistance = distance;
      idx = i;
    }
  }

  if ( idx >= 0 )
  {
    rc = _addNode( idx, start, maxDistance );
    updateSceneRect();
    emit clipPathChanged();
  }

  return rc;
}

void QgsLayoutNodesItem::drawNodes( QgsLayoutItemRenderContext &context ) const
{
  context.renderContext().painter()->setRenderHint( QPainter::Antialiasing, false );

  const double rectSize = 9.0 / context.viewScaleFactor();

  QVariantMap properties;
  properties.insert( u"name"_s, u"cross"_s );
  properties.insert( u"color_border"_s, u"red"_s );

  std::unique_ptr<QgsMarkerSymbol> symbol = QgsMarkerSymbol::createSimple( properties );
  symbol->setSize( rectSize );
  symbol->setAngle( 45 );

  symbol->startRender( context.renderContext() );
  for ( const QPointF pt : std::as_const( mPolygon ) )
    symbol->renderPoint( pt * context.viewScaleFactor(), nullptr, context.renderContext() );
  symbol->stopRender( context.renderContext() );

  if ( mSelectedNode >= 0 && mSelectedNode < mPolygon.size() )
    drawSelectedNode( context );
}

void QgsLayoutNodesItem::drawSelectedNode( QgsLayoutItemRenderContext &context ) const
{
  const double rectSize = 9.0 / context.viewScaleFactor();

  QVariantMap properties;
  properties.insert( u"name"_s, u"square"_s );
  properties.insert( u"color"_s, u"0, 0, 0, 0"_s );
  properties.insert( u"color_border"_s, u"blue"_s );
  properties.insert( u"width_border"_s, u"4"_s );

  std::unique_ptr<QgsMarkerSymbol> symbol = QgsMarkerSymbol::createSimple( properties );
  symbol->setSize( rectSize );

  symbol->startRender( context.renderContext() );
  symbol->renderPoint( mPolygon.at( mSelectedNode ) * context.viewScaleFactor(), nullptr, context.renderContext() );
  symbol->stopRender( context.renderContext() );
}

int QgsLayoutNodesItem::nodeAtPosition( QPointF node,
                                        const bool searchInRadius,
                                        const double radius ) const
{
  const QPointF pt = mapFromScene( node );
  double nearestDistance = std::numeric_limits<double>::max();
  const double maxDistance = ( searchInRadius ) ? radius : nearestDistance;
  double distance = 0;
  int idx = -1;

  int i = 0;
  for ( const QPointF polyPt : std::as_const( mPolygon ) )
  {
    distance = computeDistance( pt, polyPt );
    if ( distance < nearestDistance && distance < maxDistance )
    {
      nearestDistance = distance;
      idx = i;
    }
    i++;
  }

  return idx;
}

bool QgsLayoutNodesItem::nodePosition( const int index, QPointF &position ) const
{
  bool rc( false );

  if ( index >= 0 && index < mPolygon.size() )
  {
    position = mapToScene( mPolygon.at( index ) );
    rc = true;
  }

  return rc;
}

bool QgsLayoutNodesItem::removeNode( const int index )
{
  const bool rc = _removeNode( index );
  if ( rc )
  {
    updateSceneRect();
    emit clipPathChanged();
  }
  return rc;
}

bool QgsLayoutNodesItem::moveNode( const int index, QPointF pt )
{
  bool rc( false );

  if ( index >= 0 && index < mPolygon.size() )
  {
    const QPointF nodeItem = mapFromScene( pt );
    mPolygon.replace( index, nodeItem );
    updateSceneRect();
    emit clipPathChanged();
    rc = true;
  }

  return rc;
}

bool QgsLayoutNodesItem::readPropertiesFromElement( const QDomElement &itemElem,
    const QDomDocument &, const QgsReadWriteContext &context )
{
  // restore style
  const QDomElement styleSymbolElem = itemElem.firstChildElement( u"symbol"_s );
  if ( !styleSymbolElem.isNull() )
    _readXmlStyle( styleSymbolElem, context );

  // restore nodes
  mPolygon.clear();
  const QDomNodeList nodesList = itemElem.elementsByTagName( u"node"_s );
  for ( int i = 0; i < nodesList.size(); i++ )
  {
    const QDomElement nodeElem = nodesList.at( i ).toElement();
    QPointF newPt;
    newPt.setX( nodeElem.attribute( u"x"_s ).toDouble() );
    newPt.setY( nodeElem.attribute( u"y"_s ).toDouble() );
    mPolygon.append( newPt );
  }

  emit changed();
  emit clipPathChanged();
  return true;
}

void QgsLayoutNodesItem::rescaleToFitBoundingBox()
{
  // get the bounding rect for the polygon currently displayed
  const QRectF boundingRect = mPolygon.boundingRect();

  // compute x/y ratio
  const float ratioX = !qgsDoubleNear( boundingRect.width(), 0.0 )
                       ? rect().width() / boundingRect.width() : 0;
  const float ratioY = !qgsDoubleNear( boundingRect.height(), 0.0 )
                       ? rect().height() / boundingRect.height() : 0;

  // scaling
  QTransform trans;
  trans = trans.scale( ratioX, ratioY );
  mPolygon = trans.map( mPolygon );
  emit clipPathChanged();
}

bool QgsLayoutNodesItem::setSelectedNode( const int index )
{
  bool rc = false;

  if ( index >= 0 && index < mPolygon.size() )
  {
    mSelectedNode = index;
    rc = true;
  }

  return rc;
}

void QgsLayoutNodesItem::updateSceneRect()
{
  // set the new scene rectangle
  const QRectF br = mPolygon.boundingRect();

  const QPointF topLeft = mapToScene( br.topLeft() );
  //will trigger updateBoundingRect if necessary
  attemptSetSceneRect( QRectF( topLeft.x(), topLeft.y(), br.width(), br.height() ) );

  // update polygon position
  mPolygon.translate( -br.topLeft().x(), -br.topLeft().y() );
}

void QgsLayoutNodesItem::updateBoundingRect()
{
  QRectF br = rect();
  br.adjust( -mMaxSymbolBleed, -mMaxSymbolBleed, mMaxSymbolBleed, mMaxSymbolBleed );
  prepareGeometryChange();
  mCurrentRectangle = br;

  // update
  update();
}

bool QgsLayoutNodesItem::writePropertiesToElement( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  // style
  _writeXmlStyle( doc, elem, context );

  // write nodes
  QDomElement nodesElem = doc.createElement( u"nodes"_s );
  for ( const QPointF pt : std::as_const( mPolygon ) )
  {
    QDomElement nodeElem = doc.createElement( u"node"_s );
    nodeElem.setAttribute( u"x"_s, QString::number( pt.x() ) );
    nodeElem.setAttribute( u"y"_s, QString::number( pt.y() ) );
    nodesElem.appendChild( nodeElem );
  }
  elem.appendChild( nodesElem );

  return true;
}
