/***************************************************************************
                         qgsalgorithmtessellate.h
                         ---------------------
    begin                : November 2017
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

#ifndef QGSALGORITHMTESSELLATE_H
#define QGSALGORITHMTESSELLATE_H

#define SIP_NO_FILE

#include "processing/qgsprocessingalgorithm.h"
#include "qgis.h"

///@cond PRIVATE

/**
 * Native tessellate algorithm.
 */
class QgsTessellateAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsTessellateAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QList<int> inputLayerTypes() const override;
    [[nodiscard]] QgsTessellateAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] Qgis::ProcessingSourceType outputLayerType() const override;
    [[nodiscard]] Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;

    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};


///@endcond PRIVATE

#endif // QGSALGORITHMTESSELLATE_H
