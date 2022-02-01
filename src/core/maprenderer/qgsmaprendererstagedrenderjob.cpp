/***************************************************************************
  qgsmaprendererstagedrenderjob.cpp
  --------------------------------------
  Date                 : August 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaprendererstagedrenderjob.h"

#include "qgsfeedback.h"
#include "qgslabelingengine.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsmaplayerrenderer.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsrendereditemresults.h"

QgsMapRendererStagedRenderJob::QgsMapRendererStagedRenderJob( const QgsMapSettings &settings, Flags flags )
  : QgsMapRendererAbstractCustomPainterJob( settings )
  , mFlags( flags )
{
}

QgsMapRendererStagedRenderJob::~QgsMapRendererStagedRenderJob()
{
  // final cleanup
  cleanupJobs( mLayerJobs );
  cleanupLabelJob( mLabelJob );
}


void QgsMapRendererStagedRenderJob::startPrivate()
{
  mRenderingStart.start();
  mErrors.clear();

  QgsDebugMsgLevel( QStringLiteral( "Preparing list of layer jobs for rendering" ), 5 );
  QElapsedTimer prepareTime;
  prepareTime.start();

  mLabelingEngineV2.reset();

  if ( mSettings.testFlag( Qgis::MapSettingsFlag::DrawLabeling ) )
  {
    if ( mFlags & RenderLabelsByMapLayer )
      mLabelingEngineV2.reset( new QgsStagedRenderLabelingEngine() );
    else
      mLabelingEngineV2.reset( new QgsDefaultLabelingEngine() );
    mLabelingEngineV2->setMapSettings( mSettings );
  }

  mLayerJobs = prepareJobs( nullptr, mLabelingEngineV2.get(), true );
  mLabelJob = prepareLabelingJob( nullptr, mLabelingEngineV2.get(), false );

  mJobIt = mLayerJobs.begin();
}

void QgsMapRendererStagedRenderJob::cancel()
{
}

void QgsMapRendererStagedRenderJob::cancelWithoutBlocking()
{
}

void QgsMapRendererStagedRenderJob::waitForFinished()
{
}

bool QgsMapRendererStagedRenderJob::isActive() const
{
  return true;
}

bool QgsMapRendererStagedRenderJob::usedCachedLabels() const
{
  return false;
}

QgsLabelingResults *QgsMapRendererStagedRenderJob::takeLabelingResults()
{
  if ( mLabelingEngineV2 )
    return mLabelingEngineV2->takeResults();
  else
    return nullptr;
}

bool QgsMapRendererStagedRenderJob::renderCurrentPart( QPainter *painter )
{
  if ( isFinished() )
    return false;

  preparePainter( painter );

  if ( mJobIt != mLayerJobs.end() )
  {
    LayerRenderJob &job = *mJobIt;
    emit layerRenderingStarted( job.layerId );
    job.renderer->renderContext()->setPainter( painter );

    if ( job.context()->useAdvancedEffects() )
    {
      // Set the QPainter composition mode so that this layer is rendered using
      // the desired blending mode
      painter->setCompositionMode( job.blendMode );
    }

    if ( job.img )
    {
      job.img->fill( 0 );
      job.imageInitialized = true;
    }

    job.completed = job.renderer->render();

    if ( job.img )
    {
      // If we flattened this layer for alternate blend modes, composite it now
      painter->setOpacity( job.opacity );
      painter->drawImage( 0, 0, *job.img );
      painter->setOpacity( 1.0 );
    }
    job.context()->setPainter( nullptr );

    emit layerRendered( job.layerId );
  }
  else
  {
    if ( !mLabelingEngineV2 )
      return false;

    if ( mFlags & RenderLabelsByMapLayer )
    {
      if ( !mPreparedStagedLabelJob || mLabelLayerIt == mLabelingLayers.end() )
        return false;

      mLabelJob.context.setPainter( painter );

      // Reset the composition mode before rendering the labels
      painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

      // render just the current layer's labels
      static_cast< QgsStagedRenderLabelingEngine * >( mLabelingEngineV2.get() )->renderLabelsForLayer( mLabelJob.context, *mLabelLayerIt );

      mLabelJob.context.setPainter( nullptr );
    }
    else
    {
      mLabelJob.context.setPainter( painter );
      drawLabeling( mLabelJob.context, mLabelingEngineV2.get(), painter );
      mLabelJob.complete = true;
      mLabelJob.participatingLayers = _qgis_listRawToQPointer( mLabelingEngineV2->participatingLayers() );
      mLabelJob.context.setPainter( nullptr );
    }
  }
  return true;
}

bool QgsMapRendererStagedRenderJob::nextPart()
{
  if ( isFinished() )
    return false;

  if ( mJobIt != mLayerJobs.end() )
  {
    ++mJobIt;
    if ( mJobIt != mLayerJobs.end() )
      return true;
  }

  if ( mLabelingEngineV2 )
  {
    if ( mFlags & RenderLabelsByMapLayer )
    {
      if ( !mPreparedStagedLabelJob )
      {
        mLabelingEngineV2->run( mLabelJob.context );
        mPreparedStagedLabelJob = true;
        mLabelingLayers = mLabelingEngineV2->participatingLayerIds();
        mLabelLayerIt = mLabelingLayers.begin();
        if ( mLabelLayerIt == mLabelingLayers.end() )
        {
          // no label layers to render!
          static_cast< QgsStagedRenderLabelingEngine * >( mLabelingEngineV2.get() )->finalize();
          return false;
        }
        return true;
      }
      else
      {
        if ( mLabelLayerIt != mLabelingLayers.end() )
        {
          ++mLabelLayerIt;
          if ( mLabelLayerIt != mLabelingLayers.end() )
            return true;
        }
      }
      return false;
    }
    else
    {
      if ( mNextIsLabel )
      {
        mExportedLabels = true;
      }
      else if ( !mExportedLabels )
      {
        mNextIsLabel = true;
        return true;
      }
    }
  }
  return false;
}

bool QgsMapRendererStagedRenderJob::isFinished() const
{
  return currentStage() == Finished;
}

QString QgsMapRendererStagedRenderJob::currentLayerId() const
{
  if ( mJobIt != mLayerJobs.end() )
  {
    const LayerRenderJob &job = *mJobIt;
    return job.layerId;
  }
  else if ( mFlags & RenderLabelsByMapLayer && mPreparedStagedLabelJob )
  {
    if ( mLabelLayerIt != mLabelingLayers.end() )
      return *mLabelLayerIt;
  }
  return QString();
}

double QgsMapRendererStagedRenderJob::currentLayerOpacity() const
{
  if ( mJobIt != mLayerJobs.end() )
  {
    const LayerRenderJob &job = *mJobIt;
    return job.opacity;
  }
  return 1.0;
}

QPainter::CompositionMode QgsMapRendererStagedRenderJob::currentLayerCompositionMode() const
{
  if ( mJobIt != mLayerJobs.end() )
  {
    const LayerRenderJob &job = *mJobIt;
    return job.blendMode;
  }
  return QPainter::CompositionMode_SourceOver;
}

QgsMapRendererStagedRenderJob::RenderStage QgsMapRendererStagedRenderJob::currentStage() const
{
  if ( mJobIt != mLayerJobs.end() )
    return Symbology;
  else if ( mLabelingEngineV2 && mFlags & RenderLabelsByMapLayer )
  {
    if ( !mPreparedStagedLabelJob )
      return Labels;
    if ( mLabelLayerIt != mLabelingLayers.end() )
      return Labels;
  }
  else if ( mNextIsLabel && !mExportedLabels )
    return Labels;

  return Finished;
}
