/***************************************************************************
                         qgsalgorithmrandomraster.h
                         ---------------------
    begin                : May 2020
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

#ifndef QGSRANDOMRASTERALGORITHM_H
#define QGSRANDOMRASTERALGORITHM_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"
#include "random"

///@cond PRIVATE

class QgsRandomRasterAlgorithmBase : public QgsProcessingAlgorithm
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
     * Prepares the random number algorithm subclass for execution.
     */
    virtual Qgis::DataType getRasterDataType( int typeId ) = 0;
    virtual bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) = 0;

    /**
     * Processes a raster using the generateRandomIntValues method which is implemented in subclasses providing different fuzzy membership types.
     */
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;

    /**
     * Virtual methods for random number generation may be overridden by subclassed algorithms to use specific random number distributions.
     */
    virtual long generateRandomLongValue( std::mt19937 &mersenneTwister ) = 0;
    virtual double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) = 0;

  private:
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mPixelSize = 0;
    Qgis::DataType mRasterDataType = Qgis::DataType::UnknownDataType;
};


class QgsRandomUniformRasterAlgorithm : public QgsRandomRasterAlgorithmBase
{
  public:
    QgsRandomUniformRasterAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmRandomRaster.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRandomRaster.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRandomUniformRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() final;
    Qgis::DataType getRasterDataType( int typeId ) final;
    bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) final;
    long generateRandomLongValue( std::mt19937 &mersenneTwister ) final;
    double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) final;

  private:
    double mRandomUpperBound = 0;
    double mRandomLowerBound = 0;
    std::uniform_int_distribution<long> mRandomUniformIntDistribution;
    std::uniform_real_distribution<double> mRandomUniformDoubleDistribution;
};

class QgsRandomBinomialRasterAlgorithm : public QgsRandomRasterAlgorithmBase
{
  public:
    QgsRandomBinomialRasterAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmNormalRaster.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRandomNormalRaster.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRandomBinomialRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() final;
    Qgis::DataType getRasterDataType( int typeId ) final;
    bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) final;
    long generateRandomLongValue( std::mt19937 &mersenneTwister ) final;
    double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) final;

  private:
    std::binomial_distribution<long> mRandombinomialDistribution;
};

class QgsRandomExponentialRasterAlgorithm : public QgsRandomRasterAlgorithmBase
{
  public:
    QgsRandomExponentialRasterAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmRandomExponentialRaster.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRandomExponentialRaster.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRandomExponentialRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() final;
    Qgis::DataType getRasterDataType( int typeId ) final;
    bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) final;
    long generateRandomLongValue( std::mt19937 &mersenneTwister ) final;
    double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) final;

  private:
    std::exponential_distribution<double> mRandomExponentialDistribution;
};


class QgsRandomGammaRasterAlgorithm : public QgsRandomRasterAlgorithmBase
{
  public:
    QgsRandomGammaRasterAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmRandomGammaRaster.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRandomGammaRaster.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRandomGammaRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() final;
    Qgis::DataType getRasterDataType( int typeId ) final;
    bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) final;
    long generateRandomLongValue( std::mt19937 &mersenneTwister ) final;
    double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) final;

  private:
    std::gamma_distribution<double> mRandomGammaDistribution;
};

class QgsRandomGeometricRasterAlgorithm : public QgsRandomRasterAlgorithmBase
{
  public:
    QgsRandomGeometricRasterAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmRandomExponentialRaster.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRandomExponentialRaster.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRandomGeometricRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() override;
    Qgis::DataType getRasterDataType( int typeId ) override;
    bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) override;
    long generateRandomLongValue( std::mt19937 &mersenneTwister ) override;
    double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) override;

  private:
    std::geometric_distribution<long> mRandomGeometricDistribution;
};


class QgsRandomNegativeBinomialRasterAlgorithm : public QgsRandomRasterAlgorithmBase
{
  public:
    QgsRandomNegativeBinomialRasterAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmNormalRaster.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmNormalRaster.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRandomNegativeBinomialRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() final;
    Qgis::DataType getRasterDataType( int typeId ) final;
    bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) final;
    long generateRandomLongValue( std::mt19937 &mersenneTwister ) final;
    double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) final;

  private:
    std::negative_binomial_distribution<long> mRandomNegativeBinomialDistribution;
};

class QgsRandomNormalRasterAlgorithm : public QgsRandomRasterAlgorithmBase
{
  public:
    QgsRandomNormalRasterAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmNormalRaster.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmNormalRaster.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRandomNormalRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() final;
    Qgis::DataType getRasterDataType( int typeId ) final;
    bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) final;
    long generateRandomLongValue( std::mt19937 &mersenneTwister ) final;
    double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) final;

  private:
    std::normal_distribution<double> mRandomNormalDistribution;
};

class QgsRandomPoissonRasterAlgorithm : public QgsRandomRasterAlgorithmBase
{
  public:
    QgsRandomPoissonRasterAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmRandomPoissonRaster.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRandomPoissonRaster.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsRandomPoissonRasterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void addAlgorithmParams() final;
    Qgis::DataType getRasterDataType( int typeId ) final;
    bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) final;
    long generateRandomLongValue( std::mt19937 &mersenneTwister ) final;
    double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) final;

  private:
    std::poisson_distribution<long> mRandomPoissonDistribution;
};


///@endcond PRIVATE

#endif // QGSRANDOMRASTERALGORITHM_H
