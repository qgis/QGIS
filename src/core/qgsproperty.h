/***************************************************************************
     qgsproperty.h
     -------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROPERTY_H
#define QGSPROPERTY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsproperty_p.h"
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgscolorramp.h"

#include <QVariant>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QDomDocument>
#include <QColor>

class QgsPropertyTransformer;

/**
 * \ingroup core
 * \class QgsPropertyDefinition
 * \brief Definition for a property.
 *
 * QgsPropertyDefinition defines the type of values allowed for a property, and
 * handles descriptive names and help text for using the property. Definitions
 * can use one of the predefined standard templates to simplify definition of
 * commonly used property types, such as colors and blend modes.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPropertyDefinition
{
  public:

    //! Predefined standard property templates
    enum StandardPropertyTemplate
    {
      Boolean = 0, //!< Boolean value
      Integer, //!< Integer value (including negative values)
      IntegerPositive, //!< Positive integer values (including 0)
      IntegerPositiveGreaterZero, //!< Non-zero positive integer values
      Double, //!< Double value (including negative values)
      DoublePositive, //!< Positive double value (including 0)
      Double0To1, //!< Double value between 0-1 (inclusive)
      Rotation, //!< Rotation (value between 0-360 degrees)
      String, //!< Any string value
      Opacity, //!< Opacity (0-100)
      RenderUnits, //!< Render units (eg mm/pixels/map units)
      ColorWithAlpha, //!< Color with alpha channel
      ColorNoAlpha, //!< Color with no alpha channel
      PenJoinStyle, //!< Pen join style
      BlendMode, //!< Blend mode
      Point, //!< 2D point
      Size, //!< 1D size (eg marker radius, or square marker height/width)
      Size2D, //!< 2D size (width/height different)
      LineStyle, //!< Line style (eg solid/dashed)
      StrokeWidth, //!< Line stroke width
      FillStyle, //!< Fill style (eg solid, lines)
      CapStyle, //!< Line cap style (eg round)
      HorizontalAnchor, //!< Horizontal anchor point
      VerticalAnchor, //!< Vertical anchor point
      SvgPath, //!< Path to an SVG file
      Offset, //!< 2D offset
      Custom = 3000, //!< Custom property types
    };

    //! Valid data types required by property
    enum DataType
    {

      /**
       * Property requires a string value. No numeric values are acceptable by the property.
       * Use this for properties which require a string value such as 'dashed' which cannot
       * be stored in a non-string field.
       */
      DataTypeString = 0,

      /**
       * Property requires a numeric value. Note that setting DataTypeNumeric as the required type
       * means that the property also accepts string fields and inputs, as those may be convertible
       * to a numeric value (Eg "1.0" -> 1.0)
       */
      DataTypeNumeric,

      /**
       * Property requires a boolean value. Note that setting DataTypeBoolean as the required type
       * means that the property also accepts string and numeric fields, as those may be convertible
       * to a boolean value (Eg "1.0" -> TRUE)
       */
      DataTypeBoolean,
    };

    /**
     * Constructs an empty property.
     */
    QgsPropertyDefinition() = default;

    /**
     * Constructor for QgsPropertyDefinition, using a standard property template.
     * \param name is used internally and should be a unique, alphanumeric string.
     * \param description can be any localised string describing what the property is used for.
     * \param type one of the predefined standard property template
     * \param origin The origin of the property
     * \param comment A free comment for the property
     */
    QgsPropertyDefinition( const QString &name, const QString &description, StandardPropertyTemplate type, const QString &origin = QString(), const QString &comment = QString() );

    /**
     * Constructor for custom QgsPropertyDefinitions.
     * \param name is used internally and should be a unique, alphanumeric string.
     * \param dataType the data type for the property
     * \param description can be any localised string describing what the property is used for.
     * \param helpText parameter should specify a descriptive string for users outlining the types
     * of value acceptable by the property (eg 'dashed' or 'solid' for a line style property).
     * \param origin The origin of the property
     * \param comment A free comment for the property
     */
    QgsPropertyDefinition( const QString &name, DataType dataType, const QString &description, const QString &helpText, const QString &origin = QString(), const QString &comment = QString() );

    /**
     * Returns the name of the property. This is used internally and should be a unique, alphanumeric string.
     */
    QString name() const { return mName; }

    /**
     * Sets the name of the property
     */
    void setName( const QString &name ) { mName = name; }

    /**
     * Returns the origin of the property. For example, a PAL property has an
     * origin set to "labeling" while a diagram property has an origin set to
     * "diagram".
     */
    QString origin() const { return mOrigin; }

    /**
     * Sets the origin of the property. For example, a PAL property has an
     * origin set to "labeling" while a diagram property has an origin set to
     * "diagram".
     */
    void setOrigin( const QString &origin ) { mOrigin = origin; }

    /**
     * Descriptive name of the property.
     */
    QString description() const { return mDescription; }

    /**
     * Returns the comment of the property
     */
    QString comment() const { return mComment; }

    /**
     * Sets comment of the property
     */
    void setComment( const QString &comment ) { mComment = comment; }

    /**
     * Helper text for using the property, including a description of the valid values for the property.
     */
    QString helpText() const { return mHelpText; }

    /**
     * Sets the data type
     */
    void setDataType( DataType type ) { mTypes = type; }

    /**
     * Returns the allowable field/value data type for the property.
     */
    DataType dataType() const { return mTypes; }

    /**
     * Returns the property's standard template, if applicable. Non standard
     * types will return the Custom template.
     */
    StandardPropertyTemplate standardTemplate() const { return mStandardType; }

    /**
     * Returns TRUE if the property is of a type which is compatible with property
     * override assistants.
     */
    bool supportsAssistant() const;

  private:

    QString mName;
    QString mDescription;
    DataType mTypes = DataTypeString;
    QString mHelpText;
    StandardPropertyTemplate mStandardType = Custom;
    QString mOrigin;
    QString mComment;

    static QString trString();
};


