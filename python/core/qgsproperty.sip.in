/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsproperty.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsPropertyDefinition
{
%Docstring
 Definition for a property.

QgsPropertyDefinition defines the type of values allowed for a property, and
handles descriptive names and help text for using the property. Definitions
can use one of the predefined standard templates to simplify definition of
commonly used property types, such as colors and blend modes.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsproperty.h"
%End
  public:

    enum StandardPropertyTemplate
    {
      Boolean,
      Integer,
      IntegerPositive,
      IntegerPositiveGreaterZero,
      Double,
      DoublePositive,
      Double0To1,
      Rotation,
      String,
      Opacity,
      RenderUnits,
      ColorWithAlpha,
      ColorNoAlpha,
      PenJoinStyle,
      BlendMode,
      Point,
      Size,
      Size2D,
      LineStyle,
      StrokeWidth,
      FillStyle,
      CapStyle,
      HorizontalAnchor,
      VerticalAnchor,
      SvgPath,
      Offset,
      Custom,
    };

    enum DataType
    {

      DataTypeString,

      DataTypeNumeric,

      DataTypeBoolean,
    };

    QgsPropertyDefinition();
%Docstring
Constructs an empty property.
%End

    QgsPropertyDefinition( const QString &name, const QString &description, StandardPropertyTemplate type, const QString &origin = QString(), const QString &comment = QString() );
%Docstring
Constructor for QgsPropertyDefinition, using a standard property template.

:param name: is used internally and should be a unique, alphanumeric string.
:param description: can be any localised string describing what the property is used for.
:param type: one of the predefined standard property template
:param origin: The origin of the property
:param comment: A free comment for the property
%End

    QgsPropertyDefinition( const QString &name, DataType dataType, const QString &description, const QString &helpText, const QString &origin = QString(), const QString &comment = QString() );
%Docstring
Constructor for custom QgsPropertyDefinitions.

:param name: is used internally and should be a unique, alphanumeric string.
:param dataType: the data type for the property
:param description: can be any localised string describing what the property is used for.
:param helpText: parameter should specify a descriptive string for users outlining the types
of value acceptable by the property (eg 'dashed' or 'solid' for a line style property).
:param origin: The origin of the property
:param comment: A free comment for the property
%End

    QString name() const;
%Docstring
Returns the name of the property. This is used internally and should be a unique, alphanumeric string.
%End

    void setName( const QString &name );
%Docstring
Sets the name of the property
%End

    QString origin() const;
%Docstring
Returns the origin of the property. For example, a PAL property has an
origin set to "labeling" while a diagram property has an origin set to
"diagram".
%End

    void setOrigin( const QString &origin );
%Docstring
Sets the origin of the property. For example, a PAL property has an
origin set to "labeling" while a diagram property has an origin set to
"diagram".
%End

    QString description() const;
%Docstring
Descriptive name of the property.
%End

    QString comment() const;
%Docstring
Returns the comment of the property
%End

    void setComment( const QString &comment );
%Docstring
Sets comment of the property
%End

    QString helpText() const;
%Docstring
Helper text for using the property, including a description of the valid values for the property.
%End

    void setDataType( DataType type );
%Docstring
Sets the data type
%End

    DataType dataType() const;
%Docstring
Returns the allowable field/value data type for the property.
%End

    StandardPropertyTemplate standardTemplate() const;
%Docstring
Returns the property's standard template, if applicable. Non standard
types will return the Custom template.
%End

    bool supportsAssistant() const;
%Docstring
Returns true if the property is of a type which is compatible with property
override assistants.
%End

};



class QgsProperty
{
%Docstring
 A store for object properties.

QgsProperty objects are used for storing properties for objects, which can then be transformed to
a QVariant value by evaluating them against a supplied :py:class:`QgsExpressionContext`. Multiple QgsProperty objects
can be grouped using a QgsPropertyCollection for easier bulk storage, retrieval and evaluation.

QgsProperty objects are implicitly shared and can be inexpensively copied.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsproperty.h"
%End
  public:

