/***************************************************************************
                              qgsmaprendererjobproxy.cpp
                              --------------------------
    begin                : January 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul dot blottiere at oslandia dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaprendererjobproxy.h"

#include "qgsmessagelog.h"
#include "qgsmaprendererparalleljob.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsapplication.h"
#include <QThread>

namespace QgsWms
{

  QgsMapRendererJobProxy::QgsMapRendererJobProxy(
    bool parallelRendering, int maxThreads, QgsFeatureFilterProvider *featureFilterProvider
  )
    : mParallelRendering( parallelRendering )
    , mFeatureFilterProvider( featureFilterProvider )
  {
#ifndef HAVE_SERVER_PYTHON_PLUGINS
    Q_UNUSED( mFeatureFilterProvider )
#endif
    if ( mParallelRendering )
    {
      QgsApplication::setMaxThreads( maxThreads );
      QgsMessageLog::logMessage( QStringLiteral( "Parallel rendering activated with %1 threads" ).arg( maxThreads ), QStringLiteral( "server" ), Qgis::MessageLevel::Info );
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "Parallel rendering deactivated" ), QStringLiteral( "server" ), Qgis::MessageLevel::Info );
    }
  }

  void QgsMapRendererJobProxy::render( const QgsMapSettings &mapSettings, QImage *image, const QgsFeedback *feedback )
  {
    if ( mParallelRendering )
    {
      QgsMapRendererParallelJob renderJob( mapSettings );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      renderJob.setFeatureFilterProvider( mFeatureFilterProvider );
#endif

      // Allows the main thread to manage blocking call coming from rendering
      // threads (see discussion in https://github.com/qgis/QGIS/issues/26819).
      QEventLoop loop;
      QObject::connect( &renderJob, &QgsMapRendererParallelJob::finished, &loop, &QEventLoop::quit );
      if ( feedback )
        QObject::connect( feedback, &QgsFeedback::canceled, &renderJob, &QgsMapRendererParallelJob::cancel );

      if ( !feedback || !feedback->isCanceled() )
        renderJob.start();

      if ( renderJob.isActive() )
      {
        loop.exec();

        renderJob.waitForFinished();
        *image = renderJob.renderedImage();
        mPainter.reset( new QPainter( image ) );
      }

      mErrors = renderJob.errors();

      if ( feedback )
        QObject::disconnect( feedback, &QgsFeedback::canceled, &renderJob, &QgsMapRendererParallelJob::cancel );
    }
    else
    {
      mPainter.reset( new QPainter( image ) );
      QgsMapRendererCustomPainterJob renderJob( mapSettings, mPainter.get() );
      if ( feedback )
        QObject::connect( feedback, &QgsFeedback::canceled, &renderJob, &QgsMapRendererCustomPainterJob::cancel );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      renderJob.setFeatureFilterProvider( mFeatureFilterProvider );
#endif
      if ( !feedback || !feedback->isCanceled() )
        renderJob.renderSynchronously();

      mErrors = renderJob.errors();

      if ( feedback )
        QObject::disconnect( feedback, &QgsFeedback::canceled, &renderJob, &QgsMapRendererCustomPainterJob::cancel );
    }
  }

  QPainter *QgsMapRendererJobProxy::takePainter()
  {
    return mPainter.release();
  }
} // namespace QgsWms
