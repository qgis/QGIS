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
#include "qgsnumericformat.h"
#include "qgsnumericformatregistry.h"
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
  , mRowSpan( other.mRowSpan )
  , mColumnSpan( other.mColumnSpan )
{}

QgsTableCell::~QgsTableCell() = default;

QgsTableCell &QgsTableCell::operator=( const QgsTableCell &other )
{
  if ( &other == this )
    return *this;

  mContent = other.mContent;
  mBackgroundColor = other.mBackgroundColor;
  mForegroundColor = other.mForegroundColor;
  mTextFormat = other.mTextFormat;
  mFormat.reset( other.mFormat ? other.mFormat->clone() : nullptr );
  mHAlign = other.mHAlign;
  mVAlign = other.mVAlign;
  mRowSpan = other.mRowSpan;
  mColumnSpan = other.mColumnSpan;
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
  res.insert( u"content"_s, mContent );
  res.insert( u"background"_s, mBackgroundColor );
  res.insert( u"foreground"_s, mForegroundColor );
  if ( mFormat )
  {
    res.insert( u"format_type"_s, mFormat->id() );
    res.insert( u"format"_s, mFormat->configuration( context ) );
  }

  if ( mTextFormat.isValid() )
  {
    QDomDocument textDoc;
    const QDomElement textElem = mTextFormat.writeXml( textDoc, context );
    textDoc.appendChild( textElem );
    res.insert( u"text_format"_s, textDoc.toString() );
  }

  res.insert( u"halign"_s, static_cast< int >( mHAlign ) );
  res.insert( u"valign"_s, static_cast< int >( mVAlign ) );
  if ( mRowSpan > 1 )
    res.insert( u"row_span"_s, mRowSpan );
  if ( mColumnSpan > 1 )
    res.insert( u"column_span"_s, mColumnSpan );

  return res;
}

void QgsTableCell::setProperties( const QVariantMap &properties, const QgsReadWriteContext &context )
{
  mContent = properties.value( u"content"_s );
  mBackgroundColor = properties.value( u"background"_s ).value< QColor >();
  mForegroundColor = properties.value( u"foreground"_s ).value< QColor >();

  QDomDocument doc;
  QDomElement elem;
  const QString textXml = properties.value( u"text_format"_s ).toString();
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

  if ( properties.contains( u"format_type"_s ) )
  {

    mFormat.reset( QgsApplication::numericFormatRegistry()->create( properties.value( u"format_type"_s ).toString(),
                   properties.value( u"format"_s ).toMap(),
                   context ) );
  }
  else
  {
    mFormat.reset();
  }

  mHAlign = static_cast< Qt::Alignment >( properties.value( u"halign"_s, Qt::AlignLeft ).toInt() );
  mVAlign = static_cast< Qt::Alignment >( properties.value( u"valign"_s, Qt::AlignVCenter ).toInt() );

  mRowSpan = properties.value( u"row_span"_s, 1 ).toInt();
  mColumnSpan = properties.value( u"column_span"_s, 1 ).toInt();
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

void QgsTableCell::setSpan( int rowSpan, int columnSpan )
{
  mRowSpan = rowSpan;
  mColumnSpan = columnSpan;
}
