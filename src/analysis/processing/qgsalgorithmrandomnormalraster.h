/***************************************************************************
                         qgsalgorithmrandomnormalraster.h
                         ---------------------
    begin                : April 2020
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

#ifndef QGSRANDOMNORMALRASTERALGORITHM_H
#define QGSRANDOMNORMALRASTERALGORITHM_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Algorithm for random normal distributed raster creation algorithms.
 */

class QgsRandomNormalRasterAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsRandomNormalRasterAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) final;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmNormalRaster.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmNormalRaster.svg" ) ); }
    QString name() const override;
    QString group() const override;
    QString groupId() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRandomNormalRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;

  private:
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mPixelSize;
    Qgis::DataType mRasterDataType;
    int mTypeId;

    double mMean;
    double mStdev;
};

///@endcond PRIVATE

#endif // QGSRANDOMNORMALRASTERALGORITHM_H
