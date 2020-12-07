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

#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgis_analysis.h"

#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgsnetworkspeedstrategy.h>
#include <qgsnetworkdistancestrategy.h>
% End
#endif

/**
 * \ingroup analysis
 * \class QgsNetworkStrategy
 * \brief QgsNetworkStrategy defines strategy used for calculation of the edge cost. For example it can
 * take into account travel distance, amount of time or money. Currently there are two strategies
 * implemented in the analysis library: QgsNetworkDistanceStrategy and QgsNetworkSpeedStrategy.
 * QgsNetworkStrategy implemented using "strategy" design pattern.
 * \since QGIS 3.0
 */

class ANALYSIS_EXPORT QgsNetworkStrategy
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsNetworkDistanceStrategy * >( sipCpp ) != NULL )
      sipType = sipType_QgsNetworkDistanceStrategy;
    else if ( dynamic_cast< QgsNetworkSpeedStrategy * >( sipCpp ) != NULL )
      sipType = sipType_QgsNetworkSpeedStrategy;
    else
      sipType = NULL;
    SIP_END
#endif

  public:

    /**
     * Default constructor
     */
    QgsNetworkStrategy() = default;

    virtual ~QgsNetworkStrategy() = default;

    /**
     * Returns a list of the source layer attributes needed for cost calculation.
     * This is method called by QgsGraphDirector.
     */
    virtual QSet< int > requiredAttributes() const { return QSet< int >(); }

    /**
     * Returns edge cost
     */
    virtual QVariant cost( double distance, const QgsFeature &f ) const = 0;
};

#endif // QGSNETWORKSTRATERGY_H
