/***************************************************************************
  qgstextfragment.cpp
  -----------------
   begin                : May 2020
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextfragment.h"
#include <QFontMetricsF>
#include <QTextFragment>
#include "qgsstringutils.h"

QgsTextFragment::QgsTextFragment( const QString &text, const QgsTextCharacterFormat &format )
  : mText( text )
  , mCharFormat( format )
{}

QgsTextFragment::QgsTextFragment( const QTextFragment &fragment )
  : mText( fragment.text() )
  , mCharFormat( QgsTextCharacterFormat( fragment.charFormat() ) )
{

}

QString QgsTextFragment::text() const
{
  return mText;
}

void QgsTextFragment::setText( const QString &text )
{
  mText = text;
}

void QgsTextFragment::setCharacterFormat( const QgsTextCharacterFormat &charFormat )
{
  mCharFormat = charFormat;
}

double QgsTextFragment::horizontalAdvance( const QFont &font, bool fontHasBeenUpdatedForFragment, double scaleFactor ) const
{
  if ( fontHasBeenUpdatedForFragment )
  {
    const QFontMetricsF fm( font );
    return fm.horizontalAdvance( mText );
  }
  else
  {
    QFont updatedFont = font;
    mCharFormat.updateFontForFormat( updatedFont, scaleFactor );
    const QFontMetricsF fm( updatedFont );
    return fm.horizontalAdvance( mText );
  }
}

void QgsTextFragment::applyCapitalization( Qgis::Capitalization capitalization )
{
  mText = QgsStringUtils::capitalize( mText, capitalization );
}

