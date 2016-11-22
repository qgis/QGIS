/***************************************************************************
                         qgscomposerpolygon.cpp
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

#include "qgscomposerpolygon.h"
#include "qgscomposition.h"
#include "qgscomposerutils.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgsmapsettings.h"
#include <limits>

QgsComposerPolygon::QgsComposerPolygon( QgsComposition* c )
  : QgsComposerNodesItem( QStringLiteral( "ComposerPolygon" ), c )
  , mPolygonStyleSymbol( nullptr )
{
  createDefaultPolygonStyleSymbol();
}

QgsComposerPolygon::QgsComposerPolygon( const QPolygonF& polygon, QgsComposition* c )
  : QgsComposerNodesItem( QStringLiteral( "ComposerPolygon" ), polygon, c )
  , mPolygonStyleSymbol( nullptr )
{
  createDefaultPolygonStyleSymbol();
}

QgsComposerPolygon::~QgsComposerPolygon()
{
}

bool QgsComposerPolygon::_addNode( const int indexPoint,
                                   QPointF newPoint,
                                   const double radius )
{
  Q_UNUSED( radius );
  mPolygon.insert( indexPoint + 1, newPoint );
  return true;
}

void QgsComposerPolygon::createDefaultPolygonStyleSymbol()
{
  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );

  mPolygonStyleSymbol.reset( QgsFillSymbol::createSimple( properties ) );

  emit frameChanged();
}

QString QgsComposerPolygon::displayName() const
{
  if ( !id().isEmpty() )
    return id();

  return tr( "<polygon>" );
}

void QgsComposerPolygon::_draw( QPainter* painter )
{
  //setup painter scaling to dots so that raster symbology is drawn to scale
  const double dotsPerMM = painter->device()->logicalDpiX() / 25.4;

  QgsMapSettings ms = mComposition->mapSettings();
  ms.setOutputDpi( painter->device()->logicalDpiX() );

  QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
  context.setPainter( painter );
  context.setForceVectorOutput( true );

  context.setExpressionContext( createExpressionContext() );

  painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots
  QTransform t = QTransform::fromScale( dotsPerMM, dotsPerMM );

  QList<QPolygonF> rings; //empty
  QPainterPath polygonPath;
  polygonPath.addPolygon( mPolygon );

  mPolygonStyleSymbol->startRender( context );
  mPolygonStyleSymbol->renderPolygon( polygonPath.toFillPolygon( t ), &rings,
                                      nullptr, context );
  mPolygonStyleSymbol->stopRender( context );
  painter->scale( dotsPerMM, dotsPerMM );
}

void QgsComposerPolygon::_readXmlStyle( const QDomElement& elmt )
{
  mPolygonStyleSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( elmt ) );
}

void QgsComposerPolygon::setPolygonStyleSymbol( QgsFillSymbol* symbol )
{
  mPolygonStyleSymbol.reset( static_cast<QgsFillSymbol*>( symbol->clone() ) );
  update();
  emit frameChanged();
}

void QgsComposerPolygon::_writeXmlStyle( QDomDocument& doc, QDomElement& elmt ) const
{
  const QDomElement pe = QgsSymbolLayerUtils::saveSymbol( QString(),
                         mPolygonStyleSymbol.data(),
                         doc );
  elmt.appendChild( pe );
}

bool QgsComposerPolygon::_removeNode( const int index )
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
