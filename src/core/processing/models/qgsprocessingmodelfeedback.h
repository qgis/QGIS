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
#include "qgsprocessingmodelresult.h"

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
     * Reports that a feature source was retrieved for the specified child algorithm input parameter.
     *
     * \see childSourceLoaded()
     */
    void reportChildSourceLoaded( const QString &childId, const QString &parameterName, long long featureCount );

    /**
     * Reports that the count of features pushed to a child algorithm's sink has changed.
     *
     * The \a childOutput argument specifies the associated child algorithm output name.
     *
     * \see childSinkFeatureCountChanged()
     */
    void reportChildSinkFeatureCountChanged( const QString &childId, const QString &childOutput, long long featureCount );

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
     * Reports the \a result of the execution of a child algorithm.
     */
    void reportChildResult( const QString &childId, const QgsProcessingModelChildAlgorithmResult &result );

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
     * Emitted when a feature source was retrieved for the specified child algorithm input parameter.
     *
     * \see reportChildSourceLoaded()
     */
    void childSourceLoaded( const QString &childId, const QString &parameterName, long long featureCount );

    /**
     * Emitted when the count of features pushed to a child's sink has changed.
     *
     * The \a output argument specifies the associated child algorithm output name.
     *
     * \note For performance, this signal is not emitted for every individual feature
     * added to the sink. It is instead emitted only once for every 100 features added.
     *
     * \see reportChildSinkFeatureCountChanged()
     */
    void childSinkFeatureCountChanged( const QString &childId, const QString &childOutput, long long featureCount );

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
     * Emitted when the \a result of a child algorithm has been reported.
     */
    void childResultReported( const QString &childId, const QgsProcessingModelChildAlgorithmResult &result );

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
