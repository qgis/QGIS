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

/** \ingroup core
 * Base class for feedback objects to be used for cancelation of something running in a worker thread.
 * The class may be used as is or it may be subclassed for extended functionality
 * for a particular operation (e.g. report progress or pass some data for preview).
 *
 * When cancel() is called, the internal code has two options to check for cancelation state:
 * - if the worker thread uses an event loop (e.g. for network communication), the code can
 *   make a queued connection to canceled() signal and handle the cancelation in its slot.
 * - if the worker thread does not use an event loop, it can poll isCanceled() method regularly
 *   to see if the operation should be canceled.
 *
 * The class is meant to be created and destroyed in the main thread.
 *
 * For map rendering, the object may be created in constructor of a QgsMapLayerRenderer
 * subclass and available with QgsMapLayerRenderer::feedback() method. When a map rendering job
 * gets canceled, the cancel() method is called on the feedback object of all layers.
 *
 * @note added in QGIS 3.0
 */
class CORE_EXPORT QgsFeedback : public QObject
{
    Q_OBJECT
  public:
    //! Construct a feedback object
    QgsFeedback( QObject* parent = nullptr )
        : QObject( parent )
        , mCanceled( false )
    {}

    //! Tells the internal routines that the current operation should be canceled. This should be run by the main thread
    void cancel()
    {
      if ( mCanceled )
        return;  // only emit the signal once
      mCanceled = true;
      emit canceled();
    }

    //! Tells whether the operation has been canceled already
    bool isCanceled() const { return mCanceled; }

  signals:
    //! Internal routines can connect to this signal if they use event loop
    void canceled();

  private:
    //! Whether the operation has been canceled already. False by default.
    bool mCanceled;
};

#endif // QGSFEEDBACK_H
