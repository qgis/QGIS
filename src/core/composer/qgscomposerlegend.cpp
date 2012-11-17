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
#include "qgscomposerlegenditem.h"
#include "qgscomposermap.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsrenderer.h" //for brush scaling
#include "qgssymbol.h"
#include "qgssymbolv2.h"
#include <QDomDocument>
#include <QDomElement>
#include <QPainter>

void QgsComposerLegend::Position::expandWidth( double w )
{
  if ( widths.size() <= column )
  {
    widths.resize( column + 1 );
  }
  if ( w > widths[column] )
  {
    widths[column] = w;
  }
}

void QgsComposerLegend::Position::setColumn( QStandardItem *item )
{
  int c = columns.value( item ); // 0 if columns are not given
  if ( c > column )
  {
    double w = boxSpace;
    for ( int i = 0; i < c; i++ )
    {
      w += widths.value( i ) + columnSpace;
    }
    point.rx() = w;
    point.ry() = columnTop;
    column = c;
  }
}

QgsComposerLegend::QgsComposerLegend( QgsComposition* composition )
    : QgsComposerItem( composition )
    , mTitle( tr( "Legend" ) )
    , mBoxSpace( 2 )
    , mGroupSpace( 2 )
    , mLayerSpace( 2 )
    , mSymbolSpace( 2 )
    , mIconLabelSpace( 2 )
    , mColumnCount( 1 )
    , mComposerMap( 0 )
{
  //QStringList idList = layerIdList();
  //mLegendModel.setLayerSet( idList );

  mTitleFont.setPointSizeF( 16.0 );
  mGroupFont.setPointSizeF( 14.0 );
  mLayerFont.setPointSizeF( 12.0 );
  mItemFont.setPointSizeF( 12.0 );

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
  if ( mColumnCount > 1 && mColumns.size() == 0 )
  {
    LegendSize legendSize = paintAndDetermineSize( 0, mColumns );
    QList<Size> sizes = legendSize.sizes;

    // We know height of each layer and we have to split them into columns
    // minimizing total height. It is sort of bin packing problem, NP-hard.
    // We use brute force because we hope that the number of layers should
    // not be too big.

    QgsDebugMsg( QString( "sizes %1 columns %2" ).arg( sizes.size() ).arg( mColumnCount ) );
    if ( sizes.size() > 0 )
    {
      // Breaks between columns, index 0 means break after layer 0.
      // Breaks must not be after groups to avoid group leaving group title
      // at the bootom of column.
      QVector<bool> breaks( sizes.size() - 1 );

      // init breaks
      int idx = 0;
      for ( int i = 0; i < mColumnCount - 1; i++ )
      {
        // break must not be after group, if there is a group at the end however
        // (nonsense), it is left at bottom (obviously)
        while ( idx < sizes.size() - 1 && sizes.value( idx ).type == QgsComposerLegendItem::GroupItem ) idx++;
        breaks[idx] = true;
        idx++;
        if ( idx >= breaks.size() ) break;
      }

      // optimize by height
      double bestHeight = std::numeric_limits<double>::max();
      QVector<bool> bestBreaks;
      while ( true )
      {

        // Calculate max column height
        double maxHeight = 0;
        double height = 0;
        for ( int i = 0; i <  sizes.size(); i++ )
        {
          height += sizes[i].size.height();
          maxHeight = qMax( height, maxHeight );
          if ( i < breaks.size() && breaks.value( i ) ) // next col
          {
            height = 0;
          }
        }
        if ( maxHeight < bestHeight )
        {
          bestBreaks = breaks;
          bestHeight = maxHeight;
        }
        QString msg;
        foreach ( bool b, breaks )
        {
          msg += QString( "%1" ).arg( b );
        }
        QgsDebugMsgLevel( QString( "Breaks: %1 height %2" ).arg( msg ).arg( maxHeight ), 2 );

        // move first break from right side which has space on its right side to the right
        bool next = false;
        for ( int i = breaks.size() - 1; i > 0; i-- )
        {
          if ( !breaks[i] ) // we have space
          {
            // find first layer on the left from free space
            for ( int j = i - 1; j >= 0; j-- )
            {
              if ( breaks[j] && sizes[j].type == QgsComposerLegendItem::LayerItem )
              {
                breaks[j] = false;
                breaks[i] = true;
                // shift all not the right side down
                int count = breaks.mid( j + 1 ).count( true ); // breaks to end
                for ( int k = j + 1; k < breaks.size(); k++ )
                {
                  if ( count > 0 && sizes[k].type == QgsComposerLegendItem::LayerItem )
                  {
                    breaks[k] = true; // set shifted break
                    count--;
                  }
                  else
                  {
                    breaks[k] = false; // clear shifted
                  }
                }
                next = true;
                break;
              }
            }
            if ( next ) break;
          }
        }
        if ( !next ) break; // no more combinations
      }

      // set columns using breaks
      int column = 0;
      for ( int i = 0; i < sizes.count(); i++ )
      {
        QgsDebugMsgLevel( QString( "layer %1 column %2" ).arg( i ).arg( column ), 2 );
        mColumns.insert( sizes.at( i ).item, column );
        // if there is break after this layer (the same index), increase column
        if ( i < bestBreaks.size() && bestBreaks.value( i ) )
        {
          column++;
        }
      }
    }
  }

  LegendSize legendSize = paintAndDetermineSize( painter, mColumns );
  return legendSize.size;
}

