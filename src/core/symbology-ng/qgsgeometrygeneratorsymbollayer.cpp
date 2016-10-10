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

QgsGeometryGeneratorSymbolLayer::~QgsGeometryGeneratorSymbolLayer()
{
  delete mMarkerSymbol;
  delete mLineSymbol;
  delete mFillSymbol;
}

QgsSymbolLayer* QgsGeometryGeneratorSymbolLayer::create( const QgsStringMap& properties )
{
  QString expression = properties.value( "geometryModifier" );
  if ( expression.isEmpty() )
  {
    expression = "$geometry";
  }
  QgsGeometryGeneratorSymbolLayer* symbolLayer = new QgsGeometryGeneratorSymbolLayer( expression );

  if ( properties.value( "SymbolType" ) == "Marker" )
  {
    symbolLayer->setSubSymbol( QgsMarkerSymbol::createSimple( properties ) );
  }
  else if ( properties.value( "SymbolType" ) == "Line" )
  {
    symbolLayer->setSubSymbol( QgsLineSymbol::createSimple( properties ) );
  }
  else
  {
    symbolLayer->setSubSymbol( QgsFillSymbol::createSimple( properties ) );
  }

  return symbolLayer;
}

QgsGeometryGeneratorSymbolLayer::QgsGeometryGeneratorSymbolLayer( const QString& expression )
    : QgsSymbolLayer( QgsSymbol::Hybrid )
    , mExpression( new QgsExpression( expression ) )
    , mFillSymbol( nullptr )
    , mLineSymbol( nullptr )
    , mMarkerSymbol( nullptr )
    , mSymbol( nullptr )
    , mSymbolType( QgsSymbol::Marker )
{

}

QString QgsGeometryGeneratorSymbolLayer::layerType() const
{
  return "GeometryGenerator";
}

void QgsGeometryGeneratorSymbolLayer::setSymbolType( QgsSymbol::SymbolType symbolType )
{
  if ( symbolType == QgsSymbol::Fill )
  {
    if ( !mFillSymbol )
      mFillSymbol = QgsFillSymbol::createSimple( QgsStringMap() );
    mSymbol = mFillSymbol;
  }
  else if ( symbolType == QgsSymbol::Line )
  {
    if ( !mLineSymbol )
      mLineSymbol = QgsLineSymbol::createSimple( QgsStringMap() );
    mSymbol = mLineSymbol;
  }
  else if ( symbolType == QgsSymbol::Marker )
  {
    if ( !mMarkerSymbol )
      mMarkerSymbol = QgsMarkerSymbol::createSimple( QgsStringMap() );
    mSymbol = mMarkerSymbol;
  }
  else
    Q_ASSERT( false );

  mSymbolType = symbolType;
}

void QgsGeometryGeneratorSymbolLayer::startRender( QgsSymbolRenderContext& context )
{
  mExpression->prepare( &context.renderContext().expressionContext() );

  subSymbol()->startRender( context.renderContext() );
}

void QgsGeometryGeneratorSymbolLayer::stopRender( QgsSymbolRenderContext& context )
{
  if ( mSymbol )
    mSymbol->stopRender( context.renderContext() );
}

QgsSymbolLayer* QgsGeometryGeneratorSymbolLayer::clone() const
{
  QgsGeometryGeneratorSymbolLayer* clone = new QgsGeometryGeneratorSymbolLayer( mExpression->expression() );

  if ( mFillSymbol )
    clone->mFillSymbol = mFillSymbol->clone();
  if ( mLineSymbol )
    clone->mLineSymbol = mLineSymbol->clone();
  if ( mMarkerSymbol )
    clone->mMarkerSymbol = mMarkerSymbol->clone();

  clone->setSymbolType( mSymbolType );

  return clone;
}

QgsStringMap QgsGeometryGeneratorSymbolLayer::properties() const
{
  QgsStringMap props;
  props.insert( "geometryModifier" , mExpression->expression() );
  switch ( mSymbolType )
  {
    case QgsSymbol::Marker:
      props.insert( "SymbolType", "Marker" );
      break;
    case QgsSymbol::Line:
      props.insert( "SymbolType", "Line" );
      break;
    default:
      props.insert( "SymbolType", "Fill" );
      break;
  }

  return props;
}

void QgsGeometryGeneratorSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext& context, QSize size )
{
  if ( mSymbol )
    mSymbol->drawPreviewIcon( context.renderContext().painter(), size );
}

void QgsGeometryGeneratorSymbolLayer::setGeometryExpression( const QString& exp )
{
  mExpression.reset( new QgsExpression( exp ) );
}

bool QgsGeometryGeneratorSymbolLayer::setSubSymbol( QgsSymbol* symbol )
{
  switch ( symbol->type() )
  {
    case QgsSymbol::Marker:
      mMarkerSymbol = static_cast<QgsMarkerSymbol*>( symbol );
      break;

    case QgsSymbol::Line:
      mLineSymbol = static_cast<QgsLineSymbol*>( symbol );
      break;

    case QgsSymbol::Fill:
      mFillSymbol = static_cast<QgsFillSymbol*>( symbol );
      break;

    default:
      break;
  }

  setSymbolType( symbol->type() );

  return true;
}

QSet<QString> QgsGeometryGeneratorSymbolLayer::usedAttributes() const
{
  return mSymbol->usedAttributes() + mExpression->referencedColumns();
}

bool QgsGeometryGeneratorSymbolLayer::isCompatibleWithSymbol( QgsSymbol* symbol ) const
{
  Q_UNUSED( symbol )
  return true;
}
void QgsGeometryGeneratorSymbolLayer::render( QgsSymbolRenderContext& context )
{
  if ( context.feature() )
  {
    QgsExpressionContext& expressionContext = context.renderContext().expressionContext();

    QgsFeature f = expressionContext.feature();
    QgsGeometry geom = mExpression->evaluate( &expressionContext ).value<QgsGeometry>();
    f.setGeometry( geom );

    QgsExpressionContextScope* subSymbolExpressionContextScope = mSymbol->symbolRenderContext()->expressionContextScope();

    subSymbolExpressionContextScope->setFeature( f );

    mSymbol->renderFeature( f, context.renderContext(), -1, context.selected() );
  }
}

void QgsGeometryGeneratorSymbolLayer::setColor( const QColor& color )
{
  mSymbol->setColor( color );
}
