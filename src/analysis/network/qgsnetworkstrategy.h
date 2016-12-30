/***************************************************************************
  qgsnetworkstrategy.h
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

#ifndef QGSNETWORKSTRATERGY_H
#define QGSNETWORKSTRATERGY_H

#include <QVariant>

#include <qgsfeature.h>
#include <qgsfeaturerequest.h>

/**
 * \ingroup analysis
 * \class QgsNetworkStrategy
 * \note added in QGIS 3.0
 * \brief QgsNetworkStrategy defines strategy used for calculation of the edge cost. For example it can
 * take into account travel distance, amount of time or money. Currently there are two strategies
 * implemented in the analysis library: QgsNetworkDistanceStrategy and QgsNetworkSpeedStrategy.
 * QgsNetworkStrategy implemented using "strategy" design pattern.
 */
class ANALYSIS_EXPORT QgsNetworkStrategy
{
  public:

    /**
     * Default constructor
     */
    QgsNetworkStrategy() {}

    virtual ~QgsNetworkStrategy() = default;

    /**
     * Returns list of the source layer attributes needed for cost calculation.
     * This method called by QgsGraphDirector.
     * \return list of required attributes
     */
    virtual QgsAttributeList requiredAttributes() const { return QgsAttributeList(); }

    /**
     * Returns edge cost
     */
    virtual QVariant cost( double distance, const QgsFeature &f ) const = 0;
};

#endif // QGSNETWORKSTRATERGY_H
