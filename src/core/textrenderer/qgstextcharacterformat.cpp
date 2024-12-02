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

Qgis::TextCharacterVerticalAlignment convertTextCharFormatVAlign( const QTextCharFormat &format, bool &set )
{
  set = format.hasProperty( QTextFormat::TextVerticalAlignment );
  switch ( format.verticalAlignment() )
  {
    case QTextCharFormat::AlignNormal:
      return Qgis::TextCharacterVerticalAlignment::Normal;
    case QTextCharFormat::AlignSuperScript:
      return Qgis::TextCharacterVerticalAlignment::SuperScript;
    case QTextCharFormat::AlignSubScript:
      return Qgis::TextCharacterVerticalAlignment::SubScript;

    // not yet supported
    case QTextCharFormat::AlignMiddle:
    case QTextCharFormat::AlignTop:
    case QTextCharFormat::AlignBottom:
    case QTextCharFormat::AlignBaseline:
      set = false;
      return Qgis::TextCharacterVerticalAlignment::Normal;
  }
  BUILTIN_UNREACHABLE
}

QgsTextCharacterFormat::QgsTextCharacterFormat( const QTextCharFormat &format )
  : mTextColor( format.hasProperty( QTextFormat::ForegroundBrush ) ? format.foreground().color() : QColor() )
  , mFontWeight( format.hasProperty( QTextFormat::FontWeight ) ? format.fontWeight() : -1 )
  , mStyleName( format.font().styleName() )
  , mItalic( format.hasProperty( QTextFormat::FontItalic ) ? ( format.fontItalic() ? BooleanValue::SetTrue : BooleanValue::SetFalse ) : BooleanValue::NotSet )
  , mFontPointSize( format.hasProperty( QTextFormat::FontPointSize ) ? format.fontPointSize() : - 1 )
  , mWordSpacing( format.hasProperty( QTextFormat::FontWordSpacing ) ? format.fontWordSpacing() : std::numeric_limits< double >::quiet_NaN() )
  , mStrikethrough( format.hasProperty( QTextFormat::FontStrikeOut ) ? ( format.fontStrikeOut() ? BooleanValue::SetTrue : BooleanValue::SetFalse ) : BooleanValue::NotSet )
  , mUnderline( format.hasProperty( QTextFormat::FontUnderline ) ? ( format.fontUnderline() ? BooleanValue::SetTrue : BooleanValue::SetFalse ) : BooleanValue::NotSet )
  , mOverline( format.hasProperty( QTextFormat::FontOverline ) ? ( format.fontOverline() ? BooleanValue::SetTrue : BooleanValue::SetFalse ) : BooleanValue::NotSet )
  , mBackgroundBrush( format.background() )
  , mBackgroundPath( format.background().style() == Qt::NoBrush ? format.stringProperty( QTextFormat::BackgroundImageUrl ) : QString() )
{
  mVerticalAlign = convertTextCharFormatVAlign( format, mHasVerticalAlignSet );

  if ( format.hasProperty( QTextFormat::FontFamily ) )
  {
    mFontFamily = format.fontFamily();
  }
  if ( mFontFamily.isEmpty() && format.hasProperty( QTextFormat::FontFamilies ) )
  {
    const QStringList families = format.fontFamilies().toStringList();
    if ( !families.isEmpty() )
      mFontFamily = families.at( 0 );
  }
  if ( format.isImageFormat() )
  {
    const QTextImageFormat imageFormat = format.toImageFormat();
    mImagePath = imageFormat.name();
    mImageSize = QSizeF( imageFormat.width(), imageFormat.height() );
  }
}

