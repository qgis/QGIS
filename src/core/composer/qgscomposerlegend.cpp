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

#include "qgscomposerlegendstyle.h"
#include "qgscomposerlegend.h"
#include "qgscomposerlegenditem.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgssymbolv2.h"
#include <QDomDocument>
#include <QDomElement>
#include <QPainter>

QgsComposerLegend::QgsComposerLegend( QgsComposition* composition )
    : QgsComposerItem( composition )
    , mTitle( tr( "Legend" ) )
    , mFontColor( QColor( 0, 0, 0 ) )
    , mBoxSpace( 2 )
    , mColumnSpace( 2 )
    , mColumnCount( 1 )
    , mComposerMap( 0 )
    , mSplitLayer( false )
    , mEqualColumnWidth( false )
{
  setStyleMargin( QgsComposerLegendStyle::Group, QgsComposerLegendStyle::Top, 2 );
  setStyleMargin( QgsComposerLegendStyle::Subgroup, QgsComposerLegendStyle::Top, 2 );
  setStyleMargin( QgsComposerLegendStyle::Symbol, QgsComposerLegendStyle::Top, 2 );
  setStyleMargin( QgsComposerLegendStyle::SymbolLabel, QgsComposerLegendStyle::Top, 2 );
  setStyleMargin( QgsComposerLegendStyle::SymbolLabel, QgsComposerLegendStyle::Left, 2 );
  rstyle( QgsComposerLegendStyle::Title ).rfont().setPointSizeF( 16.0 );
  rstyle( QgsComposerLegendStyle::Group ).rfont().setPointSizeF( 14.0 );
  rstyle( QgsComposerLegendStyle::Subgroup ).rfont().setPointSizeF( 12.0 );
  rstyle( QgsComposerLegendStyle::SymbolLabel ).rfont().setPointSizeF( 12.0 );

  mSymbolWidth = 7;
  mSymbolHeight = 4;
  mWrapChar = "";
  mlineSpacing = 1.5;
  adjustBoxSize();

  connect( &mLegendModel, SIGNAL( layersChanged() ), this, SLOT( synchronizeWithModel() ) );
}

QgsComposerLegend::QgsComposerLegend(): QgsComposerItem( 0 ), mComposerMap( 0 )
{

}

QgsComposerLegend::~QgsComposerLegend()
{

}

void QgsComposerLegend::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  paintAndDetermineSize( painter );
}

QSizeF QgsComposerLegend::paintAndDetermineSize( QPainter* painter )
{
  QSizeF size( 0, 0 );
  QStandardItem* rootItem = mLegendModel.invisibleRootItem();
  if ( !rootItem ) return size;

  if ( painter )
  {
    painter->save();
    drawBackground( painter );
    painter->setPen( QPen( QColor( 0, 0, 0 ) ) );
  }

  QList<Atom> atomList = createAtomList( rootItem, mSplitLayer );

  setColumns( atomList );

  qreal maxColumnWidth = 0;
  if ( mEqualColumnWidth )
  {
    foreach ( Atom atom, atomList )
    {
      maxColumnWidth = qMax( atom.size.width(), maxColumnWidth );
    }
  }

  QSizeF titleSize = drawTitle();
  // Using mGroupSpace as space between legend title and first atom in column
  //double columnTop = mBoxSpace + titleSize.height() + mGroupSpace;
  // TODO: use margin of first used style
  double columnTop = mBoxSpace + titleSize.height() + style( QgsComposerLegendStyle::Group ).margin( QgsComposerLegendStyle::Top );

  QPointF point( mBoxSpace, columnTop );
  // bool firstInColumn = true;
  double columnMaxHeight = 0;
  qreal columnWidth = 0;
  int column = 0;
  foreach ( Atom atom, atomList )
  {
    if ( atom.column > column )
    {
      // Switch to next column
      if ( mEqualColumnWidth )
      {
        point.rx() += mColumnSpace + maxColumnWidth;
      }
      else
      {
        point.rx() += mColumnSpace + columnWidth;
      }
      point.ry() = columnTop;
      columnWidth = 0;
      column++;
      // firstInColumn = true;
    }
    // Add space if necessary, unfortunately it depends on first nucleon
    //if ( !firstInColumn )
    //{
    point.ry() += spaceAboveAtom( atom );
    //}

    QSizeF atomSize = drawAtom( atom, painter, point );
    columnWidth = qMax( atomSize.width(), columnWidth );

    point.ry() += atom.size.height();
    columnMaxHeight = qMax( point.y() - columnTop, columnMaxHeight );

    // firstInColumn = false;
  }
  point.rx() += columnWidth + mBoxSpace;

  size.rheight() = columnTop + columnMaxHeight + mBoxSpace;
  size.rwidth() = point.x();

  // Now we know total width and can draw the title centered
  if ( !mTitle.isEmpty() )
  {
    // For multicolumn center if we stay in totalWidth, otherwise allign to left
    // and expand total width. With single column keep alligned to left be cause
    // it looks better alligned with items bellow instead of centered
    Qt::AlignmentFlag halignment;
    if ( mColumnCount > 1 && titleSize.width() + 2 * mBoxSpace < size.width() )
    {
      halignment = Qt::AlignHCenter;
      point.rx() = mBoxSpace + size.rwidth() / 2;
    }
    else
    {
      halignment = Qt::AlignLeft;
      point.rx() = mBoxSpace;
      size.rwidth() = qMax( titleSize.width() + 2 * mBoxSpace, size.width() );
    }
    point.ry() = mBoxSpace;
    drawTitle( painter, point, halignment );
  }

  //adjust box if width or height is to small
  if ( painter && size.height() > rect().height() )
  {
    setSceneRect( QRectF( transform().dx(), transform().dy(), rect().width(), size.height() ) );
  }
  if ( painter && size.width() > rect().width() )
  {
    setSceneRect( QRectF( transform().dx(), transform().dy(), size.width(), rect().height() ) );
  }

  if ( painter )
  {
    painter->restore();

    //draw frame and selection boxes if necessary
    drawFrame( painter );
    if ( isSelected() )
    {
      drawSelectionBoxes( painter );
    }
  }

  return size;
}

