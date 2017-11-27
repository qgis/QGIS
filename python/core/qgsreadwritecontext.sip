/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsreadwritecontext.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsReadWriteContext
{
%Docstring
 The class is used as a container of context for various read/write operations on other objects.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsreadwritecontext.h"
%End
  public:

    QgsReadWriteContext();
%Docstring
 Constructor for QgsReadWriteContext.
%End

    const QgsPathResolver &pathResolver() const;
%Docstring
Returns path resolver for conversion between relative and absolute paths
 :rtype: QgsPathResolver
%End

    void setPathResolver( const QgsPathResolver &resolver );
%Docstring
Sets up path resolver for conversion between relative and absolute paths
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsreadwritecontext.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
