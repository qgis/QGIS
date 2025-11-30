/***************************************************************************
                         qgsalgorithmrescaleraster.h
                         ------------------------------
    begin                : July 2020
    copyright            : (C) 2020 by Alexander Bruy
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

#ifndef QGSALGORITHMRESCALERASTER_H
#define QGSALGORITHMRESCALERASTER_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native rescale raster algorithm.
 */
class QgsRescaleRasterAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsRescaleRasterAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsRescaleRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    int mBand = 1;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    int mXSize = 0;
    int mYSize = 0;
    double mNoData = 0;
    double mMinimum = 0;
    double mMaximum = 0;

    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    std::unique_ptr<QgsRasterInterface> mInterface;
};

///@endcond PRIVATE

#endif // QGSALGORITHMRESCALERASTER_H
