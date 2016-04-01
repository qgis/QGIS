/***************************************************************************
                         qgscomposerpolyline.cpp
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

#include "qgscomposerpolyline.h"
#include "qgscomposition.h"
#include "qgscomposerutils.h"
#include "qgssymbollayerv2utils.h"
#include "qgssymbolv2.h"
#include <limits>

QgsComposerPolyline::QgsComposerPolyline( QgsComposition* c )
    : QgsComposerNodesItem( "ComposerPolyline", c )
    , mPolylineStyleSymbol( nullptr )
{
  createDefaultPolylineStyleSymbol();
}

QgsComposerPolyline::QgsComposerPolyline( QPolygonF polyline, QgsComposition* c )
    : QgsComposerNodesItem( "ComposerPolyline", polyline, c )
    , mPolylineStyleSymbol( nullptr )
{
  createDefaultPolylineStyleSymbol();
}

QgsComposerPolyline::~QgsComposerPolyline()
{
}

bool QgsComposerPolyline::_addNode( const int indexPoint,
                                    const QPointF &newPoint,
                                    const double radius )
{
  const double distStart = computeDistance( newPoint, mPolygon[0] );
  const double distEnd = computeDistance( newPoint, mPolygon[mPolygon.size()-1] );

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

bool QgsComposerPolyline::_removeNode( const int index )
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

void QgsComposerPolyline::createDefaultPolylineStyleSymbol()
{
  QgsStringMap properties;
  properties.insert( "color", "0,0,0,255" );
  properties.insert( "width", "0.3" );
  properties.insert( "capstyle", "square" );

  mPolylineStyleSymbol.reset( QgsLineSymbolV2::createSimple( properties ) );

  emit frameChanged();
}

QString QgsComposerPolyline::displayName() const
{
  if ( !id().isEmpty() )
    return id();

  return tr( "<polyline>" );
}

void QgsComposerPolyline::_draw( QPainter *painter )
{
  double dotsPerMM = painter->device()->logicalDpiX() / 25.4;

  QgsMapSettings ms = mComposition->mapSettings();
  ms.setOutputDpi( painter->device()->logicalDpiX() );

  QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
  context.setPainter( painter );
  context.setForceVectorOutput( true );

  QScopedPointer<QgsExpressionContext> expressionContext;
  expressionContext.reset( createExpressionContext() );
  context.setExpressionContext( *expressionContext.data() );

  painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots
  QTransform t = QTransform::fromScale( dotsPerMM, dotsPerMM );

  mPolylineStyleSymbol->startRender( context );
  mPolylineStyleSymbol->renderPolyline( t.map( mPolygon ), nullptr, context );
  mPolylineStyleSymbol->stopRender( context );
  painter->scale( dotsPerMM, dotsPerMM );
}

void QgsComposerPolyline::_readXMLStyle( const QDomElement &elmt )
{
  mPolylineStyleSymbol.reset( QgsSymbolLayerV2Utils::loadSymbol<QgsLineSymbolV2>( elmt ) );
}

void QgsComposerPolyline::setPolylineStyleSymbol( QgsLineSymbolV2* symbol )
{
  mPolylineStyleSymbol.reset( static_cast<QgsLineSymbolV2*>( symbol->clone() ) );
  update();
  emit frameChanged();
}

void QgsComposerPolyline::_writeXMLStyle( QDomDocument &doc, QDomElement &elmt ) const
{
  const QDomElement pe = QgsSymbolLayerV2Utils::saveSymbol( QString(),
                         mPolylineStyleSymbol.data(),
                         doc );
  elmt.appendChild( pe );
}
