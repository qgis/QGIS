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
#include "qgscolorrampimpl.h"
#include "qgslegendsettings.h"
#include "qgslayertreemodel.h"
#include "qgslayertreelayer.h"
#include "qgssymbollayerutils.h"
#include "qgsexpressioncontextutils.h"
#include "qgstextrenderer.h"
#include "qgsnumericformat.h"

QgsColorRampLegendNode::QgsColorRampLegendNode( QgsLayerTreeLayer *nodeLayer, QgsColorRamp *ramp, const QString &minimumLabel, const QString &maximumLabel, QObject *parent )
  : QgsLayerTreeModelLegendNode( nodeLayer, parent )
  , mRamp( ramp )
{
  mSettings.setMinimumLabel( minimumLabel );
  mSettings.setMaximumLabel( maximumLabel );

  init( nodeLayer );
}

QgsColorRampLegendNode::QgsColorRampLegendNode( QgsLayerTreeLayer *nodeLayer, QgsColorRamp *ramp, const QgsColorRampLegendNodeSettings &settings, double minimumValue, double maximumValue, QObject *parent )
  : QgsLayerTreeModelLegendNode( nodeLayer, parent )
  , mRamp( ramp )
  , mSettings( settings )
  , mMinimumValue( minimumValue )
  , mMaximumValue( maximumValue )
{
  init( nodeLayer );
}

void QgsColorRampLegendNode::init( QgsLayerTreeLayer *nodeLayer )
{
  const int iconSize = QgsLayerTreeModel::scaleIconSize( 16 );
  mIconSize = mSettings.orientation() == Qt::Vertical ? QSize( iconSize, iconSize * 6 ) : QSize( iconSize * 6, iconSize );

  connect( nodeLayer, &QObject::destroyed, this, [ = ]() { mLayerNode = nullptr; } );
}

const QgsColorRamp *QgsColorRampLegendNode::ramp() const
{
  return mRamp.get();
}

QgsColorRampLegendNodeSettings QgsColorRampLegendNode::settings() const
{
  return mSettings;
}

void QgsColorRampLegendNode::setSettings( const QgsColorRampLegendNodeSettings &settings )
{
  mSettings = settings;
}

QString QgsColorRampLegendNode::labelForMinimum() const
{
  if ( !mSettings.minimumLabel().isEmpty() )
    return mSettings.prefix() + mSettings.minimumLabel() + mSettings.suffix();

  const QgsNumericFormatContext numericContext;
  return mSettings.prefix() + mSettings.numericFormat()->formatDouble( mMinimumValue, numericContext )  + mSettings.suffix();
}

