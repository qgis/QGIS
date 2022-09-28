/***************************************************************************
  qgstextdocument.cpp
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

#include "qgstextdocument.h"
#include "qgis.h"
#include "qgsstringutils.h"
#include "qgstextblock.h"
#include "qgstextfragment.h"

#include <QTextDocument>
#include <QTextBlock>


QgsTextDocument::~QgsTextDocument() = default;

QgsTextDocument::QgsTextDocument() = default;

QgsTextDocument::QgsTextDocument( const QgsTextBlock &block )
{
  mBlocks.append( block );
}

QgsTextDocument::QgsTextDocument( const QgsTextFragment &fragment )
{
  mBlocks.append( QgsTextBlock( fragment ) );
}

QgsTextDocument QgsTextDocument::fromPlainText( const QStringList &lines )
{
  QgsTextDocument document;
  document.reserve( lines.size() );
  for ( const QString &line : lines )
    document.append( QgsTextBlock( QgsTextFragment( line ) ) );
  return document;
}

QgsTextDocument QgsTextDocument::fromHtml( const QStringList &lines )
{
  QgsTextDocument document;

  document.reserve( lines.size() );
  for ( const QString &line : lines )
  {
    // QTextDocument is a very heavy way of parsing HTML + css (it's heavily geared toward an editable text document,
    // and includes a LOT of calculations we don't need, when all we're after is a HTML + CSS style parser).
    // TODO - try to find an alternative library we can use here
    QTextDocument sourceDoc;
    sourceDoc.setHtml( line );

    QTextBlock sourceBlock = sourceDoc.firstBlock();
    while ( true )
    {
      auto it = sourceBlock.begin();
      QgsTextBlock block;
      while ( !it.atEnd() )
      {
        const QTextFragment fragment = it.fragment();
        if ( fragment.isValid() )
        {
          block.append( QgsTextFragment( fragment ) );
        }
        it++;
      }
      if ( !block.empty() )
        document.append( block );

      sourceBlock = sourceBlock.next();
      if ( !sourceBlock.isValid() )
        break;
    }
  }
  return document;
}

void QgsTextDocument::append( const QgsTextBlock &block )
{
  mBlocks.append( block );
}

void QgsTextDocument::append( QgsTextBlock &&block )
{
  mBlocks.push_back( block );
}

void QgsTextDocument::reserve( int count )
{
  mBlocks.reserve( count );
}

const QgsTextBlock &QgsTextDocument::at( int i ) const
{
  return mBlocks.at( i );
}

QgsTextBlock &QgsTextDocument::operator[]( int i )
{
  return mBlocks[i];
}

int QgsTextDocument::size() const
{
  return mBlocks.size();
}

QStringList QgsTextDocument::toPlainText() const
{
  QStringList textLines;
  textLines.reserve( mBlocks.size() );
  for ( const QgsTextBlock &block : mBlocks )
  {
    QString line;
    for ( const QgsTextFragment &fragment : block )
    {
      line.append( fragment.text() );
    }
    textLines << line;
  }
  return textLines;
}

void QgsTextDocument::splitLines( const QString &wrapCharacter, int autoWrapLength, bool useMaxLineLengthWhenAutoWrapping )
{
  const QVector< QgsTextBlock > prevBlocks = mBlocks;
  mBlocks.clear();
  mBlocks.reserve( prevBlocks.size() );
  for ( const QgsTextBlock &block : prevBlocks )
  {
    QgsTextBlock destinationBlock;
    for ( const QgsTextFragment &fragment : block )
    {
      QStringList thisParts;
      if ( !wrapCharacter.isEmpty() && wrapCharacter != QLatin1String( "\n" ) )
      {
        //wrap on both the wrapchr and new line characters
        const QStringList lines = fragment.text().split( wrapCharacter );
        for ( const QString &line : lines )
        {
          thisParts.append( line.split( '\n' ) );
        }
      }
      else
      {
        thisParts = fragment.text().split( '\n' );
      }

      // apply auto wrapping to each manually created line
      if ( autoWrapLength != 0 )
      {
        QStringList autoWrappedLines;
        autoWrappedLines.reserve( thisParts.count() );
        for ( const QString &line : std::as_const( thisParts ) )
        {
          autoWrappedLines.append( QgsStringUtils::wordWrap( line, autoWrapLength, useMaxLineLengthWhenAutoWrapping ).split( '\n' ) );
        }
        thisParts = autoWrappedLines;
      }

      if ( thisParts.empty() )
        continue;
      else if ( thisParts.size() == 1 )
        destinationBlock.append( fragment );
      else
      {
        if ( !thisParts.at( 0 ).isEmpty() )
          destinationBlock.append( QgsTextFragment( thisParts.at( 0 ), fragment.characterFormat() ) );

        append( destinationBlock );
        destinationBlock.clear();
        for ( int i = 1 ; i < thisParts.size() - 1; ++i )
        {
          append( QgsTextBlock( QgsTextFragment( thisParts.at( i ), fragment.characterFormat() ) ) );
        }
        destinationBlock.append( QgsTextFragment( thisParts.at( thisParts.size() - 1 ), fragment.characterFormat() ) );
      }
    }
    append( destinationBlock );
  }
}

void QgsTextDocument::applyCapitalization( Qgis::Capitalization capitalization )
{
  for ( QgsTextBlock &block : mBlocks )
  {
    block.applyCapitalization( capitalization );
  }
}

///@cond PRIVATE
QVector< QgsTextBlock >::const_iterator QgsTextDocument::begin() const
{
  return mBlocks.begin();
}

QVector< QgsTextBlock >::const_iterator QgsTextDocument::end() const
{
  return mBlocks.end();
}
///@endcond
