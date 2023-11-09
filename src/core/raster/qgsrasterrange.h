/***************************************************************************
          qgsrasterrange.h
     --------------------------------------
    Date                 : Oct 9, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERRANGE_H
#define QGSRASTERRANGE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QList>

class QgsRasterRange;

typedef QList<QgsRasterRange> QgsRasterRangeList;

/**
 * \ingroup core
 * \brief Raster values range container. Represents range of values between min and max
 * including min and max value.
 */
class CORE_EXPORT QgsRasterRange
{
  public:

    //! Handling for min and max bounds
    enum BoundsType
    {
      IncludeMinAndMax = 0, //!< Min and max values are inclusive
      IncludeMax, //!< Include the max value, but not the min value, e.g. min < value <= max
      IncludeMin, //!< Include the min value, but not the max value, e.g. min <= value < max
      Exclusive, //!< Don't include either the min or max value, e.g. min < value < max
    };

    /**
     * Default constructor, both min and max value for the range will be set to NaN.
     */
    QgsRasterRange() = default;

    /**
     * Constructor for a range with the given \a min and \a max values.
     *
     * The \a bounds argument dictates how the min and max value themselves
     * will be handled by the range.
     */
    QgsRasterRange( double min, double max, BoundsType bounds = IncludeMinAndMax );

    /**
     * Returns the minimum value for the range.
     * \see setMin()
     */
    double min() const { return mMin; }

    /**
     * Returns the maximum value for the range.
     * \see setMax()
     */
    double max() const { return mMax; }

    /**
     * Returns the bounds type for the range, which specifies
     * whether or not the min and max values themselves are included
     * in the range.
     * \see setBounds()
     * \since QGIS 3.2
     */
    BoundsType bounds() const { return mType; }

    /**
     * Sets the minimum value for the range.
     * \see min()
     */
    double setMin( double min ) { return mMin = min; }

    /**
     * Sets the maximum value for the range.
     * \see max()
     */
    double setMax( double max ) { return mMax = max; }

    /**
     * Sets the bounds \a type for the range, which specifies
     * whether or not the min and max values themselves are included
     * in the range.
     * \see bounds()
     * \since QGIS 3.2
     */
    void setBounds( BoundsType type ) { mType = type; }

    inline bool operator==( const QgsRasterRange &o ) const
    {
      return ( ( std::isnan( mMin ) && std::isnan( o.mMin ) ) || qgsDoubleNear( mMin, o.mMin ) )
             && ( ( std::isnan( mMax ) && std::isnan( o.mMax ) ) || qgsDoubleNear( mMax, o.mMax ) )
             && mType == o.mType;
    }

    /**
     * Returns TRUE if this range contains the specified \a value.
     * \since QGIS 3.2
     */
    bool contains( double value ) const
    {
      return ( value > mMin
               || ( !std::isnan( mMin ) && qgsDoubleNear( value, mMin ) && ( mType == IncludeMinAndMax || mType == IncludeMin ) )
               || std::isnan( mMin ) )
             &&
             ( value < mMax
               || ( !std::isnan( mMax ) && qgsDoubleNear( value, mMax ) && ( mType == IncludeMinAndMax || mType == IncludeMax ) )
               || std::isnan( mMax ) );
    }

    /**
     * \brief Tests if a \a value is within the list of ranges
     *  \param value value
     *  \param rangeList list of ranges
     *  \returns TRUE if value is in at least one of ranges
     */
    static bool contains( double value, const QgsRasterRangeList &rangeList )
    {
      for ( const QgsRasterRange &range : rangeList )
      {
        if ( range.contains( value ) )
        {
          return true;
        }
      }
      return false;
    }

    /**
     * Returns TRUE if this range overlaps another range.
     * \since QGIS 3.2
     */
    bool overlaps( const QgsRasterRange &other ) const;

    /**
     * Returns a text representation of the range.
     * \since QGIS 3.2
     */
    QString asText() const;

  private:
    double mMin = std::numeric_limits<double>::quiet_NaN();
    double mMax = std::numeric_limits<double>::quiet_NaN();
    BoundsType mType = IncludeMinAndMax;
};

#endif


