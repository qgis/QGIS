/***************************************************************************
                         qgsprofileexporter.h
                         ---------------
    begin                : May 2023
    copyright            : (C) 2023 by Nyall Dawson
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
#ifndef QGSPROFILEEXPORTER_H
#define QGSPROFILEEXPORTER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsabstractprofilegenerator.h"
#include "qgsprofilerequest.h"
#include "qgstaskmanager.h"

class QgsAbstractProfileSource;
class QgsAbstractProfileGenerator;

/**
 * \brief Handles exports of elevation profiles in various formats.
 *
 * \ingroup core
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsProfileExporter
{

  public:

    /**
     * Constructor for QgsProfileExporter, using the provided list of profile \a sources to generate the
     * results.
     *
     * After construction, call run() to initiate the profile generation.
     */
    QgsProfileExporter( const QList< QgsAbstractProfileSource * > &sources,
                        const QgsProfileRequest &request,
                        Qgis::ProfileExportType type );

    QgsProfileExporter( const QgsProfileExporter &other ) = delete;
    QgsProfileExporter &operator=( const QgsProfileExporter &other ) = delete;

    ~QgsProfileExporter();

    /**
     * Runs the profile generation. This method must be called before retrieving any results from the
     * exporter.
     *
     * This method is safe to run in a background thread.
     */
    void run( QgsFeedback *feedback = nullptr );

    /**
     * Returns a list of vector layer containing the exported profile results.
     *
     * While this method attempts to condense all results into a single layer, multiple layers may be returned
     * when the geometry types of exported features differs.
     *
     * Ownership of the returned layers is transferred to the caller.
     */
    QList< QgsVectorLayer * > toLayers() SIP_FACTORY;

  private:

#ifdef SIP_RUN
    QgsProfileExporter( const QgsProfileExporter &other );
#endif

    Qgis::ProfileExportType mType = Qgis::ProfileExportType::Features3D;
    QgsProfileRequest mRequest;
    std::vector< std::unique_ptr< QgsAbstractProfileGenerator > > mGenerators;
    QVector< QgsAbstractProfileResults::Feature > mFeatures;

};

/**
 * \brief Handles exports of elevation profiles in various formats in a background task.
 *
 * \ingroup core
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsProfileExporterTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Results of exporting the profile.
     */
    enum class ExportResult
    {
      Success, //!< Successful export
      Empty, //!< Results were empty
      DeviceError, //!< Could not open output file device
      DxfExportFailed, //!< Generic error when outputting to DXF
      LayerExportFailed, //!< Generic error when outputting to files
      Canceled, //!< Export was canceled
    };
    Q_ENUM( ExportResult );

    /**
     * Constructor for QgsProfileExporterTask, saving results to the specified \a destination file.
     *
     * If \a destination is an empty string then the profile results will be generated only and can
     * be retrieved by calling takeLayers().
     */
    QgsProfileExporterTask( const QList< QgsAbstractProfileSource * > &sources,
                            const QgsProfileRequest &request,
                            Qgis::ProfileExportType type,
                            const QString &destination,
                            const QgsCoordinateTransformContext &transformContext );

    bool run() override;
    void cancel() override;

    /**
     * Returns a list of vector layer containing the exported profile results.
     *
     * While this method attempts to condense all results into a single layer, multiple layers may be returned
     * when the geometry types of exported features differs.
     *
     * Ownership of the returned layers is transferred to the caller.
     */
    QList< QgsVectorLayer * > takeLayers() SIP_FACTORY;

    /**
     * Returns the result of the export operation.
     *
     * \see error()
     */
    QgsProfileExporterTask::ExportResult result() const;

    /**
     * Returns a list of layer files created during the export.
     */
    QStringList createdFiles() const { return mCreatedFiles; }

    /**
     * Returns a descriptive error message, if available.
     */
    QString error() const { return mError; }

  private:

    std::unique_ptr< QgsProfileExporter > mExporter;
    QList< QgsVectorLayer * > mLayers;

    std::unique_ptr< QgsFeedback > mFeedback;
    QString mDestination;
    QgsCoordinateTransformContext mTransformContext;
    ExportResult mResult = ExportResult::Success;
    QString mError;
    QStringList mCreatedFiles;
};


#endif // QGSPROFILEEXPORTER_H
