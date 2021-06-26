/***************************************************************************
  qgsnetworkdistancestrategy.h
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

#ifndef QGSNETWORKDISTANCESTRATEGY_H
#define QGSNETWORKDISTANCESTRATEGY_H

#include "qgsnetworkstrategy.h"
#include "qgis_analysis.h"

/**
 * \ingroup analysis
 * \class QgsNetworkDistanceStrategy
 * \brief Strategy for calculating edge cost based on its length. Should be
 * used for finding shortest path between two points.
 * \since QGIS 3.0
 */
class ANALYSIS_EXPORT QgsNetworkDistanceStrategy : public QgsNetworkStrategy
{
  public:
    QVariant cost( double distance, const QgsFeature & ) const override;
};

#endif // QGSNETWORKDISTANCESTRATEGY_H
