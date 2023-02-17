/***************************************************************************
                         qgsalgorithmrasterlogicalop.h
                         ---------------------
    begin                : March 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#ifndef QGSALGORITHMRASTERLOGICALOP_H
#define QGSALGORITHMRASTERLOGICALOP_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsrasterprojector.h"
#include "qgsrasteranalysisutils.h"

///@cond PRIVATE

/**
 * Base class for raster boolean logic algorithms
 */
class ANALYSIS_EXPORT QgsRasterBooleanLogicAlgorithmBase : public QgsProcessingAlgorithm
{

  public:

    QgsRasterBooleanLogicAlgorithmBase() = default;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    std::function<void( const std::vector< std::unique_ptr< QgsRasterBlock > > &, bool &, bool &, int, int, bool )> mExtractValFunc;

  private:

    std::vector< QgsRasterAnalysisUtils::RasterLogicInput > mInputs;
    Qgis::DataType mDataType = Qgis::DataType::Float32;
    double mNoDataValue = -9999;
    int mLayerWidth;
    int mLayerHeight;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX;
    double mRasterUnitsPerPixelY;
    bool mTreatNodataAsFalse = false;
    friend class TestQgsProcessingAlgsPt1;
};

/**
 * Native raster boolean OR operation algorithm.
 */
class ANALYSIS_EXPORT QgsRasterLogicalOrAlgorithm : public QgsRasterBooleanLogicAlgorithmBase
{

  public:

    QgsRasterLogicalOrAlgorithm();

    QString name() const override;
    QString displayName() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsRasterLogicalOrAlgorithm *createInstance() const override SIP_FACTORY;

};

/**
 * Native raster boolean AND operation algorithm.
 */
class ANALYSIS_EXPORT QgsRasterLogicalAndAlgorithm : public QgsRasterBooleanLogicAlgorithmBase
{

  public:

    QgsRasterLogicalAndAlgorithm();

    QString name() const override;
    QString displayName() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsRasterLogicalAndAlgorithm *createInstance() const override SIP_FACTORY;

};

///@endcond PRIVATE

#endif // QGSALGORITHMRASTERLOGICALOP_H


