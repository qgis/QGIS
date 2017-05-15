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


    QgsProcessingContext::Flags flags() const;
%Docstring
 Returns any flags set in the context.
.. seealso:: setFlags()
 :rtype: QgsProcessingContext.Flags
%End

    void setFlags( const QgsProcessingContext::Flags &flags );
%Docstring
 Sets ``flags`` for the context.
.. seealso:: flags()
%End

    QgsProject *project() const;
%Docstring
 Returns the project in which the algorithm is being executed.
.. seealso:: setProject()
 :rtype: QgsProject
%End

    void setProject( QgsProject *project );
%Docstring
 Sets the ``project`` in which the algorithm will be executed.
.. seealso:: project()
%End

    QgsExpressionContext &expressionContext();
%Docstring
 Returns the expression context.
 :rtype: QgsExpressionContext
%End


    void setExpressionContext( const QgsExpressionContext &context );
%Docstring
 Sets the expression ``context``.
%End

    QgsMapLayerStore *temporaryLayerStore();
%Docstring
 Returns a reference to the layer store used for storing temporary layers during
 algorithm execution.
 :rtype: QgsMapLayerStore
%End

    QStringList layersToLoadOnCompletion() const;
%Docstring
 Returns a list of layers (by ID or datasource) to load into the canvas upon completion of the algorithm or model.
.. seealso:: setLayersToLoadOnCompletion()
.. seealso:: addLayerToLoadOnCompletion()
 :rtype: list of str
%End

    void setLayersToLoadOnCompletion( const QStringList &layers );
%Docstring
 Sets the list of ``layers`` (by ID or datasource) to load into the canvas upon completion of the algorithm or model.
.. seealso:: addLayerToLoadOnCompletion()
.. seealso:: layersToLoadOnCompletion()
%End

    void addLayerToLoadOnCompletion( const QString &layer );
%Docstring
 Adds a ``layer`` to load (by ID or datasource) into the canvas upon completion of the algorithm or model.
.. seealso:: setLayersToLoadOnCompletion()
.. seealso:: layersToLoadOnCompletion()
%End


    QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck() const;
%Docstring
 Returns the behavior used for checking invalid geometries in input layers.
.. seealso:: setInvalidGeometryCheck()
 :rtype: QgsFeatureRequest.InvalidGeometryCheck
%End

    void setInvalidGeometryCheck( const QgsFeatureRequest::InvalidGeometryCheck &check );
%Docstring
 Sets the behavior used for checking invalid geometries in input layers.
.. seealso:: invalidGeometryCheck()
%End


    void setInvalidGeometryCallback( SIP_PYCALLABLE / AllowNone / );
%Docstring
 Sets a callback function to use when encountering an invalid geometry and
 invalidGeometryCheck() is set to GeometryAbortOnInvalid. This function will be
 called using the feature with invalid geometry as a parameter.
.. versionadded:: 3.0
.. seealso:: invalidGeometryCallback()
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


    QString defaultEncoding() const;
%Docstring
 Returns the default encoding to use for newly created files.
.. seealso:: setDefaultEncoding()
 :rtype: str
%End

    void setDefaultEncoding( const QString &encoding );
%Docstring
 Sets the default ``encoding`` to use for newly created files.
.. seealso:: defaultEncoding()
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
