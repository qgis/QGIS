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
{
  mHorizontalAlign = convertTextBlockFormatAlign( format, mHasHorizontalAlignSet );
}

void QgsTextBlockFormat::overrideWith( const QgsTextBlockFormat &other )
{
  if ( mHasHorizontalAlignSet && other.hasHorizontalAlignmentSet() )
  {
    mHorizontalAlign = other.mHorizontalAlign;
    mHasHorizontalAlignSet = true;
  }
}

void QgsTextBlockFormat::updateFontForFormat( QFont &, const QgsRenderContext &, const double ) const
{

}
