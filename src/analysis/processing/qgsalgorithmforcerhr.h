/***************************************************************************
                         qgsalgorithmforcerhr.h
                         ---------------------
    begin                : November 2018
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

#ifndef QGSALGORITHMFORCERHR_H
#define QGSALGORITHMFORCERHR_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native force right-hand-rule algorithm.
 */
class QgsForceRHRAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsForceRHRAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QList<int> inputLayerTypes() const override;
    QgsForceRHRAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QgsProcessingFeatureSource::Flag sourceFlags() const override;
    QString outputName() const override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMFORCERHR_H