QSizeF QgsComposerLegend::drawTitle( QPainter* painter, QPointF point, Qt::AlignmentFlag halignment )
{
  QSizeF size( 0, 0 );
  if ( mTitle.isEmpty() ) return size;

  QStringList lines = splitStringForWrapping( mTitle );

  double y = point.y();

  if ( painter ) painter->setPen( mFontColor );

  for ( QStringList::Iterator titlePart = lines.begin(); titlePart != lines.end(); ++titlePart )
  {
    // it does not draw the last world if rectangle width is exactly text width
    qreal width = textWidthMillimeters( styleFont( QgsComposerLegendStyle::Title ), *titlePart ) + 1;
    qreal height = fontAscentMillimeters( styleFont( QgsComposerLegendStyle::Title ) ) + fontDescentMillimeters( styleFont( QgsComposerLegendStyle::Title ) );

    double left = halignment == Qt::AlignLeft ?  point.x() : point.x() - width / 2;

    QRectF rect( left, y, width, height );

    if ( painter ) drawText( painter, rect, *titlePart, styleFont( QgsComposerLegendStyle::Title ), halignment, Qt::AlignVCenter );

    size.rwidth() = qMax( width, size.width() );

    y += height;
    if ( titlePart != lines.end() )
    {
      y += mlineSpacing;
    }
  }
  size.rheight() = y - point.y();

  return size;
}


QSizeF QgsComposerLegend::drawGroupItemTitle( QgsComposerGroupItem* groupItem, QPainter* painter, QPointF point )
{
  QSizeF size( 0, 0 );
  if ( !groupItem ) return size;

  double y = point.y();

  if ( painter ) painter->setPen( mFontColor );

  QStringList lines = splitStringForWrapping( groupItem->text() );
  for ( QStringList::Iterator groupPart = lines.begin(); groupPart != lines.end(); ++groupPart )
  {
    y += fontAscentMillimeters( styleFont( groupItem->style() ) );
    if ( painter ) drawText( painter, point.x(), y, *groupPart, styleFont( groupItem->style() ) );
    qreal width = textWidthMillimeters( styleFont( groupItem->style() ), *groupPart );
    size.rwidth() = qMax( width, size.width() );
    if ( groupPart != lines.end() )
    {
      y += mlineSpacing;
    }
  }
  size.rheight() = y - point.y();
  return size;
}

QSizeF QgsComposerLegend::drawLayerItemTitle( QgsComposerLayerItem* layerItem, QPainter* painter, QPointF point )
{
  QSizeF size( 0, 0 );
  if ( !layerItem ) return size;

  //Let the user omit the layer title item by having an empty layer title string
  if ( layerItem->text().isEmpty() ) return size;

  double y = point.y();

  if ( painter ) painter->setPen( mFontColor );

  QStringList lines = splitStringForWrapping( layerItem->text() );
  for ( QStringList::Iterator layerItemPart = lines.begin(); layerItemPart != lines.end(); ++layerItemPart )
  {
    y += fontAscentMillimeters( styleFont( layerItem->style() ) );
    if ( painter ) drawText( painter, point.x(), y, *layerItemPart , styleFont( layerItem->style() ) );
    qreal width = textWidthMillimeters( styleFont( layerItem->style() ), *layerItemPart );
    size.rwidth() = qMax( width, size.width() );
    if ( layerItemPart != lines.end() )
    {
      y += mlineSpacing;
    }
  }
  size.rheight() = y - point.y();

  return size;
}

