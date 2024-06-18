/***************************************************************************
                         qgslayoutitemlegend.cpp
                         -----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <limits>

#include "qgslayoutitemlegend.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutmodel.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslegendrenderer.h"
#include "qgslegendstyle.h"
#include "qgslogger.h"
#include "qgsmapsettings.h"
#include "qgsproject.h"
#include "qgscolorutils.h"
#include "qgslayertreeutils.h"
#include "qgslayoututils.h"
#include "qgslayout.h"
#include "qgsstyleentityvisitor.h"
#include "qgslayertreemodellegendnode.h"
#include "qgsvectorlayer.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgslayertreefiltersettings.h"
#include "qgsreferencedgeometry.h"

#include <QDomDocument>
#include <QDomElement>
#include <QPainter>
#include "qgsexpressioncontext.h"

QgsLayoutItemLegend::QgsLayoutItemLegend( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mLegendModel( new QgsLegendModel( nullptr, this ) )
{
#if 0 //no longer required?
  connect( &layout->atlasComposition(), &QgsAtlasComposition::renderEnded, this, &QgsLayoutItemLegend::onAtlasEnded );
#endif

  mTitle = mSettings.title();

  connect( mLegendModel.get(), &QgsLayerTreeModel::hitTestStarted, this, [this] { emit backgroundTaskCountChanged( 1 ); } );
  connect( mLegendModel.get(), &QgsLayerTreeModel::hitTestCompleted, this, [this]
  {
    adjustBoxSize();
    emit backgroundTaskCountChanged( 0 );
  } );

  // Connect to the main layertreeroot.
  // It serves in "auto update mode" as a medium between the main app legend and this one
  connect( mLayout->project()->layerTreeRoot(), &QgsLayerTreeNode::customPropertyChanged, this, &QgsLayoutItemLegend::nodeCustomPropertyChanged );

  // If project colors change, we need to redraw legend, as legend symbols may rely on project colors
  connect( mLayout->project(), &QgsProject::projectColorsChanged, this, [this]
  {
    invalidateCache();
    update();
  } );
  connect( mLegendModel.get(), &QgsLegendModel::refreshLegend, this, [this]
  {
    // NOTE -- we do NOT connect to ::refresh here, as we don't want to trigger the call to onAtlasFeature() which sets mFilterAskedForUpdate to true,
    // causing an endless loop.
    invalidateCache();
    update();
  } );
}

QgsLayoutItemLegend *QgsLayoutItemLegend::create( QgsLayout *layout )
{
  return new QgsLayoutItemLegend( layout );
}

int QgsLayoutItemLegend::type() const
{
  return QgsLayoutItemRegistry::LayoutLegend;
}

QIcon QgsLayoutItemLegend::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemLegend.svg" ) );
}

QgsLayoutItem::Flags QgsLayoutItemLegend::itemFlags() const
{
  return QgsLayoutItem::FlagOverridesPaint;
}

void QgsLayoutItemLegend::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  if ( !painter )
    return;

  ensureModelIsInitialized();

  if ( mFilterAskedForUpdate )
  {
    mFilterAskedForUpdate = false;
    doUpdateFilterByMap();
  }

  if ( mLayout )
  {
    if ( !mLayout->renderContext().isPreviewRender() && mLegendModel->hitTestInProgress() )
    {
      mLegendModel->waitForHitTestBlocking();
    }
  }

  const int dpi = painter->device()->logicalDpiX();
  const double dotsPerMM = dpi / 25.4;

  if ( mLayout )
  {
    Q_NOWARN_DEPRECATED_PUSH
    // no longer required, but left set for api stability
    mSettings.setUseAdvancedEffects( mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagUseAdvancedEffects );
    mSettings.setDpi( dpi );
    mSettings.setSynchronousLegendRequests( mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagSynchronousLegendGraphics );
    Q_NOWARN_DEPRECATED_POP
  }
  if ( mMap && mLayout )
  {
    Q_NOWARN_DEPRECATED_PUSH
    // no longer required, but left set for api stability
    mSettings.setMmPerMapUnit( mLayout->convertFromLayoutUnits( mMap->mapUnitsToLayoutUnits(), Qgis::LayoutUnit::Millimeters ).length() );
    Q_NOWARN_DEPRECATED_POP

    // use a temporary QgsMapSettings to find out real map scale
    const QSizeF mapSizePixels = QSizeF( mMap->rect().width() * dotsPerMM, mMap->rect().height() * dotsPerMM );
    const QgsRectangle mapExtent = mMap->extent();

    const QgsMapSettings ms = mMap->mapSettings( mapExtent, mapSizePixels, dpi, false );

    // no longer required, but left set for api stability
    Q_NOWARN_DEPRECATED_PUSH
    mSettings.setMapScale( ms.scale() );
    Q_NOWARN_DEPRECATED_POP
  }
  mInitialMapScaleCalculated = true;

  QgsLegendRenderer legendRenderer( mLegendModel.get(), mSettings );
  legendRenderer.setLegendSize( mForceResize && mSizeToContents ? QSize() : rect().size() );

  const QPointF oldPos = pos();

  //adjust box if width or height is too small
  if ( mSizeToContents )
  {
    QgsRenderContext context = mMap ? QgsLayoutUtils::createRenderContextForMap( mMap, painter )
                               : QgsLayoutUtils::createRenderContextForLayout( mLayout, painter );

    const QSizeF size = legendRenderer.minimumSize( &context );
    if ( mForceResize )
    {
      mForceResize = false;

      //set new rect, respecting position mode and data defined size/position
      const QgsLayoutSize newSize = mLayout->convertFromLayoutUnits( size, sizeWithUnits().units() );
      attemptResize( newSize );
    }
    else if ( size.height() > rect().height() || size.width() > rect().width() )
    {
      //need to resize box
      QSizeF targetSize = rect().size();
      if ( size.height() > targetSize.height() )
        targetSize.setHeight( size.height() );
      if ( size.width() > targetSize.width() )
        targetSize.setWidth( size.width() );

      const QgsLayoutSize newSize = mLayout->convertFromLayoutUnits( targetSize, sizeWithUnits().units() );
      //set new rect, respecting position mode and data defined size/position
      attemptResize( newSize );
    }
  }

  // attemptResize may change the legend position and would call setPos
  // BUT the position is actually changed for the next draw, so we need to translate of the difference
  // between oldPos and newPos
  // the issue doesn't appear in desktop rendering but only in export because in the first one,
  // Qt triggers a redraw on position change
  painter->save();
  painter->translate( pos() - oldPos );
  QgsLayoutItem::paint( painter, itemStyle, pWidget );
  painter->restore();
}

void QgsLayoutItemLegend::finalizeRestoreFromXml()
{
  if ( !mMapUuid.isEmpty() )
  {
    setLinkedMap( qobject_cast< QgsLayoutItemMap * >( mLayout->itemByUuid( mMapUuid, true ) ) );
  }

  if ( !mFilterByMapUuids.isEmpty() )
  {
    QList< QgsLayoutItemMap * > maps;
    maps.reserve( mFilterByMapUuids.size() );
    for ( const QString &uuid : std::as_const( mFilterByMapUuids ) )
    {
      if ( QgsLayoutItemMap *map = qobject_cast< QgsLayoutItemMap * >( mLayout->itemByUuid( uuid, true ) ) )
      {
        maps << map;
      }
    }
    setFilterByMapItems( maps );
  }
}

void QgsLayoutItemLegend::refresh()
{
  QgsLayoutItem::refresh();
  clearLegendCachedData();
  onAtlasFeature();
}

void QgsLayoutItemLegend::invalidateCache()
{
  clearLegendCachedData();
  QgsLayoutItem::invalidateCache();
}

void QgsLayoutItemLegend::draw( QgsLayoutItemRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();

  QgsRenderContext rc = mMap ? QgsLayoutUtils::createRenderContextForMap( mMap, painter, context.renderContext().scaleFactor() * 25.4 )
                        : QgsLayoutUtils::createRenderContextForLayout( mLayout, painter, context.renderContext().scaleFactor() * 25.4 );

  rc.expressionContext().appendScopes( createExpressionContext().takeScopes() );

  const QgsScopedQPainterState painterState( painter );

  // painter is scaled to dots, so scale back to layout units
  painter->scale( rc.scaleFactor(), rc.scaleFactor() );

  painter->setPen( QPen( QColor( 0, 0, 0 ) ) );

  if ( !mSizeToContents )
  {
    // set a clip region to crop out parts of legend which don't fit
    const QRectF thisPaintRect = QRectF( 0, 0, rect().width(), rect().height() );
    painter->setClipRect( thisPaintRect );
  }

  if ( mLayout )
  {
    // no longer required, but left for API compatibility
    Q_NOWARN_DEPRECATED_PUSH
    mSettings.setDpi( mLayout->renderContext().dpi() );
    Q_NOWARN_DEPRECATED_POP
  }

  QgsLegendRenderer legendRenderer( mLegendModel.get(), mSettings );
  legendRenderer.setLegendSize( rect().size() );

  legendRenderer.drawLegend( rc );
}

void QgsLayoutItemLegend::adjustBoxSize()
{
  if ( !mSizeToContents )
    return;

  if ( !mInitialMapScaleCalculated )
  {
    // this is messy - but until we have painted the item we have no knowledge of the current DPI
    // and so cannot correctly calculate the map scale. This results in incorrect size calculations
    // for marker symbols with size in map units, causing the legends to initially expand to huge
    // sizes if we attempt to calculate the box size first.
    return;
  }

  QgsRenderContext context = mMap ? QgsLayoutUtils::createRenderContextForMap( mMap, nullptr ) :
                             QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );

  QgsLegendRenderer legendRenderer( mLegendModel.get(), mSettings );
  const QSizeF size = legendRenderer.minimumSize( &context );
  QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2" ).arg( size.width() ).arg( size.height() ), 2 );
  if ( size.isValid() )
  {
    const QgsLayoutSize newSize = mLayout->convertFromLayoutUnits( size, sizeWithUnits().units() );
    //set new rect, respecting position mode and data defined size/position
    attemptResize( newSize );
  }
}

void QgsLayoutItemLegend::setResizeToContents( bool enabled )
{
  mSizeToContents = enabled;
}

bool QgsLayoutItemLegend::resizeToContents() const
{
  return mSizeToContents;
}

void QgsLayoutItemLegend::setCustomLayerTree( QgsLayerTree *rootGroup )
{
  if ( !mDeferLegendModelInitialization )
  {
    mLegendModel->setRootGroup( rootGroup ? rootGroup : ( mLayout ? mLayout->project()->layerTreeRoot() : nullptr ) );
  }

  mCustomLayerTree.reset( rootGroup );
}

void QgsLayoutItemLegend::ensureModelIsInitialized()
{
  if ( mDeferLegendModelInitialization )
  {
    mDeferLegendModelInitialization = false;
    setCustomLayerTree( mCustomLayerTree.release() );
  }
}

QgsLegendModel *QgsLayoutItemLegend::model()
{
  ensureModelIsInitialized();
  return mLegendModel.get();
}

void QgsLayoutItemLegend::setAutoUpdateModel( bool autoUpdate )
{
  if ( autoUpdate == autoUpdateModel() )
    return;

  setCustomLayerTree( autoUpdate ? nullptr : mLayout->project()->layerTreeRoot()->clone() );
  adjustBoxSize();
  updateFilterByMap( false );
}

void QgsLayoutItemLegend::nodeCustomPropertyChanged( QgsLayerTreeNode *, const QString &key )
{
  if ( key == QLatin1String( "cached_name" ) )
    return;

  if ( autoUpdateModel() )
  {
    // in "auto update" mode, some parameters on the main app legend may have been changed (expression filtering)
    // we must then call updateItem to reflect the changes
    updateFilterByMap( false );
  }
}

bool QgsLayoutItemLegend::autoUpdateModel() const
{
  return !mCustomLayerTree;
}

void QgsLayoutItemLegend::setLegendFilterByMapEnabled( bool enabled )
{
  if ( mLegendFilterByMap == enabled )
    return;

  mLegendFilterByMap = enabled;
  updateFilterByMap( false );
}

void QgsLayoutItemLegend::setTitle( const QString &t )
{
  mTitle = t;
  mSettings.setTitle( t );

  if ( mLayout && id().isEmpty() )
  {
    //notify the model that the display name has changed
    mLayout->itemsModel()->updateItemDisplayName( this );
  }
}
QString QgsLayoutItemLegend::title() const
{
  return mTitle;
}

Qt::AlignmentFlag QgsLayoutItemLegend::titleAlignment() const
{
  return mSettings.titleAlignment();
}

void QgsLayoutItemLegend::setTitleAlignment( Qt::AlignmentFlag alignment )
{
  mSettings.setTitleAlignment( alignment );
}

QgsLegendStyle &QgsLayoutItemLegend::rstyle( QgsLegendStyle::Style s )
{
  return mSettings.rstyle( s );
}

QgsLegendStyle QgsLayoutItemLegend::style( QgsLegendStyle::Style s ) const
{
  return mSettings.style( s );
}

void QgsLayoutItemLegend::setStyle( QgsLegendStyle::Style s, const QgsLegendStyle &style )
{
  mSettings.setStyle( s, style );
}

QFont QgsLayoutItemLegend::styleFont( QgsLegendStyle::Style s ) const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.style( s ).font();
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutItemLegend::setStyleFont( QgsLegendStyle::Style s, const QFont &f )
{
  Q_NOWARN_DEPRECATED_PUSH
  rstyle( s ).setFont( f );
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutItemLegend::setStyleMargin( QgsLegendStyle::Style s, double margin )
{
  rstyle( s ).setMargin( margin );
}

void QgsLayoutItemLegend::setStyleMargin( QgsLegendStyle::Style s, QgsLegendStyle::Side side, double margin )
{
  rstyle( s ).setMargin( side, margin );
}

double QgsLayoutItemLegend::lineSpacing() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.lineSpacing();
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutItemLegend::setLineSpacing( double spacing )
{
  Q_NOWARN_DEPRECATED_PUSH
  mSettings.setLineSpacing( spacing );
  Q_NOWARN_DEPRECATED_POP
}

double QgsLayoutItemLegend::boxSpace() const
{
  return mSettings.boxSpace();
}

void QgsLayoutItemLegend::setBoxSpace( double s )
{
  mSettings.setBoxSpace( s );
}

double QgsLayoutItemLegend::columnSpace() const
{
  return mSettings.columnSpace();
}

void QgsLayoutItemLegend::setColumnSpace( double s )
{
  mSettings.setColumnSpace( s );
}

QColor QgsLayoutItemLegend::fontColor() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.fontColor();
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutItemLegend::setFontColor( const QColor &c )
{
  Q_NOWARN_DEPRECATED_PUSH
  mSettings.setFontColor( c );
  Q_NOWARN_DEPRECATED_POP
}

double QgsLayoutItemLegend::symbolWidth() const
{
  return mSettings.symbolSize().width();
}

void QgsLayoutItemLegend::setSymbolWidth( double w )
{
  mSettings.setSymbolSize( QSizeF( w, mSettings.symbolSize().height() ) );
}

double QgsLayoutItemLegend::maximumSymbolSize() const
{
  return mSettings.maximumSymbolSize();
}

void QgsLayoutItemLegend::setMaximumSymbolSize( double size )
{
  mSettings.setMaximumSymbolSize( size );
}

double QgsLayoutItemLegend::minimumSymbolSize() const
{
  return mSettings.minimumSymbolSize();
}

void QgsLayoutItemLegend::setMinimumSymbolSize( double size )
{
  mSettings.setMinimumSymbolSize( size );
}

void QgsLayoutItemLegend::setSymbolAlignment( Qt::AlignmentFlag alignment )
{
  mSettings.setSymbolAlignment( alignment );
}

Qt::AlignmentFlag QgsLayoutItemLegend::symbolAlignment() const
{
  return mSettings.symbolAlignment();
}

double QgsLayoutItemLegend::symbolHeight() const
{
  return mSettings.symbolSize().height();
}

void QgsLayoutItemLegend::setSymbolHeight( double h )
{
  mSettings.setSymbolSize( QSizeF( mSettings.symbolSize().width(), h ) );
}

double QgsLayoutItemLegend::wmsLegendWidth() const
{
  return mSettings.wmsLegendSize().width();
}

void QgsLayoutItemLegend::setWmsLegendWidth( double w )
{
  mSettings.setWmsLegendSize( QSizeF( w, mSettings.wmsLegendSize().height() ) );
}

double QgsLayoutItemLegend::wmsLegendHeight() const
{
  return mSettings.wmsLegendSize().height();
}
void QgsLayoutItemLegend::setWmsLegendHeight( double h )
{
  mSettings.setWmsLegendSize( QSizeF( mSettings.wmsLegendSize().width(), h ) );
}

void QgsLayoutItemLegend::setWrapString( const QString &t )
{
  mSettings.setWrapChar( t );
}

QString QgsLayoutItemLegend::wrapString() const
{
  return mSettings.wrapChar();
}

int QgsLayoutItemLegend::columnCount() const
{
  return mColumnCount;
}

void QgsLayoutItemLegend::setColumnCount( int c )
{
  mColumnCount = c;
  mSettings.setColumnCount( c );
}

bool QgsLayoutItemLegend::splitLayer() const
{
  return mSettings.splitLayer();
}

void QgsLayoutItemLegend::setSplitLayer( bool s )
{
  mSettings.setSplitLayer( s );
}

bool QgsLayoutItemLegend::equalColumnWidth() const
{
  return mSettings.equalColumnWidth();
}

void QgsLayoutItemLegend::setEqualColumnWidth( bool s )
{
  mSettings.setEqualColumnWidth( s );
}

bool QgsLayoutItemLegend::drawRasterStroke() const
{
  return mSettings.drawRasterStroke();
}

void QgsLayoutItemLegend::setDrawRasterStroke( bool enabled )
{
  mSettings.setDrawRasterStroke( enabled );
}

QColor QgsLayoutItemLegend::rasterStrokeColor() const
{
  return mSettings.rasterStrokeColor();
}

void QgsLayoutItemLegend::setRasterStrokeColor( const QColor &color )
{
  mSettings.setRasterStrokeColor( color );
}

double QgsLayoutItemLegend::rasterStrokeWidth() const
{
  return mSettings.rasterStrokeWidth();
}

void QgsLayoutItemLegend::setRasterStrokeWidth( double width )
{
  mSettings.setRasterStrokeWidth( width );
}

void QgsLayoutItemLegend::updateLegend()
{
  adjustBoxSize();
  updateFilterByMap( false );
}

bool QgsLayoutItemLegend::writePropertiesToElement( QDomElement &legendElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{

  //write general properties
  legendElem.setAttribute( QStringLiteral( "title" ), mTitle );
  legendElem.setAttribute( QStringLiteral( "titleAlignment" ), QString::number( static_cast< int >( mSettings.titleAlignment() ) ) );
  legendElem.setAttribute( QStringLiteral( "columnCount" ), QString::number( mColumnCount ) );
  legendElem.setAttribute( QStringLiteral( "splitLayer" ), QString::number( mSettings.splitLayer() ) );
  legendElem.setAttribute( QStringLiteral( "equalColumnWidth" ), QString::number( mSettings.equalColumnWidth() ) );

  legendElem.setAttribute( QStringLiteral( "boxSpace" ), QString::number( mSettings.boxSpace() ) );
  legendElem.setAttribute( QStringLiteral( "columnSpace" ), QString::number( mSettings.columnSpace() ) );

  legendElem.setAttribute( QStringLiteral( "symbolWidth" ), QString::number( mSettings.symbolSize().width() ) );
  legendElem.setAttribute( QStringLiteral( "symbolHeight" ), QString::number( mSettings.symbolSize().height() ) );
  legendElem.setAttribute( QStringLiteral( "maxSymbolSize" ), QString::number( mSettings.maximumSymbolSize() ) );
  legendElem.setAttribute( QStringLiteral( "minSymbolSize" ), QString::number( mSettings.minimumSymbolSize() ) );

  legendElem.setAttribute( QStringLiteral( "symbolAlignment" ), mSettings.symbolAlignment() );

  legendElem.setAttribute( QStringLiteral( "symbolAlignment" ), mSettings.symbolAlignment() );

  legendElem.setAttribute( QStringLiteral( "rasterBorder" ), mSettings.drawRasterStroke() );
  legendElem.setAttribute( QStringLiteral( "rasterBorderColor" ), QgsColorUtils::colorToString( mSettings.rasterStrokeColor() ) );
  legendElem.setAttribute( QStringLiteral( "rasterBorderWidth" ), QString::number( mSettings.rasterStrokeWidth() ) );

  legendElem.setAttribute( QStringLiteral( "wmsLegendWidth" ), QString::number( mSettings.wmsLegendSize().width() ) );
  legendElem.setAttribute( QStringLiteral( "wmsLegendHeight" ), QString::number( mSettings.wmsLegendSize().height() ) );
  legendElem.setAttribute( QStringLiteral( "wrapChar" ), mSettings.wrapChar() );

  legendElem.setAttribute( QStringLiteral( "resizeToContents" ), mSizeToContents );

  if ( mMap )
  {
    legendElem.setAttribute( QStringLiteral( "map_uuid" ), mMap->uuid() );
  }

  if ( !mFilterByMapItems.empty() )
  {
    QDomElement filterByMapsElem = doc.createElement( QStringLiteral( "filterByMaps" ) );
    for ( QgsLayoutItemMap *map : mFilterByMapItems )
    {
      if ( map )
      {
        QDomElement mapElem = doc.createElement( QStringLiteral( "map" ) );
        mapElem.setAttribute( QStringLiteral( "uuid" ), map->uuid() );
        filterByMapsElem.appendChild( mapElem );
      }
    }
    legendElem.appendChild( filterByMapsElem );
  }

  QDomElement legendStyles = doc.createElement( QStringLiteral( "styles" ) );
  legendElem.appendChild( legendStyles );

  style( QgsLegendStyle::Title ).writeXml( QStringLiteral( "title" ), legendStyles, doc, context );
  style( QgsLegendStyle::Group ).writeXml( QStringLiteral( "group" ), legendStyles, doc, context );
  style( QgsLegendStyle::Subgroup ).writeXml( QStringLiteral( "subgroup" ), legendStyles, doc, context );
  style( QgsLegendStyle::Symbol ).writeXml( QStringLiteral( "symbol" ), legendStyles, doc, context );
  style( QgsLegendStyle::SymbolLabel ).writeXml( QStringLiteral( "symbolLabel" ), legendStyles, doc, context );

  if ( mCustomLayerTree )
  {
    // if not using auto-update - store the custom layer tree
    mCustomLayerTree->writeXml( legendElem, context );
  }

  if ( mLegendFilterByMap )
  {
    legendElem.setAttribute( QStringLiteral( "legendFilterByMap" ), QStringLiteral( "1" ) );
  }
  legendElem.setAttribute( QStringLiteral( "legendFilterByAtlas" ), mFilterOutAtlas ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  return true;
}

bool QgsLayoutItemLegend::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  //read general properties
  mTitle = itemElem.attribute( QStringLiteral( "title" ) );
  mSettings.setTitle( mTitle );
  if ( !itemElem.attribute( QStringLiteral( "titleAlignment" ) ).isEmpty() )
  {
    mSettings.setTitleAlignment( static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "titleAlignment" ) ).toInt() ) );
  }
  int colCount = itemElem.attribute( QStringLiteral( "columnCount" ), QStringLiteral( "1" ) ).toInt();
  if ( colCount < 1 ) colCount = 1;
  mColumnCount = colCount;
  mSettings.setColumnCount( mColumnCount );
  mSettings.setSplitLayer( itemElem.attribute( QStringLiteral( "splitLayer" ), QStringLiteral( "0" ) ).toInt() == 1 );
  mSettings.setEqualColumnWidth( itemElem.attribute( QStringLiteral( "equalColumnWidth" ), QStringLiteral( "0" ) ).toInt() == 1 );

  const QDomNodeList stylesNodeList = itemElem.elementsByTagName( QStringLiteral( "styles" ) );
  if ( !stylesNodeList.isEmpty() )
  {
    const QDomNode stylesNode = stylesNodeList.at( 0 );
    for ( int i = 0; i < stylesNode.childNodes().size(); i++ )
    {
      const QDomElement styleElem = stylesNode.childNodes().at( i ).toElement();
      QgsLegendStyle style;
      style.readXml( styleElem, doc, context );
      const QString name = styleElem.attribute( QStringLiteral( "name" ) );
      QgsLegendStyle::Style s;
      if ( name == QLatin1String( "title" ) ) s = QgsLegendStyle::Title;
      else if ( name == QLatin1String( "group" ) ) s = QgsLegendStyle::Group;
      else if ( name == QLatin1String( "subgroup" ) ) s = QgsLegendStyle::Subgroup;
      else if ( name == QLatin1String( "symbol" ) ) s = QgsLegendStyle::Symbol;
      else if ( name == QLatin1String( "symbolLabel" ) ) s = QgsLegendStyle::SymbolLabel;
      else continue;
      setStyle( s, style );
    }
  }

  //font color
  if ( itemElem.hasAttribute( QStringLiteral( "fontColor" ) ) )
  {
    QColor fontClr;
    fontClr.setNamedColor( itemElem.attribute( QStringLiteral( "fontColor" ), QStringLiteral( "#000000" ) ) );
    rstyle( QgsLegendStyle::Title ).textFormat().setColor( fontClr );
    rstyle( QgsLegendStyle::Group ).textFormat().setColor( fontClr );
    rstyle( QgsLegendStyle::Subgroup ).textFormat().setColor( fontClr );
    rstyle( QgsLegendStyle::SymbolLabel ).textFormat().setColor( fontClr );
  }

  //spaces
  mSettings.setBoxSpace( itemElem.attribute( QStringLiteral( "boxSpace" ), QStringLiteral( "2.0" ) ).toDouble() );
  mSettings.setColumnSpace( itemElem.attribute( QStringLiteral( "columnSpace" ), QStringLiteral( "2.0" ) ).toDouble() );

  mSettings.setSymbolSize( QSizeF( itemElem.attribute( QStringLiteral( "symbolWidth" ), QStringLiteral( "7.0" ) ).toDouble(), itemElem.attribute( QStringLiteral( "symbolHeight" ), QStringLiteral( "14.0" ) ).toDouble() ) );
  mSettings.setSymbolAlignment( static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "symbolAlignment" ), QString::number( Qt::AlignLeft ) ).toInt() ) );

  mSettings.setMaximumSymbolSize( itemElem.attribute( QStringLiteral( "maxSymbolSize" ), QStringLiteral( "0.0" ) ).toDouble() );
  mSettings.setMinimumSymbolSize( itemElem.attribute( QStringLiteral( "minSymbolSize" ), QStringLiteral( "0.0" ) ).toDouble() );

  mSettings.setWmsLegendSize( QSizeF( itemElem.attribute( QStringLiteral( "wmsLegendWidth" ), QStringLiteral( "50" ) ).toDouble(), itemElem.attribute( QStringLiteral( "wmsLegendHeight" ), QStringLiteral( "25" ) ).toDouble() ) );

  if ( itemElem.hasAttribute( QStringLiteral( "lineSpacing" ) ) )
  {
    const double spacing = itemElem.attribute( QStringLiteral( "lineSpacing" ), QStringLiteral( "1.0" ) ).toDouble();
    // line spacing *was* a fixed amount (in mm) added between each line of text.
    QgsTextFormat f = rstyle( QgsLegendStyle::Title ).textFormat();
    // assume font sizes in points, since that was what we always had from before this method was deprecated
    f.setLineHeight( f.size() * 0.352778 + spacing );
    f.setLineHeightUnit( Qgis::RenderUnit::Millimeters );
    rstyle( QgsLegendStyle::Title ).setTextFormat( f );

    f = rstyle( QgsLegendStyle::Group ).textFormat();
    f.setLineHeight( f.size() * 0.352778 + spacing );
    f.setLineHeightUnit( Qgis::RenderUnit::Millimeters );
    rstyle( QgsLegendStyle::Group ).setTextFormat( f );

    f = rstyle( QgsLegendStyle::Subgroup ).textFormat();
    f.setLineHeight( f.size() * 0.352778 + spacing );
    f.setLineHeightUnit( Qgis::RenderUnit::Millimeters );
    rstyle( QgsLegendStyle::Subgroup ).setTextFormat( f );

    f = rstyle( QgsLegendStyle::SymbolLabel ).textFormat();
    f.setLineHeight( f.size() * 0.352778 + spacing );
    f.setLineHeightUnit( Qgis::RenderUnit::Millimeters );
    rstyle( QgsLegendStyle::SymbolLabel ).setTextFormat( f );
  }

  mSettings.setDrawRasterStroke( itemElem.attribute( QStringLiteral( "rasterBorder" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  mSettings.setRasterStrokeColor( QgsColorUtils::colorFromString( itemElem.attribute( QStringLiteral( "rasterBorderColor" ), QStringLiteral( "0,0,0" ) ) ) );
  mSettings.setRasterStrokeWidth( itemElem.attribute( QStringLiteral( "rasterBorderWidth" ), QStringLiteral( "0" ) ).toDouble() );

  mSettings.setWrapChar( itemElem.attribute( QStringLiteral( "wrapChar" ) ) );

  mSizeToContents = itemElem.attribute( QStringLiteral( "resizeToContents" ), QStringLiteral( "1" ) ) != QLatin1String( "0" );

  // map
  mLegendFilterByMap = itemElem.attribute( QStringLiteral( "legendFilterByMap" ), QStringLiteral( "0" ) ).toInt();

  mMapUuid.clear();
  if ( !itemElem.attribute( QStringLiteral( "map_uuid" ) ).isEmpty() )
  {
    mMapUuid = itemElem.attribute( QStringLiteral( "map_uuid" ) );
  }

  mFilterByMapUuids.clear();
  {
    const QDomElement filterByMapsElem = itemElem.firstChildElement( QStringLiteral( "filterByMaps" ) );
    if ( !filterByMapsElem.isNull() )
    {
      QDomElement mapsElem = filterByMapsElem.firstChildElement( QStringLiteral( "map" ) );
      while ( !mapsElem.isNull() )
      {
        mFilterByMapUuids << mapsElem.attribute( QStringLiteral( "uuid" ) );
        mapsElem = mapsElem.nextSiblingElement( QStringLiteral( "map" ) );
      }
    }
    else if ( !mMapUuid.isEmpty() )
    {
      // for compatibility with < QGIS 3.32 projects
      mFilterByMapUuids << mMapUuid;
    }
  }

  // disconnect current map
  setupMapConnections( mMap, false );
  mMap = nullptr;

  mFilterOutAtlas = itemElem.attribute( QStringLiteral( "legendFilterByAtlas" ), QStringLiteral( "0" ) ).toInt();

  // QGIS >= 2.6
  QDomElement layerTreeElem = itemElem.firstChildElement( QStringLiteral( "layer-tree" ) );
  if ( layerTreeElem.isNull() )
    layerTreeElem = itemElem.firstChildElement( QStringLiteral( "layer-tree-group" ) );

  if ( !layerTreeElem.isNull() )
  {
    std::unique_ptr< QgsLayerTree > tree( QgsLayerTree::readXml( layerTreeElem, context ) );
    if ( mLayout )
      tree->resolveReferences( mLayout->project(), true );
    setCustomLayerTree( tree.release() );
  }
  else
    setCustomLayerTree( nullptr );

  return true;
}

QString QgsLayoutItemLegend::displayName() const
{
  if ( !id().isEmpty() )
  {
    return id();
  }

  //if no id, default to portion of title text
  QString text = mSettings.title();
  if ( text.isEmpty() )
  {
    return tr( "<Legend>" );
  }
  if ( text.length() > 25 )
  {
    return tr( "%1…" ).arg( text.left( 25 ) );
  }
  else
  {
    return text;
  }
}

bool QgsLayoutItemLegend::requiresRasterization() const
{
  return blendMode() != QPainter::CompositionMode_SourceOver;
}

bool QgsLayoutItemLegend::containsAdvancedEffects() const
{
  return mEvaluatedOpacity < 1.0;
}

void QgsLayoutItemLegend::setupMapConnections( QgsLayoutItemMap *map, bool connectSlots )
{
  if ( !map )
    return;

  if ( !connectSlots )
  {
    disconnect( map, &QObject::destroyed, this, &QgsLayoutItemLegend::invalidateCurrentMap );
    disconnect( map, &QgsLayoutObject::changed, this, &QgsLayoutItemLegend::updateFilterByMapAndRedraw );
    disconnect( map, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemLegend::updateFilterByMapAndRedraw );
    disconnect( map, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutItemLegend::updateFilterByMapAndRedraw );
    disconnect( map, &QgsLayoutItemMap::layerStyleOverridesChanged, this, &QgsLayoutItemLegend::mapLayerStyleOverridesChanged );
    disconnect( map, &QgsLayoutItemMap::themeChanged, this, &QgsLayoutItemLegend::mapThemeChanged );
  }
  else
  {
    connect( map, &QObject::destroyed, this, &QgsLayoutItemLegend::invalidateCurrentMap );
    connect( map, &QgsLayoutObject::changed, this, &QgsLayoutItemLegend::updateFilterByMapAndRedraw );
    connect( map, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemLegend::updateFilterByMapAndRedraw );
    connect( map, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutItemLegend::updateFilterByMapAndRedraw );
    connect( map, &QgsLayoutItemMap::layerStyleOverridesChanged, this, &QgsLayoutItemLegend::mapLayerStyleOverridesChanged );
    connect( map, &QgsLayoutItemMap::themeChanged, this, &QgsLayoutItemLegend::mapThemeChanged );
  }
}

void QgsLayoutItemLegend::setLinkedMap( QgsLayoutItemMap *map )
{
  if ( mMap == map )
    return;

  if ( mMap )
  {
    setupMapConnections( mMap, false );
  }

  mMap = map;

  if ( mMap )
  {
    setupMapConnections( mMap, true );
    mapThemeChanged( mMap->themeToRender( mMap->createExpressionContext() ) );
  }

  updateFilterByMap();
}

void QgsLayoutItemLegend::setFilterByMapItems( const QList<QgsLayoutItemMap *> &maps )
{
  if ( filterByMapItems() == maps )
    return;

  for ( QgsLayoutItemMap *map : std::as_const( mFilterByMapItems ) )
  {
    setupMapConnections( map, false );
  }

  mFilterByMapItems.clear();
  mFilterByMapItems.reserve( maps.size() );
  for ( QgsLayoutItemMap *map : maps )
  {
    if ( map )
    {
      mFilterByMapItems.append( map );
      setupMapConnections( map, true );
    }
  }

  updateFilterByMap();
}

QList<QgsLayoutItemMap *> QgsLayoutItemLegend::filterByMapItems() const
{
  QList<QgsLayoutItemMap *> res;
  res.reserve( mFilterByMapItems.size() );
  for ( QgsLayoutItemMap *map : mFilterByMapItems )
  {
    if ( map )
      res.append( map );
  }
  return res;
}

void QgsLayoutItemLegend::invalidateCurrentMap()
{
  setLinkedMap( nullptr );
}

void QgsLayoutItemLegend::refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property )
{
  const QgsExpressionContext context = createExpressionContext();

  bool forceUpdate = false;
  //updates data defined properties and redraws item to match
  if ( property == QgsLayoutObject::DataDefinedProperty::LegendTitle || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    bool ok = false;
    const QString t = mDataDefinedProperties.valueAsString( QgsLayoutObject::DataDefinedProperty::LegendTitle, context, mTitle, &ok );
    if ( ok )
    {
      mSettings.setTitle( t );
      forceUpdate = true;
    }
  }
  if ( property == QgsLayoutObject::DataDefinedProperty::LegendColumnCount || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    bool ok = false;
    const int cols = mDataDefinedProperties.valueAsInt( QgsLayoutObject::DataDefinedProperty::LegendColumnCount, context, mColumnCount, &ok );
    if ( ok && cols >= 0 )
    {
      mSettings.setColumnCount( cols );
      forceUpdate = true;
    }
  }
  if ( forceUpdate )
  {
    adjustBoxSize();
    update();
  }

  QgsLayoutItem::refreshDataDefinedProperty( property );
}


void QgsLayoutItemLegend::updateFilterByMapAndRedraw()
{
  updateFilterByMap( true );
}

void QgsLayoutItemLegend::setModelStyleOverrides( const QMap<QString, QString> &overrides )
{
  mLegendModel->setLayerStyleOverrides( overrides );
  if ( QgsLayerTree *rootGroup = mLegendModel->rootGroup() )
  {
    const QList< QgsLayerTreeLayer * > layers =  rootGroup->findLayers();
    for ( QgsLayerTreeLayer *nodeLayer : std::as_const( layers ) )
      mLegendModel->refreshLayerLegend( nodeLayer );
  }
}

void QgsLayoutItemLegend::clearLegendCachedData()
{
  std::function< void( QgsLayerTreeNode * ) > clearNodeCache;
  clearNodeCache = [&]( QgsLayerTreeNode * node )
  {
    mLegendModel->clearCachedData( node );
    if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup *group = QgsLayerTree::toGroup( node );
      const QList< QgsLayerTreeNode * > children = group->children();
      for ( QgsLayerTreeNode *child : children )
      {
        clearNodeCache( child );
      }
    }
  };

  if ( QgsLayerTree *rootGroup = mLegendModel->rootGroup() )
  {
    clearNodeCache( rootGroup );
  }
}

void QgsLayoutItemLegend::mapLayerStyleOverridesChanged()
{
  if ( !mMap )
    return;

  // map's style has been changed, so make sure to update the legend here
  if ( mLegendFilterByMap )
  {
    // legend is being filtered by map, so we need to re run the hit test too
    // as the style overrides may also have affected the visible symbols
    updateFilterByMap( false );
  }
  else
  {
    setModelStyleOverrides( mMap->layerStyleOverrides() );
  }

  adjustBoxSize();

  updateFilterByMap( false );
}

void QgsLayoutItemLegend::mapThemeChanged( const QString &theme )
{
  if ( mThemeName == theme )
    return;

  mThemeName = theme;

  // map's theme has been changed, so make sure to update the legend here

  // legend is being filtered by map, so we need to re run the hit test too
  // as the style overrides may also have affected the visible symbols
  updateFilterByMap( false );

  adjustBoxSize();

  updateFilterByMap();
}

void QgsLayoutItemLegend::updateFilterByMap( bool redraw )
{
  // ask for update
  // the actual update will take place before the redraw.
  // This is to avoid multiple calls to the filter
  mFilterAskedForUpdate = true;

  if ( redraw )
    update();
}

void QgsLayoutItemLegend::doUpdateFilterByMap()
{
  // There's an incompatibility here between legend handling of linked map themes and layer style overrides vs
  // how expression evaluation is made in legend content text. The logic below is hacked together to get
  // all the existing unit tests passing, but these two features are incompatible with each other and fixing
  // this is extremely non-trivial. Let's just hope no-one tries to use those features together!
  // Ideally, all the branches below would be consistently using either "setModelStyleOverrides" (which forces
  // a rebuild of each layer's legend, and breaks legend text expression evaluation) OR
  // "mLegendModel->setLayerStyleOverrides" which just handles the expression updates but which doesn't correctly
  // generate the legend content from the associated theme settings.
  if ( mMap && !mThemeName.isEmpty() )
  {
    // get style overrides for theme
    const QMap<QString, QString> overrides = mLayout->project()->mapThemeCollection()->mapThemeStyleOverrides( mThemeName );
    setModelStyleOverrides( overrides );
  }
  else if ( mMap )
  {
    mLegendModel->setLayerStyleOverrides( mMap->layerStyleOverrides() );
  }
  else
  {
    mLegendModel->setLayerStyleOverrides( QMap<QString, QString>() );
  }

  const bool filterByExpression = QgsLayerTreeUtils::hasLegendFilterExpression( *( mCustomLayerTree ? mCustomLayerTree.get() : mLayout->project()->layerTreeRoot() ) );

  const bool hasValidFilter = filterByExpression
                              || ( mLegendFilterByMap && ( mMap || !mFilterByMapItems.empty() ) )
                              || mInAtlas;

  if ( hasValidFilter )
  {
    const double dpi = mLayout->renderContext().dpi();

    QSet< QgsLayoutItemMap * > linkedFilterMaps;
    if ( mLegendFilterByMap )
    {
      linkedFilterMaps = qgis::listToSet( filterByMapItems() );
      if ( mMap )
        linkedFilterMaps.insert( mMap );
    }

    QgsMapSettings mapSettings;
    QgsGeometry filterGeometry;
    if ( mMap )
    {
      // if a specific linked map has been set, use it for the reference scale and extent
      const QgsRectangle requestRectangle = mMap->requestedExtent();
      QSizeF size( requestRectangle.width(), requestRectangle.height() );
      size *= mLayout->convertFromLayoutUnits( mMap->mapUnitsToLayoutUnits(), Qgis::LayoutUnit::Millimeters ).length() * dpi / 25.4;
      mapSettings = mMap->mapSettings( requestRectangle, size, dpi, true );

      filterGeometry = QgsGeometry::fromQPolygonF( mMap->visibleExtentPolygon() );
    }
    else if ( !linkedFilterMaps.empty() )
    {
      // otherwise just take the first linked filter map
      const QgsRectangle requestRectangle = ( *linkedFilterMaps.constBegin() )->requestedExtent();
      QSizeF size( requestRectangle.width(), requestRectangle.height() );
      size *= mLayout->convertFromLayoutUnits( ( *linkedFilterMaps.constBegin() )->mapUnitsToLayoutUnits(), Qgis::LayoutUnit::Millimeters ).length() * dpi / 25.4;
      mapSettings = ( *linkedFilterMaps.constBegin() )->mapSettings( requestRectangle, size, dpi, true );

      filterGeometry = QgsGeometry::fromQPolygonF( ( *linkedFilterMaps.constBegin() )->visibleExtentPolygon() );
    }

    mapSettings.setExpressionContext( createExpressionContext() );

    const QgsGeometry atlasGeometry { mLayout->reportContext().currentGeometry( mapSettings.destinationCrs() ) };

    QgsLayerTreeFilterSettings filterSettings( mapSettings );

    QList<QgsMapLayer *> layersToClip;
    if ( !atlasGeometry.isNull() && mMap->atlasClippingSettings()->enabled() )
    {
      layersToClip = mMap->atlasClippingSettings()->layersToClip();
      for ( QgsMapLayer *layer : std::as_const( layersToClip ) )
      {
        QList<QgsMapLayer *> mapLayers { filterSettings.mapSettings().layers( true ) };
        mapLayers.removeAll( layer );
        filterSettings.mapSettings().setLayers( mapLayers );
        filterSettings.addVisibleExtentForLayer( layer, QgsReferencedGeometry( atlasGeometry, mapSettings.destinationCrs() ) );
      }
    }


    if ( !linkedFilterMaps.empty() )
    {
      for ( QgsLayoutItemMap *map : std::as_const( linkedFilterMaps ) )
      {

        if ( map == mMap )
          continue;

        QgsGeometry mapExtent = QgsGeometry::fromQPolygonF( map->visibleExtentPolygon() );

        //transform back to destination CRS
        const QgsCoordinateTransform mapTransform( map->crs(), mapSettings.destinationCrs(), mLayout->project() );
        try
        {
          mapExtent.transform( mapTransform );
        }
        catch ( QgsCsException &cse )
        {
          continue;
        }

        const QList< QgsMapLayer * > layersForMap = map->layersToRender();
        for ( QgsMapLayer *layer : layersForMap )
        {
          if ( mInAtlas && !atlasGeometry.isNull() )
          {
            mapExtent = mapExtent.intersection( atlasGeometry );
          }

          filterSettings.addVisibleExtentForLayer( layer, QgsReferencedGeometry( mapExtent, mapSettings.destinationCrs() ) );
        }
      }
    }

    if ( mInAtlas )
    {
      if ( !filterGeometry.isEmpty() )
        filterGeometry = mLayout->reportContext().currentGeometry( mapSettings.destinationCrs() );
      else
        filterGeometry = filterGeometry.intersection( mLayout->reportContext().currentGeometry( mapSettings.destinationCrs() ) );
    }

    filterSettings.setLayerFilterExpressionsFromLayerTree( mLegendModel->rootGroup() );
    if ( !filterGeometry.isNull() )
    {
      filterSettings.setFilterPolygon( filterGeometry );
    }
    else
    {
      filterSettings.setFlags( Qgis::LayerTreeFilterFlag::SkipVisibilityCheck );
    }

    mLegendModel->setFilterSettings( &filterSettings );
  }
  else
  {
    mLegendModel->setFilterSettings( nullptr );
  }

  clearLegendCachedData();
  mForceResize = true;
}

QString QgsLayoutItemLegend::themeName() const
{
  return mThemeName;
}

void QgsLayoutItemLegend::setLegendFilterOutAtlas( bool doFilter )
{
  mFilterOutAtlas = doFilter;
}

bool QgsLayoutItemLegend::legendFilterOutAtlas() const
{
  return mFilterOutAtlas;
}

void QgsLayoutItemLegend::onAtlasFeature()
{
  if ( !mLayout || !mLayout->reportContext().feature().isValid() )
    return;
  mInAtlas = mFilterOutAtlas;
  updateFilterByMap();
}

void QgsLayoutItemLegend::onAtlasEnded()
{
  mInAtlas = false;
  updateFilterByMap();
}

QgsExpressionContext QgsLayoutItemLegend::createExpressionContext() const
{
  QgsExpressionContext context = QgsLayoutItem::createExpressionContext();

  // We only want the last scope from the map's expression context, as this contains
  // the map specific variables. We don't want the rest of the map's context, because that
  // will contain duplicate global, project, layout, etc scopes.
  if ( mMap )
    context.appendScope( mMap->createExpressionContext().popScope() );

  QgsExpressionContextScope *scope = new QgsExpressionContextScope( tr( "Legend Settings" ) );

  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "legend_title" ), title(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "legend_column_count" ), columnCount(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "legend_split_layers" ), splitLayer(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "legend_wrap_string" ), wrapString(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "legend_filter_by_map" ), legendFilterByMapEnabled(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "legend_filter_out_atlas" ), legendFilterOutAtlas(), true ) );

  context.appendScope( scope );
  return context;
}

QgsLayoutItem::ExportLayerBehavior QgsLayoutItemLegend::exportLayerBehavior() const
{
  return MustPlaceInOwnLayer;
}

bool QgsLayoutItemLegend::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  std::function<bool( QgsLayerTreeGroup *group ) >visit;

  visit = [this, visitor, &visit]( QgsLayerTreeGroup * group ) -> bool
  {
    const QList<QgsLayerTreeNode *> childNodes = group->children();
    for ( QgsLayerTreeNode *node : childNodes )
    {
      if ( QgsLayerTree::isGroup( node ) )
      {
        QgsLayerTreeGroup *nodeGroup = QgsLayerTree::toGroup( node );
        if ( !visit( nodeGroup ) )
          return false;
      }
      else if ( QgsLayerTree::isLayer( node ) )
      {
        QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
        if ( !nodeLayer->patchShape().isNull() )
        {
          QgsStyleLegendPatchShapeEntity entity( nodeLayer->patchShape() );
          if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, uuid(), displayName() ) ) )
            return false;
        }
        const QList<QgsLayerTreeModelLegendNode *> legendNodes = mLegendModel->layerLegendNodes( nodeLayer );
        for ( QgsLayerTreeModelLegendNode *legendNode : legendNodes )
        {
          if ( QgsSymbolLegendNode *symbolNode = dynamic_cast< QgsSymbolLegendNode * >( legendNode ) )
          {
            if ( !symbolNode->patchShape().isNull() )
            {
              QgsStyleLegendPatchShapeEntity entity( symbolNode->patchShape() );
              if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, uuid(), displayName() ) ) )
                return false;
            }
          }
        }
      }
    }
    return true;
  };
  return visit( mLegendModel->rootGroup( ) );
}

bool QgsLayoutItemLegend::isRefreshing() const
{
  return mLegendModel->hitTestInProgress();
}


// -------------------------------------------------------------------------

QgsLegendModel::QgsLegendModel( QgsLayerTree *rootNode, QObject *parent, QgsLayoutItemLegend *layout )
  : QgsLayerTreeModel( rootNode, parent )
  , mLayoutLegend( layout )
{
  setFlag( QgsLayerTreeModel::AllowLegendChangeState, false );
  setFlag( QgsLayerTreeModel::AllowNodeReorder, true );
  setFlag( QgsLayerTreeModel::UseThreadedHitTest, true );
  connect( this, &QgsLegendModel::dataChanged, this, &QgsLegendModel::refreshLegend );
}

QgsLegendModel::QgsLegendModel( QgsLayerTree *rootNode,  QgsLayoutItemLegend *layout )
  : QgsLayerTreeModel( rootNode )
  , mLayoutLegend( layout )
{
  setFlag( QgsLayerTreeModel::AllowLegendChangeState, false );
  setFlag( QgsLayerTreeModel::AllowNodeReorder, true );
  setFlag( QgsLayerTreeModel::UseThreadedHitTest, true );
  connect( this, &QgsLegendModel::dataChanged, this, &QgsLegendModel::refreshLegend );
}

QVariant QgsLegendModel::data( const QModelIndex &index, int role ) const
{
  // handle custom layer node labels

  QgsLayerTreeNode *node = index2node( index );
  QgsLayerTreeLayer *nodeLayer = QgsLayerTree::isLayer( node ) ? QgsLayerTree::toLayer( node ) : nullptr;
  if ( nodeLayer && ( role == Qt::DisplayRole || role == Qt::EditRole ) )
  {
    QString name = node->customProperty( QStringLiteral( "cached_name" ) ).toString();
    if ( !name.isEmpty() )
      return name;

    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( nodeLayer->layer() );

    //finding the first label that is stored
    name = nodeLayer->customProperty( QStringLiteral( "legend/title-label" ) ).toString();
    if ( name.isEmpty() )
      name = nodeLayer->name();
    if ( name.isEmpty() )
      name = node->customProperty( QStringLiteral( "legend/title-label" ) ).toString();
    if ( name.isEmpty() )
      name = node->name();
    if ( nodeLayer->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toInt() )
    {
      if ( vlayer && vlayer->featureCount() >= 0 )
      {
        name += QStringLiteral( " [%1]" ).arg( vlayer->featureCount() );
        node->setCustomProperty( QStringLiteral( "cached_name" ), name );
        return name;
      }
    }
    node->setCustomProperty( QStringLiteral( "cached_name" ), name );
    return name;
  }
  return QgsLayerTreeModel::data( index, role );
}

Qt::ItemFlags QgsLegendModel::flags( const QModelIndex &index ) const
{
  // make the legend nodes selectable even if they are not by default
  if ( index2legendNode( index ) )
    return QgsLayerTreeModel::flags( index ) | Qt::ItemIsSelectable;

  return QgsLayerTreeModel::flags( index );
}

QList<QgsLayerTreeModelLegendNode *> QgsLegendModel::layerLegendNodes( QgsLayerTreeLayer *nodeLayer, bool skipNodeEmbeddedInParent ) const
{
  if ( !mLegend.contains( nodeLayer ) )
    return QList<QgsLayerTreeModelLegendNode *>();

  const LayerLegendData &data = mLegend[nodeLayer];
  QList<QgsLayerTreeModelLegendNode *> lst( data.activeNodes );
  if ( !skipNodeEmbeddedInParent && data.embeddedNodeInParent )
    lst.prepend( data.embeddedNodeInParent );
  return lst;
}

void QgsLegendModel::clearCachedData( QgsLayerTreeNode *node ) const
{
  node->removeCustomProperty( QStringLiteral( "cached_name" ) );
}

void QgsLegendModel::forceRefresh()
{
  emit refreshLegend();
}
