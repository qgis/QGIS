/***************************************************************************
                         qgsalgorithmfillsinkswangliu.h
                         ---------------------
    begin                : April 2025
    copyright            : (C) 2025 by Nyall Dawson
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

#ifndef QGSALGORITHMFILLSINKSWANGLIU_H
#define QGSALGORITHMFILLSINKSWANGLIU_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native raster DTM slope-based filter algorithm.
 */
class QgsFillSinksWangLiuAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsFillSinksWangLiuAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QgsFillSinksWangLiuAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    enum Direction
    {
      Invalid = -1,
      North = 0,
      NorthEast = 1,
      East = 2,
      SouthEast = 3,
      South = 4,
      SouthWest = 5,
      West = 6,
      NorthWest = 7
    };

    bool isInGrid( int row, int col ) const;
    Direction getDir( int row, int col, double z, const QgsRasterBlock *filled ) const;

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
    double mRasterDiagonal = 0;

    std::array< double, 8 > mDirectionalLengths { 0, 0, 0, 0, 0, 0, 0, 0 };

    static constexpr int INVERSE_DIRECTION[8] = { 4, 5, 6, 7, 0, 1, 2, 3 };
};

///@endcond PRIVATE

#endif // QGSALGORITHMFILLSINKSWANGLIU_H
