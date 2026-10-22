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
#include <utility>

#include "qgis_core.h"

#define SIP_NO_FILE

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
    explicit QgsDataDefinedStringDecoder( const std::pair<QStringView, V> ( &values )[N] )
    {
      for ( size_t i = 0; i < N; ++i )
      {
        Q_ASSERT( !values[i].first.isEmpty() );
        mLookupTable[i] = values[i];
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

      for ( const std::pair<QStringView, V> &value : mLookupTable )
      {
        if ( value.first.compare( trimmed, Qt::CaseInsensitive ) == 0 )
        {
          return value.second;
        }
      }

      return {};
    }

  private:
    std::array<std::pair<QStringView, V>, N> mLookupTable {};
};

/**
 * Creates a QgsDataDefinedStringDecoder from a hardcoded lookup table.
 *
 * Usage:
 *
 * \code{cpp}
 * // create a decoder once as a static object
 * static const auto sPointPositionMap = makeDataDefinedStringDecoder<Qgis::LabelPredefinedPointPosition>( {
 * { u"TopLeft", Qgis::LabelPredefinedPointPosition::TopLeft },
 * { u"TopRight", Qgis::LabelPredefinedPointPosition::TopRight }
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
template<typename V, size_t N> CORE_EXPORT QgsDataDefinedStringDecoder<V, N> makeDataDefinedStringDecoder( const std::pair<QStringView, V> ( &values )[N] )
{
  return QgsDataDefinedStringDecoder<V, N>( values );
}

#endif // QGSDATADEFINEDUTILS_H
