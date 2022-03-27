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
#ifndef QT_NO_PRINTER
#include <QPrinter>
#endif

class QgsMapRendererCustomPainterJob;
class QgsAbstractGeoPdfExporter;

/**
 * \class QgsMapRendererTask
 * \ingroup core
 * \brief QgsTask task which draws a map to an image file or a painter as a background
 * task. This can be used to draw maps without blocking the QGIS interface.
 * \since QGIS 3.0
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
     * If the output \a fileFormat is set to PDF, the \a geoPdf argument controls whether a GeoPDF file is created.
     * See QgsAbstractGeoPdfExporter::geoPDFCreationAvailable() for conditions on GeoPDF creation availability.
     *
     * Since QGIS 3.26 the optional \a flags argument can be used to control the task flags.
     */
    QgsMapRendererTask( const QgsMapSettings &ms,
                        const QString &fileName,
                        const QString &fileFormat = QString( "PNG" ),
                        bool forceRaster = false,
                        QgsTask::Flags flags = QgsTask::CanCancel, bool geoPdf = false, const QgsAbstractGeoPdfExporter::ExportDetails &geoPdfExportDetails = QgsAbstractGeoPdfExporter::ExportDetails()
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

    std::unique_ptr< QgsAbstractGeoPdfExporter > mGeoPdfExporter;
    std::unique_ptr< QgsRenderedFeatureHandlerInterface > mRenderedFeatureHandler;

    QPainter *mPainter = nullptr;
    QPainter *mDestPainter = nullptr;
    QImage mImage;
#ifndef QT_NO_PRINTER
    std::unique_ptr< QPrinter > mPrinter;
#endif // ! QT_NO_PRINTER

    std::unique_ptr< QPainter > mTempPainter;

    QString mFileName;
    QString mFileFormat;
    bool mForceRaster = false;
    bool mSaveWorldFile = false;
    bool mExportMetadata = false;
    bool mGeoPDF = false;
    QgsAbstractGeoPdfExporter::ExportDetails mGeoPdfExportDetails;

    QList< QgsAnnotation * > mAnnotations;
    QList< QgsMapDecoration * > mDecorations;
    QMap< QString, QString> mLayerIdToLayerNameMap;
    QStringList mMapLayerOrder;

    int mError = 0;


};

// clazy:excludeall=qstring-allocations

#endif
