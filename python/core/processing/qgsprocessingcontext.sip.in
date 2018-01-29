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
      // UseSelectionIfPresent,
    };
    typedef QFlags<QgsProcessingContext::Flag> Flags;


    QgsProcessingContext();
%Docstring
Constructor for QgsProcessingContext.
%End


    void copyThreadSafeSettings( const QgsProcessingContext &other );
%Docstring
Copies all settings which are safe for use across different threads from
``other`` to this context.
%End

    QgsProcessingContext::Flags flags() const;
%Docstring
Returns any flags set in the context.

.. seealso:: :py:func:`setFlags`
%End

    void setFlags( const QgsProcessingContext::Flags &flags );
%Docstring
Sets ``flags`` for the context.

.. seealso:: :py:func:`flags`
%End

    QgsProject *project() const;
%Docstring
Returns the project in which the algorithm is being executed.

.. seealso:: :py:func:`setProject`
%End

    void setProject( QgsProject *project );
%Docstring
Sets the ``project`` in which the algorithm will be executed.

This also automatically sets the transformContext() to match
the project's transform context.

.. seealso:: :py:func:`project`
%End

    QgsExpressionContext &expressionContext();
%Docstring
Returns the expression context.
%End


    void setExpressionContext( const QgsExpressionContext &context );
%Docstring
Sets the expression ``context``.
%End

    QgsCoordinateTransformContext transformContext() const;
%Docstring
Returns the coordinate transform context.

.. seealso:: :py:func:`setTransformContext`
%End

    void setTransformContext( const QgsCoordinateTransformContext &context );
%Docstring
Sets the coordinate transform ``context``.

Note that setting a project for the context will automatically set the coordinate transform
context.

.. seealso:: :py:func:`transformContext`
%End

    QgsMapLayerStore *temporaryLayerStore();
%Docstring
Returns a reference to the layer store used for storing temporary layers during
algorithm execution.
%End

    struct LayerDetails
    {

      LayerDetails( const QString &name, QgsProject *project, const QString &outputName = QString() );
%Docstring
Constructor for LayerDetails.
%End

      QString name;

      QString outputName;

      QgsProject *project;

    };

    QMap< QString, QgsProcessingContext::LayerDetails > layersToLoadOnCompletion() const;
%Docstring
Returns a map of layers (by ID or datasource) to LayerDetails, to load into the canvas upon completion of the algorithm or model.

.. seealso:: :py:func:`setLayersToLoadOnCompletion`

.. seealso:: :py:func:`addLayerToLoadOnCompletion`
%End

    void setLayersToLoadOnCompletion( const QMap< QString, QgsProcessingContext::LayerDetails > &layers );
%Docstring
Sets the map of ``layers`` (by ID or datasource) to LayerDetails, to load into the canvas upon completion of the algorithm or model.

.. seealso:: :py:func:`addLayerToLoadOnCompletion`

.. seealso:: :py:func:`layersToLoadOnCompletion`
%End

    void addLayerToLoadOnCompletion( const QString &layer, const QgsProcessingContext::LayerDetails &details );
%Docstring
Adds a ``layer`` to load (by ID or datasource) into the canvas upon completion of the algorithm or model.
The ``details`` parameter dictates the LayerDetails.

.. seealso:: :py:func:`setLayersToLoadOnCompletion`

.. seealso:: :py:func:`layersToLoadOnCompletion`
%End

    QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck() const;
%Docstring
Returns the behavior used for checking invalid geometries in input layers.

.. seealso:: :py:func:`setInvalidGeometryCheck`
%End

    void setInvalidGeometryCheck( const QgsFeatureRequest::InvalidGeometryCheck &check );
%Docstring
Sets the behavior used for checking invalid geometries in input layers.
Settings this to anything but QgsFeatureRequest.GeometryNoCheck will also
reset the invalidGeometryCallback() to a default implementation.

.. seealso:: :py:func:`invalidGeometryCheck`
%End

    void setInvalidGeometryCallback( SIP_PYCALLABLE / AllowNone / );
%Docstring
Sets a callback function to use when encountering an invalid geometry and
invalidGeometryCheck() is set to GeometryAbortOnInvalid. This function will be
called using the feature with invalid geometry as a parameter.

.. versionadded:: 3.0

.. seealso:: :py:func:`invalidGeometryCallback`
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


    void setTransformErrorCallback( SIP_PYCALLABLE / AllowNone / );
%Docstring
Sets a callback function to use when encountering a transform error when iterating
features. This function will be
called using the feature which encountered the transform error as a parameter.

.. versionadded:: 3.0

.. seealso:: :py:func:`transformErrorCallback`
%End
%MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setTransformErrorCallback( [a0]( const QgsFeature &arg )
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

.. seealso:: :py:func:`setDefaultEncoding`
%End

    void setDefaultEncoding( const QString &encoding );
%Docstring
Sets the default ``encoding`` to use for newly created files.

.. seealso:: :py:func:`defaultEncoding`
%End

    QgsProcessingFeedback *feedback();
%Docstring
Returns the associated feedback object.

.. seealso:: :py:func:`setFeedback`
%End

    void setFeedback( QgsProcessingFeedback *feedback );
%Docstring
Sets an associated ``feedback`` object. This allows context related functions
to report feedback and errors to users and processing logs. While ideally this feedback
object should outlive the context, only a weak pointer to ``feedback`` is stored
and no errors will occur if feedback is deleted before the context.
Ownership of ``feedback`` is not transferred.

.. seealso:: :py:func:`setFeedback`
%End

    QThread *thread();
%Docstring
Returns the thread in which the context lives.

.. seealso:: :py:func:`pushToThread`
%End

    void pushToThread( QThread *thread );
%Docstring
Pushes the thread affinity for the context (including all layers contained in the temporaryLayerStore()) into
another ``thread``. This method is only safe to call when the current thread matches the existing thread
affinity for the context (see thread()).

.. seealso:: :py:func:`thread`
%End

    void takeResultsFrom( QgsProcessingContext &context );
%Docstring
Takes the results from another ``context`` and merges them with the results currently
stored in this context. This includes settings like any layers loaded in the temporaryLayerStore()
and layersToLoadOnCompletion().
This is only safe to call when both this context and the other ``context`` share the same
thread() affinity, and that thread is the current thread.
%End

    QgsMapLayer *takeResultLayer( const QString &id ) /TransferBack/;
%Docstring
Takes the result map layer with matching ``id`` from the context and
transfers ownership of it back to the caller. This method can be used
to remove temporary layers which are not required for further processing
from a context.
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
