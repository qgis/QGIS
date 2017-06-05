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

class QgsProcessingAlgorithm;
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
                                QgsProcessingContext &context );

    virtual void cancel() override;

  protected:

    virtual bool run() override;
    virtual void finished( bool result ) override;

  private:

    const QgsProcessingAlgorithm *mAlgorithm = nullptr;
    QVariantMap mParameters;
    QVariantMap mResults;
    QgsProcessingContext &mContext;
    std::unique_ptr< QgsProcessingFeedback > mFeedback;

};

#endif // QGSPROCESSINGALGRUNNERTASK_H


