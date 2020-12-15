/***************************************************************************
  qgscolorramplegendnode.cpp
  --------------------------------------
  Date                 : December 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorramplegendnode.h"
#include "qgslegendsettings.h"
#include "qgslayertreemodel.h"
#include "qgslayertreelayer.h"
#include "qgssymbollayerutils.h"
#include "qgsexpressioncontextutils.h"
#include "qgstextrenderer.h"

QgsColorRampLegendNode::QgsColorRampLegendNode( QgsLayerTreeLayer *nodeLayer, QgsColorRamp *ramp, const QString &minimumLabel, const QString &maximumLabel, QObject *parent )
  : QgsLayerTreeModelLegendNode( nodeLayer, parent )
  , mRamp( ramp )
  , mMinimumLabel( minimumLabel )
  , mMaximumLabel( maximumLabel )
{
  const int iconSize = QgsLayerTreeModel::scaleIconSize( 16 );
  mIconSize = QSize( iconSize, iconSize * 6 );

  connect( nodeLayer, &QObject::destroyed, this, [ = ]() { mLayerNode = nullptr; } );
}

const QgsColorRamp *QgsColorRampLegendNode::ramp() const
{
  return mRamp.get();
}

QVariant QgsColorRampLegendNode::data( int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    return QString();
  }
  else if ( role == Qt::EditRole )
  {
    return QString();
  }
  else if ( role == Qt::DecorationRole )
  {
    if ( mPixmap.isNull() || mPixmap.size() != mIconSize )
    {
      QPixmap pix;

      if ( mRamp )
      {
        pix = QgsSymbolLayerUtils::colorRampPreviewPixmap( mRamp.get(), mIconSize, 0, Qt::Vertical, true, false );
      }
      else
      {
        pix = QPixmap( mIconSize );
        pix.fill( Qt::transparent );
      }

      const QFont font = data( Qt::FontRole ).value< QFont >();

      const QFontMetrics fm( font );
      const int maxTextWidth = std::max( fm.boundingRect( mMinimumLabel ).width(), fm.boundingRect( mMaximumLabel ).width() );
      const int labelGapFromRamp = fm.boundingRect( QStringLiteral( "x" ) ).width();
      const int extraAllowance = labelGapFromRamp * 0.4; // extra allowance to avoid text clipping on right
      const QRect labelRect( mIconSize.width() + labelGapFromRamp, 0, maxTextWidth + extraAllowance, mIconSize.height() );

      mPixmap = QPixmap( mIconSize.width() + maxTextWidth + labelGapFromRamp + extraAllowance, mIconSize.height() );
      mPixmap.fill( Qt::transparent );

      QPainter p( &mPixmap );
      p.drawPixmap( 0, 0, pix );
      p.setFont( font );

      p.drawText( labelRect, Qt::AlignTop | Qt::AlignLeft, mMaximumLabel );
      p.drawText( labelRect, Qt::AlignBottom | Qt::AlignLeft, mMinimumLabel );
      p.end();
    }
    return mPixmap;
  }
  else if ( role == QgsLayerTreeModelLegendNode::NodeTypeRole )
  {
    return QgsLayerTreeModelLegendNode::ColorRampLegend;
  }

  return QVariant();
}

QSizeF QgsColorRampLegendNode::drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double ) const
{
  if ( !mRamp )
  {
    return QSizeF();
  }

  // setup temporary render context
  QgsRenderContext *context = nullptr;
  std::unique_ptr< QgsRenderContext > tempRenderContext;
  if ( ctx && ctx->context )
    context = ctx->context;
  else
  {
    tempRenderContext = qgis::make_unique< QgsRenderContext >();
    // QGIS 4.0 - make ItemContext compulsory, so we don't have to construct temporary render contexts here
    Q_NOWARN_DEPRECATED_PUSH
    tempRenderContext->setScaleFactor( settings.dpi() / 25.4 );
    tempRenderContext->setRendererScale( settings.mapScale() );
    tempRenderContext->setFlag( QgsRenderContext::Antialiasing, true );
    tempRenderContext->setMapToPixel( QgsMapToPixel( 1 / ( settings.mmPerMapUnit() * tempRenderContext->scaleFactor() ) ) );
    Q_NOWARN_DEPRECATED_POP
    tempRenderContext->setForceVectorOutput( true );
    tempRenderContext->setPainter( ctx ? ctx->painter : nullptr );

    // setup a minimal expression context
    QgsExpressionContext expContext;
    expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) );
    tempRenderContext->setExpressionContext( expContext );
    context = tempRenderContext.get();
  }

  QFont symbolLabelFont = settings.style( QgsLegendStyle::SymbolLabel ).font();
  QgsTextFormat format = QgsTextFormat::fromQFont( symbolLabelFont );
  format.setColor( settings.fontColor() );

  double minHeightMm = QgsTextRenderer::textHeight( *context, format, QStringList() << mMinimumLabel << mMaximumLabel, QgsTextRenderer::Rect ) / context->scaleFactor();

  const double height = ctx && ctx->patchSize.height() > 0 ? std::max( minHeightMm / 2, ctx->patchSize.height() ) : std::max( minHeightMm, settings.symbolSize().height() );
  const double width = ctx && ctx->patchSize.width() > 0 ? ctx->patchSize.width() : settings.symbolSize().width();

  if ( ctx && ctx->painter )
  {
    double currentYCoord = ctx->top;
    QPainter *p = ctx->painter;

    //setup painter scaling to dots so that raster symbology is drawn to scale
    double dotsPerMM = context->scaleFactor();

    double opacity = 1;
    if ( QgsMapLayer *layer = layerNode()->layer() )
      opacity = layer->opacity();

    QgsScopedQPainterState painterState( p );
    context->setPainterFlagsUsingContext( p );

    double rampLeftMm = 0;
    double rampTopMm = currentYCoord;
    switch ( settings.symbolAlignment() )
    {
      case Qt::AlignLeft:
      default:
        rampLeftMm = ctx->columnLeft;
        break;

      case Qt::AlignRight:
        rampLeftMm = ctx->columnRight - width;
        break;
    }

    p->scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );

    QLinearGradient gradient( 0, rampTopMm * dotsPerMM, 0, rampTopMm * dotsPerMM + height * dotsPerMM );
    if ( mRamp->type() == QgsGradientColorRamp::typeString() || mRamp->type() == QgsCptCityColorRamp::typeString() )
    {
      //color ramp gradient
      QgsGradientColorRamp *gradRamp = static_cast<QgsGradientColorRamp *>( mRamp.get() );
      gradRamp->addStopsToGradient( &gradient, opacity );
    }

    if ( settings.drawRasterStroke() )
    {
      QPen pen;
      pen.setColor( settings.rasterStrokeColor() );
      pen.setWidthF( settings.rasterStrokeWidth() * dotsPerMM );
      pen.setJoinStyle( Qt::MiterJoin );
      ctx->painter->setPen( pen );
    }
    else
    {
      ctx->painter->setPen( Qt::NoPen );
    }

    p->setBrush( QBrush( gradient ) );
    p->drawRect( rampLeftMm * dotsPerMM, rampTopMm * dotsPerMM, width * dotsPerMM, height * dotsPerMM );

    double labelXMin = 0;
    double labelXMax = 0;
    switch ( settings.symbolAlignment() )
    {
      case Qt::AlignLeft:
      default:
        labelXMin = ctx->columnLeft + std::max( width, ctx->maxSiblingSymbolWidth )
                    + settings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right )
                    + settings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
        labelXMax = ctx->columnRight;
        break;

      case Qt::AlignRight:
        labelXMin = ctx->columnLeft;
        // NOTE -- while the below calculations use the flipped margins from the style, that's only done because
        // those are the only margins we expose and use for now! (and we expose them as generic margins, not side-specific
        // ones) TODO when/if we expose other margin settings, these should be reversed...
        labelXMax = ctx->columnRight - std::max( width, ctx->maxSiblingSymbolWidth )
                    - settings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right )
                    - settings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
        break;
    }

    const QRectF textRect( labelXMin * dotsPerMM, currentYCoord * dotsPerMM, ( labelXMax - labelXMin ) * dotsPerMM, height * dotsPerMM );
    QgsTextRenderer::drawText( textRect, 0, QgsTextRenderer::convertQtHAlignment( settings.style( QgsLegendStyle::SymbolLabel ).alignment() ), QStringList() << mMaximumLabel, *context, format, true, QgsTextRenderer::AlignTop );
    QgsTextRenderer::drawText( textRect, 0, QgsTextRenderer::convertQtHAlignment( settings.style( QgsLegendStyle::SymbolLabel ).alignment() ), QStringList() << mMinimumLabel, *context, format, true, QgsTextRenderer::AlignBottom );
  }

  return QSizeF( width, height );
}