QgsComposerLegend::LegendSize QgsComposerLegend::paintAndDetermineSize( QPainter* painter, QMap<QStandardItem *, int> columns )
{
  QSizeF size;
  QList<Size> layerSizes;

  //go through model...
  QStandardItem* rootItem = mLegendModel.invisibleRootItem();
  if ( !rootItem )
  {
    return LegendSize();
  }

  if ( painter )
  {
    painter->save();
    drawBackground( painter );
    painter->setPen( QPen( QColor( 0, 0, 0 ) ) );
  }

  int numLayerItems = rootItem->rowCount();
  QStandardItem* currentLayerItem = 0;

  //draw title
  Position currentPosition;
  currentPosition.column = 0;
  currentPosition.columns = columns;
  currentPosition.maxColumnHeight = 0;
  currentPosition.boxSpace = mBoxSpace;
  currentPosition.columnSpace = mBoxSpace;
  currentPosition.columnTop = mBoxSpace;

  // Calculate title size (all columns start under the title)
  if ( !mTitle.isEmpty() )
  {
    QStringList lines = splitStringForWrapping( mTitle );
    for ( QStringList::Iterator titlePart = lines.begin(); titlePart != lines.end(); ++titlePart )
    {
      currentPosition.titleSize.rheight() += fontAscentMillimeters( mTitleFont );
      double currentItemMaxX = textWidthMillimeters( mTitleFont, *titlePart );
      currentPosition.titleSize.rwidth() = qMax( currentPosition.titleSize.width(), currentItemMaxX );
      if ( titlePart != lines.end() )
      {
        currentPosition.titleSize.rheight() += mlineSpacing;
      }
    }
    currentPosition.columnTop += currentPosition.titleSize.height();
  }

  // set top left point where columns start
  currentPosition.point = QPointF( mBoxSpace, currentPosition.columnTop );

  for ( int i = 0; i < numLayerItems; ++i )
  {
    currentLayerItem = rootItem->child( i );
    QgsComposerLegendItem* currentLegendItem = dynamic_cast<QgsComposerLegendItem*>( currentLayerItem );
    if ( currentLegendItem )
    {
      QgsComposerLegendItem::ItemType type = currentLegendItem->itemType();
      if ( type == QgsComposerLegendItem::GroupItem )
      {
        QList<Size> groupSizes = drawGroupItem( painter, dynamic_cast<QgsComposerGroupItem*>( currentLegendItem ), currentPosition );
        layerSizes.append( groupSizes );
      }
      else if ( type == QgsComposerLegendItem::LayerItem )
      {
        Size layerSize = drawLayerItem( painter, dynamic_cast<QgsComposerLayerItem*>( currentLegendItem ), currentPosition );
        layerSizes.append( layerSize );
      }
    }
  }

  double totalWidth = 0;
  foreach ( double w, currentPosition.widths ) totalWidth += mBoxSpace + w ;
  totalWidth += mBoxSpace; // after the last column

  // Now we know total width and can draw the title centered
  if ( !mTitle.isEmpty() )
  {
    double currentYCoord = mBoxSpace;

    // For multicolumn center if we stay in totalWidth, otherwise allign to left
    // and expand total width. With single column keep alligned to left be cause
    // it looks better alligned with items bellow instead of centered
    Qt::AlignmentFlag halignement;
    if ( mColumnCount > 1 && currentPosition.titleSize.width() + 2 * mBoxSpace < totalWidth )
    {
      halignement = Qt::AlignHCenter;
    }
    else
    {
      halignement = Qt::AlignLeft;
      totalWidth = qMax( currentPosition.titleSize.width() + 2 * mBoxSpace, totalWidth );
    }

    if ( painter ) painter->setPen( QColor( 0, 0, 0 ) );

    QStringList lines = splitStringForWrapping( mTitle );
    for ( QStringList::Iterator titlePart = lines.begin(); titlePart != lines.end(); ++titlePart )
    {
      QRectF rect( mBoxSpace, currentYCoord, totalWidth - mBoxSpace, fontAscentMillimeters( mTitleFont ) );
      if ( painter ) drawText( painter, rect, *titlePart, mTitleFont, halignement, Qt::AlignVCenter );
      currentYCoord += fontAscentMillimeters( mTitleFont );
      if ( titlePart != lines.end() )
        currentYCoord += mlineSpacing;
    }
  }

  double totalHeight = 2 * mBoxSpace + currentPosition.titleSize.height() + currentPosition.maxColumnHeight;
  size.setHeight( totalHeight );
  size.setWidth( totalWidth );

  //adjust box if width or height is to small
  if ( painter && totalHeight > rect().height() )
  {
    setSceneRect( QRectF( transform().dx(), transform().dy(), rect().width(), totalHeight ) );
  }
  if ( painter && totalWidth > rect().width() )
  {
    setSceneRect( QRectF( transform().dx(), transform().dy(), totalWidth, rect().height() ) );
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


  return LegendSize( size, layerSizes );
}

QList<QgsComposerLegend::Size> QgsComposerLegend::drawGroupItem( QPainter* p, QgsComposerGroupItem* groupItem, Position& currentPosition )
{
  QList<Size> sizes;
  double startYCoord = currentPosition.point.y();
  if ( !groupItem )
  {
    return sizes;
  }
  currentPosition.setColumn( groupItem );

  currentPosition.point.ry() += mGroupSpace;

  if ( p ) p->setPen( QColor( 0, 0, 0 ) );

  QStringList lines = splitStringForWrapping( groupItem->text() );
  for ( QStringList::Iterator groupPart = lines.begin(); groupPart != lines.end(); ++groupPart )
  {
    currentPosition.point.ry() += fontAscentMillimeters( mGroupFont );
    if ( p ) drawText( p, currentPosition.point.x(), currentPosition.point.y(), *groupPart, mGroupFont );
    double currentMaxXCoord = textWidthMillimeters( mGroupFont, *groupPart );
    currentPosition.expandWidth( currentMaxXCoord );
    if ( groupPart != lines.end() )
      currentPosition.point.ry() += mlineSpacing;
  }
  // append this group size
  sizes.append( Size( QSizeF( 0, currentPosition.point.y() - startYCoord ), QgsComposerLegendItem::GroupItem, groupItem ) );

  //children can be other group items or layer items
  int numChildItems = groupItem->rowCount();
  QStandardItem* currentChildItem = 0;

  for ( int i = 0; i < numChildItems; ++i )
  {
    currentChildItem = groupItem->child( i );
    QgsComposerLegendItem* currentLegendItem = dynamic_cast<QgsComposerLegendItem*>( currentChildItem );
    QgsComposerLegendItem::ItemType type = currentLegendItem->itemType();
    if ( type == QgsComposerLegendItem::GroupItem )
    {
      QList<Size> subSizes = drawGroupItem( p, dynamic_cast<QgsComposerGroupItem*>( currentLegendItem ), currentPosition );
      sizes.append( subSizes );
    }
    else if ( type == QgsComposerLegendItem::LayerItem )
    {
      Size subSize = drawLayerItem( p, dynamic_cast<QgsComposerLayerItem*>( currentLegendItem ), currentPosition );
      sizes.append( subSize );
    }
  }
  currentPosition.maxColumnHeight = qMax( currentPosition.maxColumnHeight,  currentPosition.point.y() - currentPosition.columnTop );
  return sizes;
}

QgsComposerLegend::Size QgsComposerLegend::drawLayerItem( QPainter* p, QgsComposerLayerItem* layerItem, Position& currentPosition )
{
  double startYCoord = currentPosition.point.y();
  if ( !layerItem )
  {
    return Size();
  }
  currentPosition.setColumn( layerItem );

  int opacity = 255;
  QgsMapLayer* currentLayer = QgsMapLayerRegistry::instance()->mapLayer( layerItem->layerID() );
  if ( currentLayer )
  {
    opacity = currentLayer->getTransparency();
  }

  //Let the user omit the layer title item by having an empty layer title string
  if ( !layerItem->text().isEmpty() )
  {
    currentPosition.point.ry() += mLayerSpace;

    //draw layer Item
    if ( p ) p->setPen( QColor( 0, 0, 0 ) );

    QStringList lines = splitStringForWrapping( layerItem->text() );
    for ( QStringList::Iterator layerItemPart = lines.begin(); layerItemPart != lines.end(); ++layerItemPart )
    {
      currentPosition.point.ry() += fontAscentMillimeters( mLayerFont );
      if ( p ) drawText( p, currentPosition.point.x(), currentPosition.point.y(), *layerItemPart , mLayerFont );
      double maxXCoord = textWidthMillimeters( mLayerFont, *layerItemPart );
      currentPosition.expandWidth( maxXCoord );
      if ( layerItemPart != lines.end() )
        currentPosition.point.ry() += mlineSpacing ;
    }
  }
  else //layer title omited
  {
    //symbol space will be added before the item later
    currentPosition.point.ry() += ( mLayerSpace - mSymbolSpace );
  }

  //and child items
  drawLayerChildItems( p, layerItem, currentPosition, opacity );

  currentPosition.maxColumnHeight = qMax( currentPosition.maxColumnHeight,  currentPosition.point.y() - currentPosition.columnTop );
  return Size( QSizeF( 0, currentPosition.point.y() - startYCoord ),  QgsComposerLegendItem::LayerItem, layerItem );
}

void QgsComposerLegend::adjustBoxSize()
{
  mColumns.clear();
  QSizeF size = paintAndDetermineSize( 0 );
  if ( size.isValid() )
  {
    setSceneRect( QRectF( transform().dx(), transform().dy(), size.width(), size.height() ) );
  }
}

QSizeF QgsComposerLegend::drawLayerChildItems( QPainter* p, QStandardItem* layerItem, Position& currentPosition, int layerOpacity )
{
  double startYCoord = currentPosition.point.y();
  if ( !layerItem )
  {
    return QSizeF();
  }

  //Draw all symbols first and the texts after (to find out the x coordinate to have the text aligned)
  QList<double> childYCoords;
  QList<double> realItemHeights;

  double textHeight = fontHeightCharacterMM( mItemFont, QChar( '0' ) );
  double itemHeight = qMax( mSymbolHeight, textHeight );

  double textAlignCoord = 0; //alignment for legend text

  QStandardItem* currentItem;

  int numChildren = layerItem->rowCount();

  for ( int i = 0; i < numChildren; ++i )
  {
    //real symbol height. Can be different from standard height in case of point symbols
    double realSymbolHeight;
    double realItemHeight = itemHeight; //will be adjusted if realSymbolHeight turns out to be larger

    currentPosition.point.ry() += mSymbolSpace;
    double currentXCoord = currentPosition.point.x();

    currentItem = layerItem->child( i, 0 );

    if ( !currentItem )
    {
      continue;
    }

    int lineCount = splitStringForWrapping( currentItem->text() ).count();

    QgsSymbol* symbol = 0;
    QgsComposerSymbolItem* symbolItem = dynamic_cast<QgsComposerSymbolItem*>( currentItem );
    if ( symbolItem )
    {
      symbol = symbolItem->symbol();
    }

    QgsSymbolV2* symbolNg = 0;
    QgsComposerSymbolV2Item* symbolV2Item = dynamic_cast<QgsComposerSymbolV2Item*>( currentItem );
    if ( symbolV2Item )
    {
      symbolNg = symbolV2Item->symbolV2();
    }
    QgsComposerRasterSymbolItem* rasterItem = dynamic_cast<QgsComposerRasterSymbolItem*>( currentItem );

    if ( symbol )  //item with symbol?
    {
      //draw symbol
      if ( p ) drawSymbol( p, symbol, currentPosition.point.y() + ( itemHeight - mSymbolHeight ) / 2, currentXCoord, realSymbolHeight, layerOpacity );
      realItemHeight = qMax( realSymbolHeight, itemHeight );
      currentXCoord += mIconLabelSpace;
    }
    else if ( symbolNg ) //item with symbol NG?
    {
      if ( p ) drawSymbolV2( p, symbolNg, currentPosition.point.y() + ( itemHeight - mSymbolHeight ) / 2, currentXCoord, realSymbolHeight, layerOpacity );
      realItemHeight = qMax( realSymbolHeight, itemHeight );
      currentXCoord += mIconLabelSpace;
    }
    else if ( rasterItem )
    {
      if ( p )
      {
        p->setBrush( rasterItem->color() );
        p->drawRect( QRectF( currentXCoord, currentPosition.point.y() + ( itemHeight - mSymbolHeight ) / 2, mSymbolWidth, mSymbolHeight ) );
      }
      currentXCoord += mSymbolWidth;
      currentXCoord += mIconLabelSpace;
    }
    else //item with icon?
    {
      QIcon symbolIcon = currentItem->icon();
      if ( !symbolIcon.isNull() )
      {
        if ( p ) symbolIcon.paint( p, currentXCoord, currentPosition.point.y() + ( itemHeight - mSymbolHeight ) / 2, mSymbolWidth, mSymbolHeight );
        currentXCoord += mSymbolWidth;
        currentXCoord += mIconLabelSpace;
      }
    }

    childYCoords.push_back( currentPosition.point.y() );
    realItemHeights.push_back( realItemHeight );
    currentPosition.point.ry() += lineCount > 0 ? ( realItemHeight + mlineSpacing ) * lineCount : realItemHeight;
    textAlignCoord = qMax( currentXCoord, textAlignCoord );
  }

  currentPosition.expandWidth( textAlignCoord - currentPosition.point.x() );

  for ( int i = 0; i < numChildren; ++i )
  {
    if ( p ) p->setPen( QColor( 0, 0, 0 ) );

    QStringList lines = splitStringForWrapping( layerItem->child( i, 0 )->text() );
    double textY = childYCoords.at( i ) + textHeight + ( realItemHeights.at( i ) - textHeight ) / 2;
    for ( QStringList::Iterator itemPart = lines.begin(); itemPart != lines.end(); ++itemPart )
    {
      if ( p ) drawText( p, textAlignCoord, textY , *itemPart , mItemFont );
      double maxXCoord = textAlignCoord + mBoxSpace + textWidthMillimeters( mItemFont,  *itemPart );
      currentPosition.expandWidth( maxXCoord - currentPosition.point.x() );
      if ( itemPart != lines.end() )
        textY += mlineSpacing + textHeight + ( realItemHeights.at( i ) - textHeight ) / 2;
    }
  }
  return QSizeF( 0, currentPosition.point.y() - startYCoord );
}

void QgsComposerLegend::drawSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int layerOpacity ) const
{
  if ( !s )
  {
    return;
  }

  QGis::GeometryType symbolType = s->type();
  switch ( symbolType )
  {
    case QGis::Point:
      drawPointSymbol( p, s, currentYCoord, currentXPosition, symbolHeight, layerOpacity );
      break;
    case QGis::Line:
      drawLineSymbol( p, s, currentYCoord, currentXPosition, layerOpacity );
      symbolHeight = mSymbolHeight;
      break;
    case QGis::Polygon:
      drawPolygonSymbol( p, s, currentYCoord, currentXPosition, layerOpacity );
      symbolHeight = mSymbolHeight;
      break;
    case QGis::UnknownGeometry:
    case QGis::NoGeometry:
      // shouldn't occur
      break;
  }
}

