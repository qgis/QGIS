/***************************************************************************
                         qgsalgorithmvectorize.h
                         ---------------------
    begin                : June, 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#ifndef QGSALGORITHMVECTORIZE_H
#define QGSALGORITHMVECTORIZE_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsreclassifyutils.h"

///@cond PRIVATE

/**
 * Native vectorize algorithm
 */
class QgsVectorizeAlgorithm : public QgsProcessingAlgorithm
{
  public:

    QgsVectorizeAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsVectorizeAlgorithm *createInstance() const override SIP_FACTORY;
    QString group() const override final;
    QString groupId() const override final;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    std::unique_ptr< QgsRasterInterface > mInterface;

    Qgis::DataType mDataType = Qgis::Float32;
    double mNoDataValue = -9999;
    int mBand = 1;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX = 0;
    double mRasterUnitsPerPixelY = 0;
    int mNbCellsXProvider = 0;
    int mNbCellsYProvider = 0;
    QgsReclassifyUtils::RasterClass::BoundsType mBoundsType = QgsReclassifyUtils::RasterClass::IncludeMax;
    bool mUseNoDataForMissingValues = false;
};

///@endcond PRIVATE

#endif // QGSALGORITHMVECTORIZE_H


