/***************************************************************************
                         qgsalgorithmaddtablefield.h
                         ---------------------------------
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

#ifndef QGSALGORITHMADDTABLEFIELD_H
#define QGSALGORITHMADDTABLEFIELD_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native add table field algorithm.
 */
class QgsAddTableFieldAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsAddTableFieldAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QList<int> inputLayerTypes() const override;
    [[nodiscard]] QgsAddTableFieldAlgorithm *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] QgsFields outputFields( const QgsFields &inputFields ) const override;
    [[nodiscard]] Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QgsField mField;
};

///@endcond PRIVATE

#endif // QGSALGORITHMADDTABLEFIELD_H
