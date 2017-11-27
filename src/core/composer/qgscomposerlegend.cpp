/***************************************************************************
                         qgscomposerlegend.cpp  -  description
                         ---------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
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

#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgscomposermodel.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslegendrenderer.h"
#include "qgslegendstyle.h"
#include "qgslogger.h"
#include "qgsmapsettings.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgslayertreeutils.h"
#include <QDomDocument>
#include <QDomElement>
#include <QPainter>
#include "qgslayoutitemlegend.h"

QgsComposerLegend::QgsComposerLegend( QgsComposition *composition )
  : QgsComposerItem( composition )
  , mLegendModel( new QgsLegendModel( mComposition->project()->layerTreeRoot() ) )
{
  connect( &composition->atlasComposition(), &QgsAtlasComposition::renderEnded, this, &QgsComposerLegend::onAtlasEnded );
  connect( &composition->atlasComposition(), &QgsAtlasComposition::featureChanged, this, &QgsComposerLegend::onAtlasFeature );

  // Connect to the main layertreeroot.
  // It serves in "auto update mode" as a medium between the main app legend and this one
  connect( mComposition->project()->layerTreeRoot(), &QgsLayerTreeNode::customPropertyChanged, this, &QgsComposerLegend::nodeCustomPropertyChanged );
}

QgsComposerLegend::~QgsComposerLegend()
{
  delete mLegendModel;
}

QgsComposerLegend::QgsComposerLegend()
  : QgsComposerItem( nullptr )
{

}

void QgsComposerLegend::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );

  if ( !painter )
    return;

  if ( !shouldDrawItem() )
  {
    return;
  }

  if ( mFilterAskedForUpdate )
  {
    mFilterAskedForUpdate = false;
    doUpdateFilterByMap();
  }

  int dpi = painter->device()->logicalDpiX();
  double dotsPerMM = dpi / 25.4;

  if ( mComposition )
  {
    mSettings.setUseAdvancedEffects( mComposition->useAdvancedEffects() );
    mSettings.setDpi( dpi );
  }
  if ( mComposerMap )
  {
    mSettings.setMmPerMapUnit( mComposerMap->mapUnitsToMM() );

    // use a temporary QgsMapSettings to find out real map scale
    QSizeF mapSizePixels = QSizeF( mComposerMap->rect().width() * dotsPerMM, mComposerMap->rect().height() * dotsPerMM );
    QgsRectangle mapExtent = *mComposerMap->currentMapExtent();

    QgsMapSettings ms = mComposerMap->mapSettings( mapExtent, mapSizePixels, dpi );
    mSettings.setMapScale( ms.scale() );
  }
  mInitialMapScaleCalculated = true;

  QgsLegendRenderer legendRenderer( mLegendModel, mSettings );
  legendRenderer.setLegendSize( mForceResize && mSizeToContents ? QSize() : rect().size() );

  //adjust box if width or height is too small
  if ( mSizeToContents )
  {
    QSizeF size = legendRenderer.minimumSize();
    if ( mForceResize )
    {
      mForceResize = false;
      //set new rect, respecting position mode and data defined size/position
      QRectF targetRect = QRectF( pos().x(), pos().y(), size.width(), size.height() );
      setSceneRect( evalItemRect( targetRect, true ) );
    }
    else if ( size.height() > rect().height() || size.width() > rect().width() )
    {
      //need to resize box
      QRectF targetRect = QRectF( pos().x(), pos().y(), rect().width(), rect().height() );
      if ( size.height() > targetRect.height() )
        targetRect.setHeight( size.height() );
      if ( size.width() > rect().width() )
        targetRect.setWidth( size.width() );

      //set new rect, respecting position mode and data defined size/position
      setSceneRect( evalItemRect( targetRect, true ) );
    }
  }

  drawBackground( painter );
  painter->save();
  //antialiasing on
  painter->setRenderHint( QPainter::Antialiasing, true );
  painter->setPen( QPen( QColor( 0, 0, 0 ) ) );

  if ( !mSizeToContents )
  {
    // set a clip region to crop out parts of legend which don't fit
    QRectF thisPaintRect = QRectF( 0, 0, rect().width(), rect().height() );
    painter->setClipRect( thisPaintRect );
  }

  legendRenderer.drawLegend( painter );

  painter->restore();

  //draw frame and selection boxes if necessary
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

QSizeF QgsComposerLegend::paintAndDetermineSize( QPainter *painter )
{
  if ( mFilterAskedForUpdate )
  {
    mFilterAskedForUpdate = false;
    doUpdateFilterByMap();
  }

  QgsLegendRenderer legendRenderer( mLegendModel, mSettings );
  QSizeF size = legendRenderer.minimumSize();
  if ( painter )
    legendRenderer.drawLegend( painter );
  return size;
}


void QgsComposerLegend::adjustBoxSize()
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

  QgsLegendRenderer legendRenderer( mLegendModel, mSettings );
  QSizeF size = legendRenderer.minimumSize();
  QgsDebugMsg( QString( "width = %1 height = %2" ).arg( size.width() ).arg( size.height() ) );
  if ( size.isValid() )
  {
    QRectF targetRect = QRectF( pos().x(), pos().y(), size.width(), size.height() );
    //set new rect, respecting position mode and data defined size/position
    setSceneRect( evalItemRect( targetRect, true ) );
  }
}

void QgsComposerLegend::setResizeToContents( bool enabled )
{
  mSizeToContents = enabled;
}

bool QgsComposerLegend::resizeToContents() const
{
  return mSizeToContents;
}

QgsLegendModel *QgsComposerLegend::model()
{
  return mLegendModel;
}

void QgsComposerLegend::setCustomLayerTree( QgsLayerTree *rootGroup )
{
  mLegendModel->setRootGroup( rootGroup ? rootGroup : ( mComposition ? mComposition->project()->layerTreeRoot() : nullptr ) );

  mCustomLayerTree.reset( rootGroup );
}


void QgsComposerLegend::setAutoUpdateModel( bool autoUpdate )
{
  if ( autoUpdate == autoUpdateModel() )
    return;

  setCustomLayerTree( autoUpdate ? nullptr : mComposition->project()->layerTreeRoot()->clone() );
  adjustBoxSize();
  updateItem();
}

void QgsComposerLegend::nodeCustomPropertyChanged( QgsLayerTreeNode *, const QString & )
{
  if ( autoUpdateModel() )
  {
    // in "auto update" mode, some parameters on the main app legend may have been changed (expression filtering)
    // we must then call updateItem to reflect the changes
    updateItem();
  }
}

bool QgsComposerLegend::autoUpdateModel() const
{
  return !mCustomLayerTree;
}

void QgsComposerLegend::setLegendFilterByMapEnabled( bool enabled )
{
  mLegendFilterByMap = enabled;
  updateItem();
}

void QgsComposerLegend::setTitle( const QString &t )
{
  mTitle = t;
  mSettings.setTitle( t );

  if ( mComposition && id().isEmpty() )
  {
    //notify the model that the display name has changed
    mComposition->itemsModel()->updateItemDisplayName( this );
  }
}
QString QgsComposerLegend::title() const { return mTitle; }

Qt::AlignmentFlag QgsComposerLegend::titleAlignment() const { return mSettings.titleAlignment(); }
void QgsComposerLegend::setTitleAlignment( Qt::AlignmentFlag alignment ) { mSettings.setTitleAlignment( alignment ); }

QgsLegendStyle &QgsComposerLegend::rstyle( QgsLegendStyle::Style s ) { return mSettings.rstyle( s ); }
QgsLegendStyle QgsComposerLegend::style( QgsLegendStyle::Style s ) const { return mSettings.style( s ); }
void QgsComposerLegend::setStyle( QgsLegendStyle::Style s, const QgsLegendStyle &style ) { mSettings.setStyle( s, style ); }

QFont QgsComposerLegend::styleFont( QgsLegendStyle::Style s ) const { return mSettings.style( s ).font(); }
void QgsComposerLegend::setStyleFont( QgsLegendStyle::Style s, const QFont &f ) { rstyle( s ).setFont( f ); }

void QgsComposerLegend::setStyleMargin( QgsLegendStyle::Style s, double margin ) { rstyle( s ).setMargin( margin ); }
void QgsComposerLegend::setStyleMargin( QgsLegendStyle::Style s, QgsLegendStyle::Side side, double margin ) { rstyle( s ).setMargin( side, margin ); }

double QgsComposerLegend::lineSpacing() const { return mSettings.lineSpacing(); }
void QgsComposerLegend::setLineSpacing( double spacing ) { mSettings.setLineSpacing( spacing ); }

double QgsComposerLegend::boxSpace() const { return mSettings.boxSpace(); }
void QgsComposerLegend::setBoxSpace( double s ) { mSettings.setBoxSpace( s ); }

double QgsComposerLegend::columnSpace() const { return mSettings.columnSpace(); }
void QgsComposerLegend::setColumnSpace( double s ) { mSettings.setColumnSpace( s ); }

QColor QgsComposerLegend::fontColor() const { return mSettings.fontColor(); }
void QgsComposerLegend::setFontColor( const QColor &c ) { mSettings.setFontColor( c ); }

double QgsComposerLegend::symbolWidth() const { return mSettings.symbolSize().width(); }
void QgsComposerLegend::setSymbolWidth( double w ) { mSettings.setSymbolSize( QSizeF( w, mSettings.symbolSize().height() ) ); }

double QgsComposerLegend::symbolHeight() const { return mSettings.symbolSize().height(); }
void QgsComposerLegend::setSymbolHeight( double h ) { mSettings.setSymbolSize( QSizeF( mSettings.symbolSize().width(), h ) ); }

double QgsComposerLegend::wmsLegendWidth() const { return mSettings.wmsLegendSize().width(); }
void QgsComposerLegend::setWmsLegendWidth( double w ) { mSettings.setWmsLegendSize( QSizeF( w, mSettings.wmsLegendSize().height() ) ); }

double QgsComposerLegend::wmsLegendHeight() const {return mSettings.wmsLegendSize().height(); }
void QgsComposerLegend::setWmsLegendHeight( double h ) { mSettings.setWmsLegendSize( QSizeF( mSettings.wmsLegendSize().width(), h ) ); }

void QgsComposerLegend::setWrapChar( const QString &t ) { mSettings.setWrapChar( t ); }
QString QgsComposerLegend::wrapChar() const {return mSettings.wrapChar(); }

int QgsComposerLegend::columnCount() const { return mColumnCount; }
void QgsComposerLegend::setColumnCount( int c ) { mColumnCount = c; mSettings.setColumnCount( c ); }

bool QgsComposerLegend::splitLayer() const { return mSettings.splitLayer(); }
void QgsComposerLegend::setSplitLayer( bool s ) { mSettings.setSplitLayer( s ); }

bool QgsComposerLegend::equalColumnWidth() const { return mSettings.equalColumnWidth(); }
void QgsComposerLegend::setEqualColumnWidth( bool s ) { mSettings.setEqualColumnWidth( s ); }

bool QgsComposerLegend::drawRasterStroke() const { return mSettings.drawRasterStroke(); }
void QgsComposerLegend::setDrawRasterStroke( bool enabled ) { mSettings.setDrawRasterStroke( enabled ); }

QColor QgsComposerLegend::rasterStrokeColor() const { return mSettings.rasterStrokeColor(); }
void QgsComposerLegend::setRasterStrokeColor( const QColor &color ) { mSettings.setRasterStrokeColor( color ); }

double QgsComposerLegend::rasterStrokeWidth() const { return mSettings.rasterStrokeWidth(); }
void QgsComposerLegend::setRasterStrokeWidth( double width ) { mSettings.setRasterStrokeWidth( width ); }

void QgsComposerLegend::synchronizeWithModel()
{
  adjustBoxSize();
  updateItem();
}

void QgsComposerLegend::updateLegend()
{
  adjustBoxSize();
  updateItem();
}

void QgsComposerLegend::updateItem()
{
  if ( !updatesEnabled() )
    return;

  updateFilterByMap( false );
  QgsComposerItem::updateItem();
}

bool QgsComposerLegend::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QgsPathResolver  pathResolver;
  if ( mComposition )
    pathResolver = mComposition->project()->pathResolver();
  QgsReadWriteContext context;
  context.setPathResolver( pathResolver );

  QDomElement composerLegendElem = doc.createElement( QStringLiteral( "ComposerLegend" ) );
  elem.appendChild( composerLegendElem );

  //write general properties
  composerLegendElem.setAttribute( QStringLiteral( "title" ), mTitle );
  composerLegendElem.setAttribute( QStringLiteral( "titleAlignment" ), QString::number( static_cast< int >( mSettings.titleAlignment() ) ) );
  composerLegendElem.setAttribute( QStringLiteral( "columnCount" ), QString::number( mColumnCount ) );
  composerLegendElem.setAttribute( QStringLiteral( "splitLayer" ), QString::number( mSettings.splitLayer() ) );
  composerLegendElem.setAttribute( QStringLiteral( "equalColumnWidth" ), QString::number( mSettings.equalColumnWidth() ) );

  composerLegendElem.setAttribute( QStringLiteral( "boxSpace" ), QString::number( mSettings.boxSpace() ) );
  composerLegendElem.setAttribute( QStringLiteral( "columnSpace" ), QString::number( mSettings.columnSpace() ) );

  composerLegendElem.setAttribute( QStringLiteral( "symbolWidth" ), QString::number( mSettings.symbolSize().width() ) );
  composerLegendElem.setAttribute( QStringLiteral( "symbolHeight" ), QString::number( mSettings.symbolSize().height() ) );
  composerLegendElem.setAttribute( QStringLiteral( "lineSpacing" ), QString::number( mSettings.lineSpacing() ) );

  composerLegendElem.setAttribute( QStringLiteral( "rasterBorder" ), mSettings.drawRasterStroke() );
  composerLegendElem.setAttribute( QStringLiteral( "rasterBorderColor" ), QgsSymbolLayerUtils::encodeColor( mSettings.rasterStrokeColor() ) );
  composerLegendElem.setAttribute( QStringLiteral( "rasterBorderWidth" ), QString::number( mSettings.rasterStrokeWidth() ) );

  composerLegendElem.setAttribute( QStringLiteral( "wmsLegendWidth" ), QString::number( mSettings.wmsLegendSize().width() ) );
  composerLegendElem.setAttribute( QStringLiteral( "wmsLegendHeight" ), QString::number( mSettings.wmsLegendSize().height() ) );
  composerLegendElem.setAttribute( QStringLiteral( "wrapChar" ), mSettings.wrapChar() );
  composerLegendElem.setAttribute( QStringLiteral( "fontColor" ), mSettings.fontColor().name() );

  composerLegendElem.setAttribute( QStringLiteral( "resizeToContents" ), mSizeToContents );

  if ( mComposerMap )
  {
    composerLegendElem.setAttribute( QStringLiteral( "map" ), mComposerMap->id() );
  }

  QDomElement composerLegendStyles = doc.createElement( QStringLiteral( "styles" ) );
  composerLegendElem.appendChild( composerLegendStyles );

  style( QgsLegendStyle::Title ).writeXml( QStringLiteral( "title" ), composerLegendStyles, doc );
  style( QgsLegendStyle::Group ).writeXml( QStringLiteral( "group" ), composerLegendStyles, doc );
  style( QgsLegendStyle::Subgroup ).writeXml( QStringLiteral( "subgroup" ), composerLegendStyles, doc );
  style( QgsLegendStyle::Symbol ).writeXml( QStringLiteral( "symbol" ), composerLegendStyles, doc );
  style( QgsLegendStyle::SymbolLabel ).writeXml( QStringLiteral( "symbolLabel" ), composerLegendStyles, doc );

  if ( mCustomLayerTree )
  {
    // if not using auto-update - store the custom layer tree
    mCustomLayerTree->writeXml( composerLegendElem, context );
  }

  if ( mLegendFilterByMap )
  {
    composerLegendElem.setAttribute( QStringLiteral( "legendFilterByMap" ), QStringLiteral( "1" ) );
  }
  composerLegendElem.setAttribute( QStringLiteral( "legendFilterByAtlas" ), mFilterOutAtlas ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  return _writeXml( composerLegendElem, doc );
}

bool QgsComposerLegend::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  QgsPathResolver  pathResolver;
  if ( mComposition )
    pathResolver = mComposition->project()->pathResolver();
  QgsReadWriteContext context;
  context.setPathResolver( pathResolver );

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

  QDomNodeList stylesNodeList = itemElem.elementsByTagName( QStringLiteral( "styles" ) );
  if ( !stylesNodeList.isEmpty() )
  {
    QDomNode stylesNode = stylesNodeList.at( 0 );
    for ( int i = 0; i < stylesNode.childNodes().size(); i++ )
    {
      QDomElement styleElem = stylesNode.childNodes().at( i ).toElement();
      QgsLegendStyle style;
      style.readXml( styleElem, doc );
      QString name = styleElem.attribute( QStringLiteral( "name" ) );
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
  QColor fontClr;
  fontClr.setNamedColor( itemElem.attribute( QStringLiteral( "fontColor" ), QStringLiteral( "#000000" ) ) );
  mSettings.setFontColor( fontClr );

  //spaces
  mSettings.setBoxSpace( itemElem.attribute( QStringLiteral( "boxSpace" ), QStringLiteral( "2.0" ) ).toDouble() );
  mSettings.setColumnSpace( itemElem.attribute( QStringLiteral( "columnSpace" ), QStringLiteral( "2.0" ) ).toDouble() );

  mSettings.setSymbolSize( QSizeF( itemElem.attribute( QStringLiteral( "symbolWidth" ), QStringLiteral( "7.0" ) ).toDouble(), itemElem.attribute( QStringLiteral( "symbolHeight" ), QStringLiteral( "14.0" ) ).toDouble() ) );
  mSettings.setWmsLegendSize( QSizeF( itemElem.attribute( QStringLiteral( "wmsLegendWidth" ), QStringLiteral( "50" ) ).toDouble(), itemElem.attribute( QStringLiteral( "wmsLegendHeight" ), QStringLiteral( "25" ) ).toDouble() ) );
  mSettings.setLineSpacing( itemElem.attribute( QStringLiteral( "lineSpacing" ), QStringLiteral( "1.0" ) ).toDouble() );

  mSettings.setDrawRasterStroke( itemElem.attribute( QStringLiteral( "rasterBorder" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  mSettings.setRasterStrokeColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "rasterBorderColor" ), QStringLiteral( "0,0,0" ) ) ) );
  mSettings.setRasterStrokeWidth( itemElem.attribute( QStringLiteral( "rasterBorderWidth" ), QStringLiteral( "0" ) ).toDouble() );

  mSettings.setWrapChar( itemElem.attribute( QStringLiteral( "wrapChar" ) ) );

  mSizeToContents = itemElem.attribute( QStringLiteral( "resizeToContents" ), QStringLiteral( "1" ) ) != QLatin1String( "0" );

  //composer map
  mLegendFilterByMap = itemElem.attribute( QStringLiteral( "legendFilterByMap" ), QStringLiteral( "0" ) ).toInt();
  if ( !itemElem.attribute( QStringLiteral( "map" ) ).isEmpty() )
  {
    setComposerMap( mComposition->getComposerMapById( itemElem.attribute( QStringLiteral( "map" ) ).toInt() ) );
  }
  mFilterOutAtlas = itemElem.attribute( QStringLiteral( "legendFilterByAtlas" ), QStringLiteral( "0" ) ).toInt();

  // QGIS >= 2.6
  QDomElement layerTreeElem = itemElem.firstChildElement( QStringLiteral( "layer-tree" ) );
  if ( layerTreeElem.isNull() )
    layerTreeElem = itemElem.firstChildElement( QStringLiteral( "layer-tree-group" ) );

  if ( !layerTreeElem.isNull() )
  {
    std::unique_ptr< QgsLayerTree > tree( QgsLayerTree::readXml( layerTreeElem, context ) );
    if ( mComposition )
      tree->resolveReferences( mComposition->project(), true );
    setCustomLayerTree( tree.release() );
  }
  else
    setCustomLayerTree( nullptr );

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( QStringLiteral( "ComposerItem" ) );
  if ( !composerItemList.isEmpty() )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXml( composerItemElem, doc );
  }

  // < 2.0 projects backward compatibility >>>>>
  //title font
  QString titleFontString = itemElem.attribute( QStringLiteral( "titleFont" ) );
  if ( !titleFontString.isEmpty() )
  {
    rstyle( QgsLegendStyle::Title ).rfont().fromString( titleFontString );
  }
  //group font
  QString groupFontString = itemElem.attribute( QStringLiteral( "groupFont" ) );
  if ( !groupFontString.isEmpty() )
  {
    rstyle( QgsLegendStyle::Group ).rfont().fromString( groupFontString );
  }

  //layer font
  QString layerFontString = itemElem.attribute( QStringLiteral( "layerFont" ) );
  if ( !layerFontString.isEmpty() )
  {
    rstyle( QgsLegendStyle::Subgroup ).rfont().fromString( layerFontString );
  }
  //item font
  QString itemFontString = itemElem.attribute( QStringLiteral( "itemFont" ) );
  if ( !itemFontString.isEmpty() )
  {
    rstyle( QgsLegendStyle::SymbolLabel ).rfont().fromString( itemFontString );
  }

  if ( !itemElem.attribute( QStringLiteral( "groupSpace" ) ).isEmpty() )
  {
    rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Top, itemElem.attribute( QStringLiteral( "groupSpace" ), QStringLiteral( "3.0" ) ).toDouble() );
  }
  if ( !itemElem.attribute( QStringLiteral( "layerSpace" ) ).isEmpty() )
  {
    rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Top, itemElem.attribute( QStringLiteral( "layerSpace" ), QStringLiteral( "3.0" ) ).toDouble() );
  }
  if ( !itemElem.attribute( QStringLiteral( "symbolSpace" ) ).isEmpty() )
  {
    rstyle( QgsLegendStyle::Symbol ).setMargin( QgsLegendStyle::Top, itemElem.attribute( QStringLiteral( "symbolSpace" ), QStringLiteral( "2.0" ) ).toDouble() );
    rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Top, itemElem.attribute( QStringLiteral( "symbolSpace" ), QStringLiteral( "2.0" ) ).toDouble() );
  }
  // <<<<<<< < 2.0 projects backward compatibility

  emit itemChanged();
  return true;
}

QString QgsComposerLegend::displayName() const
{
  if ( !id().isEmpty() )
  {
    return id();
  }

  //if no id, default to portion of title text
  QString text = mSettings.title();
  if ( text.isEmpty() )
  {
    return tr( "<legend>" );
  }
  if ( text.length() > 25 )
  {
    return QString( tr( "%1..." ) ).arg( text.left( 25 ) );
  }
  else
  {
    return text;
  }
}

void QgsComposerLegend::setComposerMap( const QgsComposerMap *map )
{
  if ( mComposerMap )
  {
    disconnect( mComposerMap, &QObject::destroyed, this, &QgsComposerLegend::invalidateCurrentMap );
    disconnect( mComposerMap, &QgsComposerObject::itemChanged, this, &QgsComposerLegend::updateFilterByMapAndRedraw );
    disconnect( mComposerMap, &QgsComposerMap::extentChanged, this, &QgsComposerLegend::updateFilterByMapAndRedraw );
    disconnect( mComposerMap, &QgsComposerMap::layerStyleOverridesChanged, this, &QgsComposerLegend::mapLayerStyleOverridesChanged );
  }

  mComposerMap = map;

  if ( map )
  {
    connect( map, &QObject::destroyed, this, &QgsComposerLegend::invalidateCurrentMap );
    connect( map, &QgsComposerObject::itemChanged, this, &QgsComposerLegend::updateFilterByMapAndRedraw );
    connect( map, &QgsComposerMap::extentChanged, this, &QgsComposerLegend::updateFilterByMapAndRedraw );
    connect( map, &QgsComposerMap::layerStyleOverridesChanged, this, &QgsComposerLegend::mapLayerStyleOverridesChanged );
  }

  updateItem();
}

void QgsComposerLegend::invalidateCurrentMap()
{
  setComposerMap( nullptr );
}

void QgsComposerLegend::refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, const QgsExpressionContext *context )
{
  QgsExpressionContext scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  bool forceUpdate = false;
  //updates data defined properties and redraws item to match
  if ( property == QgsComposerObject::LegendTitle || property == QgsComposerObject::AllProperties )
  {
    bool ok = false;
    QString t = mDataDefinedProperties.valueAsString( QgsComposerObject::LegendTitle, *evalContext, mTitle, &ok );
    if ( ok )
    {
      mSettings.setTitle( t );
      forceUpdate = true;
    }
  }
  if ( property == QgsComposerObject::LegendColumnCount || property == QgsComposerObject::AllProperties )
  {
    bool ok = false;
    int cols = mDataDefinedProperties.valueAsInt( QgsComposerObject::LegendColumnCount, *evalContext, mColumnCount, &ok );
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

  QgsComposerObject::refreshDataDefinedProperty( property, context );
}

void QgsComposerLegend::updateFilterByMapAndRedraw()
{
  updateFilterByMap( true );
}

void QgsComposerLegend::mapLayerStyleOverridesChanged()
{
  if ( !mComposerMap )
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
    mLegendModel->setLayerStyleOverrides( mComposerMap->layerStyleOverrides() );

    Q_FOREACH ( QgsLayerTreeLayer *nodeLayer, mLegendModel->rootGroup()->findLayers() )
      mLegendModel->refreshLayerLegend( nodeLayer );
  }

  adjustBoxSize();
  updateItem();
}

void QgsComposerLegend::updateFilterByMap( bool redraw )
{
  if ( isRemoved() )
    return;
  // ask for update
  // the actual update will take place before the redraw.
  // This is to avoid multiple calls to the filter
  mFilterAskedForUpdate = true;

  if ( redraw )
    QgsComposerItem::updateItem();
}

void QgsComposerLegend::doUpdateFilterByMap()
{
  if ( mComposerMap )
    mLegendModel->setLayerStyleOverrides( mComposerMap->layerStyleOverrides() );
  else
    mLegendModel->setLayerStyleOverrides( QMap<QString, QString>() );


  bool filterByExpression = QgsLayerTreeUtils::hasLegendFilterExpression( *( mCustomLayerTree ? mCustomLayerTree.get() : mComposition->project()->layerTreeRoot() ) );

  if ( mComposerMap && ( mLegendFilterByMap || filterByExpression || mInAtlas ) )
  {
    int dpi = mComposition->printResolution();

    QgsRectangle requestRectangle;
    mComposerMap->requestedExtent( requestRectangle );

    QSizeF size( requestRectangle.width(), requestRectangle.height() );
    size *= mComposerMap->mapUnitsToMM() * dpi / 25.4;

    QgsMapSettings ms = mComposerMap->mapSettings( requestRectangle, size, dpi );

    QgsGeometry filterPolygon;
    if ( mInAtlas )
    {
      filterPolygon = composition()->atlasComposition().currentGeometry( mComposerMap->crs() );
    }
    mLegendModel->setLegendFilter( &ms, /* useExtent */ mInAtlas || mLegendFilterByMap, filterPolygon, /* useExpressions */ true );
  }
  else
    mLegendModel->setLegendFilterByMap( nullptr );

  mForceResize = true;
}

void QgsComposerLegend::setLegendFilterOutAtlas( bool doFilter )
{
  mFilterOutAtlas = doFilter;
}

bool QgsComposerLegend::legendFilterOutAtlas() const
{
  return mFilterOutAtlas;
}

void QgsComposerLegend::onAtlasFeature( QgsFeature *feat )
{
  if ( !feat )
    return;
  mInAtlas = mFilterOutAtlas;
  updateFilterByMap();
}

void QgsComposerLegend::onAtlasEnded()
{
  mInAtlas = false;
  updateFilterByMap();
}
