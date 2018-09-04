/***************************************************************************
                          qgsvectorfilewritertask.h
                          -------------------------
    begin                : Feb 2017
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

#ifndef QGSVECTORFILEWRITERTASK_H
#define QGSVECTORFILEWRITERTASK_H

#include "qgis_core.h"
#include "qgsvectorfilewriter.h"
#include "qgstaskmanager.h"

/**
 * \class QgsVectorFileWriterTask
 * \ingroup core
 * QgsTask task which performs a QgsVectorFileWriter layer saving operation as a background
 * task. This can be used to save a vector layer out to a file without blocking the
 * QGIS interface.
 * \see QgsVectorLayerExporterTask
 * \see QgsRasterFileWriterTask
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsVectorFileWriterTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsVectorFileWriterTask. Takes a source \a layer, destination \a fileName
     * and save \a options.
     */
    QgsVectorFileWriterTask( QgsVectorLayer *layer,
                             const QString &fileName,
                             const QgsVectorFileWriter::SaveVectorOptions &options );

    void cancel() override;

  signals:

    /**
     * Emitted when writing the layer is successfully completed. The \a newFilename
     * parameter indicates the file path for the written file.
     * \note this signal is deprecated in favor of completed().
     */
    void writeComplete( const QString &newFilename );

    /**
     * Emitted when writing the layer is successfully completed. The \a newFilename
     * parameter indicates the file path for the written file. When applicable, the
     * \a newLayer parameter indicates the layer name used.
     */
    void completed( const QString &newFilename, const QString &newLayer ) SIP_SKIP;

    /**
     * Emitted when an error occurs which prevented the file being written (or if
     * the task is canceled). The writing \a error and \a errorMessage will be reported.
     */
    void errorOccurred( int error, const QString &errorMessage );

  protected:

    bool run() override;
    void finished( bool result ) override;

  private:

    QString mDestFileName;

    std::unique_ptr< QgsFeedback > mOwnedFeedback;
    QgsVectorFileWriter::WriterError mError = QgsVectorFileWriter::NoError;

    QString mNewFilename;
    QString mNewLayer;
    QString mErrorMessage;

    QgsVectorFileWriter::SaveVectorOptions mOptions;
    QgsVectorFileWriter::PreparedWriterDetails mWriterDetails;
    std::unique_ptr< QgsVectorFileWriter::FieldValueConverter > mFieldValueConverter;
};

#endif
