/***************************************************************************
                         qgsalgorithmcalculateoverlaps.h
                         ------------------
    begin                : May 2019
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

#ifndef QGSALGORITHMCALCULATEOVERLAPS_H
#define QGSALGORITHMCALCULATEOVERLAPS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"
#include "qgsvectorlayerfeatureiterator.h"

///@cond PRIVATE

/**
 * Native calculate overlaps algorithm
 */
class QgsCalculateVectorOverlapsAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsCalculateVectorOverlapsAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QIcon icon() const override;
    [[nodiscard]] QString svgIconPath() const override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    [[nodiscard]] QgsCalculateVectorOverlapsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsProcessingFeatureSource> mSource;
    QStringList mLayerNames;
    std::vector<std::unique_ptr<QgsVectorLayerFeatureSource>> mOverlayerSources;
    QgsFields mOutputFields;
    Qgis::WkbType mOutputType = Qgis::WkbType::Unknown;
    QgsCoordinateReferenceSystem mCrs;
    QgsFeatureIterator mInputFeatures;
    long mInputCount = 0;
};

///@endcond PRIVATE

#endif // QGSALGORITHMCALCULATEOVERLAPS_H
