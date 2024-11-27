/***************************************************************************
  qgstextblock.cpp
  ---------------
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

#include "qgstextblock.h"
#include "qgstextfragment.h"

#include <QSizeF>

QgsTextBlock::QgsTextBlock( const QgsTextFragment &fragment )
{
  mFragments.append( fragment );
}

QgsTextBlock QgsTextBlock::fromPlainText( const QString &text, const QgsTextCharacterFormat &format )
{
  if ( text.contains( '\t' ) )
  {
    // split line by tab characters, each tab should be a
    // fragment by itself
    QgsTextBlock block;
    const QStringList tabSplit = text.split( '\t' );
    int index = 0;
    for ( const QString &part : tabSplit )
    {
      if ( !part.isEmpty() )
        block.append( QgsTextFragment( part, format ) );
      if ( index != tabSplit.size() - 1 )
      {
        block.append( QgsTextFragment( QString( '\t' ), format ) );
      }

      index++;
    }
    return block;
  }
  else
  {
    return QgsTextBlock( QgsTextFragment( text, format ) );
  }
}

QString QgsTextBlock::toPlainText() const
{
  QString res;
  for ( const QgsTextFragment &fragment : mFragments )
  {
    res.append( fragment.text() );
  }
  return res;
}

void QgsTextBlock::reserve( int count )
{
  mFragments.reserve( count );
}

void QgsTextBlock::append( const QgsTextFragment &fragment )
{
  mFragments.append( fragment );
}

void QgsTextBlock::append( QgsTextFragment &&fragment )
{
  mFragments.push_back( fragment );
}

void QgsTextBlock::insert( int index, const QgsTextFragment &fragment )
{
  mFragments.insert( index, fragment );
}

void QgsTextBlock::insert( int index, QgsTextFragment &&fragment )
{
  mFragments.insert( index, fragment );
}

void QgsTextBlock::clear()
{
  mFragments.clear();
}

bool QgsTextBlock::empty() const
{
  return mFragments.empty();
}

int QgsTextBlock::size() const
{
  return mFragments.size();
}

void QgsTextBlock::setBlockFormat( const QgsTextBlockFormat &format )
{
  mBlockFormat = format;
}

void QgsTextBlock::applyCapitalization( Qgis::Capitalization capitalization )
{
  for ( QgsTextFragment &fragment : mFragments )
  {
    fragment.applyCapitalization( capitalization );
  }
}

bool QgsTextBlock::hasBackgrounds() const
{
  return mBlockFormat.hasBackground()
  || std::any_of( mFragments.begin(), mFragments.end(), []( const QgsTextFragment & fragment ) { return fragment.characterFormat().hasBackground(); } );
}

const QgsTextFragment &QgsTextBlock::at( int index ) const
{
  return mFragments.at( index );
}

QgsTextFragment &QgsTextBlock::operator[]( int index )
{
  return mFragments[ index ];
}

///@cond PRIVATE
QVector< QgsTextFragment >::const_iterator QgsTextBlock::begin() const
{
  return mFragments.begin();
}

QVector< QgsTextFragment >::const_iterator QgsTextBlock::end() const
{
  return mFragments.end();
}
///@endcond
