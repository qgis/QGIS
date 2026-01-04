/***************************************************************************
                        QgsLayoutTableColumn.cpp
                         ------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayouttablecolumn.h"

#include <memory>

#include "qgis.h"

QgsLayoutTableColumn::QgsLayoutTableColumn( const QString &heading )
  : mHeading( heading )
{}

bool QgsLayoutTableColumn::writeXml( QDomElement &columnElem, QDomDocument &doc ) const
{
  //background color
  QDomElement bgColorElem = doc.createElement( u"backgroundColor"_s );
  bgColorElem.setAttribute( u"red"_s, QString::number( mBackgroundColor.red() ) );
  bgColorElem.setAttribute( u"green"_s, QString::number( mBackgroundColor.green() ) );
  bgColorElem.setAttribute( u"blue"_s, QString::number( mBackgroundColor.blue() ) );
  bgColorElem.setAttribute( u"alpha"_s, QString::number( mBackgroundColor.alpha() ) );
  columnElem.appendChild( bgColorElem );

  columnElem.setAttribute( u"hAlignment"_s, mHAlignment );
  columnElem.setAttribute( u"vAlignment"_s, mVAlignment );

  columnElem.setAttribute( u"heading"_s, mHeading );
  columnElem.setAttribute( u"attribute"_s, mAttribute );

  columnElem.setAttribute( u"sortByRank"_s, QString::number( mSortByRank ) );
  columnElem.setAttribute( u"sortOrder"_s, QString::number( mSortOrder ) );

  columnElem.setAttribute( u"width"_s, QString::number( mWidth ) );

  return true;
}

bool QgsLayoutTableColumn::readXml( const QDomElement &columnElem )
{
  mHAlignment = static_cast< Qt::AlignmentFlag >( columnElem.attribute( u"hAlignment"_s, QString::number( Qt::AlignLeft ) ).toInt() );
  mVAlignment = static_cast< Qt::AlignmentFlag >( columnElem.attribute( u"vAlignment"_s, QString::number( Qt::AlignVCenter ) ).toInt() );
  mHeading = columnElem.attribute( u"heading"_s, QString() );
  mAttribute = columnElem.attribute( u"attribute"_s, QString() );
  mSortByRank = columnElem.attribute( u"sortByRank"_s, u"0"_s ).toInt();
  mSortOrder = static_cast< Qt::SortOrder >( columnElem.attribute( u"sortOrder"_s, QString::number( Qt::AscendingOrder ) ).toInt() );
  mWidth = columnElem.attribute( u"width"_s, u"0.0"_s ).toDouble();

  const QDomNodeList bgColorList = columnElem.elementsByTagName( u"backgroundColor"_s );
  if ( !bgColorList.isEmpty() )
  {
    const QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int bgRed, bgGreen, bgBlue, bgAlpha;
    bgRed = bgColorElem.attribute( u"red"_s ).toDouble( &redOk );
    bgGreen = bgColorElem.attribute( u"green"_s ).toDouble( &greenOk );
    bgBlue = bgColorElem.attribute( u"blue"_s ).toDouble( &blueOk );
    bgAlpha = bgColorElem.attribute( u"alpha"_s ).toDouble( &alphaOk );
    if ( redOk && greenOk && blueOk && alphaOk )
    {
      mBackgroundColor = QColor( bgRed, bgGreen, bgBlue, bgAlpha );
    }
  }

  return true;
}
