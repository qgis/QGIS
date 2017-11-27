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
#include <limits>

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

QgsLayoutItemPolyline *QgsLayoutItemPolyline::create( QgsLayout *layout )
{
  return new QgsLayoutItemPolyline( layout );
}

int QgsLayoutItemPolyline::type() const
{
  return QgsLayoutItemRegistry::LayoutPolyline;
}

QString QgsLayoutItemPolyline::stringType() const
{
  return QStringLiteral( "ItemPolyline" );
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
  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "square" ) );

  mPolylineStyleSymbol.reset( QgsLineSymbol::createSimple( properties ) );
  refreshSymbol();
}

void QgsLayoutItemPolyline::refreshSymbol()
{
  if ( layout() )
  {
    QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( layout(), nullptr, layout()->context().dpi() );
    mMaxSymbolBleed = ( 25.4 / layout()->context().dpi() ) * QgsSymbolLayerUtils::estimateMaxSymbolBleed( mPolylineStyleSymbol.get(), rc );
  }

  updateSceneRect();

  emit frameChanged();
}

QString QgsLayoutItemPolyline::displayName() const
{
  if ( !id().isEmpty() )
    return id();

  return tr( "<Polyline>" );
}

void QgsLayoutItemPolyline::_draw( QgsRenderContext &context, const QStyleOptionGraphicsItem * )
{
  //setup painter scaling to dots so that raster symbology is drawn to scale
  double scale = context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
  QTransform t = QTransform::fromScale( scale, scale );

  mPolylineStyleSymbol->startRender( context );
  mPolylineStyleSymbol->renderPolyline( t.map( mPolygon ), nullptr, context );
  mPolylineStyleSymbol->stopRender( context );
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

void QgsLayoutItemPolyline::_writeXmlStyle( QDomDocument &doc, QDomElement &elmt, const QgsReadWriteContext &context ) const
{
  const QDomElement pe = QgsSymbolLayerUtils::saveSymbol( QString(),
                         mPolylineStyleSymbol.get(),
                         doc,
                         context );
  elmt.appendChild( pe );
}
