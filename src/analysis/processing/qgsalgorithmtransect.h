/***************************************************************************
                         qgsalgorithmtransect.h
                         -------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMTRANSECT_H
#define QGSALGORITHMTRANSECT_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsalgorithmtransectbase.h"

///@cond PRIVATE

/**
 * Native transect algorithm.
 */
class QgsTransectAlgorithm : public QgsTransectAlgorithmBase
{
  public:
    QgsTransectAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QString shortHelpString() const override;
    QgsTransectAlgorithm *createInstance() const override SIP_FACTORY;

  private:
    void addAlgorithmParams() override;
    bool
      prepareAlgorithmTransectParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    std::vector<QgsPoint>
      generateSamplingPoints( const QgsLineString &line, const QVariantMap &parameters, QgsProcessingContext &context ) override;
    double calculateAzimuth( const QgsLineString &line, const QgsPoint &point, int pointIndex ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMTRANSECT_H
