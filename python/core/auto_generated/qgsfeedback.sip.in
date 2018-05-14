/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeedback.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFeedback : QObject
{
%Docstring
Base class for feedback objects to be used for cancelation of something running in a worker thread.
The class may be used as is or it may be subclassed for extended functionality
for a particular operation (e.g. report progress or pass some data for preview).

When cancel() is called, the internal code has two options to check for cancelation state:
- if the worker thread uses an event loop (e.g. for network communication), the code can
make a queued connection to canceled() signal and handle the cancelation in its slot.
- if the worker thread does not use an event loop, it can poll isCanceled() method regularly
to see if the operation should be canceled.

The class is meant to be created and destroyed in the main thread.

For map rendering, the object may be created in constructor of a QgsMapLayerRenderer
subclass and available with QgsMapLayerRenderer.feedback() method. When a map rendering job
gets canceled, the cancel() method is called on the feedback object of all layers.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfeedback.h"
%End
  public:
    QgsFeedback( QObject *parent /TransferThis/ = 0 );
%Docstring
Construct a feedback object
%End

    bool isCanceled() const;
%Docstring
Tells whether the operation has been canceled already
%End

    void setProgress( double progress );
%Docstring
Sets the current progress for the feedback object. The ``progress``
argument is in percentage and valid values range from 0-100.

.. seealso:: :py:func:`progress`

.. seealso:: :py:func:`progressChanged`

.. versionadded:: 3.0
%End

    double progress() const;
%Docstring
Returns the current progress reported by the feedback object. Depending on how the
feedback object is used progress reporting may not be supported. The returned value
is in percentage and ranges from 0-100.

.. seealso:: :py:func:`setProgress`

.. seealso:: :py:func:`progressChanged`

.. versionadded:: 3.0
%End

  public slots:

    void cancel();
%Docstring
Tells the internal routines that the current operation should be canceled. This should be run by the main thread
%End

  signals:
    void canceled();
%Docstring
Internal routines can connect to this signal if they use event loop
%End

    void progressChanged( double progress );
%Docstring
Emitted when the feedback object reports a progress change. Depending on how the
feedback object is used progress reporting may not be supported. The ``progress``
argument is in percentage and ranges from 0-100.

.. versionadded:: 3.0

.. seealso:: :py:func:`setProgress`

.. seealso:: :py:func:`progress`
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeedback.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