/**
 * \ingroup core
 * \class QgsProperty
 * \brief A store for object properties.
 *
 * QgsProperty objects are used for storing properties for objects, which can then be transformed to
 * a QVariant value by evaluating them against a supplied QgsExpressionContext. Multiple QgsProperty objects
 * can be grouped using a QgsPropertyCollection for easier bulk storage, retrieval and evaluation.
 *
 * QgsProperty objects are implicitly shared and can be inexpensively copied.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsProperty
{
  public:

    //! Property types
    enum Type
    {
      InvalidProperty, //!< Invalid (not set) property
      StaticProperty, //!< Static property (QgsStaticProperty)
      FieldBasedProperty, //!< Field based property (QgsFieldBasedProperty)
      ExpressionBasedProperty, //!< Expression based property (QgsExpressionBasedProperty)
    };

    /**
     * Constructor for a QgsAbstractProperty. The property will be set to an InvalidProperty type.
     */
    QgsProperty();

    virtual ~QgsProperty() = default;

    /**
     * Returns a new ExpressionBasedProperty created from the specified expression.
     */
    static QgsProperty fromExpression( const QString &expression, bool isActive = true );

    /**
     * Returns a new FieldBasedProperty created from the specified field name.
     */
    static QgsProperty fromField( const QString &fieldName, bool isActive = true );

    /**
     * Returns a new StaticProperty created from the specified value.
     */
    static QgsProperty fromValue( const QVariant &value, bool isActive = true );

    //! Copy constructor
    QgsProperty( const QgsProperty &other );

    QgsProperty &operator=( const QgsProperty &other );

    /**
     * Returns TRUE if the property is not an invalid type.
     */
    operator bool() const;

    bool operator==( const QgsProperty &other ) const;
    bool operator!=( const QgsProperty &other ) const;

    /**
     * Returns the property type.
     */
    Type propertyType() const;

    /**
     * Returns whether the property is currently active.
     * \see setActive()
     */
    bool isActive() const;

    /**
     * Sets whether the property is currently active.
     * \see isActive()
     */
    void setActive( bool active );

    /**
     * Sets the static value for the property. Calling this will
     * transform the property into an StaticProperty.
     * \see staticValue()
     */
    void setStaticValue( const QVariant &value );

    /**
     * Returns the current static value for the property. If the property
     * is not a StaticProperty this will return an invalid variant.
     * \see setStaticValue()
     */
    QVariant staticValue() const;

    /**
     * Sets the field name the property references. Calling this will
     * transform the property into an FieldBasedProperty.
     * \see field()
     */
    void setField( const QString &field );

    /**
     * Returns the current field name the property references. If the property
     * is not a FieldBasedProperty this will return an empty string.
     * \see setField()
     */
    QString field() const;

    /**
     * Sets the expression to use for the property value. Calling this will
     * transform the property into an ExpressionBasedProperty.
     * \see expressionString()
     */
    void setExpressionString( const QString &expression );

    /**
     * Returns the expression used for the property value. If the property
     * is not a ExpressionBasedProperty this will return an empty string.
     * \see setExpressionString()
     */
    QString expressionString() const;

    /**
     * Returns an expression string representing the state of the property, or an empty
     * string if the property could not be converted to an expression
     */
    QString asExpression() const;

    /**
     * Prepares the property against a specified expression context. Calling prepare before evaluating the
     * property multiple times allows precalculation of expensive setup tasks such as parsing expressions.
     * Returns TRUE if preparation was successful.
     */
    bool prepare( const QgsExpressionContext &context = QgsExpressionContext() ) const;

    /**
     * Returns the set of any fields referenced by the property for a specified
     * expression context.
     */
    QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const;

    /**
     * Returns TRUE if the property is set to a linked project color.
     *
     * \since QGIS 3.6
     */
    bool isProjectColor() const;

    /**
     * Calculates the current value of the property, including any transforms which are set for the property
     * \param context QgsExpressionContext to evaluate the property for. The variables and functions contained
     * in the expression context can be used to alter the calculated value for the property, so that a property
     * is able to respond to the current environment, layers and features within QGIS.
     * \param defaultValue default value to return if the property is not active or cannot be calculated
     * \param ok if specified, will be set to TRUE if conversion was successful
     * \returns calculated value for property
     * \see valueAsString()
     * \see valueAsColor()
     * \see valueAsDouble()
     * \see valueAsInt()
     * \see valueAsBool()
     */
    QVariant value( const QgsExpressionContext &context, const QVariant &defaultValue = QVariant(), bool *ok SIP_OUT = nullptr ) const;

    /**
     * Calculates the current value of the property and interprets it as a string.
     * \param context QgsExpressionContext to evaluate the property for.
     * \param defaultString default string to return if the property cannot be calculated as a string
     * \param ok if specified, will be set to TRUE if conversion was successful
     * \returns value parsed to string
     * \see value()
     * \see valueAsColor()
     * \see valueAsDouble()
     * \see valueAsInt()
     * \see valueAsBool()
     */
    QString valueAsString( const QgsExpressionContext &context, const QString &defaultString = QString(), bool *ok SIP_OUT = nullptr ) const;

    /**
     * Calculates the current value of the property and interprets it as a color.
     * \param context QgsExpressionContext to evaluate the property for.
     * \param defaultColor default color to return if the property cannot be calculated as a color
     * \param ok if specified, will be set to TRUE if conversion was successful
     * \returns value parsed to color
     * \see value()
     * \see valueAsString()
     * \see valueAsDouble()
     * \see valueAsInt()
     * \see valueAsBool()
     */
    QColor valueAsColor( const QgsExpressionContext &context, const QColor &defaultColor = QColor(), bool *ok SIP_OUT = nullptr ) const;

    /**
     * Calculates the current value of the property and interprets it as a double.
     * \param context QgsExpressionContext to evaluate the property for.
     * \param defaultValue default double to return if the property cannot be calculated as a double
     * \param ok if specified, will be set to TRUE if conversion was successful
     * \returns value parsed to double
     * \see value()
     * \see valueAsString()
     * \see valueAsColor()
     * \see valueAsInt()
     * \see valueAsBool()
     */
    double valueAsDouble( const QgsExpressionContext &context, double defaultValue = 0.0, bool *ok SIP_OUT = nullptr ) const;

    /**
     * Calculates the current value of the property and interprets it as an integer.
     * \param context QgsExpressionContext to evaluate the property for.
     * \param defaultValue default integer to return if the property cannot be calculated as an integer
     * \param ok if specified, will be set to TRUE if conversion was successful
     * \returns value parsed to integer
     * \see value()
     * \see valueAsString()
     * \see valueAsColor()
     * \see valueAsDouble()
     * \see valueAsBool()
     */
    int valueAsInt( const QgsExpressionContext &context, int defaultValue = 0, bool *ok SIP_OUT = nullptr ) const;

    /**
     * Calculates the current value of the property and interprets it as an boolean.
     * \param context QgsExpressionContext to evaluate the property for.
     * \param defaultValue default boolean to return if the property cannot be calculated as an boolean
     * \param ok if specified, will be set to TRUE if conversion was successful
     * \returns value parsed to boolean
     * \see value()
     * \see valueAsString()
     * \see valueAsColor()
     * \see valueAsDouble()
     * \see valueAsInt()
     */
    bool valueAsBool( const QgsExpressionContext &context, bool defaultValue = false, bool *ok SIP_OUT = nullptr ) const;

    /**
     * Saves this property to a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::writeVariant to save it to an XML document.
     *
     * \see loadVariant()
     */
    QVariant toVariant() const;

    /**
     * Loads this property from a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::readVariant to load it from an XML document.
     *
     * \see toVariant()
     */
    bool loadVariant( const QVariant &property );

    /**
     * Sets an optional transformer to use for manipulating the calculated values for the property.
     * \param transformer transformer to install. Ownership is transferred to the property, and any
     * existing transformer will be deleted. Set to NULLPTR to remove an existing transformer.
     * \see transformer()
     */
    void setTransformer( QgsPropertyTransformer *transformer SIP_TRANSFER );

    /**
     * Returns the existing transformer used for manipulating the calculated values for the property, if set.
     * \see setTransformer()
     */
    const QgsPropertyTransformer *transformer() const;

    /**
     * Attempts to convert an existing expression based property to a base expression with
     * corresponding transformer. Returns TRUE if conversion was successful. Note that
     * calling this method requires multiple parsing of expressions, so it should only
     * be called in non-performance critical code.
     */
    bool convertToTransformer();

    //! Allows direct construction of QVariants from properties.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

  private:

    mutable QExplicitlySharedDataPointer<QgsPropertyPrivate> d;

    /**
     * Calculates the current value of the property, before any transformations or
     * conversions are applied.
     */
    QVariant propertyValue( const QgsExpressionContext &context, const QVariant &defaultValue = QVariant(), bool *ok = nullptr ) const;

};

Q_DECLARE_METATYPE( QgsProperty )

#endif // QGSPROPERTY_H
