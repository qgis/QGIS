/***************************************************************************
                         qgsprocessingmodelfeedback.cpp
                         ----------------------
    begin                : May 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#include "qgsprocessingmodelfeedback.h"

#include "moc_qgsprocessingmodelfeedback.cpp"

QgsProcessingModelFeedback::QgsProcessingModelFeedback( bool logFeedback )
  : QgsProcessingFeedback( logFeedback )
{}

void QgsProcessingModelFeedback::reportBrokenChildAlgorithms( const QSet<QString> &childIds )
{
  emit childAlgorithmsBroken( childIds );
}

void QgsProcessingModelFeedback::reportPreparingChild( const QString &childId )
{
  mPreparedChildren.insert( childId );
  emit preparingChild( childId );
}

void QgsProcessingModelFeedback::reportChildPreparationFailure( const QString &childId, const QString &error )
{
  emit childPreparationFailed( childId, error );
}

void QgsProcessingModelFeedback::reportChildStarted( const QString &childId, const QVariantMap &childParameters )
{
  mStartedChildren.insert( childId );
  emit childStarted( childId, childParameters );
}

void QgsProcessingModelFeedback::reportChildProgress( const QString &childId, double progress )
{
  emit childProgressChanged( childId, progress );
}

void QgsProcessingModelFeedback::reportChildExecutionFailure( const QString &childId, const QString &error )
{
  mFailedChildren.insert( childId );
  emit childExecutionFailed( childId, error );
}

void QgsProcessingModelFeedback::reportChildExecutionSuccess( const QString &childId, const QVariantMap &childResults )
{
  mSuccessfullyExecutedChildren.insert( childId );
  emit childExecutionSucceeded( childId, childResults );
}

void QgsProcessingModelFeedback::reportChildPruned( const QString &childId )
{
  mPrunedChildren.insert( childId );
  emit childPruned( childId );
}
