/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingparameters.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsProcessingFeatureSourceDefinition
{
%Docstring

 Encapsulates settings relating to a feature source input to a processing algorithm.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingFeatureSourceDefinition( const QString &source = QString(), bool selectedFeaturesOnly = false );
%Docstring
 Constructor for QgsProcessingFeatureSourceDefinition, accepting a static string source.
%End

    QgsProcessingFeatureSourceDefinition( const QgsProperty &source, bool selectedFeaturesOnly = false );
%Docstring
 Constructor for QgsProcessingFeatureSourceDefinition, accepting a QgsProperty source.
%End

    QgsProperty source;
%Docstring
 Source definition. Usually a static property set to a source layer's ID or file name.
%End

    bool selectedFeaturesOnly;
%Docstring
 True if only selected features in the source should be used by algorithms.
%End

    bool operator==( const QgsProcessingFeatureSourceDefinition &other );

    bool operator!=( const QgsProcessingFeatureSourceDefinition &other );
%Docstring
 :rtype: bool
%End

    operator QVariant() const;
%Docstring
Allows direct construction of QVariants.
%End

};



class QgsProcessingOutputLayerDefinition
{
%Docstring

 Encapsulates settings relating to a feature sink or output raster layer for a processing algorithm.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingOutputLayerDefinition( const QString &sink = QString(), QgsProject *destinationProject = 0 );
%Docstring
 Constructor for QgsProcessingOutputLayerDefinition, accepting a static sink/layer string.
 The ``destinationProject`` parameter can be set to a QgsProject instance in which
 to automatically load the resulting sink/layer after completing processing.
%End

    QgsProcessingOutputLayerDefinition( const QgsProperty &sink, QgsProject *destinationProject = 0 );
%Docstring
 Constructor for QgsProcessingOutputLayerDefinition, accepting a QgsProperty sink/layer.
 The ``destinationProject`` parameter can be set to a QgsProject instance in which
 to automatically load the resulting sink/layer after completing processing.
%End

    QgsProperty sink;
%Docstring
 Sink/layer definition. Usually a static property set to the destination file name for the sink's layer.
%End

    QgsProject *destinationProject;
%Docstring
 Destination project. Can be set to a QgsProject instance in which
 to automatically load the resulting sink/layer after completing processing.
 The default behavior is not to load the result into any project (None).
%End

    QString destinationName;
%Docstring
 Name to use for sink if it's to be loaded into a destination project.
%End

    QVariantMap createOptions;
%Docstring
 Map of optional sink/layer creation options, which
 are passed to the underlying provider when creating new layers. Known options also
 include 'fileEncoding', which is used to specify a file encoding to use for created
 files.
%End


    operator QVariant() const;
%Docstring
Allows direct construction of QVariants.
%End

};







class QgsProcessingParameterDefinition
{
%Docstring

 Base class for the definition of processing parameters.

 Parameter definitions encapsulate properties regarding the behavior of parameters,
 their acceptable ranges, defaults, etc.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
%ConvertToSubClassCode
    if ( sipCpp->type() == QgsProcessingParameterBoolean::typeName() )
      sipType = sipType_QgsProcessingParameterBoolean;
    else if ( sipCpp->type() == QgsProcessingParameterCrs::typeName() )
      sipType = sipType_QgsProcessingParameterCrs;
    else if ( sipCpp->type() == QgsProcessingParameterMapLayer::typeName() )
      sipType = sipType_QgsProcessingParameterMapLayer;
    else if ( sipCpp->type() == QgsProcessingParameterExtent::typeName() )
      sipType = sipType_QgsProcessingParameterExtent;
    else if ( sipCpp->type() == QgsProcessingParameterPoint::typeName() )
      sipType = sipType_QgsProcessingParameterPoint;
    else if ( sipCpp->type() == QgsProcessingParameterFile::typeName() )
      sipType = sipType_QgsProcessingParameterFile;
    else if ( sipCpp->type() == QgsProcessingParameterMatrix::typeName() )
      sipType = sipType_QgsProcessingParameterMatrix;
    else if ( sipCpp->type() == QgsProcessingParameterMultipleLayers::typeName() )
      sipType = sipType_QgsProcessingParameterMultipleLayers;
    else if ( sipCpp->type() == QgsProcessingParameterNumber::typeName() )
      sipType = sipType_QgsProcessingParameterNumber;
    else if ( sipCpp->type() == QgsProcessingParameterRange::typeName() )
      sipType = sipType_QgsProcessingParameterRange;
    else if ( sipCpp->type() == QgsProcessingParameterRasterLayer::typeName() )
      sipType = sipType_QgsProcessingParameterRasterLayer;
    else if ( sipCpp->type() == QgsProcessingParameterEnum::typeName() )
      sipType = sipType_QgsProcessingParameterEnum;
    else if ( sipCpp->type() == QgsProcessingParameterString::typeName() )
      sipType = sipType_QgsProcessingParameterString;
    else if ( sipCpp->type() == QgsProcessingParameterExpression::typeName() )
      sipType = sipType_QgsProcessingParameterExpression;
    else if ( sipCpp->type() == QgsProcessingParameterVectorLayer::typeName() )
      sipType = sipType_QgsProcessingParameterVectorLayer;
    else if ( sipCpp->type() == QgsProcessingParameterField::typeName() )
      sipType = sipType_QgsProcessingParameterField;
    else if ( sipCpp->type() == QgsProcessingParameterFeatureSource::typeName() )
      sipType = sipType_QgsProcessingParameterFeatureSource;
    else if ( sipCpp->type() == QgsProcessingParameterFeatureSink::typeName() )
      sipType = sipType_QgsProcessingParameterFeatureSink;
    else if ( sipCpp->type() == QgsProcessingParameterVectorDestination::typeName() )
      sipType = sipType_QgsProcessingParameterVectorDestination;
    else if ( sipCpp->type() == QgsProcessingParameterRasterDestination::typeName() )
      sipType = sipType_QgsProcessingParameterRasterDestination;
    else if ( sipCpp->type() == QgsProcessingParameterFileDestination::typeName() )
      sipType = sipType_QgsProcessingParameterFileDestination;
    else if ( sipCpp->type() == QgsProcessingParameterFolderDestination::typeName() )
      sipType = sipType_QgsProcessingParameterFolderDestination;
    else if ( sipCpp->type() == QgsProcessingParameterBand::typeName() )
      sipType = sipType_QgsProcessingParameterBand;
    else
      sipType = nullptr;
%End
  public:

