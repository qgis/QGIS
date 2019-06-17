/***************************************************************************
                         qgsalgorithmpointsalonggeometry.h
                         ---------------------
    begin                : June 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#ifndef QGSPOINTSALONGGEOMETRYALGORITHM_H
#define QGSPOINTSALONGGEOMETRYALGORITHM_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native points along geometry algorithm.
 */
class QgsPointsAlongGeometryAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsPointsAlongGeometryAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QList<int> inputLayerTypes() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsPointsAlongGeometryAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override;
    QString svgIconPath() const override;

  protected:
    QString outputName() const override;
    QgsProcessingFeatureSource::Flag sourceFlags() const override;
    QgsFeatureSink::SinkFlags sinkFlags() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &, QgsProcessingFeedback *feedback ) override;

  private:

    double mDistance = 0.0;
    bool mDynamicDistance = false;
    QgsProperty mDistanceProperty;

    double mStartOffset = 0.0;
    bool mDynamicStartOffset = false;
    QgsProperty mStartOffsetProperty;

    double mEndOffset = 0.0;
    bool mDynamicEndOffset = false;
    QgsProperty mEndOffsetProperty;

};


///@endcond PRIVATE

#endif // QGSPOINTSALONGGEOMETRYALGORITHM_H


