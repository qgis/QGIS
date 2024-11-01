/***************************************************************************
                         qgsalgorithmrasterstackposition.h
                         ---------------------
    begin                : July 2020
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

#ifndef QGSALGORITHMRASTERSTACKPOSITION_H
#define QGSALGORITHMRASTERSTACKPOSITION_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"
#include "qgsrasterprojector.h"
#include "qgsrasteranalysisutils.h"

///@cond PRIVATE

class QgsRasterStackPositionAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    QgsRasterStackPositionAlgorithmBase() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString group() const override;
    QString groupId() const override;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    virtual int findPosition( std::vector<std::unique_ptr<QgsRasterBlock>> &rasterBlockStack, int &row, int &col, bool &noDataInRasterBlockStack ) = 0;
    double mNoDataValue = -9999;

  private:
    std::vector<QgsRasterAnalysisUtils::RasterLogicInput> mInputs;
    bool mIgnoreNoData = false;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX = 0;
    double mRasterUnitsPerPixelY = 0;
};

class QgsRasterStackLowestPositionAlgorithm : public QgsRasterStackPositionAlgorithmBase
{
  public:
    QgsRasterStackLowestPositionAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRasterStackLowestPositionAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    int findPosition( std::vector<std::unique_ptr<QgsRasterBlock>> &rasterBlockStack, int &row, int &col, bool &noDataInRasterBlockStack ) override;
};

class QgsRasterStackHighestPositionAlgorithm : public QgsRasterStackPositionAlgorithmBase
{
  public:
    QgsRasterStackHighestPositionAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRasterStackHighestPositionAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    int findPosition( std::vector<std::unique_ptr<QgsRasterBlock>> &rasterBlockStack, int &row, int &col, bool &noDataInRasterBlockStack ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMRASTERSTACKPOSITION_H
