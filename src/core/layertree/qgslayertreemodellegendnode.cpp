/***************************************************************************
  qgslayertreemodellegendnode.cpp
  --------------------------------------
  Date                 : August 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com

  QgsWMSLegendNode     : Sandro Santilli < strk at keybit dot net >

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreemodellegendnode.h"

#include "qgsdatadefinedsizelegend.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslegendsettings.h"
#include "qgsrasterlayer.h"
#include "qgsrenderer.h"
#include "qgssymbollayerutils.h"
#include "qgsimageoperation.h"
#include "qgsvectorlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeatureid.h"
#include "qgslayoutitem.h"
#include "qgsvectorlayerfeaturecounter.h"
#include "qgsexpression.h"
#include "qgstextrenderer.h"


QgsLayerTreeModelLegendNode::QgsLayerTreeModelLegendNode( QgsLayerTreeLayer *nodeL, QObject *parent )
  : QObject( parent )
  , mLayerNode( nodeL )
  , mEmbeddedInParent( false )
{
}

QgsLayerTreeModel *QgsLayerTreeModelLegendNode::model() const
{
  return qobject_cast<QgsLayerTreeModel *>( parent() );
}

Qt::ItemFlags QgsLayerTreeModelLegendNode::flags() const
{
  return Qt::ItemIsEnabled;
}

bool QgsLayerTreeModelLegendNode::setData( const QVariant &value, int role )
{
  Q_UNUSED( value )
  Q_UNUSED( role )
  return false;
}

QSizeF QgsLayerTreeModelLegendNode::userPatchSize() const
{
  if ( mEmbeddedInParent )
    return mLayerNode->patchSize();

  return mUserSize;
}

QgsLayerTreeModelLegendNode::ItemMetrics QgsLayerTreeModelLegendNode::draw( const QgsLegendSettings &settings, ItemContext *ctx )
{
  QFont symbolLabelFont = settings.style( QgsLegendStyle::SymbolLabel ).font();

  double textHeight = settings.fontHeightCharacterMM( symbolLabelFont, QChar( '0' ) );
  // itemHeight here is not really item height, it is only for symbol
  // vertical alignment purpose, i.e. OK take single line height
  // if there are more lines, those run under the symbol
  double itemHeight = std::max( static_cast< double >( ctx && ctx->patchSize.height() > 0 ? ctx->patchSize.height() : settings.symbolSize().height() ), textHeight );

  ItemMetrics im;
  im.symbolSize = drawSymbol( settings, ctx, itemHeight );
  im.labelSize = drawSymbolText( settings, ctx, im.symbolSize );
  return im;
}

QJsonObject QgsLayerTreeModelLegendNode::exportToJson( const QgsLegendSettings &settings, const QgsRenderContext &context )
{
  QJsonObject json = exportSymbolToJson( settings, context );
  const QString text = data( Qt::DisplayRole ).toString();
  json[ QStringLiteral( "title" ) ] = text;
  return json;
}

QSizeF QgsLayerTreeModelLegendNode::drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const
{
  QIcon symbolIcon = data( Qt::DecorationRole ).value<QIcon>();
  if ( symbolIcon.isNull() )
    return QSizeF();

  QSizeF size = settings.symbolSize();
  if ( ctx )
  {
    if ( ctx->patchSize.width() > 0 )
      size.setWidth( ctx->patchSize.width( ) );
    if ( ctx->patchSize.height() > 0 )
      size.setHeight( ctx->patchSize.height( ) );
  }

  if ( ctx && ctx->painter )
  {
    switch ( settings.symbolAlignment() )
    {
      case Qt::AlignLeft:
      default:
        symbolIcon.paint( ctx->painter,
                          static_cast< int >( ctx->columnLeft ),
                          static_cast< int >( ctx->top + ( itemHeight - size.height() ) / 2 ),
                          static_cast< int >( size.width() ),
                          static_cast< int >( size.height() ) );
        break;

      case Qt::AlignRight:
        symbolIcon.paint( ctx->painter,
                          static_cast< int >( ctx->columnRight - size.width() ),
                          static_cast< int >( ctx->top + ( itemHeight - size.height() ) / 2 ),
                          static_cast< int >( size.width() ),
                          static_cast< int >( size.height() ) );
        break;
    }
  }
  return size;
}

QJsonObject QgsLayerTreeModelLegendNode::exportSymbolToJson( const QgsLegendSettings &settings, const QgsRenderContext & ) const
{
  const QIcon icon = data( Qt::DecorationRole ).value<QIcon>();
  if ( icon.isNull() )
    return QJsonObject();

  const QImage image( icon.pixmap( settings.symbolSize().width(), settings.symbolSize().height() ).toImage() );
  QByteArray byteArray;
  QBuffer buffer( &byteArray );
  image.save( &buffer, "PNG" );
  const QString base64 = QString::fromLatin1( byteArray.toBase64().data() );

  QJsonObject json;
  json[ QStringLiteral( "icon" ) ] = base64;
  return json;
}

QSizeF QgsLayerTreeModelLegendNode::drawSymbolText( const QgsLegendSettings &settings, ItemContext *ctx, QSizeF symbolSize ) const
{
  QSizeF labelSize( 0, 0 );

  QFont symbolLabelFont = settings.style( QgsLegendStyle::SymbolLabel ).font();
  double textHeight = settings.fontHeightCharacterMM( symbolLabelFont, QChar( '0' ) );
  double textDescent = settings.fontDescentMillimeters( symbolLabelFont );

  QgsExpressionContext tempContext;

  const QStringList lines = settings.evaluateItemText( data( Qt::DisplayRole ).toString(), ctx && ctx->context ? ctx->context->expressionContext() : tempContext );

  labelSize.rheight() = lines.count() * textHeight + ( lines.count() - 1 ) * ( settings.lineSpacing() + textDescent );

  double labelXMin = 0.0;
  double labelXMax = 0.0;
  double labelY = 0.0;
  if ( ctx && ctx->painter )
  {
    ctx->painter->setPen( settings.fontColor() );
    switch ( settings.symbolAlignment() )
    {
      case Qt::AlignLeft:
      default:
        labelXMin = ctx->columnLeft + std::max( static_cast< double >( symbolSize.width() ), ctx->maxSiblingSymbolWidth )
                    + settings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right )
                    + settings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
        labelXMax = ctx->columnRight;
        break;

      case Qt::AlignRight:
        labelXMin = ctx->columnLeft;
        // NOTE -- while the below calculations use the flipped margins from the style, that's only done because
        // those are the only margins we expose and use for now! (and we expose them as generic margins, not side-specific
        // ones) TODO when/if we expose other margin settings, these should be reversed...
        labelXMax = ctx->columnRight - std::max( static_cast< double >( symbolSize.width() ), ctx->maxSiblingSymbolWidth )
                    - settings.style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Right )
                    - settings.style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left );
        break;
    }

    labelY = ctx->top;

    // Vertical alignment of label with symbol
    if ( labelSize.height() < symbolSize.height() )
      labelY += symbolSize.height() / 2 - labelSize.height() / 2;  // label centered with symbol

    labelY += textHeight;
  }

  for ( QStringList::ConstIterator itemPart = lines.constBegin(); itemPart != lines.constEnd(); ++itemPart )
  {
    const double lineWidth = settings.textWidthMillimeters( symbolLabelFont, *itemPart );
    labelSize.rwidth() = std::max( lineWidth, double( labelSize.width() ) );

    if ( ctx && ctx->painter )
    {
      switch ( settings.style( QgsLegendStyle::SymbolLabel ).alignment() )
      {
        case Qt::AlignLeft:
        default:
          settings.drawText( ctx->painter, labelXMin, labelY, *itemPart, symbolLabelFont );
          break;

        case Qt::AlignRight:
          settings.drawText( ctx->painter, labelXMax - lineWidth, labelY, *itemPart, symbolLabelFont );
          break;

        case Qt::AlignHCenter:
          settings.drawText( ctx->painter, labelXMin + ( labelXMax - labelXMin - lineWidth ) / 2.0, labelY, *itemPart, symbolLabelFont );
          break;
      }

      if ( itemPart != ( lines.end() - 1 ) )
        labelY += textDescent + settings.lineSpacing() + textHeight;
    }
  }

  return labelSize;
}

// -------------------------------------------------------------------------

QgsSymbolLegendNode::QgsSymbolLegendNode( QgsLayerTreeLayer *nodeLayer, const QgsLegendSymbolItem &item, QObject *parent )
  : QgsLayerTreeModelLegendNode( nodeLayer, parent )
  , mItem( item )
  , mSymbolUsesMapUnits( false )
{
  const int iconSize = QgsLayerTreeModel::scaleIconSize( 16 );
  mIconSize = QSize( iconSize, iconSize );

  updateLabel();
  connect( qobject_cast<QgsVectorLayer *>( nodeLayer->layer() ), &QgsVectorLayer::symbolFeatureCountMapChanged, this, &QgsSymbolLegendNode::updateLabel );
  connect( nodeLayer, &QObject::destroyed, this, [ = ]() { mLayerNode = nullptr; } );

  if ( mItem.symbol() )
    mSymbolUsesMapUnits = ( mItem.symbol()->outputUnit() != QgsUnitTypes::RenderMillimeters );
}

Qt::ItemFlags QgsSymbolLegendNode::flags() const
{
  if ( mItem.isCheckable() )
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  else
    return Qt::ItemIsEnabled;
}


QSize QgsSymbolLegendNode::minimumIconSize() const
{
  std::unique_ptr<QgsRenderContext> context( createTemporaryRenderContext() );
  return minimumIconSize( context.get() );
}

QSize QgsSymbolLegendNode::minimumIconSize( QgsRenderContext *context ) const
{
  const int iconSize = QgsLayerTreeModel::scaleIconSize( 16 );
  const int largeIconSize = QgsLayerTreeModel::scaleIconSize( 512 );
  QSize minSz( iconSize, iconSize );
  if ( mItem.symbol() && mItem.symbol()->type() == QgsSymbol::Marker )
  {
    minSz = QgsImageOperation::nonTransparentImageRect(
              QgsSymbolLayerUtils::symbolPreviewPixmap( mItem.symbol(), QSize( largeIconSize, largeIconSize ), 0,
                  context ).toImage(),
              minSz,
              true ).size();
  }
  else if ( mItem.symbol() && mItem.symbol()->type() == QgsSymbol::Line )
  {
    minSz = QgsImageOperation::nonTransparentImageRect(
              QgsSymbolLayerUtils::symbolPreviewPixmap( mItem.symbol(), QSize( minSz.width(), largeIconSize ), 0,
                  context ).toImage(),
              minSz,
              true ).size();
  }

  if ( !mTextOnSymbolLabel.isEmpty() && context )
  {
    double w = QgsTextRenderer::textWidth( *context, mTextOnSymbolTextFormat, QStringList() << mTextOnSymbolLabel );
    double h = QgsTextRenderer::textHeight( *context, mTextOnSymbolTextFormat, QStringList() << mTextOnSymbolLabel, QgsTextRenderer::Point );
    int wInt = ceil( w ), hInt = ceil( h );
    if ( wInt > minSz.width() ) minSz.setWidth( wInt );
    if ( hInt > minSz.height() ) minSz.setHeight( hInt );
  }

  return minSz;
}

const QgsSymbol *QgsSymbolLegendNode::symbol() const
{
  return mItem.symbol();
}

QString QgsSymbolLegendNode::symbolLabel() const
{
  QString label;
  if ( mEmbeddedInParent )
  {
    QVariant legendlabel = mLayerNode->customProperty( QStringLiteral( "legend/title-label" ) );
    QString layerName = legendlabel.isNull() ? mLayerNode->name() : legendlabel.toString();
    label = mUserLabel.isEmpty() ? layerName : mUserLabel;
  }
  else
    label = mUserLabel.isEmpty() ? mItem.label() : mUserLabel;
  return label;
}

QgsLegendPatchShape QgsSymbolLegendNode::patchShape() const
{
  if ( mEmbeddedInParent )
  {
    return mLayerNode->patchShape();
  }
  else
  {
    return mPatchShape;
  }
}

void QgsSymbolLegendNode::setPatchShape( const QgsLegendPatchShape &shape )
{
  mPatchShape = shape;
}

QgsSymbol *QgsSymbolLegendNode::customSymbol() const
{
  return mCustomSymbol.get();
}

void QgsSymbolLegendNode::setCustomSymbol( QgsSymbol *symbol )
{
  mCustomSymbol.reset( symbol );
}

void QgsSymbolLegendNode::setSymbol( QgsSymbol *symbol )
{
  if ( !symbol )
    return;

  std::unique_ptr< QgsSymbol > s( symbol ); // this method takes ownership of symbol
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayerNode->layer() );
  if ( !vlayer || !vlayer->renderer() )
    return;

  mItem.setSymbol( s.get() ); // doesn't transfer ownership
  vlayer->renderer()->setLegendSymbolItem( mItem.ruleKey(), s.release() ); // DOES transfer ownership!

  mPixmap = QPixmap();

  emit dataChanged();
  vlayer->triggerRepaint();
}

void QgsSymbolLegendNode::checkAllItems()
{
  checkAll( true );
}

void QgsSymbolLegendNode::uncheckAllItems()
{
  checkAll( false );
}

void QgsSymbolLegendNode::toggleAllItems()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayerNode->layer() );
  if ( !vlayer || !vlayer->renderer() )
    return;

  const QgsLegendSymbolList symbolList = vlayer->renderer()->legendSymbolItems();
  for ( const auto &item : symbolList )
  {
    vlayer->renderer()->checkLegendSymbolItem( item.ruleKey(), ! vlayer->renderer()->legendSymbolItemChecked( item.ruleKey() ) );
  }

  emit dataChanged();
  vlayer->triggerRepaint();
}

QgsRenderContext *QgsLayerTreeModelLegendNode::createTemporaryRenderContext() const
{
  double scale = 0.0;
  double mupp = 0.0;
  int dpi = 0;
  if ( model() )
    model()->legendMapViewData( &mupp, &dpi, &scale );

  if ( qgsDoubleNear( mupp, 0.0 ) || dpi == 0 || qgsDoubleNear( scale, 0.0 ) )
    return nullptr;

  // setup temporary render context
  std::unique_ptr<QgsRenderContext> context = qgis::make_unique<QgsRenderContext>( );
  context->setScaleFactor( dpi / 25.4 );
  context->setRendererScale( scale );
  context->setMapToPixel( QgsMapToPixel( mupp ) );
  context->setFlag( QgsRenderContext::Antialiasing, true );
  context->setFlag( QgsRenderContext::RenderSymbolPreview, true );
  return context.release();
}

void QgsSymbolLegendNode::checkAll( bool state )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayerNode->layer() );
  if ( !vlayer || !vlayer->renderer() )
    return;

  const QgsLegendSymbolList symbolList = vlayer->renderer()->legendSymbolItems();
  for ( const auto &item : symbolList )
  {
    vlayer->renderer()->checkLegendSymbolItem( item.ruleKey(), state );
  }

  emit dataChanged();
  vlayer->triggerRepaint();
}

QVariant QgsSymbolLegendNode::data( int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    return mLabel;
  }
  else if ( role == Qt::EditRole )
  {
    return mUserLabel.isEmpty() ? mItem.label() : mUserLabel;
  }
  else if ( role == Qt::DecorationRole )
  {
    if ( mPixmap.isNull() || mPixmap.size() != mIconSize )
    {
      QPixmap pix;
      if ( mItem.symbol() )
      {
        std::unique_ptr<QgsRenderContext> context( createTemporaryRenderContext() );
        pix = QgsSymbolLayerUtils::symbolPreviewPixmap( mItem.symbol(), mIconSize, 0, context.get() );

        if ( !mTextOnSymbolLabel.isEmpty() && context )
        {
          QPainter painter( &pix );
          painter.setRenderHint( QPainter::Antialiasing );
          context->setPainter( &painter );
          QFontMetricsF fm( mTextOnSymbolTextFormat.scaledFont( *context ) );
          qreal yBaselineVCenter = ( mIconSize.height() + fm.ascent() - fm.descent() ) / 2;
          QgsTextRenderer::drawText( QPointF( mIconSize.width() / 2, yBaselineVCenter ), 0, QgsTextRenderer::AlignCenter,
                                     QStringList() << mTextOnSymbolLabel, *context, mTextOnSymbolTextFormat );
        }
      }
      else
      {
        pix = QPixmap( mIconSize );
        pix.fill( Qt::transparent );
      }

      mPixmap = pix;
    }
    return mPixmap;
  }
  else if ( role == Qt::CheckStateRole )
  {
    if ( !mItem.isCheckable() )
      return QVariant();

    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayerNode->layer() );
    if ( !vlayer || !vlayer->renderer() )
      return QVariant();

    return vlayer->renderer()->legendSymbolItemChecked( mItem.ruleKey() ) ? Qt::Checked : Qt::Unchecked;
  }
  else if ( role == RuleKeyRole )
  {
    return mItem.ruleKey();
  }
  else if ( role == ParentRuleKeyRole )
  {
    return mItem.parentRuleKey();
  }

  return QVariant();
}

bool QgsSymbolLegendNode::setData( const QVariant &value, int role )
{
  if ( role != Qt::CheckStateRole )
    return false;

  if ( !mItem.isCheckable() )
    return false;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayerNode->layer() );
  if ( !vlayer || !vlayer->renderer() )
    return false;

  vlayer->renderer()->checkLegendSymbolItem( mItem.ruleKey(), value == Qt::Checked );

  emit dataChanged();
  vlayer->emitStyleChanged();

  vlayer->triggerRepaint();

  return true;
}



QSizeF QgsSymbolLegendNode::drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const
{
  QgsSymbol *s = mCustomSymbol ? mCustomSymbol.get() : mItem.symbol();
  if ( !s )
  {
    return QSizeF();
  }

  // setup temporary render context
  QgsRenderContext *context = nullptr;
  std::unique_ptr< QgsRenderContext > tempRenderContext;
  QgsLegendPatchShape patchShape = ctx ? ctx->patchShape : QgsLegendPatchShape();
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

  //Consider symbol size for point markers
  const double desiredHeight = ctx && ctx->patchSize.height() > 0 ? ctx->patchSize.height() : settings.symbolSize().height();
  const double desiredWidth = ctx && ctx->patchSize.width() > 0 ? ctx->patchSize.width() : settings.symbolSize().width();
  double height = desiredHeight;
  double width = desiredWidth;

  //Center small marker symbols
  double widthOffset = 0;
  double heightOffset = 0;

  if ( QgsMarkerSymbol *markerSymbol = dynamic_cast<QgsMarkerSymbol *>( s ) )
  {
    // allow marker symbol to occupy bigger area if necessary
    double size = markerSymbol->size( *context ) / context->scaleFactor();
    height = size;
    width = size;
    if ( width < desiredWidth )
    {
      widthOffset = ( desiredWidth - width ) / 2.0;
    }
    if ( height < desiredHeight )
    {
      heightOffset = ( desiredHeight - height ) / 2.0;
    }
  }

  if ( ctx && ctx->painter )
  {
    double currentYCoord = ctx->top + ( itemHeight - desiredHeight ) / 2;
    QPainter *p = ctx->painter;

    //setup painter scaling to dots so that raster symbology is drawn to scale
    double dotsPerMM = context->scaleFactor();

    int opacity = 255;
    if ( QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layerNode()->layer() ) )
      opacity = static_cast<int >( std::round( 255 * vectorLayer->opacity() ) );

    p->save();
    if ( context->flags() & QgsRenderContext::Antialiasing )
      p->setRenderHint( QPainter::Antialiasing );

    switch ( settings.symbolAlignment() )
    {
      case Qt::AlignLeft:
      default:
        p->translate( ctx->columnLeft + widthOffset, currentYCoord + heightOffset );
        break;
      case Qt::AlignRight:
        p->translate( ctx->columnRight - widthOffset - width, currentYCoord + heightOffset );
        break;
    }

    p->scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );
    Q_NOWARN_DEPRECATED_PUSH
    // QGIS 4.0 -- ctx->context will be mandatory
    const bool useAdvancedEffects = ctx->context ? ctx->context->flags() & QgsRenderContext::UseAdvancedEffects : settings.useAdvancedEffects();
    Q_NOWARN_DEPRECATED_POP
    if ( opacity != 255 && useAdvancedEffects )
    {
      const int maxBleed = static_cast< int >( std::ceil( QgsSymbolLayerUtils::estimateMaxSymbolBleed( s, *context ) ) );

      //semi transparent layer, so need to draw symbol to an image (to flatten it first)
      //create image which is same size as legend rect, in case symbol bleeds outside its allotted space
      const QSize symbolSize( static_cast< int >( std::round( width * dotsPerMM ) ), static_cast<int >( std::round( height * dotsPerMM ) ) );
      const QSize tempImageSize( symbolSize.width() + maxBleed * 2, symbolSize.height() + maxBleed * 2 );
      QImage tempImage = QImage( tempImageSize, QImage::Format_ARGB32 );
      tempImage.fill( Qt::transparent );
      QPainter imagePainter( &tempImage );
      if ( context->flags() & QgsRenderContext::Antialiasing )
        imagePainter.setRenderHint( QPainter::Antialiasing );
      context->setPainter( &imagePainter );
      imagePainter.translate( maxBleed, maxBleed );
      s->drawPreviewIcon( &imagePainter, symbolSize, context, false, nullptr, &patchShape );
      imagePainter.translate( -maxBleed, -maxBleed );
      context->setPainter( ctx->painter );
      //reduce opacity of image
      imagePainter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
      imagePainter.fillRect( tempImage.rect(), QColor( 0, 0, 0, opacity ) );
      imagePainter.end();
      //draw rendered symbol image
      p->drawImage( -maxBleed, -maxBleed, tempImage );
    }
    else
    {
      s->drawPreviewIcon( p, QSize( static_cast< int >( std::round( width * dotsPerMM ) ), static_cast< int >( std::round( height * dotsPerMM ) ) ), context, false, nullptr, &patchShape );
    }

    if ( !mTextOnSymbolLabel.isEmpty() )
    {
      QFontMetricsF fm( mTextOnSymbolTextFormat.scaledFont( *context ) );
      qreal yBaselineVCenter = ( height * dotsPerMM + fm.ascent() - fm.descent() ) / 2;
      QgsTextRenderer::drawText( QPointF( width * dotsPerMM / 2, yBaselineVCenter ), 0, QgsTextRenderer::AlignCenter,
                                 QStringList() << mTextOnSymbolLabel, *context, mTextOnSymbolTextFormat );
    }

    p->restore();
  }

  return QSizeF( std::max( width + 2 * widthOffset, static_cast< double >( desiredWidth ) ),
                 std::max( height + 2 * heightOffset, static_cast< double >( desiredHeight ) ) );
}

QJsonObject QgsSymbolLegendNode::exportSymbolToJson( const QgsLegendSettings &settings, const QgsRenderContext &context ) const
{
  const QgsSymbol *s = mCustomSymbol ? mCustomSymbol.get() : mItem.symbol();
  if ( !s )
  {
    return QJsonObject();
  }


  QgsRenderContext ctx;
  // QGIS 4.0 - use render context directly here, and note in the dox that the context must be correctly setup
  Q_NOWARN_DEPRECATED_PUSH
  ctx.setScaleFactor( settings.dpi() / 25.4 );
  ctx.setRendererScale( settings.mapScale() );
  ctx.setMapToPixel( QgsMapToPixel( 1 / ( settings.mmPerMapUnit() * ctx.scaleFactor() ) ) );
  ctx.setForceVectorOutput( true );
  ctx.setFlag( QgsRenderContext::Antialiasing, context.flags() & QgsRenderContext::Antialiasing );
  Q_NOWARN_DEPRECATED_POP

  // ensure that a minimal expression context is available
  QgsExpressionContext expContext = context.expressionContext();
  expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) );
  ctx.setExpressionContext( expContext );

  const QPixmap pix = QgsSymbolLayerUtils::symbolPreviewPixmap( mItem.symbol(), minimumIconSize(), 0, &ctx );
  QImage img( pix.toImage().convertToFormat( QImage::Format_ARGB32_Premultiplied ) );

  int opacity = 255;
  if ( QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layerNode()->layer() ) )
    opacity = ( 255 * vectorLayer->opacity() );

  if ( opacity != 255 )
  {
    QPainter painter;
    painter.begin( &img );
    painter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
    painter.fillRect( pix.rect(), QColor( 0, 0, 0, opacity ) );
    painter.end();
  }

  QByteArray byteArray;
  QBuffer buffer( &byteArray );
  img.save( &buffer, "PNG" );
  const QString base64 = QString::fromLatin1( byteArray.toBase64().data() );

  QJsonObject json;
  json[ QStringLiteral( "icon" ) ] = base64;
  return json;
}

void QgsSymbolLegendNode::setEmbeddedInParent( bool embedded )
{
  QgsLayerTreeModelLegendNode::setEmbeddedInParent( embedded );
  updateLabel();
}


void QgsSymbolLegendNode::invalidateMapBasedData()
{
  if ( mSymbolUsesMapUnits )
  {
    mPixmap = QPixmap();
    emit dataChanged();
  }
}


void QgsSymbolLegendNode::updateLabel()
{
  if ( !mLayerNode )
    return;

  bool showFeatureCount = mLayerNode->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toBool();
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayerNode->layer() );
  mLabel = symbolLabel();

  if ( showFeatureCount && vl )
  {
    qlonglong count = mEmbeddedInParent ? vl->featureCount() : vl->featureCount( mItem.ruleKey() ) ;
    mLabel += QStringLiteral( " [%1]" ).arg( count != -1 ? QLocale().toString( count ) : tr( "N/A" ) );
  }

  emit dataChanged();
}

QString QgsSymbolLegendNode::evaluateLabel( const QgsExpressionContext &context, const QString &label )
{
  if ( !mLayerNode )
    return QString();

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayerNode->layer() );

  if ( vl )
  {
    QgsExpressionContext contextCopy = QgsExpressionContext( context );
    QgsExpressionContextScope *symbolScope = createSymbolScope();
    contextCopy.appendScope( symbolScope );
    contextCopy.appendScope( vl->createExpressionContextScope() );

    if ( label.isEmpty() )
    {
      if ( ! mLayerNode->labelExpression().isEmpty() )
        mLabel = QgsExpression::replaceExpressionText( "[%" + mLayerNode->labelExpression() + "%]", &contextCopy );
      else if ( mLabel.contains( "[%" ) )
      {
        const QString symLabel = symbolLabel();
        mLabel = QgsExpression::replaceExpressionText( symLabel, &contextCopy );
      }
      return mLabel;
    }
    else
    {
      QString eLabel;
      if ( ! mLayerNode->labelExpression().isEmpty() )
        eLabel = QgsExpression::replaceExpressionText( label + "[%" + mLayerNode->labelExpression() + "%]", &contextCopy );
      else if ( label.contains( "[%" ) )
        eLabel = QgsExpression::replaceExpressionText( label, &contextCopy );
      return eLabel;
    }
  }
  return mLabel;
}

QgsExpressionContextScope *QgsSymbolLegendNode::createSymbolScope() const
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayerNode->layer() );

  QgsExpressionContextScope *scope = new QgsExpressionContextScope( tr( "Symbol scope" ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_label" ), symbolLabel().remove( "[%" ).remove( "%]" ), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_id" ), mItem.ruleKey(), true ) );
  if ( vl )
  {
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_count" ), QVariant::fromValue( vl->featureCount( mItem.ruleKey() ) ), true ) );
  }
  return scope;
}

// -------------------------------------------------------------------------


QgsSimpleLegendNode::QgsSimpleLegendNode( QgsLayerTreeLayer *nodeLayer, const QString &label, const QIcon &icon, QObject *parent, const QString &key )
  : QgsLayerTreeModelLegendNode( nodeLayer, parent )
  , mLabel( label )
  , mIcon( icon )
  , mKey( key )
{
}

QVariant QgsSimpleLegendNode::data( int role ) const
{
  if ( role == Qt::DisplayRole || role == Qt::EditRole )
    return mUserLabel.isEmpty() ? mLabel : mUserLabel;
  else if ( role == Qt::DecorationRole )
    return mIcon;
  else if ( role == RuleKeyRole && !mKey.isEmpty() )
    return mKey;
  else
    return QVariant();
}


// -------------------------------------------------------------------------

QgsImageLegendNode::QgsImageLegendNode( QgsLayerTreeLayer *nodeLayer, const QImage &img, QObject *parent )
  : QgsLayerTreeModelLegendNode( nodeLayer, parent )
  , mImage( img )
{
}

QVariant QgsImageLegendNode::data( int role ) const
{
  if ( role == Qt::DecorationRole )
  {
    return QPixmap::fromImage( mImage );
  }
  else if ( role == Qt::SizeHintRole )
  {
    return mImage.size();
  }
  return QVariant();
}

QSizeF QgsImageLegendNode::drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const
{
  Q_UNUSED( itemHeight )

  if ( ctx && ctx->painter )
  {
    switch ( settings.symbolAlignment() )
    {
      case Qt::AlignLeft:
      default:
        ctx->painter->drawImage( QRectF( ctx->columnLeft, ctx->top, settings.wmsLegendSize().width(), settings.wmsLegendSize().height() ),
                                 mImage, QRectF( 0, 0, mImage.width(), mImage.height() ) );
        break;

      case Qt::AlignRight:
        ctx->painter->drawImage( QRectF( ctx->columnRight - settings.wmsLegendSize().width(), ctx->top, settings.wmsLegendSize().width(), settings.wmsLegendSize().height() ),
                                 mImage, QRectF( 0, 0, mImage.width(), mImage.height() ) );
        break;
    }
  }
  return settings.wmsLegendSize();
}

QJsonObject QgsImageLegendNode::exportSymbolToJson( const QgsLegendSettings &, const QgsRenderContext & ) const
{
  QByteArray byteArray;
  QBuffer buffer( &byteArray );
  mImage.save( &buffer, "PNG" );
  const QString base64 = QString::fromLatin1( byteArray.toBase64().data() );

  QJsonObject json;
  json[ QStringLiteral( "icon" ) ] = base64;
  return json;
}

// -------------------------------------------------------------------------

QgsRasterSymbolLegendNode::QgsRasterSymbolLegendNode( QgsLayerTreeLayer *nodeLayer, const QColor &color, const QString &label, QObject *parent )
  : QgsLayerTreeModelLegendNode( nodeLayer, parent )
  , mColor( color )
  , mLabel( label )
{
}

QVariant QgsRasterSymbolLegendNode::data( int role ) const
{
  if ( role == Qt::DecorationRole )
  {
    const int iconSize = QgsLayerTreeModel::scaleIconSize( 16 ); // TODO: configurable?
    QPixmap pix( iconSize, iconSize );
    pix.fill( mColor );
    return QIcon( pix );
  }
  else if ( role == Qt::DisplayRole || role == Qt::EditRole )
    return mUserLabel.isEmpty() ? mLabel : mUserLabel;
  else
    return QVariant();
}


QSizeF QgsRasterSymbolLegendNode::drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const
{
  QSizeF size = settings.symbolSize();
  double offsetX = 0;
  if ( ctx )
  {
    if ( ctx->patchSize.width() > 0 )
    {
      if ( ctx->patchSize.width() < size.width() )
        offsetX = ( size.width() - ctx->patchSize.width() ) / 2.0;
      size.setWidth( ctx->patchSize.width() );
    }
    if ( ctx->patchSize.height() > 0 )
    {
      size.setHeight( ctx->patchSize.height() );
    }
  }

  if ( ctx && ctx->painter )
  {
    QColor itemColor = mColor;
    if ( QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( layerNode()->layer() ) )
    {
      if ( QgsRasterRenderer *rasterRenderer = rasterLayer->renderer() )
        itemColor.setAlpha( rasterRenderer->opacity() * 255.0 );
    }
    ctx->painter->setBrush( itemColor );

    if ( settings.drawRasterStroke() )
    {
      QPen pen;
      pen.setColor( settings.rasterStrokeColor() );
      pen.setWidthF( settings.rasterStrokeWidth() );
      pen.setJoinStyle( Qt::MiterJoin );
      ctx->painter->setPen( pen );
    }
    else
    {
      ctx->painter->setPen( Qt::NoPen );
    }

    switch ( settings.symbolAlignment() )
    {
      case Qt::AlignLeft:
      default:
        ctx->painter->drawRect( QRectF( ctx->columnLeft + offsetX, ctx->top + ( itemHeight - size.height() ) / 2,
                                        size.width(), size.height() ) );
        break;

      case Qt::AlignRight:
        ctx->painter->drawRect( QRectF( ctx->columnRight - size.width() - offsetX, ctx->top + ( itemHeight - size.height() ) / 2,
                                        size.width(), size.height() ) );
        break;
    }
  }
  return size;
}

QJsonObject QgsRasterSymbolLegendNode::exportSymbolToJson( const QgsLegendSettings &settings, const QgsRenderContext & ) const
{
  QImage img = QImage( settings.symbolSize().toSize(), QImage::Format_ARGB32 );
  img.fill( Qt::transparent );

  QPainter painter( &img );
  painter.setRenderHint( QPainter::Antialiasing );

  QColor itemColor = mColor;
  if ( QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( layerNode()->layer() ) )
  {
    if ( QgsRasterRenderer *rasterRenderer = rasterLayer->renderer() )
      itemColor.setAlpha( rasterRenderer->opacity() * 255.0 );
  }
  painter.setBrush( itemColor );

  if ( settings.drawRasterStroke() )
  {
    QPen pen;
    pen.setColor( settings.rasterStrokeColor() );
    pen.setWidthF( settings.rasterStrokeWidth() );
    pen.setJoinStyle( Qt::MiterJoin );
    painter.setPen( pen );
  }
  else
  {
    painter.setPen( Qt::NoPen );
  }

  painter.drawRect( QRectF( 0, 0, settings.symbolSize().width(), settings.symbolSize().height() ) );

  QByteArray byteArray;
  QBuffer buffer( &byteArray );
  img.save( &buffer, "PNG" );
  const QString base64 = QString::fromLatin1( byteArray.toBase64().data() );

  QJsonObject json;
  json[ QStringLiteral( "icon" ) ] = base64;
  return json;
}

// -------------------------------------------------------------------------

QgsWmsLegendNode::QgsWmsLegendNode( QgsLayerTreeLayer *nodeLayer, QObject *parent )
  : QgsLayerTreeModelLegendNode( nodeLayer, parent )
  , mValid( false )
{
}

QImage QgsWmsLegendNode::getLegendGraphic() const
{
  if ( ! mValid && ! mFetcher )
  {
    // or maybe in presence of a downloader we should just delete it
    // and start a new one ?

    QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( mLayerNode->layer() );
    const QgsLayerTreeModel *mod = model();
    if ( ! mod )
      return mImage;
    const QgsMapSettings *ms = mod->legendFilterMapSettings();

    QgsRasterDataProvider *prov = layer->dataProvider();
    if ( ! prov )
      return mImage;

    Q_ASSERT( ! mFetcher );
    mFetcher.reset( prov->getLegendGraphicFetcher( ms ) );
    if ( mFetcher )
    {
      connect( mFetcher.get(), &QgsImageFetcher::finish, this, &QgsWmsLegendNode::getLegendGraphicFinished );
      connect( mFetcher.get(), &QgsImageFetcher::error, this, &QgsWmsLegendNode::getLegendGraphicErrored );
      connect( mFetcher.get(), &QgsImageFetcher::progress, this, &QgsWmsLegendNode::getLegendGraphicProgress );
      mFetcher->start();
    } // else QgsDebugMsg("XXX No legend supported?");

  }

  return mImage;
}

QVariant QgsWmsLegendNode::data( int role ) const
{
  //QgsDebugMsg( QStringLiteral("XXX data called with role %1 -- mImage size is %2x%3").arg(role).arg(mImage.width()).arg(mImage.height()) );

  if ( role == Qt::DecorationRole )
  {
    return QPixmap::fromImage( getLegendGraphic() );
  }
  else if ( role == Qt::SizeHintRole )
  {
    return getLegendGraphic().size();
  }
  return QVariant();
}

QSizeF QgsWmsLegendNode::drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const
{
  Q_UNUSED( itemHeight )

  if ( ctx && ctx->painter )
  {
    switch ( settings.symbolAlignment() )
    {
      case Qt::AlignLeft:
      default:
        ctx->painter->drawImage( QRectF( ctx->columnLeft,
                                         ctx->top,
                                         settings.wmsLegendSize().width(),
                                         settings.wmsLegendSize().height() ),
                                 mImage,
                                 QRectF( QPointF( 0, 0 ), mImage.size() ) );
        break;

      case Qt::AlignRight:
        ctx->painter->drawImage( QRectF( ctx->columnRight - settings.wmsLegendSize().width(),
                                         ctx->top,
                                         settings.wmsLegendSize().width(),
                                         settings.wmsLegendSize().height() ),
                                 mImage,
                                 QRectF( QPointF( 0, 0 ), mImage.size() ) );
        break;
    }
  }
  return settings.wmsLegendSize();
}

QJsonObject QgsWmsLegendNode::exportSymbolToJson( const QgsLegendSettings &, const QgsRenderContext & ) const
{
  QByteArray byteArray;
  QBuffer buffer( &byteArray );
  mImage.save( &buffer, "PNG" );
  const QString base64 = QString::fromLatin1( byteArray.toBase64().data() );

  QJsonObject json;
  json[ QStringLiteral( "icon" ) ] = base64;
  return json;
}

/* private */
QImage QgsWmsLegendNode::renderMessage( const QString &msg ) const
{
  const int fontHeight = 10;
  const int margin = fontHeight / 2;
  const int nlines = 1;

  const int w = 512, h = fontHeight * nlines + margin * ( nlines + 1 );
  QImage image( w, h, QImage::Format_ARGB32_Premultiplied );
  QPainter painter;
  painter.begin( &image );
  painter.setPen( QColor( 255, 0, 0 ) );
  painter.setFont( QFont( QStringLiteral( "Chicago" ), fontHeight ) );
  painter.fillRect( 0, 0, w, h, QColor( 255, 255, 255 ) );
  painter.drawText( 0, margin + fontHeight, msg );
  //painter.drawText(0,2*(margin+fontHeight),tr("retrying in 5 seconds…"));
  painter.end();

  return image;
}

