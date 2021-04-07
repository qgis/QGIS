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

#include "qgsgeometry.h"
#include "qgspallabeling.h"
#include "qgsmaptopixel.h"
#include "pal/feature.h"
#include "qgstextcharacterformat.h"
#include "qgstextfragment.h"
#include "qgstextblock.h"

QgsTextLabelFeature::QgsTextLabelFeature( QgsFeatureId id, geos::unique_ptr geometry, QSizeF size )
  : QgsLabelFeature( id, std::move( geometry ), size )
{
  mDefinedFont = QFont();
}


QgsTextLabelFeature::~QgsTextLabelFeature()
{
  delete mFontMetrics;
}


QString QgsTextLabelFeature::text( int partId ) const
{
  if ( partId == -1 )
    return mLabelText;
  else
    return mClusters.at( partId );
}

QgsTextCharacterFormat QgsTextLabelFeature::characterFormat( int partId ) const
{
  return mCharacterFormats.value( partId );
}

bool QgsTextLabelFeature::hasCharacterFormat( int partId ) const
{
  return partId < mCharacterFormats.size();
}

void QgsTextLabelFeature::calculateInfo( bool curvedLabeling, QFontMetricsF *fm, const QgsMapToPixel *xform, QgsTextDocument *document )
{
  if ( mInfo )
    return;

  mFontMetrics = new QFontMetricsF( *fm ); // duplicate metrics for when drawing label

  qreal letterSpacing = mDefinedFont.letterSpacing();
  qreal wordSpacing = mDefinedFont.wordSpacing();

  // create label info!
  const double mapScale = xform->mapUnitsPerPixel();
  const double characterHeight = mapScale * fm->height();

  // mLetterSpacing/mWordSpacing = 0.0 is default for non-curved labels
  // (non-curved spacings handled by Qt in QgsPalLayerSettings/QgsPalLabeling)
  qreal charWidth;
  qreal wordSpaceFix;

  if ( document && curvedLabeling )
  {
    for ( const QgsTextBlock &block : std::as_const( *document ) )
    {
      for ( const QgsTextFragment &fragment : block )
      {
        const QStringList graphemes = QgsPalLabeling::splitToGraphemes( fragment.text() );
        for ( const QString &grapheme : graphemes )
        {
          mClusters.append( grapheme );
          mCharacterFormats.append( fragment.characterFormat() );
        }
      }
    }
  }
  else
  {
    //split string by valid grapheme boundaries - required for certain scripts (see #6883)
    mClusters = QgsPalLabeling::splitToGraphemes( mLabelText );
  }

  std::vector< double > characterWidths( mClusters.count() );
  for ( int i = 0; i < mClusters.count(); i++ )
  {
    // reconstruct how Qt creates word spacing, then adjust per individual stored character
    // this will allow PAL to create each candidate width = character width + correct spacing
    charWidth = fm->horizontalAdvance( mClusters[i] );
    if ( curvedLabeling )
    {
      wordSpaceFix = qreal( 0.0 );
      if ( mClusters[i] == QLatin1String( " " ) )
      {
        // word spacing only gets added once at end of consecutive run of spaces, see QTextEngine::shapeText()
        int nxt = i + 1;
        wordSpaceFix = ( nxt < mClusters.count() && mClusters[nxt] != QLatin1String( " " ) ) ? wordSpacing : qreal( 0.0 );
      }
      // this workaround only works for clusters with a single character. Not sure how it should be handled
      // with multi-character clusters.
      if ( mClusters[i].length() == 1 &&
           !qgsDoubleNear( fm->horizontalAdvance( QString( mClusters[i].at( 0 ) ) ), fm->horizontalAdvance( mClusters[i].at( 0 ) ) + letterSpacing ) )
      {
        // word spacing applied when it shouldn't be
        wordSpaceFix -= wordSpacing;
      }

      charWidth = fm->horizontalAdvance( QString( mClusters[i] ) ) + wordSpaceFix;
    }

    characterWidths[i] = mapScale * charWidth;
  }
  mInfo = new pal::LabelInfo( characterHeight, std::move( characterWidths ) );
}

QgsTextDocument QgsTextLabelFeature::document() const
{
  return mDocument;
}

void QgsTextLabelFeature::setDocument( const QgsTextDocument &document )
{
  mDocument = document;
}
