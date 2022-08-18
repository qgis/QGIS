/***************************************************************************
                         qgslayoutitemmanualtable.cpp
                         ---------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgslayoutitemmanualtable.h"
#include "qgsconditionalstyle.h"
#include "qgslayoutitemregistry.h"
#include "qgslayouttablecolumn.h"
#include "qgsnumericformat.h"
#include "qgsxmlutils.h"
#include "qgsexpressioncontextutils.h"

//
// QgsLayoutItemManualTable
//

QgsLayoutItemManualTable::QgsLayoutItemManualTable( QgsLayout *layout )
  : QgsLayoutTable( layout )
{
  setHeaderMode( NoHeaders );
  refreshAttributes();
}

QgsLayoutItemManualTable::~QgsLayoutItemManualTable()
{
}

int QgsLayoutItemManualTable::type() const
{
  return QgsLayoutItemRegistry::LayoutManualTable;
}

QIcon QgsLayoutItemManualTable::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemTable.svg" ) );
}

QgsLayoutItemManualTable *QgsLayoutItemManualTable::create( QgsLayout *layout )
{
  return new QgsLayoutItemManualTable( layout );
}

QString QgsLayoutItemManualTable::displayName() const
{
  return tr( "<Table frame>" );
}

bool QgsLayoutItemManualTable::getTableContents( QgsLayoutTableContents &contents )
{
  contents.clear();

  QgsNumericFormatContext numericContext;

  QgsExpressionContext context = createExpressionContext();

  int rowNumber = 0;
  for ( const QgsTableRow &row : std::as_const( mContents ) )
  {
    QgsLayoutTableRow currentRow;

    for ( int columnNumber = 0; columnNumber < mColumns.count(); ++columnNumber )
    {
      if ( columnNumber < row.count() )
      {
        QVariant cellContent = row.at( columnNumber ).content();

        if ( cellContent.userType() == QMetaType::type( "QgsProperty" ) )
        {
          // expression based cell content, evaluate now
          QgsExpressionContextScopePopper popper( context, scopeForCell( rowNumber, columnNumber ) );
          cellContent = cellContent.value< QgsProperty >().value( context );
        }

        if ( row.at( columnNumber ).numericFormat() )
          currentRow << row.at( columnNumber ).numericFormat()->formatDouble( cellContent.toDouble(), numericContext );
        else
          currentRow << cellContent.toString();
      }
      else
      {
        currentRow << QString();
      }
    }
    contents << currentRow;
    rowNumber++;
  }

  recalculateTableSize();
  return true;
}

QgsConditionalStyle QgsLayoutItemManualTable::conditionalCellStyle( int row, int column ) const
{
  if ( row < 0 || row >= mContents.size() )
    return QgsConditionalStyle();

  const QgsTableRow &rowContents = mContents[ row ];
  if ( column < 0 || column > rowContents.size() )
    return QgsConditionalStyle();

  const QgsTableCell &c = rowContents[ column ];
  QgsConditionalStyle res;
  if ( c.foregroundColor().isValid() )
    res.setTextColor( c.foregroundColor() );
  if ( c.backgroundColor().isValid() )
    res.setBackgroundColor( c.backgroundColor() );

  return res;
}

void QgsLayoutItemManualTable::setTableContents( const QgsTableContents &contents )
{
  mContents = contents;

  refreshColumns();
  refreshAttributes();
}

QgsTableContents QgsLayoutItemManualTable::tableContents() const
{
  return mContents;
}

void QgsLayoutItemManualTable::setRowHeights( const QList<double> &heights )
{
  mRowHeights = heights;

  refreshAttributes();
}

void QgsLayoutItemManualTable::setColumnWidths( const QList<double> &widths )
{
  mColumnWidths = widths;

  refreshColumns();
  refreshAttributes();
}

bool QgsLayoutItemManualTable::includeTableHeader() const
{
  return mIncludeHeader;
}

void QgsLayoutItemManualTable::setIncludeTableHeader( bool included )
{
  mIncludeHeader = included;

  if ( !mIncludeHeader )
    setHeaderMode( NoHeaders );
  else
    setHeaderMode( AllFrames );
  refreshColumns();
  refreshAttributes();
}

QgsLayoutTableColumns &QgsLayoutItemManualTable::headers()
{
  return mHeaders;
}

void QgsLayoutItemManualTable::setHeaders( const QgsLayoutTableColumns &headers )
{
  mHeaders.clear();

  mHeaders.append( headers );
  refreshColumns();
  refreshAttributes();
}

bool QgsLayoutItemManualTable::writePropertiesToElement( QDomElement &tableElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  if ( !QgsLayoutTable::writePropertiesToElement( tableElem, doc, context ) )
    return false;

  tableElem.setAttribute( QStringLiteral( "includeHeader" ), mIncludeHeader ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  //headers
  QDomElement headersElem = doc.createElement( QStringLiteral( "headers" ) );
  for ( const QgsLayoutTableColumn &header : std::as_const( mHeaders ) )
  {
    QDomElement headerElem = doc.createElement( QStringLiteral( "header" ) );
    header.writeXml( headerElem, doc );
    headersElem.appendChild( headerElem );
  }
  tableElem.appendChild( headersElem );

  QDomElement contentsElement = doc.createElement( QStringLiteral( "contents" ) );
  for ( const QgsTableRow &row : mContents )
  {
    QDomElement rowElement = doc.createElement( QStringLiteral( "row" ) );
    for ( int i = 0; i < mColumns.count(); ++i )
    {
      if ( i < row.count() )
      {
        rowElement.appendChild( QgsXmlUtils::writeVariant( row.at( i ).properties( context ), doc ) );
      }
    }
    contentsElement.appendChild( rowElement );
  }
  tableElem.appendChild( contentsElement );

  QDomElement rowHeightsElement = doc.createElement( QStringLiteral( "rowHeights" ) );
  for ( double height : mRowHeights )
  {
    QDomElement rowElement = doc.createElement( QStringLiteral( "row" ) );
    rowElement.setAttribute( QStringLiteral( "height" ), height );
    rowHeightsElement.appendChild( rowElement );
  }
  tableElem.appendChild( rowHeightsElement );

  QDomElement columnWidthsElement = doc.createElement( QStringLiteral( "columnWidths" ) );
  for ( double width : mColumnWidths )
  {
    QDomElement columnElement = doc.createElement( QStringLiteral( "column" ) );
    columnElement.setAttribute( QStringLiteral( "width" ), width );
    columnWidthsElement.appendChild( columnElement );
  }
  tableElem.appendChild( columnWidthsElement );

  return true;
}

bool QgsLayoutItemManualTable::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  if ( !QgsLayoutTable::readPropertiesFromElement( itemElem, doc, context ) )
    return false;

  mIncludeHeader = itemElem.attribute( QStringLiteral( "includeHeader" ) ).toInt();
  //restore header specifications
  mHeaders.clear();
  QDomNodeList headersList = itemElem.elementsByTagName( QStringLiteral( "headers" ) );
  if ( !headersList.isEmpty() )
  {
    QDomElement headersElem = headersList.at( 0 ).toElement();
    QDomNodeList headerEntryList = headersElem.elementsByTagName( QStringLiteral( "header" ) );
    for ( int i = 0; i < headerEntryList.size(); ++i )
    {
      QDomElement headerElem = headerEntryList.at( i ).toElement();
      QgsLayoutTableColumn header;
      header.readXml( headerElem );
      mHeaders.append( header );
    }
  }

  mRowHeights.clear();
  const QDomNodeList rowHeightNodeList = itemElem.firstChildElement( QStringLiteral( "rowHeights" ) ).childNodes();
  mRowHeights.reserve( rowHeightNodeList.size() );
  for ( int r = 0; r < rowHeightNodeList.size(); ++r )
  {
    const QDomElement rowHeightElement = rowHeightNodeList.at( r ).toElement();
    mRowHeights.append( rowHeightElement.attribute( QStringLiteral( "height" ) ).toDouble() );
  }

  mColumnWidths.clear();
  const QDomNodeList columnWidthNodeList = itemElem.firstChildElement( QStringLiteral( "columnWidths" ) ).childNodes();
  mColumnWidths.reserve( columnWidthNodeList.size() );
  for ( int r = 0; r < columnWidthNodeList.size(); ++r )
  {
    const QDomElement columnWidthElement = columnWidthNodeList.at( r ).toElement();
    mColumnWidths.append( columnWidthElement.attribute( QStringLiteral( "width" ) ).toDouble() );
  }

  QgsTableContents newContents;
  const QDomElement contentsElement = itemElem.firstChildElement( QStringLiteral( "contents" ) );
  const QDomNodeList rowNodeList = contentsElement.childNodes();
  newContents.reserve( rowNodeList.size() );
  for ( int r = 0; r < rowNodeList.size(); ++r )
  {
    QgsTableRow row;
    const QDomElement rowElement = rowNodeList.at( r ).toElement();
    const QDomNodeList cellNodeList = rowElement.childNodes();
    row.reserve( cellNodeList.size() );
    for ( int c = 0; c < cellNodeList.size(); ++c )
    {
      const QDomElement cellElement = cellNodeList.at( c ).toElement();
      QgsTableCell newCell;
      newCell.setProperties( QgsXmlUtils::readVariant( cellElement ).toMap(), context );
      row << newCell;
    }
    newContents << row;
  }
  setTableContents( newContents );

  emit changed();
  return true;
}

bool QgsLayoutItemManualTable::calculateMaxRowHeights()
{
  if ( !QgsLayoutTable::calculateMaxRowHeights() )
    return false;

  QMap<int, double> newHeights;
  for ( auto it = mMaxRowHeightMap.constBegin(); it != mMaxRowHeightMap.constEnd(); ++it )
  {
    // first row in mMaxRowHeightMap corresponds to header, which we ignore here
    const int row = it.key() - 1;
    const double presetHeight = mRowHeights.value( row );
    double thisRowHeight = it.value();
    if ( presetHeight > 0 )
      thisRowHeight = presetHeight;
    newHeights.insert( row + 1, thisRowHeight );
  }
  mMaxRowHeightMap = newHeights;
  return true;
}

QgsTextFormat QgsLayoutItemManualTable::textFormatForHeader( int column ) const
{
//  if ( mHeaders.value( column ).)
  return QgsLayoutTable::textFormatForHeader( column );
}

QgsTextFormat QgsLayoutItemManualTable::textFormatForCell( int row, int column ) const
{
  if ( mContents.value( row ).value( column ).textFormat().isValid() )
    return mContents.value( row ).value( column ).textFormat();

  return QgsLayoutTable::textFormatForCell( row, column );
}

Qt::Alignment QgsLayoutItemManualTable::horizontalAlignmentForCell( int row, int column ) const
{
  if ( row < mContents.size() && column < mContents.at( row ).size() )
    return mContents.value( row ).value( column ).horizontalAlignment();

  return QgsLayoutTable::horizontalAlignmentForCell( row, column );
}

Qt::Alignment QgsLayoutItemManualTable::verticalAlignmentForCell( int row, int column ) const
{
  if ( row < mContents.size() && column < mContents.at( row ).size() )
    return mContents.value( row ).value( column ).verticalAlignment();

  return QgsLayoutTable::verticalAlignmentForCell( row, column );
}

void QgsLayoutItemManualTable::refreshColumns()
{
  // refresh columns
  QgsLayoutTableColumns columns;
  if ( !mContents.empty() )
  {
    int colIndex = 0;
    const QgsTableRow &firstRow = mContents[ 0 ];
    columns.reserve( firstRow.size() );
    for ( const QgsTableCell &cell : firstRow )
    {
      ( void )cell;
      QgsLayoutTableColumn newCol( mHeaders.value( colIndex ).heading() );
      newCol.setWidth( mColumnWidths.value( colIndex ) );
      columns << newCol;
      colIndex++;
    }
  }
  setColumns( columns );
}
