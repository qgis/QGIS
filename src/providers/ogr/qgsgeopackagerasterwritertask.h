/***************************************************************************
  qgsgeopackagerasterwritertask.h - QgsGeoPackageRasterWriterTask

 ---------------------
 begin                : 23.8.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGEOPACKAGERASTERWRITERTASK_H
#define QGSGEOPACKAGERASTERWRITERTASK_H


///@cond PRIVATE

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsgeopackagerasterwriter.h"
#include "qgstaskmanager.h"
#include "qgsfeedback.h"


/**
 * \class QgsGeoPackageRasterWriterTask
 * QgsTask task which performs a QgsGeoPackageRasterWriter layer saving operation as a background
 * task. This can be used to save a raster layer out to a file without blocking the
 * QGIS interface.
 * \since QGIS 3.0
 * \see QgsGeoPackageRasterWriterTask
 */
class QgsGeoPackageRasterWriterTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsVectorFileWriterTask. Takes a source \a layer, destination \a fileName
     * and save \a options.
     */
    QgsGeoPackageRasterWriterTask( const QgsMimeDataUtils::Uri &sourceUri, const QString &destinationPath );

    virtual void cancel() override;

  signals:

    /**
     * Emitted when writing the layer is successfully completed. The \a newFilename
     * parameter indicates the file path for the written file.
     */
    void writeComplete( const QString &newFilename );

    /**
     * Emitted when an error occurs which prevented the file being written (or if
     * the task is canceled). The writing \a error and \a errorMessage will be reported.
     */
    void errorOccurred( QgsGeoPackageRasterWriter::WriterError error, const QString &errorMessage );

  protected:

    virtual bool run() override;
    virtual void finished( bool result ) override;

  private:

    QgsGeoPackageRasterWriter mWriter;
    std::unique_ptr< QgsFeedback > mFeedback;
    QgsGeoPackageRasterWriter::WriterError mError = QgsGeoPackageRasterWriter::WriterError::NoError;
    QString mErrorMessage;

};


///@endcond

#endif // QGSGEOPACKAGERASTERWRITERTASK_H