void QgsComposerLegend::adjustBoxSize()
{
  QSizeF size = paintAndDetermineSize( 0 );
  QgsDebugMsg( QString( "width = %1 height = %2" ).arg( size.width() ).arg( size.height() ) );
  if ( size.isValid() )
  {
    setSceneRect( QRectF( transform().dx(), transform().dy(), size.width(), size.height() ) );
  }
}

QgsComposerLegend::Nucleon QgsComposerLegend::drawSymbolItem( QgsComposerLegendItem* symbolItem, QPainter* painter, QPointF point, double labelXOffset )
{
  QSizeF symbolSize( 0, 0 );
  QSizeF labelSize( 0, 0 );
  if ( !symbolItem ) return Nucleon();

  double textHeight = fontHeightCharacterMM( styleFont( QgsComposerLegendStyle::SymbolLabel ), QChar( '0' ) );
  // itemHeight here is not realy item height, it is only for symbol
  // vertical alignment purpose, i.e. ok take single line height
  // if there are more lines, thos run under the symbol
  double itemHeight = qMax( mSymbolHeight, textHeight );

  //real symbol height. Can be different from standard height in case of point symbols
  double realSymbolHeight;

  int opacity = 255;
  QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem*>( symbolItem->parent() );
  if ( layerItem )
  {
    QgsMapLayer* currentLayer = QgsMapLayerRegistry::instance()->mapLayer( layerItem->layerID() );
    if ( currentLayer )
    {
      opacity = currentLayer->getTransparency();
    }
  }

  QString text = symbolItem->text();
  if ( text.isEmpty() )
  {
    // Use layer label, used for single symbols
    text = layerItem->text();
  }

  QStringList lines = splitStringForWrapping( text );

  QgsSymbolV2* symbolNg = 0;
  QgsComposerSymbolV2Item* symbolV2Item = dynamic_cast<QgsComposerSymbolV2Item*>( symbolItem );
  if ( symbolV2Item )
  {
    symbolNg = symbolV2Item->symbolV2();
  }
  QgsComposerRasterSymbolItem* rasterItem = dynamic_cast<QgsComposerRasterSymbolItem*>( symbolItem );

  double x = point.x();
  if ( symbolNg ) //item with symbol NG?
  {
    // must be called also with painter=0 to get real size
    drawSymbolV2( painter, symbolNg, point.y() + ( itemHeight - mSymbolHeight ) / 2, x, realSymbolHeight, opacity );
    symbolSize.rwidth() = qMax( x - point.x(), mSymbolWidth );
    symbolSize.rheight() = qMax( realSymbolHeight, mSymbolHeight );
  }
  else if ( rasterItem )
  {
    if ( painter )
    {
      painter->setBrush( rasterItem->color() );
      painter->drawRect( QRectF( point.x(), point.y() + ( itemHeight - mSymbolHeight ) / 2, mSymbolWidth, mSymbolHeight ) );
    }
    symbolSize.rwidth() = mSymbolWidth;
    symbolSize.rheight() = mSymbolHeight;
  }
  else //item with icon?
  {
    QIcon symbolIcon = symbolItem->icon();
    if ( !symbolIcon.isNull() )
    {
      if ( painter ) symbolIcon.paint( painter, point.x(), point.y() + ( itemHeight - mSymbolHeight ) / 2, mSymbolWidth, mSymbolHeight );
      symbolSize.rwidth() = mSymbolWidth;
      symbolSize.rheight() = mSymbolHeight;
    }
  }

  if ( painter ) painter->setPen( mFontColor );

  //double labelX = point.x() + labelXOffset; // + mIconLabelSpace;
  double labelX = point.x() + qMax( symbolSize.width(), labelXOffset );

  // Vertical alignment of label with symbol:
  // a) label height < symbol heigh: label centerd with symbol
  // b) label height > symbol height: label starts at top and runs under symbol

  labelSize.rheight() = lines.count() * textHeight + ( lines.count() - 1 ) * mlineSpacing;

  double labelY;
  if ( labelSize.height() < symbolSize.height() )
  {
    labelY = point.y() +  symbolSize.height() / 2 + textHeight / 2;
  }
  else
  {
    labelY = point.y() + textHeight;
  }

  for ( QStringList::Iterator itemPart = lines.begin(); itemPart != lines.end(); ++itemPart )
  {
    if ( painter ) drawText( painter, labelX, labelY, *itemPart , styleFont( QgsComposerLegendStyle::SymbolLabel ) );
    labelSize.rwidth() = qMax( textWidthMillimeters( styleFont( QgsComposerLegendStyle::SymbolLabel ),  *itemPart ), double( labelSize.width() ) );
    if ( itemPart != lines.end() )
    {
      labelY += mlineSpacing + textHeight;
    }
  }

  Nucleon nucleon;
  nucleon.item = symbolItem;
  nucleon.symbolSize = symbolSize;
  nucleon.labelSize = labelSize;
  //QgsDebugMsg( QString( "symbol height = %1 label height = %2").arg( symbolSize.height()).arg( labelSize.height() ));
  double width = qMax( symbolSize.width(), labelXOffset ) + labelSize.width();
  double height = qMax( symbolSize.height(), labelSize.height() );
  nucleon.size = QSizeF( width, height );
  return nucleon;
}