void QgsWmsLegendNode::getLegendGraphicProgress( qint64 cur, qint64 tot )
{
  QString msg = QStringLiteral( "Downloading... %1/%2" ).arg( cur ).arg( tot );
  //QgsDebugMsg ( QString("XXX %1").arg(msg) );
  mImage = renderMessage( msg );
  emit dataChanged();
}

void QgsWmsLegendNode::getLegendGraphicErrored( const QString &msg )
{
  if ( ! mFetcher ) return; // must be coming after finish

  mImage = renderMessage( msg );
  //QgsDebugMsg( QStringLiteral("XXX emitting dataChanged after writing an image of %1x%2").arg(mImage.width()).arg(mImage.height()) );

  emit dataChanged();

  mFetcher.reset();

  mValid = true; // we consider it valid anyway
  // ... but remove validity after 5 seconds
  //QTimer::singleShot(5000, this, SLOT(invalidateMapBasedData()));
}

void QgsWmsLegendNode::getLegendGraphicFinished( const QImage &image )
{
  if ( ! mFetcher ) return; // must be coming after error

  //QgsDebugMsg( QStringLiteral("XXX legend graphic finished, image is %1x%2").arg(theImage.width()).arg(theImage.height()) );
  if ( ! image.isNull() )
  {
    if ( image != mImage )
    {
      mImage = image;
      //QgsDebugMsg( QStringLiteral("XXX emitting dataChanged") );
      emit dataChanged();
    }
    mValid = true; // only if not null I guess
  }
  mFetcher.reset();
}

