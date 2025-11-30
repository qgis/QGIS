/***************************************************************************
                         qgsalgorithmrenametablefield.h
                         ---------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSALGORITHMRENAMETABLEFIELD_H
#define QGSALGORITHMRENAMETABLEFIELD_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native add table field algorithm.
 */
class QgsRenameTableFieldAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsRenameTableFieldAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QList<int> inputLayerTypes() const override;
    [[nodiscard]] QgsRenameTableFieldAlgorithm *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] QgsFields outputFields( const QgsFields &inputFields ) const override;
    [[nodiscard]] Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QString mOriginalName;
    QString mNewName;
};

///@endcond PRIVATE

#endif // QGSALGORITHMRENAMETABLEFIELD_H
