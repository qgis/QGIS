/***************************************************************************
                             qgsproxyprogresstask.h
                             ----------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#ifndef QGSPROXYPROGRESSTASK_H
#define QGSPROXYPROGRESSTASK_H

#include "qgsvirtuallayerdefinition.h"
#include "qgstaskmanager.h"

#include <QWaitCondition>

/**
 * \ingroup core
 *
 * \brief A QgsTask shell which proxies progress reports.
 *
 * Simple task shell which runs until finalized and reports progress only.
 * This is usually used to expose a blocking operation's progress via
 * task manager.
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsProxyProgressTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProxyProgressTask, with the specified \a description.
     */
    QgsProxyProgressTask( const QString &description, bool canCancel = false );

    /**
     * Finalizes the task, with the specified \a result.
     *
     * This should be called when the operation being proxied has completed,
     * to remove this proxy task from the task manager.
     */
    void finalize( bool result );

    bool run() override;

    /**
     * Sets the \a progress (from 0 to 100) for the proxied operation.
     *
     * This method is safe to call from the main thread.
     */
    void setProxyProgress( double progress );

    void cancel() override;

  signals:

    /**
     * Emitted when the task is canceled.
     *
     * \since QGIS 3.26
     */
    void canceled();

  private:

    QWaitCondition mNotFinishedWaitCondition;
    QMutex mNotFinishedMutex;
    bool mAlreadyFinished = false;
    bool mResult = true;

};

/**
 * \ingroup core
 *
 * \brief Scoped QgsScopedProxyProgressTask, which automatically adds the proxy task
 * to the application task manager on construction and finalizes the task
 * when it goes out of scope.
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsScopedProxyProgressTask
{
  public:

    /**
     * Constructor for QgsScopedProxyProgressTask, with the specified \a description.
     */
    QgsScopedProxyProgressTask( const QString &description );

    ~QgsScopedProxyProgressTask();

    /**
     * Sets the \a progress (from 0 to 100) for the proxied operation.
     */
    void setProgress( double progress );

  private:

    QgsProxyProgressTask *mTask = nullptr;

    // SIP generates .cpp code that doesn't compile if commenting out the legit following line.
    // Q_DISABLE_COPY( QgsScopedProxyProgressTask )
};

#endif // QGSPROXYPROGRESSTASK_H
