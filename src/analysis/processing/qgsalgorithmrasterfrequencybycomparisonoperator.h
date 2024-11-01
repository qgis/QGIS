/***************************************************************************
                         qgsalgorithmrasterfrequencybycomparisonoperator.h
                         ---------------------
    begin                : June 2020
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

#ifndef QGSALGORITHMRASTERFREQUENCYBYCOMPARISON_H
#define QGSALGORITHMRASTERFREQUENCYBYCOMPARISON_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"
#include "qgsrasterprojector.h"
#include "qgsrasteranalysisutils.h"

///@cond PRIVATE

class QgsRasterFrequencyByComparisonOperatorBase : public QgsProcessingAlgorithm
{
  public:
    QgsRasterFrequencyByComparisonOperatorBase() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString group() const override;
    QString groupId() const override;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    virtual int applyComparisonOperator( double value, std::vector<double> cellValueStack ) = 0;

  private:
    std::unique_ptr<QgsRasterInterface> mInputValueRasterInterface;
    int mInputValueRasterBand = 1;
    std::vector<QgsRasterAnalysisUtils::RasterLogicInput> mInputs;
    bool mIgnoreNoData = false;
    double mNoDataValue = -9999;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX = 0;
    double mRasterUnitsPerPixelY = 0;
};

class QgsRasterFrequencyByEqualOperatorAlgorithm : public QgsRasterFrequencyByComparisonOperatorBase
{
  public:
    QgsRasterFrequencyByEqualOperatorAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRasterFrequencyByEqualOperatorAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    int applyComparisonOperator( double searchValue, std::vector<double> cellValueStack ) override;
};

class QgsRasterFrequencyByGreaterThanOperatorAlgorithm : public QgsRasterFrequencyByComparisonOperatorBase
{
  public:
    QgsRasterFrequencyByGreaterThanOperatorAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRasterFrequencyByGreaterThanOperatorAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    int applyComparisonOperator( double value, std::vector<double> cellValueStack ) override;
};

class QgsRasterFrequencyByLessThanOperatorAlgorithm : public QgsRasterFrequencyByComparisonOperatorBase
{
  public:
    QgsRasterFrequencyByLessThanOperatorAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRasterFrequencyByLessThanOperatorAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    int applyComparisonOperator( double value, std::vector<double> cellValueStack ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMRASTERFREQUENCYBYCOMPARISON_H
