/***************************************************************************
                         qgsalgorithmdetectdatasetchanges.h
                         ---------------------------------------
    begin                : December 2019
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

#ifndef QGSALGORITHMDETECTDATASETCHANGES_H
#define QGSALGORITHMDETECTDATASETCHANGES_H


#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"
#include "qgsvectorlayerfeatureiterator.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Native detect changes algorithm
 */
class QgsDetectVectorChangesAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsDetectVectorChangesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsDetectVectorChangesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    enum GeometryMatchType
    {
      Exact,
      Topological
    };

    std::unique_ptr<QgsProcessingFeatureSource> mOriginal;
    std::unique_ptr<QgsProcessingFeatureSource> mRevised;
    QStringList mFieldsToCompare;
    QList<int> mOriginalFieldsToCompareIndices;
    QList<int> mRevisedFieldsToCompareIndices;

    GeometryMatchType mMatchType = Topological;
};

///@endcond PRIVATE

#endif // QGSALGORITHMDETECTDATASETCHANGES_H
