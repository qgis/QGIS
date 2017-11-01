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

QgsComposerTableColumn::QgsComposerTableColumn( const QString &heading )
  : mBackgroundColor( Qt::transparent )
  , mHeading( heading )
{

}

bool QgsComposerTableColumn::writeXml( QDomElement &columnElem, QDomDocument &doc ) const
{
  //background color
  QDomElement bgColorElem = doc.createElement( QStringLiteral( "backgroundColor" ) );
  bgColorElem.setAttribute( QStringLiteral( "red" ), QString::number( mBackgroundColor.red() ) );
  bgColorElem.setAttribute( QStringLiteral( "green" ), QString::number( mBackgroundColor.green() ) );
  bgColorElem.setAttribute( QStringLiteral( "blue" ), QString::number( mBackgroundColor.blue() ) );
  bgColorElem.setAttribute( QStringLiteral( "alpha" ), QString::number( mBackgroundColor.alpha() ) );
  columnElem.appendChild( bgColorElem );

  columnElem.setAttribute( QStringLiteral( "hAlignment" ), mHAlignment );
  columnElem.setAttribute( QStringLiteral( "vAlignment" ), mVAlignment );

  columnElem.setAttribute( QStringLiteral( "heading" ), mHeading );
  columnElem.setAttribute( QStringLiteral( "attribute" ), mAttribute );

  columnElem.setAttribute( QStringLiteral( "sortByRank" ), QString::number( mSortByRank ) );
  columnElem.setAttribute( QStringLiteral( "sortOrder" ), QString::number( mSortOrder ) );

  columnElem.setAttribute( QStringLiteral( "width" ), QString::number( mWidth ) );

  return true;
}

bool QgsComposerTableColumn::readXml( const QDomElement &columnElem )
{
  mHAlignment = static_cast< Qt::AlignmentFlag >( columnElem.attribute( QStringLiteral( "hAlignment" ), QString::number( Qt::AlignLeft ) ).toInt() );
  mVAlignment = static_cast< Qt::AlignmentFlag >( columnElem.attribute( QStringLiteral( "vAlignment" ), QString::number( Qt::AlignVCenter ) ).toInt() );
  mHeading = columnElem.attribute( QStringLiteral( "heading" ), QLatin1String( "" ) );
  mAttribute = columnElem.attribute( QStringLiteral( "attribute" ), QLatin1String( "" ) );
  mSortByRank = columnElem.attribute( QStringLiteral( "sortByRank" ), QStringLiteral( "0" ) ).toInt();
  mSortOrder = static_cast< Qt::SortOrder >( columnElem.attribute( QStringLiteral( "sortOrder" ), QString::number( Qt::AscendingOrder ) ).toInt() );
  mWidth = columnElem.attribute( QStringLiteral( "width" ), QStringLiteral( "0.0" ) ).toDouble();

  QDomNodeList bgColorList = columnElem.elementsByTagName( QStringLiteral( "backgroundColor" ) );
  if ( !bgColorList.isEmpty() )
  {
    QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int bgRed, bgGreen, bgBlue, bgAlpha;
    bgRed = bgColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    bgGreen = bgColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    bgBlue = bgColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    bgAlpha = bgColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );
    if ( redOk && greenOk && blueOk && alphaOk )
    {
      mBackgroundColor = QColor( bgRed, bgGreen, bgBlue, bgAlpha );
    }
  }

  return true;
}

QgsComposerTableColumn *QgsComposerTableColumn::clone()
{
  QgsComposerTableColumn *newColumn = new QgsComposerTableColumn;
  newColumn->setAttribute( mAttribute );
  newColumn->setHeading( mHeading );
  newColumn->setHAlignment( mHAlignment );
  newColumn->setVAlignment( mVAlignment );
  newColumn->setSortByRank( mSortByRank );
  newColumn->setSortOrder( mSortOrder );
  newColumn->setWidth( mWidth );
  return newColumn;
}
