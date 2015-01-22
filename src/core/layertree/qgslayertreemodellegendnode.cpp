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

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslegendsettings.h"
#include "qgsrasterlayer.h"
#include "qgsrendererv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorlayer.h"



QgsLayerTreeModelLegendNode::QgsLayerTreeModelLegendNode( QgsLayerTreeLayer* nodeL, QObject* parent )
    : QObject( parent )
    , mLayerNode( nodeL )
    , mEmbeddedInParent( false )
{
}

QgsLayerTreeModelLegendNode::~QgsLayerTreeModelLegendNode()
{
}

QgsLayerTreeModel* QgsLayerTreeModelLegendNode::model() const
{
  return qobject_cast<QgsLayerTreeModel*>( parent() );
}

Qt::ItemFlags QgsLayerTreeModelLegendNode::flags() const
{
  return Qt::ItemIsEnabled;
}

bool QgsLayerTreeModelLegendNode::setData( const QVariant& value, int role )
{
  Q_UNUSED( value );
  Q_UNUSED( role );
  return false;
}


QgsLayerTreeModelLegendNode::ItemMetrics QgsLayerTreeModelLegendNode::draw( const QgsLegendSettings& settings, ItemContext* ctx )
{
  QFont symbolLabelFont = settings.style( QgsComposerLegendStyle::SymbolLabel ).font();

  double textHeight = settings.fontHeightCharacterMM( symbolLabelFont, QChar( '0' ) );
  // itemHeight here is not realy item height, it is only for symbol
  // vertical alignment purpose, i.e. ok take single line height
  // if there are more lines, thos run under the symbol
  double itemHeight = qMax(( double ) settings.symbolSize().height(), textHeight );

  ItemMetrics im;
  im.symbolSize = drawSymbol( settings, ctx, itemHeight );
  im.labelSize = drawSymbolText( settings, ctx, im.symbolSize );
  return im;
}


QSizeF QgsLayerTreeModelLegendNode::drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const
{
  QIcon symbolIcon = data( Qt::DecorationRole ).value<QIcon>();
  if ( symbolIcon.isNull() )
    return QSizeF();

  if ( ctx )
    symbolIcon.paint( ctx->painter, ctx->point.x(), ctx->point.y() + ( itemHeight - settings.symbolSize().height() ) / 2,
                      settings.symbolSize().width(), settings.symbolSize().height() );
  return settings.symbolSize();
}


QSizeF QgsLayerTreeModelLegendNode::drawSymbolText( const QgsLegendSettings& settings, ItemContext* ctx, const QSizeF& symbolSize ) const
{
  QSizeF labelSize( 0, 0 );

  QFont symbolLabelFont = settings.style( QgsComposerLegendStyle::SymbolLabel ).font();
  double textHeight = settings.fontHeightCharacterMM( symbolLabelFont, QChar( '0' ) );

  QStringList lines = settings.splitStringForWrapping( data( Qt::DisplayRole ).toString() );

  labelSize.rheight() = lines.count() * textHeight + ( lines.count() - 1 ) * settings.lineSpacing();

  double labelX, labelY;
  if ( ctx )
  {
    ctx->painter->setPen( settings.fontColor() );

    labelX = ctx->point.x() + qMax(( double ) symbolSize.width(), ctx->labelXOffset );
    labelY = ctx->point.y();

    // Vertical alignment of label with symbol
    if ( labelSize.height() < symbolSize.height() )
      labelY += symbolSize.height() / 2 + textHeight / 2;  // label centered with symbol
    else
      labelY += textHeight; // label starts at top and runs under symbol
  }

  for ( QStringList::Iterator itemPart = lines.begin(); itemPart != lines.end(); ++itemPart )
  {
    labelSize.rwidth() = qMax( settings.textWidthMillimeters( symbolLabelFont, *itemPart ), double( labelSize.width() ) );

    if ( ctx )
    {
      settings.drawText( ctx->painter, labelX, labelY, *itemPart, symbolLabelFont );
      if ( itemPart != lines.end() )
        labelY += settings.lineSpacing() + textHeight;
    }
  }

  return labelSize;
}

// -------------------------------------------------------------------------


QgsSymbolV2LegendNode::QgsSymbolV2LegendNode( QgsLayerTreeLayer* nodeLayer, const QgsLegendSymbolItemV2& item, QObject* parent )
    : QgsLayerTreeModelLegendNode( nodeLayer, parent )
    , mItem( item )
    , mSymbolUsesMapUnits( false )
{
  updateLabel();

  if ( mItem.symbol() )
    mSymbolUsesMapUnits = ( mItem.symbol()->outputUnit() != QgsSymbolV2::MM );
}

