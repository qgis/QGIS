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
#include "qgsreadwritecontext.h"

QgsTableCell::QgsTableCell( const QVariant &content )
  : mContent( content )
{}

QgsTableCell::QgsTableCell( const QgsTableCell &other )
  : mContent( other.mContent )
  , mBackgroundColor( other.mBackgroundColor )
  , mForegroundColor( other.mForegroundColor )
  , mTextFormat( other.mTextFormat )
  , mFormat( other.mFormat ? other.mFormat->clone() : nullptr )
  , mHAlign( other.mHAlign )
  , mVAlign( other.mVAlign )
{}

QgsTableCell::~QgsTableCell() = default;

QgsTableCell &QgsTableCell::operator=( const QgsTableCell &other )
{
  mContent = other.mContent;
  mBackgroundColor = other.mBackgroundColor;
  mForegroundColor = other.mForegroundColor;
  mTextFormat = other.mTextFormat;
  mFormat.reset( other.mFormat ? other.mFormat->clone() : nullptr );
  mHAlign = other.mHAlign;
  mVAlign = other.mVAlign;
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
  res.insert( QStringLiteral( "background" ), mBackgroundColor );
  res.insert( QStringLiteral( "foreground" ), mForegroundColor );
  if ( mFormat )
  {
    res.insert( QStringLiteral( "format_type" ), mFormat->id() );
    res.insert( QStringLiteral( "format" ), mFormat->configuration( context ) );
  }

  if ( mTextFormat.isValid() )
  {
    QDomDocument textDoc;
    const QDomElement textElem = mTextFormat.writeXml( textDoc, context );
    textDoc.appendChild( textElem );
    res.insert( QStringLiteral( "text_format" ), textDoc.toString() );
  }

  res.insert( QStringLiteral( "halign" ), static_cast< int >( mHAlign ) );
  res.insert( QStringLiteral( "valign" ), static_cast< int >( mVAlign ) );

  return res;
}

void QgsTableCell::setProperties( const QVariantMap &properties, const QgsReadWriteContext &context )
{
  mContent = properties.value( QStringLiteral( "content" ) );
  mBackgroundColor = properties.value( QStringLiteral( "background" ) ).value< QColor >();
  mForegroundColor = properties.value( QStringLiteral( "foreground" ) ).value< QColor >();

  QDomDocument doc;
  QDomElement elem;
  const QString textXml = properties.value( QStringLiteral( "text_format" ) ).toString();
  if ( !textXml.isEmpty() )
  {
    doc.setContent( textXml );
    elem = doc.documentElement();
    mTextFormat.readXml( elem, context );
  }
  else
  {
    mTextFormat = QgsTextFormat();
  }

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

  mHAlign = static_cast< Qt::Alignment >( properties.value( QStringLiteral( "halign" ), Qt::AlignLeft ).toInt() );
  mVAlign = static_cast< Qt::Alignment >( properties.value( QStringLiteral( "valign" ), Qt::AlignVCenter ).toInt() );
}

Qt::Alignment QgsTableCell::horizontalAlignment() const
{
  return mHAlign;
}

void QgsTableCell::setHorizontalAlignment( Qt::Alignment alignment )
{
  mHAlign = alignment;
}

Qt::Alignment QgsTableCell::verticalAlignment() const
{
  return mVAlign;
}

void QgsTableCell::setVerticalAlignment( Qt::Alignment alignment )
{
  mVAlign = alignment;
}