void QgsComposerLegend::drawSymbolV2( QPainter* p, QgsSymbolV2* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int layerOpacity ) const
{
  Q_UNUSED( layerOpacity );
  if ( !s )
  {
    return;
  }

  double rasterScaleFactor = 1.0;
  if ( p )
  {
    QPaintDevice* paintDevice = p->device();
    if ( !paintDevice )
    {
      return;
    }
    rasterScaleFactor = ( paintDevice->logicalDpiX() + paintDevice->logicalDpiY() ) / 2.0 / 25.4;
  }

  //consider relation to composer map for symbol sizes in mm
  bool sizeInMapUnits = s->outputUnit() == QgsSymbolV2::MapUnit;
  double mmPerMapUnit = 1;
  if ( mComposerMap )
  {
    mmPerMapUnit = mComposerMap->mapUnitsToMM();
  }
  QgsMarkerSymbolV2* markerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( s );

  //Consider symbol size for point markers
  double height = mSymbolHeight;
  double width = mSymbolWidth;
  double size = 0;
  //Center small marker symbols
  double widthOffset = 0;
  double heightOffset = 0;

  if ( markerSymbol )
  {
    size = markerSymbol->size();
    height = size;
    width = size;
    if ( mComposerMap && sizeInMapUnits )
    {
      height *= mmPerMapUnit;
      width *= mmPerMapUnit;
      markerSymbol->setSize( width );
    }
    if ( width < mSymbolWidth )
    {
      widthOffset = ( mSymbolWidth - width ) / 2.0;
    }
    if ( height < mSymbolHeight )
    {
      heightOffset = ( mSymbolHeight - height ) / 2.0;
    }
  }

  if ( p )
  {
    p->save();
    p->translate( currentXPosition + widthOffset, currentYCoord + heightOffset );
    p->scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor );

    if ( markerSymbol && sizeInMapUnits )
    {
      s->setOutputUnit( QgsSymbolV2::MM );
    }

    s->drawPreviewIcon( p, QSize( width * rasterScaleFactor, height * rasterScaleFactor ) );

    if ( markerSymbol && sizeInMapUnits )
    {
      s->setOutputUnit( QgsSymbolV2::MapUnit );
      markerSymbol->setSize( size );
    }
    p->restore();
  }
  currentXPosition += width;
  currentXPosition += 2 * widthOffset;
  symbolHeight = height + 2 * heightOffset;
}


QStringList QgsComposerLegend::layerIdList() const
{
  //take layer list from map renderer (to have legend order)
  if ( mComposition )
  {
    QgsMapRenderer* r = mComposition->mapRenderer();
    if ( r )
    {
      return r->layerSet();
    }
  }
  return QStringList();
}

void QgsComposerLegend::synchronizeWithModel()
{
  QgsDebugMsg( "Entered" );
  adjustBoxSize();
  update();
}

void QgsComposerLegend::setStyleFont( QgsComposerLegendStyle::Style s, const QFont& f )
{
  rstyle( s ).setFont( f );
}

void QgsComposerLegend::setStyleMargin( QgsComposerLegendStyle::Style s, double margin )
{
  rstyle( s ).setMargin( margin );
}

void QgsComposerLegend::setStyleMargin( QgsComposerLegendStyle::Style s, QgsComposerLegendStyle::Side side, double margin )
{
  rstyle( s ).setMargin( side, margin );
}

void QgsComposerLegend::updateLegend()
{
  mLegendModel.setLayerSet( layerIdList() );
  adjustBoxSize();
  update();
}

