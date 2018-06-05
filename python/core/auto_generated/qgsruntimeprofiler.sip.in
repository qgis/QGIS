/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsruntimeprofiler.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsRuntimeProfiler
{

%TypeHeaderCode
#include "qgsruntimeprofiler.h"
%End
  public:

    QgsRuntimeProfiler();
%Docstring
Constructor to create a new runtime profiler.
%End

    void beginGroup( const QString &name );
%Docstring
Begin the group for the profiler. Groups will append {GroupName}/ to the
front of the profile tag set using start.

:param name: The name of the group.
%End

    void endGroup();
%Docstring
End the current active group.
%End

    void start( const QString &name );
%Docstring
Start a profile event with the given name.

:param name: The name of the profile event. Will have the name of
             the active group appended after ending.
%End

    void end();
%Docstring
End the current profile event.
%End


    void clear();
%Docstring
clear Clear all profile data.
%End

    double totalTime();
%Docstring
The current total time collected in the profiler.

:return: The current total time collected in the profiler.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsruntimeprofiler.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
