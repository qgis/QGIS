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
#include "qgslogger.h"
#include <QPainter>

QgsComposerTable::QgsComposerTable( QgsComposition* composition ): QgsComposerItem( composition ), mLineTextDistance( 1.0 ), mShowGrid( true ), mGridStrokeWidth( 0.5 ), mGridColor( QColor( 0, 0, 0 ) )
{

}

QgsComposerTable::~QgsComposerTable()
{

}

void QgsComposerTable::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  if ( !painter )
  {
    return;
  }

  //getFeatureAttributes
  QList<QgsAttributeMap> attributeList;
  if ( !getFeatureAttributes( attributeList ) )
  {
    return;
  }

  QMap<int, double> maxColumnWidthMap;
  //check how much space each column needs
  calculateMaxColumnWidths( maxColumnWidthMap, attributeList );
  //adapt item frame to max width / height
  adaptItemFrame( maxColumnWidthMap, attributeList );

  drawBackground( painter );

  //now draw the text
  double currentX = mGridStrokeWidth;
  double currentY;

  QMap<int, QString> headerMap = getHeaderLabels();
  QMap<int, QString>::const_iterator columnIt = headerMap.constBegin();

  for ( ; columnIt != headerMap.constEnd(); ++columnIt )
  {
    currentY = mGridStrokeWidth;
    currentY += mLineTextDistance;
    currentY += fontAscentMillimeters( mHeaderFont );
    currentX += mLineTextDistance;
    drawText( painter, currentX, currentY, columnIt.value(), mHeaderFont );

    currentY += mLineTextDistance;
    currentY += mGridStrokeWidth;

    //draw the attribute values
    QList<QgsAttributeMap>::const_iterator attIt = attributeList.begin();
    for ( ; attIt != attributeList.end(); ++attIt )
    {
      currentY += fontAscentMillimeters( mContentFont );
      currentY += mLineTextDistance;

      QgsAttributeMap currentAttributeMap = *attIt;
      QgsAttributeMap::const_iterator attMapIt = currentAttributeMap.find( columnIt.key() );
      if ( attMapIt != currentAttributeMap.constEnd() )
      {
        drawText( painter, currentX, currentY, attMapIt.value().toString(), mContentFont );
      }
      currentY += mLineTextDistance;
      currentY += mGridStrokeWidth;
    }

    currentX += maxColumnWidthMap[columnIt.key()];
    currentX += mLineTextDistance;
    currentX += mGridStrokeWidth;
  }

  //and the borders
  if ( mShowGrid )
  {
    QPen gridPen;
    gridPen.setWidthF( mGridStrokeWidth );
    gridPen.setColor( mGridColor );
    painter->setPen( gridPen );
    drawHorizontalGridLines( painter, attributeList.size() );
    drawVerticalGridLines( painter, maxColumnWidthMap );
  }

  //draw frame and selection boxes if necessary
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerTable::adjustFrameToSize()
{
  QList<QgsAttributeMap> attributes;
  if ( !getFeatureAttributes( attributes ) )
  {
    return;
  }

  QMap<int, double> maxWidthMap;
  if ( !calculateMaxColumnWidths( maxWidthMap, attributes ) )
  {
    return;
  }
  adaptItemFrame( maxWidthMap, attributes );
}

bool QgsComposerTable::tableWriteXML( QDomElement& elem, QDomDocument & doc ) const
{
  elem.setAttribute( "lineTextDist", mLineTextDistance );
  elem.setAttribute( "headerFont", mHeaderFont.toString() );
  elem.setAttribute( "contentFont", mContentFont.toString() );
  elem.setAttribute( "gridStrokeWidth", mGridStrokeWidth );
  elem.setAttribute( "gridColorRed", mGridColor.red() );
  elem.setAttribute( "gridColorGreen", mGridColor.green() );
  elem.setAttribute( "gridColorBlue", mGridColor.blue() );
  elem.setAttribute( "showGrid", mShowGrid );
  return _writeXML( elem, doc );
}