bool QgsComposerLegend::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement composerLegendElem = doc.createElement( "ComposerLegend" );
  elem.appendChild( composerLegendElem );

  //write general properties
  composerLegendElem.setAttribute( "title", mTitle );
  composerLegendElem.setAttribute( "columnCount", QString::number( mColumnCount ) );
  composerLegendElem.setAttribute( "splitLayer", QString::number( mSplitLayer ) );
  composerLegendElem.setAttribute( "equalColumnWidth", QString::number( mEqualColumnWidth ) );

  composerLegendElem.setAttribute( "boxSpace", QString::number( mBoxSpace ) );
  composerLegendElem.setAttribute( "columnSpace", QString::number( mColumnSpace ) );

  composerLegendElem.setAttribute( "symbolWidth", QString::number( mSymbolWidth ) );
  composerLegendElem.setAttribute( "symbolHeight", QString::number( mSymbolHeight ) );
  composerLegendElem.setAttribute( "wrapChar", mWrapChar );
  composerLegendElem.setAttribute( "fontColor", mFontColor.name() );

  if ( mComposerMap )
  {
    composerLegendElem.setAttribute( "map", mComposerMap->id() );
  }

  QDomElement composerLegendStyles = doc.createElement( "styles" );
  composerLegendElem.appendChild( composerLegendStyles );

  style( QgsComposerLegendStyle::Title ).writeXML( "title", composerLegendStyles, doc );
  style( QgsComposerLegendStyle::Group ).writeXML( "group", composerLegendStyles, doc );
  style( QgsComposerLegendStyle::Subgroup ).writeXML( "subgroup", composerLegendStyles, doc );
  style( QgsComposerLegendStyle::Symbol ).writeXML( "symbol", composerLegendStyles, doc );
  style( QgsComposerLegendStyle::SymbolLabel ).writeXML( "symbolLabel", composerLegendStyles, doc );

  //write model properties
  mLegendModel.writeXML( composerLegendElem, doc );

  return _writeXML( composerLegendElem, doc );
}

bool QgsComposerLegend::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  //read general properties
  mTitle = itemElem.attribute( "title" );
  mColumnCount = itemElem.attribute( "columnCount", "1" ).toInt();
  if ( mColumnCount < 1 ) mColumnCount = 1;
  mSplitLayer = itemElem.attribute( "splitLayer", "0" ).toInt() == 1;
  mEqualColumnWidth = itemElem.attribute( "equalColumnWidth", "0" ).toInt() == 1;

  QDomNodeList stylesNodeList = itemElem.elementsByTagName( "styles" );
  if ( stylesNodeList.size() > 0 )
  {
    QDomNode stylesNode = stylesNodeList.at( 0 );
    for ( int i = 0; i < stylesNode.childNodes().size(); i++ )
    {
      QDomElement styleElem = stylesNode.childNodes().at( i ).toElement();
      QgsComposerLegendStyle style;
      style.readXML( styleElem, doc );
      QString name = styleElem.attribute( "name" );
      QgsComposerLegendStyle::Style s;
      if ( name == "title" ) s = QgsComposerLegendStyle::Title;
      else if ( name == "group" ) s = QgsComposerLegendStyle::Group;
      else if ( name == "subgroup" ) s = QgsComposerLegendStyle::Subgroup;
      else if ( name == "symbol" ) s = QgsComposerLegendStyle::Symbol;
      else if ( name == "symbolLabel" ) s = QgsComposerLegendStyle::SymbolLabel;
      else continue;
      setStyle( s, style );
    }
  }

  //font color
  mFontColor.setNamedColor( itemElem.attribute( "fontColor", "#000000" ) );

  //spaces
  mBoxSpace = itemElem.attribute( "boxSpace", "2.0" ).toDouble();
  mColumnSpace = itemElem.attribute( "columnSpace", "2.0" ).toDouble();

  mSymbolWidth = itemElem.attribute( "symbolWidth", "7.0" ).toDouble();
  mSymbolHeight = itemElem.attribute( "symbolHeight", "14.0" ).toDouble();

  mWrapChar = itemElem.attribute( "wrapChar" );

  //composer map
  if ( !itemElem.attribute( "map" ).isEmpty() )
  {
    mComposerMap = mComposition->getComposerMapById( itemElem.attribute( "map" ).toInt() );
  }

  //read model properties
  QDomNodeList modelNodeList = itemElem.elementsByTagName( "Model" );
  if ( modelNodeList.size() > 0 )
  {
    QDomElement modelElem = modelNodeList.at( 0 ).toElement();
    mLegendModel.readXML( modelElem, doc );
  }

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }

  // < 2.0 projects backward compatibility >>>>>
  //title font
  QString titleFontString = itemElem.attribute( "titleFont" );
  if ( !titleFontString.isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Title ).rfont().fromString( titleFontString );
  }
  //group font
  QString groupFontString = itemElem.attribute( "groupFont" );
  if ( !groupFontString.isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Group ).rfont().fromString( groupFontString );
  }

  //layer font
  QString layerFontString = itemElem.attribute( "layerFont" );
  if ( !layerFontString.isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Subgroup ).rfont().fromString( layerFontString );
  }
  //item font
  QString itemFontString = itemElem.attribute( "itemFont" );
  if ( !itemFontString.isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::SymbolLabel ).rfont().fromString( itemFontString );
  }

  if ( !itemElem.attribute( "groupSpace" ).isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Group ).setMargin( QgsComposerLegendStyle::Top, itemElem.attribute( "groupSpace", "3.0" ).toDouble() );
  }
  if ( !itemElem.attribute( "layerSpace" ).isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Subgroup ).setMargin( QgsComposerLegendStyle::Top, itemElem.attribute( "layerSpace", "3.0" ).toDouble() );
  }
  if ( !itemElem.attribute( "symbolSpace" ).isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Symbol ).setMargin( QgsComposerLegendStyle::Top, itemElem.attribute( "symbolSpace", "2.0" ).toDouble() );
    rstyle( QgsComposerLegendStyle::SymbolLabel ).setMargin( QgsComposerLegendStyle::Top, itemElem.attribute( "symbolSpace", "2.0" ).toDouble() );
  }
  // <<<<<<< < 2.0 projects backward compatibility

  emit itemChanged();
  return true;
}

