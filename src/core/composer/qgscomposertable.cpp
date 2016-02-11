/***************************************************************************
                         qgscomposertable.cpp
                         --------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposertable.h"
#include "qgscomposertablecolumn.h"
#include "qgssymbollayerv2utils.h"
#include "qgscomposerutils.h"
#include "qgsfontutils.h"
#include <QPainter>
#include <QSettings>


QgsComposerTable::QgsComposerTable( QgsComposition* composition )
    : QgsComposerItem( composition )
    , mLineTextDistance( 1.0 )
    , mHeaderFontColor( Qt::black )
    , mContentFontColor( Qt::black )
    , mHeaderHAlignment( FollowColumn )
    , mShowGrid( true )
    , mGridStrokeWidth( 0.5 )
    , mGridColor( QColor( 0, 0, 0 ) )
{
  //get default composer font from settings
  QSettings settings;
  QString defaultFontString = settings.value( "/Composer/defaultFont" ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    mHeaderFont.setFamily( defaultFontString );
    mContentFont.setFamily( defaultFontString );
  }
}

QgsComposerTable::~QgsComposerTable()
{
  qDeleteAll( mColumns );
  mColumns.clear();
}

void QgsComposerTable::refreshAttributes()
{
  mMaxColumnWidthMap.clear();
  mAttributeMaps.clear();

  //getFeatureAttributes
  if ( !getFeatureAttributes( mAttributeMaps ) )
  {
    return;
  }

  //since attributes have changed, we also need to recalculate the column widths
  //and size of table
  adjustFrameToSize();
}

void QgsComposerTable::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter )
  {
    return;
  }
  if ( !shouldDrawItem() )
  {
    return;
  }

  if ( mComposition->plotStyle() == QgsComposition::Print ||
       mComposition->plotStyle() == QgsComposition::Postscript )
  {
    //exporting composition, so force an attribute refresh
    //we do this in case vector layer has changed via an external source (eg, another database user)
    refreshAttributes();
  }

  drawBackground( painter );
  painter->save();
  //antialiasing on
  painter->setRenderHint( QPainter::Antialiasing, true );

  painter->setPen( Qt::SolidLine );

  //now draw the text
  double currentX = mGridStrokeWidth;
  double currentY;

  QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();

  int col = 0;
  double cellHeaderHeight = QgsComposerUtils::fontAscentMM( mHeaderFont ) + 2 * mLineTextDistance;
  double cellBodyHeight = QgsComposerUtils::fontAscentMM( mContentFont ) + 2 * mLineTextDistance;
  QRectF cell;
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    currentY = mGridStrokeWidth;
    currentX += mLineTextDistance;

    cell = QRectF( currentX, currentY, mMaxColumnWidthMap[col], cellHeaderHeight );

    //calculate alignment of header
    Qt::AlignmentFlag headerAlign = Qt::AlignLeft;
    switch ( mHeaderHAlignment )
    {
      case FollowColumn:
        headerAlign = ( *columnIt )->hAlignment();
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

    QgsComposerUtils::drawText( painter, cell, ( *columnIt )->heading(), mHeaderFont, mHeaderFontColor, headerAlign, Qt::AlignVCenter, Qt::TextDontClip );

    currentY += cellHeaderHeight;
    currentY += mGridStrokeWidth;

    //draw the attribute values
    QList<QgsAttributeMap>::const_iterator attIt = mAttributeMaps.constBegin();
    for ( ; attIt != mAttributeMaps.constEnd(); ++attIt )
    {
      cell = QRectF( currentX, currentY, mMaxColumnWidthMap[col], cellBodyHeight );

      const QgsAttributeMap &currentAttributeMap = *attIt;
      QString str = currentAttributeMap[ col ].toString();
      QgsComposerUtils::drawText( painter, cell, str, mContentFont, mContentFontColor, ( *columnIt )->hAlignment(), ( *columnIt )->vAlignment(), Qt::TextDontClip );

      currentY += cellBodyHeight;
      currentY += mGridStrokeWidth;
    }

    currentX += mMaxColumnWidthMap[ col ];
    currentX += mLineTextDistance;
    currentX += mGridStrokeWidth;
    col++;
  }

  //and the borders
  if ( mShowGrid )
  {
    QPen gridPen;
    gridPen.setWidthF( mGridStrokeWidth );
    gridPen.setColor( mGridColor );
    gridPen.setJoinStyle( Qt::MiterJoin );
    painter->setPen( gridPen );
    drawHorizontalGridLines( painter, mAttributeMaps.size() );
    drawVerticalGridLines( painter, mMaxColumnWidthMap );
  }

  painter->restore();

  //draw frame and selection boxes if necessary
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerTable::setLineTextDistance( double d )
{
  mLineTextDistance = d;
  //since spacing has changed, we need to recalculate the table size
  adjustFrameToSize();
}

void QgsComposerTable::setHeaderFont( const QFont& f )
{
  mHeaderFont = f;
  //since font attributes have changed, we need to recalculate the table size
  adjustFrameToSize();
}

void QgsComposerTable::setHeaderFontColor( const QColor &color )
{
  mHeaderFontColor = color;
  repaint();
}

void QgsComposerTable::setHeaderHAlignment( const QgsComposerTable::HeaderHAlignment alignment )
{
  mHeaderHAlignment = alignment;
  repaint();
}

void QgsComposerTable::setContentFont( const QFont& f )
{
  mContentFont = f;
  //since font attributes have changed, we need to recalculate the table size
  adjustFrameToSize();
}

void QgsComposerTable::setContentFontColor( const QColor &color )
{
  mContentFontColor = color;
  repaint();
}

void QgsComposerTable::setShowGrid( bool show )
{
  mShowGrid = show;
  //since grid spacing has changed, we need to recalculate the table size
  adjustFrameToSize();
}

void QgsComposerTable::setGridStrokeWidth( double w )
{
  mGridStrokeWidth = w;
  //since grid spacing has changed, we need to recalculate the table size
  adjustFrameToSize();
}

void QgsComposerTable::adjustFrameToSize()
{
  //check how much space each column needs
  if ( !calculateMaxColumnWidths( mMaxColumnWidthMap, mAttributeMaps ) )
  {
    return;
  }
  //adapt item frame to max width / height
  adaptItemFrame( mMaxColumnWidthMap, mAttributeMaps );

  repaint();
}

QMap<int, QString> QgsComposerTable::headerLabels() const
{
  QMap<int, QString> headers;

  QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();
  int col = 0;
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    headers.insert( col, ( *columnIt )->heading() );
    col++;
  }
  return headers;
}

void QgsComposerTable::setColumns( const QList<QgsComposerTableColumn*>& columns )
{
  //remove existing columns
  qDeleteAll( mColumns );
  mColumns.clear();

  mColumns.append( columns );
}

bool QgsComposerTable::tableWriteXML( QDomElement& elem, QDomDocument & doc ) const
{
  elem.setAttribute( "lineTextDist", QString::number( mLineTextDistance ) );
  elem.appendChild( QgsFontUtils::toXmlElement( mHeaderFont, doc, "headerFontProperties" ) );
  elem.setAttribute( "headerFontColor", QgsSymbolLayerV2Utils::encodeColor( mHeaderFontColor ) );
  elem.setAttribute( "headerHAlignment", QString::number( static_cast< int >( mHeaderHAlignment ) ) );
  elem.appendChild( QgsFontUtils::toXmlElement( mContentFont, doc, "contentFontProperties" ) );
  elem.setAttribute( "contentFontColor", QgsSymbolLayerV2Utils::encodeColor( mContentFontColor ) );
  elem.setAttribute( "gridStrokeWidth", QString::number( mGridStrokeWidth ) );
  elem.setAttribute( "gridColor", QgsSymbolLayerV2Utils::encodeColor( mGridColor ) );
  elem.setAttribute( "showGrid", mShowGrid );

  //columns
  QDomElement displayColumnsElem = doc.createElement( "displayColumns" );
  QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    QDomElement columnElem = doc.createElement( "column" );
    ( *columnIt )->writeXML( columnElem, doc );
    displayColumnsElem.appendChild( columnElem );
  }
  elem.appendChild( displayColumnsElem );

  return _writeXML( elem, doc );
}

bool QgsComposerTable::tableReadXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  if ( !QgsFontUtils::setFromXmlChildNode( mHeaderFont, itemElem, "headerFontProperties" ) )
  {
    mHeaderFont.fromString( itemElem.attribute( "headerFont", "" ) );
  }
  mHeaderFontColor = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "headerFontColor", "0,0,0,255" ) );
  mHeaderHAlignment = QgsComposerTable::HeaderHAlignment( itemElem.attribute( "headerHAlignment", "0" ).toInt() );
  if ( !QgsFontUtils::setFromXmlChildNode( mContentFont, itemElem, "contentFontProperties" ) )
  {
    mContentFont.fromString( itemElem.attribute( "contentFont", "" ) );
  }
  mContentFontColor = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "contentFontColor", "0,0,0,255" ) );
  mLineTextDistance = itemElem.attribute( "lineTextDist", "1.0" ).toDouble();
  mGridStrokeWidth = itemElem.attribute( "gridStrokeWidth", "0.5" ).toDouble();
  mShowGrid = itemElem.attribute( "showGrid", "1" ).toInt();

  //grid color
  if ( itemElem.hasAttribute( "gridColor" ) )
  {
    mGridColor = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "gridColor", "0,0,0,255" ) );
  }
  else
  {
    //old style grid color
    int gridRed = itemElem.attribute( "gridColorRed", "0" ).toInt();
    int gridGreen = itemElem.attribute( "gridColorGreen", "0" ).toInt();
    int gridBlue = itemElem.attribute( "gridColorBlue", "0" ).toInt();
    int gridAlpha = itemElem.attribute( "gridColorAlpha", "255" ).toInt();
    mGridColor = QColor( gridRed, gridGreen, gridBlue, gridAlpha );
  }

  //restore column specifications
  qDeleteAll( mColumns );
  mColumns.clear();
  QDomNodeList columnsList = itemElem.elementsByTagName( "displayColumns" );
  if ( !columnsList.isEmpty() )
  {
    QDomElement columnsElem =  columnsList.at( 0 ).toElement();
    QDomNodeList columnEntryList = columnsElem.elementsByTagName( "column" );
    for ( int i = 0; i < columnEntryList.size(); ++i )
    {
      QDomElement columnElem = columnEntryList.at( i ).toElement();
      QgsComposerTableColumn* column = new QgsComposerTableColumn;
      column->readXML( columnElem );
      mColumns.append( column );
    }
  }

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( !composerItemList.isEmpty() )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }
  return true;
}

bool QgsComposerTable::calculateMaxColumnWidths( QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeMaps ) const
{
  maxWidthMap.clear();
  QList<QgsComposerTableColumn*>::const_iterator columnIt = mColumns.constBegin();

  int col = 0;
  for ( ; columnIt != mColumns.constEnd(); ++columnIt )
  {
    maxWidthMap.insert( col, QgsComposerUtils::textWidthMM( mHeaderFont, ( *columnIt )->heading() ) );
    col++;
  }

  //go through all the attributes and adapt the max width values
  QList<QgsAttributeMap>::const_iterator attIt = attributeMaps.constBegin();

  double currentAttributeTextWidth;

  for ( ; attIt != attributeMaps.constEnd(); ++attIt )
  {
    QgsAttributeMap::const_iterator attIt2 = attIt->constBegin();
    for ( ; attIt2 != attIt->constEnd(); ++attIt2 )
    {
      currentAttributeTextWidth = QgsComposerUtils::textWidthMM( mContentFont, attIt2.value().toString() );
      if ( currentAttributeTextWidth > maxWidthMap[ attIt2.key()] )
      {
        maxWidthMap[ attIt2.key()] = currentAttributeTextWidth;
      }
    }
  }
  return true;
}

void QgsComposerTable::adaptItemFrame( const QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeMaps )
{
  //calculate height
  int n = attributeMaps.size();
  double totalHeight = QgsComposerUtils::fontAscentMM( mHeaderFont )
                       + n * QgsComposerUtils::fontAscentMM( mContentFont )
                       + ( n + 1 ) * mLineTextDistance * 2
                       + ( n + 2 ) * mGridStrokeWidth;

  //adapt frame to total width
  double totalWidth = 0;
  QMap<int, double>::const_iterator maxColWidthIt = maxWidthMap.constBegin();
  for ( ; maxColWidthIt != maxWidthMap.constEnd(); ++maxColWidthIt )
  {
    totalWidth += maxColWidthIt.value();
  }
  totalWidth += ( 2 * maxWidthMap.size() * mLineTextDistance );
  totalWidth += ( maxWidthMap.size() + 1 ) * mGridStrokeWidth;

  QRectF evaluatedRect = evalItemRect( QRectF( pos().x(), pos().y(), totalWidth, totalHeight ) );

  //update rect for data defined size and position
  QgsComposerItem::setSceneRect( evaluatedRect );
}

void QgsComposerTable::drawHorizontalGridLines( QPainter* p, int nAttributes )
{
  //horizontal lines
  double halfGridStrokeWidth = mGridStrokeWidth / 2.0;
  double currentY = halfGridStrokeWidth;
  p->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( rect().width() - halfGridStrokeWidth, currentY ) );
  currentY += mGridStrokeWidth;
  currentY += ( QgsComposerUtils::fontAscentMM( mHeaderFont ) + 2 * mLineTextDistance );
  for ( int i = 0; i < nAttributes; ++i )
  {
    p->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( rect().width() - halfGridStrokeWidth, currentY ) );
    currentY += mGridStrokeWidth;
    currentY += ( QgsComposerUtils::fontAscentMM( mContentFont ) + 2 * mLineTextDistance );
  }
  p->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( rect().width() - halfGridStrokeWidth, currentY ) );
}

void QgsComposerTable::drawVerticalGridLines( QPainter* p, const QMap<int, double>& maxWidthMap )
{
  //vertical lines
  double halfGridStrokeWidth = mGridStrokeWidth / 2.0;
  double currentX = halfGridStrokeWidth;
  p->drawLine( QPointF( currentX, halfGridStrokeWidth ), QPointF( currentX, rect().height() - halfGridStrokeWidth ) );
  currentX += mGridStrokeWidth;
  QMap<int, double>::const_iterator maxColWidthIt = maxWidthMap.constBegin();
  for ( ; maxColWidthIt != maxWidthMap.constEnd(); ++maxColWidthIt )
  {
    currentX += ( maxColWidthIt.value() + 2 * mLineTextDistance );
    p->drawLine( QPointF( currentX, halfGridStrokeWidth ), QPointF( currentX, rect().height() - halfGridStrokeWidth ) );
    currentX += mGridStrokeWidth;
  }
}
