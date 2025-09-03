/***************************************************************************
                         qgsalgorithmremovepartsbylength.h
                         ---------------------
    begin                : July 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#ifndef QGSALGORITHMREMOVEPARTSBYLENGTH_H
#define QGSALGORITHMREMOVEPARTSBYLENGTH_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native remove parts by length algorithm.
 */
class QgsRemovePartsByLengthAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsRemovePartsByLengthAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsRemovePartsByLengthAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    QList<int> inputLayerTypes() const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    double mMinLength = 0.0;
    bool mDynamicMinLength = false;
    QgsProperty mMinLengthProperty;
};


///@endcond PRIVATE

#endif // QGSALGORITHMREMOVEPARTSBYLENGTH_H
