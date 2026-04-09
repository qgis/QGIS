/***************************************************************************
                         qgsalgorithmhypsometriccurves.h
                         ---------------------
    begin                : April 2026
    copyright            : (C) 2026 by Alexander Bruy
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

#ifndef QGSALGORITHMHYPSOMETRICCURVES_H
#define QGSALGORITHMHYPSOMETRICCURVES_H

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Native hypsometric curves algorithm.
 */
class QgsHypsometricCurvesAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsHypsometricCurvesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsHypsometricCurvesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;

  private:
    void calculateHypsometry( const QVector<double> &elevations, const QString &filePath, QgsProcessingFeedback *feedback ) const;

    std::unique_ptr<QgsRasterInterface> mRasterInterface;
    bool mHasNoDataValue = false;
    double mNodataValue = -1;
    QgsRectangle mRasterExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mCellSizeX = 0;
    double mCellSizeY = 0;
    double mNbCellsXProvider = 0;
    double mNbCellsYProvider = 0;
    bool mUsePercentage = false;
    double mStep = 0;

    static constexpr qgssize MAX_BINS = 1000;
};

///@endcond PRIVATE


#endif // QGSALGORITHMHYPSOMETRICCURVES_H
