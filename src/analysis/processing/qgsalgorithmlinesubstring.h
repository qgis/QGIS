/***************************************************************************
                         qgsalgorithmlinesubstring.h
                         ---------------------
    begin                : August 2018
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

#ifndef QGSLINESUBSTRINGALGORITHM_H
#define QGSLINESUBSTRINGALGORITHM_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native line substring algorithm.
 */
class QgsLineSubstringAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsLineSubstringAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QList<int> inputLayerTypes() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsLineSubstringAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    QgsProcessingFeatureSource::Flag sourceFlags() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &, QgsProcessingFeedback *feedback ) override;

  private:

    double mStartDistance = 0.0;
    bool mDynamicStartDistance = false;
    QgsProperty mStartDistanceProperty;

    double mEndDistance = 0.0;
    bool mDynamicEndDistance = false;
    QgsProperty mEndDistanceProperty;
};


///@endcond PRIVATE

#endif // QGSLINESUBSTRINGALGORITHM_H


