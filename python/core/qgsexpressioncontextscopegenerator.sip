/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsexpressioncontextscopegenerator.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsExpressionContextScopeGenerator
{
%Docstring
Abstract interface for generating an expression context scope.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsexpressioncontextscopegenerator.h"
%End
  public:

    virtual QgsExpressionContextScope *createExpressionContextScope() const = 0 /Factory/;
%Docstring
This method needs to be reimplemented in all classes which implement this interface
and return an expression context scope.

.. versionadded:: 3.0
%End

    virtual ~QgsExpressionContextScopeGenerator();
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsexpressioncontextscopegenerator.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
