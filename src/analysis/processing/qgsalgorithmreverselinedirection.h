/***************************************************************************
                         qgsalgorithmreverselinedirection.h
                         ----------------------------------
    begin                : July 2018
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

#ifndef QGSALGORITHMREVERSELINEDIRECTION_H
#define QGSALGORITHMREVERSELINEDIRECTION_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native reverse line direction algorithm.
 */
class QgsReverseLineDirectionAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsReverseLineDirectionAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsReverseLineDirectionAlgorithm  *createInstance() const override SIP_FACTORY;
    QgsProcessing::SourceType outputLayerType() const override;
    QList<int> inputLayerTypes() const override;

  protected:

    QString outputName() const override;
    QgsProcessingFeatureSource::Flag sourceFlags() const override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMREVERSELINEDIRECTION_H


