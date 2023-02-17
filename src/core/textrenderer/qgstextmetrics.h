/***************************************************************************
  qgstextmetrics.h
  -----------------
   begin                : April 2021
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

#ifndef QGSTEXTMETRICS_H
#define QGSTEXTMETRICS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgstextcharacterformat.h"
#include <QVector>
#include <QStringList>

#define SIP_NO_FILE

/**
 * \class QgsPrecalculatedTextMetrics
  * \ingroup core
  * \brief Contains precalculated properties regarding text metrics for text to be renderered at a later stage.
  * \note Not available in Python bindings
  * \since QGIS 3.20
 */
class CORE_EXPORT QgsPrecalculatedTextMetrics
{
  public:

    /**
     * Constructor for QgsPrecalculatedTextMetrics
     * \param graphemes list of graphemes contained in the text
     * \param characterWidths vector of character widths
     * \param characterHeights height of characters
     * \param characterDescents descent of characters
     */
    QgsPrecalculatedTextMetrics( const QStringList &graphemes,
                                 const QVector< double > &characterWidths,
                                 const QVector< double > &characterHeights,
                                 const QVector< double > &characterDescents )
      : mGraphemes( graphemes )
      , mCharacterHeights( characterHeights )
      , mCharacterWidths( characterWidths )
      , mCharacterDescents( characterDescents )
    {
      Q_ASSERT( mCharacterHeights.size() == mCharacterWidths.size() );
      Q_ASSERT( mCharacterDescents.size() == mCharacterWidths.size() );

      if ( !mCharacterHeights.empty() )
      {
        mMaximumCharacterHeight = *std::max_element( mCharacterHeights.constBegin(), mCharacterHeights.constEnd() );
        mMaximumCharacterDescent = *std::max_element( mCharacterDescents.constBegin(), mCharacterDescents.constEnd() );
      }
    }

    /**
     * Returns the total number of characters.
     */
    int count() const { return static_cast< int >( mCharacterWidths.size() ); }

    /**
     * Returns the width of the character at the specified position.
     */
    double characterWidth( int position ) const { return mCharacterWidths[position]; }

    /**
     * Returns the character height of the character at the specified position (actually font metrics height, not individual character height).
     *
     * \since QGIS 3.28
     */
    double characterHeight( int position ) const { return mCharacterHeights[position]; }

    /**
     * Returns the descent of the character at the specified position.
     *
     * \since QGIS 3.28
     */
    double characterDescent( int position ) const { return mCharacterDescents[position]; }

    /**
     * Returns the maximum height of any character found in the text.
     *
     * \since QGIS 3.28
     */
    double maximumCharacterHeight() const { return mMaximumCharacterHeight; }

    /**
     * Returns the maximum descent of any character found in the text.
     *
     * \since QGIS 3.28
     */
    double maximumCharacterDescent() const { return mMaximumCharacterDescent; }

    /**
     * Returns the list of graphemes contained in the text.
     */
    QStringList graphemes() const { return mGraphemes; }

    /**
     * Returns the grapheme at the specified \a index.
     */
    QString grapheme( int index ) const { return mGraphemes.at( index ); }

    /**
     * Sets the character \a formats associated with the text graphemes().
     */
    void setGraphemeFormats( const QVector< QgsTextCharacterFormat > &formats ) { mGraphemeFormats = formats; }

    /**
     * Returns the number of grapheme formats available.
     */
    int graphemeFormatCount() const { return mGraphemeFormats.count(); }

    /**
     * Returns the character format for the grapheme at the specified \a index.
     */
    QgsTextCharacterFormat graphemeFormat( int index ) const { return mGraphemeFormats.value( index ); }

  private:

    QStringList mGraphemes;
    QVector< QgsTextCharacterFormat > mGraphemeFormats;
    QVector< double > mCharacterHeights;
    QVector< double > mCharacterWidths;
    QVector< double > mCharacterDescents;
    double mMaximumCharacterHeight = 0;
    double mMaximumCharacterDescent = 0;

};


#endif // QGSTEXTMETRICS_H
