/***************************************************************************
  qgstextcharacterformat.cpp
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

#include "qgstextcharacterformat.h"

#include <QTextCharFormat>

QgsTextCharacterFormat::QgsTextCharacterFormat( const QTextCharFormat &format )
  : mTextColor( format.hasProperty( QTextFormat::ForegroundBrush ) ? format.foreground().color() : QColor() )
#if 0 // settings which affect font metrics are disabled for now
  , mFontWeight( format.hasProperty( QTextFormat::FontWeight ) ? format.fontWeight() : -1 )
  , mItalic( format.hasProperty( QTextFormat::FontItalic ) ? ( format.fontItalic() ? BooleanValue::True : BooleanValue::False ) : BooleanValue::NotSet )
#endif
  , mStrikethrough( format.hasProperty( QTextFormat::FontStrikeOut ) ? ( format.fontStrikeOut() ? BooleanValue::True : BooleanValue::False ) : BooleanValue::NotSet )
  , mUnderline( format.hasProperty( QTextFormat::FontUnderline ) ? ( format.fontUnderline() ? BooleanValue::True : BooleanValue::False ) : BooleanValue::NotSet )
  , mOverline( format.hasProperty( QTextFormat::FontOverline ) ? ( format.fontOverline() ? BooleanValue::True : BooleanValue::False ) : BooleanValue::NotSet )
{

}

QColor QgsTextCharacterFormat::textColor() const
{
  return mTextColor;
}

void QgsTextCharacterFormat::setTextColor( const QColor &textColor )
{
  mTextColor = textColor;
}

QgsTextCharacterFormat::BooleanValue QgsTextCharacterFormat::strikeOut() const
{
  return mStrikethrough;
}

void QgsTextCharacterFormat::setStrikeOut( BooleanValue strikethrough )
{
  mStrikethrough = strikethrough;
}

QgsTextCharacterFormat::BooleanValue QgsTextCharacterFormat::underline() const
{
  return mUnderline;
}

void QgsTextCharacterFormat::setUnderline( BooleanValue underline )
{
  mUnderline = underline;
}

QgsTextCharacterFormat::BooleanValue QgsTextCharacterFormat::overline() const
{
  return mOverline;
}

void QgsTextCharacterFormat::setOverline( QgsTextCharacterFormat::BooleanValue enabled )
{
  mOverline = enabled;
}

void QgsTextCharacterFormat::updateFontForFormat( QFont &font ) const
{
  if ( mUnderline != BooleanValue::NotSet )
    font.setUnderline( mUnderline == QgsTextCharacterFormat::BooleanValue::True );
  if ( mOverline != BooleanValue::NotSet )
    font.setOverline( mOverline == QgsTextCharacterFormat::BooleanValue::True );
  if ( mStrikethrough != QgsTextCharacterFormat::BooleanValue::NotSet )
    font.setStrikeOut( mStrikethrough == QgsTextCharacterFormat::BooleanValue::True );

#if 0 // settings which affect font metrics are disabled for now
  if ( mItalic != QgsTextCharacterFormat::BooleanValue::NotSet )
    font.setItalic( mItalic == QgsTextCharacterFormat::BooleanValue::True );
  if ( mFontWeight != -1 )
    font.setWeight( mFontWeight );
#endif
}

#if 0 // settings which affect font metrics are disabled for now
QgsTextCharacterFormat::BooleanValue QgsTextCharacterFormat::italic() const
{
  return mItalic;
}

void QgsTextCharacterFormat::setItalic( QgsTextCharacterFormat::BooleanValue enabled )
{
  mItalic = enabled;
}

int QgsTextCharacterFormat::fontWeight() const
{
  return mFontWeight;
}

void QgsTextCharacterFormat::setFontWeight( int fontWeight )
{
  mFontWeight = fontWeight;
}
#endif
