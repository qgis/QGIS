/***************************************************************************
  qgsfeedback.h
  --------------------------------------
  Date                 : July 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEEDBACK_H
#define QGSFEEDBACK_H

#include <QObject>

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \ingroup core
 * \brief Base class for feedback objects to be used for cancellation of something running in a worker thread.
 *
 * The class may be used as is or it may be subclassed for extended functionality
 * for a particular operation (e.g. report progress or pass some data for preview).
 *
 * When cancel() is called, the internal code has two options to check for cancellation state:
 *
 * - if the worker thread uses an event loop (e.g. for network communication), the code can make a queued connection to canceled() signal and handle the cancellation in its slot.
 * - if the worker thread does not use an event loop, it can poll isCanceled() method regularly to see if the operation should be canceled.
 *
 * The class is meant to be created and destroyed in the main thread.
 *
 * For map rendering, the object may be created in constructor of a QgsMapLayerRenderer
 * subclass and available with QgsMapLayerRenderer::feedback() method. When a map rendering job
 * gets canceled, the cancel() method is called on the feedback object of all layers.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFeedback : public QObject
{
    Q_OBJECT
  public:
    //! Construct a feedback object
    QgsFeedback( QObject *parent SIP_TRANSFERTHIS = nullptr )
      : QObject( parent )
    {}

    //! Tells whether the operation has been canceled already
    bool isCanceled() const SIP_HOLDGIL { return mCanceled; }

    /**
     * Sets the current progress for the feedback object. The \a progress
     * argument is in percentage and valid values range from 0-100.
     * \see progress()
     * \see progressChanged()
     * \since QGIS 3.0
     */
    void setProgress( double progress )
    {
      // avoid flooding with too many events
      if ( static_cast< int >( mProgress * 10 ) != static_cast< int >( progress * 10 ) )
        emit progressChanged( progress );

      mProgress = progress;
    }

    /**
     * Returns the current progress reported by the feedback object. Depending on how the
     * feedback object is used progress reporting may not be supported. The returned value
     * is in percentage and ranges from 0-100.
     * \see setProgress()
     * \see progressChanged()
     * \since QGIS 3.0
     */
    double progress() const SIP_HOLDGIL { return mProgress; }

    /**
     * Returns the current processed objects count reported by the feedback object. Depending on how the
     * feedback object is used processed count reporting may not be supported. The returned value
     * is an unsigned long integer and starts from 0.
     * \see setProcessedCount()
     * \see processedCountChanged()
     * \since QGIS 3.24
     */
    unsigned long long processedCount() const SIP_HOLDGIL { return mProcessedCount; }

    /**
     * Sets the current processed objects count for the feedback object. The \a processedCount
     * argument is an unsigned long integer and starts from 0.
     * \see processedCount()
     * \see processedCountChanged()
     * \since QGIS 3.24
     */
    void setProcessedCount( unsigned long long processedCount )
    {
      mProcessedCount = processedCount;
      emit processedCountChanged( processedCount );
    }

  public slots:

    //! Tells the internal routines that the current operation should be canceled. This should be run by the main thread
    void cancel()
    {
      if ( mCanceled )
        return;  // only emit the signal once
      mCanceled = true;
      emit canceled();
    }

  signals:
    //! Internal routines can connect to this signal if they use event loop
    void canceled();

    /**
     * Emitted when the feedback object reports a progress change. Depending on how the
     * feedback object is used progress reporting may not be supported. The \a progress
     * argument is in percentage and ranges from 0-100.
     * \see setProgress()
     * \see progress()
     * \since QGIS 3.0
     */
    void progressChanged( double progress );

    /**
     * Emitted when the feedback object reports a change in the number of processed objects.
     * Depending on how the feedback object is used processed count reporting may not be supported. The \a processedCount
     * argument is an unsigned long integer and starts from 0.
     * \see setProgress()
     * \see progress()
     * \since QGIS 3.24
     */
    void processedCountChanged( unsigned long long processedCount );

  private:
    //! Whether the operation has been canceled already. False by default.
    bool mCanceled = false;

    double mProgress = 0.0;
    unsigned long long mProcessedCount = 0;
};


#endif // QGSFEEDBACK_H
