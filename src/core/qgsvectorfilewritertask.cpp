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
#include "moc_qgsvectorfilewritertask.cpp"
#include "qgsvectorlayer.h"

QgsVectorFileWriterTask::QgsVectorFileWriterTask( QgsVectorLayer *layer, const QString &fileName, const QgsVectorFileWriter::SaveVectorOptions &options, QgsFeatureSink::SinkFlags sinkFlags )
  : QgsTask( tr( "Saving %1" ).arg( fileName ), QgsTask::CanCancel )
  , mDestFileName( fileName )
  , mSinkFlags( sinkFlags )
  , mOptions( options )
{
  if ( mOptions.fieldValueConverter )
  {
    // fieldValueConverter is not owned - so we need to clone it here
    // to ensure it exists for lifetime of task
    mFieldValueConverter.reset( mOptions.fieldValueConverter->clone() );
    mOptions.fieldValueConverter = mFieldValueConverter.get();
  }
  if ( !mOptions.feedback )
  {
    mOwnedFeedback.reset( new QgsFeedback() );
    mOptions.feedback = mOwnedFeedback.get();
  }

  if ( layer )
  {
    mTransformContext = layer->transformContext();
  }

  mError = QgsVectorFileWriter::prepareWriteAsVectorFormat( layer, mOptions, mWriterDetails );
}

void QgsVectorFileWriterTask::cancel()
{
  mOptions.feedback->cancel();
  QgsTask::cancel();
}

bool QgsVectorFileWriterTask::run()
{
  if ( mError != QgsVectorFileWriter::NoError )
    return false;

  connect( mOptions.feedback, &QgsFeedback::progressChanged, this, &QgsVectorFileWriterTask::setProgress );


  mError = QgsVectorFileWriter::writeAsVectorFormatV2(
             mWriterDetails, mDestFileName, mTransformContext, mOptions, &mNewFilename, &mNewLayer, &mErrorMessage, mSinkFlags );
  return mError == QgsVectorFileWriter::NoError;
}

void QgsVectorFileWriterTask::finished( bool result )
{
  if ( result )
  {
    emit writeComplete( mNewFilename );
    emit completed( mNewFilename, mNewLayer );
  }
  else
  {
    emit errorOccurred( mError, mErrorMessage );
  }
}
