/***************************************************************************
                         qgsalgorithmsmooth.h
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

#ifndef QGSALGORITHMSMOOTH_H
#define QGSALGORITHMSMOOTH_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native smooth algorithm.
 */
class QgsSmoothAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsSmoothAlgorithm() = default;
    Flags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsSmoothAlgorithm *createInstance() const override SIP_FACTORY;
    QList<int> inputLayerTypes() const override;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeature processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    int mIterations = 1;
    double mOffset = 0.25;
    double mMaxAngle = 0;
};

///@endcond PRIVATE

#endif // QGSALGORITHMSMOOTH_H


