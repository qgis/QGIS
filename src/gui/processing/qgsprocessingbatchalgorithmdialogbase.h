/***************************************************************************
                             qgsprocessingalgorithmdialogbase.h
                             ----------------------------------
    Date                 : November 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGBATCHALGORITHMDIALOGBASE_H
#define QGSPROCESSINGBATCHALGORITHMDIALOGBASE_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsprocessingalgorithmdialogbase.h"

#include <QElapsedTimer>

class QgsProcessingBatchFeedback;
class QgsProxyProgressTask;

///@cond NOT_STABLE


/**
 * \ingroup gui
 * \brief Base class for processing batch algorithm dialogs.
 * \note This is not considered stable API and may change in future QGIS versions.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsProcessingBatchAlgorithmDialogBase : public QgsProcessingAlgorithmDialogBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingBatchAlgorithmDialogBase.
     */
    QgsProcessingBatchAlgorithmDialogBase( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
    ~QgsProcessingBatchAlgorithmDialogBase() override;

    void resetAdditionalGui() override;
    void blockAdditionalControlsWhileRunning() override;

  public slots:

    /**
     * Will be called when the "Run as Single" button is clicked.
     */
    virtual void runAsSingle() = 0;

  protected slots:

    void algExecuted( bool successful, const QVariantMap &results ) override;

  protected:

    bool isFinalized() override;

    /**
     * Starts the batch execution, where the \a parameters list dictates the parameters for each component
     * step of the batch.
     */
    void execute( const QList< QVariantMap > &parameters );

    /**
     * Creates a new Processing context.
     *
     * (Each step in the batch processing will use a new Processing context)
     */
    virtual QgsProcessingContext *createContext( QgsProcessingFeedback *feedback ) = 0 SIP_FACTORY;

    /**
     * Called when the dialog should handle the results of an algorithm, e.g. by loading layers into the current project.
     */
    virtual void handleAlgorithmResults( QgsProcessingAlgorithm *algorithm, QgsProcessingContext &context, QgsProcessingFeedback *feedback, const QVariantMap &parameters ) = 0;

    /**
     * Populates the HTML results dialog as a result of a successful algorithm execution.
     */
    virtual void loadHtmlResults( const QVariantMap &results, int index ) = 0;

    /**
     * Creates a summary table of the results of a batch execution.
     */
    virtual void createSummaryTable( const QList< QVariantMap > &results, const QList< QVariantMap > &errors ) = 0;

  private slots:

    void onTaskComplete( bool ok, const QVariantMap &results );
    void taskTriggered( QgsTask *task );

  private:

    void executeNext();
    void allTasksComplete( bool canceled );

    QPushButton *mButtonRunSingle = nullptr;

    int mCurrentStep = 0;
    int mTotalSteps = 0;
    QList< QVariantMap > mQueuedParameters;
    QVariantMap mCurrentParameters;
    QPointer< QgsProxyProgressTask > mProxyTask;
    std::unique_ptr< QgsProcessingFeedback > mFeedback;
    std::unique_ptr< QgsProcessingBatchFeedback > mBatchFeedback;
    std::unique_ptr< QgsProcessingContext > mTaskContext;
    QList< QVariantMap > mResults;
    QList< QVariantMap > mErrors;
    QElapsedTimer mTotalTimer;
    QElapsedTimer mCurrentStepTimer;
};

///@endcond

#endif // QGSPROCESSINGBATCHALGORITHMDIALOGBASE_H
