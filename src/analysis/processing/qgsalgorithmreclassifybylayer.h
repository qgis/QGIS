/***************************************************************************
                         qgsalgorithmreclassifybylayer.h
                         ---------------------
    begin                : June, 2018
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

#ifndef QGSALGORITHMRECLASSIFYBYLAYER_H
#define QGSALGORITHMRECLASSIFYBYLAYER_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native zonal histogram algorithm.
 */
class QgsReclassifyByLayerAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsReclassifyByLayerAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsReclassifyByLayerAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;


    struct Class
    {
      Class() = default;
      Class( double minValue, double maxValue, double value )
        : range( minValue, maxValue )
        , value( value )
      {}
      QgsRasterRange range;
      double value = 0;
    };


    void reclassify( const QVector< Class > &classes, QgsRasterDataProvider *destinationRaster, QgsProcessingFeedback *feedback );
    double reclassifyValue( const QVector< Class > &classes, double input ) const;

  private:

    std::unique_ptr< QgsRasterInterface > mInterface;

    int mMinFieldIdx = -1;
    int mMaxFieldIdx = -1;
    int mValueFieldIdx = -1;
    double mNoDataValue = -9999;

    QgsFeatureIterator mTableIterator;
    int mBand = 1;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX = 0;
    double mRasterUnitsPerPixelY = 0;
    int mNbCellsXProvider = 0;
    int mNbCellsYProvider = 0;

};

///@endcond PRIVATE

#endif // QGSALGORITHMRECLASSIFYBYLAYER_H


