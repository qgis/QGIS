/***************************************************************************
                              qgsidwinterpolator.h
                              --------------------
  begin                : March 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSIDWINTERPOLATOR_H
#define QGSIDWINTERPOLATOR_H

#include "qgsinterpolator.h"
#include "qgis_analysis.h"

/**
 * \ingroup analysis
 * \class QgsIDWInterpolator
 * Inverse distance weight interpolator.
 */
class ANALYSIS_EXPORT QgsIDWInterpolator: public QgsInterpolator
{
  public:

    /**
     * Constructor for QgsIDWInterpolator, with the specified \a layerData sources.
     */
    QgsIDWInterpolator( const QList<QgsInterpolator::LayerData> &layerData );

    int interpolatePoint( double x, double y, double &result SIP_OUT, QgsFeedback *feedback = nullptr ) override;

    /**
     * Sets the distance \a coefficient, the parameter that sets how the values are
     * weighted with distance. Smaller values mean sharper peaks at the data points.
     *
     * Point values are weighted by 1 / ( distance ^ coefficient ).
     *
     * \see distanceCoefficient()
     * \since QGIS 3.0
    */
    void setDistanceCoefficient( double coefficient ) { mDistanceCoefficient = coefficient;}

    /**
     * Returns the distance coefficient, the parameter that sets how the values are
     * weighted with distance. Smaller values mean sharper peaks at the data points.
     * The default is a coefficient of 2.
     *
     * Point values are weighted by 1 / ( distance ^ coefficient ).
     *
     * \see setDistanceCoefficient()
     * \since QGIS 3.0
    */
    double distanceCoefficient() const { return mDistanceCoefficient; }

  private:

    QgsIDWInterpolator() = delete;

    double mDistanceCoefficient = 2.0;
};

#endif