void QgsComposerLegend::drawSymbolV2( QPainter* p, QgsSymbolV2* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int layerOpacity ) const
{
  Q_UNUSED( layerOpacity );
  if ( !p || !s )
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
  currentXPosition += width;
  currentXPosition += 2 * widthOffset;
  symbolHeight = height + 2 * heightOffset;
}

void QgsComposerLegend::drawPointSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int opacity ) const
{
  if ( !s )
  {
    return;
  }

  QImage pointImage;
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

  //width scale is 1.0
  pointImage = s->getPointSymbolAsImage( 1.0, false, Qt::yellow, 1.0, 0.0, rasterScaleFactor, opacity / 255.0 );

  if ( p )
  {
    p->save();
    p->scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor );

    QPointF imageTopLeft( currentXPosition * rasterScaleFactor, currentYCoord * rasterScaleFactor );
    p->drawImage( imageTopLeft, pointImage );
    p->restore();
  }

  currentXPosition += s->pointSize(); //pointImage.width() / rasterScaleFactor;
  symbolHeight = s->pointSize(); //pointImage.height() / rasterScaleFactor;
}

void QgsComposerLegend::drawLineSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, int opacity ) const
{
  if ( !s )
  {
    return;
  }

  double yCoord = currentYCoord + mSymbolHeight / 2;

  if ( p )
  {
    p->save();
    QPen symbolPen = s->pen();
    QColor penColor = symbolPen.color();
    penColor.setAlpha( opacity );
    symbolPen.setColor( penColor );
    symbolPen.setCapStyle( Qt::FlatCap );
    p->setPen( symbolPen );
    p->drawLine( QPointF( currentXPosition, yCoord ), QPointF( currentXPosition + mSymbolWidth, yCoord ) );
    p->restore();
  }

  currentXPosition += mSymbolWidth;
}

