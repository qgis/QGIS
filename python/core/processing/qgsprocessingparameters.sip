/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingparameters.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsProcessingParameters
{
%Docstring

 A collection of utilities for working with parameters when running a processing algorithm.

 Parameters are stored in a QVariantMap and referenced by a unique string key.
 The QVariants in parameters are not usually accessed
 directly, and instead the high level API provided through QgsProcessingParameters
 parameterAsString(), parameterAsDouble() are used instead.

 Parameters are evaluated using a provided QgsProcessingContext, allowing
 the evaluation to understand available map layers and expression contexts
 (for expression based parameters).

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    static bool isDynamic( const QVariantMap &parameters, const QString &name );
%Docstring
 Returns true if the parameter with matching ``name`` is a dynamic parameter, and must
 be evaluated once for every input feature processed.
 :rtype: bool
%End

    static QString parameterAsString( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``name`` to a static string value.
 :rtype: str
%End

    static double parameterAsDouble( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``name`` to a static double value.
 :rtype: float
%End

    static int parameterAsInt( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``name`` to a static integer value.
 :rtype: int
%End

    static bool parameterAsBool( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``name`` to a static boolean value.
 :rtype: bool
%End

    static QgsMapLayer *parameterAsLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``name`` to a map layer.

 Layers will either be taken from ``context``'s active project, or loaded from external
 sources and stored temporarily in the ``context``. In either case, callers do not
 need to handle deletion of the returned layer.
 :rtype: QgsMapLayer
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingparameters.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
