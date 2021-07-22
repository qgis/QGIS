/***************************************************************************
  qgsaidwinterpolation.h - QgsAidwInterpolation

 ---------------------
 begin                : 19.7.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAIDWINTERPOLATION_H
#define QGSAIDWINTERPOLATION_H

#include "qgis_analysis.h"
#include "qgsexception.h"
#include "qgis_sip.h"

#include <QString>

class QgsVectorLayer;
class QgsRasterLayer;
class QgsFeedback;

/**
 * \ingroup analysis
 * \brief The QgsAidwInterpolation class implements OpenCL accelerated IDW interpolation.
 * \since QGIS 3.22
 */
class ANALYSIS_EXPORT QgsAidwInterpolation
{
  public:

    /**
     * Constructs a QgsAidwInterpolation object.
     * \param dataLayer the input points layer.
     * \param dataAttributeName name of the input layer attribute that contains the values for the interpolation, it must be a numeric field.
     * \param interpolatedLayer the destination layer for the interpolation, data type must be Float64 (double precision).
     * \param coefficient coefficient for distance weighting, default to 2.0.
     */
    QgsAidwInterpolation( QgsVectorLayer *dataLayer, QString &dataAttributeName, QgsRasterLayer *interpolatedLayer, double coefficient = 2.0 );

    /**
     * Executes the interpolation using the GPU and an optional \a feedback.
     */
    void process( QgsFeedback *feedback = nullptr ) SIP_THROW( QgsProcessingException );

  private:

    QgsVectorLayer *mDataLayer = nullptr;
    QgsRasterLayer *mInterpolatedLayer = nullptr;
    QString mDataAttributeName;
    QString mErrorMessage;
    double mCoefficient;

    void interpolate( QgsFeedback *feedback = nullptr );

};

#endif // QGSAIDWINTERPOLATION_H
