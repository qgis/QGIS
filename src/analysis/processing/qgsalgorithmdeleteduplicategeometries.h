/***************************************************************************
                         qgsalgorithmdeleteduplicategeometries.h
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

#ifndef QGSALGORITHMDELETEDUPLICATEGEOMETRIES_H
#define QGSALGORITHMDELETEDUPLICATEGEOMETRIES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"
#include "qgsvectorlayerfeatureiterator.h"

///@cond PRIVATE

/**
 * Native delete duplicate geometries
 */
class QgsDeleteDuplicateGeometriesAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsDeleteDuplicateGeometriesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsDeleteDuplicateGeometriesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsProcessingFeatureSource> mSource;
};

///@endcond PRIVATE

#endif // QGSALGORITHMDELETEDUPLICATEGEOMETRIES_H
