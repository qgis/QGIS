/***************************************************************************
                         qgsalgorithmdropmzvalues.h
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

#ifndef QGSALGORITHMDROPMZVALUES_H
#define QGSALGORITHMDROPMZVALUES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native drop M/Z values algorithm.
 */
class QgsDropMZValuesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsDropMZValuesAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsDropMZValuesAlgorithm *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    [[nodiscard]] Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    bool mDropM = false;
    bool mDropZ = false;
};

///@endcond PRIVATE

#endif // QGSALGORITHMDROPMZVALUES_H
