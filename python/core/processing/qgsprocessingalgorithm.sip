/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingalgorithm.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsProcessingAlgorithm
{
%Docstring
 Abstract base class for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingalgorithm.h"
%End
  public:

    enum Flag
    {
      FlagHideFromToolbox,
      FlagHideFromModeler,
      FlagSupportsBatch,
      FlagCanCancel,
      FlagRequiresMatchingCrs,
      FlagDeprecated,
    };
    typedef QFlags<QgsProcessingAlgorithm::Flag> Flags;


    QgsProcessingAlgorithm();
%Docstring
 Constructor for QgsProcessingAlgorithm.
%End

    virtual ~QgsProcessingAlgorithm();


    virtual QString name() const = 0;
%Docstring
 Returns the algorithm name, used for identifying the algorithm. This string
 should be fixed for the algorithm, and must not be localised. The name should
 be unique within each provider. Names should contain lowercase alphanumeric characters
 only and no spaces or other formatting characters.
.. seealso:: displayName()
.. seealso:: group()
.. seealso:: tags()
 :rtype: str
%End

    QString id() const;
%Docstring
 Returns the unique ID for the algorithm, which is a combination of the algorithm
 provider's ID and the algorithms unique name (e.g. "qgis:mergelayers" ).
.. seealso:: name()
.. seealso:: provider()
 :rtype: str
%End

    virtual QString displayName() const = 0;
%Docstring
 Returns the translated algorithm name, which should be used for any user-visible display
 of the algorithm name.
.. seealso:: name()
 :rtype: str
%End

    virtual QStringList tags() const;
%Docstring
 Returns a list of tags which relate to the algorithm, and are used to assist users in searching
 for suitable algorithms. These tags should be localised.
 :rtype: list of str
%End

    virtual QString shortHelpString() const;
%Docstring
 Returns a localised short helper string for the algorithm. This string should provide a basic description
 about what the algorithm does and the parameters and outputs associated with it.
.. seealso:: helpString()
.. seealso:: helpUrl()
 :rtype: str
%End

    virtual QString helpString() const;
%Docstring
 Returns a localised help string for the algorithm. Algorithm subclasses should implement either
 helpString() or helpUrl().
.. seealso:: helpUrl()
.. seealso:: shortHelpString()
 :rtype: str
%End

    virtual QString helpUrl() const;
%Docstring
 Returns a url pointing to the algorithm's help page.
.. seealso:: helpString()
.. seealso:: shortHelpString()
 :rtype: str
%End

    virtual QIcon icon() const;
%Docstring
 Returns an icon for the algorithm.
.. seealso:: svgIconPath()
 :rtype: QIcon
%End

    virtual QString svgIconPath() const;
%Docstring
 Returns a path to an SVG version of the algorithm's icon.
.. seealso:: icon()
 :rtype: str
%End

    virtual QString group() const;
%Docstring
 Returns the name of the group this algorithm belongs to. This string
 should be localised.
.. seealso:: tags()
 :rtype: str
%End

    virtual Flags flags() const;
%Docstring
 Returns the flags indicating how and when the algorithm operates and should be exposed to users.
 Default flags are FlagSupportsBatch and FlagCanCancel.
 :rtype: Flags
%End

    virtual bool canExecute( QString *errorMessage /Out/ = 0 ) const;
%Docstring
 Returns true if the algorithm can execute. Algorithm subclasses can return false
 here to indicate that they are not able to execute, e.g. as a result of unmet
 external dependencies. If specified, the ``errorMessage`` argument will be filled
 with a localised error message describing why the algorithm cannot execute.
 :rtype: bool
%End

    virtual bool checkParameterValues( const QVariantMap &parameters,
                                       QgsProcessingContext &context, QString *message /Out/ = 0 ) const;
%Docstring
 Checks the supplied ``parameter`` values to verify that they satisfy the requirements
 of this algorithm in the supplied ``context``. The ``message`` parameter will be
 filled with explanatory text if validation fails.
 Overridden implementations should also check this base class implementation.
 :return: true if parameters are acceptable for the algorithm.
 :rtype: bool
%End

    QgsProcessingProvider *provider() const;
%Docstring
 Returns the provider to which this algorithm belongs.
 :rtype: QgsProcessingProvider
%End

    QgsProcessingParameterDefinitions parameterDefinitions() const;
%Docstring
 Returns an ordered list of parameter definitions utilized by the algorithm.
.. seealso:: addParameter()
.. seealso:: parameterDefinition()
.. seealso:: destinationParameterDefinitions()
 :rtype: QgsProcessingParameterDefinitions
%End

    const QgsProcessingParameterDefinition *parameterDefinition( const QString &name ) const;
%Docstring
 Returns a matching parameter by ``name``. Matching is done in a case-insensitive
 manner.
.. seealso:: parameterDefinitions()
 :rtype: QgsProcessingParameterDefinition
%End

    int countVisibleParameters() const;
%Docstring
 Returns the number of visible (non-hidden) parameters defined by this
 algorithm.
 :rtype: int
%End

    QgsProcessingParameterDefinitions destinationParameterDefinitions() const;
%Docstring
 Returns a list of destination parameters definitions utilized by the algorithm.
.. seealso:: QgsProcessingParameterDefinition.isDestination()
.. seealso:: parameterDefinitions()
 :rtype: QgsProcessingParameterDefinitions
%End

    QgsProcessingOutputDefinitions outputDefinitions() const;
%Docstring
 Returns an ordered list of output definitions utilized by the algorithm.
.. seealso:: addOutput()
.. seealso:: outputDefinition()
 :rtype: QgsProcessingOutputDefinitions
%End

    const QgsProcessingOutputDefinition *outputDefinition( const QString &name ) const;
%Docstring
 Returns a matching output by ``name``. Matching is done in a case-insensitive
 manner.
.. seealso:: outputDefinitions()
 :rtype: QgsProcessingOutputDefinition
%End

    bool hasHtmlOutputs() const;
%Docstring
 Returns true if this algorithm generates HTML outputs.
 :rtype: bool
%End

    QVariantMap run( const QVariantMap &parameters,
                     QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const;
%Docstring
 Executes the algorithm using the specified ``parameters``.

 The ``context`` argument specifies the context in which the algorithm is being run.

 Algorithm progress should be reported using the supplied ``feedback`` object.

 :return: A map of algorithm outputs. These may be output layer references, or calculated
 values such as statistical calculations.
 :rtype: QVariantMap
%End

    virtual QWidget *createCustomParametersWidget( QWidget *parent = 0 ) const /Factory/;
%Docstring
 If an algorithm subclass implements a custom parameters widget, a copy of this widget
 should be constructed and returned by this method.
 The base class implementation returns None, which indicates that an autogenerated
 parameters widget should be used.
 :rtype: QWidget
%End

    QgsExpressionContext createExpressionContext( const QVariantMap &parameters,
        QgsProcessingContext &context ) const;
%Docstring
 Creates an expression context relating to the algorithm. This can be called by algorithms
 to create a new expression context ready for evaluating expressions within the algorithm.
 :rtype: QgsExpressionContext
%End

    virtual bool validateInputCrs( const QVariantMap &parameters,
                                   QgsProcessingContext &context ) const;
%Docstring
 Checks whether the coordinate reference systems for the specified set of ``parameters``
 are valid for the algorithm. For instance, the base implementation performs
 checks to ensure that all input CRS are equal
 Returns true if ``parameters`` have passed the CRS check.
 :rtype: bool
%End

    virtual QString asPythonCommand( const QVariantMap &parameters, QgsProcessingContext &context ) const;
%Docstring
 Returns a Python command string which can be executed to run the algorithm
 using the specified ``parameters``.

 Algorithms which cannot be run from a Python command should return an empty
 string.
 :rtype: str
%End

  protected:

    bool addParameter( QgsProcessingParameterDefinition *parameterDefinition /Transfer/ );
%Docstring
 Adds a parameter ``definition`` to the algorithm. Ownership of the definition is transferred to the algorithm.
 Returns true if parameter could be successfully added, or false if the parameter could not be added (e.g.
 as a result of a duplicate name).
.. seealso:: addOutput()
 :rtype: bool
%End

    void removeParameter( const QString &name );
%Docstring
 Removes the parameter with matching ``name`` from the algorithm, and deletes any existing
 definition.
%End

    bool addOutput( QgsProcessingOutputDefinition *outputDefinition /Transfer/ );
%Docstring
 Adds an output ``definition`` to the algorithm. Ownership of the definition is transferred to the algorithm.
 Returns true if the output could be successfully added, or false if the output could not be added (e.g.
 as a result of a duplicate name).
.. seealso:: addParameter()
 :rtype: bool
%End

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const = 0 /VirtualErrorHandler=processing_exception_handler/;
%Docstring
 Runs the algorithm using the specified ``parameters``. Algorithms should implement
 their custom processing logic here.

 The ``context`` argument specifies the context in which the algorithm is being run.

 Algorithm progress should be reported using the supplied ``feedback`` object. Additionally,
 well-behaved algorithms should periodically check ``feedback`` to determine whether the
 algorithm should be canceled and exited early.

 :return: A map of algorithm outputs. These may be output layer references, or calculated
 values such as statistical calculations.
 :rtype: QVariantMap
%End

    QString parameterAsString( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a static string value.
 :rtype: str
%End

    QString parameterAsExpression( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to an expression.
 :rtype: str
%End

    double parameterAsDouble( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a static double value.
 :rtype: float
%End

    int parameterAsInt( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a static integer value.
 :rtype: int
%End

    int parameterAsEnum( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a enum value.
 :rtype: int
%End

    QList<int> parameterAsEnums( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to list of enum values.
 :rtype: list of int
%End

    bool parameterAsBool( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a static boolean value.
 :rtype: bool
%End

    QgsFeatureSink *parameterAsSink( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context,
                                     const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs,
                                     QString &destinationIdentifier /Out/ ) const /Factory/;
%Docstring
 Evaluates the parameter with matching ``name`` to a feature sink.

 Sinks will either be taken from ``context``'s active project, or created from external
 providers and stored temporarily in the ``context``.

 The ``fields``, ``geometryType`` and ``crs`` parameters dictate the properties
 of the resulting feature sink.

 The ``destinationIdentifier`` argument will be set to a string which can be used to retrieve the layer corresponding
 to the sink, e.g. via calling QgsProcessingUtils.mapLayerFromString().

 This function creates a new object and the caller takes responsibility for deleting the returned object.
 :rtype: QgsFeatureSink
%End

    QgsFeatureSource *parameterAsSource( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const /Factory/;
%Docstring
 Evaluates the parameter with matching ``definition`` to a feature source.

 Sources will either be taken from ``context``'s active project, or loaded from external
 sources and stored temporarily in the ``context``.

 This function creates a new object and the caller takes responsibility for deleting the returned object.
 :rtype: QgsFeatureSource
%End

    QgsMapLayer *parameterAsLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a map layer.

 Layers will either be taken from ``context``'s active project, or loaded from external
 sources and stored temporarily in the ``context``. In either case, callers do not
 need to handle deletion of the returned layer.
 :rtype: QgsMapLayer
%End

    QgsRasterLayer *parameterAsRasterLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a raster layer.

 Layers will either be taken from ``context``'s active project, or loaded from external
 sources and stored temporarily in the ``context``. In either case, callers do not
 need to handle deletion of the returned layer.
 :rtype: QgsRasterLayer
%End

    QString parameterAsRasterOutputLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a raster output layer destination.
 :rtype: str
%End

    QString parameterAsFileOutput( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a file based output destination.
 :rtype: str
%End

    QgsVectorLayer *parameterAsVectorLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a vector layer.

 Layers will either be taken from ``context``'s active project, or loaded from external
 sources and stored temporarily in the ``context``. In either case, callers do not
 need to handle deletion of the returned layer.
 :rtype: QgsVectorLayer
%End

    QgsCoordinateReferenceSystem parameterAsCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a coordinate reference system.
 :rtype: QgsCoordinateReferenceSystem
%End

    QgsRectangle parameterAsExtent( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a rectangular extent.
 :rtype: QgsRectangle
%End

    QgsPointXY parameterAsPoint( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a point.
 :rtype: QgsPointXY
%End

    QString parameterAsFile( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a file/folder name.
 :rtype: str
%End

    QVariantList parameterAsMatrix( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a matrix/table of values.
 Tables are collapsed to a 1 dimensional list.
 :rtype: QVariantList
%End

    QList< QgsMapLayer *> parameterAsLayerList( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a list of map layers.
 :rtype: list of QgsMapLayer
%End

    QList<double> parameterAsRange( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a range of values.
 :rtype: list of float
%End

    QStringList parameterAsFields( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a list of fields.
 :rtype: list of str
%End


  private:
    QgsProcessingAlgorithm( const QgsProcessingAlgorithm &other );
};
QFlags<QgsProcessingAlgorithm::Flag> operator|(QgsProcessingAlgorithm::Flag f1, QFlags<QgsProcessingAlgorithm::Flag> f2);






/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingalgorithm.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
