/***************************************************************************
                         qgsalgorithmextractspecificvertices.h
                         -------------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXTRACTSPECIFICVERTICES_H
#define QGSALGORITHMEXTRACTSPECIFICVERTICES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Native extract specific vertices algorithm.
 */
class QgsExtractSpecificVerticesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsExtractSpecificVerticesAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QgsExtractSpecificVerticesAlgorithm *createInstance() const override SIP_FACTORY;

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
    Qgis::GeometryType mGeometryType;
    QList<int> mIndices;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXTRACTSPECIFICVERTICES_H
