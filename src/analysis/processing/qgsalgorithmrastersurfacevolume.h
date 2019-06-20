/***************************************************************************
                         qgsalgorithmrasterlayeruniquevalues.cpp
                         ---------------------
    begin                : January 2019
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

#ifndef QGSALGORITHMRASTERSURFACEVOLUME_H
#define QGSALGORITHMRASTERSURFACEVOLUME_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native raster layer unique values report algorithm.
 */
class QgsRasterSurfaceVolumeAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsRasterSurfaceVolumeAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsRasterSurfaceVolumeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    enum Method
    {
      CountOnlyAboveBaseLevel = 0,
      CountOnlyBelowBaseLevel,
      SubtractVolumesBelowBaseLevel,
      AddVolumesBelowBaseLevel
    };

    std::unique_ptr< QgsRasterInterface > mInterface;
    bool mHasNoDataValue = false;
    int mBand = 1;
    int mLayerWidth;
    int mLayerHeight;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX;
    double mRasterUnitsPerPixelY;
    double mLevel = 0;
    QString mSource;
    Method mMethod;

};

///@endcond PRIVATE

#endif // QGSALGORITHMRASTERSURFACEVOLUME_H


