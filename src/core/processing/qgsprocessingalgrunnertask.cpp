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

QgsProcessingAlgRunnerTask::QgsProcessingAlgRunnerTask( const QgsProcessingAlgorithm *algorithm, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback, Flags flags )
  : QgsTask(
      tr( "Executing “%1”" ).arg( algorithm->displayName() ),
      flags & ( !( algorithm->flags() & QgsProcessingAlgorithm::FlagCanCancel ) ? ( ~QgsTask::CanCancel ) : ( ~QgsTask::Flags() ) )
    )
  , mParameters( parameters )
  , mContext( context )
  , mFeedback( feedback )
{
  if ( !mFeedback )
  {
    mOwnedFeedback.reset( new QgsProcessingFeedback() );
    mFeedback = mOwnedFeedback.get();
  }
  try
  {
    mAlgorithm.reset( algorithm->create() );
    if ( !( mAlgorithm && mAlgorithm->prepare( mParameters, context, mFeedback ) ) )
      cancel();
  }
  catch ( QgsProcessingException &e )
  {
    QgsMessageLog::logMessage( e.what(), QObject::tr( "Processing" ), Qgis::MessageLevel::Critical );
    mFeedback->reportError( e.what() );
    cancel();
  }
}

void QgsProcessingAlgRunnerTask::cancel()
{
  if ( mFeedback )
    mFeedback->cancel();
  QgsTask::cancel();
}

bool QgsProcessingAlgRunnerTask::run()
{
  if ( isCanceled() )
    return false;

  connect( mFeedback, &QgsFeedback::progressChanged, this, &QgsProcessingAlgRunnerTask::setProgress );
  bool ok = false;
  try
  {
    mResults = mAlgorithm->runPrepared( mParameters, mContext, mFeedback );
    ok = true;
  }
  catch ( QgsProcessingException &e )
  {
    QgsMessageLog::logMessage( e.what(), QObject::tr( "Processing" ), Qgis::MessageLevel::Critical );
    mFeedback->reportError( e.what() );
    return false;
  }
  return ok && !mFeedback->isCanceled();
}

void QgsProcessingAlgRunnerTask::finished( bool result )
{
  Q_UNUSED( result )
  QVariantMap ppResults;
  if ( result )
  {
    ppResults = mAlgorithm->postProcess( mContext, mFeedback );
  }
  emit executed( result, !ppResults.isEmpty() ? ppResults : mResults );
}
