/***************************************************************************
                         qgsalgorithmcellstatistics.h
                         ---------------------
    begin                : May 2020
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

#ifndef QGSALGORITHMCELLSTATISTICS_H
#define QGSALGORITHMCELLSTATISTICS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"
#include "qgsrasterprojector.h"
#include "qgsrasteranalysisutils.h"

///@cond PRIVATE

class QgsCellStatisticsAlgorithm : public QgsProcessingAlgorithm
{

  public:
    QgsCellStatisticsAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCellStatistics.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmCellStatistics.svg" ) ); }
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsCellStatisticsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::vector< QgsRasterAnalysisUtils::RasterLogicInput > mInputs;
    bool mIgnoreNoData;
    Qgis::DataType mDataType;
    double mNoDataValue = -9999;
    int mLayerWidth;
    int mLayerHeight;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX;
    double mRasterUnitsPerPixelY;
};

///@endcond PRIVATE

#endif // QGSALGORITHMCELLSTATISTICS_H

