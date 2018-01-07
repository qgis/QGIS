/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspathresolver.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsPathResolver
{
%Docstring
Resolves relative paths into absolute paths and vice versa. Used for writing

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspathresolver.h"
%End
  public:
    explicit QgsPathResolver( const QString &baseFileName = QString() );
%Docstring
Initialize path resolver with a base filename. Null filename means no conversion between relative/absolute path
%End

    QString writePath( const QString &filename ) const;
%Docstring
Prepare a filename to save it to the project file.
Creates an absolute or relative path according to the project settings.
Paths written to the project file should be prepared with this method.
%End

    QString readPath( const QString &filename ) const;
%Docstring
Turn filename read from the project file to an absolute path
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspathresolver.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
