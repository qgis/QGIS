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
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmExtractVertices.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmExtractVertices.svg"_s ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    QgsExtractVerticesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
    QgsFeatureSink::SinkFlags sinkFlags() const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    Qgis::GeometryType mGeometryType = Qgis::GeometryType::Unknown;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXTRACTVERTICES_H
