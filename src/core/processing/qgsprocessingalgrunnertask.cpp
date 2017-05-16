/***************************************************************************
                         qgsprocessingalgrunnertask.cpp
                         ------------------------------
    begin                : May 2017
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

#include "qgsprocessingalgrunnertask.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingutils.h"
#include "qgsvectorlayer.h"

QgsProcessingAlgRunnerTask::QgsProcessingAlgRunnerTask( const QgsProcessingAlgorithm *algorithm, const QVariantMap &parameters, QgsProcessingContext &context )
  : QgsTask( tr( "Running %1" ).arg( algorithm->name() ), QgsTask::CanCancel )
  , mAlgorithm( algorithm )
  , mParameters( parameters )
  , mContext( context )
{
  mFeedback.reset( new QgsProcessingFeedback() );
}

void QgsProcessingAlgRunnerTask::cancel()
{
  mFeedback->cancel();
}

bool QgsProcessingAlgRunnerTask::run()
{
  connect( mFeedback.get(), &QgsFeedback::progressChanged, this, &QgsProcessingAlgRunnerTask::setProgress );
  mResults = mAlgorithm->run( mParameters, mContext, mFeedback.get() );
  return !mFeedback->isCanceled();
}

void QgsProcessingAlgRunnerTask::finished( bool result )
{
  Q_UNUSED( result );
  if ( !mResults.isEmpty() )
  {
    QgsMapLayer *layer = QgsProcessingUtils::mapLayerFromString( mResults.value( "OUTPUT_LAYER" ).toString(), mContext );
    if ( layer )
    {
      mContext.project()->addMapLayer( mContext.temporaryLayerStore()->takeMapLayer( layer ) );
    }
  }
}
