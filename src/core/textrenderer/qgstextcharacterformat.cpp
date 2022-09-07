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
#include "qgsrendercontext.h"
#include "qgsfontutils.h"

#include <QTextCharFormat>

QgsTextCharacterFormat::QgsTextCharacterFormat( const QTextCharFormat &format )
  : mTextColor( format.hasProperty( QTextFormat::ForegroundBrush ) ? format.foreground().color() : QColor() )
  , mFontWeight( format.hasProperty( QTextFormat::FontWeight ) ? format.fontWeight() : -1 )
  , mStyleName( format.font().styleName() )
  , mItalic( format.hasProperty( QTextFormat::FontItalic ) ? ( format.fontItalic() ? BooleanValue::SetTrue : BooleanValue::SetFalse ) : BooleanValue::NotSet )
  , mFontPointSize( format.hasProperty( QTextFormat::FontPointSize ) ? format.fontPointSize() : - 1 )
  , mFontFamily( format.hasProperty( QTextFormat::FontFamily ) ? format.fontFamily() : QString() )
  , mStrikethrough( format.hasProperty( QTextFormat::FontStrikeOut ) ? ( format.fontStrikeOut() ? BooleanValue::SetTrue : BooleanValue::SetFalse ) : BooleanValue::NotSet )
  , mUnderline( format.hasProperty( QTextFormat::FontUnderline ) ? ( format.fontUnderline() ? BooleanValue::SetTrue : BooleanValue::SetFalse ) : BooleanValue::NotSet )
  , mOverline( format.hasProperty( QTextFormat::FontOverline ) ? ( format.fontOverline() ? BooleanValue::SetTrue : BooleanValue::SetFalse ) : BooleanValue::NotSet )
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

double QgsTextCharacterFormat::fontPointSize() const
{
  return mFontPointSize;
}

void QgsTextCharacterFormat::setFontPointSize( double size )
{
  mFontPointSize = size;
}

QString QgsTextCharacterFormat::family() const
{
  return mFontFamily;
}

void QgsTextCharacterFormat::setFamily( const QString &family )
{
  mFontFamily = family;
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

void QgsTextCharacterFormat::updateFontForFormat( QFont &font, const QgsRenderContext &context, const double scaleFactor ) const
{
  // important -- MUST set family first
  if ( !mFontFamily.isEmpty() )
    font.setFamily( mFontFamily );

  if ( mFontPointSize != -1 )
    font.setPixelSize( scaleFactor * context.convertToPainterUnits( mFontPointSize, QgsUnitTypes::RenderPoints ) );

  if ( mItalic != QgsTextCharacterFormat::BooleanValue::NotSet )
    font.setItalic( mItalic == QgsTextCharacterFormat::BooleanValue::SetTrue );

  if ( mFontWeight != - 1 )
  {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    font.setWeight( mFontWeight );
#else
    if ( mFontWeight <= 150 )
      newFont.setWeight( QFont::Thin );
    else if ( mFontWeight <= 250 )
      newFont.setWeight( QFont::ExtraLight );
    else if ( mFontWeight <= 350 )
      newFont.setWeight( QFont::Light );
    else if ( mFontWeight <= 450 )
      newFont.setWeight( QFont::Normal );
    else if ( mFontWeight <= 550 )
      newFont.setWeight( QFont::Medium );
    else if ( mFontWeight <= 650 )
      newFont.setWeight( QFont::DemiBold );
    else if ( mFontWeight <= 750 )
      newFont.setWeight( QFont::Bold );
    else if ( mFontWeight <= 850 )
      newFont.setWeight( QFont::ExtraBold );
    else
      newFont.setWeight( QFont::Black );
#endif

    // depending on the font, platform, and the phase of the moon, we need to both set the font weight AND the style name
    // in order to get correct rendering!
    font.setStyleName( mStyleName );
  }

  if ( mUnderline != BooleanValue::NotSet )
    font.setUnderline( mUnderline == QgsTextCharacterFormat::BooleanValue::SetTrue );
  if ( mOverline != BooleanValue::NotSet )
    font.setOverline( mOverline == QgsTextCharacterFormat::BooleanValue::SetTrue );
  if ( mStrikethrough != QgsTextCharacterFormat::BooleanValue::NotSet )
    font.setStrikeOut( mStrikethrough == QgsTextCharacterFormat::BooleanValue::SetTrue );
}

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
