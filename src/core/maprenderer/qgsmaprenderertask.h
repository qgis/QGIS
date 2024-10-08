/***************************************************************************
                          qgsmaprenderertask.h
                          -------------------------
    begin                : Apr 2017
    copyright            : (C) 2017 by Mathieu Pellerin
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

#ifndef QGSMAPRENDERERTASK_H
#define QGSMAPRENDERERTASK_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsannotation.h"
#include "qgsannotationmanager.h"
#include "qgsmapsettings.h"
#include "qgsmapdecoration.h"
#include "qgstaskmanager.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsabstractgeopdfexporter.h"

#include <QPainter>
#include <QPdfWriter>

class QgsMapRendererCustomPainterJob;
class QgsAbstractGeospatialPdfExporter;

/**
 * \class QgsMapRendererTask
 * \ingroup core
 * \brief QgsTask task which draws a map to an image file or a painter as a background
 * task. This can be used to draw maps without blocking the QGIS interface.
 */
class CORE_EXPORT QgsMapRendererTask : public QgsTask
{
    Q_OBJECT

  public:

    //! \brief Error type
    enum ErrorType
    {
      ImageAllocationFail = 1, //!< Image allocation failure
      ImageSaveFail, //!< Image save failure
      ImageUnsupportedFormat //!< Format is unsupported on the platform \since QGIS 3.4
    };

#ifndef SIP_RUN

    /**
     * Constructor for QgsMapRendererTask to render a map to an image file.
     *
     * If the output \a fileFormat is set to PDF, the \a geospatialPdf argument controls whether a geospatial PDF file is created.
     * See QgsAbstractGeospatialPdfExporter::geospatialPDFCreationAvailable() for conditions on geospatial PDF creation availability.
     *
     * Since QGIS 3.26 the optional \a flags argument can be used to control the task flags.
     */
    QgsMapRendererTask( const QgsMapSettings &ms,
                        const QString &fileName,
                        const QString &fileFormat = QString( "PNG" ),
                        bool forceRaster = false,
                        QgsTask::Flags flags = QgsTask::CanCancel, bool geospatialPdf = false, const QgsAbstractGeospatialPdfExporter::ExportDetails &geospatialPdfExportDetails = QgsAbstractGeospatialPdfExporter::ExportDetails()
                      );
#else

    /**
     * Constructor for QgsMapRendererTask to render a map to an image file.
     *
     * Since QGIS 3.26 the optional \a flags argument can be used to control the task flags.
     */
    QgsMapRendererTask( const QgsMapSettings &ms,
                        const QString &fileName,
                        const QString &fileFormat = QString( "PNG" ),
                        bool forceRaster = false,
                        QgsTask::Flags flags = QgsTask::CanCancel );
#endif

    /**
     * Constructor for QgsMapRendererTask to render a map to a QPainter object.
     */
    QgsMapRendererTask( const QgsMapSettings &ms,
                        QPainter *p );

    ~QgsMapRendererTask() override;

    /**
     * Adds \a annotations to be rendered on the map.
     */
    void addAnnotations( const QList<QgsAnnotation *> &annotations );

    /**
     * Adds \a decorations to be rendered on the map.
     */
    void addDecorations( const QList<QgsMapDecoration *> &decorations );

    /**
     * Sets whether the image file will be georeferenced (embedded or via a world file).
     */
    void setSaveWorldFile( bool save ) { mSaveWorldFile = save; }

    /**
     * Sets whether metadata such as title and subject will be exported whenever possible.
     */
    void setExportMetadata( bool exportMetadata ) { mExportMetadata = exportMetadata; }

    void cancel() override;

  signals:

    /**
     * Emitted when the map rendering is successfully completed.
     */
    void renderingComplete();

    /**
     * Emitted when map rendering failed.
     */
    void errorOccurred( int error );

  protected:

    bool run() override;
    void finished( bool result ) override;

  private:

    //! Prepares the job, doing the work which HAS to be done on the main thread in advance
    void prepare();
    bool mErrored = false;

    QgsMapSettings mMapSettings;

    QMutex mJobMutex;
    std::unique_ptr< QgsMapRendererJob > mJob;

    std::unique_ptr< QgsAbstractGeospatialPdfExporter > mGeospatialPdfExporter;
    std::unique_ptr< QgsRenderedFeatureHandlerInterface > mRenderedFeatureHandler;

    QPainter *mPainter = nullptr;
    QPainter *mDestPainter = nullptr;
    QImage mImage;
    std::unique_ptr< QPdfWriter > mPdfWriter;

    std::unique_ptr< QPainter > mTempPainter;

    QString mFileName;
    QString mFileFormat;
    bool mForceRaster = false;
    bool mSaveWorldFile = false;
    bool mExportMetadata = false;
    bool mGeospatialPDF = false;
    QgsAbstractGeospatialPdfExporter::ExportDetails mGeospatialPdfExportDetails;

    QList< QgsAnnotation * > mAnnotations;
    QList< QgsMapDecoration * > mDecorations;
    QMap< QString, QString> mLayerIdToLayerNameMap;
    QStringList mMapLayerOrder;

    int mError = 0;


};

// clazy:excludeall=qstring-allocations

#endif
