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
#include <QDate>
#include <QDomElement>
#include <QPainter>

QgsComposerLabel::QgsComposerLabel( QgsComposition *composition ): QgsComposerItem( composition ), mMargin( 1.0 ), mFontColor( QColor( 0, 0, 0 ) ), \
    mHAlignment( Qt::AlignLeft ), mVAlignment( Qt::AlignTop )
{
  //default font size is 10 point
  mFont.setPointSizeF( 10 );
}

QgsComposerLabel::~QgsComposerLabel()
{
}

void QgsComposerLabel::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter )
  {
    return;
  }

  drawBackground( painter );
  painter->setPen( QPen( QColor( mFontColor ) ) ); //draw all text black
  painter->setFont( mFont );

  QFontMetricsF fontSize( mFont );

  //support multiline labels
  double penWidth = pen().widthF();
  QRectF painterRect( penWidth + mMargin, penWidth + mMargin, rect().width() - 2 * penWidth - 2 * mMargin,
                      rect().height() - 2 * penWidth - 2 * mMargin );


  drawText( painter, painterRect, displayText(), mFont, mHAlignment, mVAlignment );

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

QString QgsComposerLabel::displayText() const
{
  QString displayText = mText;
  replaceDateText( displayText );
  return displayText;
}

void QgsComposerLabel::replaceDateText( QString& text ) const
{
  int currentDatePos = text.indexOf( "$CURRENT_DATE" );
  if ( currentDatePos != -1 )
  {
    //check if there is a bracket just after $CURRENT_DATE
    QString formatText;
    int openingBracketPos = text.indexOf( "(", currentDatePos );
    int closingBracketPos = text.indexOf( ")", openingBracketPos + 1 );
    if ( openingBracketPos != -1 && closingBracketPos != -1 && ( closingBracketPos - openingBracketPos ) > 1 )
    {
      formatText = text.mid( openingBracketPos + 1, closingBracketPos - openingBracketPos - 1 );
      text.replace( currentDatePos, closingBracketPos - currentDatePos + 1, QDate::currentDate().toString( formatText ) );
    }
    else //no bracket
    {
      text.replace( "$CURRENT_DATE", QDate::currentDate().toString() );
    }
  }
}

void QgsComposerLabel::setFont( const QFont& f )
{
  mFont = f;
}

void QgsComposerLabel::adjustSizeToText()
{
  double textWidth = textWidthMillimeters( mFont, displayText() );
  double fontAscent = fontAscentMillimeters( mFont );

  setSceneRect( QRectF( transform().dx(), transform().dy(), textWidth + 2 * mMargin + 2 * pen().widthF() + 1, \
                        fontAscent + 2 * mMargin + 2 * pen().widthF() + 1 ) );
}

QFont QgsComposerLabel::font() const
{
  return mFont;
}

bool QgsComposerLabel::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QString alignment;

  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement composerLabelElem = doc.createElement( "ComposerLabel" );

  composerLabelElem.setAttribute( "labelText", mText );
  composerLabelElem.setAttribute( "margin", QString::number( mMargin ) );

  composerLabelElem.setAttribute( "halign", mHAlignment );
  composerLabelElem.setAttribute( "valign", mVAlignment );
  composerLabelElem.setAttribute( "id", mId );


  //font
  QDomElement labelFontElem = doc.createElement( "LabelFont" );
  labelFontElem.setAttribute( "description", mFont.toString() );
  composerLabelElem.appendChild( labelFontElem );

  //font color
  QDomElement fontColorElem = doc.createElement( "FontColor" );
  fontColorElem.setAttribute( "red", mFontColor.red() );
  fontColorElem.setAttribute( "green", mFontColor.green() );
  fontColorElem.setAttribute( "blue", mFontColor.blue() );
  composerLabelElem.appendChild( fontColorElem );

  elem.appendChild( composerLabelElem );
  return _writeXML( composerLabelElem, doc );
}

bool QgsComposerLabel::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  QString alignment;

  if ( itemElem.isNull() )
  {
    return false;
  }

  //restore label specific properties

  //text
  mText = itemElem.attribute( "labelText" );

  //margin
  mMargin = itemElem.attribute( "margin" ).toDouble();

  //Horizontal alignment
  mHAlignment = ( Qt::AlignmentFlag )( itemElem.attribute( "halign" ).toInt() );

  //Vertical alignment
  mVAlignment = ( Qt::AlignmentFlag )( itemElem.attribute( "valign" ).toInt() );

  //id
  mId = itemElem.attribute( "id", "" );

  //font
  QDomNodeList labelFontList = itemElem.elementsByTagName( "LabelFont" );
  if ( labelFontList.size() > 0 )
  {
    QDomElement labelFontElem = labelFontList.at( 0 ).toElement();
    mFont.fromString( labelFontElem.attribute( "description" ) );
  }

  //font color
  QDomNodeList fontColorList = itemElem.elementsByTagName( "FontColor" );
  if ( fontColorList.size() > 0 )
  {
    QDomElement fontColorElem = fontColorList.at( 0 ).toElement();
    int red = fontColorElem.attribute( "red", "0" ).toInt();
    int green = fontColorElem.attribute( "green", "0" ).toInt();
    int blue = fontColorElem.attribute( "blue", "0" ).toInt();
    mFontColor = QColor( red, green, blue );
  }
  else
  {
    mFontColor = QColor( 0, 0, 0 );
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
