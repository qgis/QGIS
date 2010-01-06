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
#include "qgscomposermap.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include <QPainter>

QgsComposerTable::QgsComposerTable( QgsComposition* composition ): QgsComposerItem( composition ), mVectorLayer( 0 ), mComposerMap( 0 ), mMaximumNumberOfFeatures( 5 )
{
  mLineTextDistance = 1;
}

QgsComposerTable::~QgsComposerTable()
{

}

void QgsComposerTable::setComposerMap( const QgsComposerMap* map )
{
  if ( mComposerMap )
  {
    QObject::disconnect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( repaint() ) );
  }
  mComposerMap = map;
  if ( mComposerMap )
  {
    QObject::connect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( repaint() ) );
  }
}

void QgsComposerTable::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  if ( !painter )
  {
    return;
  }

  if ( mComposerMap && mComposerMap->isDrawing() )
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
  //adapt item fram to max width / height
  adaptItemFrame( maxColumnWidthMap, attributeList );

  //now draw the text
  double currentX = 0;
  double currentY;

  QgsFieldMap vectorFields = mVectorLayer->pendingFields();
  QgsFieldMap::const_iterator fieldIt = vectorFields.constBegin();
  for ( ; fieldIt != vectorFields.constEnd(); ++fieldIt )
  {
    currentY = 0;
    currentY += mLineTextDistance;
    currentY += fontAscentMillimeters( mHeaderFont );
    currentX += mLineTextDistance;
    drawText( painter, currentX, currentY, fieldIt.value().name(), mHeaderFont );

    currentY += mLineTextDistance;

    //draw the attribute values
    QList<QgsAttributeMap>::const_iterator attIt = attributeList.begin();
    for ( ; attIt != attributeList.end(); ++attIt )
    {
      currentY += fontAscentMillimeters( mContentFont );
      currentY += mLineTextDistance;

      QgsAttributeMap currentAttributeMap = *attIt;
      QgsAttributeMap::const_iterator attMapIt = currentAttributeMap.find( fieldIt.key() );
      if ( attMapIt != currentAttributeMap.constEnd() )
      {
        drawText( painter, currentX, currentY, attMapIt.value().toString(), mContentFont );
      }

      currentY += mLineTextDistance;
    }

    currentX += mLineTextDistance;
    currentX += maxColumnWidthMap[fieldIt.key()];
  }

  //and the borders
  painter->setPen( mGridPen );
  drawHorizontalGridLines( painter, attributeList.size() );
  drawVerticalGridLines( painter, maxColumnWidthMap );

  //draw frame and selection boxes if necessary
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

bool QgsComposerTable::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerTableElem = doc.createElement( "ComposerTable" );
  composerTableElem.setAttribute( "maxFeatures", mMaximumNumberOfFeatures );
  composerTableElem.setAttribute( "lineTextDist", mLineTextDistance );
  composerTableElem.setAttribute( "headerFont", mHeaderFont.toString() );
  composerTableElem.setAttribute( "contentFont", mContentFont.toString() );
  if ( mComposerMap )
  {
    composerTableElem.setAttribute( "composerMap", mComposerMap->id() );
  }
  else
  {
    composerTableElem.setAttribute( "composerMap", -1 );
  }
  if ( mVectorLayer )
  {
    composerTableElem.setAttribute( "vectorLayer", mVectorLayer->getLayerID() );
  }
  elem.appendChild( composerTableElem );
  return _writeXML( composerTableElem, doc );;
}

