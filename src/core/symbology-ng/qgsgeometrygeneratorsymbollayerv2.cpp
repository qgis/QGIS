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

QgsSymbolLayerV2* QgsGeometryGeneratorSymbolLayerV2::create( const QgsStringMap& properties )
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
    symbolLayer->setSubSymbol( QgsLineSymbolV2::createSimple( properties ) );
  }
  else
  {
    symbolLayer->setSubSymbol( QgsFillSymbolV2::createSimple( properties ) );
  }

  return symbolLayer;
}

QgsGeometryGeneratorSymbolLayerV2::QgsGeometryGeneratorSymbolLayerV2( const QString& expression )
    : QgsSymbolLayerV2( QgsSymbolV2::Hybrid )
    , mExpression( new QgsExpression( expression ) )
    , mFillSymbol( nullptr )
    , mLineSymbol( nullptr )
    , mMarkerSymbol( nullptr )
    , mSymbol( nullptr )
    , mSymbolType( QgsSymbolV2::Marker )
{

}

QString QgsGeometryGeneratorSymbolLayerV2::layerType() const
{
  return "GeometryGenerator";
}

void QgsGeometryGeneratorSymbolLayerV2::setSymbolType( QgsSymbolV2::SymbolType symbolType )
{
  if ( symbolType == QgsSymbolV2::Fill )
  {
    if ( !mFillSymbol )
      mFillSymbol = QgsFillSymbolV2::createSimple( QgsStringMap() );
    mSymbol = mFillSymbol;
  }
  else if ( symbolType == QgsSymbolV2::Line )
  {
    if ( !mLineSymbol )
      mLineSymbol = QgsLineSymbolV2::createSimple( QgsStringMap() );
    mSymbol = mLineSymbol;
  }
  else if ( symbolType == QgsSymbolV2::Marker )
  {
    if ( !mMarkerSymbol )
      mMarkerSymbol = QgsMarkerSymbolV2::createSimple( QgsStringMap() );
    mSymbol = mMarkerSymbol;
  }
  else
    Q_ASSERT( false );

  mSymbolType = symbolType;
}

void QgsGeometryGeneratorSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  mExpression->prepare( &context.renderContext().expressionContext() );

  subSymbol()->startRender( context.renderContext() );
}

void QgsGeometryGeneratorSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  if ( mSymbol )
    mSymbol->stopRender( context.renderContext() );
}

QgsSymbolLayerV2* QgsGeometryGeneratorSymbolLayerV2::clone() const
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
    case QgsSymbolV2::Marker:
      props.insert( "SymbolType", "Marker" );
      break;
    case QgsSymbolV2::Line:
      props.insert( "SymbolType", "Line" );
      break;
    default:
      props.insert( "SymbolType", "Fill" );
      break;
  }

  return props;
}

void QgsGeometryGeneratorSymbolLayerV2::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  if ( mSymbol )
    mSymbol->drawPreviewIcon( context.renderContext().painter(), size );
}

void QgsGeometryGeneratorSymbolLayerV2::setGeometryExpression( const QString& exp )
{
  mExpression.reset( new QgsExpression( exp ) );
}

bool QgsGeometryGeneratorSymbolLayerV2::setSubSymbol( QgsSymbolV2* symbol )
{
  switch ( symbol->type() )
  {
    case QgsSymbolV2::Marker:
      mMarkerSymbol = static_cast<QgsMarkerSymbolV2*>( symbol );
      break;

    case QgsSymbolV2::Line:
      mLineSymbol = static_cast<QgsLineSymbolV2*>( symbol );
      break;

    case QgsSymbolV2::Fill:
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

bool QgsGeometryGeneratorSymbolLayerV2::isCompatibleWithSymbol( QgsSymbolV2* symbol ) const
{
  Q_UNUSED( symbol )
  return true;
}
void QgsGeometryGeneratorSymbolLayerV2::render( QgsSymbolV2RenderContext& context )
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
