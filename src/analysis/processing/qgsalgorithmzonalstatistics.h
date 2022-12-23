/***************************************************************************
                         qgsalgorithmzonalstatistics.h
                         ------------------------------
    begin                : December 2019
    copyright            : (C) 2019 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMZONALSTATISTICS_H
#define QGSALGORITHMZONALSTATISTICS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsvectorlayer.h"
#include "vector/qgszonalstatistics.h"

///@cond PRIVATE

/**
 * Native zonal statistics algorithm.
 */
class QgsZonalStatisticsAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsZonalStatisticsAlgorithm() = default;
    Flags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QgsZonalStatisticsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr< QgsRasterInterface > mInterface;
    int mBand;
    QString mPrefix;
    QgsZonalStatistics::Statistics mStats = QgsZonalStatistics::All;
    QgsCoordinateReferenceSystem mCrs;
    double mPixelSizeX;
    double mPixelSizeY;
};

///@endcond PRIVATE

#endif // QGSALGORITHMZONALSTATISTICS_H
