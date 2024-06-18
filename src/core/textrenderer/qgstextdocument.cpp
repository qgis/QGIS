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
  {
    document.append( QgsTextBlock::fromPlainText( line ) );
  }
  return document;
}

// Note -- must start and end with spaces, so that a tab character within
// a html or css tag doesn't mess things up. Instead, Qt will just silently
// ignore html attributes it doesn't know about, like this replacement string
#define TAB_REPLACEMENT_MARKER " ignore_me_i_am_a_tab "

QgsTextDocument QgsTextDocument::fromHtml( const QStringList &lines )
{
  QgsTextDocument document;

  document.reserve( lines.size() );

  for ( const QString &l : std::as_const( lines ) )
  {
    QString line = l;
    // QTextDocument is a very heavy way of parsing HTML + css (it's heavily geared toward an editable text document,
    // and includes a LOT of calculations we don't need, when all we're after is a HTML + CSS style parser).
    // TODO - try to find an alternative library we can use here

    QTextDocument sourceDoc;

    // QTextDocument will replace tab characters with a space. We need to hack around this
    // by first replacing it with a string which QTextDocument won't mess with, and then
    // handle these markers as tab characters in the parsed HTML document.
    line.replace( QString( '\t' ), QStringLiteral( TAB_REPLACEMENT_MARKER ) );

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
          // Search for line breaks in the fragment
          const QString fragmentText = fragment.text();
          if ( fragmentText.contains( QStringLiteral( "\u2028" ) ) )
          {
            // Split fragment text into lines
            const QStringList splitLines = fragmentText.split( QStringLiteral( "\u2028" ), Qt::SplitBehaviorFlags::SkipEmptyParts );

            for ( const QString &splitLine : std::as_const( splitLines ) )
            {
              const QgsTextCharacterFormat *previousFormat = nullptr;

              // If the splitLine is not the first, inherit style from previous fragment
              if ( splitLine != splitLines.first() && document.size() > 0 )
              {
                previousFormat = &document.at( document.size() - 1 ).at( 0 ).characterFormat();
              }

              if ( splitLine.contains( QStringLiteral( TAB_REPLACEMENT_MARKER ) ) )
              {
                // split line by tab characters, each tab should be a
                // fragment by itself
                QgsTextFragment splitFragment( fragment );
                if ( previousFormat )
                {
                  // Apply overrides from previous fragment
                  QgsTextCharacterFormat newFormat { splitFragment.characterFormat() };
                  newFormat.overrideWith( *previousFormat );
                  splitFragment.setCharacterFormat( newFormat );
                }

                const QStringList tabSplit = splitLine.split( QStringLiteral( TAB_REPLACEMENT_MARKER ) );
                int index = 0;
                for ( const QString &part : tabSplit )
                {
                  if ( !part.isEmpty() )
                  {
                    splitFragment.setText( part );
                    block.append( splitFragment );
                  }
                  if ( index != tabSplit.size() - 1 )
                  {
                    block.append( QgsTextFragment( QString( '\t' ) ) );
                  }
                  index++;
                }
              }
              else
              {
                QgsTextFragment splitFragment( fragment );
                splitFragment.setText( splitLine );

                if ( previousFormat )
                {
                  // Apply overrides from previous fragment
                  QgsTextCharacterFormat newFormat { splitFragment.characterFormat() };
                  newFormat.overrideWith( *previousFormat );
                  splitFragment.setCharacterFormat( newFormat );
                }

                block.append( splitFragment );
              }

              document.append( block );
              block = QgsTextBlock();
            }
          }
          else if ( fragmentText.contains( QStringLiteral( TAB_REPLACEMENT_MARKER ) ) )
          {
            // split line by tab characters, each tab should be a
            // fragment by itself
            QgsTextFragment tmpFragment( fragment );
            const QStringList tabSplit = fragmentText.split( QStringLiteral( TAB_REPLACEMENT_MARKER ) );
            int index = 0;
            for ( const QString &part : tabSplit )
            {
              if ( !part.isEmpty() )
              {
                tmpFragment.setText( part );
                block.append( tmpFragment );
              }
              if ( index != tabSplit.size() - 1 )
              {
                block.append( QgsTextFragment( QString( '\t' ) ) );
              }
              index++;
            }
          }
          else
          {
            block.append( QgsTextFragment( fragment ) );
          }
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
