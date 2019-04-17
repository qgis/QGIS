/***************************************************************************
                          qgsrasterfilewritertask.h
                          -------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSRASTERFILEWRITERTASK_H
#define QGSRASTERFILEWRITERTASK_H

#include "qgis_core.h"
#include "qgstaskmanager.h"
#include "qgsrasterfilewriter.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsrasterinterface.h"
#include "qgsrasterpipe.h"

/**
 * \class QgsRasterFileWriterTask
 * \ingroup core
 * QgsTask task which performs a QgsRasterFileWriter layer saving operation as a background
 * task. This can be used to save a raster layer out to a file without blocking the
 * QGIS interface.
 * \see QgsVectorFileWriterTask
 * \see QgsVectorFileExporterTask
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsRasterFileWriterTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRasterFileWriterTask. Takes a source \a writer,
     * \a columns, \a rows, \a outputExtent and destination \a crs.
     * Ownership of the \a pipe is transferred to the writer task, and will
     * be deleted when the task completes.
     * \deprecated since QGIS 3.8, use version with transformContext instead
     */
    Q_DECL_DEPRECATED QgsRasterFileWriterTask( const QgsRasterFileWriter &writer, QgsRasterPipe *pipe SIP_TRANSFER,
        int columns, int rows,
        const QgsRectangle &outputExtent,
        const QgsCoordinateReferenceSystem &crs ) SIP_DEPRECATED;


    /**
     * Constructor for QgsRasterFileWriterTask. Takes a source \a writer,
     * \a columns, \a rows, \a outputExtent, destination \a crs and
     * coordinate \a transformContext .
     * Ownership of the \a pipe is transferred to the writer task, and will
     * be deleted when the task completes.
     */
    QgsRasterFileWriterTask( const QgsRasterFileWriter &writer, QgsRasterPipe *pipe SIP_TRANSFER,
                             int columns, int rows,
                             const QgsRectangle &outputExtent,
                             const QgsCoordinateReferenceSystem &crs,
                             const QgsCoordinateTransformContext &transformContext
                           );

    void cancel() override;

  signals:

    /**
     * Emitted when writing the layer is successfully completed. The \a outputUrl
     * parameter indicates the file path for the written file(s).
     */
    void writeComplete( const QString &outputUrl );

    /**
     * Emitted when an error occurs which prevented the file being written (or if
     * the task is canceled). The writing \a error will be reported.
     */
    void errorOccurred( int error );

  protected:

    bool run() override;
    void finished( bool result ) override;

  private:

    QgsRasterFileWriter mWriter;
    int mRows = 0;
    int mColumns = 0;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    std::unique_ptr< QgsRasterPipe > mPipe;

    QString mDestFileName;

    std::unique_ptr< QgsRasterBlockFeedback > mFeedback;

    QgsRasterFileWriter::WriterError mError = QgsRasterFileWriter::NoError;

    QgsCoordinateTransformContext mTransformContext;

};

#endif //QGSRASTERFILEWRITERTASK_H
