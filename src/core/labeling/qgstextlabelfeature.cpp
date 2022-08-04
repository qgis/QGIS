/***************************************************************************
    qgstextlabelfeature.cpp
    ---------------------
    begin                : December 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextlabelfeature.h"

#include "qgspallabeling.h"
#include "qgsmaptopixel.h"
#include "qgstextcharacterformat.h"
#include "qgstextfragment.h"
#include "qgstextblock.h"

QgsTextLabelFeature::QgsTextLabelFeature( QgsFeatureId id, geos::unique_ptr geometry, QSizeF size )
  : QgsLabelFeature( id, std::move( geometry ), size )
{
  mDefinedFont = QFont();
}

QgsTextLabelFeature::~QgsTextLabelFeature() = default;

QString QgsTextLabelFeature::text( int partId ) const
{
  if ( partId == -1 )
    return mLabelText;
  else
    return mTextMetrics->grapheme( partId );
}

QgsTextCharacterFormat QgsTextLabelFeature::characterFormat( int partId ) const
{
  return mTextMetrics.has_value() ? mTextMetrics->graphemeFormat( partId ) : QgsTextCharacterFormat();
}

bool QgsTextLabelFeature::hasCharacterFormat( int partId ) const
{
  return mTextMetrics.has_value() && partId < mTextMetrics->graphemeFormatCount();
}

QgsPrecalculatedTextMetrics QgsTextLabelFeature::calculateTextMetrics( const QgsMapToPixel *xform, const QFontMetricsF &fontMetrics, double letterSpacing, double wordSpacing, const QString &text, QgsTextDocument *document )
{
  // create label info!
  const double mapScale = xform->mapUnitsPerPixel();
  const double characterHeight = mapScale * fontMetrics.height();
  QStringList graphemes;
  QVector< QgsTextCharacterFormat > graphemeFormats;

  if ( document )
  {
    for ( const QgsTextBlock &block : std::as_const( *document ) )
    {
      for ( const QgsTextFragment &fragment : block )
      {

        // fragment.horizontalAdvance( ..., false,  .... );

        const QStringList fragmentGraphemes = QgsPalLabeling::splitToGraphemes( fragment.text() );
        for ( const QString &grapheme : fragmentGraphemes )
        {
          graphemes.append( grapheme );
          graphemeFormats.append( fragment.characterFormat() );
        }
      }
    }
  }
  else
  {
    //split string by valid grapheme boundaries - required for certain scripts (see #6883)
    graphemes = QgsPalLabeling::splitToGraphemes( text );
  }

  QVector< double > characterWidths( graphemes.count() );
  for ( int i = 0; i < graphemes.count(); i++ )
  {
    // reconstruct how Qt creates word spacing, then adjust per individual stored character
    // this will allow PAL to create each candidate width = character width + correct spacing

    qreal wordSpaceFix = qreal( 0.0 );
    if ( graphemes[i] == QLatin1String( " " ) )
    {
      // word spacing only gets added once at end of consecutive run of spaces, see QTextEngine::shapeText()
      int nxt = i + 1;
      wordSpaceFix = ( nxt < graphemes.count() && graphemes[nxt] != QLatin1String( " " ) ) ? wordSpacing : qreal( 0.0 );
    }
    // this workaround only works for clusters with a single character. Not sure how it should be handled
    // with multi-character clusters.
    if ( graphemes[i].length() == 1 &&
         !qgsDoubleNear( fontMetrics.horizontalAdvance( QString( graphemes[i].at( 0 ) ) ), fontMetrics.horizontalAdvance( graphemes[i].at( 0 ) ) + letterSpacing ) )
    {
      // word spacing applied when it shouldn't be
      wordSpaceFix -= wordSpacing;
    }

    const double charWidth = fontMetrics.horizontalAdvance( QString( graphemes[i] ) ) + wordSpaceFix;
    characterWidths[i] = mapScale * charWidth;
  }

  QgsPrecalculatedTextMetrics res( graphemes, characterHeight, std::move( characterWidths ) );
  res.setGraphemeFormats( graphemeFormats );
  return res;
}

QgsTextDocument QgsTextLabelFeature::document() const
{
  return mDocument;
}

QgsTextDocumentMetrics QgsTextLabelFeature::documentMetrics() const
{
  return mDocumentMetrics;
}

void QgsTextLabelFeature::setDocument( const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics )
{
  mDocument = document;
  mDocumentMetrics = metrics;
}
