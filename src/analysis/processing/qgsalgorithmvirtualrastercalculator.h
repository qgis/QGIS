/***************************************************************************
                         qgsalgorithmvirtualrastercalculator.h
                         ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Alexander Bruy
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

#ifndef QGSALGORITHMVIRTUALRASTERCALCULATOR_H
#define QGSALGORITHMVIRTUALRASTERCALCULATOR_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Native virtual raster calculator algorithm.
 */
class QgsVirtualRasterCalculatorAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsVirtualRasterCalculatorAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmRasterCalculator.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRasterCalculator.svg" ) ); }
    Qgis::ProcessingAlgorithmFlags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsVirtualRasterCalculatorAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    QList<QgsMapLayer *> mLayers;
};

class QgsVirtualRasterCalculatorModelerAlgorithm : public QgsVirtualRasterCalculatorAlgorithm
{
  public:
    QgsVirtualRasterCalculatorModelerAlgorithm() = default;
    Qgis::ProcessingAlgorithmFlags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QgsVirtualRasterCalculatorModelerAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    /**
     * Generates Excel-like names from the number
     * A, B, C, …, Y, Z, AA, AB, AC, …, AZ, BA, BB, BC…
     */
    QString indexToName( int index ) const;
};

///@endcond PRIVATE

#endif // QGSALGORITHMVIRTUALRASTERCALCULATOR_H
