/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingcontext.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsProcessingContext
{
%Docstring
 Contains information about the context in which a processing algorithm is executed.

 Contextual information includes settings such as the associated project, and
 expression context.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingcontext.h"
%End
  public:

    QgsProcessingContext();

    QgsProject *project() const;
%Docstring
 Returns the project in which the algorithm is being executed.
 \see setProject()
 :rtype: QgsProject
%End

    void setProject( QgsProject *project );
%Docstring
 Sets the ``project`` in which the algorithm will be executed.
 \see project()
%End

    QgsExpressionContext expressionContext() const;
%Docstring
 Returns the expression context.
 :rtype: QgsExpressionContext
%End

    void setExpressionContext( const QgsExpressionContext &context );
%Docstring
 Sets the expression ``context``.
%End

};





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingcontext.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