QgsSymbolV2LegendNode::~QgsSymbolV2LegendNode()
{
}

Qt::ItemFlags QgsSymbolV2LegendNode::flags() const
{
  if ( mItem.isCheckable() )
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  else
    return Qt::ItemIsEnabled;
}


QVariant QgsSymbolV2LegendNode::data( int role ) const
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
    QSize iconSize( 16, 16 ); // TODO: configurable
    const int indentSize = 20;
    if ( mPixmap.isNull() )
    {
      QPixmap pix;
      if ( mItem.symbol() )
      {
        double scale = 0.0;
        double mupp = 0.0;
        int dpi = 0;
        if ( model() )
          model()->legendMapViewData( &mupp, &dpi, &scale );
        bool validData = mupp != 0 && dpi != 0 && scale != 0;

        // setup temporary render context
        QgsRenderContext context;
        context.setScaleFactor( dpi / 25.4 );
        context.setRendererScale( scale );
        context.setMapToPixel( QgsMapToPixel( mupp ) ); // hope it's ok to leave out other params

        pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( mItem.symbol(), iconSize, validData ? &context : 0 );
      }
      else
      {
        pix = QPixmap( iconSize );
        pix.fill( Qt::transparent );
      }

      if ( mItem.level() == 0 || ( model() && model()->testFlag( QgsLayerTreeModel::ShowLegendAsTree ) ) )
        mPixmap = pix;
      else
      {
        // ident the symbol icon to make it look like a tree structure
        QPixmap pix2( pix.width() + mItem.level() * indentSize, pix.height() );
        pix2.fill( Qt::transparent );
        QPainter p( &pix2 );
        p.drawPixmap( mItem.level() * indentSize, 0, pix );
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

    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( mLayerNode->layer() );
    if ( !vlayer || !vlayer->rendererV2() )
      return QVariant();

    return vlayer->rendererV2()->legendSymbolItemChecked( mItem.ruleKey() ) ? Qt::Checked : Qt::Unchecked;
  }
  else if ( role == RuleKeyRole )
  {
    return mItem.ruleKey();
  }
  else if ( role == SymbolV2LegacyRuleKeyRole )
  {
    return QVariant::fromValue<void*>( mItem.legacyRuleKey() );
  }
  else if ( role == ParentRuleKeyRole )
  {
    return mItem.parentRuleKey();
  }

  return QVariant();
}

bool QgsSymbolV2LegendNode::setData( const QVariant& value, int role )
{
  if ( role != Qt::CheckStateRole )
    return false;

  if ( !mItem.isCheckable() )
    return false;

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( mLayerNode->layer() );
  if ( !vlayer || !vlayer->rendererV2() )
    return false;

  vlayer->rendererV2()->checkLegendSymbolItem( mItem.ruleKey(), value == Qt::Checked );

  emit dataChanged();

  vlayer->triggerRepaint();

  return true;
}



QSizeF QgsSymbolV2LegendNode::drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const
{
  QgsSymbolV2* s = mItem.symbol();
  if ( !s )
  {
    return QSizeF();
  }

  // setup temporary render context
  QgsRenderContext context;
  context.setScaleFactor( settings.dpi() / 25.4 );
  context.setRendererScale( settings.mapScale() );
  context.setMapToPixel( QgsMapToPixel( 1 / ( settings.mmPerMapUnit() * context.scaleFactor() ) ) ); // hope it's ok to leave out other params
  context.setForceVectorOutput( true );
  context.setPainter( ctx ? ctx->painter : 0 );

  //Consider symbol size for point markers
  double height = settings.symbolSize().height();
  double width = settings.symbolSize().width();
  double size = 0;
  //Center small marker symbols
  double widthOffset = 0;
  double heightOffset = 0;

  if ( QgsMarkerSymbolV2* markerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( s ) )
  {
    // allow marker symbol to occupy bigger area if necessary
    size = markerSymbol->size() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context, s->outputUnit(), s->mapUnitScale() ) / context.scaleFactor();
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

  if ( ctx )
  {
    double currentXPosition = ctx->point.x();
    double currentYCoord = ctx->point.y() + ( itemHeight - settings.symbolSize().height() ) / 2;
    QPainter* p = ctx->painter;

    //setup painter scaling to dots so that raster symbology is drawn to scale
    double dotsPerMM = context.scaleFactor();

    int opacity = 255;
    if ( QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( layerNode()->layer() ) )
      opacity = 255 - ( 255 * vectorLayer->layerTransparency() / 100 );

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
    p->restore();
  }

  return QSizeF( qMax( width + 2 * widthOffset, ( double ) settings.symbolSize().width() ),
                 qMax( height + 2 * heightOffset, ( double ) settings.symbolSize().height() ) );
}


