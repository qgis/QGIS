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

    enum Flag
    {
      UseSelectionIfPresent,
    };
    typedef QFlags<QgsProcessingContext::Flag> Flags;


    QgsProcessingContext();
%Docstring
 Constructor for QgsProcessingContext.
%End

//     QgsProcessingContext &operator=( const QgsProcessingContext &other ) = delete;
    QgsProcessingContext::Flags flags() const;
%Docstring
 Returns any flags set in the context.
 \see setFlags()
 :rtype: QgsProcessingContext.Flags
%End

    void setFlags( const QgsProcessingContext::Flags &flags );
%Docstring
 Sets ``flags`` for the context.
 \see flags()
%End

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




    QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck() const;
%Docstring
 Returns the behavior used for checking invalid geometries in input layers.
 \see setInvalidGeometryCheck()
 :rtype: QgsFeatureRequest.InvalidGeometryCheck
%End

    void setInvalidGeometryCheck( const QgsFeatureRequest::InvalidGeometryCheck &check );
%Docstring
 Sets the behavior used for checking invalid geometries in input layers.
 \see invalidGeometryCheck()
%End


    void setInvalidGeometryCallback( SIP_PYCALLABLE / AllowNone / );
%Docstring
 Sets a callback function to use when encountering an invalid geometry and
 invalidGeometryCheck() is set to GeometryAbortOnInvalid. This function will be
 called using the feature with invalid geometry as a parameter.
.. versionadded:: 3.0
 \see invalidGeometryCallback()
%End
%MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setInvalidGeometryCallback( [a0]( const QgsFeature &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QgsFeature, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    Py_END_ALLOW_THREADS
%End


  private:
    QgsProcessingContext( const QgsProcessingContext &other );
};
QFlags<QgsProcessingContext::Flag> operator|(QgsProcessingContext::Flag f1, QFlags<QgsProcessingContext::Flag> f2);






/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingcontext.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
