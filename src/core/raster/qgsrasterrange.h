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
     * \brief Constructor.
     */
    QgsRasterRange();

    /**
     * \brief Constructor
     *  \param min minimum value
     *  \param max max value
     */
    QgsRasterRange( double min, double max );

    double min() const { return mMin; }
    double max() const { return mMax; }

    double setMin( double min ) { return mMin = min; }
    double setMax( double max ) { return mMax = max; }

    inline bool operator==( QgsRasterRange o ) const
    {
      return qgsDoubleNear( mMin, o.mMin ) && qgsDoubleNear( mMax, o.mMax );
    }

    /**
     * \brief Test if value is within the list of ranges
     *  \param value value
     *  \param rangeList list of ranges
     *  \returns true if value is in at least one of ranges
     *  \note not available in Python bindings
     */
    static bool contains( double value, const QgsRasterRangeList &rangeList ) SIP_SKIP;

  private:
    double mMin;
    double mMax;
};

#endif


