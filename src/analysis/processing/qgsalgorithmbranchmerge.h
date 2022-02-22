/***************************************************************************
                         qgsalgorithmbranchmerge.h
                         ---------------------
    begin                : September 2021
    copyright            : (C) 2021 by Antoine Facchini
    email                : antoine dot facchini @oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMBRANCHMERGE_H
#define QGSALGORITHMBRANCHMERGE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

class QgsProcessingModelAlgorithm;
class QTableWidget;

///@cond PRIVATE

/**
 * Branch merge algorithm for modeler.
 * Takes 2 map layers as input and returns the first one if it is valid
 * else the second one. If the 2 layers aren't valid, raises an error.
 *
 * \since QGIS 3.26
 */
class QgsBranchMergeAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsBranchMergeAlgorithm() = default;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsBranchMergeAlgorithm *createInstance() const override SIP_FACTORY;


  protected:
    Flags flags() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QgsMapLayer *mDefaultInput;
    QgsMapLayer *mFallbackInput;
};

///@endcond PRIVATE

#endif // QGSALGORITHMBRANCHMERGE_H
