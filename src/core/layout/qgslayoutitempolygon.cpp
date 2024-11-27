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
#include "moc_qgslayoutitempolygon.cpp"
#include "qgslayoutitemregistry.h"
#include "qgslayoututils.h"
#include "qgslayout.h"
#include "qgslayoutrendercontext.h"
#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgsstyleentityvisitor.h"
#include "qgsfillsymbol.h"

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

QgsLayoutItemPolygon::~QgsLayoutItemPolygon() = default;

QgsLayoutItemPolygon *QgsLayoutItemPolygon::create( QgsLayout *layout )
{
  return new QgsLayoutItemPolygon( layout );
}

int QgsLayoutItemPolygon::type() const
{
  return QgsLayoutItemRegistry::LayoutPolygon;
}

QIcon QgsLayoutItemPolygon::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemPolygon.svg" ) );
}

bool QgsLayoutItemPolygon::_addNode( const int indexPoint,
                                     QPointF newPoint,
                                     const double radius )
{
  Q_UNUSED( radius )
  mPolygon.insert( indexPoint + 1, newPoint );
  return true;
}

void QgsLayoutItemPolygon::createDefaultPolygonStyleSymbol()
{
  QVariantMap properties;
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
  if ( auto *lLayout = layout() )
  {
    const QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( lLayout, nullptr, lLayout->renderContext().dpi() );
    mMaxSymbolBleed = ( 25.4 / lLayout->renderContext().dpi() ) * QgsSymbolLayerUtils::estimateMaxSymbolBleed( mPolygonStyleSymbol.get(), rc );
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

bool QgsLayoutItemPolygon::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mPolygonStyleSymbol )
  {
    QgsStyleSymbolEntity entity( mPolygonStyleSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, uuid(), displayName() ) ) )
      return false;
  }

  return true;
}

QgsLayoutItem::Flags QgsLayoutItemPolygon::itemFlags() const
{
  QgsLayoutItem::Flags flags = QgsLayoutNodesItem::itemFlags();
  flags |= QgsLayoutItem::FlagProvidesClipPath;
  return flags;
}

QgsGeometry QgsLayoutItemPolygon::clipPath() const
{
  QPolygonF path = mapToScene( mPolygon );
  // ensure polygon is closed
  if ( path.at( 0 ) != path.constLast() )
    path << path.at( 0 );
  return QgsGeometry::fromQPolygonF( path );
}


bool QgsLayoutItemPolygon::isValid() const
{
  // A Polygon is valid if it has at least 3 unique points
  QList<QPointF> uniquePoints;
  int seen = 0;
  for ( QPointF point : mPolygon )
  {
    if ( !uniquePoints.contains( point ) )
    {
      uniquePoints.append( point );
      if ( ++seen > 2 )
        return true;
    }
  }
  return false;
}

QgsFillSymbol *QgsLayoutItemPolygon::symbol()
{
  return mPolygonStyleSymbol.get();
}

void QgsLayoutItemPolygon::_draw( QgsLayoutItemRenderContext &context, const QStyleOptionGraphicsItem * )
{
  QgsRenderContext renderContext = context.renderContext();
  // symbol clipping messes with geometry generators used in the symbol for this item, and has no
  // valid use here. See https://github.com/qgis/QGIS/issues/58909
  renderContext.setFlag( Qgis::RenderContextFlag::DisableSymbolClippingToExtent );

  //setup painter scaling to dots so that raster symbology is drawn to scale
  const double scale = renderContext.convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );
  const QTransform t = QTransform::fromScale( scale, scale );

  const QVector<QPolygonF> rings; //empty
  QPainterPath polygonPath;
  polygonPath.addPolygon( mPolygon );

  mPolygonStyleSymbol->startRender( renderContext );
  mPolygonStyleSymbol->renderPolygon( polygonPath.toFillPolygon( t ), &rings,
                                      nullptr, renderContext );
  mPolygonStyleSymbol->stopRender( renderContext );
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
  if ( index < 0 || index >= mPolygon.size() || mPolygon.size() <= 3 )
    return false;

  mPolygon.remove( index );

  int newSelectNode = index;
  if ( index == mPolygon.size() )
    newSelectNode = 0;
  setSelectedNode( newSelectNode );

  return true;
}

