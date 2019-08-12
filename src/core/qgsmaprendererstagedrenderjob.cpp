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
#include "qgsmaplayerlistutils.h"

QgsMapRendererStagedRenderJob::QgsMapRendererStagedRenderJob( const QgsMapSettings &settings )
  : QgsMapRendererAbstractCustomPainterJob( settings )
{
}

QgsMapRendererStagedRenderJob::~QgsMapRendererStagedRenderJob()
{
  // final cleanup
  cleanupJobs( mLayerJobs );
  cleanupLabelJob( mLabelJob );
}


void QgsMapRendererStagedRenderJob::start()
{
  mRenderingStart.start();
  mErrors.clear();

  QgsDebugMsgLevel( QStringLiteral( "Preparing list of layer jobs for rendering" ), 5 );
  QTime prepareTime;
  prepareTime.start();

  mLabelingEngineV2.reset();

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) )
  {
    mLabelingEngineV2.reset( new QgsLabelingEngine() );
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
    job.context.setPainter( painter );

    if ( job.context.useAdvancedEffects() )
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

    job.renderer->render();

    if ( job.img )
    {
      // If we flattened this layer for alternate blend modes, composite it now
      painter->setOpacity( job.opacity );
      painter->drawImage( 0, 0, *job.img );
      painter->setOpacity( 1.0 );
    }
  }
  else
  {
    mLabelJob.context.setPainter( painter );
    drawLabeling( mLabelJob.context, mLabelingEngineV2.get(), painter );
    mLabelJob.complete = true;
    mLabelJob.participatingLayers = _qgis_listRawToQPointer( mLabelingEngineV2->participatingLayers() );
  }
  return true;
}

bool QgsMapRendererStagedRenderJob::nextPart()
{
  if ( isFinished() )
    return false;

  if ( mJobIt != mLayerJobs.end() )
  {
    mJobIt++;
    if ( mJobIt != mLayerJobs.end() )
      return true;
  }

  if ( mLabelingEngineV2 )
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
  return false;
}

bool QgsMapRendererStagedRenderJob::isFinished()
{
  return mJobIt == mLayerJobs.end() && ( mExportedLabels || !mLabelingEngineV2 );
}

const QgsMapLayer *QgsMapRendererStagedRenderJob::currentLayer()
{
  if ( mJobIt != mLayerJobs.end() )
  {
    LayerRenderJob &job = *mJobIt;
    return job.layer;
  }
  return nullptr;
}

QgsMapRendererStagedRenderJob::RenderStage QgsMapRendererStagedRenderJob::currentStage() const
{
  if ( mJobIt != mLayerJobs.end() )
    return Symbology;
  else if ( mNextIsLabel && !mExportedLabels )
    return Labels;
  else
    return Finished;
}
