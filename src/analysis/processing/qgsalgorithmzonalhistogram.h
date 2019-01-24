/***************************************************************************
                         qgsalgorithmzonalhistogram.h
                         ---------------------
    begin                : May, 2018
    copyright            : (C) 2018 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMZONALHISTOGRAM_H
#define QGSALGORITHMZONALHISTOGRAM_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native zonal histogram algorithm.
 */
class QgsZonalHistogramAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsZonalHistogramAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsZonalHistogramAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    std::unique_ptr< QgsRasterInterface > mRasterInterface;
    int mRasterBand;
    bool mHasNoDataValue = false;
    float mNodataValue = -1;
    QgsRectangle mRasterExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mCellSizeX;
    double mCellSizeY;
    double mNbCellsXProvider;
    double mNbCellsYProvider;

};

///@endcond PRIVATE

#endif // QGSALGORITHMZONALHISTOGRAM_H


