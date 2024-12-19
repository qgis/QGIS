/***************************************************************************
                         qgsalgorithmgpsbabeltools.h
                         ------------------
    begin                : July 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#ifndef QGSALGORITHMGPSBABELTOOLS_H
#define QGSALGORITHMGPSBABELTOOLS_H

#define SIP_NO_FILE

#include <QtGlobal>
#if QT_CONFIG(process)


#include "qgis_sip.h"
#include "qgis_analysis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"
#include "qgsvectorlayerfeatureiterator.h"

///@cond PRIVATE

/**
 * Convert GPX feature type algorithm
 */
class ANALYSIS_EXPORT QgsConvertGpxFeatureTypeAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsConvertGpxFeatureTypeAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsConvertGpxFeatureTypeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    enum ConversionType
    {
      WaypointsFromRoute = 0,
      WaypointsFromTrack,
      RouteFromWaypoints,
      TrackFromWaypoints
    };

    /**
     * Builds the argument lists for the babel command
     */
    static void createArgumentLists(
      const QString &inputFile,
      const QString &outputFile,
      ConversionType conversion,
      QStringList &processArgs,
      QStringList &logArgs
    );

    friend class TestQgsProcessingAlgsPt2;

};


/**
 * Convert GPS data to GPX algorithm
 */
class ANALYSIS_EXPORT QgsConvertGpsDataAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsConvertGpsDataAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsConvertGpsDataAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    friend class TestQgsProcessingAlgs;

};


/**
 * Download GPS data algorithm
 */
class ANALYSIS_EXPORT QgsDownloadGpsDataAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsDownloadGpsDataAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsDownloadGpsDataAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    friend class TestQgsProcessingAlgs;

};


/**
 * Upload GPS data algorithm
 */
class ANALYSIS_EXPORT QgsUploadGpsDataAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsUploadGpsDataAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsUploadGpsDataAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    friend class TestQgsProcessingAlgs;

};

///@endcond PRIVATE

#endif // process
#endif // QGSALGORITHMGPSBABELTOOLS_H


