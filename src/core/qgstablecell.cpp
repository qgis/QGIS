/***************************************************************************
    qgstablecell.h
    --------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstablecell.h"
#include "qgsapplication.h"
#include "qgsnumericformatregistry.h"
#include "qgsnumericformat.h"

QgsTableCell::QgsTableCell( const QVariant &content )
  : mContent( content )
{}

QgsTableCell::QgsTableCell( const QgsTableCell &other )
  : mContent( other.mContent )
  , mBackgroundColor( other.mBackgroundColor )
  , mForegroundColor( other.mForegroundColor )
  , mFormat( other.mFormat ? other.mFormat->clone() : nullptr )
{}

QgsTableCell::~QgsTableCell() = default;

QgsTableCell &QgsTableCell::operator=( const QgsTableCell &other )
{
  mContent = other.mContent;
  mBackgroundColor = other.mBackgroundColor;
  mForegroundColor =  other.mForegroundColor;
  mFormat.reset( other.mFormat ? other.mFormat->clone() : nullptr );
  return *this;
}

const QgsNumericFormat *QgsTableCell::numericFormat() const
{
  return mFormat.get();
}

void QgsTableCell::setNumericFormat( QgsNumericFormat *format )
{
  mFormat.reset( format );
}

QVariantMap QgsTableCell::properties( const QgsReadWriteContext &context ) const
{
  QVariantMap res;
  res.insert( QStringLiteral( "content" ), mContent );
  res.insert( QStringLiteral( "foreground" ), mForegroundColor );
  res.insert( QStringLiteral( "background" ), mBackgroundColor );
  if ( mFormat )
  {
    res.insert( QStringLiteral( "format_type" ), mFormat->id() );
    res.insert( QStringLiteral( "format" ), mFormat->configuration( context ) );
  }
  return res;
}

void QgsTableCell::setProperties( const QVariantMap &properties, const QgsReadWriteContext &context )
{
  mContent = properties.value( QStringLiteral( "content" ) );
  mForegroundColor = properties.value( QStringLiteral( "foreground" ) ).value< QColor >();
  mBackgroundColor = properties.value( QStringLiteral( "background" ) ).value< QColor >();
  if ( properties.contains( QStringLiteral( "format_type" ) ) )
  {

    mFormat.reset( QgsApplication::numericFormatRegistry()->create( properties.value( QStringLiteral( "format_type" ) ).toString(),
                   properties.value( QStringLiteral( "format" ) ).toMap(),
                   context ) );
  }
  else
  {
    mFormat.reset();
  }
}