bool QgsComposerTable::tableReadXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  mHeaderFont.fromString( itemElem.attribute( "headerFont", "" ) );
  mContentFont.fromString( itemElem.attribute( "contentFont", "" ) );
  mLineTextDistance = itemElem.attribute( "lineTextDist", "1.0" ).toDouble();
  mGridStrokeWidth = itemElem.attribute( "gridStrokeWidth", "0.5" ).toDouble();
  mShowGrid = itemElem.attribute( "showGrid", "1" ).toInt();

  //grid color
  int gridRed = itemElem.attribute( "gridColorRed", "0" ).toInt();
  int gridGreen = itemElem.attribute( "gridColorGreen", "0" ).toInt();
  int gridBlue = itemElem.attribute( "gridColorBlue", "0" ).toInt();
  mGridColor = QColor( gridRed, gridGreen, gridBlue );

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }
  return true;
}

bool QgsComposerTable::calculateMaxColumnWidths( QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeList ) const
{
  maxWidthMap.clear();
  QMap<int, QString> headerMap = getHeaderLabels();
  QMap<int, QString>::const_iterator headerIt = headerMap.constBegin();
  for ( ; headerIt != headerMap.constEnd(); ++headerIt )
  {
    maxWidthMap.insert( headerIt.key(), textWidthMillimeters( mHeaderFont, headerIt.value() ) );
  }

  //go through all the attributes and adapt the max width values
  QList<QgsAttributeMap>::const_iterator attIt = attributeList.constBegin();

  QgsAttributeMap currentAttributeMap;
  double currentAttributeTextWidth;

  for ( ; attIt != attributeList.constEnd(); ++attIt )
  {
    currentAttributeMap = *attIt;
    QgsAttributeMap::const_iterator attMapIt = currentAttributeMap.constBegin();
    for ( ; attMapIt != currentAttributeMap.constEnd(); ++attMapIt )
    {
      currentAttributeTextWidth = textWidthMillimeters( mContentFont, attMapIt.value().toString() );
      if ( currentAttributeTextWidth > maxWidthMap[attMapIt.key()] )
      {
        maxWidthMap[attMapIt.key()] = currentAttributeTextWidth;
      }
    }
  }
  return true;
}





void QgsComposerTable::adaptItemFrame( const QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeList )
{
  //calculate height
  double totalHeight = fontAscentMillimeters( mHeaderFont ) + attributeList.size() * fontAscentMillimeters( mContentFont ) \
                       + ( attributeList.size() + 1 ) * mLineTextDistance * 2 + ( attributeList.size() + 2 ) * mGridStrokeWidth;

  //adapt frame to total width
  double totalWidth = 0;
  QMap<int, double>::const_iterator maxColWidthIt = maxWidthMap.constBegin();
  for ( ; maxColWidthIt != maxWidthMap.constEnd(); ++maxColWidthIt )
  {
    totalWidth += maxColWidthIt.value();
  }
  totalWidth += ( 2 * maxWidthMap.size() * mLineTextDistance );
  totalWidth += ( maxWidthMap.size() + 1 ) * mGridStrokeWidth;
  QTransform t = transform();
  QgsComposerItem::setSceneRect( QRectF( t.dx(), t.dy(), totalWidth, totalHeight ) );
}

void QgsComposerTable::drawHorizontalGridLines( QPainter* p, int nAttributes )
{
  //horizontal lines
  double halfGridStrokeWidth = mGridStrokeWidth / 2.0;
  double currentY = halfGridStrokeWidth;
  p->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( rect().width() - halfGridStrokeWidth, currentY ) );
  currentY += mGridStrokeWidth;
  currentY += ( fontAscentMillimeters( mHeaderFont ) + 2 * mLineTextDistance );
  for ( int i = 0; i < nAttributes; ++i )
  {
    p->drawLine( QPointF( halfGridStrokeWidth, currentY ), QPointF( rect().width() - halfGridStrokeWidth, currentY ) );
    currentY += mGridStrokeWidth;
    currentY += ( fontAscentMillimeters( mContentFont ) + 2 * mLineTextDistance );
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



