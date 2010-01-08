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

QgsComposerTable::QgsComposerTable( QgsComposition* composition ): QgsComposerItem( composition ), mVectorLayer( 0 ), mComposerMap( 0 ), \
    mMaximumNumberOfFeatures( 5 ), mLineTextDistance( 1.0 ), mShowGrid( true ), mGridStrokeWidth( 0.5 ), mGridColor( QColor( 0, 0, 0 ) )
{

}

QgsComposerTable::~QgsComposerTable()
{

}

void QgsComposerTable::initializeAliasMap()
{
  mFieldAliasMap.clear();
  if ( mVectorLayer )
  {
    QgsFieldMap fieldMap = mVectorLayer->pendingFields();
    QgsFieldMap::const_iterator it = fieldMap.constBegin();
    for ( ; it != fieldMap.constEnd(); ++it )
    {
      QString currentAlias = mVectorLayer->attributeAlias( it.key() );
      if ( !currentAlias.isEmpty() )
      {
        mFieldAliasMap.insert( it.key(), currentAlias );
      }
    }
  }
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

void QgsComposerTable::setVectorLayer( QgsVectorLayer* vl )
{
  if ( vl != mVectorLayer )
  {
    mDisplayAttributes.clear();
    mVectorLayer = vl;
    initializeAliasMap();
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

  drawBackground( painter );

  //now draw the text
  double currentX = mGridStrokeWidth;
  double currentY;

  QgsFieldMap vectorFields = mVectorLayer->pendingFields();
  QgsFieldMap::const_iterator fieldIt = vectorFields.constBegin();
  for ( ; fieldIt != vectorFields.constEnd(); ++fieldIt )
  {
    if ( mDisplayAttributes.size() > 0 && !mDisplayAttributes.contains( fieldIt.key() ) )
    {
      continue;
    }
    currentY = mGridStrokeWidth;
    currentY += mLineTextDistance;
    currentY += fontAscentMillimeters( mHeaderFont );
    currentX += mLineTextDistance;
    drawText( painter, currentX, currentY, attributeDisplayName( fieldIt.key(), fieldIt.value().name() ), mHeaderFont );

    currentY += mLineTextDistance;
    currentY += mGridStrokeWidth;

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
      currentY += mGridStrokeWidth;
    }

    currentX += maxColumnWidthMap[fieldIt.key()];
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

bool QgsComposerTable::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerTableElem = doc.createElement( "ComposerTable" );
  composerTableElem.setAttribute( "maxFeatures", mMaximumNumberOfFeatures );
  composerTableElem.setAttribute( "lineTextDist", mLineTextDistance );
  composerTableElem.setAttribute( "headerFont", mHeaderFont.toString() );
  composerTableElem.setAttribute( "contentFont", mContentFont.toString() );
  composerTableElem.setAttribute( "gridStrokeWidth", mGridStrokeWidth );
  composerTableElem.setAttribute( "gridColorRed", mGridColor.red() );
  composerTableElem.setAttribute( "gridColorGreen", mGridColor.green() );
  composerTableElem.setAttribute( "gridColorBlue", mGridColor.blue() );
  composerTableElem.setAttribute( "showGrid", mShowGrid );

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

  //display attributes
  QDomElement displayAttributesElem = doc.createElement( "displayAttributes" );
  QSet<int>::const_iterator attIt = mDisplayAttributes.constBegin();
  for ( ; attIt != mDisplayAttributes.constEnd(); ++attIt )
  {
    QDomElement attributeIndexElem = doc.createElement( "attributeEntry" );
    attributeIndexElem.setAttribute( "index", *attIt );
    displayAttributesElem.appendChild( attributeIndexElem );
  }
  composerTableElem.appendChild( displayAttributesElem );

  //alias map
  QDomElement aliasMapElem = doc.createElement( "attributeAliasMap" );
  QMap<int, QString>::const_iterator aliasIt = mFieldAliasMap.constBegin();
  for ( ; aliasIt != mFieldAliasMap.constEnd(); ++aliasIt )
  {
    QDomElement mapEntryElem = doc.createElement( "aliasEntry" );
    mapEntryElem.setAttribute( "key", aliasIt.key() );
    mapEntryElem.setAttribute( "value", aliasIt.value() );
    aliasMapElem.appendChild( mapEntryElem );
  }
  composerTableElem.appendChild( aliasMapElem );

  elem.appendChild( composerTableElem );
  return _writeXML( composerTableElem, doc );
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
  mGridStrokeWidth = itemElem.attribute( "gridStrokeWidth", "0.5" ).toDouble();
  mShowGrid = itemElem.attribute( "showGrid", "1" ).toInt();

  //grid color
  int gridRed = itemElem.attribute( "gridColorRed", "0" ).toInt();
  int gridGreen = itemElem.attribute( "gridColorGreen", "0" ).toInt();
  int gridBlue = itemElem.attribute( "gridColorBlue", "0" ).toInt();
  mGridColor = QColor( gridRed, gridGreen, gridBlue );

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

  //restore display attribute map
  mDisplayAttributes.clear();
  QDomNodeList displayAttributeList = itemElem.elementsByTagName( "displayAttributes" );
  if ( displayAttributeList.size() > 0 )
  {
    QDomElement displayAttributesElem =  displayAttributeList.at( 0 ).toElement();
    QDomNodeList attributeEntryList = displayAttributesElem.elementsByTagName( "attributeEntry" );
    for ( int i = 0; i < attributeEntryList.size(); ++i )
    {
      QDomElement attributeEntryElem = attributeEntryList.at( i ).toElement();
      int index = attributeEntryElem.attribute( "index", "-1" ).toInt();
      if ( index != -1 )
      {
        mDisplayAttributes.insert( index );
      }
    }
  }

  //restore alias map
  mFieldAliasMap.clear();
  QDomNodeList aliasMapNodeList = itemElem.elementsByTagName( "attributeAliasMap" );
  if ( aliasMapNodeList.size() > 0 )
  {
    QDomElement attributeAliasMapElem = aliasMapNodeList.at( 0 ).toElement();
    QDomNodeList aliasMepEntryList = attributeAliasMapElem.elementsByTagName( "aliasEntry" );
    for ( int i = 0; i < aliasMepEntryList.size(); ++i )
    {
      QDomElement aliasEntryElem = aliasMepEntryList.at( i ).toElement();
      int key = aliasEntryElem.attribute( "key", "-1" ).toInt();
      QString value = aliasEntryElem.attribute( "value", "" );
      mFieldAliasMap.insert( key, value );
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

  if ( mDisplayAttributes.size() < 1 )
  {
    mVectorLayer->select( mVectorLayer->pendingAllAttributesList(), selectionRect, false, true );
  }
  else
  {
    mVectorLayer->select( mDisplayAttributes.toList(), selectionRect, false, true );
  }
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
    if ( mDisplayAttributes.size() < 1 || mDisplayAttributes.contains( fieldIt.key() ) )
    {
      maxWidthMap.insert( fieldIt.key(), textWidthMillimeters( mHeaderFont, attributeDisplayName( fieldIt.key(), fieldIt.value().name() ) ) );
    }
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
  setSceneRect( QRectF( t.dx(), t.dy(), totalWidth, totalHeight ) );
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

QString QgsComposerTable::attributeDisplayName( int attributeIndex, const QString& name ) const
{
  QMap<int, QString>::const_iterator it = mFieldAliasMap.find( attributeIndex );
  if ( it != mFieldAliasMap.constEnd() )
  {
    return it.value();
  }
  else
  {
    return name;
  }
}

