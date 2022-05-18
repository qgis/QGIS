/***************************************************************************
                         qgsalgorithmbatchnominatimgeocode.h
                         ------------------
    begin                : December 2020
    copyright            : (C) 2020 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMBATCHNOMINATIMGEOCODE_H
#define QGSALGORITHMBATCHNOMINATIMGEOCODE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgis_analysis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsalgorithmbatchgeocode.h"
#include "qgsnominatimgeocoder.h"

///@cond PRIVATE

/**
 * Native batch Nominatim geocoder.
 */
class QgsBatchNominatimGeocodeAlgorithm : public QgsBatchGeocodeAlgorithm
{

  public:

    /**
     * Constructor for QgsBatchNominatimGeocodeAlgorithm.
     */
    QgsBatchNominatimGeocodeAlgorithm();

    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsBatchNominatimGeocodeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QgsCoordinateReferenceSystem outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    QgsNominatimGeocoder mNominatimGeocoder;
    mutable QgsCoordinateReferenceSystem mOutputCrs;

};

///@endcond PRIVATE

#endif // QGSALGORITHMBATCHNOMINATIMGEOCODE_H