void QgsTextCharacterFormat::overrideWith( const QgsTextCharacterFormat &other )
{
  if ( !mTextColor.isValid() && other.mTextColor.isValid() )
    mTextColor = other.mTextColor;
  if ( mFontPointSize == -1 && other.mFontPointSize != -1 )
    mFontPointSize = other.mFontPointSize;
  if ( mFontPercentageSize == -1 && other.mFontPercentageSize != -1 )
    mFontPercentageSize = other.mFontPercentageSize;
  if ( std::isnan( mWordSpacing ) )
    mWordSpacing = other.mWordSpacing;
  if ( mFontFamily.isEmpty() && !other.mFontFamily.isEmpty() )
    mFontFamily = other.mFontFamily;
  if ( mStrikethrough == BooleanValue::NotSet && other.mStrikethrough != BooleanValue::NotSet )
    mStrikethrough = other.mStrikethrough;
  if ( mUnderline == BooleanValue::NotSet && other.mUnderline != BooleanValue::NotSet )
    mUnderline = other.mUnderline;
  if ( mOverline == BooleanValue::NotSet && other.mOverline != BooleanValue::NotSet )
    mOverline = other.mOverline;
  if ( mItalic == BooleanValue::NotSet && other.mItalic != BooleanValue::NotSet )
    mItalic = other.mItalic;
  if ( mFontWeight == -1 && other.mFontWeight != -1 )
    mFontWeight = other.mFontWeight;
  if ( mStyleName.isEmpty() && ! other.mStyleName.isEmpty() )
    mStyleName = other.mStyleName;
  if ( mHasVerticalAlignSet && other.hasVerticalAlignmentSet() )
  {
    mVerticalAlign = other.mVerticalAlign;
    mHasVerticalAlignSet = true;
  }
  if ( mBackgroundBrush.style() == Qt::NoBrush && mBackgroundPath.isEmpty() && other.mBackgroundBrush.style() != Qt::NoBrush )
    mBackgroundBrush = other.mBackgroundBrush;
  if ( mBackgroundBrush.style() == Qt::NoBrush  && mBackgroundPath.isEmpty() && !other.mBackgroundPath.isEmpty() )
    mBackgroundPath = other.mBackgroundPath;
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

double QgsTextCharacterFormat::fontPercentageSize() const
{
  return mFontPercentageSize;
}

void QgsTextCharacterFormat::setFontPercentageSize( double size )
{
  mFontPercentageSize = size;
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

QString QgsTextCharacterFormat::imagePath() const
{
  return mImagePath;
}

void QgsTextCharacterFormat::setImagePath( const QString &path )
{
  mImagePath = path;
}

QSizeF QgsTextCharacterFormat::imageSize() const
{
  return mImageSize;
}

void QgsTextCharacterFormat::setImageSize( const QSizeF &size )
{
  mImageSize = size;
}

void QgsTextCharacterFormat::updateFontForFormat( QFont &font, const QgsRenderContext &context, const double scaleFactor ) const
{
  // important -- MUST set family first
  if ( !mFontFamily.isEmpty() )
    QgsFontUtils::setFontFamily( font, mFontFamily );

  if ( mFontPointSize != -1 )
    font.setPixelSize( scaleFactor * context.convertToPainterUnits( mFontPointSize, Qgis::RenderUnit::Points ) );

  if ( mFontPercentageSize != -1 )
    font.setPixelSize( font.pixelSize() * mFontPercentageSize );

  if ( mItalic != QgsTextCharacterFormat::BooleanValue::NotSet )
    font.setItalic( mItalic == QgsTextCharacterFormat::BooleanValue::SetTrue );

  if ( mFontWeight != - 1 )
  {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    font.setWeight( mFontWeight );
#else
    if ( mFontWeight <= 150 )
      font.setWeight( QFont::Thin );
    else if ( mFontWeight <= 250 )
      font.setWeight( QFont::ExtraLight );
    else if ( mFontWeight <= 350 )
      font.setWeight( QFont::Light );
    else if ( mFontWeight <= 450 )
      font.setWeight( QFont::Normal );
    else if ( mFontWeight <= 550 )
      font.setWeight( QFont::Medium );
    else if ( mFontWeight <= 650 )
      font.setWeight( QFont::DemiBold );
    else if ( mFontWeight <= 750 )
      font.setWeight( QFont::Bold );
    else if ( mFontWeight <= 850 )
      font.setWeight( QFont::ExtraBold );
    else
      font.setWeight( QFont::Black );
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

  if ( !std::isnan( mWordSpacing ) )
  {
    font.setWordSpacing( scaleFactor * context.convertToPainterUnits( mWordSpacing, Qgis::RenderUnit::Points ) );
  }
}

QString QgsTextCharacterFormat::backgroundImagePath() const
{
  return mBackgroundPath;
}

void QgsTextCharacterFormat::setBackgroundImagePath( const QString &path )
{
  mBackgroundPath = path;
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

double QgsTextCharacterFormat::wordSpacing() const
{
  return mWordSpacing;
}

void QgsTextCharacterFormat::setWordSpacing( double spacing )
{
  mWordSpacing = spacing;
}

bool QgsTextCharacterFormat::hasBackground() const
{
  return mBackgroundBrush.style() != Qt::NoBrush || !mBackgroundPath.isEmpty();
}

QBrush QgsTextCharacterFormat::backgroundBrush() const
{
  return mBackgroundBrush;
}

void QgsTextCharacterFormat::setBackgroundBrush( const QBrush &brush )
{
  mBackgroundBrush = brush;
}