    enum Flag
    {
      FlagAdvanced,
      FlagHidden,
      FlagOptional,
    };
    typedef QFlags<QgsProcessingParameterDefinition::Flag> Flags;


    QgsProcessingParameterDefinition( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                      bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterDefinition.
%End

    virtual ~QgsProcessingParameterDefinition();

    virtual QgsProcessingParameterDefinition *clone() const = 0 /Factory/;
%Docstring
 Creates a clone of the parameter definition.
 :rtype: QgsProcessingParameterDefinition
%End

    virtual QString type() const = 0;
%Docstring
 Unique parameter type name.
 :rtype: str
%End

    virtual bool isDestination() const;
%Docstring
 Returns true if this parameter represents a file or layer destination, e.g. parameters
 which are used for the destination for layers output by an algorithm will return
 true.
 :rtype: bool
%End

    QString name() const;
%Docstring
 Returns the name of the parameter. This is the internal identifier by which
 algorithms access this parameter.
 @see setName()
 :rtype: str
%End

    void setName( const QString &name );
%Docstring
 Sets the ``name`` of the parameter. This is the internal identifier by which
 algorithms access this parameter.
 @see name()
%End

    QString description() const;
%Docstring
 Returns the description for the parameter. This is the user-visible string
 used to identify this parameter.
 @see setDescription()
 :rtype: str
%End

    void setDescription( const QString &description );
%Docstring
 Sets the ``description`` for the parameter. This is the user-visible string
 used to identify this parameter.
 @see description()
%End

    QVariant defaultValue() const;
%Docstring
 Returns the default value for the parameter.
 @see setDefaultValue()
 :rtype: QVariant
%End

    void setDefaultValue( const QVariant &value );
%Docstring
 Sets the default ``value`` for the parameter. Caller takes responsibility
 to ensure that ``value`` is a valid input for the parameter subclass.
 @see defaultValue()
%End

    Flags flags() const;
%Docstring
 Returns any flags associated with the parameter.
 @see setFlags()
 :rtype: Flags
%End

    void setFlags( const Flags &flags );
%Docstring
 Sets the ``flags`` associated with the parameter.
 @see flags()
%End

    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;
%Docstring
 Checks whether the specified ``input`` value is acceptable for the
 parameter. Returns true if the value can be accepted.
 The optional ``context`` parameter can be specified to allow a more stringent
 check to be performed, capable of checking for the presence of required
 layers and other factors within the context.
 :rtype: bool
%End

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;
%Docstring
 Returns a string version of the parameter input ``value``, which is suitable for use as an input
 parameter value when running an algorithm directly from a Python command.
 The returned value must be correctly escaped - e.g. string values must be wrapped in ' 's.
 :rtype: str
%End

    virtual QString asScriptCode() const;
%Docstring
 Returns the parameter definition encoded in a string which can be used within a
 Python processing script.
 :rtype: str
%End

    virtual QVariantMap toVariantMap() const;
%Docstring
 Saves this parameter to a QVariantMap. Subclasses should ensure that they call the base class
 method and then extend the result with additional properties.
.. seealso:: fromVariantMap()
 :rtype: QVariantMap
%End

    virtual bool fromVariantMap( const QVariantMap &map );
%Docstring
 Restores this parameter to a QVariantMap. Subclasses should ensure that they call the base class
 method.
.. seealso:: toVariantMap()
 :rtype: bool
%End


    QVariantMap &metadata();
%Docstring
 Returns the parameter's freeform metadata. This is mostly used by parameter widget wrappers
 in order to customise their appearance and behavior.
.. seealso:: setMetadata()
 :rtype: QVariantMap
%End

    void setMetadata( const QVariantMap &metadata );
%Docstring
 Sets the parameter's freeform ``metadata``. This is mostly used by parameter widget wrappers
 in order to customise their appearance and behavior.
.. seealso:: metadata()
%End

    virtual QStringList dependsOnOtherParameters() const;
%Docstring
 Returns a list of other parameter names on which this parameter is dependent (e.g.
 field parameters which depend on a parent layer parameter).
 :rtype: list of str
%End

    QgsProcessingAlgorithm *algorithm() const;
%Docstring
 Returns a pointer to the algorithm which owns this parameter. May be None
 for non-owned parameters.
.. seealso:: provider()
 :rtype: QgsProcessingAlgorithm
%End

    QgsProcessingProvider *provider() const;
%Docstring
 Returns a pointer to the provider for the algorithm which owns this parameter. May be None
 for non-owned parameters or algorithms.
.. seealso:: algorithm()
 :rtype: QgsProcessingProvider
%End

  protected:








};

QFlags<QgsProcessingParameterDefinition::Flag> operator|(QgsProcessingParameterDefinition::Flag f1, QFlags<QgsProcessingParameterDefinition::Flag> f2);


typedef QList< const QgsProcessingParameterDefinition * > QgsProcessingParameterDefinitions;


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

