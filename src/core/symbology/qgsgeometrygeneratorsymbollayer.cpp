/***************************************************************************
 qgsgeometrygeneratorsymbollayer.cpp
 ---------------------
 begin                : November 2015
 copyright            : (C) 2015 by Matthias Kuhn
 email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsgeometry.h"

QgsGeometryGeneratorSymbolLayer::~QgsGeometryGeneratorSymbolLayer() = default;

QgsSymbolLayer *QgsGeometryGeneratorSymbolLayer::create( const QgsStringMap &properties )
{
  QString expression = properties.value( QStringLiteral( "geometryModifier" ) );
  if ( expression.isEmpty() )
  {
    expression = QStringLiteral( "$geometry" );
  }
  QgsGeometryGeneratorSymbolLayer *symbolLayer = new QgsGeometryGeneratorSymbolLayer( expression );

  if ( properties.value( QStringLiteral( "SymbolType" ) ) == QLatin1String( "Marker" ) )
  {
    symbolLayer->setSubSymbol( QgsMarkerSymbol::createSimple( properties ) );
  }
  else if ( properties.value( QStringLiteral( "SymbolType" ) ) == QLatin1String( "Line" ) )
  {
    symbolLayer->setSubSymbol( QgsLineSymbol::createSimple( properties ) );
  }
  else
  {
    symbolLayer->setSubSymbol( QgsFillSymbol::createSimple( properties ) );
  }
  symbolLayer->restoreOldDataDefinedProperties( properties );

  return symbolLayer;
}

QgsGeometryGeneratorSymbolLayer::QgsGeometryGeneratorSymbolLayer( const QString &expression )
  : QgsSymbolLayer( QgsSymbol::Hybrid )
  , mExpression( new QgsExpression( expression ) )
  , mSymbolType( QgsSymbol::Marker )
{

}

QString QgsGeometryGeneratorSymbolLayer::layerType() const
{
  return QStringLiteral( "GeometryGenerator" );
}

void QgsGeometryGeneratorSymbolLayer::setSymbolType( QgsSymbol::SymbolType symbolType )
{
  if ( symbolType == QgsSymbol::Fill )
  {
    if ( !mFillSymbol )
      mFillSymbol.reset( QgsFillSymbol::createSimple( QgsStringMap() ) );
    mSymbol = mFillSymbol.get();
  }
  else if ( symbolType == QgsSymbol::Line )
  {
    if ( !mLineSymbol )
      mLineSymbol.reset( QgsLineSymbol::createSimple( QgsStringMap() ) );
    mSymbol = mLineSymbol.get();
  }
  else if ( symbolType == QgsSymbol::Marker )
  {
    if ( !mMarkerSymbol )
      mMarkerSymbol.reset( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
    mSymbol = mMarkerSymbol.get();
  }
  else
    Q_ASSERT( false );

  mSymbolType = symbolType;
}

void QgsGeometryGeneratorSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  mExpression->prepare( &context.renderContext().expressionContext() );

  subSymbol()->startRender( context.renderContext() );
}

void QgsGeometryGeneratorSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mSymbol )
    mSymbol->stopRender( context.renderContext() );
}

void QgsGeometryGeneratorSymbolLayer::startFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  mRenderingFeature = true;
  mHasRenderedFeature = false;
}

void QgsGeometryGeneratorSymbolLayer::stopFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  mRenderingFeature = false;
}

QgsSymbolLayer *QgsGeometryGeneratorSymbolLayer::clone() const
{
  QgsGeometryGeneratorSymbolLayer *clone = new QgsGeometryGeneratorSymbolLayer( mExpression->expression() );

  if ( mFillSymbol )
    clone->mFillSymbol.reset( mFillSymbol->clone() );
  if ( mLineSymbol )
    clone->mLineSymbol.reset( mLineSymbol->clone() );
  if ( mMarkerSymbol )
    clone->mMarkerSymbol.reset( mMarkerSymbol->clone() );

  clone->setSymbolType( mSymbolType );

  copyDataDefinedProperties( clone );
  copyPaintEffect( clone );

  return clone;
}

QgsStringMap QgsGeometryGeneratorSymbolLayer::properties() const
{
  QgsStringMap props;
  props.insert( QStringLiteral( "geometryModifier" ), mExpression->expression() );
  switch ( mSymbolType )
  {
    case QgsSymbol::Marker:
      props.insert( QStringLiteral( "SymbolType" ), QStringLiteral( "Marker" ) );
      break;
    case QgsSymbol::Line:
      props.insert( QStringLiteral( "SymbolType" ), QStringLiteral( "Line" ) );
      break;
    default:
      props.insert( QStringLiteral( "SymbolType" ), QStringLiteral( "Fill" ) );
      break;
  }
  return props;
}

void QgsGeometryGeneratorSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext &context, QSize size )
{
  if ( mSymbol )
    mSymbol->drawPreviewIcon( context.renderContext().painter(), size, nullptr, false, nullptr, context.patchShape() );
}

void QgsGeometryGeneratorSymbolLayer::setGeometryExpression( const QString &exp )
{
  mExpression.reset( new QgsExpression( exp ) );
}

bool QgsGeometryGeneratorSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  switch ( symbol->type() )
  {
    case QgsSymbol::Marker:
      mMarkerSymbol.reset( static_cast<QgsMarkerSymbol *>( symbol ) );
      break;

    case QgsSymbol::Line:
      mLineSymbol.reset( static_cast<QgsLineSymbol *>( symbol ) );
      break;

    case QgsSymbol::Fill:
      mFillSymbol.reset( static_cast<QgsFillSymbol *>( symbol ) );
      break;

    default:
      break;
  }

  setSymbolType( symbol->type() );

  return true;
}

QSet<QString> QgsGeometryGeneratorSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  return QgsSymbolLayer::usedAttributes( context )
         + mSymbol->usedAttributes( context )
         + mExpression->referencedColumns();
}

bool QgsGeometryGeneratorSymbolLayer::hasDataDefinedProperties() const
{
  // we treat geometry generator layers like they have data defined properties,
  // since the WHOLE layer is based on expressions and requires the full expression
  // context
  return true;
}

bool QgsGeometryGeneratorSymbolLayer::isCompatibleWithSymbol( QgsSymbol *symbol ) const
{
  Q_UNUSED( symbol )
  return true;
}
void QgsGeometryGeneratorSymbolLayer::render( QgsSymbolRenderContext &context )
{
  if ( mRenderingFeature && mHasRenderedFeature )
    return;

  if ( context.feature() )
  {
    QgsExpressionContext &expressionContext = context.renderContext().expressionContext();

    QgsFeature f = expressionContext.feature();
    QgsGeometry geom = mExpression->evaluate( &expressionContext ).value<QgsGeometry>();
    f.setGeometry( geom );

    QgsExpressionContextScope *subSymbolExpressionContextScope = mSymbol->symbolRenderContext()->expressionContextScope();

    subSymbolExpressionContextScope->setFeature( f );

    mSymbol->renderFeature( f, context.renderContext(), -1, context.selected() );

    if ( mRenderingFeature )
      mHasRenderedFeature = true;
  }
}

void QgsGeometryGeneratorSymbolLayer::setColor( const QColor &color )
{
  mSymbol->setColor( color );
}
