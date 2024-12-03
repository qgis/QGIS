/***************************************************************************
                         qgsalgorithmroundrastervalues.h
                         ---------------------
    begin                : April 2020
    copyright            : (C) 2020 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSALGORITHMROUNDRASTERVALUES_H
#define QGSALGORITHMROUNDRASTERVALUES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Round raster values algorithm:
 * This algorithm rounds the Values of floating point raster datasets
 * based on a predefined precision value.
 */
class QgsRoundRasterValuesAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsRoundRasterValuesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmRoundRastervalues.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRoundRastervalues.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsRoundRasterValuesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    double roundNearest( double value, double m );
    double roundUp( double value, double m );
    double roundDown( double value, double m );
    double roundNearestBaseN( double value );
    double roundUpBaseN( double value );
    double roundDownBaseN( double value );

    int mDecimalPrecision = 2;
    int mBaseN = 10;
    double mScaleFactor = 0;
    int mMultipleOfBaseN = 0;
    int mBand = 1;
    int mRoundingDirection = 0;
    std::unique_ptr<QgsRasterInterface> mInterface;
    Qgis::DataType mDataType = Qgis::DataType::UnknownDataType;
    bool mIsInteger = false;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    int mNbCellsXProvider = 0;
    int mNbCellsYProvider = 0;
    double mInputNoDataValue = 0;
};

///@endcond PRIVATE

#endif // QGSALGORITHMROUNDRASTERVALUES_H
