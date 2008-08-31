/***************************************************************************
                         qgscomposerlabel.cpp
                             -------------------
    begin                : January 2005
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

#include "qgscomposerlabel.h"
#include <QDomElement>
#include <QPainter>

QgsComposerLabel::QgsComposerLabel( QgsComposition *composition ): QgsComposerItem( composition ), mMargin( 0.0 )
{
  //default font size is 10 point
  if ( mComposition )
  {
    mFont.setPixelSize( mComposition->pixelFontSize( 10 ) );
  }
  else
  {
    mFont.setPixelSize( 4 );
  }
}

QgsComposerLabel::~QgsComposerLabel()
{
}

void QgsComposerLabel::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  if ( !painter )
  {
    return;
  }

  drawBackground( painter );
  painter->setPen( QPen( QColor( 0, 0, 0 ) ) ); //draw all text black
  painter->setFont( mFont );

  QFontMetricsF fontSize( mFont );

  //support multiline labels
  double penWidth = pen().widthF();
  QRectF painterRect( penWidth, penWidth, rect().width() - 2 * penWidth,
                      rect().height() - 2 * penWidth );
  painter->drawText( painterRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, mText );


  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerLabel::setText( const QString& text )
{
  mText = text;
}

void QgsComposerLabel::setFont( const QFont& f )
{
  //set font size in pixels for proper preview and printout
  if ( mComposition )
  {
    int pixelSize = mComposition->pixelFontSize( f.pointSizeF() );
    mFont = f;
    mFont.setPixelSize( pixelSize );
  }
  else
  {
    mFont = f;
  }
}

void QgsComposerLabel::adjustSizeToText()
{
  QFontMetricsF fontInfo( mFont );
  setSceneRect( QRectF( transform().dx(), transform().dy(), fontInfo.width( mText ) + 2 * mMargin + 2 * pen().widthF(), fontInfo.ascent() + 2 * mMargin + 2 * pen().widthF() ) );
}

QFont QgsComposerLabel::font() const
{
  if ( mComposition ) //make pixel to point conversion to show correct point value in dialogs
  {
    double pointSize = mComposition->pointFontSize( mFont.pixelSize() );
    QFont returnFont = mFont;
    returnFont.setPointSize( pointSize );
    return returnFont;
  }
  return mFont;
}

bool QgsComposerLabel::writeXML( QDomElement& elem, QDomDocument & doc )
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement composerLabelElem = doc.createElement( "ComposerLabel" );

  composerLabelElem.setAttribute( "labelText", mText );
  composerLabelElem.setAttribute( "margin", QString::number( mMargin ) );


  //font
  QDomElement labelFontElem = doc.createElement( "LabelFont" );
  labelFontElem.setAttribute( "description", mFont.toString() );
  composerLabelElem.appendChild( labelFontElem );

  elem.appendChild( composerLabelElem );
  return _writeXML( composerLabelElem, doc );
}

bool QgsComposerLabel::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  //restore label specific properties

  //text
  mText = itemElem.attribute( "labelText" );

  //margin
  mMargin = itemElem.attribute( "margin" ).toDouble();

  //font
  QDomNodeList labelFontList = itemElem.elementsByTagName( "LabelFont" );
  if ( labelFontList.size() > 0 )
  {
    QDomElement labelFontElem = labelFontList.at( 0 ).toElement();
    mFont.fromString( labelFontElem.attribute( "description" ) );
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
