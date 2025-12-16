/***************************************************************************
                         qgsalgorithmtransectfixeddistance.h
                         ------------------------------------
    begin                : September 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMTRANSECTFIXEDDISTANCE_H
#define QGSALGORITHMTRANSECTFIXEDDISTANCE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsalgorithmtransectbase.h"

///@cond PRIVATE

/**
 * Native transect (fixed distance) algorithm.
 */
class QgsTransectFixedDistanceAlgorithm : public QgsTransectAlgorithmBase
{
  public:
    QgsTransectFixedDistanceAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsTransectFixedDistanceAlgorithm *
      createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() override;
    bool
      prepareAlgorithmTransectParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    std::vector<QgsPoint>
      generateSamplingPoints( const QgsLineString &line, const QVariantMap &parameters, QgsProcessingContext &context ) override;
    double calculateAzimuth( const QgsLineString &line, const QgsPoint &point, int pointIndex ) override;

  private:
    double mInterval = 10.0;
    bool mIncludeStartPoint = true;
};

///@endcond PRIVATE

#endif // QGSALGORITHMTRANSECTFIXEDDISTANCE_H