void QgsSymbolV2LegendNode::setEmbeddedInParent( bool embedded )
{
  QgsLayerTreeModelLegendNode::setEmbeddedInParent( embedded );
  updateLabel();
}


void QgsSymbolV2LegendNode::invalidateMapBasedData()
{
  if ( mSymbolUsesMapUnits )
  {
    mPixmap = QPixmap();
    emit dataChanged();
  }
}


void QgsSymbolV2LegendNode::updateLabel()
{
  bool showFeatureCount = mLayerNode->customProperty( "showFeatureCount", 0 ).toBool();
  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( mLayerNode->layer() );

  if ( mEmbeddedInParent )
  {
    QString layerName = mLayerNode->layerName();
    if ( !mLayerNode->customProperty( "legend/title-label" ).isNull() )
      layerName = mLayerNode->customProperty( "legend/title-label" ).toString();

    mLabel = mUserLabel.isEmpty() ? layerName : mUserLabel;
    if ( showFeatureCount && vl && vl->pendingFeatureCount() >= 0 )
      mLabel += QString( " [%1]" ).arg( vl->pendingFeatureCount() );
  }
  else
  {
    mLabel = mUserLabel.isEmpty() ? mItem.label() : mUserLabel;
    if ( showFeatureCount && vl && mItem.legacyRuleKey() )
      mLabel += QString( " [%1]" ).arg( vl->featureCount( mItem.legacyRuleKey() ) );
  }
}



// -------------------------------------------------------------------------


QgsSimpleLegendNode::QgsSimpleLegendNode( QgsLayerTreeLayer* nodeLayer, const QString& label, const QIcon& icon, QObject* parent )
    : QgsLayerTreeModelLegendNode( nodeLayer, parent )
    , mLabel( label )
    , mIcon( icon )
{
}

QVariant QgsSimpleLegendNode::data( int role ) const
{
  if ( role == Qt::DisplayRole || role == Qt::EditRole )
    return mLabel;
  else if ( role == Qt::DecorationRole )
    return mIcon;
  else
    return QVariant();
}


// -------------------------------------------------------------------------

QgsImageLegendNode::QgsImageLegendNode( QgsLayerTreeLayer* nodeLayer, const QImage& img, QObject* parent )
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

QSizeF QgsImageLegendNode::drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const
{
  Q_UNUSED( itemHeight );

  if ( ctx )
  {
    ctx->painter->drawImage( QRectF( ctx->point.x(), ctx->point.y(), settings.wmsLegendSize().width(), settings.wmsLegendSize().height() ),
                             mImage, QRectF( 0, 0, mImage.width(), mImage.height() ) );
  }
  return settings.wmsLegendSize();
}

// -------------------------------------------------------------------------

QgsRasterSymbolLegendNode::QgsRasterSymbolLegendNode( QgsLayerTreeLayer* nodeLayer, const QColor& color, const QString& label, QObject* parent )
    : QgsLayerTreeModelLegendNode( nodeLayer, parent )
    , mColor( color )
    , mLabel( label )
{
}

QVariant QgsRasterSymbolLegendNode::data( int role ) const
{
  if ( role == Qt::DecorationRole )
  {
    QSize iconSize( 16, 16 ); // TODO: configurable?
    QPixmap pix( iconSize );
    pix.fill( mColor );
    return QIcon( pix );
  }
  else if ( role == Qt::DisplayRole || role == Qt::EditRole )
    return mUserLabel.isEmpty() ? mLabel : mUserLabel;
  else
    return QVariant();
}


QSizeF QgsRasterSymbolLegendNode::drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const
{
  if ( ctx )
  {
    QColor itemColor = mColor;
    if ( QgsRasterLayer* rasterLayer = dynamic_cast<QgsRasterLayer*>( layerNode()->layer() ) )
    {
      if ( QgsRasterRenderer* rasterRenderer = rasterLayer->renderer() )
        itemColor.setAlpha( rasterRenderer ? rasterRenderer->opacity() * 255.0 : 255 );
    }

    ctx->painter->setBrush( itemColor );
    ctx->painter->drawRect( QRectF( ctx->point.x(), ctx->point.y() + ( itemHeight - settings.symbolSize().height() ) / 2,
                                    settings.symbolSize().width(), settings.symbolSize().height() ) );
  }
  return settings.symbolSize();
}

// -------------------------------------------------------------------------

QgsWMSLegendNode::QgsWMSLegendNode( QgsLayerTreeLayer* nodeLayer, QObject* parent )
    : QgsLayerTreeModelLegendNode( nodeLayer, parent )
    , mValid( false )
{
}

