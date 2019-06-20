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

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsreclassifyutils.h"

///@cond PRIVATE

/**
 * Base class for reclassify algorithms.
 */
class QgsReclassifyAlgorithmBase : public QgsProcessingAlgorithm
{
  public:

    QString group() const final;
    QString groupId() const final;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) final;

  protected:

    /**
     * Adds specific subclass algorithm parameters. The common parameters, such as raster destination, are automatically
     * added by the base class.
     */
    virtual void addAlgorithmParams() = 0;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;

    /**
     * Prepares the reclassify algorithm subclass for execution.
     */
    virtual bool _prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) = 0;

    /**
     * Returns a list of classes to use during the reclassification.
     */
    virtual QVector< QgsReclassifyUtils::RasterClass > createClasses(
      QgsReclassifyUtils::RasterClass::BoundsType boundsType,
      const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) = 0;

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;

    std::unique_ptr< QgsRasterInterface > mInterface;

    Qgis::DataType mDataType = Qgis::Float32;
    double mNoDataValue = -9999;
    int mBand = 1;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX = 0;
    double mRasterUnitsPerPixelY = 0;
    int mNbCellsXProvider = 0;
    int mNbCellsYProvider = 0;
    QgsReclassifyUtils::RasterClass::BoundsType mBoundsType = QgsReclassifyUtils::RasterClass::IncludeMax;
    bool mUseNoDataForMissingValues = false;
};

/**
 * Native reclassify by layer algorithm.
 */
class QgsReclassifyByLayerAlgorithm : public QgsReclassifyAlgorithmBase
{

  public:

    QgsReclassifyByLayerAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsReclassifyByLayerAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() override;
    bool _prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVector< QgsReclassifyUtils::RasterClass > createClasses(
      QgsReclassifyUtils::RasterClass::BoundsType boundsType,
      const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    int mMinFieldIdx = -1;
    int mMaxFieldIdx = -1;
    int mValueFieldIdx = -1;
    QgsFeatureIterator mTableIterator;

};

/**
 * Native reclassify by table algorithm.
 */
class QgsReclassifyByTableAlgorithm : public QgsReclassifyAlgorithmBase
{

  public:

    QgsReclassifyByTableAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsReclassifyByTableAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    void addAlgorithmParams() override;
    bool _prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVector< QgsReclassifyUtils::RasterClass > createClasses( QgsReclassifyUtils::RasterClass::BoundsType boundsType,
        const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMRECLASSIFYBYLAYER_H


