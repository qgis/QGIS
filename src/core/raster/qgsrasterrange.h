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
 * Raster values range container. Represents range of values between min and max
 * including min and max value.
 */
class CORE_EXPORT QgsRasterRange
{
  public:

    /**
     * Default constructor, both min and max value for the range will be set to NaN.
     */
    QgsRasterRange() = default;

    /**
     * Constructor for a range with the given \a min and \a max values.
     */
    QgsRasterRange( double min, double max );

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
     * Sets the minimum value for the range.
     * \see min()
     */
    double setMin( double min ) { return mMin = min; }

    /**
     * Sets the maximum value for the range.
     * \see max()
     */
    double setMax( double max ) { return mMax = max; }

    inline bool operator==( QgsRasterRange o ) const
    {
      return qgsDoubleNear( mMin, o.mMin ) && qgsDoubleNear( mMax, o.mMax );
    }

    /**
     * \brief Tests if a \a value is within the list of ranges
     *  \param value value
     *  \param rangeList list of ranges
     *  \returns true if value is in at least one of ranges
     */
    static bool contains( double value, const QgsRasterRangeList &rangeList );

  private:
    double mMin = std::numeric_limits<double>::quiet_NaN();
    double mMax = std::numeric_limits<double>::quiet_NaN();
};

#endif


