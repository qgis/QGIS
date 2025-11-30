/***************************************************************************
  qgsalgorithmextractlabels.h - QgsExtractLabelsAlgorithm
 ---------------------
 begin                : 30.12.2021
 copyright            : (C) 2021 by Mathieu Pellerin
 email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXTRACTLABELS_H
#define QGSALGORITHMEXTRACTLABELS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgslabelingenginesettings.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

class QgsExtractLabelsAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsExtractLabelsAlgorithm() = default;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] Qgis::ProcessingAlgorithmFlags flags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    [[nodiscard]] QgsExtractLabelsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QList<QgsMapLayer *> mMapLayers;
    QMap<QString, QString> mMapLayerNames;
    QMap<QString, QString> mMapThemeStyleOverrides;
    QgsLabelingEngineSettings mLabelSettings;
    QgsCoordinateReferenceSystem mCrs;
    Qgis::ScaleCalculationMethod mScaleMethod = Qgis::ScaleCalculationMethod::HorizontalMiddle;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXTRACTLABELS_H
