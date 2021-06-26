/***************************************************************************
                         qgsprocessingalgrunnertask.h
                         ------------------------
    begin                : May 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
 * QgsTask task which runs a QgsProcessingAlgorithm in a background task.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingAlgRunnerTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingAlgRunnerTask. Takes an \a algorithm, algorithm \a parameters
     * and processing \a context.
     */
    QgsProcessingAlgRunnerTask( const QgsProcessingAlgorithm *algorithm,
                                const QVariantMap &parameters,
                                QgsProcessingContext &context,
                                QgsProcessingFeedback *feedback = nullptr );

    void cancel() override;

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


