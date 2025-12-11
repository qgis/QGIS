/***************************************************************************
                         qgsalgorithmlayoutatlastopdf.h
                         ---------------------
    begin                : August 2020
    copyright            : (C) 2020 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMLAYOUTATLASTOPDF_H
#define QGSALGORITHMLAYOUTATLASTOPDF_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgslayoutexporter.h"
#include "qgsprocessingalgorithm.h"

class QgsLayoutAtlas;

///@cond PRIVATE

/**
 * Base class for atlas layout to pdf algorithms
 */
class QgsLayoutAtlasToPdfAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    QgsLayoutAtlasToPdfAlgorithmBase() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] Qgis::ProcessingAlgorithmFlags flags() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;

    QgsLayoutAtlas *atlas();
    QgsLayoutExporter exporter();
    QgsLayoutExporter::PdfExportSettings settings();
    QString error();

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    virtual QVariantMap exportAtlas( QgsLayoutAtlas *atlas, const QgsLayoutExporter &exporter, const QgsLayoutExporter::PdfExportSettings &settings, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) = 0;
};


/**
 * Export atlas layout to single pdf file algorithm.
 */
class QgsLayoutAtlasToPdfAlgorithm : public QgsLayoutAtlasToPdfAlgorithmBase
{
  public:
    QgsLayoutAtlasToPdfAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QgsLayoutAtlasToPdfAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap exportAtlas( QgsLayoutAtlas *atlas, const QgsLayoutExporter &exporter, const QgsLayoutExporter::PdfExportSettings &settings, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

/**
 * Export atlas layout to multiple pdf files algorithm.
 */
class QgsLayoutAtlasToMultiplePdfAlgorithm : public QgsLayoutAtlasToPdfAlgorithmBase
{
  public:
    QgsLayoutAtlasToMultiplePdfAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QgsLayoutAtlasToMultiplePdfAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap exportAtlas( QgsLayoutAtlas *atlas, const QgsLayoutExporter &exporter, const QgsLayoutExporter::PdfExportSettings &settings, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMLAYOUTATLASTOPDF_H
