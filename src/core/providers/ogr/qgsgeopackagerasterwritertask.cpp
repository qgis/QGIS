/***************************************************************************
  qgsgeopackagerasterwritertask.cpp - QgsGeoPackageRasterWriterTask

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
#include "qgsgeopackagerasterwritertask.h"
#include "moc_qgsgeopackagerasterwritertask.cpp"


///@cond PRIVATE


QgsGeoPackageRasterWriterTask::QgsGeoPackageRasterWriterTask( const QgsMimeDataUtils::Uri &sourceUri, const QString &destinationPath )
  : QgsTask( tr( "Saving %1" ).arg( destinationPath ), QgsTask::CanCancel )
  , mWriter( sourceUri, destinationPath )
  , mFeedback( new QgsFeedback() )
{

}

void QgsGeoPackageRasterWriterTask::cancel()
{
  mError = QgsGeoPackageRasterWriter::WriterError::ErrUserCanceled;
  mFeedback->cancel();
}

bool QgsGeoPackageRasterWriterTask::run()
{
  connect( mFeedback.get(), &QgsFeedback::progressChanged, this, &QgsGeoPackageRasterWriterTask::setProgress );
  mError = mWriter.writeRaster( mFeedback.get(), &mErrorMessage );
  return mError == QgsGeoPackageRasterWriter::WriterError::NoError;
}

void QgsGeoPackageRasterWriterTask::finished( bool result )
{
  if ( result )
  {
    emit writeComplete( mWriter.outputUrl() );
  }
  else
  {
    emit errorOccurred( mError, mErrorMessage );
  }
}

///@endcond
