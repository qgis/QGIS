/***************************************************************************
                         qgsalgorithmflattenrelationships.h
                         ---------------------
    begin                : August 2020
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

#ifndef QGSALGORITHMFLATTENRELATIONSHIPS_H
#define QGSALGORITHMFLATTENRELATIONSHIPS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsrelation.h"

class QgsVectorLayerFeatureSource;

///@cond PRIVATE

/**
 * Native join by attribute algorithm.
 */
class QgsFlattenRelationshipsAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsFlattenRelationshipsAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    Qgis::ProcessingAlgorithmFlags flags() const override;
    QgsFlattenRelationshipsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QgsRelation mRelation;
    std::unique_ptr<QgsVectorLayerFeatureSource> mReferencingSource;
    QgsFields mReferencingFields;
};

///@endcond PRIVATE

#endif // QGSALGORITHMFLATTENRELATIONSHIPS_H
