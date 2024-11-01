/***************************************************************************
                         qgsalgorithmrasterlayeruniquevalues.h
                         ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Clemens Raffler
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

#ifndef QGSFUZZIFYRASTERALGORITHM_H
#define QGSFUZZIFYRASTERALGORITHM_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Base class for raster fuzzification algorithms.
 */
class QgsFuzzifyRasterAlgorithmBase : public QgsProcessingAlgorithm
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
     * Prepares the fuzzfication algorithm subclass for execution.
     */
    virtual bool prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) = 0;

    /**
     * Processes a raster using fuzzify() method which is implemented in subclasses providing different fuzzy membership types.
     */
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;

    /**
     * Pure virtual method fuzzify() performs subclass specific fuzzification.
     */
    virtual void fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback ) = 0;

    QgsRasterLayer *mInputRaster = nullptr;
    int mBand = 1;
    std::unique_ptr<QgsRasterInterface> mInterface;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    int mNbCellsXProvider = 0;
    int mNbCellsYProvider = 0;

    Qgis::DataType mDataType = Qgis::DataType::Float32;
    const double mNoDataValue = -9999;
};


class QgsFuzzifyRasterLinearMembershipAlgorithm : public QgsFuzzifyRasterAlgorithmBase
{
  public:
    QgsFuzzifyRasterLinearMembershipAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmFuzzifyLinear.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmFuzzifyLinear.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsFuzzifyRasterLinearMembershipAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() override;
    bool prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback ) override;

  private:
    double mFuzzifyLowBound = 0;
    double mFuzzifyHighBound = 0;
};


class QgsFuzzifyRasterPowerMembershipAlgorithm : public QgsFuzzifyRasterAlgorithmBase
{
  public:
    QgsFuzzifyRasterPowerMembershipAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmFuzzifyPower.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmFuzzifyPower.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsFuzzifyRasterPowerMembershipAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() override;
    bool prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback ) override;

  private:
    double mFuzzifyLowBound = 0;
    double mFuzzifyHighBound = 0;
    double mFuzzifyExponent = 0;
};


class QgsFuzzifyRasterLargeMembershipAlgorithm : public QgsFuzzifyRasterAlgorithmBase
{
  public:
    QgsFuzzifyRasterLargeMembershipAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmFuzzifyLarge.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmFuzzifyLarge.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsFuzzifyRasterLargeMembershipAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() override;
    bool prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback ) override;

  private:
    double mFuzzifyMidpoint = 0;
    double mFuzzifySpread = 0;
};


class QgsFuzzifyRasterSmallMembershipAlgorithm : public QgsFuzzifyRasterAlgorithmBase
{
  public:
    QgsFuzzifyRasterSmallMembershipAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmFuzzifySmall.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmFuzzifySmall.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsFuzzifyRasterSmallMembershipAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() override;
    bool prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback ) override;

  private:
    double mFuzzifyMidpoint = 0;
    double mFuzzifySpread = 0;
};


class QgsFuzzifyRasterGaussianMembershipAlgorithm : public QgsFuzzifyRasterAlgorithmBase
{
  public:
    QgsFuzzifyRasterGaussianMembershipAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmFuzzifyGaussian.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmFuzzifyGaussian.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsFuzzifyRasterGaussianMembershipAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() override;
    bool prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback ) override;

  private:
    double mFuzzifyMidpoint = 0;
    double mFuzzifySpread = 0;
};


class QgsFuzzifyRasterNearMembershipAlgorithm : public QgsFuzzifyRasterAlgorithmBase
{
  public:
    QgsFuzzifyRasterNearMembershipAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmFuzzifyNear.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmFuzzifyNear.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsFuzzifyRasterNearMembershipAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() override;
    bool prepareAlgorithmFuzzificationParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void fuzzify( QgsRasterDataProvider *destinationProvider, QgsProcessingFeedback *feedback ) override;

  private:
    double mFuzzifyMidpoint = 0;
    double mFuzzifySpread = 0;
};


///@endcond PRIVATE

#endif // QGSALGORITHMFUZZIFYRASTER_H
