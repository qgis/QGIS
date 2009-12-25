/***************************************************************************
                           qgscomposerscalebar.cpp
                             -------------------
    begin                : March 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerscalebar.h"
#include "qgscomposermap.h"
#include "qgsscalebarstyle.h"
#include "qgsdoubleboxscalebarstyle.h"
#include "qgsnumericscalebarstyle.h"
#include "qgssingleboxscalebarstyle.h"
#include "qgsticksscalebarstyle.h"
#include "qgsrectangle.h"
#include <QDomDocument>
#include <QDomElement>
#include <QFontMetricsF>
#include <QPainter>
#include <cmath>

QgsComposerScaleBar::QgsComposerScaleBar( QgsComposition* composition ): QgsComposerItem( composition ), mComposerMap( 0 ), mStyle( 0 ), mSegmentMillimeters( 0.0 )
{
  applyDefaultSettings();
}

QgsComposerScaleBar::~QgsComposerScaleBar()
{
  delete mStyle;
}

void QgsComposerScaleBar::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  if ( !mStyle || !painter )
  {
    return;
  }

  drawBackground( painter );
  painter->setPen( QPen( QColor( 0, 0, 0 ) ) ); //draw all text black

  //x-offset is half of first label width because labels are drawn centered
  QString firstLabel = firstLabelString();
  double firstLabelWidth = textWidthMillimeters( mFont, firstLabel );

  mStyle->draw( painter, firstLabelWidth / 2 );

  //draw frame and selection boxes if necessary
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerScaleBar::setNumUnitsPerSegment( double units )
{
  mNumUnitsPerSegment = units;
  refreshSegmentMillimeters();
}

void QgsComposerScaleBar::setComposerMap( const QgsComposerMap* map )
{
  disconnect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( updateSegmentSize() ) );
  disconnect( mComposerMap, SIGNAL( destroyed( QObject* ) ), this, SLOT( invalidateCurrentMap() ) );
  mComposerMap = map;

  if ( !map )
  {
    return;
  }

  connect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( updateSegmentSize() ) );
  connect( mComposerMap, SIGNAL( destroyed( QObject* ) ), this, SLOT( invalidateCurrentMap() ) );

  refreshSegmentMillimeters();
}

void QgsComposerScaleBar::invalidateCurrentMap()
{
  disconnect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( updateSegmentSize() ) );
  disconnect( mComposerMap, SIGNAL( destroyed( QObject* ) ), this, SLOT( invalidateCurrentMap() ) );
  mComposerMap = 0;
}

void QgsComposerScaleBar::refreshSegmentMillimeters()
{
  if ( mComposerMap )
  {
    //get extent of composer map
    QgsRectangle composerMapRect = mComposerMap->extent();

    //get mm dimension of composer map
    QRectF composerItemRect = mComposerMap->rect();

    //calculate size depending on mNumUnitsPerSegment
    mSegmentMillimeters = composerItemRect.width() / composerMapRect.width() * mNumUnitsPerSegment;
  }
}

void QgsComposerScaleBar::applyDefaultSettings()
{
  mNumSegments = 2;
  mNumSegmentsLeft = 0;

  mNumMapUnitsPerScaleBarUnit = 1.0;

  //style
  delete mStyle;
  mStyle = new QgsSingleBoxScaleBarStyle( this );

  mHeight = 5;

  mPen = QPen( QColor( 0, 0, 0 ) );
  mPen.setWidthF( 1.0 );

  mBrush.setColor( QColor( 0, 0, 0 ) );
  mBrush.setStyle( Qt::SolidPattern );

  mFont.setPointSizeF( 12.0 );

  mLabelBarSpace = 3.0;
  mBoxContentSpace = 1.0;

  if ( mComposerMap )
  {
    //calculate mNumUnitsPerSegment
    QRectF composerItemRect = mComposerMap->rect();
    QgsRectangle composerMapRect = mComposerMap->extent();

    double proposedScaleBarLength = composerMapRect.width() / 4;
    int powerOf10 = int ( pow( 10.0, int ( log( proposedScaleBarLength ) / log( 10.0 ) ) ) ); // from scalebar plugin
    int nPow10 = proposedScaleBarLength / powerOf10;
    mNumSegments = 2;
    mNumUnitsPerSegment = ( nPow10 / 2 ) * powerOf10;
  }

  refreshSegmentMillimeters();
  adjustBoxSize();
}

void QgsComposerScaleBar::adjustBoxSize()
{
  if ( !mStyle )
  {
    return;
  }

  QRectF box = mStyle->calculateBoxSize();
  setSceneRect( box );
}

void QgsComposerScaleBar::update()
{
  adjustBoxSize();
  QgsComposerItem::update();
}

void QgsComposerScaleBar::updateSegmentSize()
{
  refreshSegmentMillimeters();
  update();
}

void QgsComposerScaleBar::segmentPositions( QList<QPair<double, double> >& posWidthList ) const
{
  posWidthList.clear();
  double mCurrentXCoord = mPen.widthF() + mBoxContentSpace;

  //left segments
  for ( int i = 0; i < mNumSegmentsLeft; ++i )
  {
    posWidthList.push_back( qMakePair( mCurrentXCoord, mSegmentMillimeters / mNumSegmentsLeft ) );
    mCurrentXCoord += mSegmentMillimeters / mNumSegmentsLeft;
  }

  //right segments
  for ( int i = 0; i < mNumSegments; ++i )
  {
    posWidthList.push_back( qMakePair( mCurrentXCoord, mSegmentMillimeters ) );
    mCurrentXCoord += mSegmentMillimeters;
  }
}

void QgsComposerScaleBar::setStyle( const QString& styleName )
{
  delete mStyle;
  mStyle = 0;

  //switch depending on style name
  if ( styleName == "Single Box" )
  {
    mStyle = new QgsSingleBoxScaleBarStyle( this );
  }
  else if ( styleName == "Double Box" )
  {
    mStyle = new QgsDoubleBoxScaleBarStyle( this );
  }
  else if ( styleName == "Line Ticks Middle"  || styleName == "Line Ticks Down" || styleName == "Line Ticks Up" )
  {
    QgsTicksScaleBarStyle* tickStyle = new QgsTicksScaleBarStyle( this );
    if ( styleName == "Line Ticks Middle" )
    {
      tickStyle->setTickPosition( QgsTicksScaleBarStyle::TicksMiddle );
    }
    else if ( styleName == "Line Ticks Down" )
    {
      tickStyle->setTickPosition( QgsTicksScaleBarStyle::TicksDown );
    }
    else if ( styleName == "Line Ticks Up" )
    {
      tickStyle->setTickPosition( QgsTicksScaleBarStyle::TicksUp );
    }
    mStyle = tickStyle;
  }
  else if ( styleName == "Numeric" )
  {
    mStyle = new QgsNumericScaleBarStyle( this );
  }
}

QString QgsComposerScaleBar::style() const
{
  if ( mStyle )
  {
    return mStyle->name();
  }
  else
  {
    return "";
  }
}

QString QgsComposerScaleBar::firstLabelString() const
{
  if ( mNumSegmentsLeft > 0 )
  {
    return QString::number( mNumUnitsPerSegment / mNumMapUnitsPerScaleBarUnit );
  }
  else
  {
    return "0";
  }
}

QFont QgsComposerScaleBar::font() const
{
  return mFont;
}

void QgsComposerScaleBar::setFont( const QFont& font )
{
  mFont = font;
  adjustBoxSize();
  update();
}

bool QgsComposerScaleBar::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement composerScaleBarElem = doc.createElement( "ComposerScaleBar" );
  composerScaleBarElem.setAttribute( "height", mHeight );
  composerScaleBarElem.setAttribute( "labelBarSpace", mLabelBarSpace );
  composerScaleBarElem.setAttribute( "boxContentSpace", mBoxContentSpace );
  composerScaleBarElem.setAttribute( "numSegments", mNumSegments );
  composerScaleBarElem.setAttribute( "numSegmentsLeft", mNumSegmentsLeft );
  composerScaleBarElem.setAttribute( "numUnitsPerSegment", mNumUnitsPerSegment );
  composerScaleBarElem.setAttribute( "segmentMillimeters", mSegmentMillimeters );
  composerScaleBarElem.setAttribute( "numMapUnitsPerScaleBarUnit", mNumMapUnitsPerScaleBarUnit );
  composerScaleBarElem.setAttribute( "font", mFont.toString() );
  composerScaleBarElem.setAttribute( "outlineWidth", mPen.widthF() );
  composerScaleBarElem.setAttribute( "unitLabel", mUnitLabeling );

  //style
  if ( mStyle )
  {
    composerScaleBarElem.setAttribute( "style", mStyle->name() );
  }

  //map id
  if ( mComposerMap )
  {
    composerScaleBarElem.setAttribute( "mapId", mComposerMap->id() );
  }

  //fill color
  QColor brushColor = mBrush.color();
  QDomElement colorElem = doc.createElement( "BrushColor" );
  colorElem.setAttribute( "red", brushColor.red() );
  colorElem.setAttribute( "green", brushColor.green() );
  colorElem.setAttribute( "blue", brushColor.blue() );
  composerScaleBarElem.appendChild( colorElem );

  elem.appendChild( composerScaleBarElem );
  return _writeXML( composerScaleBarElem, doc );
}

bool QgsComposerScaleBar::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  mHeight = itemElem.attribute( "height", "5.0" ).toDouble();
  mLabelBarSpace = itemElem.attribute( "labelBarSpace", "3.0" ).toDouble();
  mBoxContentSpace = itemElem.attribute( "boxContentSpace", "1.0" ).toDouble();
  mNumSegments = itemElem.attribute( "numSegments", "2" ).toInt();
  mNumSegmentsLeft = itemElem.attribute( "numSegmentsLeft", "0" ).toInt();
  mNumUnitsPerSegment = itemElem.attribute( "numUnitsPerSegment", "1.0" ).toDouble();
  mSegmentMillimeters = itemElem.attribute( "segmentMillimeters", "0.0" ).toDouble();
  mNumMapUnitsPerScaleBarUnit = itemElem.attribute( "numMapUnitsPerScaleBarUnit", "1.0" ).toDouble();
  mPen.setWidthF( itemElem.attribute( "outlineWidth", "1.0" ).toDouble() );
  mUnitLabeling = itemElem.attribute( "unitLabel" );
  QString fontString = itemElem.attribute( "font", "" );
  if ( !fontString.isEmpty() )
  {
    mFont.fromString( fontString );
  }

  //style
  delete mStyle;
  mStyle = 0;
  QString styleString = itemElem.attribute( "style", "" );
  setStyle( tr( styleString.toLocal8Bit().data() ) );

  //map
  int mapId = itemElem.attribute( "mapId", "-1" ).toInt();
  if ( mapId >= 0 )
  {
    const QgsComposerMap* composerMap = mComposition->getComposerMapById( mapId );
    mComposerMap = composerMap;
    if ( mComposerMap )
    {
      connect( mComposerMap, SIGNAL( extentChanged() ), this, SLOT( updateSegmentSize() ) );
      connect( mComposerMap, SIGNAL( destroyed( QObject* ) ), this, SLOT( invalidateCurrentMap() ) );
    }
  }

  refreshSegmentMillimeters();

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }

  return true;
}


