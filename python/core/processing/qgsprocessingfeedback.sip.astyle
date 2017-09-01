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
.. seealso:: setProgress()
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
.. seealso:: pushCommandInfo()
.. seealso:: pushDebugInfo()
.. seealso:: pushConsoleInfo()
%End

    virtual void pushCommandInfo( const QString &info );
%Docstring
 Pushes an informational message containing a command from the algorithm.
 This is usually used to report commands which are executed in an external
 application or as subprocesses.
.. seealso:: pushInfo()
.. seealso:: pushDebugInfo()
.. seealso:: pushConsoleInfo()
%End

    virtual void pushDebugInfo( const QString &info );
%Docstring
 Pushes an informational message containing debugging helpers from
 the algorithm.
.. seealso:: pushInfo()
.. seealso:: pushCommandInfo()
.. seealso:: pushConsoleInfo()
%End

    virtual void pushConsoleInfo( const QString &info );
%Docstring
 Pushes a console feedback message from the algorithm. This is used to
 report the output from executing an external command or subprocess.
.. seealso:: pushInfo()
.. seealso:: pushDebugInfo()
.. seealso:: pushCommandInfo()
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingfeedback.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