void QgsComposerLegend::setComposerMap( const QgsComposerMap* map )
{
  mComposerMap = map;
  if ( map )
  {
    QObject::connect( map, SIGNAL( destroyed( QObject* ) ), this, SLOT( invalidateCurrentMap() ) );
  }
}

void QgsComposerLegend::invalidateCurrentMap()
{
  if ( mComposerMap )
  {
    disconnect( mComposerMap, SIGNAL( destroyed( QObject* ) ), this, SLOT( invalidateCurrentMap() ) );
  }
  mComposerMap = 0;
}

QStringList QgsComposerLegend::splitStringForWrapping( QString stringToSplt )
{
  QStringList list;
  // If the string contains nothing then just return the string without spliting.
  if ( mWrapChar.count() == 0 )
    list << stringToSplt;
  else
    list = stringToSplt.split( mWrapChar );
  return list;
}

QList<QgsComposerLegend::Atom> QgsComposerLegend::createAtomList( QStandardItem* rootItem, bool splitLayer )
{
  QList<Atom> atoms;

  if ( !rootItem ) return atoms;

  Atom atom;

  for ( int i = 0; i < rootItem->rowCount(); i++ )
  {
    QStandardItem* currentLayerItem = rootItem->child( i );
    QgsComposerLegendItem* currentLegendItem = dynamic_cast<QgsComposerLegendItem*>( currentLayerItem );
    if ( !currentLegendItem ) continue;

    QgsComposerLegendItem::ItemType type = currentLegendItem->itemType();
    if ( type == QgsComposerLegendItem::GroupItem )
    {
      // Group subitems
      QList<Atom> groupAtoms = createAtomList( currentLayerItem, splitLayer );

      Nucleon nucleon;
      nucleon.item = currentLegendItem;
      nucleon.size = drawGroupItemTitle( dynamic_cast<QgsComposerGroupItem*>( currentLegendItem ) );

      if ( groupAtoms.size() > 0 )
      {
        // Add internal space between this group title and the next nucleon
        groupAtoms[0].size.rheight() += spaceAboveAtom( groupAtoms[0] );
        // Prepend this group title to the first atom
        groupAtoms[0].nucleons.prepend( nucleon );
        groupAtoms[0].size.rheight() += nucleon.size.height();
        groupAtoms[0].size.rwidth() = qMax( nucleon.size.width(), groupAtoms[0].size.width() );
      }
      else
      {
        // no subitems, append new atom
        Atom atom;
        atom.nucleons.append( nucleon );
        atom.size.rwidth() += nucleon.size.width();
        atom.size.rheight() += nucleon.size.height();
        atom.size.rwidth() = qMax( nucleon.size.width(), atom.size.width() );
        groupAtoms.append( atom );
      }
      atoms.append( groupAtoms );
    }
    else if ( type == QgsComposerLegendItem::LayerItem )
    {
      Atom atom;

      if ( currentLegendItem->style() != QgsComposerLegendStyle::Hidden )
      {
        Nucleon nucleon;
        nucleon.item = currentLegendItem;
        nucleon.size = drawLayerItemTitle( dynamic_cast<QgsComposerLayerItem*>( currentLegendItem ) );
        atom.nucleons.append( nucleon );
        atom.size.rwidth() = nucleon.size.width();
        atom.size.rheight() = nucleon.size.height();
      }

      QList<Atom> layerAtoms;

      for ( int j = 0; j < currentLegendItem->rowCount(); j++ )
      {
        QgsComposerLegendItem * symbolItem = dynamic_cast<QgsComposerLegendItem*>( currentLegendItem->child( j, 0 ) );
        if ( !symbolItem ) continue;

        Nucleon symbolNucleon = drawSymbolItem( symbolItem );

        if ( !mSplitLayer || j == 0 )
        {
          // append to layer atom
          // the width is not correct at this moment, we must align all symbol labels
          atom.size.rwidth() = qMax( symbolNucleon.size.width(), atom.size.width() );
          //if ( currentLegendItem->rowCount() > 1 )
          //if ( currentLegendItem->style() != QgsComposerLegendStyle::Hidden )
          //{
          //atom.size.rheight() += mSymbolSpace;
          // TODO: for now we keep Symbol and SymbolLabel Top margin in sync
          atom.size.rheight() += style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Top );
          //}
          atom.size.rheight() += symbolNucleon.size.height();
          atom.nucleons.append( symbolNucleon );
        }
        else
        {
          Atom symbolAtom;
          symbolAtom.nucleons.append( symbolNucleon );
          symbolAtom.size.rwidth() = symbolNucleon.size.width();
          symbolAtom.size.rheight() = symbolNucleon.size.height();
          layerAtoms.append( symbolAtom );
        }
      }
      layerAtoms.prepend( atom );
      atoms.append( layerAtoms );
    }
  }

  return atoms;
}

