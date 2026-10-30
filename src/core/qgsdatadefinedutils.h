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

        if ( key.size() > MAX_KEY_SIZE )
          throw "QgsDataDefinedStringDecoder: keys must be less than MAX_KEY_SIZE characters length";

        for ( int j = 0; j < key.size(); ++j )
        {
          if ( key[j].unicode() >= u'A' && key[j].unicode() <= u'Z' )
            throw "QgsDataDefinedStringDecoder: keys must be lowercase";

          if ( shouldIgnoreCharacter( key[j].unicode() ) )
            throw "QgsDataDefinedStringDecoder: keys must not contain spaces, hyphens, or underscores";
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

      // this function may be called MANY MANY thousands of times per map render, and for some data-defined
      // enums there are many keys to lookup. This adds up! We can optimize this a bit by pre-allocating
      // the keys into alphabetical partitions, so that we only have to compare candidate values to keys
      // with matching first letters
      for ( size_t i = 0; i < N; ++i )
      {
        // work out what partition this key falls into - the partition index is just the alphabetical index of the first letter
        const int partitionIndex = getPartitionIndexForChar( mLookupTable[i].first[0].unicode() );
        if ( partitionIndex < 0 )
          throw "Something went wrong -- partition index < 0";
        if ( partitionIndex >= 26 )
          throw "Something went wrong -- partition index >= 26";

        if ( mEntryPartitions[partitionIndex].count == 0 )
        {
          mEntryPartitions[partitionIndex].offset = static_cast<uint16_t>( i );
        }
        mEntryPartitions[partitionIndex].count++;
      }
    }

    /**
     * Looks up a string in the lookup table, returning the corresponding enum value if found.
     */
    [[nodiscard]] std::optional<V> lookup( QStringView string ) const
    {
      if ( string.isEmpty() )
        return {};

      // clean up the candidate string -- remove whitespace and other unwanted characters
      std::array<char16_t, MAX_KEY_SIZE> cleanedString;
      int cleanedStringLength = 0;
      for ( qsizetype i = 0; i < string.size(); ++i )
      {
        char16_t unicode = string[i].unicode();
        if ( shouldIgnoreCharacter( unicode ) )
          continue;

        // strings with more than MAX_KEY_SIZE non-ignored characters cannot possibly match anything,
        // cos we have compile-time assertion that all valid keys are < this length...
        if ( cleanedStringLength >= MAX_KEY_SIZE )
          return {};

        static constexpr int UPPER_TO_LOWER_CHAR_OFFSET = 32;
        if ( unicode >= u'A' && unicode <= u'Z' )
          unicode += UPPER_TO_LOWER_CHAR_OFFSET;

        cleanedString[cleanedStringLength++] = unicode;
      }

      if ( cleanedStringLength == 0 )
        return {};

      // find the corresponding partition
      const int partitionIndex = getPartitionIndexForChar( cleanedString[0] );
      if ( partitionIndex < 0 )
      {
        // doesn't start with an a-z character, so cannot possibly match any key (since we have
        // compile-time requirement that keys start with a-z)
        return {};
      }

      const Partition &partition = mEntryPartitions[partitionIndex];
      if ( partition.count == 0 )
        return {}; // no keys match the start character of the string

      const DataDefinedDecoderEntry<V> *partitionData = mLookupTable.data() + partition.offset;
      const QStringView cleanedStringView( cleanedString.data(), cleanedStringLength );
      for ( uint16_t i = 0; i < partition.count; ++i )
      {
        if ( partitionData[i].first.size() == cleanedStringLength && partitionData[i].first.compare( cleanedStringView ) == 0 )
        {
          return partitionData[i].second;
        }
      }

      return {};
    }

  private:
    static constexpr int MAX_KEY_SIZE = 32;
    std::array<DataDefinedDecoderEntry<V>, N> mLookupTable {};

    // we have one partition per a-z character, allowing us to jump immediately to the
    // candidates with the matching first letter
    struct Partition
    {
        uint16_t offset = 0;
        uint16_t count = 0;
    };
    std::array<Partition, 26> mEntryPartitions {};

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

    static constexpr int getPartitionIndexForChar( char16_t c )
    {
      // work out what partition this character falls into - the partition index is just the alphabetical index of the character
      if ( c >= u'a' && c <= u'z' )
      {
        return c - u'a';
      }
      else if ( c >= u'A' && c <= u'Z' )
      {
        return c - u'A';
      }

      return -1;
    }

    static constexpr bool shouldIgnoreCharacter( char16_t unicode ) noexcept
    {
      // when matching strings to keys, these characters will be completely ignored.
      // this gives us tolerant matching, so e.g any of "top-right", "top right", "top_right" will match to the "topright" key
      return unicode == u' ' || unicode == u'-' || unicode == u'_';
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
 * - not contain spaces, hyphens or underscores
 *
 * Usage:
 *
 * \code{cpp}
 * // create a decoder once as a static object
 * static constexpr auto sPointPositionMap = makeDataDefinedStringDecoder<Qgis::LabelPredefinedPointPosition>( {
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
template<typename V, size_t N> constexpr QgsDataDefinedStringDecoder<V, N> makeDataDefinedStringDecoder( const DataDefinedDecoderEntry<V> ( &values )[N] )
{
  return QgsDataDefinedStringDecoder<V, N>( values );
}

#endif // QGSDATADEFINEDUTILS_H
