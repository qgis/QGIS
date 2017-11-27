/***************************************************************************
                         qgscomposernodesitem.cpp
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

#include "qgscomposernodesitem.h"
#include "qgscomposition.h"
#include "qgscomposerutils.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgsmapsettings.h"
#include <limits>
#include <cmath>

QgsComposerNodesItem::QgsComposerNodesItem( const QString &tagName,
    QgsComposition *c )
  : QgsComposerItem( c )
  , mTagName( tagName )
  , mSelectedNode( -1 )
  , mDrawNodes( false )
{
}

QgsComposerNodesItem::QgsComposerNodesItem( const QString &tagName,
    const QPolygonF &polygon,
    QgsComposition *c )
  : QgsComposerItem( c )
  , mTagName( tagName )
  , mSelectedNode( -1 )
  , mDrawNodes( false )
{
  const QRectF boundingRect = polygon.boundingRect();
  setSceneRect( boundingRect );

  const QPointF topLeft = boundingRect.topLeft();
  mPolygon = polygon.translated( -topLeft );
}

double QgsComposerNodesItem::computeDistance( QPointF pt1,
    QPointF pt2 ) const
{
  return std::sqrt( std::pow( pt1.x() - pt2.x(), 2 ) + std::pow( pt1.y() - pt2.y(), 2 ) );
}

bool QgsComposerNodesItem::addNode( QPointF pt,
                                    const bool checkArea,
                                    const double radius )
{
  const QPointF start = mapFromScene( pt );
  double minDistance = std::numeric_limits<double>::max();
  double maxDistance = ( checkArea ) ? radius : minDistance;
  bool rc = false;
  int idx = -1;

  for ( int i = 0; i != mPolygon.size(); i++ )
  {
    // get nodes of polyline
    const QPointF pt1 = mPolygon.at( i );
    QPointF pt2 = mPolygon.first();
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
  }

  return rc;
}

void QgsComposerNodesItem::drawNodes( QPainter *painter ) const
{
  double rectSize = 3.0 / horizontalViewScaleFactor();

  QgsStringMap properties;
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "cross" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "red" ) );

  std::unique_ptr<QgsMarkerSymbol> symbol;
  symbol.reset( QgsMarkerSymbol::createSimple( properties ) );
  symbol->setSize( rectSize );
  symbol->setAngle( 45 );

  QgsRenderContext context = QgsComposerUtils::createRenderContextForComposition( mComposition, painter );
  context.setForceVectorOutput( true );

  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  symbol->startRender( context );

  Q_FOREACH ( QPointF pt, mPolygon )
    symbol->renderPoint( pt, nullptr, context );

  symbol->stopRender( context );

  if ( mSelectedNode >= 0 && mSelectedNode < mPolygon.size() )
    drawSelectedNode( painter );
}

void QgsComposerNodesItem::drawSelectedNode( QPainter *painter ) const
{
  double rectSize = 3.0 / horizontalViewScaleFactor();

  QgsStringMap properties;
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "square" ) );
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0, 0, 0, 0" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "blue" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "4" ) );

  std::unique_ptr<QgsMarkerSymbol> symbol;
  symbol.reset( QgsMarkerSymbol::createSimple( properties ) );
  symbol->setSize( rectSize );

  QgsRenderContext context = QgsComposerUtils::createRenderContextForComposition( mComposition, painter );
  context.setForceVectorOutput( true );

  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  symbol->startRender( context );
  symbol->renderPoint( mPolygon.at( mSelectedNode ), nullptr, context );
  symbol->stopRender( context );
}

void QgsComposerNodesItem::paint( QPainter *painter,
                                  const QStyleOptionGraphicsItem *itemStyle,
                                  QWidget *pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );

  if ( !painter )
    return;

  if ( !shouldDrawItem() )
  {
    return;
  }

  painter->save();
  painter->setPen( Qt::NoPen );
  painter->setBrush( Qt::NoBrush );
  painter->setRenderHint( QPainter::Antialiasing, true );

  rescaleToFitBoundingBox();
  _draw( painter );

  if ( mDrawNodes && composition()->plotStyle() == QgsComposition::Preview )
    drawNodes( painter );

  painter->restore();
}

int QgsComposerNodesItem::nodeAtPosition( QPointF node,
    const bool searchInRadius,
    const double radius )
{
  const QPointF pt = mapFromScene( node );
  double nearestDistance = std::numeric_limits<double>::max();
  double maxDistance = ( searchInRadius ) ? radius : nearestDistance;
  double distance = 0;
  int idx = -1;

  QVector<QPointF>::const_iterator it = mPolygon.constBegin();
  for ( ; it != mPolygon.constEnd(); ++it )
  {
    distance = computeDistance( pt, *it );
    if ( distance < nearestDistance && distance < maxDistance )
    {
      nearestDistance = distance;
      idx = it - mPolygon.constBegin();
    }
  }

  return idx;
}

bool QgsComposerNodesItem::nodePosition( const int index, QPointF &position )
{
  bool rc( false );

  if ( index >= 0 && index < mPolygon.size() )
  {
    position = mapToScene( mPolygon.at( index ) );
    rc = true;
  }

  return rc;
}

bool QgsComposerNodesItem::removeNode( const int index )
{
  bool rc = _removeNode( index );
  if ( rc )
    updateSceneRect();
  return rc;
}

bool QgsComposerNodesItem::moveNode( const int index, QPointF pt )
{
  bool rc( false );

  if ( index >= 0 && index < mPolygon.size() )
  {
    QPointF nodeItem = mapFromScene( pt );
    mPolygon.replace( index, nodeItem );
    updateSceneRect();

    rc = true;
  }

  return rc;
}

bool QgsComposerNodesItem::readXml( const QDomElement &itemElem,
                                    const QDomDocument &doc )
{

  // restore general composer item properties
  const QDomNodeList composerItemList = itemElem.elementsByTagName( QStringLiteral( "ComposerItem" ) );
  if ( !composerItemList.isEmpty() )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    if ( !qgsDoubleNear( composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble(), 0.0 ) )
      setItemRotation( composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble() );

    _readXml( composerItemElem, doc );
  }

  // restore style
  QDomElement styleSymbolElem = itemElem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !styleSymbolElem.isNull() )
    _readXmlStyle( styleSymbolElem );

  // restore nodes
  mPolygon.clear();
  QDomNodeList nodesList = itemElem.elementsByTagName( QStringLiteral( "node" ) );
  for ( int i = 0; i < nodesList.size(); i++ )
  {
    QDomElement nodeElem = nodesList.at( i ).toElement();
    QPointF newPt;
    newPt.setX( nodeElem.attribute( QStringLiteral( "x" ) ).toDouble() );
    newPt.setY( nodeElem.attribute( QStringLiteral( "y" ) ).toDouble() );
    mPolygon.append( newPt );
  }

  emit itemChanged();
  return true;
}

void QgsComposerNodesItem::rescaleToFitBoundingBox()
{
  // get the bounding rect for the polygon currently displayed
  const QRectF boundingRect = mPolygon.boundingRect();

  // compute x/y ratio
  const float ratioX = rect().width() / boundingRect.width();
  const float ratioY = rect().height() / boundingRect.height();

  // scaling
  QTransform trans;
  trans = trans.scale( ratioX, ratioY );
  mPolygon = trans.map( mPolygon );
}

bool QgsComposerNodesItem::setSelectedNode( const int index )
{
  bool rc = false;

  if ( index >= 0 && index < mPolygon.size() )
  {
    mSelectedNode = index;
    rc = true;
  }

  return rc;
}

void QgsComposerNodesItem::updateSceneRect()
{
  // set the new scene rectangle
  const QRectF br = mPolygon.boundingRect();

  const QPointF topLeft = mapToScene( br.topLeft() );
  setSceneRect( QRectF( topLeft.x(), topLeft.y(), br.width(), br.height() ) );

  // update polygon position
  mPolygon.translate( -br.topLeft().x(), -br.topLeft().y() );

  // update
  prepareGeometryChange();
  update();
  emit itemChanged();
}

bool QgsComposerNodesItem::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  QDomElement composerPolygonElem = doc.createElement( mTagName );

  // style
  _writeXmlStyle( doc, composerPolygonElem );

  // write nodes
  QDomElement nodesElem = doc.createElement( QStringLiteral( "nodes" ) );
  Q_FOREACH ( QPointF pt, mPolygon )
  {
    QDomElement nodeElem = doc.createElement( QStringLiteral( "node" ) );
    nodeElem.setAttribute( QStringLiteral( "x" ), QString::number( pt.x() ) );
    nodeElem.setAttribute( QStringLiteral( "y" ), QString::number( pt.y() ) );
    nodesElem.appendChild( nodeElem );
  }
  composerPolygonElem.appendChild( nodesElem );

  elem.appendChild( composerPolygonElem );

  return _writeXml( composerPolygonElem, doc );
}
