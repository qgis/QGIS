/***************************************************************************
                         qgsalgorithmzonalstatisticsfeaturebased.h
                         ------------------------------
    begin                : September 2020
    copyright            : (C) 2020 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMZONALSTATISTICSFEATUREBASED_H
#define QGSALGORITHMZONALSTATISTICSFEATUREBASED_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsvectorlayer.h"
#include "vector/qgszonalstatistics.h"

///@cond PRIVATE

/**
 * Native zonal statistics algorithm.
 */
class QgsZonalStatisticsFeatureBasedAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsZonalStatisticsFeatureBasedAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;

    QgsZonalStatisticsFeatureBasedAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  private:
    std::unique_ptr< QgsRasterInterface > mRaster;
    int mBand;
    QString mPrefix;
    QgsZonalStatistics::Statistics mStats = QgsZonalStatistics::All;
    QgsCoordinateReferenceSystem mCrs;
    bool mCreatedTransform = false;
    QgsCoordinateTransform mFeatureToRasterTransform;
    double mPixelSizeX;
    double mPixelSizeY;
    QgsFields mOutputFields;
    QMap<QgsZonalStatistics::Statistic, int> mStatFieldsMapping;
};

///@endcond PRIVATE

#endif // QGSALGORITHMZONALSTATISTICSFEATUREBASED_H
