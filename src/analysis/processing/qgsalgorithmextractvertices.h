/***************************************************************************
                         qgsalgorithmextractvertices.h
                         -------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Mathieu Pellerin
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

#ifndef QGSALGORITHMEXTRACTVERTICES_H
#define QGSALGORITHMEXTRACTVERTICES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native extract nodes algorithm.
 */
class QgsExtractVerticesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsExtractVerticesAlgorithm() = default;
    [[nodiscard]] QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmExtractVertices.svg" ) ); }
    [[nodiscard]] QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmExtractVertices.svg" ) ); }
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    [[nodiscard]] QgsExtractVerticesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] QgsFields outputFields( const QgsFields &inputFields ) const override;
    [[nodiscard]] Qgis::ProcessingSourceType outputLayerType() const override;
    [[nodiscard]] Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    [[nodiscard]] Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
    [[nodiscard]] QgsFeatureSink::SinkFlags sinkFlags() const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    Qgis::GeometryType mGeometryType = Qgis::GeometryType::Unknown;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXTRACTVERTICES_H
