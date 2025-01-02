/***************************************************************************
                         qgsalgorithmfillnodata.h
                         ---------------------
    begin                : January 2020
    copyright            : (C) 2020 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSALGORITHMFILLNODATA_H
#define QGSALGORITHMFILLNODATA_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Fill NoData algorithm:
 * This is an algorithm that fills the NoDataValues in a raster dataset
 * based on a dynamic input parameter
 */
class QgsFillNoDataAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsFillNoDataAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmFillNoData.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmFillNoData.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsFillNoDataAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QgsRasterLayer *mInputRaster = nullptr;
    double mFillValue = 0;
    int mBand = 1;
    std::unique_ptr<QgsRasterInterface> mInterface;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    int mNbCellsXProvider = 0;
    int mNbCellsYProvider = 0;
    double mInputNoDataValue = 0;
};

///@endcond PRIVATE

#endif // QGSALGORITHMFILLNODATA_H