void QgsComposerLegend::drawPolygonSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, int opacity ) const
{
  if ( !s )
  {
    return;
  }

  if ( p )
  {
    //scale brush and set transparencies
    QBrush symbolBrush = s->brush();
    QColor brushColor = symbolBrush.color();
    brushColor.setAlpha( opacity );
    symbolBrush.setColor( brushColor );
    QPaintDevice* paintDevice = p->device();
    if ( paintDevice )
    {
      double rasterScaleFactor = ( paintDevice->logicalDpiX() + paintDevice->logicalDpiY() ) / 2.0 / 25.4;
      QgsRenderer::scaleBrush( symbolBrush, rasterScaleFactor );
    }
    p->setBrush( symbolBrush );

    QPen symbolPen = s->pen();
    QColor penColor = symbolPen.color();
    penColor.setAlpha( opacity );
    symbolPen.setColor( penColor );
    p->setPen( symbolPen );

    p->drawRect( QRectF( currentXPosition, currentYCoord, mSymbolWidth, mSymbolHeight ) );
  }

  currentXPosition += mSymbolWidth;
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
  mColumns.clear();
  adjustBoxSize();
  update();
}

void QgsComposerLegend::setTitleFont( const QFont& f )
{
  mTitleFont = f;
  adjustBoxSize();
  update();
}

