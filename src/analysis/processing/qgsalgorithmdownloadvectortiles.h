/***************************************************************************
                         qgsalgorithmdownloadvectortiles.h
                         ---------------------
    begin                : May 2023
    copyright            : (C) 2023 by Alexander Bruy
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

#ifndef QGSALGORITHMDOWNLOADVECTORTILES_H
#define QGSALGORITHMDOWNLOADVECTORTILES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#include "qgsvectortilematrixset.h"
#include "qgsvectortiledataprovider.h"

///@cond PRIVATE

/**
 * Native download vector tiles algorithm.
 */
class QgsDownloadVectorTilesAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsDownloadVectorTilesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsDownloadVectorTilesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsVectorTileDataProvider> mProvider;
    int mMaxZoom = 10;
    int mSourceMinZoom = 1;
    long long mTileLimit = 100;
    QString mLayerName;
    QgsRectangle mExtent;
    QgsVectorTileMatrixSet mTileMatrixSet;
    QDomDocument mStyleDocument;
};

///@endcond PRIVATE

#endif // QGSALGORITHMDOWNLOADVECTORTILES_H