const QImage& QgsWMSLegendNode::getLegendGraphic() const
{
  if ( ! mValid && ! mFetcher )
  {
    // or maybe in presence of a downloader we should just delete it
    // and start a new one ?

    QgsRasterLayer* layer = qobject_cast<QgsRasterLayer*>( mLayerNode->layer() );
    const QgsLayerTreeModel* mod = model();
    if ( ! mod ) return mImage;
    const QgsMapSettings* ms = mod->legendFilterByMap();

    QgsRasterDataProvider* prov = layer->dataProvider();

    Q_ASSERT( ! mFetcher );
    mFetcher.reset( prov->getLegendGraphicFetcher( ms ) );
    if ( mFetcher )
    {
      connect( mFetcher.data(), SIGNAL( finish( const QImage& ) ), this, SLOT( getLegendGraphicFinished( const QImage& ) ) );
      connect( mFetcher.data(), SIGNAL( error( const QString& ) ), this, SLOT( getLegendGraphicErrored( const QString& ) ) );
      connect( mFetcher.data(), SIGNAL( progress( qint64, qint64 ) ), this, SLOT( getLegendGraphicProgress( qint64, qint64 ) ) );
      mFetcher->start();
    } // else QgsDebugMsg("XXX No legend supported ?");

  }

  return mImage;
}

QVariant QgsWMSLegendNode::data( int role ) const
{
  //QgsDebugMsg( QString("XXX data called with role %1 -- mImage size is %2x%3").arg(role).arg(mImage.width()).arg(mImage.height()) );

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

QSizeF QgsWMSLegendNode::drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const
{
  Q_UNUSED( itemHeight );

  if ( ctx )
  {
    ctx->painter->drawImage( QRectF( ctx->point, settings.wmsLegendSize() ),
                             mImage,
                             QRectF( QPointF( 0, 0 ), mImage.size() ) );
  }
  return settings.wmsLegendSize();
}

/* private */
QImage QgsWMSLegendNode::renderMessage( const QString& msg ) const
{
  const int fontHeight = 10;
  const int margin = fontHeight / 2;
  const int nlines = 1;

  const int w = 512, h = fontHeight * nlines + margin * ( nlines + 1 );
  QImage theImage( w, h, QImage::Format_ARGB32_Premultiplied );
  QPainter painter;
  painter.begin( &theImage );
  painter.setPen( QColor( 255, 0, 0 ) );
  painter.setFont( QFont( "Chicago", fontHeight ) );
  painter.fillRect( 0, 0, w, h, QColor( 255, 255, 255 ) );
  painter.drawText( 0, margin + fontHeight, msg );
  //painter.drawText(0,2*(margin+fontHeight),QString("retrying in 5 seconds..."));
  painter.end();

  return theImage;
}

void QgsWMSLegendNode::getLegendGraphicProgress( qint64 cur, qint64 tot )
{
  QString msg = QString( "Downloading... %1/%2" ).arg( cur ).arg( tot );
  //QgsDebugMsg ( QString("XXX %1").arg(msg) );
  mImage = renderMessage( msg );
  emit dataChanged();
}

void QgsWMSLegendNode::getLegendGraphicErrored( const QString& msg )
{
  if ( ! mFetcher ) return; // must be coming after finish

  mImage = renderMessage( msg );
  //QgsDebugMsg( QString("XXX emitting dataChanged after writing an image of %1x%2").arg(mImage.width()).arg(mImage.height()) );

  emit dataChanged();

  mFetcher.reset();

  mValid = true; // we consider it valid anyway
  // ... but remove validity after 5 seconds
  //QTimer::singleShot(5000, this, SLOT(invalidateMapBasedData()));
}

void QgsWMSLegendNode::getLegendGraphicFinished( const QImage& theImage )
{
  if ( ! mFetcher ) return; // must be coming after error

  //QgsDebugMsg( QString("XXX legend graphic finished, image is %1x%2").arg(theImage.width()).arg(theImage.height()) );
  if ( ! theImage.isNull() )
  {
    if ( theImage != mImage )
    {
      mImage = theImage;
      //QgsDebugMsg( QString("XXX emitting dataChanged") );
      emit dataChanged();
    }
    mValid = true; // only if not null I guess
  }
  mFetcher.reset();
}

void QgsWMSLegendNode::invalidateMapBasedData()
{
  //QgsDebugMsg( QString("XXX invalidateMapBasedData called") );
  // TODO: do this only if this extent != prev extent ?
  mValid = false;
  emit dataChanged();
}
