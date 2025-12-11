/***************************************************************************
                         qgsalgorithmmergelines.h
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

#ifndef QGSALGORITHMMERGELINES_H
#define QGSALGORITHMMERGELINES_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingutils.h"

///@cond PRIVATE

/**
 * Native merge lines algorithm.
 */
class QgsMergeLinesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsMergeLinesAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QList<int> inputLayerTypes() const override;
    [[nodiscard]] QgsMergeLinesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] Qgis::ProcessingSourceType outputLayerType() const override;
    [[nodiscard]] Qgis::WkbType outputWkbType( Qgis::WkbType ) const override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMMERGELINES_H
