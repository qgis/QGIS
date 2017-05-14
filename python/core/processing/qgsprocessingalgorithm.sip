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
 :rtype: Flags
%End

    QgsProcessingProvider *provider() const;
%Docstring
 Returns the provider to which this algorithm belongs.
 :rtype: QgsProcessingProvider
%End

    void setProvider( QgsProcessingProvider *provider );

    QgsProcessingParameterDefinitions parameterDefinitions() const;
%Docstring
 Returns an ordered list of parameter definitions utilized by the algorithm.
.. seealso:: addParameter()
.. seealso:: parameterDefinition()
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

    virtual QVariantMap run( const QVariantMap &parameters,
                             QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const;
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

  protected:

    bool addParameter( QgsProcessingParameterDefinition *parameterDefinition /Transfer/ );
%Docstring
 Adds a parameter ``definition`` to the algorithm. Ownership of the definition is transferred to the algorithm.
 Returns true if parameter could be successfully added, or false if the parameter could not be added (e.g.
 as a result of a duplicate name).
 :rtype: bool
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

    QgsPoint parameterAsPoint( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;
%Docstring
 Evaluates the parameter with matching ``name`` to a point.
 :rtype: QgsPoint
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
