/***************************************************************************
                         qgsalgorithmrasterminmax.cpp
                         ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#ifndef QGSALGORITHMRASTERMINMAX_H
#define QGSALGORITHMRASTERMINMAX_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native raster layer minimum and/or maximum value algorithm.
 */
class QgsRasterMinMaxAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsRasterMinMaxAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsRasterMinMaxAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsRasterInterface> mInterface;
    bool mHasNoDataValue = false;
    int mBand = 1;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX = 0;
    double mRasterUnitsPerPixelY = 0;
};

///@endcond PRIVATE

#endif // QGSALGORITHMRASTERMINMAX_H
