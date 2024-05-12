/***************************************************************************
  qgslabelpointsettings.h
  --------------------------
  Date                 : May 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELPOINTSETTINGS_H
#define QGSLABELPOINTSETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmapunitscale.h"
#include <QString>

class QgsPropertyCollection;
class QgsExpressionContext;

/**
 * \ingroup core
 * \class QgsLabelPointSettings
 *
 * \brief Contains settings related to how the label engine places and formats
 * labels for point features.
 *
 * \since QGIS 3.38
 */
class CORE_EXPORT QgsLabelPointSettings
{
    Q_GADGET

  public:

    /**
     * Updates the point settings to respect any data defined properties
     * set within the specified \a properties collection.
     */
    void updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context );

    /**
     * Returns the ordered list of predefined label positions for points.
     *
     * Positions earlier in the list will be prioritized over later positions.
     * Only used when the placement is set to Qgis::LabelPlacement::OrderedPositionsAroundPoint.
     *
     * \note not available in Python bindings
     *
     * \see setPredefinedPositionOrder()
    */
    QVector< Qgis::LabelPredefinedPointPosition > predefinedPositionOrder() const SIP_SKIP { return mPredefinedPositionOrder; }

    /**
     * Sets the ordered list of predefined label positions for points.
     *
     * Positions earlier in the list will be prioritized over later positions.
     * Only used when the placement is set to Qgis::LabelPlacement::OrderedPositionsAroundPoint.
     *
     * \note not available in Python bindings
     *
     * \see predefinedPositionOrder()
    */
    void setPredefinedPositionOrder( const QVector< Qgis::LabelPredefinedPointPosition > &order ) SIP_SKIP { mPredefinedPositionOrder = order; }

  private:

    QVector< Qgis::LabelPredefinedPointPosition > mPredefinedPositionOrder;

};

#endif // QGSLABELPOINTSETTINGS_H
