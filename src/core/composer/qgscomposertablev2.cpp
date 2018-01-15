/***************************************************************************
                         qgscomposertablev2.cpp
                         ------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson, Marco Hugentobler
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

#include "qgscomposertablev2.h"
#include "qgscomposerutils.h"
#include "qgscomposertablecolumn.h"
#include "qgssymbollayerutils.h"
#include "qgscomposerframe.h"
#include "qgsfontutils.h"
#include "qgssettings.h"

//
// QgsComposerTableStyle
//

bool QgsComposerTableStyle::writeXml( QDomElement &styleElem, QDomDocument &doc ) const
{
  Q_UNUSED( doc );
  styleElem.setAttribute( QStringLiteral( "cellBackgroundColor" ), QgsSymbolLayerUtils::encodeColor( cellBackgroundColor ) );
  styleElem.setAttribute( QStringLiteral( "enabled" ), enabled );
  return true;
}

bool QgsComposerTableStyle::readXml( const QDomElement &styleElem )
{
  cellBackgroundColor = QgsSymbolLayerUtils::decodeColor( styleElem.attribute( QStringLiteral( "cellBackgroundColor" ), QStringLiteral( "255,255,255,255" ) ) );
  enabled = ( styleElem.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
  return true;
}


//
// QgsComposerTableV2
//

QgsComposerTableV2::QgsComposerTableV2( QgsComposition *composition, bool createUndoCommands )
  : QgsComposerMultiFrame( composition, createUndoCommands )
  , mHeaderFontColor( Qt::black )
  , mContentFontColor( Qt::black )
  , mGridColor( Qt::black )
  , mBackgroundColor( Qt::white )
{

  if ( mComposition )
  {
    connect( mComposition, &QgsComposition::itemRemoved, this, &QgsComposerMultiFrame::handleFrameRemoval );
  }

  //get default composer font from settings
  QgsSettings settings;
  QString defaultFontString = settings.value( QStringLiteral( "Composer/defaultFont" ) ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    mHeaderFont.setFamily( defaultFontString );
    mContentFont.setFamily( defaultFontString );
  }

  initStyles();
}

QgsComposerTableV2::QgsComposerTableV2()
  : QgsComposerMultiFrame( nullptr, false )
  , mHeaderFontColor( Qt::black )
  , mContentFontColor( Qt::black )
  , mGridColor( Qt::black )
  , mBackgroundColor( Qt::white )
{
  initStyles();
}

QgsComposerTableV2::~QgsComposerTableV2()
{
  qDeleteAll( mColumns );
  mColumns.clear();

  qDeleteAll( mCellStyles );
  mCellStyles.clear();
}

bool QgsComposerTableV2::writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames ) const
{
  elem.setAttribute( QStringLiteral( "cellMargin" ), QString::number( mCellMargin ) );
  elem.setAttribute( QStringLiteral( "emptyTableMode" ), QString::number( static_cast< int >( mEmptyTableMode ) ) );
  elem.setAttribute( QStringLiteral( "emptyTableMessage" ), mEmptyTableMessage );
  elem.setAttribute( QStringLiteral( "showEmptyRows" ), mShowEmptyRows );
  elem.appendChild( QgsFontUtils::toXmlElement( mHeaderFont, doc, QStringLiteral( "headerFontProperties" ) ) );
  elem.setAttribute( QStringLiteral( "headerFontColor" ), QgsSymbolLayerUtils::encodeColor( mHeaderFontColor ) );
  elem.setAttribute( QStringLiteral( "headerHAlignment" ), QString::number( static_cast< int >( mHeaderHAlignment ) ) );
  elem.setAttribute( QStringLiteral( "headerMode" ), QString::number( static_cast< int >( mHeaderMode ) ) );
  elem.appendChild( QgsFontUtils::toXmlElement( mContentFont, doc, QStringLiteral( "contentFontProperties" ) ) );
  elem.setAttribute( QStringLiteral( "contentFontColor" ), QgsSymbolLayerUtils::encodeColor( mContentFontColor ) );
  elem.setAttribute( QStringLiteral( "gridStrokeWidth" ), QString::number( mGridStrokeWidth ) );
  elem.setAttribute( QStringLiteral( "gridColor" ), QgsSymbolLayerUtils::encodeColor( mGridColor ) );
  elem.setAttribute( QStringLiteral( "horizontalGrid" ), mHorizontalGrid );
  elem.setAttribute( QStringLiteral( "verticalGrid" ), mVerticalGrid );
  elem.setAttribute( QStringLiteral( "showGrid" ), mShowGrid );
  elem.setAttribute( QStringLiteral( "backgroundColor" ), QgsSymbolLayerUtils::encodeColor( mBackgroundColor ) );
  elem.setAttribute( QStringLiteral( "wrapBehavior" ), QString::number( static_cast< int >( mWrapBehavior ) ) );

  //columns
  QDomElement displayColumnsElem = doc.createElement( QStringLiteral( "displayColumns" ) );
  QList<QgsComposerTableColumn *>::const_iterator columnIt = mColumns.constBegin();
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    QDomElement columnElem = doc.createElement( QStringLiteral( "column" ) );
    ( *columnIt )->writeXml( columnElem, doc );
    displayColumnsElem.appendChild( columnElem );
  }
  elem.appendChild( displayColumnsElem );

  //cell styles
  QDomElement stylesElem = doc.createElement( QStringLiteral( "cellStyles" ) );
  QMap< CellStyleGroup, QString >::const_iterator it = mCellStyleNames.constBegin();
  for ( ; it != mCellStyleNames.constEnd(); ++it )
  {
    QString styleName = it.value();
    QDomElement styleElem = doc.createElement( styleName );
    QgsComposerTableStyle *style = mCellStyles.value( it.key() );
    if ( style )
    {
      style->writeXml( styleElem, doc );
      stylesElem.appendChild( styleElem );
    }
  }
  elem.appendChild( stylesElem );

  bool state = _writeXml( elem, doc, ignoreFrames );
  return state;
}

bool QgsComposerTableV2::readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames )
{
  deleteFrames();

  //first create the frames
  if ( !_readXml( itemElem, doc, ignoreFrames ) )
  {
    return false;
  }

  if ( itemElem.isNull() )
  {
    return false;
  }

  mEmptyTableMode = QgsComposerTableV2::EmptyTableMode( itemElem.attribute( QStringLiteral( "emptyTableMode" ), QStringLiteral( "0" ) ).toInt() );
  mEmptyTableMessage = itemElem.attribute( QStringLiteral( "emptyTableMessage" ), tr( "No matching records" ) );
  mShowEmptyRows = itemElem.attribute( QStringLiteral( "showEmptyRows" ), QStringLiteral( "0" ) ).toInt();
  if ( !QgsFontUtils::setFromXmlChildNode( mHeaderFont, itemElem, QStringLiteral( "headerFontProperties" ) ) )
  {
    mHeaderFont.fromString( itemElem.attribute( QStringLiteral( "headerFont" ), QLatin1String( "" ) ) );
  }
  mHeaderFontColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "headerFontColor" ), QStringLiteral( "0,0,0,255" ) ) );
  mHeaderHAlignment = QgsComposerTableV2::HeaderHAlignment( itemElem.attribute( QStringLiteral( "headerHAlignment" ), QStringLiteral( "0" ) ).toInt() );
  mHeaderMode = QgsComposerTableV2::HeaderMode( itemElem.attribute( QStringLiteral( "headerMode" ), QStringLiteral( "0" ) ).toInt() );
  if ( !QgsFontUtils::setFromXmlChildNode( mContentFont, itemElem, QStringLiteral( "contentFontProperties" ) ) )
  {
    mContentFont.fromString( itemElem.attribute( QStringLiteral( "contentFont" ), QLatin1String( "" ) ) );
  }
  mContentFontColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "contentFontColor" ), QStringLiteral( "0,0,0,255" ) ) );
  mCellMargin = itemElem.attribute( QStringLiteral( "cellMargin" ), QStringLiteral( "1.0" ) ).toDouble();
  mGridStrokeWidth = itemElem.attribute( QStringLiteral( "gridStrokeWidth" ), QStringLiteral( "0.5" ) ).toDouble();
  mHorizontalGrid = itemElem.attribute( QStringLiteral( "horizontalGrid" ), QStringLiteral( "1" ) ).toInt();
  mVerticalGrid = itemElem.attribute( QStringLiteral( "verticalGrid" ), QStringLiteral( "1" ) ).toInt();
  mShowGrid = itemElem.attribute( QStringLiteral( "showGrid" ), QStringLiteral( "1" ) ).toInt();
  mGridColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "gridColor" ), QStringLiteral( "0,0,0,255" ) ) );
  mBackgroundColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "backgroundColor" ), QStringLiteral( "255,255,255,0" ) ) );
  mWrapBehavior = QgsComposerTableV2::WrapBehavior( itemElem.attribute( QStringLiteral( "wrapBehavior" ), QStringLiteral( "0" ) ).toInt() );

  //restore column specifications
  qDeleteAll( mColumns );
  mColumns.clear();
  QDomNodeList columnsList = itemElem.elementsByTagName( QStringLiteral( "displayColumns" ) );
  if ( !columnsList.isEmpty() )
  {
    QDomElement columnsElem = columnsList.at( 0 ).toElement();
    QDomNodeList columnEntryList = columnsElem.elementsByTagName( QStringLiteral( "column" ) );
    for ( int i = 0; i < columnEntryList.size(); ++i )
    {
      QDomElement columnElem = columnEntryList.at( i ).toElement();
      QgsComposerTableColumn *column = new QgsComposerTableColumn;
      column->readXml( columnElem );
      mColumns.append( column );
    }
  }

  //restore cell styles
  QDomNodeList stylesList = itemElem.elementsByTagName( QStringLiteral( "cellStyles" ) );
  if ( !stylesList.isEmpty() )
  {
    QDomElement stylesElem = stylesList.at( 0 ).toElement();

    QMap< CellStyleGroup, QString >::const_iterator it = mCellStyleNames.constBegin();
    for ( ; it != mCellStyleNames.constEnd(); ++it )
    {
      QString styleName = it.value();
      QDomNodeList styleList = stylesElem.elementsByTagName( styleName );
      if ( !styleList.isEmpty() )
      {
        QDomElement styleElem = styleList.at( 0 ).toElement();
        QgsComposerTableStyle *style = mCellStyles.value( it.key() );
        if ( style )
          style->readXml( styleElem );
      }
    }
  }

  return true;
}

QSizeF QgsComposerTableV2::totalSize() const
{
  return mTableSize;
}

int QgsComposerTableV2::rowsVisible( double frameHeight, int firstRow, bool includeHeader, bool includeEmptyRows ) const
{
  //calculate header height
  double headerHeight = 0;
  if ( includeHeader )
  {
    //frame has a header
    headerHeight = 2 * ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 ) + 2 * mCellMargin +  QgsComposerUtils::fontAscentMM( mHeaderFont );
  }
  else
  {
    //frame has no header text, just the stroke
    headerHeight = ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 );
  }

  //remaining height available for content rows
  double contentHeight = frameHeight - headerHeight;

  double gridHeight = ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 );

  int currentRow = firstRow;
  while ( contentHeight > 0 && currentRow <= mTableContents.count() )
  {
    double currentRowHeight = mMaxRowHeightMap.value( currentRow + 1 ) + gridHeight + 2 * mCellMargin;
    contentHeight -= currentRowHeight;
    currentRow++;
  }

  if ( includeEmptyRows && contentHeight > 0 )
  {
    double rowHeight = ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 ) + 2 * mCellMargin + QgsComposerUtils::fontAscentMM( mContentFont );
    currentRow += std::max( std::floor( contentHeight / rowHeight ), 0.0 );
  }

  return currentRow - firstRow - 1;
}

int QgsComposerTableV2::rowsVisible( int frameIndex, int firstRow, bool includeEmptyRows ) const
{
  //get frame extent
  if ( frameIndex >= frameCount() )
  {
    return 0;
  }
  QRectF frameExtent = frame( frameIndex )->extent();

  bool includeHeader = false;
  if ( ( mHeaderMode == QgsComposerTableV2::FirstFrame && frameIndex < 1 )
       || ( mHeaderMode == QgsComposerTableV2::AllFrames ) )
  {
    includeHeader = true;
  }
  return rowsVisible( frameExtent.height(), firstRow, includeHeader, includeEmptyRows );
}

QPair<int, int> QgsComposerTableV2::rowRange( const int frameIndex ) const
{
  //calculate row height
  if ( frameIndex >= frameCount() )
  {
    //bad frame index
    return qMakePair( 0, 0 );
  }

  //loop through all previous frames to calculate how many rows are visible in each
  //as the entire height of a frame may not be utilized for content rows
  int rowsAlreadyShown = 0;
  for ( int idx = 0; idx < frameIndex; ++idx )
  {
    rowsAlreadyShown += rowsVisible( idx, rowsAlreadyShown, false );
  }

  //using zero based indexes
  int firstVisible = std::min( rowsAlreadyShown, mTableContents.length() );
  int possibleRowsVisible = rowsVisible( frameIndex, rowsAlreadyShown, false );
  int lastVisible = std::min( firstVisible + possibleRowsVisible, mTableContents.length() );

  return qMakePair( firstVisible, lastVisible );
}

void QgsComposerTableV2::render( QPainter *p, const QRectF &, const int frameIndex )
{
  if ( !p )
  {
    return;
  }

  bool emptyTable = mTableContents.length() == 0;
  if ( emptyTable && mEmptyTableMode == QgsComposerTableV2::HideTable )
  {
    //empty table set to hide table mode, so don't draw anything
    return;
  }

  if ( mComposition->plotStyle() == QgsComposition::Print ||
       mComposition->plotStyle() == QgsComposition::Postscript )
  {
    //exporting composition, so force an attribute refresh
    //we do this in case vector layer has changed via an external source (e.g., another database user)
    refreshAttributes();
  }

  //calculate which rows to show in this frame
  QPair< int, int > rowsToShow = rowRange( frameIndex );

  double gridSizeX = mShowGrid && mVerticalGrid ? mGridStrokeWidth : 0;
  double gridSizeY = mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0;
  double cellHeaderHeight = QgsComposerUtils::fontAscentMM( mHeaderFont ) + 2 * mCellMargin;
  double cellBodyHeight = QgsComposerUtils::fontAscentMM( mContentFont ) + 2 * mCellMargin;
  QRectF cell;

  //calculate whether a header is required
  bool drawHeader = ( ( mHeaderMode == QgsComposerTableV2::FirstFrame && frameIndex < 1 )
                      || ( mHeaderMode == QgsComposerTableV2::AllFrames ) );
  //calculate whether drawing table contents is required
  bool drawContents = !( emptyTable && mEmptyTableMode == QgsComposerTableV2::ShowMessage );

  int numberRowsToDraw = rowsToShow.second - rowsToShow.first;
  int numberEmptyRows = 0;
  if ( drawContents && mShowEmptyRows )
  {
    numberRowsToDraw = rowsVisible( frameIndex, rowsToShow.first, true );
    numberEmptyRows = numberRowsToDraw - rowsToShow.second + rowsToShow.first;
  }
  bool mergeCells = false;
  if ( emptyTable && mEmptyTableMode == QgsComposerTableV2::ShowMessage )
  {
    //draw a merged row for the empty table message
    numberRowsToDraw++;
    rowsToShow.second++;
    mergeCells = true;
  }

  p->save();
  //antialiasing on
  p->setRenderHint( QPainter::Antialiasing, true );

  //draw the text
  p->setPen( Qt::SolidLine );

  double currentX = gridSizeX;
  double currentY = gridSizeY;
  if ( drawHeader )
  {
    //draw the headers
    int col = 0;
    Q_FOREACH ( const QgsComposerTableColumn *column, mColumns )
    {
      //draw background
      p->save();
      p->setPen( Qt::NoPen );
      p->setBrush( backgroundColor( -1, col ) );
      p->drawRect( QRectF( currentX, currentY, mMaxColumnWidthMap[col] + 2 * mCellMargin, cellHeaderHeight ) );
      p->restore();

      currentX += mCellMargin;

      Qt::TextFlag textFlag = static_cast< Qt::TextFlag >( 0 );
      if ( column->width() <= 0 )
      {
        //automatic column width, so we use the Qt::TextDontClip flag when drawing contents, as this works nicer for italicised text
        //which may slightly exceed the calculated width
        //if column size was manually set then we do apply text clipping, to avoid painting text outside of columns width
        textFlag = Qt::TextDontClip;
      }

      cell = QRectF( currentX, currentY, mMaxColumnWidthMap[col], cellHeaderHeight );

      //calculate alignment of header
      Qt::AlignmentFlag headerAlign = Qt::AlignLeft;
      switch ( mHeaderHAlignment )
      {
        case FollowColumn:
          headerAlign = column->hAlignment();
          break;
        case HeaderLeft:
          headerAlign = Qt::AlignLeft;
          break;
        case HeaderCenter:
          headerAlign = Qt::AlignHCenter;
          break;
        case HeaderRight:
          headerAlign = Qt::AlignRight;
          break;
      }

      QgsComposerUtils::drawText( p, cell, column->heading(), mHeaderFont, mHeaderFontColor, headerAlign, Qt::AlignVCenter, textFlag );

      currentX += mMaxColumnWidthMap[ col ];
      currentX += mCellMargin;
      currentX += gridSizeX;
      col++;
    }

    currentY += cellHeaderHeight;
    currentY += gridSizeY;
  }

  //now draw the body cells
  int rowsDrawn = 0;
  if ( drawContents )
  {
    //draw the attribute values
    for ( int row = rowsToShow.first; row < rowsToShow.second; ++row )
    {
      rowsDrawn++;
      currentX = gridSizeX;
      int col = 0;

      //calculate row height
      double rowHeight = mMaxRowHeightMap[row + 1] + 2 * mCellMargin;


      Q_FOREACH ( const QgsComposerTableColumn *column, mColumns )
      {
        //draw background
        p->save();
        p->setPen( Qt::NoPen );
        p->setBrush( backgroundColor( row, col ) );
        p->drawRect( QRectF( currentX, currentY, mMaxColumnWidthMap[col] + 2 * mCellMargin, rowHeight ) );
        p->restore();

        // currentY = gridSize;
        currentX += mCellMargin;

        QVariant cellContents = mTableContents.at( row ).at( col );
        QString str = cellContents.toString();

        Qt::TextFlag textFlag = static_cast< Qt::TextFlag >( 0 );
        if ( column->width() <= 0 && mWrapBehavior == TruncateText )
        {
          //automatic column width, so we use the Qt::TextDontClip flag when drawing contents, as this works nicer for italicised text
          //which may slightly exceed the calculated width
          //if column size was manually set then we do apply text clipping, to avoid painting text outside of columns width
          textFlag = Qt::TextDontClip;
        }
        else if ( textRequiresWrapping( str, column->width(), mContentFont ) )
        {
          str = wrappedText( str, column->width(), mContentFont );
        }

        cell = QRectF( currentX, currentY, mMaxColumnWidthMap[col], rowHeight );
        QgsComposerUtils::drawText( p, cell, str, mContentFont, mContentFontColor, column->hAlignment(), column->vAlignment(), textFlag );

        currentX += mMaxColumnWidthMap[ col ];
        currentX += mCellMargin;
        currentX += gridSizeX;
        col++;
      }
      currentY += rowHeight;
      currentY += gridSizeY;
    }
  }

  if ( numberRowsToDraw > rowsDrawn )
  {
    p->save();
    p->setPen( Qt::NoPen );

    //draw background of empty rows
    for ( int row = rowsDrawn; row < numberRowsToDraw; ++row )
    {
      currentX = gridSizeX;
      int col = 0;

      if ( mergeCells )
      {
        p->setBrush( backgroundColor( row + 10000, 0 ) );
        p->drawRect( QRectF( gridSizeX, currentY, mTableSize.width() - 2 * gridSizeX, cellBodyHeight ) );
      }
      else
      {
        for ( QList<QgsComposerTableColumn *>::const_iterator columnIt = mColumns.constBegin(); columnIt != mColumns.constEnd(); ++columnIt )
        {
          //draw background

          //we use a bit of a hack here - since we don't want these extra blank rows to match the firstrow/lastrow rule, add 10000 to row number
          p->setBrush( backgroundColor( row + 10000, col ) );
          p->drawRect( QRectF( currentX, currentY, mMaxColumnWidthMap[col] + 2 * mCellMargin, cellBodyHeight ) );

          // currentY = gridSize;
          currentX += mMaxColumnWidthMap[ col ] + 2 * mCellMargin;
          currentX += gridSizeX;
          col++;
        }
      }
      currentY += cellBodyHeight + gridSizeY;
    }
    p->restore();
  }

  //and the borders
  if ( mShowGrid )
  {
    QPen gridPen;
    gridPen.setWidthF( mGridStrokeWidth );
    gridPen.setColor( mGridColor );
    gridPen.setJoinStyle( Qt::MiterJoin );
    p->setPen( gridPen );
    if ( mHorizontalGrid )
    {
      drawHorizontalGridLines( p, rowsToShow.first, rowsToShow.second + numberEmptyRows, drawHeader );
    }
    if ( mVerticalGrid )
    {
      drawVerticalGridLines( p, mMaxColumnWidthMap, rowsToShow.first, rowsToShow.second + numberEmptyRows, drawHeader, mergeCells );
    }
  }

  //special case - no records and table is set to ShowMessage mode
  if ( emptyTable && mEmptyTableMode == QgsComposerTableV2::ShowMessage )
  {
    double messageX = gridSizeX + mCellMargin;
    double messageY = gridSizeY + ( drawHeader ? cellHeaderHeight + gridSizeY : 0 );
    cell = QRectF( messageX, messageY, mTableSize.width() - messageX, cellBodyHeight );
    QgsComposerUtils::drawText( p, cell, mEmptyTableMessage, mContentFont, mContentFontColor, Qt::AlignHCenter, Qt::AlignVCenter, static_cast< Qt::TextFlag >( 0 ) );
  }

  p->restore();

}

void QgsComposerTableV2::setCellMargin( const double margin )
{
  if ( qgsDoubleNear( margin, mCellMargin ) )
  {
    return;
  }

  mCellMargin = margin;

  //since spacing has changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setEmptyTableBehavior( const QgsComposerTableV2::EmptyTableMode mode )
{
  if ( mode == mEmptyTableMode )
  {
    return;
  }

  mEmptyTableMode = mode;

  //since appearance has changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setEmptyTableMessage( const QString &message )
{
  if ( message == mEmptyTableMessage )
  {
    return;
  }

  mEmptyTableMessage = message;

  //since message has changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setShowEmptyRows( const bool showEmpty )
{
  if ( showEmpty == mShowEmptyRows )
  {
    return;
  }

  mShowEmptyRows = showEmpty;
  update();
  emit changed();
}

void QgsComposerTableV2::setHeaderFont( const QFont &font )
{
  if ( font == mHeaderFont )
  {
    return;
  }

  mHeaderFont = font;
  //since font attributes have changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setHeaderFontColor( const QColor &color )
{
  if ( color == mHeaderFontColor )
  {
    return;
  }

  mHeaderFontColor = color;
  update();

  emit changed();
}

void QgsComposerTableV2::setHeaderHAlignment( const QgsComposerTableV2::HeaderHAlignment alignment )
{
  if ( alignment == mHeaderHAlignment )
  {
    return;
  }

  mHeaderHAlignment = alignment;
  update();

  emit changed();
}

void QgsComposerTableV2::setHeaderMode( const QgsComposerTableV2::HeaderMode mode )
{
  if ( mode == mHeaderMode )
  {
    return;
  }

  mHeaderMode = mode;
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setContentFont( const QFont &font )
{
  if ( font == mContentFont )
  {
    return;
  }

  mContentFont = font;
  //since font attributes have changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setContentFontColor( const QColor &color )
{
  if ( color == mContentFontColor )
  {
    return;
  }

  mContentFontColor = color;
  update();

  emit changed();
}

void QgsComposerTableV2::setShowGrid( const bool showGrid )
{
  if ( showGrid == mShowGrid )
  {
    return;
  }

  mShowGrid = showGrid;
  //since grid spacing has changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setGridStrokeWidth( const double width )
{
  if ( qgsDoubleNear( width, mGridStrokeWidth ) )
  {
    return;
  }

  mGridStrokeWidth = width;
  //since grid spacing has changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setGridColor( const QColor &color )
{
  if ( color == mGridColor )
  {
    return;
  }

  mGridColor = color;
  update();

  emit changed();
}

void QgsComposerTableV2::setHorizontalGrid( const bool horizontalGrid )
{
  if ( horizontalGrid == mHorizontalGrid )
  {
    return;
  }

  mHorizontalGrid = horizontalGrid;
  //since grid spacing has changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setVerticalGrid( const bool verticalGrid )
{
  if ( verticalGrid == mVerticalGrid )
  {
    return;
  }

  mVerticalGrid = verticalGrid;
  //since grid spacing has changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setBackgroundColor( const QColor &color )
{
  if ( color == mBackgroundColor )
  {
    return;
  }

  mBackgroundColor = color;
  update();

  emit changed();
}

void QgsComposerTableV2::setWrapBehavior( QgsComposerTableV2::WrapBehavior behavior )
{
  if ( behavior == mWrapBehavior )
  {
    return;
  }

  mWrapBehavior = behavior;
  recalculateTableSize();

  emit changed();
}

void QgsComposerTableV2::setColumns( const QgsComposerTableColumns &columns )
{
  //remove existing columns
  qDeleteAll( mColumns );
  mColumns.clear();

  mColumns.append( columns );
}

void QgsComposerTableV2::setCellStyle( QgsComposerTableV2::CellStyleGroup group, const QgsComposerTableStyle &style )
{
  if ( mCellStyles.contains( group ) )
    delete mCellStyles.take( group );

  mCellStyles.insert( group, new QgsComposerTableStyle( style ) );
}

const QgsComposerTableStyle *QgsComposerTableV2::cellStyle( QgsComposerTableV2::CellStyleGroup group ) const
{
  if ( !mCellStyles.contains( group ) )
    return nullptr;

  return mCellStyles.value( group );
}

QMap<int, QString> QgsComposerTableV2::headerLabels() const
{
  QMap<int, QString> headers;

  QgsComposerTableColumns::const_iterator columnIt = mColumns.constBegin();
  int col = 0;
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    headers.insert( col, ( *columnIt )->heading() );
    col++;
  }
  return headers;
}

QSizeF QgsComposerTableV2::fixedFrameSize( const int frameIndex ) const
{
  Q_UNUSED( frameIndex );
  return QSizeF( mTableSize.width(), 0 );
}

QSizeF QgsComposerTableV2::minFrameSize( const int frameIndex ) const
{
  double height = 0;
  if ( ( mHeaderMode == QgsComposerTableV2::FirstFrame && frameIndex < 1 )
       || ( mHeaderMode == QgsComposerTableV2::AllFrames ) )
  {
    //header required, force frame to be high enough for header
    height = 2 * ( mShowGrid ? mGridStrokeWidth : 0 ) + 2 * mCellMargin +  QgsComposerUtils::fontAscentMM( mHeaderFont );
  }
  return QSizeF( 0, height );
}

void QgsComposerTableV2::refreshAttributes()
{
  mMaxColumnWidthMap.clear();
  mMaxRowHeightMap.clear();
  mTableContents.clear();

  //get new contents
  if ( !getTableContents( mTableContents ) )
  {
    return;
  }
}

void QgsComposerTableV2::recalculateFrameSizes()
{
  mTableSize = QSizeF( totalWidth(), totalHeight() );
  QgsComposerMultiFrame::recalculateFrameSizes();
}

void QgsComposerTableV2::initStyles()
{
  mCellStyles.insert( OddColumns, new QgsComposerTableStyle() );
  mCellStyles.insert( EvenColumns, new QgsComposerTableStyle() );
  mCellStyles.insert( OddRows, new QgsComposerTableStyle() );
  mCellStyles.insert( EvenRows, new QgsComposerTableStyle() );
  mCellStyles.insert( FirstColumn, new QgsComposerTableStyle() );
  mCellStyles.insert( LastColumn, new QgsComposerTableStyle() );
  mCellStyles.insert( HeaderRow, new QgsComposerTableStyle() );
  mCellStyles.insert( FirstRow, new QgsComposerTableStyle() );
  mCellStyles.insert( LastRow, new QgsComposerTableStyle() );

  mCellStyleNames.insert( OddColumns, QStringLiteral( "oddColumns" ) );
  mCellStyleNames.insert( EvenColumns, QStringLiteral( "evenColumns" ) );
  mCellStyleNames.insert( OddRows, QStringLiteral( "oddRows" ) );
  mCellStyleNames.insert( EvenRows, QStringLiteral( "evenRows" ) );
  mCellStyleNames.insert( FirstColumn, QStringLiteral( "firstColumn" ) );
  mCellStyleNames.insert( LastColumn, QStringLiteral( "lastColumn" ) );
  mCellStyleNames.insert( HeaderRow, QStringLiteral( "headerRow" ) );
  mCellStyleNames.insert( FirstRow, QStringLiteral( "firstRow" ) );
  mCellStyleNames.insert( LastRow, QStringLiteral( "lastRow" ) );
}

bool QgsComposerTableV2::calculateMaxColumnWidths()
{
  mMaxColumnWidthMap.clear();

  //total number of cells (rows + 1 for header)
  int cols = mColumns.count();
  int cells = cols * ( mTableContents.count() + 1 );
  QVector< double > widths( cells );

  //first, go through all the column headers and calculate the sizes
  QgsComposerTableColumns::const_iterator columnIt = mColumns.constBegin();
  int col = 0;
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    if ( ( *columnIt )->width() > 0 )
    {
      //column has manually specified width
      widths[col] = ( *columnIt )->width();
    }
    else if ( mHeaderMode != QgsComposerTableV2::NoHeaders )
    {
      widths[col] = QgsComposerUtils::textWidthMM( mHeaderFont, ( *columnIt )->heading() );
    }
    else
    {
      widths[col] = 0.0;
    }
    col++;
  }

  //next, go through all the table contents and calculate the sizes
  QgsComposerTableContents::const_iterator rowIt = mTableContents.constBegin();
  double currentCellTextWidth;
  int row = 1;
  for ( ; rowIt != mTableContents.constEnd(); ++rowIt )
  {
    QgsComposerTableRow::const_iterator colIt = rowIt->constBegin();
    col = 0;
    for ( ; colIt != rowIt->constEnd(); ++colIt )
    {
      if ( mColumns.at( col )->width() <= 0 )
      {
        //column width set to automatic, so check content size
        QStringList multiLineSplit = ( *colIt ).toString().split( '\n' );
        currentCellTextWidth = 0;
        Q_FOREACH ( const QString &line, multiLineSplit )
        {
          currentCellTextWidth = std::max( currentCellTextWidth, QgsComposerUtils::textWidthMM( mContentFont, line ) );
        }
        widths[ row * cols + col ] = currentCellTextWidth;
      }
      else
      {
        widths[ row * cols + col ] = 0;
      }

      col++;
    }
    row++;
  }

  //calculate maximum
  for ( int col = 0; col < cols; ++col )
  {
    double maxColWidth = 0;
    for ( int row = 0; row < mTableContents.count() + 1; ++row )
    {
      maxColWidth = std::max( widths[ row * cols + col ], maxColWidth );
    }
    mMaxColumnWidthMap.insert( col, maxColWidth );
  }

  return true;
}

bool QgsComposerTableV2::calculateMaxRowHeights()
{
  mMaxRowHeightMap.clear();

  //total number of cells (rows + 1 for header)
  int cols = mColumns.count();
  int cells = cols * ( mTableContents.count() + 1 );
  QVector< double > heights( cells );

  //first, go through all the column headers and calculate the sizes
  QgsComposerTableColumns::const_iterator columnIt = mColumns.constBegin();
  int col = 0;
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    //height
    heights[col] = mHeaderMode != QgsComposerTableV2::NoHeaders ? QgsComposerUtils::textHeightMM( mHeaderFont, ( *columnIt )->heading() ) : 0;
    col++;
  }

  //next, go through all the table contents and calculate the sizes
  QgsComposerTableContents::const_iterator rowIt = mTableContents.constBegin();
  int row = 1;
  for ( ; rowIt != mTableContents.constEnd(); ++rowIt )
  {
    QgsComposerTableRow::const_iterator colIt = rowIt->constBegin();
    col = 0;
    for ( ; colIt != rowIt->constEnd(); ++colIt )
    {
      if ( textRequiresWrapping( ( *colIt ).toString(), mColumns.at( col )->width(), mContentFont ) )
      {
        //contents too wide for cell, need to wrap
        heights[ row * cols + col ] = QgsComposerUtils::textHeightMM( mContentFont, wrappedText( ( *colIt ).toString(), mColumns.at( col )->width(), mContentFont ) );
      }
      else
      {
        heights[ row * cols + col ] = QgsComposerUtils::textHeightMM( mContentFont, ( *colIt ).toString() );
      }

      col++;
    }
    row++;
  }

  //calculate maximum
  for ( int row = 0; row < mTableContents.count() + 1; ++row )
  {
    double maxRowHeight = 0;
    for ( int col = 0; col < cols; ++col )
    {
      maxRowHeight = std::max( heights[ row * cols + col ], maxRowHeight );
    }
    mMaxRowHeightMap.insert( row, maxRowHeight );
  }

  return true;
}

double QgsComposerTableV2::totalWidth()
{
  //check how much space each column needs
  if ( !calculateMaxColumnWidths() )
  {
    return 0;
  }

  //adapt frame to total width
  double totalWidth = 0;
  QMap<int, double>::const_iterator maxColWidthIt = mMaxColumnWidthMap.constBegin();
  for ( ; maxColWidthIt != mMaxColumnWidthMap.constEnd(); ++maxColWidthIt )
  {
    totalWidth += maxColWidthIt.value();
  }
  totalWidth += ( 2 * mMaxColumnWidthMap.size() * mCellMargin );
  totalWidth += ( mMaxColumnWidthMap.size() + 1 ) * ( mShowGrid && mVerticalGrid ? mGridStrokeWidth : 0 );

  return totalWidth;
}

double QgsComposerTableV2::totalHeight()
{
  //check how much space each row needs
  if ( !calculateMaxRowHeights() )
  {
    return 0;
  }

  double height = 0;

  //loop through all existing frames to calculate how many rows are visible in each
  //as the entire height of a frame may not be utilized for content rows
  int rowsAlreadyShown = 0;
  int numberExistingFrames = frameCount();
  int rowsVisibleInLastFrame = 0;
  double heightOfLastFrame = 0;
  for ( int idx = 0; idx < numberExistingFrames; ++idx )
  {
    bool hasHeader = ( ( mHeaderMode == QgsComposerTableV2::FirstFrame && idx == 0 )
                       || ( mHeaderMode == QgsComposerTableV2::AllFrames ) );
    heightOfLastFrame = frame( idx )->rect().height();
    rowsVisibleInLastFrame = rowsVisible( heightOfLastFrame, rowsAlreadyShown, hasHeader, false );
    rowsAlreadyShown += rowsVisibleInLastFrame;
    height += heightOfLastFrame;
    if ( rowsAlreadyShown >= mTableContents.length() )
    {
      //shown entire contents of table, nothing remaining
      return height;
    }
  }

  //calculate how many rows left to show
  int remainingRows = mTableContents.length() - rowsAlreadyShown;

  if ( remainingRows <= 0 )
  {
    //no remaining rows
    return height;
  }

  if ( mResizeMode == QgsComposerMultiFrame::ExtendToNextPage )
  {
    heightOfLastFrame = mComposition->paperHeight();
  }

  bool hasHeader = ( ( mHeaderMode == QgsComposerTableV2::FirstFrame && numberExistingFrames < 1 )
                     || ( mHeaderMode == QgsComposerTableV2::AllFrames ) );

  int numberFramesMissing = 0;
  while ( remainingRows > 0 )
  {
    numberFramesMissing++;

    rowsVisibleInLastFrame = rowsVisible( heightOfLastFrame, rowsAlreadyShown, hasHeader, false );
    if ( rowsVisibleInLastFrame < 1 )
    {
      //if no rows are visible in the last frame, calculation of missing frames
      //is impossible. So just return total height of existing frames
      return height;
    }

    rowsAlreadyShown += rowsVisibleInLastFrame;
    remainingRows = mTableContents.length() - rowsAlreadyShown;
  }

  //rows remain unshown -- how many extra frames would we need to complete the table?
  //assume all added frames are same size as final frame
  height += heightOfLastFrame * numberFramesMissing;
  return height;
}

void QgsComposerTableV2::drawHorizontalGridLines( QPainter *painter, int firstRow, int lastRow, bool drawHeaderLines ) const
{
  //horizontal lines
  if ( lastRow - firstRow < 1 && !drawHeaderLines )
  {
    return;
  }

  double cellBodyHeight = QgsComposerUtils::fontAscentMM( mContentFont );
  double halfGridStrokeWidth = ( mShowGrid ? mGridStrokeWidth : 0 ) / 2.0;
  double currentY = 0;
  currentY = halfGridStrokeWidth;
  if ( drawHeaderLines )
  {
    painter->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( mTableSize.width() - halfGridStrokeWidth, currentY ) );
    currentY += ( mShowGrid ? mGridStrokeWidth : 0 );
    currentY += ( QgsComposerUtils::fontAscentMM( mHeaderFont ) + 2 * mCellMargin );
  }
  for ( int row = firstRow; row < lastRow; ++row )
  {
    painter->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( mTableSize.width() - halfGridStrokeWidth, currentY ) );
    currentY += ( mShowGrid ? mGridStrokeWidth : 0 );
    double rowHeight = row < mTableContents.count() ? mMaxRowHeightMap[row + 1] : cellBodyHeight;
    currentY += ( rowHeight + 2 * mCellMargin );
  }
  painter->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( mTableSize.width() - halfGridStrokeWidth, currentY ) );
}

bool QgsComposerTableV2::textRequiresWrapping( const QString &text, double columnWidth, const QFont &font ) const
{
  if ( qgsDoubleNear( columnWidth, 0.0 ) || mWrapBehavior != WrapText )
    return false;

  QStringList multiLineSplit = text.split( '\n' );
  double currentTextWidth = 0;
  Q_FOREACH ( const QString &line, multiLineSplit )
  {
    currentTextWidth = std::max( currentTextWidth, QgsComposerUtils::textWidthMM( font, line ) );
  }

  return ( currentTextWidth > columnWidth );
}

QString QgsComposerTableV2::wrappedText( const QString &value, double columnWidth, const QFont &font ) const
{
  QStringList lines = value.split( '\n' );
  QStringList outLines;
  Q_FOREACH ( const QString &line, lines )
  {
    if ( textRequiresWrapping( line, columnWidth, font ) )
    {
      //first step is to identify words which must be on their own line (too long to fit)
      QStringList words = line.split( ' ' );
      QStringList linesToProcess;
      QString wordsInCurrentLine;
      Q_FOREACH ( const QString &word, words )
      {
        if ( textRequiresWrapping( word, columnWidth, font ) )
        {
          //too long to fit
          if ( !wordsInCurrentLine.isEmpty() )
            linesToProcess << wordsInCurrentLine;
          wordsInCurrentLine.clear();
          linesToProcess << word;
        }
        else
        {
          if ( !wordsInCurrentLine.isEmpty() )
            wordsInCurrentLine.append( ' ' );
          wordsInCurrentLine.append( word );
        }
      }
      if ( !wordsInCurrentLine.isEmpty() )
        linesToProcess << wordsInCurrentLine;

      Q_FOREACH ( const QString &line, linesToProcess )
      {
        QString remainingText = line;
        int lastPos = remainingText.lastIndexOf( ' ' );
        while ( lastPos > -1 )
        {
          if ( !textRequiresWrapping( remainingText.left( lastPos ), columnWidth, font ) )
          {
            outLines << remainingText.left( lastPos );
            remainingText = remainingText.mid( lastPos + 1 );
            lastPos = 0;
          }
          lastPos = remainingText.lastIndexOf( ' ', lastPos - 1 );
        }
        outLines << remainingText;
      }
    }
    else
    {
      outLines << line;
    }
  }

  return outLines.join( QStringLiteral( "\n" ) );
}

QColor QgsComposerTableV2::backgroundColor( int row, int column ) const
{
  QColor color = mBackgroundColor;
  if ( QgsComposerTableStyle *style = mCellStyles.value( OddColumns ) )
    if ( style->enabled && column % 2 == 0 )
      color = style->cellBackgroundColor;
  if ( QgsComposerTableStyle *style = mCellStyles.value( EvenColumns ) )
    if ( style->enabled && column % 2 == 1 )
      color = style->cellBackgroundColor;
  if ( QgsComposerTableStyle *style = mCellStyles.value( OddRows ) )
    if ( style->enabled && row % 2 == 0 )
      color = style->cellBackgroundColor;
  if ( QgsComposerTableStyle *style = mCellStyles.value( EvenRows ) )
    if ( style->enabled && row % 2 == 1 )
      color = style->cellBackgroundColor;
  if ( QgsComposerTableStyle *style = mCellStyles.value( FirstColumn ) )
    if ( style->enabled && column == 0 )
      color = style->cellBackgroundColor;
  if ( QgsComposerTableStyle *style = mCellStyles.value( LastColumn ) )
    if ( style->enabled && column == mColumns.count() - 1 )
      color = style->cellBackgroundColor;
  if ( QgsComposerTableStyle *style = mCellStyles.value( HeaderRow ) )
    if ( style->enabled && row == -1 )
      color = style->cellBackgroundColor;
  if ( QgsComposerTableStyle *style = mCellStyles.value( FirstRow ) )
    if ( style->enabled && row == 0 )
      color = style->cellBackgroundColor;
  if ( QgsComposerTableStyle *style = mCellStyles.value( LastRow ) )
    if ( style->enabled && row == mTableContents.count() - 1 )
      color = style->cellBackgroundColor;

  return color;
}

void QgsComposerTableV2::drawVerticalGridLines( QPainter *painter, const QMap<int, double> &maxWidthMap, int firstRow, int lastRow, bool hasHeader, bool mergeCells ) const
{
  //vertical lines
  if ( lastRow - firstRow < 1 && !hasHeader )
  {
    return;
  }

  //calculate height of table within frame
  double tableHeight = 0;
  if ( hasHeader )
  {
    tableHeight += ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 ) + mCellMargin * 2 + QgsComposerUtils::fontAscentMM( mHeaderFont );
  }
  tableHeight += ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 );
  double headerHeight = tableHeight;

  double cellBodyHeight = QgsComposerUtils::fontAscentMM( mContentFont );
  for ( int row = firstRow; row < lastRow; ++row )
  {
    double rowHeight = row < mTableContents.count() ? mMaxRowHeightMap[row + 1] : cellBodyHeight;
    tableHeight += rowHeight + ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 ) + mCellMargin * 2;
  }

  double halfGridStrokeWidth = ( mShowGrid && mVerticalGrid ? mGridStrokeWidth : 0 ) / 2.0;
  double currentX = halfGridStrokeWidth;
  painter->drawLine( QPointF( currentX, halfGridStrokeWidth ), QPointF( currentX, tableHeight - halfGridStrokeWidth ) );
  currentX += ( mShowGrid && mVerticalGrid ? mGridStrokeWidth : 0 );
  QMap<int, double>::const_iterator maxColWidthIt = maxWidthMap.constBegin();
  int col = 1;
  for ( ; maxColWidthIt != maxWidthMap.constEnd(); ++maxColWidthIt )
  {
    currentX += ( maxColWidthIt.value() + 2 * mCellMargin );
    if ( col == maxWidthMap.size() || !mergeCells )
    {
      painter->drawLine( QPointF( currentX, halfGridStrokeWidth ), QPointF( currentX, tableHeight - halfGridStrokeWidth ) );
    }
    else if ( hasHeader )
    {
      painter->drawLine( QPointF( currentX, halfGridStrokeWidth ), QPointF( currentX, headerHeight - halfGridStrokeWidth ) );
    }

    currentX += ( mShowGrid && mVerticalGrid ? mGridStrokeWidth : 0 );
    col++;
  }
}

void QgsComposerTableV2::recalculateTableSize()
{
  recalculateFrameSizes();

  //force recalculation of frame rects, so that they are set to the correct
  //fixed and minimum frame sizes
  recalculateFrameRects();
}

bool QgsComposerTableV2::contentsContainsRow( const QgsComposerTableContents &contents, const QgsComposerTableRow &row ) const
{
  return ( contents.indexOf( row ) >= 0 );
}