    static QString parameterAsString( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a static string value.
 :rtype: str
%End

    static QString parameterAsExpression( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to an expression.
 :rtype: str
%End

    static double parameterAsDouble( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a static double value.
 :rtype: float
%End

    static int parameterAsInt( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a static integer value.
 :rtype: int
%End

    static int parameterAsEnum( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a enum value.
 :rtype: int
%End

    static QList<int> parameterAsEnums( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to list of enum values.
 :rtype: list of int
%End

    static bool parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a static boolean value.
 :rtype: bool
%End

    static QgsFeatureSink *parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters,
                                            const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs,
                                            QgsProcessingContext &context, QString &destinationIdentifier /Out/ ) /Factory/;
%Docstring
 Evaluates the parameter with matching ``definition`` to a feature sink.

 The ``fields``, ``geometryType`` and ``crs`` parameters dictate the properties
 of the resulting feature sink.

 Sinks will either be taken from ``context``'s active project, or created from external
 providers and stored temporarily in the ``context``. The ``destinationIdentifier``
 argument will be set to a string which can be used to retrieve the layer corresponding
 to the sink, e.g. via calling QgsProcessingUtils.mapLayerFromString().

 This function creates a new object and the caller takes responsibility for deleting the returned object.
 :rtype: QgsFeatureSink
%End

    static QgsProcessingFeatureSource *parameterAsSource( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context ) /Factory/;
%Docstring
 Evaluates the parameter with matching ``definition`` to a feature source.

 Sources will either be taken from ``context``'s active project, or loaded from external
 sources and stored temporarily in the ``context``.

 This function creates a new object and the caller takes responsibility for deleting the returned object.
 :rtype: QgsProcessingFeatureSource
%End

    static QString parameterAsCompatibleSourceLayerPath( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters,
        QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat = QString( "shp" ), QgsProcessingFeedback *feedback = 0 );
%Docstring
 Evaluates the parameter with matching ``definition`` to a source vector layer file path of compatible format.

 If the parameter is evaluated to an existing layer, and that layer is not of the format listed in the
 ``compatibleFormats`` argument, then the layer will first be exported to a compatible format
 in a temporary location. The function will then return the path to that temporary file.

 ``compatibleFormats`` should consist entirely of lowercase file extensions, e.g. 'shp'.

 The ``preferredFormat`` argument is used to specify to desired file extension to use when a temporary
 layer export is required. This defaults to shapefiles, because shapefiles are the future (don't believe the geopackage hype!).
 :rtype: str
%End

    static QgsMapLayer *parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a map layer.

 Layers will either be taken from ``context``'s active project, or loaded from external
 sources and stored temporarily in the ``context``. In either case, callers do not
 need to handle deletion of the returned layer.
 :rtype: QgsMapLayer
%End

    static QgsRasterLayer *parameterAsRasterLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a raster layer.

 Layers will either be taken from ``context``'s active project, or loaded from external
 sources and stored temporarily in the ``context``. In either case, callers do not
 need to handle deletion of the returned layer.
 :rtype: QgsRasterLayer
%End

    static QString parameterAsOutputLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a output layer destination.
 :rtype: str
%End

    static QString parameterAsFileOutput( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a file based output destination.
 :rtype: str
%End

    static QgsVectorLayer *parameterAsVectorLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a vector layer.

 Layers will either be taken from ``context``'s active project, or loaded from external
 sources and stored temporarily in the ``context``. In either case, callers do not
 need to handle deletion of the returned layer.
 :rtype: QgsVectorLayer
%End

    static QgsCoordinateReferenceSystem parameterAsCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a coordinate reference system.
 :rtype: QgsCoordinateReferenceSystem
%End

    static QgsRectangle parameterAsExtent( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context,
                                           const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );
%Docstring
 Evaluates the parameter with matching ``definition`` to a rectangular extent.

 If ``crs`` is set, and the original coordinate reference system of the parameter can be determined, then the extent will be automatically
 reprojected so that it is in the specified ``crs``. In this case the extent of the reproject rectangle will be returned.

.. seealso:: parameterAsExtentGeometry()
.. seealso:: parameterAsExtentCrs()
 :rtype: QgsRectangle
%End

    static QgsGeometry parameterAsExtentGeometry( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context,
        const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );
%Docstring
 Evaluates the parameter with matching ``definition`` to a rectangular extent, and returns a geometry covering this extent.

 If ``crs`` is set, and the original coordinate reference system of the parameter can be determined, then the extent will be automatically
 reprojected so that it is in the specified ``crs``. Unlike parameterAsExtent(), the reprojected rectangle returned by this function
 will no longer be a rectangle itself (i.e. this method returns the geometry of the actual reprojected rectangle, while parameterAsExtent() returns
 just the extent of the reprojected rectangle).

.. seealso:: parameterAsExtent()
.. seealso:: parameterAsExtentCrs()
 :rtype: QgsGeometry
%End

    static QgsCoordinateReferenceSystem parameterAsExtentCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Returns the coordinate reference system associated with an extent parameter value.

.. seealso:: parameterAsExtent()
 :rtype: QgsCoordinateReferenceSystem
%End

    static QgsPointXY parameterAsPoint( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context,
                                        const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );
%Docstring
 Evaluates the parameter with matching ``definition`` to a point.

 If ``crs`` is set then the point will be automatically reprojected so that it is in the specified ``crs``.

.. seealso:: parameterAsPointCrs()
 :rtype: QgsPointXY
%End

    static QgsCoordinateReferenceSystem parameterAsPointCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Returns the coordinate reference system associated with an point parameter value.

.. seealso:: parameterAsPoint()
 :rtype: QgsCoordinateReferenceSystem
%End

    static QString parameterAsFile( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a file/folder name.
 :rtype: str
%End

    static QVariantList parameterAsMatrix( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a matrix/table of values.
 Tables are collapsed to a 1 dimensional list.
 :rtype: QVariantList
%End

    static QList< QgsMapLayer *> parameterAsLayerList( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a list of map layers.
 :rtype: list of QgsMapLayer
%End

    static QList<double> parameterAsRange( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a range of values.
 :rtype: list of float
%End

    static QStringList parameterAsFields( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a list of fields.
 :rtype: list of str
%End

    static QgsProcessingParameterDefinition *parameterFromVariantMap( const QVariantMap &map ) /Factory/;
%Docstring
 Creates a new QgsProcessingParameterDefinition using the configuration from a
 supplied variant ``map``.
 The caller takes responsibility for deleting the returned object.
 :rtype: QgsProcessingParameterDefinition
%End

    static QString descriptionFromName( const QString &name );
%Docstring
 Creates an autogenerated parameter description from a parameter ``name``.
 :rtype: str
%End

    static QgsProcessingParameterDefinition *parameterFromScriptCode( const QString &code ) /Factory/;
%Docstring
 Creates a new QgsProcessingParameterDefinition using the configuration from a
 supplied script ``code`` string.
 The caller takes responsibility for deleting the returned object.
 :rtype: QgsProcessingParameterDefinition
%End

};



class QgsProcessingParameterBoolean : QgsProcessingParameterDefinition
{
%Docstring
 A boolean parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterBoolean( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                   bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterBoolean.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QString asScriptCode() const;


    static QgsProcessingParameterBoolean *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterBoolean
%End
};

class QgsProcessingParameterCrs : QgsProcessingParameterDefinition
{
%Docstring
 A coordinate reference system parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterCrs( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                               bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterCrs.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;


    static QgsProcessingParameterCrs *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterCrs
%End

};

class QgsProcessingParameterMapLayer : QgsProcessingParameterDefinition
{
%Docstring
 A map layer parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterMapLayer( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                    bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterMapLayer.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;


    static QgsProcessingParameterMapLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterMapLayer
%End

};

class QgsProcessingParameterExtent : QgsProcessingParameterDefinition
{
%Docstring
 A rectangular map extent parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterExtent( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                  bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterExtent.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;


    static QgsProcessingParameterExtent *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterExtent
%End

};


class QgsProcessingParameterPoint : QgsProcessingParameterDefinition
{
%Docstring
 A point parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterPoint( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                 bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterPoint.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;


    static QgsProcessingParameterPoint *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterPoint
%End

};

class QgsProcessingParameterFile : QgsProcessingParameterDefinition
{
%Docstring
 An input file or folder parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    enum Behavior
    {
      File,
      Folder,
    };

    QgsProcessingParameterFile( const QString &name, const QString &description = QString(), Behavior behavior = File, const QString &extension = QString(), const QVariant &defaultValue = QVariant(),
                                bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterFile.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString asScriptCode() const;


    Behavior behavior() const;
%Docstring
 Returns the parameter behavior (e.g. File or Folder).
.. seealso:: setBehavior()
 :rtype: Behavior
%End

    void setBehavior( Behavior behavior );
%Docstring
 Sets the parameter ``behavior`` (e.g. File or Folder).
.. seealso:: behavior()
%End

    QString extension() const;
%Docstring
 Returns any specified file extension for the parameter.
.. seealso:: setExtension()
 :rtype: str
%End

    void setExtension( const QString &extension );
%Docstring
 Sets a file ``extension`` for the parameter.
.. seealso:: extension()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterFile *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition, Behavior behavior = File ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterFile
%End

};

class QgsProcessingParameterMatrix : QgsProcessingParameterDefinition
{
%Docstring
 An table (matrix) parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterMatrix( const QString &name, const QString &description = QString(), int numberRows = 3,
                                  bool hasFixedNumberRows = false, const QStringList &headers = QStringList(),
                                  const QVariant &defaultValue = QVariant(),
                                  bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterMatrix.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;


    QStringList headers() const;
%Docstring
 Returns a list of column headers (if set).
.. seealso:: setHeaders()
 :rtype: list of str
%End

    void setHeaders( const QStringList &headers );
%Docstring
 Sets the list of column ``headers``.
.. seealso:: headers()
%End

    int numberRows() const;
%Docstring
 Returns the fixed number of rows in the table. This parameter only has an
 effect if hasFixedNumberRows() is true.
.. seealso:: setNumberRows()
.. seealso:: setFixedNumberRows()
 :rtype: int
%End

    void setNumberRows( int rows );
%Docstring
 Sets the fixed number of ``rows`` in the table. This parameter only has an
 effect if hasFixedNumberRows() is true.
.. seealso:: numberRows()
.. seealso:: setFixedNumberRows()
%End

    bool hasFixedNumberRows() const;
%Docstring
 Returns whether the table has a fixed number of rows.
.. seealso:: numberRows()
.. seealso:: setHasFixedNumberRows()
 :rtype: bool
%End

    void setHasFixedNumberRows( bool hasFixedNumberRows );
%Docstring
 Sets whether the table has a fixed number of rows.
.. seealso:: setNumberRows()
.. seealso:: hasFixedNumberRows()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterMatrix *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterMatrix
%End

};

class QgsProcessingParameterMultipleLayers : QgsProcessingParameterDefinition
{
%Docstring
 A parameter for processing algorithms which accepts multiple map layers.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterMultipleLayers( const QString &name, const QString &description = QString(), QgsProcessing::SourceType layerType = QgsProcessing::TypeVectorAnyGeometry,
                                          const QVariant &defaultValue = QVariant(),
                                          bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterMultipleLayers.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QString asScriptCode() const;


    QgsProcessing::SourceType layerType() const;
%Docstring
 Returns the layer type for layers acceptable by the parameter.
.. seealso:: setLayerType()
 :rtype: QgsProcessing.SourceType
%End

    void setLayerType( QgsProcessing::SourceType type );
%Docstring
 Sets the layer ``type`` for layers acceptable by the parameter.
.. seealso:: layerType()
%End

    int minimumNumberInputs() const;
%Docstring
 Returns the minimum number of layers required for the parameter. If the return value is < 1
 then the parameter accepts any number of layers.
.. seealso:: setMinimumNumberInputs()
 :rtype: int
%End

    void setMinimumNumberInputs( int minimum );
%Docstring
 Sets the ``minimum`` number of layers required for the parameter. The minimum must be >= 1
 if the parameter is not optional.
.. seealso:: minimumNumberInputs()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterMultipleLayers *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterMultipleLayers
%End

};

class QgsProcessingParameterNumber : QgsProcessingParameterDefinition
{
%Docstring
 A numeric parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    enum Type
    {
      Integer,
      Double,
    };

    explicit QgsProcessingParameterNumber( const QString &name, const QString &description = QString(),
                                           Type type = Integer,
                                           const QVariant &defaultValue = QVariant(),
                                           bool optional = false,
                                           double minValue = -DBL_MAX,
                                           double maxValue = DBL_MAX
                                         );
%Docstring
 Constructor for QgsProcessingParameterNumber.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;


    double minimum() const;
%Docstring
 Returns the minimum value acceptable by the parameter.
.. seealso:: setMinimum()
 :rtype: float
%End

    void setMinimum( double minimum );
%Docstring
 Sets the ``minimum`` value acceptable by the parameter.
.. seealso:: minimum()
%End

    double maximum() const;
%Docstring
 Returns the maximum value acceptable by the parameter.
.. seealso:: setMaximum()
 :rtype: float
%End

    void setMaximum( double maximum );
%Docstring
 Sets the ``maximum`` value acceptable by the parameter.
.. seealso:: maximum()
%End

    Type dataType() const;
%Docstring
 Returns the acceptable data type for the parameter.
.. seealso:: setDataType()
 :rtype: Type
%End

    void setDataType( const Type &type );
%Docstring
 Sets the acceptable data ``type`` for the parameter.
.. seealso:: dataType()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterNumber *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterNumber
%End

};

class QgsProcessingParameterRange : QgsProcessingParameterDefinition
{
%Docstring
 A numeric range parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterRange( const QString &name, const QString &description = QString(),
                                 QgsProcessingParameterNumber::Type type = QgsProcessingParameterNumber::Integer,
                                 const QVariant &defaultValue = QVariant(),
                                 bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterRange.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;


    QgsProcessingParameterNumber::Type dataType() const;
%Docstring
 Returns the acceptable data type for the range.
.. seealso:: setDataType()
 :rtype: QgsProcessingParameterNumber.Type
%End

    void setDataType( const QgsProcessingParameterNumber::Type &dataType );
%Docstring
 Sets the acceptable data ``type`` for the range.
.. seealso:: dataType()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterRange *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterRange
%End

};

class QgsProcessingParameterRasterLayer : QgsProcessingParameterDefinition
{
%Docstring
 A raster layer parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterRasterLayer( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                       bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterRasterLayer.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;


    static QgsProcessingParameterRasterLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterRasterLayer
%End

};

class QgsProcessingParameterEnum : QgsProcessingParameterDefinition
{
%Docstring
 An enum based parameter for processing algorithms, allowing for selection from predefined values.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterEnum( const QString &name, const QString &description = QString(), const QStringList &options = QStringList(),
                                bool allowMultiple = false,
                                const QVariant &defaultValue = QVariant(),
                                bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterEnum.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QString asScriptCode() const;


    QStringList options() const;
%Docstring
 Returns the list of acceptable options for the parameter.
.. seealso:: setOptions()
 :rtype: list of str
%End

    void setOptions( const QStringList &options );
%Docstring
 Sets the list of acceptable ``options`` for the parameter.
.. seealso:: options()
%End

    bool allowMultiple() const;
%Docstring
 Returns true if the parameter allows multiple selected values.
.. seealso:: setAllowMultiple()
 :rtype: bool
%End

    void setAllowMultiple( bool allowMultiple );
%Docstring
 Sets whether the parameter allows multiple selected values.
.. seealso:: allowMultiple()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterEnum *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterEnum
%End

};

class QgsProcessingParameterString : QgsProcessingParameterDefinition
{
%Docstring
 A string parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterString( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                  bool multiLine = false,
                                  bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterString.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QString asScriptCode() const;


    bool multiLine() const;
%Docstring
 Returns true if the parameter allows multiline strings.
.. seealso:: setMultiLine()
 :rtype: bool
%End

    void setMultiLine( bool multiLine );
%Docstring
 Sets whether the parameter allows multiline strings.
.. seealso:: multiLine()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterString *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterString
%End

};

class QgsProcessingParameterExpression : QgsProcessingParameterDefinition
{
%Docstring
 An expression parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterExpression( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                      const QString &parentLayerParameterName = QString(),
                                      bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterExpression.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QStringList dependsOnOtherParameters() const;


    QString parentLayerParameterName() const;
%Docstring
 Returns the name of the parent layer parameter, or an empty string if this is not set.
.. seealso:: setParentLayerParameterName()
 :rtype: str
%End

    void setParentLayerParameterName( const QString &parentLayerParameterName );
%Docstring
 Sets the name of the parent layer parameter. Use an empty string if this is not required.
.. seealso:: parentLayerParameterName()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterExpression *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterExpression
%End

};


class QgsProcessingParameterLimitedDataTypes
{
%Docstring
 Can be inherited by parameters which require limits to their acceptable data types.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterLimitedDataTypes( const QList< int > &types = QList< int >() );
%Docstring
 Constructor for QgsProcessingParameterLimitedDataTypes, with a list of acceptable data ``types``.
%End

    QList< int > dataTypes() const;
%Docstring
 Returns the geometry types for sources acceptable by the parameter.
.. seealso:: setDataTypes()
 :rtype: list of int
%End

    void setDataTypes( const QList< int > &types );
%Docstring
 Sets the geometry ``types`` for sources acceptable by the parameter.
.. seealso:: dataTypes()
%End

  protected:

};

class QgsProcessingParameterVectorLayer : QgsProcessingParameterDefinition, QgsProcessingParameterLimitedDataTypes
{
%Docstring
 A vector layer (with or without geometry) parameter for processing algorithms. Consider using
 the more versatile QgsProcessingParameterFeatureSource wherever possible.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterVectorLayer( const QString &name,
                                       const QString &description = QString(),
                                       const QList< int > &types = QList< int >(),
                                       const QVariant &defaultValue = QVariant(),
                                       bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterVectorLayer.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;


    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterVectorLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterVectorLayer
%End

};

class QgsProcessingParameterField : QgsProcessingParameterDefinition
{
%Docstring
 A vector layer or feature source field parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    enum DataType
    {
      Any,
      Numeric,
      String,
      DateTime
    };

    QgsProcessingParameterField( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                 const QString &parentLayerParameterName = QString(),
                                 DataType type = Any,
                                 bool allowMultiple = false,
                                 bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterField.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QString asScriptCode() const;

    virtual QStringList dependsOnOtherParameters() const;


    QString parentLayerParameterName() const;
%Docstring
 Returns the name of the parent layer parameter, or an empty string if this is not set.
.. seealso:: setParentLayerParameterName()
 :rtype: str
%End

    void setParentLayerParameterName( const QString &parentLayerParameterName );
%Docstring
 Sets the name of the parent layer parameter. Use an empty string if this is not required.
.. seealso:: parentLayerParameterName()
%End

    DataType dataType() const;
%Docstring
 Returns the acceptable data type for the field.
.. seealso:: setDataType()
 :rtype: DataType
%End

    void setDataType( const DataType &type );
%Docstring
 Sets the acceptable data ``type`` for the field.
.. seealso:: dataType()
%End

    bool allowMultiple() const;
%Docstring
 Returns whether multiple field selections are permitted.
.. seealso:: setAllowMultiple()
 :rtype: bool
%End

    void setAllowMultiple( bool allowMultiple );
%Docstring
 Sets whether multiple field selections are permitted.
.. seealso:: allowMultiple()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterField *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterField
%End

};


class QgsProcessingParameterFeatureSource : QgsProcessingParameterDefinition, QgsProcessingParameterLimitedDataTypes
{
%Docstring
 An input feature source (such as vector layers) parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterFeatureSource( const QString &name, const QString &description = QString(),
                                         const QList< int > &types = QList< int >(),
                                         const QVariant &defaultValue = QVariant(), bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterFeatureSource.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QString asScriptCode() const;


    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterFeatureSource *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterFeatureSource
%End

};

class QgsProcessingDestinationParameter : QgsProcessingParameterDefinition
{
%Docstring
 Base class for all parameter definitions which represent file or layer destinations, e.g. parameters
 which are used for the destination for layers output by an algorithm.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingDestinationParameter( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                       bool optional = false );
%Docstring
 Constructor for QgsProcessingDestinationParameter.
%End

    virtual bool isDestination() const;
    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    virtual QgsProcessingOutputDefinition *toOutputDefinition() const = 0 /Factory/;
%Docstring
 Returns a new QgsProcessingOutputDefinition corresponding to the definition of the destination
 parameter.
 :rtype: QgsProcessingOutputDefinition
%End

    bool supportsNonFileBasedOutputs() const;
%Docstring
 Returns true if the destination parameter supports non filed-based outputs,
 such as memory layers or direct database outputs.
.. seealso:: setSupportsNonFileBasedOutputs()
 :rtype: bool
%End

    void setSupportsNonFileBasedOutputs( bool supportsNonFileBasedOutputs );
%Docstring
 Sets whether the destination parameter supports non filed-based outputs,
 such as memory layers or direct database outputs.
.. seealso:: supportsNonFileBasedOutputs()
%End

    virtual QString defaultFileExtension() const = 0;
%Docstring
 Returns the default file extension for destination file paths
 associated with this parameter.
 :rtype: str
%End

    virtual QString generateTemporaryDestination() const;
%Docstring
 Generates a temporary destination value for this parameter. The returned
 value will be a file path or QGIS data provider URI suitable for
 temporary storage of created layers and files.
 :rtype: str
%End

    bool createByDefault() const;
%Docstring
 Returns true if the destination should be created by default. For optional parameters,
 a return value of false indicates that the destination should not be created by default.
.. seealso:: setCreateByDefault()
 :rtype: bool
%End

    void setCreateByDefault( bool createByDefault );
%Docstring
 Sets whether the destination should be created by default. For optional parameters,
 a value of false indicates that the destination should not be created by default.
.. seealso:: createByDefault()
%End

};


class QgsProcessingParameterFeatureSink : QgsProcessingDestinationParameter
{
%Docstring
 A feature sink output for processing algorithms.

 A parameter which represents the destination feature sink for features created by an algorithm.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterFeatureSink( const QString &name, const QString &description = QString(), QgsProcessing::SourceType type = QgsProcessing::TypeVectorAnyGeometry, const QVariant &defaultValue = QVariant(),
                                       bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterFeatureSink.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QString asScriptCode() const;

    virtual QgsProcessingOutputDefinition *toOutputDefinition() const /Factory/;

    virtual QString defaultFileExtension() const;


    QgsProcessing::SourceType dataType() const;
%Docstring
 Returns the layer type for sinks associated with the parameter.
.. seealso:: setDataType()
 :rtype: QgsProcessing.SourceType
%End

    bool hasGeometry() const;
%Docstring
 Returns true if sink is likely to include geometries. In cases were presence of geometry
 cannot be reliably determined in advance, this method will default to returning true.
 :rtype: bool
%End

    void setDataType( QgsProcessing::SourceType type );
%Docstring
 Sets the layer ``type`` for the sinks associated with the parameter.
.. seealso:: dataType()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );

    virtual QString generateTemporaryDestination() const;


    static QgsProcessingParameterFeatureSink *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterFeatureSink
%End

};


class QgsProcessingParameterVectorDestination : QgsProcessingDestinationParameter
{
%Docstring
 A vector layer destination parameter, for specifying the destination path for a vector layer
 created by the algorithm.

.. note::

   Consider using the more flexible QgsProcessingParameterFeatureSink wherever
 possible.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterVectorDestination( const QString &name, const QString &description = QString(), QgsProcessing::SourceType type = QgsProcessing::TypeVectorAnyGeometry, const QVariant &defaultValue = QVariant(),
        bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterVectorDestination.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QString asScriptCode() const;

    virtual QgsProcessingOutputDefinition *toOutputDefinition() const /Factory/;

    virtual QString defaultFileExtension() const;


    QgsProcessing::SourceType dataType() const;
%Docstring
 Returns the layer type for this created vector layer.
.. seealso:: setDataType()
 :rtype: QgsProcessing.SourceType
%End

    bool hasGeometry() const;
%Docstring
 Returns true if the created layer is likely to include geometries. In cases were presence of geometry
 cannot be reliably determined in advance, this method will default to returning true.
 :rtype: bool
%End

    void setDataType( QgsProcessing::SourceType type );
%Docstring
 Sets the layer ``type`` for the created vector layer.
.. seealso:: dataType()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterVectorDestination *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterVectorDestination
%End


};

class QgsProcessingParameterRasterDestination : QgsProcessingDestinationParameter
{
%Docstring
 A raster layer destination parameter, for specifying the destination path for a raster layer
 created by the algorithm.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterRasterDestination( const QString &name, const QString &description = QString(),
        const QVariant &defaultValue = QVariant(),
        bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterRasterDestination.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QgsProcessingOutputDefinition *toOutputDefinition() const /Factory/;

    virtual QString defaultFileExtension() const;


    static QgsProcessingParameterRasterDestination *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterRasterDestination
%End
};

class QgsProcessingParameterFileDestination : QgsProcessingDestinationParameter
{
%Docstring
 A generic file based destination parameter, for specifying the destination path for a file (non-map layer)
 created by the algorithm.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterFileDestination( const QString &name, const QString &description = QString(),
                                           const QString &fileFilter = QString(),
                                           const QVariant &defaultValue = QVariant(),
                                           bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterFileDestination.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QgsProcessingOutputDefinition *toOutputDefinition() const /Factory/;

    virtual QString defaultFileExtension() const;


    QString fileFilter() const;
%Docstring
 Returns the file filter string for file destinations compatible with this parameter.
.. seealso:: setFileFilter()
 :rtype: str
%End

    void setFileFilter( const QString &filter );
%Docstring
 Sets the file ``filter`` string for file destinations compatible with this parameter.
.. seealso:: fileFilter()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterFileDestination *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterFileDestination
%End


};

class QgsProcessingParameterFolderDestination : QgsProcessingDestinationParameter
{
%Docstring
 A folder destination parameter, for specifying the destination path for a folder created
 by the algorithm or used for creating new files within the algorithm.
 A folder output parameter.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterFolderDestination( const QString &name, const QString &description = QString(),
        const QVariant &defaultValue = QVariant(),
        bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterFolderDestination.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QgsProcessingOutputDefinition *toOutputDefinition() const /Factory/;

    virtual QString defaultFileExtension() const;


    static QgsProcessingParameterFolderDestination *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterFolderDestination
%End

};

class QgsProcessingParameterBand : QgsProcessingParameterDefinition
{
%Docstring
 A raster band parameter for Processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterBand( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                const QString &parentLayerParameterName = QString(),
                                bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterBand.
%End

    static QString typeName();
%Docstring
 Returns the type name for the parameter class.
 :rtype: str
%End
    virtual QgsProcessingParameterDefinition *clone() const /Factory/;

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    virtual QString asScriptCode() const;

    virtual QStringList dependsOnOtherParameters() const;


    QString parentLayerParameterName() const;
%Docstring
 Returns the name of the parent layer parameter, or an empty string if this is not set.
.. seealso:: setParentLayerParameterName()
 :rtype: str
%End

    void setParentLayerParameterName( const QString &parentLayerParameterName );
%Docstring
 Sets the name of the parent layer parameter. Use an empty string if this is not required.
.. seealso:: parentLayerParameterName()
%End

    virtual QVariantMap toVariantMap() const;

    virtual bool fromVariantMap( const QVariantMap &map );


    static QgsProcessingParameterBand *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) /Factory/;
%Docstring
 Creates a new parameter using the definition from a script code.
 :rtype: QgsProcessingParameterBand
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingparameters.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
