#include "qgsgeometrymodifiersymbollayerv2.h"

QgsSymbolLayerV2* QgsPolygonGeneratorSymbolLayer::create( const QgsStringMap& properties )
{
  return new QgsPolygonGeneratorSymbolLayer( QgsFillSymbolV2::createSimple( QgsStringMap() ) );
}

QgsPolygonGeneratorSymbolLayer::QgsPolygonGeneratorSymbolLayer( QgsSymbolV2* symbol )
    : QgsSymbolLayerV2( QgsSymbolV2::Fill )
    , mSymbol( symbol )
{
}

QString QgsPolygonGeneratorSymbolLayer::layerType() const
{
  return "PolygonGenerator";
}

void QgsPolygonGeneratorSymbolLayer::startRender( QgsSymbolV2RenderContext& context )
{
  // prepare expression
  if ( mSymbol )
    mSymbol->startRender( context.renderContext() );
}

void QgsPolygonGeneratorSymbolLayer::stopRender( QgsSymbolV2RenderContext& context )
{
  if ( mSymbol )
    mSymbol->stopRender( context.renderContext() );
}

QgsSymbolLayerV2* QgsPolygonGeneratorSymbolLayer::clone() const
{
  QgsPolygonGeneratorSymbolLayer* clone = new QgsPolygonGeneratorSymbolLayer( mSymbol->clone() );
  clone->mExpression = mExpression;

  return clone;
}

QgsStringMap QgsPolygonGeneratorSymbolLayer::properties() const
{
  return QgsStringMap();
}

void QgsPolygonGeneratorSymbolLayer::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  if ( mSymbol )
    mSymbol->drawPreviewIcon( context.renderContext().painter(), size );
}

void QgsPolygonGeneratorSymbolLayer::setGeometryModifier( const QString& exp )
{
  mExpression = exp;
}

QgsExpressionContext* QgsPolygonGeneratorSymbolLayer::expressionContext()
{

}

bool QgsPolygonGeneratorSymbolLayer::isCompatibleWithSymbol( QgsSymbolV2* symbol )
{
  Q_UNUSED( symbol )
  return true;
}


