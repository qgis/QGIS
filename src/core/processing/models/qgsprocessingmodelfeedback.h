/***************************************************************************
                         qgsprocessingmodelfeedback.h
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

#ifndef QGSPROCESSINGMODELFEEDBACK_H
#define QGSPROCESSINGMODELFEEDBACK_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgsprocessingfeedback.h"

#define SIP_NO_FILE

/**
 * \ingroup core
 * \brief A Processing feedback class with extra signals and properties specific to feedback from Processing model execution.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 4.2
*/
class CORE_EXPORT QgsProcessingModelFeedback : public QgsProcessingFeedback
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingModelFeedback.
     *
     * If \a logFeedback is TRUE, then all feedback received will be directed
     * to QgsMessageLog.
     */
    QgsProcessingModelFeedback( bool logFeedback = true );

    /**
     * Report a set of child algorithms as broken (e.g. depending on algorithms which are not available).
     *
     * \see childAlgorithmsBroken()
     */
    void reportBrokenChildAlgorithms( const QSet< QString > &childIds );

    /**
     * Report a child algorithm as undergoing the preparation step.
     *
     * \see preparingChild()
     */
    void reportPreparingChild( const QString &childId );

    /**
     * Returns the IDs of children which have been prepared.
     */
    QSet< QString > preparedChildren() const { return mPreparedChildren; }

    /**
     * Report an error which occurred while preparing a child algorithm.
     *
     * \see childPreparationFailed()
     */
    void reportChildPreparationFailure( const QString &childId, const QString &error );

    /**
     * Report a child algorithm as started execution.
     *
     * \see childStarted()
     */
    void reportChildStarted( const QString &childId, const QVariantMap &childParameters );

    /**
     * Returns the IDs of children which were started.
     */
    QSet< QString > startedChildren() const { return mStartedChildren; }

    /**
     * Reports the \a progress of a running child algorithm.
     *
     * \see childProgressChanged()
     */
    void reportChildProgress( const QString &childId, double progress );

    /**
     * Report an error which occurred while executing a child algorithm.
     *
     * \see childExecutionFailed()
     */
    void reportChildExecutionFailure( const QString &childId, const QString &error );

    /**
     * Returns the IDs of children which were failed execution.
     */
    QSet< QString > failedChildren() const { return mFailedChildren; }

    /**
     * Report that a child algorithm successfully executed.
     *
     * \see childExecutionSucceeded()
     */
    void reportChildExecutionSuccess( const QString &childId, const QVariantMap &childResults );

    /**
     * Returns the IDs of children which were successfully executed.
     */
    QSet< QString > successfulChildren() const { return mSuccessfullyExecutedChildren; }

    /**
     * Report that a child algorithm was pruned from the pending children (i.e. it does not need to execute).
     *
     * \see childPruned()
     */
    void reportChildPruned( const QString &childId );

    /**
     * Returns the IDs of children which were pruned.
     */
    QSet< QString > prunedChildren() const { return mPrunedChildren; }

  signals:

    /**
     * Emitted when a set of child algorithms was reported as broken (e.g. depending on algorithms which are not available).
     *
     * \see reportBrokenChildAlgorithms()
     */
    void childAlgorithmsBroken( const QSet< QString > &childIds );

    /**
     * Emitted when a child algorithm starts preparation.
     *
     * \see reportPreparingChild()
     */
    void preparingChild( const QString &childId );

    /**
     * Emitted when an error occurred while preparing a child algorithm.
     *
     * \see reportChildPreparationFailure()
     */
    void childPreparationFailed( const QString &childId, const QString &error );

    /**
     * Emitted when a child algorithm has started executing.
     *
     * \see reportChildStarted()
     */
    void childStarted( const QString &childId, const QVariantMap &childParameters );

    /**
     * Emitted when a child algorithm changes \a progress.
     *
     * \see reportChildProgress()
     */
    void childProgressChanged( const QString &childId, double progress );

    /**
     * Emitted when an error occurred while executing a child algorithm.
     *
     * \see reportChildExecutionFailure()
     */
    void childExecutionFailed( const QString &childId, const QString &error );

    /**
     * Emitted when a child algorithm successfully executed.
     *
     * \see reportChildExecutionSuccess()
     */
    void childExecutionSucceeded( const QString &childId, const QVariantMap &childResults );

    /**
     * Emitted when a child algorithm was pruned from the pending children (i.e. it does not need to execute).
     *
     * \see reportChildPruned()
     */
    void childPruned( const QString &childId );

  private:
    QSet< QString > mPreparedChildren;
    QSet< QString > mStartedChildren;
    QSet< QString > mSuccessfullyExecutedChildren;
    QSet< QString > mFailedChildren;
    QSet< QString > mPrunedChildren;
};

#endif // QGSPROCESSINGMODELFEEDBACK_H