bool QgsComposerTable::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  mMaximumNumberOfFeatures = itemElem.attribute( "maxFeatures", "5" ).toInt();
  mHeaderFont.fromString( itemElem.attribute( "headerFont", "" ) );
  mContentFont.fromString( itemElem.attribute( "contentFont", "" ) );
  mLineTextDistance = itemElem.attribute( "lineTextDist", "1.0" ).toDouble();

  //composer map
  int composerMapId = itemElem.attribute( "composerMap", "-1" ).toInt();
  if ( composerMapId == -1 )
  {
    mComposerMap = 0;
  }

  if ( composition() )
  {
    mComposerMap = composition()->getComposerMapById( composerMapId );
  }
  else
  {
    mComposerMap = 0;
  }

  //vector layer
  QString layerId = itemElem.attribute( "vectorLayer", "not_existing" );
  if ( layerId == "not_existing" )
  {
    mVectorLayer = 0;
  }
  else
  {
    QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );
    if ( ml )
    {
      mVectorLayer = dynamic_cast<QgsVectorLayer*>( ml );
    }
  }

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }
  return true;
}

bool QgsComposerTable::getFeatureAttributes( QList<QgsAttributeMap>& attributes )
{
  if ( !mVectorLayer )
  {
    return false;
  }
  attributes.clear();

  QgsRectangle selectionRect;
  if ( mComposerMap )
  {
    selectionRect = mComposerMap->extent();
  }

  mVectorLayer->select( mVectorLayer->pendingAllAttributesList(), selectionRect, false, true );
  QgsFeature f;
  int counter = 0;
  while ( mVectorLayer->nextFeature( f ) && counter < mMaximumNumberOfFeatures )
  {
    attributes.push_back( f.attributeMap() );
    ++counter;
  }
  return true;
}

bool QgsComposerTable::calculateMaxColumnWidths( QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeList ) const
{
  maxWidthMap.clear();
  if ( !mVectorLayer )
  {
    return false;
  }

  QgsFieldMap vectorFields = mVectorLayer->pendingFields();

  //initialize max width map with attribute names
  QgsFieldMap::const_iterator fieldIt = vectorFields.constBegin();
  for ( ; fieldIt != vectorFields.constEnd(); ++fieldIt )
  {
    maxWidthMap.insert( fieldIt.key(), textWidthMillimeters( mHeaderFont, fieldIt.value().name() ) );
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
  double totalHeight = fontAscentMillimeters( mHeaderFont ) + attributeList.size() * fontAscentMillimeters( mContentFont ) + ( attributeList.size() + 1 ) * mLineTextDistance * 2;

  //adapt frame to total width
  double totalWidth = 0;
  QMap<int, double>::const_iterator maxColWidthIt = maxWidthMap.constBegin();
  for ( ; maxColWidthIt != maxWidthMap.constEnd(); ++maxColWidthIt )
  {
    totalWidth += maxColWidthIt.value();
  }
  totalWidth += ( 2 * maxWidthMap.size() * mLineTextDistance );
  QTransform t = transform();
  setSceneRect( QRectF( t.dx(), t.dy(), totalWidth, totalHeight ) );
}

void QgsComposerTable::drawHorizontalGridLines( QPainter* p, int nAttributes )
{
  //horizontal lines
  double currentY = 0;
  p->drawLine( QPointF( 0, currentY ), QPointF( rect().width(), currentY ) );
  currentY += ( fontAscentMillimeters( mHeaderFont ) + 2 * mLineTextDistance );
  for ( int i = 0; i < nAttributes; ++i )
  {
    p->drawLine( QPointF( 0, currentY ), QPointF( rect().width(), currentY ) );
    currentY += ( fontAscentMillimeters( mContentFont ) + 2 * mLineTextDistance );
  }
  p->drawLine( QPointF( 0, currentY ), QPointF( rect().width(), currentY ) );
}

void QgsComposerTable::drawVerticalGridLines( QPainter* p, const QMap<int, double>& maxWidthMap )
{
  //vertical lines
  double currentX = 0;
  p->drawLine( QPointF( currentX, 0 ), QPointF( currentX, rect().height() ) );
  QMap<int, double>::const_iterator maxColWidthIt = maxWidthMap.constBegin();
  for ( ; maxColWidthIt != maxWidthMap.constEnd(); ++maxColWidthIt )
  {
    currentX += ( maxColWidthIt.value() + 2 * mLineTextDistance );
    p->drawLine( QPointF( currentX, 0 ), QPointF( currentX, rect().height() ) );
  }
}

