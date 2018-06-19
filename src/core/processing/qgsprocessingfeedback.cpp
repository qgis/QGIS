/***************************************************************************
                         qgsprocessingfeedback.cpp
                         -------------------------
    begin                : June 2017
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

#include "qgsprocessingfeedback.h"

QgsProcessingMultiStepFeedback::QgsProcessingMultiStepFeedback( int childAlgorithmCount, QgsProcessingFeedback *feedback )
  : mChildSteps( childAlgorithmCount )
  , mFeedback( feedback )
{
  connect( mFeedback, &QgsFeedback::canceled, this, &QgsFeedback::cancel, Qt::DirectConnection );
  connect( this, &QgsFeedback::progressChanged, this, &QgsProcessingMultiStepFeedback::updateOverallProgress );
}

void QgsProcessingMultiStepFeedback::setCurrentStep( int step )
{
  mCurrentStep = step;
  mFeedback->setProgress( 100.0 * static_cast< double >( mCurrentStep ) / mChildSteps );
}

void QgsProcessingMultiStepFeedback::setProgressText( const QString &text )
{
  mFeedback->setProgressText( text );
}

void QgsProcessingMultiStepFeedback::reportError( const QString &error, bool fatalError )
{
  mFeedback->reportError( error, fatalError );
}

void QgsProcessingMultiStepFeedback::pushInfo( const QString &info )
{
  mFeedback->pushInfo( info );
}

void QgsProcessingMultiStepFeedback::pushCommandInfo( const QString &info )
{
  mFeedback->pushCommandInfo( info );
}

void QgsProcessingMultiStepFeedback::pushDebugInfo( const QString &info )
{
  mFeedback->pushDebugInfo( info );
}

void QgsProcessingMultiStepFeedback::pushConsoleInfo( const QString &info )
{
  mFeedback->pushConsoleInfo( info );
}

void QgsProcessingMultiStepFeedback::updateOverallProgress( double progress )
{
  double baseProgress = 100.0 * static_cast< double >( mCurrentStep ) / mChildSteps;
  double currentAlgorithmProgress = progress / mChildSteps;
  mFeedback->setProgress( baseProgress + currentAlgorithmProgress );
}


