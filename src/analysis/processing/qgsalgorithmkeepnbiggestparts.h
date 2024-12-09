/***************************************************************************
                         qgsalgorithmkeepnbiggestparts.h
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#ifndef QGSALGORITHMKEEPNBIGGESTPARTS_H
#define QGSALGORITHMKEEPNBIGGESTPARTS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native keep n biggest parts algorithm.
 */
class QgsKeepNBiggestPartsAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsKeepNBiggestPartsAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsKeepNBiggestPartsAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString inputParameterName() const override;
    QString outputName() const override;
    QList<int> inputLayerTypes() const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    int mPartsToKeep = 1;
    bool mDynamicPartsToKeep = false;
    QgsProperty mPartsToKeepProperty;
};


///@endcond PRIVATE

#endif // QGSALGORITHMKEEPNBIGGESTPARTS_H
