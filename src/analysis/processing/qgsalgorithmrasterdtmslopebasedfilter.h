/***************************************************************************
                         qgsalgorithmrasterdtmslopebasedfilter.h
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#ifndef QGSALGORITHMRASTERDTMSLOPEBASEDFILTER_H
#define QGSALGORITHMRASTERDTMSLOPEBASEDFILTER_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native raster DTM slope-based filter algorithm.
 */
class QgsRasterDtmSlopeBasedFilterAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsRasterDtmSlopeBasedFilterAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QgsRasterDtmSlopeBasedFilterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsRasterInterface> mInterface;
    bool mHasNoDataValue = false;
    double mNoData = 0;
    int mBand = 1;
    Qgis::DataType mDataType = Qgis::DataType::UnknownDataType;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX = 0;
    double mRasterUnitsPerPixelY = 0;
};

///@endcond PRIVATE

#endif // QGSALGORITHMRASTERDTMSLOPEBASEDFILTER_H
