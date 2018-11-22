/***************************************************************************
                         qgsalgorithmwedgebuffers.h
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#ifndef QGSALGORITHMWEDGEBUFFERS_H
#define QGSALGORITHMWEDGEBUFFERS_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native create wedge buffers algorithm
 */
class QgsWedgeBuffersAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsWedgeBuffersAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsWedgeBuffersAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;
    QgsProcessingFeatureSource::Flag sourceFlags() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    double mAzimuth = 0.0;
    bool mDynamicAzimuth = false;
    QgsProperty mAzimuthProperty;
    double mWidth = 45.0;
    bool mDynamicWidth = false;
    QgsProperty mWidthProperty;
    double mOuterRadius = 1.0;
    bool mDynamicOuterRadius = false;
    QgsProperty mOuterRadiusProperty;
    double mInnerRadius = 1.0;
    bool mDynamicInnerRadius = false;
    QgsProperty mInnerRadiusProperty;

};

///@endcond PRIVATE

#endif // QGSALGORITHMWEDGEBUFFERS_H