// Draw atom and expand its size (using actual nucleons labelXOffset)
QSizeF QgsComposerLegend::drawAtom( Atom atom, QPainter* painter, QPointF point )
{
  // bool first = true;
  QSizeF size = QSizeF( atom.size );
  foreach ( Nucleon nucleon, atom.nucleons )
  {
    QgsComposerLegendItem* item = nucleon.item;
    //QgsDebugMsg( "text: " + item->text() );
    if ( !item ) continue;
    QgsComposerLegendItem::ItemType type = item->itemType();
    if ( type == QgsComposerLegendItem::GroupItem )
    {
      QgsComposerGroupItem* groupItem = dynamic_cast<QgsComposerGroupItem*>( item );
      if ( !groupItem ) continue;
      // TODO: is it better to avoid marginand align all types of items to the same top like it was before?
      //if ( !first ) point.ry() += style(groupItem->style()).margin(QgsComposerLegendStyle::Top);
      if ( groupItem->style() != QgsComposerLegendStyle::Hidden )
      {
        point.ry() += style( groupItem->style() ).margin( QgsComposerLegendStyle::Top );
        drawGroupItemTitle( groupItem, painter, point );
      }
    }
    else if ( type == QgsComposerLegendItem::LayerItem )
    {
      QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem*>( item );
      if ( !layerItem ) continue;
      //if ( !first ) point.ry() += style(layerItem->style()).margin(QgsComposerLegendStyle::Top);
      if ( layerItem->style() != QgsComposerLegendStyle::Hidden )
      {
        point.ry() += style( layerItem->style() ).margin( QgsComposerLegendStyle::Top );
        drawLayerItemTitle( layerItem, painter, point );
      }
    }
    else if ( type == QgsComposerLegendItem::SymbologyV2Item ||
              type == QgsComposerLegendItem::RasterSymbolItem )
    {
      //if ( !first )
      point.ry() += style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Top );
      //}
      double labelXOffset = nucleon.labelXOffset;
      Nucleon symbolNucleon = drawSymbolItem( item, painter, point, labelXOffset );
      // expand width, it may be wider because of labelXOffset
      size.rwidth() = qMax( symbolNucleon.size.width(), size.width() );
    }
    point.ry() += nucleon.size.height();
    // first = false;
  }
  return size;
}

double QgsComposerLegend::spaceAboveAtom( Atom atom )
{
  if ( atom.nucleons.size() == 0 ) return 0;

  Nucleon nucleon = atom.nucleons.first();

  QgsComposerLegendItem* item = nucleon.item;
  if ( !item ) return 0;

  QgsComposerLegendItem::ItemType type = item->itemType();
  switch ( type )
  {
    case QgsComposerLegendItem::GroupItem:
      return style( item->style() ).margin( QgsComposerLegendStyle::Top );
      break;
    case QgsComposerLegendItem::LayerItem:
      return style( item->style() ).margin( QgsComposerLegendStyle::Top );
      break;
    case QgsComposerLegendItem::SymbologyV2Item:
    case QgsComposerLegendItem::RasterSymbolItem:
      // TODO: use Symbol or SymbolLabel Top margin
      return style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Top );
      break;
    default:
      break;
  }
  return 0;
}

