/***************************************************************************
                         qgsprocessingalgrunnertask.h
                         ------------------------
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

#ifndef QGSPROCESSINGALGRUNNERTASK_H
#define QGSPROCESSINGALGRUNNERTASK_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgstaskmanager.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingalgorithm.h"

class QgsProcessingContext;

/**
 * \class QgsProcessingAlgRunnerTask
 * \ingroup core
 * \brief QgsTask task which runs a QgsProcessingAlgorithm in a background task.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingAlgRunnerTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingAlgRunnerTask. Takes an \a algorithm, algorithm \a parameters
     * and processing \a context.
     *
     * Since QGIS 3.26, the \a flags argument allows control over task flags.
     */
    QgsProcessingAlgRunnerTask( const QgsProcessingAlgorithm *algorithm,
                                const QVariantMap &parameters,
                                QgsProcessingContext &context,
                                QgsProcessingFeedback *feedback = nullptr,
                                QgsTask::Flags flags = QgsTask::CanCancel );

    void cancel() override;

    /**
     * Returns TRUE if the algorithm was canceled.
     *
     * \since QGIS 3.26
     */
    bool algorithmCanceled() { return isCanceled(); }

  signals:

    /**
     * Emitted when the algorithm has finished execution. If the algorithm completed
     * execution without errors then \a successful will be TRUE. The \a results argument
     * contains the results reported by the algorithm.
     */
    void executed( bool successful, const QVariantMap &results );

  protected:

    bool run() override;
    void finished( bool result ) override;

  private:

    QVariantMap mParameters;
    QVariantMap mResults;
    QgsProcessingContext &mContext;
    QgsProcessingFeedback *mFeedback = nullptr;
    std::unique_ptr< QgsProcessingFeedback > mOwnedFeedback;
    std::unique_ptr< QgsProcessingAlgorithm > mAlgorithm;

};

#endif // QGSPROCESSINGALGRUNNERTASK_H


