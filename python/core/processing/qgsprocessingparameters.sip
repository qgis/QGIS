/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingparameters.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








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
    if ( sipCpp->type() == "boolean" )
      sipType = sipType_QgsProcessingParameterBoolean;
    else if ( sipCpp->type() == "crs" )
      sipType = sipType_QgsProcessingParameterCrs;
    else if ( sipCpp->type() == "layer" )
      sipType = sipType_QgsProcessingParameterMapLayer;
    else if ( sipCpp->type() == "extent" )
      sipType = sipType_QgsProcessingParameterExtent;
    else if ( sipCpp->type() == "point" )
      sipType = sipType_QgsProcessingParameterPoint;
    else if ( sipCpp->type() == "file" )
      sipType = sipType_QgsProcessingParameterFile;
    else if ( sipCpp->type() == "matrix" )
      sipType = sipType_QgsProcessingParameterMatrix;
    else if ( sipCpp->type() == "multilayer" )
      sipType = sipType_QgsProcessingParameterMultipleLayers;
    else if ( sipCpp->type() == "number" )
      sipType = sipType_QgsProcessingParameterNumber;
    else if ( sipCpp->type() == "range" )
      sipType = sipType_QgsProcessingParameterRange;
    else if ( sipCpp->type() == "raster" )
      sipType = sipType_QgsProcessingParameterRasterLayer;
    else if ( sipCpp->type() == "enum" )
      sipType = sipType_QgsProcessingParameterEnum;
    else if ( sipCpp->type() == "string" )
      sipType = sipType_QgsProcessingParameterString;
    else if ( sipCpp->type() == "expression" )
      sipType = sipType_QgsProcessingParameterExpression;
    else if ( sipCpp->type() == "table" )
      sipType = sipType_QgsProcessingParameterTable;
    else if ( sipCpp->type() == "field" )
      sipType = sipType_QgsProcessingParameterTableField;
    else if ( sipCpp->type() == "source" )
      sipType = sipType_QgsProcessingParameterFeatureSource;
    else if ( sipCpp->type() == "vectorOut" )
      sipType = sipType_QgsProcessingParameterOutputVectorLayer;
%End
  public:

    enum Flag
    {
      FlagAdvanced,
      FlagHidden,
      FlagOptional,
    };
    typedef QFlags<QgsProcessingParameterDefinition::Flag> Flags;


    enum LayerType
    {
      TypeAny
      TypeVectorAny
      TypeVectorPoint,
      TypeVectorLine,
      TypeVectorPolygon,
      TypeRaster,
      TypeFile,
      TypeTable,
    };

    QgsProcessingParameterDefinition( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                      bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterDefinition.
%End

    virtual ~QgsProcessingParameterDefinition();

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
                                            const QString &encoding, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs,
                                            QgsProcessingContext &context, QString &destinationIdentifier /Out/ ) /Factory/;
%Docstring
 Evaluates the parameter with matching ``definition`` to a feature sink.

 The ``encoding``, ``fields``, ``geometryType`` and ``crs`` parameters dictate the properties
 of the resulting feature sink.

 Sinks will either be taken from ``context``'s active project, or created from external
 providers and stored temporarily in the ``context``. The ``destinationIdentifier``
 argument will be set to a string which can be used to retrieve the layer corresponding
 to the sink, e.g. via calling QgsProcessingUtils.mapLayerFromString().

 This function creates a new object and the caller takes responsibility for deleting the returned object.
 :rtype: QgsFeatureSink
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

    static QgsRectangle parameterAsExtent( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a rectangular extent.
 :rtype: QgsRectangle
%End

    static QgsPointXY parameterAsPoint( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );
%Docstring
 Evaluates the parameter with matching ``definition`` to a point.
 :rtype: QgsPointXY
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

    virtual QString type() const;
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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;

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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


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

    QgsProcessingParameterMultipleLayers( const QString &name, const QString &description = QString(), QgsProcessingParameterDefinition::LayerType layerType = QgsProcessingParameterDefinition::TypeVectorAny,
                                          const QVariant &defaultValue = QVariant(),
                                          bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterMultipleLayers.
%End

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


    QgsProcessingParameterDefinition::LayerType layerType() const;
%Docstring
 Returns the layer type for layers acceptable by the parameter.
.. seealso:: setLayerType()
 :rtype: QgsProcessingParameterDefinition.LayerType
%End

    void setLayerType( QgsProcessingParameterDefinition::LayerType type );
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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


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

    virtual QString type() const;

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

    virtual QString type() const;

    QString parentLayerParameter() const;
%Docstring
 Returns the name of the parent layer parameter, or an empty string if this is not set.
.. seealso:: setParentLayerParameter()
 :rtype: str
%End

    void setParentLayerParameter( const QString &parentLayerParameter );
%Docstring
 Sets the name of the parent layer parameter. Use an empty string if this is not required.
.. seealso:: parentLayerParameter()
%End

};

class QgsProcessingParameterTable : QgsProcessingParameterDefinition
{
%Docstring
 A table (i.e. vector layers with or without geometry) parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterTable( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                 bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterTable.
%End

    virtual QString type() const;

};

class QgsProcessingParameterTableField : QgsProcessingParameterDefinition
{
%Docstring
 A table field parameter for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    enum DataType
    {
      Any
      Numeric,
      String,
      DateTime
    };

    QgsProcessingParameterTableField( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                      const QString &parentLayerParameterName = QString(),
                                      DataType type = Any,
                                      bool allowMultiple = false,
                                      bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterTableField.
%End

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


    QString parentLayerParameter() const;
%Docstring
 Returns the name of the parent layer parameter, or an empty string if this is not set.
.. seealso:: setParentLayerParameter()
 :rtype: str
%End

    void setParentLayerParameter( const QString &parentLayerParameter );
%Docstring
 Sets the name of the parent layer parameter. Use an empty string if this is not required.
.. seealso:: parentLayerParameter()
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

};

class QgsProcessingParameterFeatureSource : QgsProcessingParameterDefinition
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

    virtual QString type() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


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

};


class QgsProcessingParameterOutputVectorLayer : QgsProcessingParameterDefinition
{
%Docstring
 A vector layer output for processing algorithms.

 A parameter which represents the destination for a vector layer created by an algorithm.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingparameters.h"
%End
  public:

    QgsProcessingParameterOutputVectorLayer( const QString &name, const QString &description = QString(), QgsProcessingParameterDefinition::LayerType type = QgsProcessingParameterDefinition::TypeVectorAny, const QVariant &defaultValue = QVariant(),
        bool optional = false );
%Docstring
 Constructor for QgsProcessingParameterOutputVectorLayer.
%End

    virtual QString type() const;
    virtual bool isDestination() const;
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = 0 ) const;


    QgsProcessingParameterDefinition::LayerType dataType() const;
%Docstring
 Returns the layer type for the output layer associated with the parameter.
.. seealso:: setDataType()
 :rtype: QgsProcessingParameterDefinition.LayerType
%End

    void setDataType( QgsProcessingParameterDefinition::LayerType type );
%Docstring
 Sets the layer ``type`` for the output layer associated with the parameter.
.. seealso:: dataType()
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingparameters.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
