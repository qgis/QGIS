/***************************************************************************
                         qgslayoutitempolygon.cpp
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

#include "qgslayoutitempolygon.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoututils.h"
#include "qgslayout.h"
#include "qgspathresolver.h"
#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgsmapsettings.h"
#include <limits>

QgsLayoutItemPolygon::QgsLayoutItemPolygon( QgsLayout *layout )
  : QgsLayoutNodesItem( layout )
{
  createDefaultPolygonStyleSymbol();
}

QgsLayoutItemPolygon::QgsLayoutItemPolygon( const QPolygonF &polygon, QgsLayout *layout )
  : QgsLayoutNodesItem( polygon, layout )
{
  createDefaultPolygonStyleSymbol();
}

QgsLayoutItemPolygon *QgsLayoutItemPolygon::create( QgsLayout *layout )
{
  return new QgsLayoutItemPolygon( layout );
}

QString QgsLayoutItemPolygon::stringType() const
{
  return QStringLiteral( "ItemPolygon" );
}

int QgsLayoutItemPolygon::type() const
{
  return QgsLayoutItemRegistry::LayoutPolygon;
}

bool QgsLayoutItemPolygon::_addNode( const int indexPoint,
                                     QPointF newPoint,
                                     const double radius )
{
  Q_UNUSED( radius );
  mPolygon.insert( indexPoint + 1, newPoint );
  return true;
}

void QgsLayoutItemPolygon::createDefaultPolygonStyleSymbol()
{
  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );

  mPolygonStyleSymbol.reset( QgsFillSymbol::createSimple( properties ) );

  refreshSymbol();
}

void QgsLayoutItemPolygon::refreshSymbol()
{
  if ( layout() )
  {
    QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( layout(), nullptr, layout()->context().dpi() );
    mMaxSymbolBleed = ( 25.4 / layout()->context().dpi() ) * QgsSymbolLayerUtils::estimateMaxSymbolBleed( mPolygonStyleSymbol.get(), rc );
  }

  updateSceneRect();

  emit frameChanged();
}

QString QgsLayoutItemPolygon::displayName() const
{
  if ( !id().isEmpty() )
    return id();

  return tr( "<Polygon>" );
}

void QgsLayoutItemPolygon::_draw( QgsRenderContext &context, const QStyleOptionGraphicsItem * )
{
  //setup painter scaling to dots so that raster symbology is drawn to scale
  double scale = context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
  QTransform t = QTransform::fromScale( scale, scale );

  QList<QPolygonF> rings; //empty
  QPainterPath polygonPath;
  polygonPath.addPolygon( mPolygon );

  mPolygonStyleSymbol->startRender( context );
  mPolygonStyleSymbol->renderPolygon( polygonPath.toFillPolygon( t ), &rings,
                                      nullptr, context );
  mPolygonStyleSymbol->stopRender( context );
}

void QgsLayoutItemPolygon::_readXmlStyle( const QDomElement &elmt, const QgsReadWriteContext &context )
{
  mPolygonStyleSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( elmt, context ) );
}

void QgsLayoutItemPolygon::setSymbol( QgsFillSymbol *symbol )
{
  mPolygonStyleSymbol.reset( static_cast<QgsFillSymbol *>( symbol->clone() ) );
  refreshSymbol();
}

void QgsLayoutItemPolygon::_writeXmlStyle( QDomDocument &doc, QDomElement &elmt, const QgsReadWriteContext &context ) const
{
  const QDomElement pe = QgsSymbolLayerUtils::saveSymbol( QString(),
                         mPolygonStyleSymbol.get(),
                         doc,
                         context );
  elmt.appendChild( pe );
}

bool QgsLayoutItemPolygon::_removeNode( const int index )
{
  if ( index < 0 || index >= mPolygon.size() )
    return false;

  mPolygon.remove( index );

  if ( mPolygon.size() < 3 )
    mPolygon.clear();
  else
  {
    int newSelectNode = index;
    if ( index == mPolygon.size() )
      newSelectNode = 0;
    setSelectedNode( newSelectNode );
  }

  return true;
}