void QgsComposerLegend::setColumns( QList<Atom>& atomList )
{
  if ( mColumnCount == 0 ) return;

  // Divide atoms to columns
  double totalHeight = 0;
  // bool first = true;
  qreal maxAtomHeight = 0;
  foreach ( Atom atom, atomList )
  {
    //if ( !first )
    //{
    totalHeight += spaceAboveAtom( atom );
    //}
    totalHeight += atom.size.height();
    maxAtomHeight = qMax( atom.size.height(), maxAtomHeight );
    // first  = false;
  }

  // We know height of each atom and we have to split them into columns
  // minimizing max column height. It is sort of bin packing problem, NP-hard.
  // We are using simple heuristic, brute fore appeared to be to slow,
  // the number of combinations is N = n!/(k!*(n-k)!) where n = atomsCount-1
  // and k = columnsCount-1

  double avgColumnHeight = totalHeight / mColumnCount;
  int currentColumn = 0;
  int currentColumnAtomCount = 0; // number of atoms in current column
  double currentColumnHeight = 0;
  double maxColumnHeight = 0;
  double closedColumnsHeight = 0;
  // first = true; // first in column
  for ( int i = 0; i < atomList.size(); i++ )
  {
    Atom atom = atomList[i];
    double currentHeight = currentColumnHeight;
    //if ( !first )
    //{
    currentHeight += spaceAboveAtom( atom );
    //}
    currentHeight += atom.size.height();

    // Recalc average height for remaining columns including current
    avgColumnHeight = ( totalHeight - closedColumnsHeight ) / ( mColumnCount - currentColumn );
    if (( currentHeight - avgColumnHeight ) > atom.size.height() / 2 // center of current atom is over average height
        && currentColumnAtomCount > 0 // do not leave empty column
        && currentHeight > maxAtomHeight  // no sense to make smaller columns than max atom height
        && currentHeight > maxColumnHeight  // no sense to make smaller columns than max column already created
        && currentColumn < mColumnCount - 1 ) // must not exceed max number of columns
    {
      // New column
      currentColumn++;
      currentColumnAtomCount = 0;
      closedColumnsHeight += currentColumnHeight;
      currentColumnHeight = atom.size.height();
    }
    else
    {
      currentColumnHeight = currentHeight;
    }
    atomList[i].column = currentColumn;
    currentColumnAtomCount++;
    maxColumnHeight = qMax( currentColumnHeight, maxColumnHeight );

    // first  = false;
  }

  // Alling labels of symbols for each layr/column to the same labelXOffset
  QMap<QString, qreal> maxSymbolWidth;
  for ( int i = 0; i < atomList.size(); i++ )
  {
    for ( int j = 0; j < atomList[i].nucleons.size(); j++ )
    {
      QgsComposerLegendItem* item = atomList[i].nucleons[j].item;
      if ( !item ) continue;
      QgsComposerLegendItem::ItemType type = item->itemType();
      if ( type == QgsComposerLegendItem::SymbologyV2Item ||
           type == QgsComposerLegendItem::RasterSymbolItem )
      {
        QString key = QString( "%1-%2" ).arg(( qulonglong )item->parent() ).arg( atomList[i].column );
        maxSymbolWidth[key] = qMax( atomList[i].nucleons[j].symbolSize.width(), maxSymbolWidth[key] );
      }
    }
  }
  for ( int i = 0; i < atomList.size(); i++ )
  {
    for ( int j = 0; j < atomList[i].nucleons.size(); j++ )
    {
      QgsComposerLegendItem* item = atomList[i].nucleons[j].item;
      if ( !item ) continue;
      QgsComposerLegendItem::ItemType type = item->itemType();
      if ( type == QgsComposerLegendItem::SymbologyV2Item ||
           type == QgsComposerLegendItem::RasterSymbolItem )
      {
        QString key = QString( "%1-%2" ).arg(( qulonglong )item->parent() ).arg( atomList[i].column );
        double space = style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Right ) +
                       style( QgsComposerLegendStyle::SymbolLabel ).margin( QgsComposerLegendStyle::Left );
        atomList[i].nucleons[j].labelXOffset =  maxSymbolWidth[key] + space;
        atomList[i].nucleons[j].size.rwidth() =  maxSymbolWidth[key] + space + atomList[i].nucleons[j].labelSize.width();
      }
    }
  }
}

