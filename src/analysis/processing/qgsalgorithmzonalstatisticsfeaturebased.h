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
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;
    QgsZonalStatisticsFeatureBasedAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsRasterInterface> mRaster;
    int mBand = 1;
    QString mPrefix;
    Qgis::ZonalStatistics mStats = Qgis::ZonalStatistic::All;
    QgsCoordinateReferenceSystem mCrs;
    bool mCreatedTransform = false;
    QgsCoordinateTransform mFeatureToRasterTransform;
    double mPixelSizeX = 0;
    double mPixelSizeY = 0;
    QgsFields mOutputFields;
    QMap<Qgis::ZonalStatistic, int> mStatFieldsMapping;
};

///@endcond PRIVATE

#endif // QGSALGORITHMZONALSTATISTICSFEATUREBASED_H
