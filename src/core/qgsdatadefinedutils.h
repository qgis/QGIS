/***************************************************************************
  qgsdatadefinedutils.h
  -----------------
   begin                : October 2026
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

#ifndef QGSDATADEFINEDUTILS_H
#define QGSDATADEFINEDUTILS_H

#include <algorithm>
#include <array>
#include <optional>

#include "qgis_core.h"

#define SIP_NO_FILE

///@cond PRIVATE
template<typename V> struct DataDefinedDecoderEntry
{
    QStringView first;
    V second;
};
///@endcond

/**
 * \class QgsDataDefinedStringDecoder
 * \ingroup core
 * \brief A template based class for efficiently decoding a string value to a corresponding enum value.
 *
 * Matching is done in a case-insensitive, tolerant way.
 *
 * \since QGIS 4.4
 */
template<typename V, size_t N> class QgsDataDefinedStringDecoder
{
  public:
    /**
     * Constructor for QgsDataDefinedStringDecoder.
     */
    constexpr explicit QgsDataDefinedStringDecoder( const DataDefinedDecoderEntry<V> ( &values )[N] )
    {
      for ( size_t i = 0; i < N; ++i )
      {
        const QStringView key = values[i].first;

        // compile-time assumption checks
        if ( key.isEmpty() )
          throw "QgsDataDefinedStringDecoder: keys cannot be empty";

        if ( key[0].unicode() < u'a' || key[0].unicode() > u'z' )
          throw "QgsDataDefinedStringDecoder: keys must start with a lowercase letter";

        for ( int j = 0; j < key.size(); ++j )
        {
          if ( key[j].unicode() >= u'A' && key[j].unicode() <= u'Z' )
            throw "QgsDataDefinedStringDecoder: keys must be lowercase";
        }

        mLookupTable[i] = values[i];
      }

      // sort entries alphabetically
      std::sort( mLookupTable.begin(), mLookupTable.end(), []( const DataDefinedDecoderEntry<V> &a, const DataDefinedDecoderEntry<V> &b ) { return compareStringViews( a.first, b.first ) < 0; } );

      // now our entries are sorted, lets make sure they are unique too...
      for ( size_t i = 1; i < N; ++i )
      {
        if ( compareStringViews( mLookupTable[i - 1].first, mLookupTable[i].first ) == 0 )
          throw "QgsDataDefinedStringDecoder: keys must be unique";
      }
    }

    /**
     * Looks up a string in the lookup table, returning the corresponding enum value if found.
     */
    [[nodiscard]] std::optional<V> lookup( QStringView string ) const
    {
      if ( string.isEmpty() )
        return {};

      const QStringView trimmed = string.trimmed();
      if ( trimmed.isEmpty() )
        return {};

      for ( const DataDefinedDecoderEntry<V> &value : mLookupTable )
      {
        if ( value.first.compare( trimmed, Qt::CaseInsensitive ) == 0 )
        {
          return value.second;
        }
      }

      return {};
    }

  private:
    std::array<DataDefinedDecoderEntry<V>, N> mLookupTable {};

    // dang, can't use Qt's QStringView comparison methods, they aren't constexpr :(
    static constexpr int compareStringViews( QStringView a, QStringView b ) noexcept
    {
      for ( qsizetype i = 0; i < std::min( a.size(), b.size() ); ++i )
      {
        const char16_t aUnicode = a[i].unicode();
        const char16_t bUnicode = b[i].unicode();
        if ( aUnicode < bUnicode )
          return -1;
        if ( aUnicode > bUnicode )
          return 1;
      }
      if ( a.size() < b.size() )
        return -1;
      if ( a.size() > b.size() )
        return 1;
      return 0;
    }
};

/**
 * Creates a QgsDataDefinedStringDecoder from a hardcoded lookup table.
 *
 * String keys must:
 *
 * - be non-empty
 * - start with a letter character (a-z)
 * - be completely lower-case
 * - be unique
 *
 * Usage:
 *
 * \code{cpp}
 * // create a decoder once as a static object
 * static const auto sPointPositionMap = makeDataDefinedStringDecoder<Qgis::LabelPredefinedPointPosition>( {
 * { u"topleft", Qgis::LabelPredefinedPointPosition::TopLeft },
 * { u"topright", Qgis::LabelPredefinedPointPosition::TopRight }
 * } );
 *
 * // use the static object to lookup candidate string values
 * if ( auto evaluatedPosition = sPointPositionMap.lookup( candidate ) )
 * {
 *   // string was successfully matched, use the matched enum value
 *   Qgis::LabelPredefinedPointPosition position = *evaluatedPosition;
 * }
 * else
 * {
 *   // string did not match any entry in the lookup table...
 * }
 * \endcode
 *
 * \since QGIS 4.4
 */
template<typename V, size_t N> constexpr CORE_EXPORT QgsDataDefinedStringDecoder<V, N> makeDataDefinedStringDecoder( const DataDefinedDecoderEntry<V> ( &values )[N] )
{
  return QgsDataDefinedStringDecoder<V, N>( values );
}

#endif // QGSDATADEFINEDUTILS_H
