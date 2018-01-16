/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/models/qgsprocessingmodelchildalgorithm.h        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsProcessingModelChildAlgorithm : QgsProcessingModelComponent
{
%Docstring
Child algorithm representing a single component of a QgsProcessingModelAlgorithm.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingmodelchildalgorithm.h"
%End
  public:

    QgsProcessingModelChildAlgorithm( const QString &algorithmId = QString() );
%Docstring
Constructor for QgsProcessingModelChildAlgorithm. The ``algorithmId`` parameter
should be set to a QgsProcessingAlgorithm algorithm ID.
%End

    QgsProcessingModelChildAlgorithm( const QgsProcessingModelChildAlgorithm &other );

    QString childId() const;
%Docstring
Returns the child algorithm's unique ID string, used the identify
this child algorithm within its parent model.

.. seealso:: :py:func:`setChildId`

.. seealso:: :py:func:`generateChildId`
%End

    void setChildId( const QString &id );
%Docstring
Sets the child algorithm's unique ``id`` string, used the identify
this child algorithm within its parent model.

.. seealso:: :py:func:`childId`

.. seealso:: :py:func:`generateChildId`
%End

    void generateChildId( const QgsProcessingModelAlgorithm &model );
%Docstring
Automatically generates a unique childId() for the algorithm,
avoiding child IDs which are already present in ``model``.

.. seealso:: :py:func:`childId`

.. seealso:: :py:func:`setChildId`
%End

    QString algorithmId() const;
%Docstring
Returns the underlying child algorithm's ID.

.. seealso:: :py:func:`algorithm`

.. seealso:: :py:func:`setAlgorithmId`
%End

    void setAlgorithmId( const QString &algorithmId );
%Docstring
Sets the underlying child algorithm's ID. This
should be set to an existing QgsProcessingAlgorithm algorithm ID.

.. seealso:: :py:func:`algorithm`

.. seealso:: :py:func:`algorithmId`
%End

    QVariantMap configuration() const;
%Docstring
Returns the child algorithm's configuration map.

This map specifies configuration settings which are passed
to the algorithm, allowing it to dynamically adjust its initialized parameters
and outputs according to this configuration. This allows child algorithms in the model
to adjust their behavior at run time according to some user configuration.

.. seealso:: :py:func:`setConfiguration`
%End

    void setConfiguration( const QVariantMap &configuration );
%Docstring
Sets the child algorithm's ``configuration`` map.

This map specifies configuration settings which are passed
to the algorithm, allowing it to dynamically adjust its initialized parameters
and outputs according to this configuration. This allows child algorithms in the model
to adjust their behavior at run time according to some user configuration.

.. seealso:: :py:func:`configuration`
%End

    const QgsProcessingAlgorithm *algorithm() const;
%Docstring
Returns the underlying child algorithm, or a None
if a matching algorithm is not available.

.. seealso:: :py:func:`algorithmId`
%End

    QMap< QString, QList< QgsProcessingModelChildParameterSource > > parameterSources() const;
%Docstring
Returns a map of parameter sources. The keys are the child algorithm
parameter names, the values are the sources for that parameter.

.. seealso:: :py:func:`setParameterSources`

.. seealso:: :py:func:`addParameterSources`
%End

    void setParameterSources( const QMap< QString, QList< QgsProcessingModelChildParameterSource > > &sources );
%Docstring
Sets the map of parameter ``sources``. The keys are the child algorithm
parameter names, the values are the sources for that parameter.

.. seealso:: :py:func:`parameterSources`

.. seealso:: :py:func:`addParameterSources`
%End

    void addParameterSources( const QString &name, const QList< QgsProcessingModelChildParameterSource > &source );
%Docstring
Adds a parameter source. The ``name`` argument should match
one of the child algorithm's parameter names, and the ``sources``
argument is used to set the sources for that parameter.

Any existing parameter sources with matching name will be replaced.

.. seealso:: :py:func:`parameterSources`

.. seealso:: :py:func:`setParameterSources`
%End

    bool isActive() const;
%Docstring
Returns true if the child algorithm is active.

.. seealso:: :py:func:`setActive`
%End

    void setActive( bool active );
%Docstring
Sets whether the child algorithm is active.

.. seealso:: :py:func:`isActive`
%End

    QStringList dependencies() const;
%Docstring
Returns the list of child algorithms from the parent model on which this
algorithm is dependent. The returned list contains the id() of the
dependent algorithms.

.. seealso:: :py:func:`setDependencies`
%End

    void setDependencies( const QStringList &dependencies );
%Docstring
Sets the list of child algorithms from the parent model on which this
algorithm is dependent. The list should contain the id() of the
dependent algorithms.

.. seealso:: :py:func:`dependencies`
%End

    bool parametersCollapsed() const;
%Docstring
Returns true if the list of parameters for this algorithm should be collapsed
in the graphical modeller.

.. seealso:: :py:func:`setParametersCollapsed`

.. seealso:: :py:func:`outputsCollapsed`
%End

    void setParametersCollapsed( bool collapsed );
%Docstring
Sets whether the list of parameters for this algorithm should be collapsed
in the graphical modeller.

.. seealso:: :py:func:`parametersCollapsed`

.. seealso:: :py:func:`setOutputsCollapsed`
%End

    bool outputsCollapsed() const;
%Docstring
Returns true if the list of outputs for this algorithm should be collapsed
in the graphical modeller.

.. seealso:: :py:func:`setParametersCollapsed`

.. seealso:: :py:func:`parametersCollapsed`
%End

    void setOutputsCollapsed( bool collapsed );
%Docstring
Sets whether the list of outputs for this algorithm should be collapsed
in the graphical modeller.

.. seealso:: :py:func:`outputsCollapsed`

.. seealso:: :py:func:`setParametersCollapsed`
%End

    QMap<QString, QgsProcessingModelOutput> modelOutputs() const;
%Docstring
Returns the map of final model outputs which are generated by this child algorithm.
The keys are the output names from this child algorithm. Only outputs which are
part of the final outputs from the model are included in this map.

.. seealso:: :py:func:`setModelOutputs`

.. seealso:: :py:func:`modelOutput`
%End

    QgsProcessingModelOutput &modelOutput( const QString &name );
%Docstring
Returns the final model output with matching ``name``. If no output
exists with the name, a new one will be created and returned.

If child model outputs are altered by this method, :py:func:`QgsProcessingModelAlgorithm.updateDestinationParameters()`
must be called on the parent model.

.. seealso:: :py:func:`modelOutputs`

.. seealso:: :py:func:`setModelOutputs`
%End

    void setModelOutputs( const QMap<QString, QgsProcessingModelOutput> &outputs );
%Docstring
Sets the map of final model ``outputs`` which are generated by this child algorithm.
Only outputs which are part of the final outputs from the model should be included in this map.

If child model outputs are altered by this method, :py:func:`QgsProcessingModelAlgorithm.updateDestinationParameters()`
must be called on the parent model.

.. seealso:: :py:func:`modelOutputs`
%End

    QVariant toVariant() const;
%Docstring
Saves this child to a QVariant.

.. seealso:: :py:func:`loadVariant`
%End

    bool loadVariant( const QVariant &child );
%Docstring
Loads this child from a QVariant.

.. seealso:: :py:func:`toVariant`
%End

    QString asPythonCode() const;
%Docstring
Attempts to convert the child to executable Python code.
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/models/qgsprocessingmodelchildalgorithm.h        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