void QgsComposerLegend::setGroupFont( const QFont& f )
{
  mColumns.clear();
  mGroupFont = f;
  adjustBoxSize();
  update();
}

void QgsComposerLegend::setLayerFont( const QFont& f )
{
  mColumns.clear();
  mLayerFont = f;
  adjustBoxSize();
  update();
}

void QgsComposerLegend::setItemFont( const QFont& f )
{
  mColumns.clear();
  mItemFont = f;
  adjustBoxSize();
  update();
}

QFont QgsComposerLegend::titleFont() const
{
  return mTitleFont;
}

QFont QgsComposerLegend::groupFont() const
{
  return mGroupFont;
}

QFont QgsComposerLegend::layerFont() const
{
  return mLayerFont;
}

QFont QgsComposerLegend::itemFont() const
{
  return mItemFont;
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

  //write general properties
  composerLegendElem.setAttribute( "title", mTitle );
  composerLegendElem.setAttribute( "columnCount", QString::number( mColumnCount ) );
  composerLegendElem.setAttribute( "titleFont", mTitleFont.toString() );
  composerLegendElem.setAttribute( "groupFont", mGroupFont.toString() );
  composerLegendElem.setAttribute( "layerFont", mLayerFont.toString() );
  composerLegendElem.setAttribute( "itemFont", mItemFont.toString() );
  composerLegendElem.setAttribute( "boxSpace", QString::number( mBoxSpace ) );
  composerLegendElem.setAttribute( "groupSpace", QString::number( mGroupSpace ) );
  composerLegendElem.setAttribute( "layerSpace", QString::number( mLayerSpace ) );
  composerLegendElem.setAttribute( "symbolSpace", QString::number( mSymbolSpace ) );
  composerLegendElem.setAttribute( "iconLabelSpace", QString::number( mIconLabelSpace ) );
  composerLegendElem.setAttribute( "symbolWidth", QString::number( mSymbolWidth ) );
  composerLegendElem.setAttribute( "symbolHeight", QString::number( mSymbolHeight ) );
  composerLegendElem.setAttribute( "wrapChar", mWrapChar );

  if ( mComposerMap )
  {
    composerLegendElem.setAttribute( "map", mComposerMap->id() );
  }

  //write model properties
  mLegendModel.writeXML( composerLegendElem, doc );

  elem.appendChild( composerLegendElem );
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
  //title font
  QString titleFontString = itemElem.attribute( "titleFont" );
  if ( !titleFontString.isEmpty() )
  {
    mTitleFont.fromString( titleFontString );
  }
  //group font
  QString groupFontString = itemElem.attribute( "groupFont" );
  if ( !groupFontString.isEmpty() )
  {
    mGroupFont.fromString( groupFontString );
  }

  //layer font
  QString layerFontString = itemElem.attribute( "layerFont" );
  if ( !layerFontString.isEmpty() )
  {
    mLayerFont.fromString( layerFontString );
  }
  //item font
  QString itemFontString = itemElem.attribute( "itemFont" );
  if ( !itemFontString.isEmpty() )
  {
    mItemFont.fromString( itemFontString );
  }

  //spaces
  mBoxSpace = itemElem.attribute( "boxSpace", "2.0" ).toDouble();
  mGroupSpace = itemElem.attribute( "groupSpace", "3.0" ).toDouble();
  mLayerSpace = itemElem.attribute( "layerSpace", "3.0" ).toDouble();
  mSymbolSpace = itemElem.attribute( "symbolSpace", "2.0" ).toDouble();
  mIconLabelSpace = itemElem.attribute( "iconLabelSpace", "2.0" ).toDouble();
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

  emit itemChanged();
  return true;
}

void QgsComposerLegend::setComposerMap( const QgsComposerMap* map )
{
  mColumns.clear();
  mComposerMap = map;
  QObject::connect( map, SIGNAL( destroyed( QObject* ) ), this, SLOT( invalidateCurrentMap() ) );
}

void QgsComposerLegend::invalidateCurrentMap()
{
  disconnect( mComposerMap, SIGNAL( destroyed( QObject* ) ), this, SLOT( invalidateCurrentMap() ) );
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
