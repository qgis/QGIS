/***************************************************************************
                          qgsrasterfilewritertask.cpp
                          ---------------------------
    begin                : Apr 2017
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

#include "qgsrasterfilewritertask.h"
#include "qgsrasterinterface.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterpipe.h"

// Deprecated!
QgsRasterFileWriterTask::QgsRasterFileWriterTask( const QgsRasterFileWriter &writer, QgsRasterPipe *pipe, int columns, int rows,
    const QgsRectangle &outputExtent, const QgsCoordinateReferenceSystem &crs )
  : QgsRasterFileWriterTask( writer, pipe, columns, rows, outputExtent, crs,
                             ( pipe && pipe->provider() ) ? pipe->provider()->transformContext() : QgsCoordinateTransformContext() )
{
}

QgsRasterFileWriterTask::QgsRasterFileWriterTask( const QgsRasterFileWriter &writer, QgsRasterPipe *pipe, int columns, int rows,
    const QgsRectangle &outputExtent,
    const QgsCoordinateReferenceSystem &crs,
    const QgsCoordinateTransformContext &transformContext )
  : QgsTask( tr( "Saving %1" ).arg( writer.outputUrl() ), QgsTask::CanCancel )
  , mWriter( writer )
  , mRows( rows )
  , mColumns( columns )
  , mExtent( outputExtent )
  , mCrs( crs )
  , mPipe( pipe )
  , mFeedback( new QgsRasterBlockFeedback() )
  , mTransformContext( transformContext )
{
  QgsRenderContext renderContext;
  renderContext.setRendererUsage( Qgis::RendererUsage::Export );
  mFeedback->setRenderContext( renderContext );
}

QgsRasterFileWriterTask::~QgsRasterFileWriterTask() = default;

void QgsRasterFileWriterTask::cancel()
{
  mFeedback->cancel();
  QgsTask::cancel();
}

bool QgsRasterFileWriterTask::run()
{
  if ( !mPipe )
    return false;

  connect( mFeedback.get(), &QgsRasterBlockFeedback::progressChanged, this, &QgsRasterFileWriterTask::setProgress );

  mError = mWriter.writeRaster( mPipe.get(), mColumns, mRows, mExtent, mCrs, mTransformContext, mFeedback.get() );

  return mError == QgsRasterFileWriter::NoError;
}

void QgsRasterFileWriterTask::finished( bool result )
{
  if ( result )
    emit writeComplete( mWriter.outputUrl() );
  else
  {
    emit errorOccurred( mError );
    QString errorMsg;
    if ( !mFeedback->errors().isEmpty() )
      errorMsg = mFeedback->errors().front();
    emit errorOccurred( mError, errorMsg );
  }
}