void QgsWmsLegendNode::invalidateMapBasedData()
{
  //QgsDebugMsg( QStringLiteral("XXX invalidateMapBasedData called") );
  // TODO: do this only if this extent != prev extent ?
  mValid = false;
  emit dataChanged();
}

// -------------------------------------------------------------------------

QgsDataDefinedSizeLegendNode::QgsDataDefinedSizeLegendNode( QgsLayerTreeLayer *nodeLayer, const QgsDataDefinedSizeLegend &settings, QObject *parent )
  : QgsLayerTreeModelLegendNode( nodeLayer, parent )
  , mSettings( new QgsDataDefinedSizeLegend( settings ) )
{
}

QgsDataDefinedSizeLegendNode::~QgsDataDefinedSizeLegendNode()
{
  delete mSettings;
}

QVariant QgsDataDefinedSizeLegendNode::data( int role ) const
{
  if ( role == Qt::DecorationRole )
  {
    cacheImage();
    return QPixmap::fromImage( mImage );
  }
  else if ( role == Qt::SizeHintRole )
  {
    cacheImage();
    return mImage.size();
  }
  return QVariant();
}

QgsLayerTreeModelLegendNode::ItemMetrics QgsDataDefinedSizeLegendNode::draw( const QgsLegendSettings &settings, QgsLayerTreeModelLegendNode::ItemContext *ctx )
{
  // setup temporary render context if none specified
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
    tempRenderContext->setForceVectorOutput( true );
    tempRenderContext->setPainter( ctx ? ctx->painter : nullptr );
    tempRenderContext->setFlag( QgsRenderContext::Antialiasing, true );
    Q_NOWARN_DEPRECATED_POP

    // setup a minimal expression context
    QgsExpressionContext expContext;
    expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) );
    tempRenderContext->setExpressionContext( expContext );
    context = tempRenderContext.get();
  }

  if ( context->painter() )
  {
    context->painter()->save();
    context->painter()->translate( ctx->columnLeft, ctx->top );

    // scale to pixels
    context->painter()->scale( 1 / context->scaleFactor(), 1 / context->scaleFactor() );
  }

  QgsDataDefinedSizeLegend ddsLegend( *mSettings );
  ddsLegend.setFont( settings.style( QgsLegendStyle::SymbolLabel ).font() );
  ddsLegend.setTextColor( settings.fontColor() );

  QSizeF contentSize;
  double labelXOffset;
  ddsLegend.drawCollapsedLegend( *context, &contentSize, &labelXOffset );

  if ( context->painter() )
    context->painter()->restore();

  ItemMetrics im;
  im.symbolSize = QSizeF( ( contentSize.width() - labelXOffset ) / context->scaleFactor(), contentSize.height() / context->scaleFactor() );
  im.labelSize = QSizeF( labelXOffset / context->scaleFactor(), contentSize.height() / context->scaleFactor() );
  return im;
}


void QgsDataDefinedSizeLegendNode::cacheImage() const
{
  if ( mImage.isNull() )
  {
    std::unique_ptr<QgsRenderContext> context( createTemporaryRenderContext() );
    if ( !context )
    {
      context.reset( new QgsRenderContext );
      Q_ASSERT( context ); // to make cppcheck happy
      context->setScaleFactor( 96 / 25.4 );
    }
    mImage = mSettings->collapsedLegendImage( *context );
  }
}
