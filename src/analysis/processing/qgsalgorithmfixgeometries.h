/***************************************************************************
                         qgsalgorithmfixgeometries.h
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

#ifndef QGSALGORITHMFIXGEOMETRIES_H
#define QGSALGORITHMFIXGEOMETRIES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native repair geometries algorithm.
 */
class QgsFixGeometriesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsFixGeometriesAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsFixGeometriesAlgorithm *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    [[nodiscard]] Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] Qgis::WkbType outputWkbType( Qgis::WkbType type ) const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    Qgis::MakeValidMethod mMethod = Qgis::MakeValidMethod::Linework;
};

///@endcond PRIVATE

#endif // QGSALGORITHMFIXGEOMETRIES_H
