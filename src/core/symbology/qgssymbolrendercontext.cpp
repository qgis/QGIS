/***************************************************************************
 qgssymbolrendercontext.cpp
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbolrendercontext.h"
#include "qgsrendercontext.h"
#include "qgslegendpatchshape.h"

QgsSymbolRenderContext::QgsSymbolRenderContext( QgsRenderContext &c, Qgis::RenderUnit u, qreal opacity, bool selected, Qgis::SymbolRenderHints renderHints, const QgsFeature *f, const QgsFields &fields, const QgsMapUnitScale &mapUnitScale )
  : mRenderContext( c )
  , mOutputUnit( u )
  , mMapUnitScale( mapUnitScale )
  , mOpacity( opacity )
  , mSelected( selected )
  , mRenderHints( renderHints )
  , mFeature( f )
  , mFields( fields )
  , mGeometryPartCount( 0 )
  , mGeometryPartNum( 0 )
{
}

QgsSymbolRenderContext::~QgsSymbolRenderContext() = default;

void QgsSymbolRenderContext::setOriginalValueVariable( const QVariant &value )
{
  mRenderContext.expressionContext().setOriginalValueVariable( value );
}

bool QgsSymbolRenderContext::forceVectorRendering() const
{
  return mRenderContext.testFlag( Qgis::RenderContextFlag::ForceVectorOutput )
         || mRenderHints.testFlag( Qgis::SymbolRenderHint::ForceVectorRendering );
}

double QgsSymbolRenderContext::outputLineWidth( double width ) const
{
  return mRenderContext.convertToPainterUnits( width, mOutputUnit, mMapUnitScale );
}

double QgsSymbolRenderContext::outputPixelSize( double size ) const
{
  return mRenderContext.convertToPainterUnits( size, mOutputUnit, mMapUnitScale );
}

// cppcheck-suppress operatorEqVarError
QgsSymbolRenderContext &QgsSymbolRenderContext::operator=( const QgsSymbolRenderContext & )
{
  // This is just a dummy implementation of assignment.
  // sip 4.7 generates a piece of code that needs this function to exist.
  // It's not generated automatically by the compiler because of
  // mRenderContext member which is a reference (and thus can't be changed).
  Q_ASSERT( false );
  return *this;
}

QgsExpressionContextScope *QgsSymbolRenderContext::expressionContextScope()
{
  return mExpressionContextScope.get();
}

void QgsSymbolRenderContext::setExpressionContextScope( QgsExpressionContextScope *contextScope )
{
  mExpressionContextScope.reset( contextScope );
}

const QgsLegendPatchShape *QgsSymbolRenderContext::patchShape() const
{
  return mPatchShape.get();
}

void QgsSymbolRenderContext::setPatchShape( const QgsLegendPatchShape &patchShape )
{
  mPatchShape.reset( new QgsLegendPatchShape( patchShape ) );
}
