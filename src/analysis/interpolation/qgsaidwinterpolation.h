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
 * @brief The QgsAidwInterpolation class
 */
class ANALYSIS_EXPORT QgsAidwInterpolation
{
  public:

    QgsAidwInterpolation( QgsVectorLayer *dataLayer, QString &dataAttributeName, QgsRasterLayer *interpolatedLayer );

    void process( QgsFeedback *feedback = nullptr ) SIP_THROW( QgsProcessingException );

  private:

    QgsVectorLayer *mDataLayer = nullptr;
    QgsRasterLayer *mInterpolatedLayer = nullptr;
    QString mDataAttributeName;
    QString mErrorMessage;

    void interpolate( QgsFeedback *feedback = nullptr );

};

#endif // QGSAIDWINTERPOLATION_H
