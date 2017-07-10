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
.. seealso:: setChildId()
.. seealso:: generateChildId()
 :rtype: str
%End

    void setChildId( const QString &id );
%Docstring
 Sets the child algorithm's unique ``id`` string, used the identify
 this child algorithm within its parent model.
.. seealso:: childId()
.. seealso:: generateChildId()
%End

    void generateChildId( const QgsProcessingModelAlgorithm &model );
%Docstring
 Automatically generates a unique childId() for the algorithm,
 avoiding child IDs which are already present in ``model``.
.. seealso:: childId()
.. seealso:: setChildId()
%End

    QString algorithmId() const;
%Docstring
 Returns the underlying child algorithm's ID.
.. seealso:: algorithm()
.. seealso:: setAlgorithmId()
 :rtype: str
%End

    void setAlgorithmId( const QString &algorithmId );
%Docstring
 Sets the underlying child algorithm's ID. This
 should be set to an existing QgsProcessingAlgorithm algorithm ID.
.. seealso:: algorithm()
.. seealso:: algorithmId()
%End

    QVariantMap configuration() const;
%Docstring
 Returns the child algorithm's configuration map.

 This map specifies configuration settings which are passed
 to the algorithm, allowing it to dynamically adjust its initialized parameters
 and outputs according to this configuration. This allows child algorithms in the model
 to adjust their behavior at run time according to some user configuration.

.. seealso:: setConfiguration()
 :rtype: QVariantMap
%End

    void setConfiguration( const QVariantMap &configuration );
%Docstring
 Sets the child algorithm's ``configuration`` map.

 This map specifies configuration settings which are passed
 to the algorithm, allowing it to dynamically adjust its initialized parameters
 and outputs according to this configuration. This allows child algorithms in the model
 to adjust their behavior at run time according to some user configuration.

.. seealso:: configuration()
%End

    const QgsProcessingAlgorithm *algorithm() const;
%Docstring
 Returns the underlying child algorithm, or a None
 if a matching algorithm is not available.
.. seealso:: algorithmId()
 :rtype: QgsProcessingAlgorithm
%End

    QMap< QString, QList< QgsProcessingModelChildParameterSource > > parameterSources() const;
%Docstring
 Returns a map of parameter sources. The keys are the child algorithm
 parameter names, the values are the sources for that parameter.
.. seealso:: setParameterSources()
.. seealso:: addParameterSources()
 :rtype: QMap< str, QList< QgsProcessingModelChildParameterSource > >
%End

    void setParameterSources( const QMap< QString, QList< QgsProcessingModelChildParameterSource > > &sources );
%Docstring
 Sets the map of parameter ``sources``. The keys are the child algorithm
 parameter names, the values are the sources for that parameter.
.. seealso:: parameterSources()
.. seealso:: addParameterSources()
%End

    void addParameterSources( const QString &name, const QList< QgsProcessingModelChildParameterSource > &source );
%Docstring
 Adds a parameter source. The ``name`` argument should match
 one of the child algorithm's parameter names, and the ``sources``
 argument is used to set the sources for that parameter.

 Any existing parameter sources with matching name will be replaced.
.. seealso:: parameterSources()
.. seealso:: setParameterSources()
%End

    bool isActive() const;
%Docstring
 Returns true if the child algorithm is active.
.. seealso:: setActive()
 :rtype: bool
%End

    void setActive( bool active );
%Docstring
 Sets whether the child algorithm is active.
.. seealso:: isActive()
%End

    QStringList dependencies() const;
%Docstring
 Returns the list of child algorithms from the parent model on which this
 algorithm is dependent. The returned list contains the id() of the
 dependent algorithms.
.. seealso:: setDependencies()
 :rtype: list of str
%End

    void setDependencies( const QStringList &dependencies );
%Docstring
 Sets the list of child algorithms from the parent model on which this
 algorithm is dependent. The list should contain the id() of the
 dependent algorithms.
.. seealso:: dependencies()
%End

    bool parametersCollapsed() const;
%Docstring
 Returns true if the list of parameters for this algorithm should be collapsed
 in the graphical modeller.
.. seealso:: setParametersCollapsed()
.. seealso:: outputsCollapsed()
 :rtype: bool
%End

    void setParametersCollapsed( bool collapsed );
%Docstring
 Sets whether the list of parameters for this algorithm should be collapsed
 in the graphical modeller.
.. seealso:: parametersCollapsed()
.. seealso:: setOutputsCollapsed()
%End

    bool outputsCollapsed() const;
%Docstring
 Returns true if the list of outputs for this algorithm should be collapsed
 in the graphical modeller.
.. seealso:: setParametersCollapsed()
.. seealso:: parametersCollapsed()
 :rtype: bool
%End

    void setOutputsCollapsed( bool collapsed );
%Docstring
 Sets whether the list of outputs for this algorithm should be collapsed
 in the graphical modeller.
.. seealso:: outputsCollapsed()
.. seealso:: setParametersCollapsed()
%End

    QMap<QString, QgsProcessingModelOutput> modelOutputs() const;
%Docstring
 Returns the map of final model outputs which are generated by this child algorithm.
 The keys are the output names from this child algorithm. Only outputs which are
 part of the final outputs from the model are included in this map.
.. seealso:: setModelOutputs()
.. seealso:: modelOutput()
 :rtype: QMap<str, QgsProcessingModelOutput>
%End

    QgsProcessingModelOutput &modelOutput( const QString &name );
%Docstring
 Returns the final model output with matching ``name``. If no output
 exists with the name, a new one will be created and returned.

 If child model outputs are altered by this method, QgsProcessingModelAlgorithm.updateDestinationParameters()
 must be called on the parent model.

.. seealso:: modelOutputs()
.. seealso:: setModelOutputs()
 :rtype: QgsProcessingModelOutput
%End

    void setModelOutputs( const QMap<QString, QgsProcessingModelOutput> &outputs );
%Docstring
 Sets the map of final model ``outputs`` which are generated by this child algorithm.
 Only outputs which are part of the final outputs from the model should be included in this map.

 If child model outputs are altered by this method, QgsProcessingModelAlgorithm.updateDestinationParameters()
 must be called on the parent model.

.. seealso:: modelOutputs()
%End

    QVariant toVariant() const;
%Docstring
 Saves this child to a QVariant.
.. seealso:: loadVariant()
 :rtype: QVariant
%End

    bool loadVariant( const QVariant &child );
%Docstring
 Loads this child from a QVariant.
.. seealso:: toVariant()
 :rtype: bool
%End

    QString asPythonCode() const;
%Docstring
 Attempts to convert the child to executable Python code.
 :rtype: str
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/models/qgsprocessingmodelchildalgorithm.h        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
