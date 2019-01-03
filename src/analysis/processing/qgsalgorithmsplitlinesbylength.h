/***************************************************************************
                         qgsalgorithmsplitlinesbylength.h
                         ---------------------
    begin                : December 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#ifndef QGSALGORITHMSPLITLINESBYLENGTH_H
#define QGSALGORITHMSPLITLINESBYLENGTH_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native split lines by maximum length algorithm.
 */
class QgsSplitLinesByLengthAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsSplitLinesByLengthAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QList<int> inputLayerTypes() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsSplitLinesByLengthAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  protected:
    QString outputName() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    double mLength = 0.0;
    bool mDynamicLength = false;
    QgsProperty mLengthProperty;

};

///@endcond PRIVATE

#endif // QGSALGORITHMSPLITLINESBYLENGTH_H


