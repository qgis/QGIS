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
#include "qgssymbollayerv2utils.h"
#include "qgssymbolv2.h"
#include <limits>
#include <math.h>

QgsComposerNodesItem::QgsComposerNodesItem( QString tagName,
    QgsComposition* c )
    : QgsComposerItem( c )
    , mTagName( tagName )
    , mSelectedNode( -1 )
    , mDrawNodes( false )
{
}

QgsComposerNodesItem::QgsComposerNodesItem( QString tagName,
    QPolygonF polygon,
    QgsComposition* c )
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

QgsComposerNodesItem::~QgsComposerNodesItem()
{
}

double QgsComposerNodesItem::computeDistance( const QPointF &pt1,
    const QPointF &pt2 ) const
{
  return sqrt( pow( pt1.x() - pt2.x(), 2 ) + pow( pt1.y() - pt2.y(), 2 ) );
}

bool QgsComposerNodesItem::addNode( const QPointF &pt,
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
    if (( i + 1 ) != mPolygon.size() )
      pt2 = mPolygon.at( i + 1 );

    // compute line eq
    const double coef = ( pt2.y() - pt1.y() ) / ( pt2.x() - pt1.x() );
    const double b = pt1.y() - coef * pt1.x();

    double distance = std::numeric_limits<double>::max();
    if ( qIsInf( coef ) )
      distance = qAbs( pt1.x() - start.x() );
    else
    {
      const double coef2 = ( -1 / coef );
      const double b2 = start.y() - coef2 * start.x();

      QPointF inter;
      if ( qIsInf( coef2 ) )
      {
        distance = qAbs( pt1.y() - start.y() );
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

      if ( qAbs( length3 - length4 ) < std::numeric_limits<float>::epsilon() )
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
  properties.insert( "name", "cross" );
  properties.insert( "color_border", "red" );

  QScopedPointer<QgsMarkerSymbolV2> symbol;
  symbol.reset( QgsMarkerSymbolV2::createSimple( properties ) );
  symbol.data()->setSize( rectSize );
  symbol.data()->setAngle( 45 );

  QgsMapSettings ms = mComposition->mapSettings();
  ms.setOutputDpi( painter->device()->logicalDpiX() );

  QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
  context.setPainter( painter );
  context.setForceVectorOutput( true );

  QScopedPointer<QgsExpressionContext> expressionContext;
  expressionContext.reset( createExpressionContext() );
  context.setExpressionContext( *expressionContext.data() );

  symbol.data()->startRender( context );

  Q_FOREACH ( QPointF pt, mPolygon )
    symbol.data()->renderPoint( pt, nullptr, context );

  symbol.data()->stopRender( context );

  if ( mSelectedNode >= 0 && mSelectedNode < mPolygon.size() )
    drawSelectedNode( painter );
}

void QgsComposerNodesItem::drawSelectedNode( QPainter *painter ) const
{
  double rectSize = 3.0 / horizontalViewScaleFactor();

  QgsStringMap properties;
  properties.insert( "name", "square" );
  properties.insert( "color", "0, 0, 0, 0" );
  properties.insert( "color_border", "blue" );
  properties.insert( "width_border", "4" );

  QScopedPointer<QgsMarkerSymbolV2> symbol;
  symbol.reset( QgsMarkerSymbolV2::createSimple( properties ) );
  symbol.data()->setSize( rectSize );

  QgsMapSettings ms = mComposition->mapSettings();
  ms.setOutputDpi( painter->device()->logicalDpiX() );

  QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
  context.setPainter( painter );
  context.setForceVectorOutput( true );

  QScopedPointer<QgsExpressionContext> expressionContext;
  expressionContext.reset( createExpressionContext() );
  context.setExpressionContext( *expressionContext.data() );

  symbol.data()->startRender( context );
  symbol.data()->renderPoint( mPolygon.at( mSelectedNode ), nullptr, context );
  symbol.data()->stopRender( context );
}

void QgsComposerNodesItem::paint( QPainter* painter,
                                  const QStyleOptionGraphicsItem* itemStyle,
                                  QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );

  if ( !painter )
    return;

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

bool QgsComposerNodesItem::moveNode( const int index, const QPointF &pt )
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

bool QgsComposerNodesItem::readXML( const QDomElement& itemElem,
                                    const QDomDocument& doc )
{
  // restore general composer item properties
  const QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( !composerItemList.isEmpty() )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    if ( !qgsDoubleNear( composerItemElem.attribute( "rotation", "0" ).toDouble(), 0.0 ) )
      setItemRotation( composerItemElem.attribute( "rotation", "0" ).toDouble() );

    _readXML( composerItemElem, doc );
  }

  // restore style
  QDomElement styleSymbolElem = itemElem.firstChildElement( "symbol" );
  if ( !styleSymbolElem.isNull() )
    _readXMLStyle( styleSymbolElem );

  // restore nodes
  mPolygon.clear();
  QDomNodeList nodesList = itemElem.elementsByTagName( "node" );
  for ( int i = 0; i < nodesList.size(); i++ )
  {
    QDomElement nodeElem = nodesList.at( i ).toElement();
    QPointF newPt;
    newPt.setX( nodeElem.attribute( "x" ).toDouble() );
    newPt.setY( nodeElem.attribute( "y" ).toDouble() );
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
  const float ratioX =  rect().width() / boundingRect.width();
  const float ratioY =  rect().height() / boundingRect.height();

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

bool QgsComposerNodesItem::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerPolygonElem = doc.createElement( mTagName );

  // style
  _writeXMLStyle( doc, composerPolygonElem );

  // write nodes
  QDomElement nodesElem = doc.createElement( "nodes" );
  Q_FOREACH ( QPointF pt, mPolygon )
  {
    QDomElement nodeElem = doc.createElement( "node" );
    nodeElem.setAttribute( "x", QString::number( pt.x() ) );
    nodeElem.setAttribute( "y", QString::number( pt.y() ) );
    nodesElem.appendChild( nodeElem );
  }
  composerPolygonElem.appendChild( nodesElem );

  elem.appendChild( composerPolygonElem );

  return _writeXML( composerPolygonElem, doc );
}
