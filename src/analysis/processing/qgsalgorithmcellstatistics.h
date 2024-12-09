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

class QgsCellStatisticsAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    QString group() const final;
    QString groupId() const final;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) final;


  protected:
    virtual void addSpecificAlgorithmParams() = 0;
    virtual bool prepareSpecificAlgorithmParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) = 0;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;
    virtual void processRasterStack( QgsProcessingFeedback *feedback ) = 0;

    std::vector<QgsRasterAnalysisUtils::RasterLogicInput> mInputs;
    bool mIgnoreNoData = false;
    Qgis::DataType mDataType = Qgis::DataType::UnknownDataType;
    double mNoDataValue = -9999;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX = 0;
    double mRasterUnitsPerPixelY = 0;
    std::unique_ptr<QgsRasterDataProvider> mOutputRasterDataProvider;
};

class QgsCellStatisticsAlgorithm : public QgsCellStatisticsAlgorithmBase
{
  public:
    QgsCellStatisticsAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCellStatistics.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmCellStatistics.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsCellStatisticsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addSpecificAlgorithmParams() override;
    bool prepareSpecificAlgorithmParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void processRasterStack( QgsProcessingFeedback *feedback ) override;

  private:
    QgsRasterAnalysisUtils::CellValueStatisticMethods mMethod = QgsRasterAnalysisUtils::CellValueStatisticMethods::Sum;
};


class QgsCellStatisticsPercentileAlgorithm : public QgsCellStatisticsAlgorithmBase
{
  public:
    QgsCellStatisticsPercentileAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCellStatisticsPercentile.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmCellStatisticsPercentile.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsCellStatisticsPercentileAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addSpecificAlgorithmParams() override;
    bool prepareSpecificAlgorithmParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void processRasterStack( QgsProcessingFeedback *feedback ) override;

  private:
    QgsRasterAnalysisUtils::CellValuePercentileMethods mMethod = QgsRasterAnalysisUtils::CellValuePercentileMethods::NearestRankPercentile;
    double mPercentile = 0.0;
};


class QgsCellStatisticsPercentRankFromValueAlgorithm : public QgsCellStatisticsAlgorithmBase
{
  public:
    QgsCellStatisticsPercentRankFromValueAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCellStatisticsPercentRank.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmCellStatisticsPercentRank.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsCellStatisticsPercentRankFromValueAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addSpecificAlgorithmParams() override;
    bool prepareSpecificAlgorithmParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void processRasterStack( QgsProcessingFeedback *feedback ) override;

  private:
    QgsRasterAnalysisUtils::CellValuePercentRankMethods mMethod = QgsRasterAnalysisUtils::CellValuePercentRankMethods::InterpolatedPercentRankInc;
    double mValue = 0.0;
};


class QgsCellStatisticsPercentRankFromRasterAlgorithm : public QgsCellStatisticsAlgorithmBase
{
  public:
    QgsCellStatisticsPercentRankFromRasterAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCellStatisticsPercentRank.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmCellStatisticsPercentRank.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsCellStatisticsPercentRankFromRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addSpecificAlgorithmParams() override;
    bool prepareSpecificAlgorithmParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void processRasterStack( QgsProcessingFeedback *feedback ) override;

  private:
    QgsRasterAnalysisUtils::CellValuePercentRankMethods mMethod = QgsRasterAnalysisUtils::CellValuePercentRankMethods::InterpolatedPercentRankInc;
    std::unique_ptr<QgsRasterInterface> mValueRasterInterface;
    int mValueRasterBand = 1;
};


///@endcond PRIVATE

#endif // QGSALGORITHMCELLSTATISTICS_H
