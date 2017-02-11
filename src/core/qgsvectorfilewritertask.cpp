/***************************************************************************
                          qgsvectorfilewritertask.cpp
                          ---------------------------
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

#include "qgsvectorfilewritertask.h"


QgsVectorFileWriterTask::QgsVectorFileWriterTask( QgsVectorLayer* layer, const QString& fileName, const QgsVectorFileWriter::SaveVectorOptions& options )
    : QgsTask( tr( "Saving %1 " ).arg( fileName ), QgsTask::CanCancel )
    , mLayer( layer )
    , mDestFileName( fileName )
    , mOptions( options )
{
  if ( !mOptions.feedback )
  {
    mOwnedFeedback.reset( new QgsFeedback() );
    mOptions.feedback = mOwnedFeedback.get();
  }
}

void QgsVectorFileWriterTask::cancel()
{
  mOptions.feedback->cancel();
  QgsTask::cancel();
}

bool QgsVectorFileWriterTask::run()
{
  if ( !mLayer )
    return false;


  mError = QgsVectorFileWriter::writeAsVectorFormat(
             mLayer, mDestFileName, mOptions, &mNewFilename, &mErrorMessage );
  return mError == QgsVectorFileWriter::NoError;
}

void QgsVectorFileWriterTask::finished( bool result )
{
  if ( result )
    emit writeComplete( mNewFilename );
  else
    emit errorOccurred( mError, mErrorMessage );
}
