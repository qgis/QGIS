/***************************************************************************
 qgsgeometrygeneratorsymbollayerv2.cpp
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

#include "qgsgeometrygeneratorsymbollayerv2.h"
#include "qgsgeometry.h"

QgsGeometryGeneratorSymbolLayerV2::~QgsGeometryGeneratorSymbolLayerV2()
{
  delete mMarkerSymbol;
  delete mLineSymbol;
  delete mFillSymbol;
}

QgsSymbolLayer* QgsGeometryGeneratorSymbolLayerV2::create( const QgsStringMap& properties )
{
  QString expression = properties.value( "geometryModifier" );
  if ( expression.isEmpty() )
  {
    expression = "$geometry";
  }
  QgsGeometryGeneratorSymbolLayerV2* symbolLayer = new QgsGeometryGeneratorSymbolLayerV2( expression );

  if ( properties.value( "SymbolType" ) == "Marker" )
  {
    symbolLayer->setSubSymbol( QgsMarkerSymbolV2::createSimple( properties ) );
  }
  else if ( properties.value( "SymbolType" ) == "Line" )
  {
    symbolLayer->setSubSymbol( QgsLineSymbol::createSimple( properties ) );
  }
  else
  {
    symbolLayer->setSubSymbol( QgsFillSymbolV2::createSimple( properties ) );
  }

  return symbolLayer;
}

QgsGeometryGeneratorSymbolLayerV2::QgsGeometryGeneratorSymbolLayerV2( const QString& expression )
    : QgsSymbolLayer( QgsSymbol::Hybrid )
    , mExpression( new QgsExpression( expression ) )
    , mFillSymbol( nullptr )
    , mLineSymbol( nullptr )
    , mMarkerSymbol( nullptr )
    , mSymbol( nullptr )
    , mSymbolType( QgsSymbol::Marker )
{

}

QString QgsGeometryGeneratorSymbolLayerV2::layerType() const
{
  return "GeometryGenerator";
}

void QgsGeometryGeneratorSymbolLayerV2::setSymbolType( QgsSymbol::SymbolType symbolType )
{
  if ( symbolType == QgsSymbol::Fill )
  {
    if ( !mFillSymbol )
      mFillSymbol = QgsFillSymbolV2::createSimple( QgsStringMap() );
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
      mMarkerSymbol = QgsMarkerSymbolV2::createSimple( QgsStringMap() );
    mSymbol = mMarkerSymbol;
  }
  else
    Q_ASSERT( false );

  mSymbolType = symbolType;
}

void QgsGeometryGeneratorSymbolLayerV2::startRender( QgsSymbolRenderContext& context )
{
  mExpression->prepare( &context.renderContext().expressionContext() );

  subSymbol()->startRender( context.renderContext() );
}

void QgsGeometryGeneratorSymbolLayerV2::stopRender( QgsSymbolRenderContext& context )
{
  if ( mSymbol )
    mSymbol->stopRender( context.renderContext() );
}

QgsSymbolLayer* QgsGeometryGeneratorSymbolLayerV2::clone() const
{
  QgsGeometryGeneratorSymbolLayerV2* clone = new QgsGeometryGeneratorSymbolLayerV2( mExpression->expression() );

  if ( mFillSymbol )
    clone->mFillSymbol = mFillSymbol->clone();
  if ( mLineSymbol )
    clone->mLineSymbol = mLineSymbol->clone();
  if ( mMarkerSymbol )
    clone->mMarkerSymbol = mMarkerSymbol->clone();

  clone->setSymbolType( mSymbolType );

  return clone;
}

QgsStringMap QgsGeometryGeneratorSymbolLayerV2::properties() const
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

void QgsGeometryGeneratorSymbolLayerV2::drawPreviewIcon( QgsSymbolRenderContext& context, QSize size )
{
  if ( mSymbol )
    mSymbol->drawPreviewIcon( context.renderContext().painter(), size );
}

void QgsGeometryGeneratorSymbolLayerV2::setGeometryExpression( const QString& exp )
{
  mExpression.reset( new QgsExpression( exp ) );
}

bool QgsGeometryGeneratorSymbolLayerV2::setSubSymbol( QgsSymbol* symbol )
{
  switch ( symbol->type() )
  {
    case QgsSymbol::Marker:
      mMarkerSymbol = static_cast<QgsMarkerSymbolV2*>( symbol );
      break;

    case QgsSymbol::Line:
      mLineSymbol = static_cast<QgsLineSymbol*>( symbol );
      break;

    case QgsSymbol::Fill:
      mFillSymbol = static_cast<QgsFillSymbolV2*>( symbol );
      break;

    default:
      break;
  }

  setSymbolType( symbol->type() );

  return true;
}

QSet<QString> QgsGeometryGeneratorSymbolLayerV2::usedAttributes() const
{
  return mSymbol->usedAttributes() + mExpression->referencedColumns().toSet();
}

bool QgsGeometryGeneratorSymbolLayerV2::isCompatibleWithSymbol( QgsSymbol* symbol ) const
{
  Q_UNUSED( symbol )
  return true;
}
void QgsGeometryGeneratorSymbolLayerV2::render( QgsSymbolRenderContext& context )
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

void QgsGeometryGeneratorSymbolLayerV2::setColor( const QColor& color )
{
  mSymbol->setColor( color );
}
