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
  Q_UNUSED( value );
  Q_UNUSED( role );
  return false;
}


QgsLayerTreeModelLegendNode::ItemMetrics QgsLayerTreeModelLegendNode::draw( const QgsLegendSettings &settings, ItemContext *ctx )
{
  QFont symbolLabelFont = settings.style( QgsLegendStyle::SymbolLabel ).font();

  double textHeight = settings.fontHeightCharacterMM( symbolLabelFont, QChar( '0' ) );
  // itemHeight here is not really item height, it is only for symbol
  // vertical alignment purpose, i.e. OK take single line height
  // if there are more lines, thos run under the symbol
  double itemHeight = std::max( static_cast< double >( settings.symbolSize().height() ), textHeight );

  ItemMetrics im;
  im.symbolSize = drawSymbol( settings, ctx, itemHeight );
  im.labelSize = drawSymbolText( settings, ctx, im.symbolSize );
  return im;
}


QSizeF QgsLayerTreeModelLegendNode::drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const
{
  QIcon symbolIcon = data( Qt::DecorationRole ).value<QIcon>();
  if ( symbolIcon.isNull() )
    return QSizeF();

  if ( ctx && ctx->painter )
    symbolIcon.paint( ctx->painter, ctx->point.x(), ctx->point.y() + ( itemHeight - settings.symbolSize().height() ) / 2,
                      settings.symbolSize().width(), settings.symbolSize().height() );
  return settings.symbolSize();
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

  double labelX = 0.0, labelY = 0.0;
  if ( ctx && ctx->painter )
  {
    ctx->painter->setPen( settings.fontColor() );

    labelX = ctx->point.x() + std::max( static_cast< double >( symbolSize.width() ), ctx->labelXOffset );
    labelY = ctx->point.y();

    // Vertical alignment of label with symbol
    if ( labelSize.height() < symbolSize.height() )
      labelY += symbolSize.height() / 2 - labelSize.height() / 2;  // label centered with symbol

    labelY += textHeight;
  }

  for ( QStringList::ConstIterator itemPart = lines.constBegin(); itemPart != lines.constEnd(); ++itemPart )
  {
    labelSize.rwidth() = std::max( settings.textWidthMillimeters( symbolLabelFont, *itemPart ), double( labelSize.width() ) );

    if ( ctx && ctx->painter )
    {
      settings.drawText( ctx->painter, labelX, labelY, *itemPart, symbolLabelFont );
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

  if ( mItem.level() != 0 && !( model() && model()->testFlag( QgsLayerTreeModel::ShowLegendAsTree ) ) )
    minSz.setWidth( mItem.level() * INDENT_SIZE + minSz.width() );

  return minSz;
}

const QgsSymbol *QgsSymbolLegendNode::symbol() const
{
  return mItem.symbol();
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

      if ( mItem.level() == 0 || ( model() && model()->testFlag( QgsLayerTreeModel::ShowLegendAsTree ) ) )
        mPixmap = pix;
      else
      {
        // ident the symbol icon to make it look like a tree structure
        QPixmap pix2( pix.width() + mItem.level() * INDENT_SIZE, pix.height() );
        pix2.fill( Qt::transparent );
        QPainter p( &pix2 );
        p.drawPixmap( mItem.level() * INDENT_SIZE, 0, pix );
        p.end();
        mPixmap = pix2;
      }
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
  QgsSymbol *s = mItem.symbol();
  if ( !s )
  {
    return QSizeF();
  }

  // setup temporary render context
  QgsRenderContext context;
  context.setScaleFactor( settings.dpi() / 25.4 );
  context.setRendererScale( settings.mapScale() );
  context.setMapToPixel( QgsMapToPixel( 1 / ( settings.mmPerMapUnit() * context.scaleFactor() ) ) );
  context.setForceVectorOutput( true );
  context.setPainter( ctx ? ctx->painter : nullptr );

  if ( ctx && ctx->context )
  {
    context.setExpressionContext( ctx->context->expressionContext() );
  }
  else
  {
    // setup a minimal expression context
    QgsExpressionContext expContext;
    expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) );
    context.setExpressionContext( expContext );
  }

  //Consider symbol size for point markers
  double height = settings.symbolSize().height();
  double width = settings.symbolSize().width();

  //Center small marker symbols
  double widthOffset = 0;
  double heightOffset = 0;

  if ( QgsMarkerSymbol *markerSymbol = dynamic_cast<QgsMarkerSymbol *>( s ) )
  {
    // allow marker symbol to occupy bigger area if necessary
    double size = markerSymbol->size( context ) / context.scaleFactor();
    height = size;
    width = size;
    if ( width < settings.symbolSize().width() )
    {
      widthOffset = ( settings.symbolSize().width() - width ) / 2.0;
    }
    if ( height < settings.symbolSize().height() )
    {
      heightOffset = ( settings.symbolSize().height() - height ) / 2.0;
    }
  }

  if ( ctx && ctx->painter )
  {
    double currentXPosition = ctx->point.x();
    double currentYCoord = ctx->point.y() + ( itemHeight - settings.symbolSize().height() ) / 2;
    QPainter *p = ctx->painter;

    //setup painter scaling to dots so that raster symbology is drawn to scale
    double dotsPerMM = context.scaleFactor();

    int opacity = 255;
    if ( QgsVectorLayer *vectorLayer = dynamic_cast<QgsVectorLayer *>( layerNode()->layer() ) )
      opacity = ( 255 * vectorLayer->opacity() );

    p->save();
    p->setRenderHint( QPainter::Antialiasing );
    p->translate( currentXPosition + widthOffset, currentYCoord + heightOffset );
    p->scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );
    if ( opacity != 255 && settings.useAdvancedEffects() )
    {
      //semi transparent layer, so need to draw symbol to an image (to flatten it first)
      //create image which is same size as legend rect, in case symbol bleeds outside its alloted space
      QSize tempImageSize( width * dotsPerMM, height * dotsPerMM );
      QImage tempImage = QImage( tempImageSize, QImage::Format_ARGB32 );
      tempImage.fill( Qt::transparent );
      QPainter imagePainter( &tempImage );
      context.setPainter( &imagePainter );
      s->drawPreviewIcon( &imagePainter, tempImageSize, &context );
      context.setPainter( ctx->painter );
      //reduce opacity of image
      imagePainter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
      imagePainter.fillRect( tempImage.rect(), QColor( 0, 0, 0, opacity ) );
      imagePainter.end();
      //draw rendered symbol image
      p->drawImage( 0, 0, tempImage );
    }
    else
    {
      s->drawPreviewIcon( p, QSize( width * dotsPerMM, height * dotsPerMM ), &context );
    }

    if ( !mTextOnSymbolLabel.isEmpty() )
    {
      QFontMetricsF fm( mTextOnSymbolTextFormat.scaledFont( context ) );
      qreal yBaselineVCenter = ( height * dotsPerMM + fm.ascent() - fm.descent() ) / 2;
      QgsTextRenderer::drawText( QPointF( width * dotsPerMM / 2, yBaselineVCenter ), 0, QgsTextRenderer::AlignCenter,
                                 QStringList() << mTextOnSymbolLabel, context, mTextOnSymbolTextFormat );
    }

    p->restore();
  }

  return QSizeF( std::max( width + 2 * widthOffset, static_cast< double >( settings.symbolSize().width() ) ),
                 std::max( height + 2 * heightOffset, static_cast< double >( settings.symbolSize().height() ) ) );
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

  if ( mEmbeddedInParent )
  {
    QString layerName = mLayerNode->name();
    if ( !mLayerNode->customProperty( QStringLiteral( "legend/title-label" ) ).isNull() )
      layerName = mLayerNode->customProperty( QStringLiteral( "legend/title-label" ) ).toString();

    mLabel = mUserLabel.isEmpty() ? layerName : mUserLabel;
    if ( showFeatureCount && vl && vl->featureCount() >= 0 )
      mLabel += QStringLiteral( " [%1]" ).arg( vl->featureCount() );
  }
  else
  {
    mLabel = mUserLabel.isEmpty() ? mItem.label() : mUserLabel;
    if ( showFeatureCount && vl )
    {
      qlonglong count = vl->featureCount( mItem.ruleKey() );
      mLabel += QStringLiteral( " [%1]" ).arg( count != -1 ? QLocale().toString( count ) : tr( "N/A" ) );
    }
  }

  emit dataChanged();
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
  Q_UNUSED( itemHeight );

  if ( ctx && ctx->painter )
  {
    ctx->painter->drawImage( QRectF( ctx->point.x(), ctx->point.y(), settings.wmsLegendSize().width(), settings.wmsLegendSize().height() ),
                             mImage, QRectF( 0, 0, mImage.width(), mImage.height() ) );
  }
  return settings.wmsLegendSize();
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
  if ( ctx && ctx->painter )
  {
    QColor itemColor = mColor;
    if ( QgsRasterLayer *rasterLayer = dynamic_cast<QgsRasterLayer *>( layerNode()->layer() ) )
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

    ctx->painter->drawRect( QRectF( ctx->point.x(), ctx->point.y() + ( itemHeight - settings.symbolSize().height() ) / 2,
                                    settings.symbolSize().width(), settings.symbolSize().height() ) );
  }
  return settings.symbolSize();
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
  Q_UNUSED( itemHeight );

  if ( ctx && ctx->painter )
  {
    ctx->painter->drawImage( QRectF( ctx->point, settings.wmsLegendSize() ),
                             mImage,
                             QRectF( QPointF( 0, 0 ), mImage.size() ) );
  }
  return settings.wmsLegendSize();
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
  // setup temporary render context
  QgsRenderContext context;
  context.setScaleFactor( settings.dpi() / 25.4 );
  context.setRendererScale( settings.mapScale() );
  context.setMapToPixel( QgsMapToPixel( 1 / ( settings.mmPerMapUnit() * context.scaleFactor() ) ) );
  context.setForceVectorOutput( true );

  if ( ctx && ctx->painter )
  {
    context.setPainter( ctx->painter );
    ctx->painter->save();
    ctx->painter->setRenderHint( QPainter::Antialiasing );
    ctx->painter->translate( ctx->point );
    ctx->painter->scale( 1 / context.scaleFactor(), 1 / context.scaleFactor() );
  }

  QgsDataDefinedSizeLegend ddsLegend( *mSettings );
  ddsLegend.setFont( settings.style( QgsLegendStyle::SymbolLabel ).font() );
  ddsLegend.setTextColor( settings.fontColor() );

  QSize contentSize;
  int labelXOffset;
  ddsLegend.drawCollapsedLegend( context, &contentSize, &labelXOffset );

  if ( ctx && ctx->painter )
    ctx->painter->restore();

  ItemMetrics im;
  im.symbolSize = QSizeF( ( contentSize.width() - labelXOffset ) / context.scaleFactor(), contentSize.height() / context.scaleFactor() );
  im.labelSize = QSizeF( labelXOffset / context.scaleFactor(), contentSize.height() / context.scaleFactor() );
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
      context->setScaleFactor( 96 / 25.4 );
    }
    mImage = mSettings->collapsedLegendImage( *context );
  }
}
