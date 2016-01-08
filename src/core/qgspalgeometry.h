/***************************************************************************
    qgspalgeometry.h
    ---------------------
    begin                : May 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPALGEOMETRY_H
#define QGSPALGEOMETRY_H

#include "qgsgeometry.h"
#include "qgspallabeling.h"
#include <pal/feature.h>

#include "qgslabelingenginev2.h"

/**
 * Class that adds extra information to QgsLabelFeature for text labels
 *
 * @note not part of public API
 * @note not available in Python bindings
 */
class QgsTextLabelFeature : public QgsLabelFeature
{
  public:
    //! Construct text label feature
    QgsTextLabelFeature( QgsFeatureId id, GEOSGeometry* geometry, const QSizeF& size )
        : QgsLabelFeature( id, geometry, size )
        , mFontMetrics( nullptr )
    {
      mDefinedFont = QFont();
    }

    //! Clean up
    ~QgsTextLabelFeature()
    {
      delete mFontMetrics;
    }

    /** Returns the text component corresponding to a specified label part
     * @param partId Set to -1 for labels which are not broken into parts (eg, non-curved labels), or the required
     * part index for labels which are broken into parts (curved labels)
     * @note added in QGIS 2.10
     */
    QString text( int partId ) const
    {
      if ( partId == -1 )
        return mLabelText;
      else
        return mClusters.at( partId );
    }

    //! calculate data for info(). setDefinedFont() must have been called already.
    void calculateInfo( bool curvedLabeling, QFontMetricsF* fm, const QgsMapToPixel* xform, double fontScale, double maxinangle, double maxoutangle )
    {
      if ( mInfo )
        return;

      mFontMetrics = new QFontMetricsF( *fm ); // duplicate metrics for when drawing label

      qreal letterSpacing = mDefinedFont.letterSpacing();
      qreal wordSpacing = mDefinedFont.wordSpacing();

      // max angle between curved label characters (20.0/-20.0 was default in QGIS <= 1.8)
      if ( maxinangle < 20.0 )
        maxinangle = 20.0;
      if ( 60.0 < maxinangle )
        maxinangle = 60.0;
      if ( maxoutangle > -20.0 )
        maxoutangle = -20.0;
      if ( -95.0 > maxoutangle )
        maxoutangle = -95.0;

      // create label info!
      double mapScale = xform->mapUnitsPerPixel();
      double labelHeight = mapScale * fm->height() / fontScale;

      // mLetterSpacing/mWordSpacing = 0.0 is default for non-curved labels
      // (non-curved spacings handled by Qt in QgsPalLayerSettings/QgsPalLabeling)
      qreal charWidth;
      qreal wordSpaceFix;

      //split string by valid grapheme boundaries - required for certain scripts (see #6883)
      mClusters = QgsPalLabeling::splitToGraphemes( mLabelText );

      mInfo = new pal::LabelInfo( mClusters.count(), labelHeight, maxinangle, maxoutangle );
      for ( int i = 0; i < mClusters.count(); i++ )
      {
        // reconstruct how Qt creates word spacing, then adjust per individual stored character
        // this will allow PAL to create each candidate width = character width + correct spacing
        charWidth = fm->width( mClusters[i] );
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
               !qgsDoubleNear( fm->width( QString( mClusters[i].at( 0 ) ) ), fm->width( mClusters[i].at( 0 ) ) + letterSpacing ) )
          {
            // word spacing applied when it shouldn't be
            wordSpaceFix -= wordSpacing;
          }

          charWidth = fm->width( QString( mClusters[i] ) ) + wordSpaceFix;
        }

        double labelWidth = mapScale * charWidth / fontScale;
        mInfo->char_info[i].width = labelWidth;
      }
    }

    //! Get data-defined values
    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& dataDefinedValues() const { return mDataDefinedValues; }
    //! Set data-defined values
    void setDataDefinedValues( const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& values ) { mDataDefinedValues = values; }

    //! Set font to be used for rendering
    void setDefinedFont( const QFont& f ) { mDefinedFont = f; }
    //! Font to be used for rendering
    QFont definedFont() { return mDefinedFont; }

    //! Metrics of the font for rendering
    QFontMetricsF* labelFontMetrics() { return mFontMetrics; }

  protected:
    //! List of graphemes (used for curved labels)
    QStringList mClusters;
    //! Font for rendering
    QFont mDefinedFont;
    //! Metrics of the font for rendering
    QFontMetricsF* mFontMetrics;
    /** Stores attribute values for data defined properties*/
    QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant > mDataDefinedValues;

};

#endif //QGSPALGEOMETRY_H
