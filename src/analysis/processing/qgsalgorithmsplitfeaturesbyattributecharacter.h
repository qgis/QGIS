/***************************************************************************
                         qgsalgorithmsplitfeaturesbyattributecharacter.h
                         ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#ifndef QGSALGORITHMSPLITFEATURESBYATTRIBUTECHARACTER_H
#define QGSALGORITHMSPLITFEATURESBYATTRIBUTECHARACTER_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#include <QRegularExpression>

///@cond PRIVATE

/**
 * Native explode lines algorithm.
 */
class QgsSplitFeaturesByAttributeCharacterAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsSplitFeaturesByAttributeCharacterAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    QList<int> inputLayerTypes() const override;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    QgsSplitFeaturesByAttributeCharacterAlgorithm *createInstance() const override SIP_FACTORY;
    QgsFields outputFields( const QgsFields &inputFields ) const override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureSink::SinkFlags sinkFlags() const override;

  private:
    QString mFieldName;
    mutable int mFieldIndex = -1;
    QString mChar;
    bool mUseRegex = false;
    QRegularExpression mRegex;
};

///@endcond PRIVATE

#endif // QGSALGORITHMSPLITFEATURESBYATTRIBUTECHARACTER_H
