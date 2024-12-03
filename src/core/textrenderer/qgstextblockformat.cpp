/***************************************************************************
  qgstextblockformat.cpp
  -----------------
   begin                : September 2024
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

#include "qgstextblockformat.h"
#include "qgsrendercontext.h"

#include <QTextBlockFormat>


Qgis::TextHorizontalAlignment convertTextBlockFormatAlign( const QTextBlockFormat &format, bool &set )
{
  set = format.hasProperty( QTextFormat::BlockAlignment );
  if ( format.alignment() & Qt::AlignLeft )
  {
    return Qgis::TextHorizontalAlignment::Left;
  }
  else if ( format.alignment() & Qt::AlignRight )
  {
    return Qgis::TextHorizontalAlignment::Right;
  }
  else if ( format.alignment() & Qt::AlignHCenter )
  {
    return Qgis::TextHorizontalAlignment::Center;
  }
  else if ( format.alignment() & Qt::AlignJustify )
  {
    return Qgis::TextHorizontalAlignment::Justify;
  }

  set = false;
  return Qgis::TextHorizontalAlignment::Left;
}

QgsTextBlockFormat::QgsTextBlockFormat( const QTextBlockFormat &format )
  : mBackgroundBrush( format.background() )
  , mBackgroundPath( format.stringProperty( QTextFormat::BackgroundImageUrl ) )
  , mLineHeight( format.hasProperty( QTextFormat::LineHeight ) && format.lineHeightType() != QTextBlockFormat::ProportionalHeight ? format.lineHeight() : std::numeric_limits< double >::quiet_NaN() )
  , mLineHeightPercentage( format.hasProperty( QTextFormat::LineHeight ) && format.lineHeightType() == QTextBlockFormat::ProportionalHeight ? ( format.lineHeight() / 100.0 ) : std::numeric_limits< double >::quiet_NaN() )
{
  mHorizontalAlign = convertTextBlockFormatAlign( format, mHasHorizontalAlignSet );

  const double topMargin = format.hasProperty( QTextFormat::BlockTopMargin ) ? format.topMargin() : std::numeric_limits< double >::quiet_NaN();
  const double leftMargin = format.hasProperty( QTextFormat::BlockLeftMargin ) ? format.leftMargin() : std::numeric_limits< double >::quiet_NaN();
  const double rightMargin = format.hasProperty( QTextFormat::BlockRightMargin ) ? format.rightMargin() : std::numeric_limits< double >::quiet_NaN();
  const double bottomMargin = format.hasProperty( QTextFormat::BlockBottomMargin ) ? format.bottomMargin() : std::numeric_limits< double >::quiet_NaN();
  mMargins = QgsMargins( leftMargin, topMargin, rightMargin, bottomMargin );
}

void QgsTextBlockFormat::overrideWith( const QgsTextBlockFormat &other )
{
  if ( mHasHorizontalAlignSet && other.hasHorizontalAlignmentSet() )
  {
    mHorizontalAlign = other.mHorizontalAlign;
    mHasHorizontalAlignSet = true;
  }

  if ( std::isnan( mLineHeight ) )
    mLineHeight = other.mLineHeight;
  if ( std::isnan( mLineHeightPercentage ) )
    mLineHeightPercentage = other.mLineHeightPercentage;

  if ( std::isnan( mMargins.left() ) )
    mMargins.setLeft( other.mMargins.left() );
  if ( std::isnan( mMargins.right() ) )
    mMargins.setRight( other.mMargins.right() );
  if ( std::isnan( mMargins.top() ) )
    mMargins.setTop( other.mMargins.top() );
  if ( std::isnan( mMargins.bottom() ) )
    mMargins.setBottom( other.mMargins.bottom() );
}

double QgsTextBlockFormat::lineHeight() const
{
  return mLineHeight;
}

void QgsTextBlockFormat::setLineHeight( double height )
{
  mLineHeight = height;
}

double QgsTextBlockFormat::lineHeightPercentage() const
{
  return mLineHeightPercentage;
}

void QgsTextBlockFormat::setLineHeightPercentage( double height )
{
  mLineHeightPercentage = height;
}

void QgsTextBlockFormat::updateFontForFormat( QFont &, const QgsRenderContext &, const double ) const
{

}

bool QgsTextBlockFormat::hasBackground() const
{
  return mBackgroundBrush.style() != Qt::NoBrush || !mBackgroundPath.isEmpty();
}

QBrush QgsTextBlockFormat::backgroundBrush() const
{
  return mBackgroundBrush;
}

void QgsTextBlockFormat::setBackgroundBrush( const QBrush &brush )
{
  mBackgroundBrush = brush;
}

QString QgsTextBlockFormat::backgroundImagePath() const
{
  return mBackgroundPath;
}

void QgsTextBlockFormat::setBackgroundImagePath( const QString &path )
{
  mBackgroundPath = path;
}