    enum Type
    {
      InvalidProperty,
      StaticProperty,
      FieldBasedProperty,
      ExpressionBasedProperty,
    };

    QgsProperty();
%Docstring
Constructor for a :py:class:`QgsAbstractProperty`. The property will be set to an InvalidProperty type.
%End

    virtual ~QgsProperty();

    static QgsProperty fromExpression( const QString &expression, bool isActive = true );
%Docstring
Returns a new ExpressionBasedProperty created from the specified expression.
%End

    static QgsProperty fromField( const QString &fieldName, bool isActive = true );
%Docstring
Returns a new FieldBasedProperty created from the specified field name.
%End

    static QgsProperty fromValue( const QVariant &value, bool isActive = true );
%Docstring
Returns a new StaticProperty created from the specified value.
%End

    QgsProperty( const QgsProperty &other );
%Docstring
Copy constructor
%End


    operator bool() const;

    bool operator==( const QgsProperty &other ) const;
    bool operator!=( const QgsProperty &other ) const;

    Type propertyType() const;
%Docstring
Returns the property type.
%End

    bool isActive() const;
%Docstring
Returns whether the property is currently active.

.. seealso:: :py:func:`setActive`
%End

    void setActive( bool active );
%Docstring
Sets whether the property is currently active.

.. seealso:: :py:func:`isActive`
%End

    void setStaticValue( const QVariant &value );
%Docstring
Sets the static value for the property. Calling this will
transform the property into an StaticProperty.

.. seealso:: :py:func:`staticValue`
%End

    QVariant staticValue() const;
%Docstring
Returns the current static value for the property. If the property
is not a StaticProperty this will return an invalid variant.

.. seealso:: :py:func:`setStaticValue`
%End

    void setField( const QString &field );
%Docstring
Sets the field name the property references. Calling this will
transform the property into an FieldBasedProperty.

.. seealso:: :py:func:`field`
%End

    QString field() const;
%Docstring
Returns the current field name the property references. If the property
is not a FieldBasedProperty this will return an empty string.

.. seealso:: :py:func:`setField`
%End

    void setExpressionString( const QString &expression );
%Docstring
Sets the expression to use for the property value. Calling this will
transform the property into an ExpressionBasedProperty.

.. seealso:: :py:func:`expressionString`
%End

    QString expressionString() const;
%Docstring
Returns the expression used for the property value. If the property
is not a ExpressionBasedProperty this will return an empty string.

.. seealso:: :py:func:`setExpressionString`
%End

    QString asExpression() const;
%Docstring
Returns an expression string representing the state of the property, or an empty
string if the property could not be converted to an expression
%End

    bool prepare( const QgsExpressionContext &context = QgsExpressionContext() ) const;
%Docstring
Prepares the property against a specified expression context. Calling prepare before evaluating the
property multiple times allows precalculation of expensive setup tasks such as parsing expressions.
Returns true if preparation was successful.
%End

    QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const;
%Docstring
Returns the set of any fields referenced by the property for a specified
expression context.
%End