QString QgsColorRampLegendNode::labelForMaximum() const
{
  if ( !mSettings.maximumLabel().isEmpty() )
    return mSettings.prefix() + mSettings.maximumLabel() + mSettings.suffix();

  const QgsNumericFormatContext numericContext;
  return mSettings.prefix() + mSettings.numericFormat()->formatDouble( mMaximumValue, numericContext ) + mSettings.suffix();
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
      const QFont font = data( Qt::FontRole ).value< QFont >();

      const QString minLabel = labelForMinimum();
      const QString maxLabel = labelForMaximum();

      const QFontMetrics fm( font );

      const QRect minBoundingRect = fm.boundingRect( minLabel );
      const QRect maxBoundingRect = fm.boundingRect( maxLabel );

      const int minLabelWidth = minBoundingRect.width();
      const int maxLabelWidth = maxBoundingRect.width();
      const int maxTextWidth = std::max( minLabelWidth, maxLabelWidth );
      const int labelGapFromRamp = fm.boundingRect( QStringLiteral( "x" ) ).width();
      const int extraAllowance = labelGapFromRamp * 0.4; // extra allowance to avoid text clipping on right
      QRect labelRect;
      QSize rampSize;
      switch ( mSettings.orientation() )
      {
        case Qt::Vertical:
          labelRect = QRect( mIconSize.width() + labelGapFromRamp, 0, maxTextWidth + extraAllowance, mIconSize.height() );
          mPixmap = QPixmap( mIconSize.width() + maxTextWidth + labelGapFromRamp + extraAllowance, mIconSize.height() );
          rampSize = mIconSize;
          break;

        case Qt::Horizontal:
          labelRect = QRect( 0, mIconSize.height() + labelGapFromRamp, std::max( mIconSize.width(), minLabelWidth + maxLabelWidth + labelGapFromRamp ), std::max( minBoundingRect.height(),
                             maxBoundingRect.height() ) + extraAllowance );
          mPixmap = QPixmap( std::max( mIconSize.width(), minLabelWidth + maxLabelWidth + labelGapFromRamp ), mIconSize.height() + maxTextWidth + labelGapFromRamp + extraAllowance );
          rampSize = QSize( labelRect.width(), mIconSize.height() );
          break;
      }

      mPixmap.fill( Qt::transparent );

      QPixmap pix;

      if ( mRamp )
      {
        pix = QgsSymbolLayerUtils::colorRampPreviewPixmap( mRamp.get(), rampSize, 0, mSettings.orientation(),
              mSettings.orientation() == Qt::Vertical ? mSettings.direction() != QgsColorRampLegendNodeSettings::MaximumToMinimum
              : mSettings.direction() != QgsColorRampLegendNodeSettings::MinimumToMaximum,
              false );
      }
      else
      {
        pix = QPixmap( rampSize );
        pix.fill( Qt::transparent );
      }

      QPainter p( &mPixmap );
      p.drawPixmap( 0, 0, pix );
      p.setFont( font );

      switch ( mSettings.orientation() )
      {
        case Qt::Vertical:
          p.drawText( labelRect, Qt::AlignBottom | Qt::AlignLeft, mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? minLabel : maxLabel );
          p.drawText( labelRect, Qt::AlignTop | Qt::AlignLeft, mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? maxLabel : minLabel );
          break;

        case Qt::Horizontal:
          p.drawText( labelRect, Qt::AlignTop | Qt::AlignLeft, mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? minLabel : maxLabel );
          p.drawText( labelRect, Qt::AlignTop | Qt::AlignRight, mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? maxLabel : minLabel );
          break;
      }

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
    tempRenderContext = std::make_unique< QgsRenderContext >();
    // QGIS 4.0 - make ItemContext compulsory, so we don't have to construct temporary render contexts here
    Q_NOWARN_DEPRECATED_PUSH
    tempRenderContext->setScaleFactor( settings.dpi() / 25.4 );
    tempRenderContext->setRendererScale( settings.mapScale() );
    tempRenderContext->setFlag( Qgis::RenderContextFlag::Antialiasing, true );
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

  const QgsTextFormat format = mSettings.textFormat().isValid() ? mSettings.textFormat() : settings.style( QgsLegendStyle::SymbolLabel ).textFormat();
  const QString minLabel = labelForMinimum();
  const QString maxLabel = labelForMaximum();

  const double patchWidth = ctx && ctx->patchSize.width() > 0 ? ctx->patchSize.width() : settings.symbolSize().width();
  const double patchHeight = ctx && ctx->patchSize.height() > 0 ? ctx->patchSize.height() : settings.symbolSize().height();

  double minHeightMm = 0;
  double minWidthMm = 0;
  double rampHeight = 0;
  double rampWidth = 0;
  switch ( mSettings.orientation() )
  {
    case Qt::Vertical:
      // vertical bar, min height is the text height of the min and max labels
      minHeightMm = QgsTextRenderer::textHeight( *context, format, QStringList() << minLabel << maxLabel, Qgis::TextLayoutMode::Rectangle ) / context->scaleFactor();
      rampHeight = ctx && ctx->patchSize.height() > 0 ? std::max( minHeightMm / 2, ctx->patchSize.height() ) : std::max( minHeightMm, settings.symbolSize().height() );
      rampWidth = patchWidth;
      break;

    case Qt::Horizontal:
      // horizontal bar, min width is text width of the min and max labels
      minWidthMm = ( QgsTextRenderer::textWidth( *context, format, QStringList() << minLabel ) +
                     QgsTextRenderer::textWidth( *context, format, QStringList() << maxLabel ) ) / context->scaleFactor();
      rampHeight = patchHeight;
      rampWidth = std::max( minWidthMm, patchWidth );
      break;
  }

  if ( ctx && ctx->painter )
  {
    const double currentYCoord = ctx->top;
    QPainter *p = ctx->painter;

    //setup painter scaling to dots so that raster symbology is drawn to scale
    const double dotsPerMM = context->scaleFactor();

    double opacity = 1;
    if ( QgsMapLayer *layer = layerNode()->layer() )
      opacity = layer->opacity();

    const QgsScopedQPainterState painterState( p );
    context->setPainterFlagsUsingContext( p );

    double rampLeftMm = 0;
    const double rampTopMm = currentYCoord;
    switch ( settings.symbolAlignment() )
    {
      case Qt::AlignLeft:
      default:
        rampLeftMm = ctx->columnLeft;
        break;

      case Qt::AlignRight:
        rampLeftMm = ctx->columnRight - rampWidth;
        break;
    }

    p->scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );

    QLinearGradient gradient;
    switch ( mSettings.orientation() )
    {
      case Qt::Vertical:
      {
        const double gradientTop = rampTopMm * dotsPerMM;
        const double gradientBottom = gradientTop + rampHeight * dotsPerMM;
        gradient = QLinearGradient( 0, mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? gradientBottom : gradientTop,
                                    0, mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? gradientTop : gradientBottom );
        break;
      }

      case Qt::Horizontal:
      {
        const double gradientLeft = rampLeftMm * dotsPerMM;
        const double gradientRight = gradientLeft + rampWidth * dotsPerMM;
        gradient = QLinearGradient( mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? gradientLeft : gradientRight, 0,
                                    mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? gradientRight : gradientLeft, 0 );
        break;
      }
    }


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
    p->drawRect( rampLeftMm * dotsPerMM, rampTopMm * dotsPerMM, rampWidth * dotsPerMM, rampHeight * dotsPerMM );
  }

  double labelHeight = 0;
  if ( mSettings.orientation() == Qt::Horizontal )
  {
    // we treat the text as part of the symbol for horizontal bar items
    if ( ctx && ctx->painter )
    {
      const double currentYCoord = ctx->top;
      QPainter *p = ctx->painter;

      //setup painter scaling to dots so that raster symbology is drawn to scale
      const double dotsPerMM = context->scaleFactor();

      const QgsScopedQPainterState painterState( p );
      context->setPainterFlagsUsingContext( p );

      p->scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );

      double labelXMin = 0;
      double labelXMax = 0;
      // NOTE -- while the below calculations use the flipped margins from the style, that's only done because
      // those are the only margins we expose and use for now! (and we expose them as generic margins, not side-specific
      // ones) TODO when/if we expose other margin settings, these should be reversed...
      const double labelYMin = currentYCoord + rampHeight + settings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right )
                               + settings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
      const double labelHeight = std::max( QgsTextRenderer::textHeight( *context, format, QStringList() << minLabel ),
                                           QgsTextRenderer::textHeight( *context, format, QStringList() << maxLabel ) ) / dotsPerMM;
      switch ( settings.symbolAlignment() )
      {
        case Qt::AlignLeft:
        default:
          labelXMin = ctx->columnLeft;
          labelXMax = ctx->columnLeft + rampWidth;
          break;

        case Qt::AlignRight:
          labelXMin = ctx->columnRight - rampWidth;
          labelXMin = ctx->columnRight;
          break;
      }

      const QRectF textRect( labelXMin * dotsPerMM, labelYMin * dotsPerMM, ( labelXMax - labelXMin ) * dotsPerMM, labelHeight * dotsPerMM );
      QgsTextRenderer::drawText( textRect, 0, Qgis::TextHorizontalAlignment::Left,
                                 QStringList() << ( mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? minLabel : maxLabel ),
                                 *context, format, true, Qgis::TextVerticalAlignment::Top );
      QgsTextRenderer::drawText( textRect, 0, Qgis::TextHorizontalAlignment::Right,
                                 QStringList() << ( mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? maxLabel : minLabel ),
                                 *context, format, true, Qgis::TextVerticalAlignment::Bottom );
    }
    else
    {
      // we only need this when we are calculating the size of the node, not at render time
      labelHeight = std::max( QgsTextRenderer::textHeight( *context, format, QStringList() << minLabel ),
                              QgsTextRenderer::textHeight( *context, format, QStringList() << maxLabel ) ) / context->scaleFactor()
                    + settings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right )
                    + settings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
    }
  }

  return QSizeF( rampWidth, rampHeight + labelHeight );
}

