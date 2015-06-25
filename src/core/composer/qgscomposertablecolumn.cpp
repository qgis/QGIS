/***************************************************************************
                         qgscomposertablecolumn.cpp
                         --------------------------
    begin                : May 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgscomposertablecolumn.h"

QgsComposerTableColumn::QgsComposerTableColumn( const QString& heading ) :
    mBackgroundColor( Qt::transparent ),
    mHAlignment( Qt::AlignLeft ),
    mHeading( heading ),
    mSortByRank( 0 ),
    mSortOrder( Qt::AscendingOrder ),
    mWidth( 0.0 )
{

}


QgsComposerTableColumn::~QgsComposerTableColumn()
{

}

bool QgsComposerTableColumn::writeXML( QDomElement& columnElem, QDomDocument& doc ) const
{
  //background color
  QDomElement bgColorElem = doc.createElement( "backgroundColor" );
  bgColorElem.setAttribute( "red", QString::number( mBackgroundColor.red() ) );
  bgColorElem.setAttribute( "green", QString::number( mBackgroundColor.green() ) );
  bgColorElem.setAttribute( "blue", QString::number( mBackgroundColor.blue() ) );
  bgColorElem.setAttribute( "alpha", QString::number( mBackgroundColor.alpha() ) );
  columnElem.appendChild( bgColorElem );

  columnElem.setAttribute( "hAlignment", mHAlignment );

  columnElem.setAttribute( "heading", mHeading );
  columnElem.setAttribute( "attribute", mAttribute );

  columnElem.setAttribute( "sortByRank", QString::number( mSortByRank ) );
  columnElem.setAttribute( "sortOrder", QString::number( mSortOrder ) );

  columnElem.setAttribute( "width", QString::number( mWidth ) );

  return true;
}

bool QgsComposerTableColumn::readXML( const QDomElement& columnElem )
{
  mHAlignment = ( Qt::AlignmentFlag )columnElem.attribute( "hAlignment", QString::number( Qt::AlignLeft ) ).toInt();
  mHeading = columnElem.attribute( "heading", "" );
  mAttribute = columnElem.attribute( "attribute", "" );
  mSortByRank = columnElem.attribute( "sortByRank", "0" ).toInt();
  mSortOrder = ( Qt::SortOrder )columnElem.attribute( "sortOrder", QString::number( Qt::AscendingOrder ) ).toInt();
  mWidth = columnElem.attribute( "width", "0.0" ).toDouble();

  QDomNodeList bgColorList = columnElem.elementsByTagName( "backgroundColor" );
  if ( bgColorList.size() > 0 )
  {
    QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int bgRed, bgGreen, bgBlue, bgAlpha;
    bgRed = bgColorElem.attribute( "red" ).toDouble( &redOk );
    bgGreen = bgColorElem.attribute( "green" ).toDouble( &greenOk );
    bgBlue = bgColorElem.attribute( "blue" ).toDouble( &blueOk );
    bgAlpha = bgColorElem.attribute( "alpha" ).toDouble( &alphaOk );
    if ( redOk && greenOk && blueOk && alphaOk )
    {
      mBackgroundColor = QColor( bgRed, bgGreen, bgBlue, bgAlpha );
    }
  }

  return true;
}

QgsComposerTableColumn* QgsComposerTableColumn::clone()
{
  QgsComposerTableColumn* newColumn = new QgsComposerTableColumn;
  newColumn->setAttribute( mAttribute );
  newColumn->setHeading( mHeading );
  newColumn->setHAlignment( mHAlignment );
  newColumn->setSortByRank( mSortByRank );
  newColumn->setSortOrder( mSortOrder );
  newColumn->setWidth( mWidth );
  return newColumn;
}