    QVariant value( const QgsExpressionContext &context, const QVariant &defaultValue = QVariant(), bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property, including any transforms which are set for the property

:param context: QgsExpressionContext to evaluate the property for. The variables and functions contained
in the expression context can be used to alter the calculated value for the property, so that a property
is able to respond to the current environment, layers and features within QGIS.
:param defaultValue: default value to return if the property is not active or cannot be calculated
:param ok: if specified, will be set to true if conversion was successful

:return: calculated value for property

.. seealso:: :py:func:`valueAsString`

.. seealso:: :py:func:`valueAsColor`

.. seealso:: :py:func:`valueAsDouble`

.. seealso:: :py:func:`valueAsInt`

.. seealso:: :py:func:`valueAsBool`
%End

    QString valueAsString( const QgsExpressionContext &context, const QString &defaultString = QString(), bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property and interprets it as a string.

:param context: QgsExpressionContext to evaluate the property for.
:param defaultString: default string to return if the property cannot be calculated as a string
:param ok: if specified, will be set to true if conversion was successful

:return: value parsed to string

.. seealso:: :py:func:`value`

.. seealso:: :py:func:`valueAsColor`

.. seealso:: :py:func:`valueAsDouble`

.. seealso:: :py:func:`valueAsInt`

.. seealso:: :py:func:`valueAsBool`
%End

    QColor valueAsColor( const QgsExpressionContext &context, const QColor &defaultColor = QColor(), bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property and interprets it as a color.

:param context: QgsExpressionContext to evaluate the property for.
:param defaultColor: default color to return if the property cannot be calculated as a color
:param ok: if specified, will be set to true if conversion was successful

:return: value parsed to color

.. seealso:: :py:func:`value`

.. seealso:: :py:func:`valueAsString`

.. seealso:: :py:func:`valueAsDouble`

.. seealso:: :py:func:`valueAsInt`

.. seealso:: :py:func:`valueAsBool`
%End

    double valueAsDouble( const QgsExpressionContext &context, double defaultValue = 0.0, bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property and interprets it as a double.

:param context: QgsExpressionContext to evaluate the property for.
:param defaultValue: default double to return if the property cannot be calculated as a double
:param ok: if specified, will be set to true if conversion was successful

:return: value parsed to double

.. seealso:: :py:func:`value`

.. seealso:: :py:func:`valueAsString`

.. seealso:: :py:func:`valueAsColor`

.. seealso:: :py:func:`valueAsInt`

.. seealso:: :py:func:`valueAsBool`
%End

    int valueAsInt( const QgsExpressionContext &context, int defaultValue = 0, bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property and interprets it as an integer.

:param context: QgsExpressionContext to evaluate the property for.
:param defaultValue: default integer to return if the property cannot be calculated as an integer
:param ok: if specified, will be set to true if conversion was successful

:return: value parsed to integer

.. seealso:: :py:func:`value`

.. seealso:: :py:func:`valueAsString`

.. seealso:: :py:func:`valueAsColor`

.. seealso:: :py:func:`valueAsDouble`

.. seealso:: :py:func:`valueAsBool`
%End

    bool valueAsBool( const QgsExpressionContext &context, bool defaultValue = false, bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property and interprets it as an boolean.

:param context: QgsExpressionContext to evaluate the property for.
:param defaultValue: default boolean to return if the property cannot be calculated as an boolean
:param ok: if specified, will be set to true if conversion was successful

:return: value parsed to boolean

.. seealso:: :py:func:`value`

.. seealso:: :py:func:`valueAsString`

.. seealso:: :py:func:`valueAsColor`

.. seealso:: :py:func:`valueAsDouble`

.. seealso:: :py:func:`valueAsInt`
%End

    QVariant toVariant() const;
%Docstring
Saves this property to a QVariantMap, wrapped in a QVariant.
You can use QgsXmlUtils.writeVariant to save it to an XML document.

.. seealso:: :py:func:`loadVariant`
%End

    bool loadVariant( const QVariant &property );
%Docstring
Loads this property from a QVariantMap, wrapped in a QVariant.
You can use QgsXmlUtils.readVariant to load it from an XML document.

.. seealso:: :py:func:`toVariant`
%End

    void setTransformer( QgsPropertyTransformer *transformer /Transfer/ );
%Docstring
Sets an optional transformer to use for manipulating the calculated values for the property.

:param transformer: transformer to install. Ownership is transferred to the property, and any
existing transformer will be deleted. Set to null to remove an existing transformer.

.. seealso:: :py:func:`transformer`
%End

    const QgsPropertyTransformer *transformer() const;
%Docstring
Returns the existing transformer used for manipulating the calculated values for the property, if set.

.. seealso:: :py:func:`setTransformer`
%End

    bool convertToTransformer();
%Docstring
Attempts to convert an existing expression based property to a base expression with
corresponding transformer. Returns true if conversion was successful. Note that
calling this method requires multiple parsing of expressions, so it should only
be called in non-performance critical code.
%End

    operator QVariant() const;

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsproperty.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
