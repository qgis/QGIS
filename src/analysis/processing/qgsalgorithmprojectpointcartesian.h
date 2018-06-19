/***************************************************************************
                         qgsalgorithmprojectpointcartesian.h
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMPROJECTPOINTCARTESIAN_H
#define QGSALGORITHMPROJECTPOINTCARTESIAN_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native Cartesian project points algorithm.
 */
class QgsProjectPointCartesianAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsProjectPointCartesianAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsProjectPointCartesianAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    QgsProcessingFeatureSource::Flag sourceFlags() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    double mBearing = 0.0;
    bool mDynamicBearing = false;
    QgsProperty mBearingProperty;
    double mDistance = 0.0;
    bool mDynamicDistance = false;
    QgsProperty mDistanceProperty;

};

///@endcond PRIVATE

#endif // QGSALGORITHMPROJECTPOINTCARTESIAN_H


