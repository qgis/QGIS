/***************************************************************************
                         qgsalgorithmrasterzonalstats.h
                         ---------------------
    begin                : December 2018
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

#ifndef QGSALGORITHMRASTERZONALSTATS_H
#define QGSALGORITHMRASTERZONALSTATS_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsrasterprojector.h"

///@cond PRIVATE

/**
 * Native raster layer unique values report algorithm.
 */
class QgsRasterLayerZonalStatsAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsRasterLayerZonalStatsAlgorithm() = default;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsRasterLayerZonalStatsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    enum RefLayer
    {
      Source,
      Zones
    };

    std::unique_ptr< QgsRasterInterface > mSourceDataProvider;
    std::unique_ptr< QgsRasterInterface > mZonesDataProvider;
    std::unique_ptr< QgsRasterProjector> mProjector;
    QgsRasterInterface *mSourceInterface = nullptr;
    QgsRasterInterface *mZonesInterface = nullptr;
    bool mHasNoDataValue = false;
    bool mZonesHasNoDataValue = false;
    int mBand = 1;
    int mZonesBand = 1;
    int mLayerWidth;
    int mLayerHeight;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX;
    double mRasterUnitsPerPixelY;
    RefLayer mRefLayer = Source;

};

///@endcond PRIVATE

#endif // QGSALGORITHMRASTERZONALSTATS_H