QSizeF QgsColorRampLegendNode::drawSymbolText( const QgsLegendSettings &settings, QgsLayerTreeModelLegendNode::ItemContext *ctx, QSizeF symbolSize ) const
{
  if ( !mRamp || mSettings.orientation() == Qt::Horizontal )
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
    tempRenderContext = std::make_unique< QgsRenderContext >();
    // QGIS 4.0 - make ItemContext compulsory, so we don't have to construct temporary render contexts here
    Q_NOWARN_DEPRECATED_PUSH
    tempRenderContext->setScaleFactor( settings.dpi() / 25.4 );
    tempRenderContext->setRendererScale( settings.mapScale() );
    tempRenderContext->setFlag( Qgis::RenderContextFlag::Antialiasing, true );
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

  const QgsTextFormat format = mSettings.textFormat().isValid() ? mSettings.textFormat() : settings.style( QgsLegendStyle::SymbolLabel ).textFormat();

  const QString minLabel = labelForMinimum();
  const QString maxLabel = labelForMaximum();

  const double rampHeight = symbolSize.height();
  const double rampWidth = symbolSize.width();
  double textWidth = 0;
  double textHeight = 0;

  if ( ctx && ctx->painter )
  {
    const double currentYCoord = ctx->top;
    QPainter *p = ctx->painter;

    //setup painter scaling to dots so that raster symbology is drawn to scale
    const double dotsPerMM = context->scaleFactor();

    const QgsScopedQPainterState painterState( p );
    context->setPainterFlagsUsingContext( p );

    p->scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );

    double labelXMin = 0;
    double labelXMax = 0;
    switch ( settings.symbolAlignment() )
    {
      case Qt::AlignLeft:
      default:
        labelXMin = ctx->columnLeft + std::max( rampWidth, ctx->maxSiblingSymbolWidth )
                    + settings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right )
                    + settings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
        labelXMax = ctx->columnRight;
        break;

      case Qt::AlignRight:
        labelXMin = ctx->columnLeft;
        // NOTE -- while the below calculations use the flipped margins from the style, that's only done because
        // those are the only margins we expose and use for now! (and we expose them as generic margins, not side-specific
        // ones) TODO when/if we expose other margin settings, these should be reversed...
        labelXMax = ctx->columnRight - std::max( rampWidth, ctx->maxSiblingSymbolWidth )
                    - settings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right )
                    - settings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
        break;
    }

    const QRectF textRect( labelXMin * dotsPerMM, currentYCoord * dotsPerMM, ( labelXMax - labelXMin ) * dotsPerMM, rampHeight * dotsPerMM );
    QgsTextRenderer::drawText( textRect, 0, QgsTextRenderer::convertQtHAlignment( settings.style( QgsLegendStyle::SymbolLabel ).alignment() ),
                               QStringList() << ( mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? maxLabel : minLabel ),
                               *context, format, true, Qgis::TextVerticalAlignment::Top );
    QgsTextRenderer::drawText( textRect, 0, QgsTextRenderer::convertQtHAlignment( settings.style( QgsLegendStyle::SymbolLabel ).alignment() ),
                               QStringList() << ( mSettings.direction() == QgsColorRampLegendNodeSettings::MinimumToMaximum ? minLabel : maxLabel ),
                               *context, format, true, Qgis::TextVerticalAlignment::Bottom );
  }
  else
  {
    // we only need this when we are calculating the size of the node, not at render time
    textWidth = QgsTextRenderer::textWidth( *context, format, QStringList() << minLabel << maxLabel ) / context->scaleFactor();
    textHeight = rampHeight;
  }

  return QSizeF( textWidth, textHeight );
}
