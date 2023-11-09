/***************************************************************************
  qgsnetworkspeedstrategy.h
  --------------------------------------
  Date                 : 2011-04-01
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/

#ifndef QGSNETWORKSPEEDSTRATEGY_H
#define QGSNETWORKSPEEDSTRATEGY_H

#include "qgsnetworkstrategy.h"
#include "qgis_analysis.h"

/**
 * \ingroup analysis
 * \class QgsNetworkSpeedStrategy
 * \brief Strategy for calculating edge cost based on travel time. Should be
 * used for finding fastest path between two points.
 * \since QGIS 3.0
 */
class ANALYSIS_EXPORT QgsNetworkSpeedStrategy : public QgsNetworkStrategy
{
  public:

    /**
     * Default constructor
     */
    QgsNetworkSpeedStrategy( int attributeId, double defaultValue, double toMetricFactor );

    QVariant cost( double distance, const QgsFeature &f ) const override;
    QSet< int > requiredAttributes() const override;

  private:
    int mAttributeId;
    double mDefaultValue;
    double mToMetricFactor;

};

#endif // QGSNETWORKSPEEDSTRATEGY_H
