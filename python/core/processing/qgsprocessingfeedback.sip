/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingfeedback.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsProcessingFeedback : QgsFeedback
{
%Docstring
Base class for providing feedback from a processing algorithm.

This base class implementation silently ignores all feedback reported by algorithms.
Subclasses of QgsProcessingFeedback can be used to log this feedback or report
it to users via the GUI.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingfeedback.h"
%End
  public:

    virtual void setProgressText( const QString &text );
%Docstring
Sets a progress report text string. This can be used in conjunction with
setProgress() to provide detailed progress reports, such as "Transformed
4 of 5 layers".

.. seealso:: :py:func:`setProgress`
%End

    virtual void reportError( const QString &error );
%Docstring
Reports that the algorithm encountered an error which prevented it
from successfully executing.
%End

    virtual void pushInfo( const QString &info );
%Docstring
Pushes a general informational message from the algorithm. This can
be used to report feedback which is neither a status report or an
error, such as "Found 47 matching features".

.. seealso:: :py:func:`pushCommandInfo`

.. seealso:: :py:func:`pushDebugInfo`

.. seealso:: :py:func:`pushConsoleInfo`
%End

    virtual void pushCommandInfo( const QString &info );
%Docstring
Pushes an informational message containing a command from the algorithm.
This is usually used to report commands which are executed in an external
application or as subprocesses.

.. seealso:: :py:func:`pushInfo`

.. seealso:: :py:func:`pushDebugInfo`

.. seealso:: :py:func:`pushConsoleInfo`
%End

    virtual void pushDebugInfo( const QString &info );
%Docstring
Pushes an informational message containing debugging helpers from
the algorithm.

.. seealso:: :py:func:`pushInfo`

.. seealso:: :py:func:`pushCommandInfo`

.. seealso:: :py:func:`pushConsoleInfo`
%End

    virtual void pushConsoleInfo( const QString &info );
%Docstring
Pushes a console feedback message from the algorithm. This is used to
report the output from executing an external command or subprocess.

.. seealso:: :py:func:`pushInfo`

.. seealso:: :py:func:`pushDebugInfo`

.. seealso:: :py:func:`pushCommandInfo`
%End

};


class QgsProcessingMultiStepFeedback : QgsProcessingFeedback
{
%Docstring

Processing feedback object for multi-step operations.

A processing feedback object which proxies its calls to an underlying
feedback object, but scales overall progress reports to account
for a number of child steps which each report their own feedback.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingfeedback.h"
%End
  public:

    QgsProcessingMultiStepFeedback( int steps, QgsProcessingFeedback *feedback );
%Docstring
Constructor for QgsProcessingMultiStepFeedback, for a process with the specified
number of ``steps``. This feedback object will proxy calls
to the specified ``feedback`` object.
%End

    void setCurrentStep( int step );
%Docstring
Sets the ``step`` which is being executed. This is used
to scale the current progress to account for progress through the overall process.
%End

    virtual void setProgressText( const QString &text );

    virtual void reportError( const QString &error );

    virtual void pushInfo( const QString &info );

    virtual void pushCommandInfo( const QString &info );

    virtual void pushDebugInfo( const QString &info );

    virtual void pushConsoleInfo( const QString &info );


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingfeedback.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
