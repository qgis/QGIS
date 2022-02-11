/***************************************************************************
                         qgslayouttable.cpp
                         ------------------
    begin                : November 2017
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

#include "qgsexpressioncontextutils.h"
#include "qgsexpressionutils.h"
#include "qgslayouttable.h"
#include "qgslayout.h"
#include "qgslayoututils.h"
#include "qgslayouttablecolumn.h"
#include "qgssymbollayerutils.h"
#include "qgslayoutframe.h"
#include "qgsfontutils.h"
#include "qgssettings.h"
#include "qgslayoutpagecollection.h"
#include "qgstextrenderer.h"

//
// QgsLayoutTableStyle
//

bool QgsLayoutTableStyle::writeXml( QDomElement &styleElem, QDomDocument &doc ) const
{
  Q_UNUSED( doc )
  styleElem.setAttribute( QStringLiteral( "cellBackgroundColor" ), QgsSymbolLayerUtils::encodeColor( cellBackgroundColor ) );
  styleElem.setAttribute( QStringLiteral( "enabled" ), enabled );
  return true;
}

bool QgsLayoutTableStyle::readXml( const QDomElement &styleElem )
{
  cellBackgroundColor = QgsSymbolLayerUtils::decodeColor( styleElem.attribute( QStringLiteral( "cellBackgroundColor" ), QStringLiteral( "255,255,255,255" ) ) );
  enabled = ( styleElem.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
  return true;
}


//
// QgsLayoutTable
//

QgsLayoutTable::QgsLayoutTable( QgsLayout *layout )
  : QgsLayoutMultiFrame( layout )
{
  initStyles();
}

QgsLayoutTable::~QgsLayoutTable()
{
  mColumns.clear();
  mSortColumns.clear();

  qDeleteAll( mCellStyles );
  mCellStyles.clear();
}

bool QgsLayoutTable::writePropertiesToElement( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( QStringLiteral( "cellMargin" ), QString::number( mCellMargin ) );
  elem.setAttribute( QStringLiteral( "emptyTableMode" ), QString::number( static_cast< int >( mEmptyTableMode ) ) );
  elem.setAttribute( QStringLiteral( "emptyTableMessage" ), mEmptyTableMessage );
  elem.setAttribute( QStringLiteral( "showEmptyRows" ), mShowEmptyRows );

  QDomElement headerElem = doc.createElement( QStringLiteral( "headerTextFormat" ) );
  const QDomElement headerTextElem = mHeaderTextFormat.writeXml( doc, context );
  headerElem.appendChild( headerTextElem );
  elem.appendChild( headerElem );
  elem.setAttribute( QStringLiteral( "headerHAlignment" ), QString::number( static_cast< int >( mHeaderHAlignment ) ) );
  elem.setAttribute( QStringLiteral( "headerMode" ), QString::number( static_cast< int >( mHeaderMode ) ) );

  QDomElement contentElem = doc.createElement( QStringLiteral( "contentTextFormat" ) );
  const QDomElement contentTextElem = mContentTextFormat.writeXml( doc, context );
  contentElem.appendChild( contentTextElem );
  elem.appendChild( contentElem );
  elem.setAttribute( QStringLiteral( "gridStrokeWidth" ), QString::number( mGridStrokeWidth ) );
  elem.setAttribute( QStringLiteral( "gridColor" ), QgsSymbolLayerUtils::encodeColor( mGridColor ) );
  elem.setAttribute( QStringLiteral( "horizontalGrid" ), mHorizontalGrid );
  elem.setAttribute( QStringLiteral( "verticalGrid" ), mVerticalGrid );
  elem.setAttribute( QStringLiteral( "showGrid" ), mShowGrid );
  elem.setAttribute( QStringLiteral( "backgroundColor" ), QgsSymbolLayerUtils::encodeColor( mBackgroundColor ) );
  elem.setAttribute( QStringLiteral( "wrapBehavior" ), QString::number( static_cast< int >( mWrapBehavior ) ) );

  // display columns
  QDomElement displayColumnsElem = doc.createElement( QStringLiteral( "displayColumns" ) );
  for ( const QgsLayoutTableColumn &column : std::as_const( mColumns ) )
  {
    QDomElement columnElem = doc.createElement( QStringLiteral( "column" ) );
    column.writeXml( columnElem, doc );
    displayColumnsElem.appendChild( columnElem );
  }
  elem.appendChild( displayColumnsElem );
  // sort columns
  QDomElement sortColumnsElem = doc.createElement( QStringLiteral( "sortColumns" ) );
  for ( const QgsLayoutTableColumn &column : std::as_const( mSortColumns ) )
  {
    QDomElement columnElem = doc.createElement( QStringLiteral( "column" ) );
    column.writeXml( columnElem, doc );
    sortColumnsElem.appendChild( columnElem );
  }
  elem.appendChild( sortColumnsElem );


  //cell styles
  QDomElement stylesElem = doc.createElement( QStringLiteral( "cellStyles" ) );
  QMap< CellStyleGroup, QString >::const_iterator it = mCellStyleNames.constBegin();
  for ( ; it != mCellStyleNames.constEnd(); ++it )
  {
    QString styleName = it.value();
    QDomElement styleElem = doc.createElement( styleName );
    QgsLayoutTableStyle *style = mCellStyles.value( it.key() );
    if ( style )
    {
      style->writeXml( styleElem, doc );
      stylesElem.appendChild( styleElem );
    }
  }
  elem.appendChild( stylesElem );
  return true;
}

bool QgsLayoutTable::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext &context )
{
  mEmptyTableMode = QgsLayoutTable::EmptyTableMode( itemElem.attribute( QStringLiteral( "emptyTableMode" ), QStringLiteral( "0" ) ).toInt() );
  mEmptyTableMessage = itemElem.attribute( QStringLiteral( "emptyTableMessage" ), tr( "No matching records" ) );
  mShowEmptyRows = itemElem.attribute( QStringLiteral( "showEmptyRows" ), QStringLiteral( "0" ) ).toInt();

  const QDomElement headerTextFormat = itemElem.firstChildElement( QStringLiteral( "headerTextFormat" ) );
  if ( !headerTextFormat.isNull() )
  {
    QDomNodeList textFormatNodeList = headerTextFormat.elementsByTagName( QStringLiteral( "text-style" ) );
    QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mHeaderTextFormat.readXml( textFormatElem, context );
  }
  else
  {
    QFont headerFont;
    if ( !QgsFontUtils::setFromXmlChildNode( headerFont, itemElem, QStringLiteral( "headerFontProperties" ) ) )
    {
      headerFont.fromString( itemElem.attribute( QStringLiteral( "headerFont" ), QString() ) );
    }
    QColor headerFontColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "headerFontColor" ), QStringLiteral( "0,0,0,255" ) ) );
    mHeaderTextFormat.setFont( headerFont );
    if ( headerFont.pointSizeF() > 0 )
    {
      mHeaderTextFormat.setSize( headerFont.pointSizeF() );
      mHeaderTextFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
    }
    else if ( headerFont.pixelSize() > 0 )
    {
      mHeaderTextFormat.setSize( headerFont.pixelSize() );
      mHeaderTextFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
    }
    mHeaderTextFormat.setColor( headerFontColor );
  }

  mHeaderHAlignment = QgsLayoutTable::HeaderHAlignment( itemElem.attribute( QStringLiteral( "headerHAlignment" ), QStringLiteral( "0" ) ).toInt() );
  mHeaderMode = QgsLayoutTable::HeaderMode( itemElem.attribute( QStringLiteral( "headerMode" ), QStringLiteral( "0" ) ).toInt() );

  const QDomElement contentTextFormat = itemElem.firstChildElement( QStringLiteral( "contentTextFormat" ) );
  if ( !contentTextFormat.isNull() )
  {
    QDomNodeList textFormatNodeList = contentTextFormat.elementsByTagName( QStringLiteral( "text-style" ) );
    QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mContentTextFormat.readXml( textFormatElem, context );
  }
  else
  {
    QFont contentFont;
    if ( !QgsFontUtils::setFromXmlChildNode( contentFont, itemElem, QStringLiteral( "contentFontProperties" ) ) )
    {
      contentFont.fromString( itemElem.attribute( QStringLiteral( "contentFont" ), QString() ) );
    }
    QColor contentFontColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "contentFontColor" ), QStringLiteral( "0,0,0,255" ) ) );
    mContentTextFormat.setFont( contentFont );
    if ( contentFont.pointSizeF() > 0 )
    {
      mContentTextFormat.setSize( contentFont.pointSizeF() );
      mContentTextFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
    }
    else if ( contentFont.pixelSize() > 0 )
    {
      mContentTextFormat.setSize( contentFont.pixelSize() );
      mContentTextFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
    }
    mContentTextFormat.setColor( contentFontColor );
  }

  mCellMargin = itemElem.attribute( QStringLiteral( "cellMargin" ), QStringLiteral( "1.0" ) ).toDouble();
  mGridStrokeWidth = itemElem.attribute( QStringLiteral( "gridStrokeWidth" ), QStringLiteral( "0.5" ) ).toDouble();
  mHorizontalGrid = itemElem.attribute( QStringLiteral( "horizontalGrid" ), QStringLiteral( "1" ) ).toInt();
  mVerticalGrid = itemElem.attribute( QStringLiteral( "verticalGrid" ), QStringLiteral( "1" ) ).toInt();
  mShowGrid = itemElem.attribute( QStringLiteral( "showGrid" ), QStringLiteral( "1" ) ).toInt();
  mGridColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "gridColor" ), QStringLiteral( "0,0,0,255" ) ) );
  mBackgroundColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "backgroundColor" ), QStringLiteral( "255,255,255,0" ) ) );
  mWrapBehavior = QgsLayoutTable::WrapBehavior( itemElem.attribute( QStringLiteral( "wrapBehavior" ), QStringLiteral( "0" ) ).toInt() );

  //restore display column specifications
  mColumns.clear();
  QDomNodeList columnsList = itemElem.elementsByTagName( QStringLiteral( "displayColumns" ) );
  if ( !columnsList.isEmpty() )
  {
    QDomElement columnsElem = columnsList.at( 0 ).toElement();
    QDomNodeList columnEntryList = columnsElem.elementsByTagName( QStringLiteral( "column" ) );
    for ( int i = 0; i < columnEntryList.size(); ++i )
    {
      QDomElement columnElem = columnEntryList.at( i ).toElement();
      QgsLayoutTableColumn column;
      column.readXml( columnElem );
      mColumns.append( column );
    }
  }
  // sort columns
  mSortColumns.clear();
  QDomNodeList sortColumnsList = itemElem.elementsByTagName( QStringLiteral( "sortColumns" ) );
  if ( !sortColumnsList.isEmpty() )
  {
    QDomElement columnsElem = sortColumnsList.at( 0 ).toElement();
    QDomNodeList columnEntryList = columnsElem.elementsByTagName( QStringLiteral( "column" ) );
    for ( int i = 0; i < columnEntryList.size(); ++i )
    {
      QDomElement columnElem = columnEntryList.at( i ).toElement();
      QgsLayoutTableColumn column;
      column.readXml( columnElem );
      mSortColumns.append( column );
    }
  }
  else
  {
    // backward compatibility for QGIS < 3.14
    // copy the display columns if sortByRank > 0 and then, sort them by rank
    Q_NOWARN_DEPRECATED_PUSH
    std::copy_if( mColumns.begin(), mColumns.end(), std::back_inserter( mSortColumns ), []( const QgsLayoutTableColumn & col ) {return col.sortByRank() > 0;} );
    std::sort( mSortColumns.begin(), mSortColumns.end(), []( const QgsLayoutTableColumn & a, const QgsLayoutTableColumn & b ) {return a.sortByRank() < b.sortByRank();} );
    Q_NOWARN_DEPRECATED_POP
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
        QgsLayoutTableStyle *style = mCellStyles.value( it.key() );
        if ( style )
          style->readXml( styleElem );
      }
    }
  }

  emit changed();
  return true;
}

QSizeF QgsLayoutTable::totalSize() const
{
  return mTableSize;
}

void QgsLayoutTable::refresh()
{
  QgsLayoutMultiFrame::refresh();
  refreshAttributes();
}

int QgsLayoutTable::rowsVisible( QgsRenderContext &context, double frameHeight, int firstRow, bool includeHeader, bool includeEmptyRows ) const
{
  //calculate header height
  double headerHeight = 0;
  if ( includeHeader )
  {
    headerHeight = mMaxRowHeightMap.value( 0 ) + 2 * ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 ) + 2 * mCellMargin;
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
    const QFontMetricsF emptyRowContentFontMetrics = QgsTextRenderer::fontMetrics( context, mContentTextFormat, QgsTextRenderer::FONT_WORKAROUND_SCALE );
    double rowHeight = ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 ) + 2 * mCellMargin + emptyRowContentFontMetrics.ascent() / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters ) / QgsTextRenderer::FONT_WORKAROUND_SCALE;
    currentRow += std::max( std::floor( contentHeight / rowHeight ), 0.0 );
  }

  return currentRow - firstRow - 1;
}

int QgsLayoutTable::rowsVisible( QgsRenderContext &context, int frameIndex, int firstRow, bool includeEmptyRows ) const
{
  //get frame extent
  if ( frameIndex >= frameCount() )
  {
    return 0;
  }
  QRectF frameExtent = frame( frameIndex )->extent();

  bool includeHeader = false;
  if ( ( mHeaderMode == QgsLayoutTable::FirstFrame && frameIndex < 1 )
       || ( mHeaderMode == QgsLayoutTable::AllFrames ) )
  {
    includeHeader = true;
  }
  return rowsVisible( context, frameExtent.height(), firstRow, includeHeader, includeEmptyRows );
}

QPair<int, int> QgsLayoutTable::rowRange( QgsRenderContext &context, const int frameIndex ) const
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
    rowsAlreadyShown += rowsVisible( context, idx, rowsAlreadyShown, false );
  }

  //using zero based indexes
  int firstVisible = std::min( rowsAlreadyShown, static_cast<int>( mTableContents.length() ) );
  int possibleRowsVisible = rowsVisible( context, frameIndex, rowsAlreadyShown, false );
  int lastVisible = std::min( firstVisible + possibleRowsVisible, static_cast<int>( mTableContents.length() ) );

  return qMakePair( firstVisible, lastVisible );
}

void QgsLayoutTable::render( QgsLayoutItemRenderContext &context, const QRectF &, const int frameIndex )
{
  bool emptyTable = mTableContents.length() == 0;
  if ( emptyTable && mEmptyTableMode == QgsLayoutTable::HideTable )
  {
    //empty table set to hide table mode, so don't draw anything
    return;
  }

  if ( !mLayout->renderContext().isPreviewRender() )
  {
    //exporting composition, so force an attribute refresh
    //we do this in case vector layer has changed via an external source (e.g., another database user)
    refreshAttributes();
  }

  const bool prevTextFormatScaleFlag = context.renderContext().testFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering );
  context.renderContext().setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering );

  //calculate which rows to show in this frame
  QPair< int, int > rowsToShow = rowRange( context.renderContext(), frameIndex );

  double gridSizeX = mShowGrid && mVerticalGrid ? mGridStrokeWidth : 0;
  double gridSizeY = mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0;
  double cellHeaderHeight = mMaxRowHeightMap[0] + 2 * mCellMargin;
  double cellBodyHeightForEmptyRows = QgsTextRenderer::fontMetrics( context.renderContext(), mContentTextFormat, QgsTextRenderer::FONT_WORKAROUND_SCALE ).ascent() / context.renderContext().convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters ) / QgsTextRenderer::FONT_WORKAROUND_SCALE + 2 * mCellMargin;
  QRectF cell;

  //calculate whether a header is required
  bool drawHeader = ( ( mHeaderMode == QgsLayoutTable::FirstFrame && frameIndex < 1 )
                      || ( mHeaderMode == QgsLayoutTable::AllFrames ) );
  //calculate whether drawing table contents is required
  bool drawContents = !( emptyTable && mEmptyTableMode == QgsLayoutTable::ShowMessage );

  int numberRowsToDraw = rowsToShow.second - rowsToShow.first;
  int numberEmptyRows = 0;
  if ( drawContents && mShowEmptyRows )
  {
    numberRowsToDraw = rowsVisible( context.renderContext(), frameIndex, rowsToShow.first, true );
    numberEmptyRows = numberRowsToDraw - rowsToShow.second + rowsToShow.first;
  }
  bool mergeCells = false;
  if ( emptyTable && mEmptyTableMode == QgsLayoutTable::ShowMessage )
  {
    //draw a merged row for the empty table message
    numberRowsToDraw++;
    rowsToShow.second++;
    mergeCells = true;
  }

  QPainter *p = context.renderContext().painter();
  QgsScopedQPainterState painterState( p );
  // painter is scaled to dots, so scale back to layout units
  p->scale( context.renderContext().scaleFactor(), context.renderContext().scaleFactor() );

  //draw the text
  p->setPen( Qt::SolidLine );

  double currentX = gridSizeX;
  double currentY = gridSizeY;
  if ( drawHeader )
  {
    //draw the headers
    int col = 0;
    for ( const QgsLayoutTableColumn &column : std::as_const( mColumns ) )
    {
      std::unique_ptr< QgsExpressionContextScope > headerCellScope = std::make_unique< QgsExpressionContextScope >();
      headerCellScope->setVariable( QStringLiteral( "column_number" ), col + 1, true );
      QgsExpressionContextScopePopper popper( context.renderContext().expressionContext(), headerCellScope.release() );

      const QgsTextFormat headerFormat = textFormatForHeader( col );
      //draw background
      p->save();
      p->setPen( Qt::NoPen );
      p->setBrush( backgroundColor( -1, col ) );
      p->drawRect( QRectF( currentX, currentY, mMaxColumnWidthMap[col] + 2 * mCellMargin, cellHeaderHeight ) );
      p->restore();

      currentX += mCellMargin;

      cell = QRectF( currentX, currentY, mMaxColumnWidthMap[col], cellHeaderHeight );

      //calculate alignment of header
      QgsTextRenderer::HAlignment headerAlign = QgsTextRenderer::AlignLeft;
      switch ( mHeaderHAlignment )
      {
        case FollowColumn:
          headerAlign = QgsTextRenderer::convertQtHAlignment( column.hAlignment() );
          break;
        case HeaderLeft:
          headerAlign = QgsTextRenderer::AlignLeft;
          break;
        case HeaderCenter:
          headerAlign = QgsTextRenderer::AlignCenter;
          break;
        case HeaderRight:
          headerAlign = QgsTextRenderer::AlignRight;
          break;
      }

      const QRectF textCell = QRectF( currentX, currentY + mCellMargin, mMaxColumnWidthMap[col], cellHeaderHeight - 2 * mCellMargin );

      const QStringList str = column.heading().split( '\n' );

      // scale to dots
      {
        QgsScopedRenderContextScaleToPixels scale( context.renderContext() );
        QgsTextRenderer::drawText( QRectF( textCell.left() * context.renderContext().scaleFactor(),
                                           textCell.top() * context.renderContext().scaleFactor(),
                                           textCell.width() * context.renderContext().scaleFactor(),
                                           textCell.height() * context.renderContext().scaleFactor() ), 0,
                                   headerAlign, str, context.renderContext(), headerFormat, true, QgsTextRenderer::AlignVCenter,
                                   mWrapBehavior == WrapText ? Qgis::TextRendererFlag::WrapLines : Qgis::TextRendererFlags()
                                 );
      }

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

      for ( const QgsLayoutTableColumn &column : std::as_const( mColumns ) )
      {
        ( void )column;
        const QRectF fullCell( currentX, currentY, mMaxColumnWidthMap[col] + 2 * mCellMargin, rowHeight );
        //draw background
        p->save();
        p->setPen( Qt::NoPen );
        p->setBrush( backgroundColor( row, col ) );
        p->drawRect( fullCell );
        p->restore();

        // currentY = gridSize;
        currentX += mCellMargin;

        QVariant cellContents = mTableContents.at( row ).at( col );
        const QString localizedString { QgsExpressionUtils::toLocalizedString( cellContents ) };
        const QStringList str = localizedString.split( '\n' );

        QgsTextFormat cellFormat = textFormatForCell( row, col );
        QgsExpressionContextScopePopper popper( context.renderContext().expressionContext(), scopeForCell( row, col ) );
        cellFormat.updateDataDefinedProperties( context.renderContext() );

        p->save();
        p->setClipRect( fullCell );
        const QRectF textCell = QRectF( currentX, currentY + mCellMargin, mMaxColumnWidthMap[col], rowHeight - 2 * mCellMargin );

        const QgsConditionalStyle style = conditionalCellStyle( row, col );
        QColor foreColor = cellFormat.color();
        if ( style.textColor().isValid() )
          foreColor = style.textColor();

        cellFormat.setColor( foreColor );

        // scale to dots
        {
          QgsScopedRenderContextScaleToPixels scale( context.renderContext() );
          QgsTextRenderer::drawText( QRectF( textCell.left() * context.renderContext().scaleFactor(),
                                             textCell.top() * context.renderContext().scaleFactor(),
                                             textCell.width() * context.renderContext().scaleFactor(),
                                             textCell.height() * context.renderContext().scaleFactor() ), 0,
                                     QgsTextRenderer::convertQtHAlignment( horizontalAlignmentForCell( row, col ) ), str, context.renderContext(), cellFormat, true,
                                     QgsTextRenderer::convertQtVAlignment( verticalAlignmentForCell( row, col ) ),
                                     mWrapBehavior == WrapText ? Qgis::TextRendererFlag::WrapLines : Qgis::TextRendererFlags() );
        }
        p->restore();

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
        p->drawRect( QRectF( gridSizeX, currentY, mTableSize.width() - 2 * gridSizeX, cellBodyHeightForEmptyRows ) );
      }
      else
      {
        for ( const QgsLayoutTableColumn &column : std::as_const( mColumns ) )
        {
          Q_UNUSED( column )

          //draw background

          //we use a bit of a hack here - since we don't want these extra blank rows to match the firstrow/lastrow rule, add 10000 to row number
          p->setBrush( backgroundColor( row + 10000, col ) );
          p->drawRect( QRectF( currentX, currentY, mMaxColumnWidthMap[col] + 2 * mCellMargin, cellBodyHeightForEmptyRows ) );

          // currentY = gridSize;
          currentX += mMaxColumnWidthMap[ col ] + 2 * mCellMargin;
          currentX += gridSizeX;
          col++;
        }
      }
      currentY += cellBodyHeightForEmptyRows + gridSizeY;
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
      drawHorizontalGridLines( context, rowsToShow.first, rowsToShow.second + numberEmptyRows, drawHeader );
    }
    if ( mVerticalGrid )
    {
      drawVerticalGridLines( context, mMaxColumnWidthMap, rowsToShow.first, rowsToShow.second + numberEmptyRows, drawHeader, mergeCells );
    }
  }

  //special case - no records and table is set to ShowMessage mode
  if ( emptyTable && mEmptyTableMode == QgsLayoutTable::ShowMessage )
  {
    double messageX = gridSizeX + mCellMargin;
    double messageY = gridSizeY + ( drawHeader ? cellHeaderHeight + gridSizeY : 0 );
    cell = QRectF( messageX, messageY, mTableSize.width() - messageX, cellBodyHeightForEmptyRows );

    // scale to dots
    {
      QgsScopedRenderContextScaleToPixels scale( context.renderContext() );
      QgsTextRenderer::drawText( QRectF( cell.left() * context.renderContext().scaleFactor(),
                                         cell.top() * context.renderContext().scaleFactor(),
                                         cell.width() * context.renderContext().scaleFactor(),
                                         cell.height() * context.renderContext().scaleFactor() ), 0,
                                 QgsTextRenderer::AlignCenter, QStringList() << mEmptyTableMessage, context.renderContext(), mContentTextFormat, true, QgsTextRenderer::AlignVCenter );
    }
  }

  context.renderContext().setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, prevTextFormatScaleFlag );
}

void QgsLayoutTable::setCellMargin( const double margin )
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

void QgsLayoutTable::setEmptyTableBehavior( const QgsLayoutTable::EmptyTableMode mode )
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

void QgsLayoutTable::setEmptyTableMessage( const QString &message )
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

void QgsLayoutTable::setShowEmptyRows( const bool showEmpty )
{
  if ( showEmpty == mShowEmptyRows )
  {
    return;
  }

  mShowEmptyRows = showEmpty;
  update();
  emit changed();
}

void QgsLayoutTable::setHeaderFont( const QFont &font )
{
  mHeaderTextFormat.setFont( font );
  if ( font.pointSizeF() > 0 )
  {
    mHeaderTextFormat.setSize( font.pointSizeF() );
    mHeaderTextFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
  }
  else if ( font.pixelSize() > 0 )
  {
    mHeaderTextFormat.setSize( font.pixelSize() );
    mHeaderTextFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
  }

  //since font attributes have changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

QFont QgsLayoutTable::headerFont() const
{
  return mHeaderTextFormat.toQFont();
}

void QgsLayoutTable::setHeaderFontColor( const QColor &color )
{
  if ( color == mHeaderTextFormat.color() )
  {
    return;
  }

  mHeaderTextFormat.setColor( color );
  update();

  emit changed();
}

QColor QgsLayoutTable::headerFontColor() const
{
  return mHeaderTextFormat.color();
}

void QgsLayoutTable::setHeaderTextFormat( const QgsTextFormat &format )
{
  mHeaderTextFormat = format;

  //since font attributes have changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

QgsTextFormat QgsLayoutTable::headerTextFormat() const
{
  return mHeaderTextFormat;
}

void QgsLayoutTable::setHeaderHAlignment( const QgsLayoutTable::HeaderHAlignment alignment )
{
  if ( alignment == mHeaderHAlignment )
  {
    return;
  }

  mHeaderHAlignment = alignment;
  update();

  emit changed();
}

void QgsLayoutTable::setHeaderMode( const QgsLayoutTable::HeaderMode mode )
{
  if ( mode == mHeaderMode )
  {
    return;
  }

  mHeaderMode = mode;
  recalculateTableSize();

  emit changed();
}

void QgsLayoutTable::setContentFont( const QFont &font )
{
  mContentTextFormat.setFont( font );
  if ( font.pointSizeF() > 0 )
  {
    mContentTextFormat.setSize( font.pointSizeF() );
    mContentTextFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
  }
  else if ( font.pixelSize() > 0 )
  {
    mContentTextFormat.setSize( font.pixelSize() );
    mContentTextFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
  }

  //since font attributes have changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

QFont QgsLayoutTable::contentFont() const
{
  return mContentTextFormat.toQFont();
}

void QgsLayoutTable::setContentFontColor( const QColor &color )
{
  if ( color == mContentTextFormat.color() )
  {
    return;
  }

  mContentTextFormat.setColor( color );
  update();

  emit changed();
}

QColor QgsLayoutTable::contentFontColor() const
{
  return mContentTextFormat.color();
}

void QgsLayoutTable::setContentTextFormat( const QgsTextFormat &format )
{
  mContentTextFormat = format;

  //since spacing has changed, we need to recalculate the table size
  recalculateTableSize();

  emit changed();
}

QgsTextFormat QgsLayoutTable::contentTextFormat() const
{
  return mContentTextFormat;
}

void QgsLayoutTable::setShowGrid( const bool showGrid )
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

void QgsLayoutTable::setGridStrokeWidth( const double width )
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

void QgsLayoutTable::setGridColor( const QColor &color )
{
  if ( color == mGridColor )
  {
    return;
  }

  mGridColor = color;
  update();

  emit changed();
}

void QgsLayoutTable::setHorizontalGrid( const bool horizontalGrid )
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

void QgsLayoutTable::setVerticalGrid( const bool verticalGrid )
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

void QgsLayoutTable::setBackgroundColor( const QColor &color )
{
  if ( color == mBackgroundColor )
  {
    return;
  }

  mBackgroundColor = color;
  update();

  emit changed();
}

void QgsLayoutTable::setWrapBehavior( QgsLayoutTable::WrapBehavior behavior )
{
  if ( behavior == mWrapBehavior )
  {
    return;
  }

  mWrapBehavior = behavior;
  recalculateTableSize();

  emit changed();
}

void QgsLayoutTable::setColumns( const QgsLayoutTableColumns &columns )
{
  //remove existing columns
  mColumns = columns;

  // backward compatibility
  // test if sorting is provided with the columns and call setSortColumns in such case
  QgsLayoutTableSortColumns newSortColumns;
  Q_NOWARN_DEPRECATED_PUSH
  std::copy_if( mColumns.begin(), mColumns.end(), std::back_inserter( newSortColumns ), []( const QgsLayoutTableColumn & col ) {return col.sortByRank() > 0;} );
  if ( !newSortColumns.isEmpty() )
  {
    std::sort( newSortColumns.begin(), newSortColumns.end(), []( const QgsLayoutTableColumn & a, const QgsLayoutTableColumn & b ) {return a.sortByRank() < b.sortByRank();} );
    setSortColumns( newSortColumns );
  }
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutTable::setSortColumns( const QgsLayoutTableSortColumns &sortColumns )
{
  mSortColumns = sortColumns;
}

void QgsLayoutTable::setCellStyle( QgsLayoutTable::CellStyleGroup group, const QgsLayoutTableStyle &style )
{
  if ( mCellStyles.contains( group ) )
    delete mCellStyles.take( group );

  mCellStyles.insert( group, new QgsLayoutTableStyle( style ) );
}

const QgsLayoutTableStyle *QgsLayoutTable::cellStyle( QgsLayoutTable::CellStyleGroup group ) const
{
  if ( !mCellStyles.contains( group ) )
    return nullptr;

  return mCellStyles.value( group );
}

QMap<int, QString> QgsLayoutTable::headerLabels() const
{
  QMap<int, QString> headers;

  int i = 0;
  for ( const QgsLayoutTableColumn &col : std::as_const( mColumns ) )
  {
    headers.insert( i, col.heading() );
    i++;
  }
  return headers;
}

QgsExpressionContextScope *QgsLayoutTable::scopeForCell( int row, int column ) const
{
  std::unique_ptr< QgsExpressionContextScope > cellScope = std::make_unique< QgsExpressionContextScope >();
  cellScope->setVariable( QStringLiteral( "row_number" ), row + 1, true );
  cellScope->setVariable( QStringLiteral( "column_number" ), column + 1, true );
  return cellScope.release();
}

QgsConditionalStyle QgsLayoutTable::conditionalCellStyle( int, int ) const
{
  return QgsConditionalStyle();
}

QSizeF QgsLayoutTable::fixedFrameSize( const int frameIndex ) const
{
  Q_UNUSED( frameIndex )
  return QSizeF( mTableSize.width(), 0 );
}

QSizeF QgsLayoutTable::minFrameSize( const int frameIndex ) const
{
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering );

  double height = 0;
  if ( ( mHeaderMode == QgsLayoutTable::FirstFrame && frameIndex < 1 )
       || ( mHeaderMode == QgsLayoutTable::AllFrames ) )
  {
    //header required, force frame to be high enough for header
    for ( int col = 0; col < mColumns.size(); ++ col )
    {
      height = std::max( height, 2 * ( mShowGrid ? mGridStrokeWidth : 0 ) + 2 * mCellMargin + QgsTextRenderer::fontMetrics( context, textFormatForHeader( col ), QgsTextRenderer::FONT_WORKAROUND_SCALE ).ascent() / QgsTextRenderer::FONT_WORKAROUND_SCALE / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters ) );
    }
  }
  return QSizeF( 0, height );
}

void QgsLayoutTable::refreshAttributes()
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

void QgsLayoutTable::recalculateFrameSizes()
{
  mTableSize = QSizeF( totalWidth(), totalHeight() );
  QgsLayoutMultiFrame::recalculateFrameSizes();
}

void QgsLayoutTable::initStyles()
{
  mCellStyles.insert( OddColumns, new QgsLayoutTableStyle() );
  mCellStyles.insert( EvenColumns, new QgsLayoutTableStyle() );
  mCellStyles.insert( OddRows, new QgsLayoutTableStyle() );
  mCellStyles.insert( EvenRows, new QgsLayoutTableStyle() );
  mCellStyles.insert( FirstColumn, new QgsLayoutTableStyle() );
  mCellStyles.insert( LastColumn, new QgsLayoutTableStyle() );
  mCellStyles.insert( HeaderRow, new QgsLayoutTableStyle() );
  mCellStyles.insert( FirstRow, new QgsLayoutTableStyle() );
  mCellStyles.insert( LastRow, new QgsLayoutTableStyle() );

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

bool QgsLayoutTable::calculateMaxColumnWidths()
{
  mMaxColumnWidthMap.clear();

  //total number of cells (rows + 1 for header)
  int cols = mColumns.count();
  int cells = cols * ( mTableContents.count() + 1 );
  QVector< double > widths( cells );

  double currentCellTextWidth;

  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering );

  //first, go through all the column headers and calculate the sizes
  int i = 0;
  for ( const QgsLayoutTableColumn &col : std::as_const( mColumns ) )
  {
    if ( col.width() > 0 )
    {
      //column has manually specified width
      widths[i] = col.width();
    }
    else if ( mHeaderMode != QgsLayoutTable::NoHeaders )
    {
      std::unique_ptr< QgsExpressionContextScope > headerCellScope = std::make_unique< QgsExpressionContextScope >();
      headerCellScope->setVariable( QStringLiteral( "column_number" ), i + 1, true );
      QgsExpressionContextScopePopper popper( context.expressionContext(), headerCellScope.release() );

      //column width set to automatic, so check content size
      const QStringList multiLineSplit = col.heading().split( '\n' );
      currentCellTextWidth = QgsTextRenderer::textWidth( context, textFormatForHeader( i ), multiLineSplit ) / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
      widths[i] = currentCellTextWidth;
    }
    else
    {
      widths[i] = 0.0;
    }
    i++;
  }

  //next, go through all the table contents and calculate the sizes
  QgsLayoutTableContents::const_iterator rowIt = mTableContents.constBegin();
  int row = 1;
  for ( ; rowIt != mTableContents.constEnd(); ++rowIt )
  {
    QgsLayoutTableRow::const_iterator colIt = rowIt->constBegin();
    int col = 0;
    for ( ; colIt != rowIt->constEnd(); ++colIt )
    {
      if ( mColumns.at( col ).width() <= 0 )
      {
        //column width set to automatic, so check content size
        const QStringList multiLineSplit = QgsExpressionUtils::toLocalizedString( *colIt ).split( '\n' );

        QgsTextFormat cellFormat = textFormatForCell( row - 1, col );
        QgsExpressionContextScopePopper popper( context.expressionContext(), scopeForCell( row - 1, col ) );
        cellFormat.updateDataDefinedProperties( context );

        currentCellTextWidth = QgsTextRenderer::textWidth( context, cellFormat, multiLineSplit ) / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
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

bool QgsLayoutTable::calculateMaxRowHeights()
{
  mMaxRowHeightMap.clear();

  //total number of cells (rows + 1 for header)
  int cols = mColumns.count();
  int cells = cols * ( mTableContents.count() + 1 );
  QVector< double > heights( cells );

  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering );

  //first, go through all the column headers and calculate the sizes
  int i = 0;
  for ( const QgsLayoutTableColumn &col : std::as_const( mColumns ) )
  {
    std::unique_ptr< QgsExpressionContextScope > headerCellScope = std::make_unique< QgsExpressionContextScope >();
    headerCellScope->setVariable( QStringLiteral( "column_number" ), i + 1, true );
    QgsExpressionContextScopePopper popper( context.expressionContext(), headerCellScope.release() );

    const QgsTextFormat cellFormat = textFormatForHeader( i );
    const double headerDescentMm = QgsTextRenderer::fontMetrics( context, cellFormat, QgsTextRenderer::FONT_WORKAROUND_SCALE ).descent() / QgsTextRenderer::FONT_WORKAROUND_SCALE  / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
    //height
    if ( mHeaderMode == QgsLayoutTable::NoHeaders )
    {
      heights[i] = 0;
    }
    else
    {
      heights[i] = QgsTextRenderer::textHeight( context,
                   cellFormat,
                   QStringList() << col.heading(), QgsTextRenderer::Rect,
                   nullptr,
                   mWrapBehavior == WrapText ? Qgis::TextRendererFlag::WrapLines : Qgis::TextRendererFlags(),
                   context.convertToPainterUnits( mColumns.at( i ).width(), QgsUnitTypes::RenderMillimeters )
                                              )
                   / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters )
                   - headerDescentMm;
    }
    i++;
  }

  //next, go through all the table contents and calculate the sizes
  QgsLayoutTableContents::const_iterator rowIt = mTableContents.constBegin();
  int row = 1;
  for ( ; rowIt != mTableContents.constEnd(); ++rowIt )
  {
    QgsLayoutTableRow::const_iterator colIt = rowIt->constBegin();
    int i = 0;
    for ( ; colIt != rowIt->constEnd(); ++colIt )
    {
      QgsTextFormat cellFormat = textFormatForCell( row - 1, i );
      QgsExpressionContextScopePopper popper( context.expressionContext(), scopeForCell( row - 1, i ) );
      cellFormat.updateDataDefinedProperties( context );
      const double contentDescentMm = QgsTextRenderer::fontMetrics( context, cellFormat, QgsTextRenderer::FONT_WORKAROUND_SCALE ).descent() / QgsTextRenderer::FONT_WORKAROUND_SCALE  / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
      const QString localizedString { QgsExpressionUtils::toLocalizedString( *colIt ) };

      heights[ row * cols + i ] = QgsTextRenderer::textHeight( context,
                                  cellFormat,
                                  QStringList() << localizedString.split( '\n' ),
                                  QgsTextRenderer::Rect,
                                  nullptr,
                                  mWrapBehavior == WrapText ? Qgis::TextRendererFlag::WrapLines : Qgis::TextRendererFlags(),
                                  context.convertToPainterUnits( mColumns.at( i ).width(), QgsUnitTypes::RenderMillimeters )
                                                             ) / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters ) - contentDescentMm;

      i++;
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

double QgsLayoutTable::totalWidth()
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

double QgsLayoutTable::totalHeight()
{
  //check how much space each row needs
  if ( !calculateMaxRowHeights() )
  {
    return 0;
  }

  double height = 0;

  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering );

  //loop through all existing frames to calculate how many rows are visible in each
  //as the entire height of a frame may not be utilized for content rows
  int rowsAlreadyShown = 0;
  int numberExistingFrames = frameCount();
  int rowsVisibleInLastFrame = 0;
  double heightOfLastFrame = 0;
  for ( int idx = 0; idx < numberExistingFrames; ++idx )
  {
    bool hasHeader = ( ( mHeaderMode == QgsLayoutTable::FirstFrame && idx == 0 )
                       || ( mHeaderMode == QgsLayoutTable::AllFrames ) );
    heightOfLastFrame = frame( idx )->rect().height();
    rowsVisibleInLastFrame = rowsVisible( context, heightOfLastFrame, rowsAlreadyShown, hasHeader, false );
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

  if ( mResizeMode == QgsLayoutMultiFrame::ExtendToNextPage )
  {
    QgsLayoutItemPage *page = mLayout->pageCollection()->page( mLayout->pageCollection()->pageCount() - 1 );
    if ( page )
      heightOfLastFrame = page->sizeWithUnits().height();
  }

  bool hasHeader = ( ( mHeaderMode == QgsLayoutTable::FirstFrame && numberExistingFrames < 1 )
                     || ( mHeaderMode == QgsLayoutTable::AllFrames ) );

  int numberFramesMissing = 0;
  while ( remainingRows > 0 )
  {
    numberFramesMissing++;

    rowsVisibleInLastFrame = rowsVisible( context, heightOfLastFrame, rowsAlreadyShown, hasHeader, false );
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

void QgsLayoutTable::drawHorizontalGridLines( QgsLayoutItemRenderContext &context, int firstRow, int lastRow, bool drawHeaderLines ) const
{
  //horizontal lines
  if ( lastRow - firstRow < 1 && !drawHeaderLines )
  {
    return;
  }

  QPainter *painter = context.renderContext().painter();

  double cellBodyHeightForEmptyRows = QgsTextRenderer::fontMetrics( context.renderContext(), mContentTextFormat, QgsTextRenderer::FONT_WORKAROUND_SCALE ).ascent() / QgsTextRenderer::FONT_WORKAROUND_SCALE / context.renderContext().convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
  double halfGridStrokeWidth = ( mShowGrid ? mGridStrokeWidth : 0 ) / 2.0;
  double currentY = 0;
  currentY = halfGridStrokeWidth;
  if ( drawHeaderLines )
  {
    painter->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( mTableSize.width() - halfGridStrokeWidth, currentY ) );
    currentY += ( mShowGrid ? mGridStrokeWidth : 0 );
    currentY += mMaxRowHeightMap[0] + 2 * mCellMargin;
  }
  for ( int row = firstRow; row < lastRow; ++row )
  {
    painter->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( mTableSize.width() - halfGridStrokeWidth, currentY ) );
    currentY += ( mShowGrid ? mGridStrokeWidth : 0 );
    double rowHeight = row < mTableContents.count() ? mMaxRowHeightMap[row + 1] : cellBodyHeightForEmptyRows;
    currentY += ( rowHeight + 2 * mCellMargin );
  }
  painter->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( mTableSize.width() - halfGridStrokeWidth, currentY ) );
}

QColor QgsLayoutTable::backgroundColor( int row, int column ) const
{
  QColor color = mBackgroundColor;
  if ( QgsLayoutTableStyle *style = mCellStyles.value( OddColumns ) )
    if ( style->enabled && column % 2 == 0 )
      color = style->cellBackgroundColor;
  if ( QgsLayoutTableStyle *style = mCellStyles.value( EvenColumns ) )
    if ( style->enabled && column % 2 == 1 )
      color = style->cellBackgroundColor;
  if ( QgsLayoutTableStyle *style = mCellStyles.value( OddRows ) )
    if ( style->enabled && row % 2 == 0 )
      color = style->cellBackgroundColor;
  if ( QgsLayoutTableStyle *style = mCellStyles.value( EvenRows ) )
    if ( style->enabled && row % 2 == 1 )
      color = style->cellBackgroundColor;
  if ( QgsLayoutTableStyle *style = mCellStyles.value( FirstColumn ) )
    if ( style->enabled && column == 0 )
      color = style->cellBackgroundColor;
  if ( QgsLayoutTableStyle *style = mCellStyles.value( LastColumn ) )
    if ( style->enabled && column == mColumns.count() - 1 )
      color = style->cellBackgroundColor;
  if ( QgsLayoutTableStyle *style = mCellStyles.value( HeaderRow ) )
    if ( style->enabled && row == -1 )
      color = style->cellBackgroundColor;
  if ( QgsLayoutTableStyle *style = mCellStyles.value( FirstRow ) )
    if ( style->enabled && row == 0 )
      color = style->cellBackgroundColor;
  if ( QgsLayoutTableStyle *style = mCellStyles.value( LastRow ) )
    if ( style->enabled && row == mTableContents.count() - 1 )
      color = style->cellBackgroundColor;

  if ( row >= 0 )
  {
    QgsConditionalStyle conditionalStyle = conditionalCellStyle( row, column );
    if ( conditionalStyle.backgroundColor().isValid() )
      color = conditionalStyle.backgroundColor();
  }

  return color;
}

void QgsLayoutTable::drawVerticalGridLines( QgsLayoutItemRenderContext &context, const QMap<int, double> &maxWidthMap, int firstRow, int lastRow, bool hasHeader, bool mergeCells ) const
{
  //vertical lines
  if ( lastRow - firstRow < 1 && !hasHeader )
  {
    return;
  }

  QPainter *painter = context.renderContext().painter();

  //calculate height of table within frame
  double tableHeight = 0;
  if ( hasHeader )
  {
    tableHeight += ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 ) + mCellMargin * 2 + mMaxRowHeightMap[0];
  }
  tableHeight += ( mShowGrid && mHorizontalGrid ? mGridStrokeWidth : 0 );
  double headerHeight = tableHeight;

  double cellBodyHeightForEmptyRows = QgsTextRenderer::fontMetrics( context.renderContext(), mContentTextFormat, QgsTextRenderer::FONT_WORKAROUND_SCALE ).ascent() / QgsTextRenderer::FONT_WORKAROUND_SCALE / context.renderContext().convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
  for ( int row = firstRow; row < lastRow; ++row )
  {
    double rowHeight = row < mTableContents.count() ? mMaxRowHeightMap[row + 1] : cellBodyHeightForEmptyRows;
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

void QgsLayoutTable::recalculateTableSize()
{
  recalculateFrameSizes();

  //force recalculation of frame rects, so that they are set to the correct
  //fixed and minimum frame sizes
  recalculateFrameRects();
}

bool QgsLayoutTable::contentsContainsRow( const QgsLayoutTableContents &contents, const QgsLayoutTableRow &row ) const
{
  return ( contents.indexOf( row ) >= 0 );
}

QgsTextFormat QgsLayoutTable::textFormatForCell( int, int ) const
{
  return mContentTextFormat;
}

QgsTextFormat QgsLayoutTable::textFormatForHeader( int ) const
{
  return mHeaderTextFormat;
}

Qt::Alignment QgsLayoutTable::horizontalAlignmentForCell( int, int column ) const
{
  return mColumns.value( column ).hAlignment();
}

Qt::Alignment QgsLayoutTable::verticalAlignmentForCell( int, int column ) const
{
  return mColumns.value( column ).vAlignment();
}

