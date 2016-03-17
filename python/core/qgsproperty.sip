/** \ingroup core
 * \class QgsAbstractProperty
 * \brief Abstract base class for properties.
 *
 * QgsAbstractProperty objects are used for storing properties for objects, which can then be evaluated using
 * custom logic by evaluating them against a supplied QgsExpressionContext. Multiple QgsAbstractProperty objects
 * can be grouped using a QgsPropertyCollection for easier bulk storage, retrieval and evaluation.
 * \note Added in version 2.16
 */

class QgsAbstractProperty
{
%TypeHeaderCode
#include <qgsproperty.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsStaticProperty*>(sipCpp) != NULL)
	sipType = sipType_QgsStaticProperty;
  else if (dynamic_cast<QgsFieldBasedProperty*>(sipCpp) != NULL)
	sipType = sipType_QgsFieldBasedProperty;
  else if (dynamic_cast<QgsExpressionBasedProperty*>(sipCpp) != NULL)
	sipType = sipType_QgsExpressionBasedProperty;
  else
	sipType = sipType_QgsAbstractProperty;
%End

  public:

    //! Property types
    enum Type
    {
      StaticProperty, /*!< Static property (QgsStaticProperty) */
      FieldBasedProperty, /*!< Field based property (QgsFieldBasedProperty) */
      ExpressionBasedProperty, /*!< Expression based property (QgsExpressionBasedProperty) */
    };

    /** Factory method for creating a new property of the specified type.
     * @param type property type to create
     */
    static QgsAbstractProperty* create( Type type ) /Factory/;

    /** Constructor for a QgsAbstractProperty.
     * @param active whether the property is initially active
     */
    QgsAbstractProperty( bool active = true );

    //! Copy constructor
    QgsAbstractProperty( const QgsAbstractProperty& other );

    virtual ~QgsAbstractProperty();

    /** Returns the property type.
     */
    virtual Type propertyType() const = 0;

    /** Returns a clone of the property.
     */
    virtual QgsAbstractProperty* clone() = 0 /Factory/;

    /** Returns whether the property is currently active.
     * @see setActive()
     */
    bool isActive() const;

    /** Sets whether the property is currently active.
     * @param active set to true to activate the property
     * @see isActive()
     */
    void setActive( bool active );

    /** Returns the set of any fields referenced by the property.
     * @param context expression context the property will be evaluated against.
     */
    virtual QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    /** Calculates the current value of the property, including any transforms which are set for the property
     * @param context QgsExpressionContext to evaluate the property for. The variables and functions contained
     * in the expression context can be used to alter the calculated value for the property, so that a property
     * is able to respond to the current environment, layers and features within QGIS.
     * @param defaultValue default value to return if the property is not active or cannot be calculated
     * @returns calculated value for property
     * @see valueAsColor()
     * @see valueAsDouble()
     * @see valueAsInt()
     */
    QVariant value( const QgsExpressionContext& context, const QVariant& defaultValue = QVariant() ) const;

    /** Calculates the current value of the property and interprets it as a color.
     * @param context QgsExpressionContext to evaluate the property for.
     * @param defaultColor default color to return if the property cannot be calculated as a color
     * @returns value parsed to color
     * @see value()
     * @see valueAsDouble()
     * @see valueAsInt()
     */
    QColor valueAsColor( const QgsExpressionContext& context, const QColor& defaultColor = QColor() ) const;

    /** Calculates the current value of the property and interprets it as a double.
     * @param context QgsExpressionContext to evaluate the property for.
     * @param defaultValue default double to return if the property cannot be calculated as a double
     * @returns value parsed to double
     * @see value()
     * @see valueAsInt()
     * @see valueAsColor()
     */
    double valueAsDouble( const QgsExpressionContext& context, double defaultValue = 0.0 ) const;

    /** Calculates the current value of the property and interprets it as an integer.
     * @param context QgsExpressionContext to evaluate the property for.
     * @param defaultValue default integer to return if the property cannot be calculated as an integer
     * @returns value parsed to integer
     * @see value()
     * @see valueAsDouble()
     * @see valueAsColor()
     */
    int valueAsInt( const QgsExpressionContext& context, int defaultValue = 0 ) const;

    /** Writes the current state of the property into an XML element
     * @param propertyElem destination element for the property's state
     * @param doc DOM document
     * @see readXML()
    */
    virtual bool writeXML( QDomElement& propertyElem, QDomDocument& doc ) const;

    /** Reads property state from an XML element.
     * @param propertyElem source DOM element for property's state
     * @param doc DOM document
     * @see writeXML()
    */
    virtual bool readXML( const QDomElement& propertyElem, const QDomDocument& doc );

    /** Sets an optional transformer to use for manipulating the calculated values for the property.
     * @param transformer transformer to install. Ownership is transferred to the property, and any
     * existing transformer will be deleted. Set to null to remove an existing transformer.
     * @see transformer()
     */
    void setTransformer( QgsPropertyTransformer* transformer /Transfer/ );

    /** Returns the existing transformer used for manipulating the calculated values for the property, if set.
     * @see setTransformer()
     */
    const QgsPropertyTransformer* transformer() const;

  protected:

    /** Calculates the current value of the property. Derived classes must implement this to return their
     * current value.
     * @param context QgsExpressionContext to evaluate the property for. The variables and functions contained
     * in the expression context can be used to alter the calculated value for the property, so that a property
     * is able to respond to the current environment, layers and features within QGIS.
     * @param defaultValue default value to return if the property is not active or cannot be calculated
     * @returns calculated value for property
     */
    virtual QVariant propertyValue( const QgsExpressionContext& context, const QVariant& defaultValue = QVariant() ) const = 0;

};


/** \ingroup core
 * \class QgsStaticProperty
 * \brief Simple QgsAbstractProperty subclass which returns a static value, regardless of the context it is evaluated within.
 * \note Added in version 2.16
 */

class QgsStaticProperty : QgsAbstractProperty
{
%TypeHeaderCode
#include <qgsproperty.h>
%End

  public:

    /** Constructor for QgsStaticProperty.
     * @param value initial static value to use for property
     * @param isActive whether the property is intially active
     */
    QgsStaticProperty( const QVariant& value = QVariant(), bool isActive = true );

    virtual Type propertyType() const;

    virtual QgsStaticProperty* clone() /Factory/;

    /** Sets the static value for the property.
     * @param value property value
     * @see staticValue()
     */
    void setStaticValue( const QVariant& value );

    /** Returns the current static value for the property.
     * @see setStaticValue()
     */
    QVariant staticValue() const;

    bool writeXML( QDomElement& propertyElem, QDomDocument& doc ) const;
    bool readXML( const QDomElement& propertyElem, const QDomDocument& doc );

  protected:

    virtual QVariant propertyValue( const QgsExpressionContext& context, const QVariant& defaultValue = QVariant() ) const;

};


/** \ingroup core
 * \class QgsFieldBasedProperty
 * \brief QgsAbstractProperty subclass which references the value of a field from a feature.
 * \note Added in version 2.16
 */

class QgsFieldBasedProperty : QgsAbstractProperty
{
%TypeHeaderCode
#include <qgsproperty.h>
%End

  public:

    /** Constructor for QgsFieldBasedProperty.
     * @param field field name
     * @param isActive whether the property is intially active
     */
    QgsFieldBasedProperty( const QString& field = QString(), bool isActive = false );

    virtual Type propertyType() const;

    virtual QgsFieldBasedProperty* clone() /Factory/;

    /** Sets the field name the property references.
     * @param field field name
     * @see field()
     */
    void setField( const QString& field );

    /** Returns the current field name the property references.
     * @see setField()
     */
    QString field() const;

    virtual QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    bool writeXML( QDomElement& propertyElem, QDomDocument& doc ) const;
    bool readXML( const QDomElement& propertyElem, const QDomDocument& doc );

  protected:

    virtual QVariant propertyValue( const QgsExpressionContext& context, const QVariant& defaultValue = QVariant() ) const;

};

/** \ingroup core
 * \class QgsExpressionBasedProperty
 * \brief QgsAbstractProperty subclass which takes its value from a QgsExpression result.
 * \note Added in version 2.16
 */

class QgsExpressionBasedProperty : QgsAbstractProperty
{
%TypeHeaderCode
#include <qgsproperty.h>
%End

  public:

    /** Constructor for QgsExpressionBasedProperty.
     * @param expression expression string
     * @param isActive whether the property is intially active
     */
    QgsExpressionBasedProperty( const QString& expression = QString(), bool isActive = false );

    virtual Type propertyType() const;

    virtual QgsExpressionBasedProperty* clone() /Factory/;

    /** Sets the expression to use for the property value.
     * @param expression expression string
     * @see expressionString()
     */
    void setExpressionString( const QString& expression );

    /** Returns the expression to use for the property value.
     * @see setExpressionString()
     */
    QString expressionString() const;

    virtual QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    bool writeXML( QDomElement& propertyElem, QDomDocument& doc ) const;
    bool readXML( const QDomElement& propertyElem, const QDomDocument& doc );

  protected:

    virtual QVariant propertyValue( const QgsExpressionContext& context, const QVariant& defaultValue = QVariant() ) const;

};


/** \ingroup core
 * \class QgsPropertyTransformer
 * \brief Abstract base class for objects which transform the calculated value of a property.
 * Possible uses include transformers which map a value into a scaled size or color from a gradient.
 * \note Added in version 2.16
 */

class QgsPropertyTransformer
{
%TypeHeaderCode
#include <qgsproperty.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsSizeScaleTransformer*>(sipCpp) != NULL)
	sipType = sipType_QgsSizeScaleTransformer;
  else if (dynamic_cast<QgsColorRampTransformer*>(sipCpp) != NULL)
	sipType = sipType_QgsColorRampTransformer;
  else
	sipType = sipType_QgsPropertyTransformer;
%End

  public:

    //! Transformer types
    enum Type
    {
      SizeScaleTransformer, /*!< Size scaling transformer (QgsSizeScaleTransformer) */
      ColorRampTransformer, /*!< Color ramp transformer (QgsColorRampTransformer) */
    };

    /** Factory method for creating a new property transformer of the specified type.
     * @param type transformer type to create
     */
    static QgsPropertyTransformer* create( Type type ) /Factory/;

    /** Constructor for QgsPropertyTransformer
     * @param minValue minimum expected value from source property
     * @param maxValue maximum expected value from source property
     */
    QgsPropertyTransformer( double minValue = 0.0, double maxValue = 1.0 );

    virtual ~QgsPropertyTransformer();

    /** Returns the transformer type.
     */
    virtual Type transformerType() const = 0;

    /** Returns a clone of the transformer.
     */
    virtual QgsPropertyTransformer* clone() = 0 /Factory/;

    /** Reads transformer's state from an XML element.
     * @param transformerElem source DOM element for transformer's state
     * @param doc DOM document
     * @see writeXML()
    */
    virtual bool readXML( const QDomElement& transformerElem, const QDomDocument& doc );

    /** Writes the current state of the transformer into an XML element
     * @param transformerElem destination element for the transformer's state
     * @param doc DOM document
     * @see readXML()
    */
    virtual bool writeXML( QDomElement& transformerElem, QDomDocument& doc ) const;

    /** Returns the minimum value expected by the transformer.
     * @see maxValue()
     * @see setMinValue()
     */
    double minValue() const;

    /** Sets the minimum value expected by the transformer.
     * @param min minimum value
     * @see setMaxValue()
     * @see minValue()
     */
    void setMinValue( double min );

    /** Returns the maximum value expected by the transformer.
     * @see minValue()
     * @see setMaxValue()
     */
    double maxValue() const;

    /** Sets the maximum value expected by the transformer.
     * @param max maximum value
     * @see setMinValue()
     * @see maxValue()
     */
    void setMaxValue( double max );

    /** Calculates the transform of a value. Derived classes must implement this to perform their transformations
     * on input values
     * @param context expression context
     * @param value input value to transform
     */
    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const = 0;

};

/** \ingroup core
 * \class QgsSizeScaleTransformer
 * \brief QgsPropertyTransformer subclass for scaling a value into a size according to various
 * scaling methods.
 * \note Added in version 2.16
 */

class QgsSizeScaleTransformer : QgsPropertyTransformer
{
%TypeHeaderCode
#include <qgsproperty.h>
%End
  public:

    //! Size scaling methods
    enum ScaleType
    {
      Linear, /*!< Linear scaling */
      Area, /*!< Area based scaling */
      Flannery, /*!< Flannery scaling method */
      Exponential, /*!< Scale using set exponent */
    };

    /** Constructor for QgsSizeScaleTransformer.
     * @param type scaling type
     * @param minValue minimum expected value
     * @param maxValue maximum expected value
     * @param minSize minimum size to return
     * @param maxSize maximum size to return
     * @param nullSize size to return for null values
     * @param exponent exponent for Exponential scaling method
     */
    QgsSizeScaleTransformer( ScaleType type = Linear,
                             double minValue = 0.0,
                             double maxValue = 1.0,
                             double minSize = 0.0,
                             double maxSize = 1.0,
                             double nullSize = 0.0,
                             double exponent = 1.0 );

    virtual Type transformerType() const;
    virtual QgsSizeScaleTransformer* clone() /Factory/;
    virtual bool writeXML( QDomElement& transformerElem, QDomDocument& doc ) const;
    virtual bool readXML( const QDomElement& transformerElem, const QDomDocument& doc );
    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const;

    /** Calculates the size corresponding to a specific value.
     * @param value value to calculate size for
     * @returns calculated size using size scale transformer's parameters and type
     */
    double size( double value ) const;

    /** Returns the minimum calculated size.
     * @see setMinSize()
     * @see maxSize()
     */
    double minSize() const;

    /** Sets the minimum calculated size.
     * @param size minimum size
     * @see minSize()
     * @see setMaxSize()
     */
    void setMinSize( double size );

    /** Returns the maximum calculated size.
     * @see minSize()
     */
    double maxSize() const;

    /** Sets the maximum calculated size.
     * @param size maximum size
     * @see maxSize()
     * @see setMinSize()
     */
    void setMaxSize( double size );

    /** Returns the size value when an expression evaluates to NULL.
     * @see setNullSize()
     */
    double nullSize() const;

    /** Sets the size value for when an expression evaluates to NULL.
     * @param size null size
     * @see nullSize()
     */
    void setNullSize( double size );

    /** Returns the exponent for an exponential expression.
     * @see setExponent()
     * @see type()
     */
    double exponent() const;

    /** Sets the exponent for an exponential expression.
     * @param exponent exponent
     * @see exponent()
     */
    void setExponent( double exponent );

    /** Returns the size transformer's scaling type (the method used to calculate
     * the size from a value).
     * @see setType()
     */
    ScaleType type() const;

    /** Sets the size transformer's scaling type (the method used to calculate
     * the size from a value).
     * @param type scale type
     * @see type()
     */
    void setType( ScaleType type );

};

/** \ingroup core
 * \class QgsColorRampTransformer
 * \brief QgsPropertyTransformer subclass for transforming a numeric value into a color from a
 * color ramp.
 * \note Added in version 2.16
 */

class QgsColorRampTransformer : QgsPropertyTransformer
{
%TypeHeaderCode
#include <qgsproperty.h>
%End
  public:

    /** Constructor for QgsColorRampTransformer.
     * @param minValue minimum expected value
     * @param maxValue maximum expected value
     * @param ramp source color ramp. Ownership is transferred to the transformer.
     * @param nullColor color to return for null values
     */
    QgsColorRampTransformer( double minValue = 0.0,
                             double maxValue = 1.0,
                             QgsColorRamp* ramp /Transfer/ = 0,
                             const QColor& nullColor = QColor( 0, 0, 0, 0 ) );

    //! Copy constructor
    QgsColorRampTransformer( const QgsColorRampTransformer& other );

    //QgsColorRampTransformer& operator=( const QgsColorRampTransformer& other );

    ~QgsColorRampTransformer();

    virtual Type transformerType() const;
    virtual QgsColorRampTransformer* clone() /Factory/;
    virtual bool writeXML( QDomElement& transformerElem, QDomDocument& doc ) const;
    virtual bool readXML( const QDomElement& transformerElem, const QDomDocument& doc );
    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const;

    /** Calculates the color corresponding to a specific value.
     * @param value value to calculate color for
     * @returns calculated color using transformer's parameters and type
     */
    QColor color( double value ) const;

    /** Returns the color ramp used for calculating property colors.
     * @returns color ramp
     * @see setColorRamp()
     */
    QgsColorRamp* colorRamp() const;

    /** Sets the color ramp to use for calculating property colors.
     * @param ramp color ramp, ownership of ramp is transferred to the transformer.
     * @see colorRamp()
     */
    void setColorRamp( QgsColorRamp* ramp /Transfer/ );

    /** Returns the color corresponding to a null value.
     * @see setNullColor()
     */
    QColor nullColor() const;

    /** Sets the color corresponding to a null value.
     * @param color null color
     * @see nullSize()
     */
    void setNullColor( const QColor& color );

};

/** \ingroup core
 * \class QgsPropertyCollection
 * \brief A grouped map of multiple QgsAbstractProperty objects, each referenced by a integer key value.
 *
 * Properties within a collection are referenced by an integer key. This is done to avoid the cost of
 * string creation and comparisons which would be required by a string key. The intended use case is that
 * a context specific enum is cast to int and used for the key value.
 * \note Added in version 2.16
 */

class QgsPropertyCollection
{
%TypeHeaderCode
#include <qgsproperty.h>
%End

  public:

    /** Constructor for QgsPropertyCollection
     * @param name collection name
     */
    QgsPropertyCollection( const QString& name = QString() );

    ~QgsPropertyCollection();

    //! Copy constructor
    QgsPropertyCollection( const QgsPropertyCollection& other );

    //QgsPropertyCollection& operator=( const QgsPropertyCollection& other );

    /** Returns the name of the property collection.
     */
    QString name() const;

    /** Returns the number of properties contained within the collection.
     */
    int count() const;

    /** Returns a list of property keys contained within the collection.
     */
    QList<int> propertyKeys() const;

    /** Removes all properties from the collection.
     */
    void clear();

    /** Adds a property to the collection and takes ownership of it.
     * @param key integer key for property. Any existing property with the same key will be deleted
     * and replaced by this property. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @param property property to add. Ownership is transferred to the collection. Setting a property
     * to null will remove the property from the collection.
     */
    void setProperty( int key, QgsAbstractProperty* property /Transfer/ );

    /** Convience method, creates a QgsStaticProperty and stores it within the collection.
     * @param key integer key for property. Any existing property with the same key will be deleted
     * and replaced by this property. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @param value static value for property
     */
    void setProperty( int key, const QVariant& value );

    /** Returns true if the collection contains a property with the specified key.
     * @param key integer key for property. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @see property()
     */
    bool hasProperty( int key ) const;

    /** Returns a matching property from the collection, if one exists.
     * @param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @returns matching property, or null if no matching, active property found.
     * @see hasProperty()
     */
    QgsAbstractProperty* property( int key );

    /** Returns the calculated value of the property with the specified key from within the collection.
     * @param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @param context expression context to evaluate property against
     * @param defaultValue default value to return if no matching, active property found or if the property value
     * cannot be calculated
     * @returns calculated property value, or default value if property could not be evaluated
     * @see valueAsColor()
     * @see valueAsDouble()
     * @see valueAsInt()
     */
    QVariant value( int key, const QgsExpressionContext& context, const QVariant& defaultValue = QVariant() ) const;

    /** Calculates the current value of the property with the specified key and interprets it as a color.
     * @param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @param context QgsExpressionContext to evaluate the property for.
     * @param defaultColor default color to return if the property cannot be calculated as a color
     * @returns value parsed to color
     * @see value()
     * @see valueAsDouble()
     * @see valueAsInteger()
     */
    QColor valueAsColor( int key, const QgsExpressionContext& context, const QColor& defaultColor = QColor() ) const;

    /** Calculates the current value of the property with the specified key and interprets it as a double.
     * @param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @param context QgsExpressionContext to evaluate the property for.
     * @param defaultValue default double to return if the property cannot be calculated as a double
     * @returns value parsed to double
     * @see value()
     * @see valueAsInteger()
     * @see valueAsColor()
     */
    double valueAsDouble( int key, const QgsExpressionContext& context, double defaultValue = 0.0 ) const;

    /** Calculates the current value of the property with the specified key and interprets it as an integer.
     * @param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @param context QgsExpressionContext to evaluate the property for.
     * @param defaultValue default integer to return if the property cannot be calculated as a integer
     * @returns value parsed to integer
     * @see value()
     * @see valueAsDouble()
     * @see valueAsColor()
     */
    int valueAsInt( int key, const QgsExpressionContext& context, int defaultValue = 0 ) const;

    /** Returns the set of any fields referenced by the active properties from the collection.
     * @param context expression context the properties will be evaluated against.
     */
    QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    /** Returns true if the collection contains an active property with the specified key.
     * @param key integer key for property to test. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     */
    bool isActive( int key ) const;

    /** Returns true if the collection has any active properties, or false if all properties
     * within the collection are deactived.
     * @see hasActiveDynamicProperties()
     */
    bool hasActiveProperties() const;

    /** Returns true if the collection has any active, non-static properties, or false if either all non-static properties
     * within the collection are deactived or if the collection only contains static properties.
     * @see hasActiveProperties()
     */
    bool hasActiveDynamicProperties() const;

    /** Writes the current state of the property collection into an XML element
     * @param collectionElem destination element for the property collection's state
     * @param doc DOM document
     * @param propertyNameMap map of key integers to string to use for property name in XML elements. This map is used
     * to avoid writing the raw integer key values to XML, for readability and future-proofness.
     * @see readXML()
    */
    bool writeXML( QDomElement& collectionElem, QDomDocument& doc, const QMap< int, QString >& propertyNameMap ) const;

    /** Reads property collection state from an XML element.
     * @param collectionElem source DOM element for property collection's state
     * @param doc DOM document
     * @param propertyNameMap map of key integers to string to use for property name in XML elements. This map must match
     * the propertyNameMap specified when writeXML() was called.
     * @see writeXML()
    */
    bool readXML( const QDomElement& collectionElem, const QDomDocument& doc, const QMap<int, QString> &propertyNameMap );

};


/** \ingroup core
 * \class QgsPropertyCollectionStack
 * \brief An ordered stack of QgsPropertyCollection containers, where collections added later to the stack will take
 * priority over earlier collections.
 * \note Added in version 2.16
 */

class QgsPropertyCollectionStack
{
%TypeHeaderCode
#include <qgsproperty.h>
%End

  public:

    QgsPropertyCollectionStack();

    ~QgsPropertyCollectionStack();

    //! Copy constructor
    QgsPropertyCollectionStack( const QgsPropertyCollectionStack& other );

    //QgsPropertyCollectionStack& operator=( const QgsPropertyCollectionStack& other );

    /** Returns the number of collections contained within the stack.
     */
    int count() const;

    /** Removes all collections from the stack.
     */
    void clear();

    /** Appends a collection to the end of the stack, and transfers ownership of the collection to the stack. Properties
     * from the newly added collection will take priority over any existing properties with the same name.
     * @param collection collection to append. Ownership is transferred to the stack.
     */
    void appendCollection( QgsPropertyCollection* collection /Transfer/ );

    /** Returns the collection at the corresponding index from the stack.
     * @param index position of collection, 0 based
     * @returns collection if one exists at the specified index
     */
    QgsPropertyCollection* at( int index );

    /** Returns the collection at the corresponding index from the stack.
     * @param index position of collection, 0 based
     * @returns collection if one exists at the specified index
     */
    //const QgsPropertyCollection* at( int index ) const;

    /** Returns the first collection with a matching name from the stack.
     * @param name name of collection to find
     * @returns collection if one exists with the specified name
     */
    QgsPropertyCollection* collection( const QString& name );

    /** Returns true if the collection has any active properties, or false if all properties
     * within the collection are deactived.
     * @see hasActiveProperty()
     * @see hasActiveDynamicProperties()
     */
    bool hasActiveProperties() const;

    /** Returns true if the collection has any active, non-static properties, or false if either all non-static properties
     * within the collection are deactived or if the collection only contains static properties.
     * @see hasActiveProperties()
     */
    bool hasActiveDynamicProperties() const;

    /** Returns true if the stack contains an active property with the specified key.
     * @param key integer key for property to test. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @see hasActiveProperties()
     */
    bool hasActiveProperty( int key ) const;

    /** Returns the highest priority property with a matching key from within the stack.
     * @param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @returns matching property, or null if no matching, active property found.
     * @see hasActiveProperty()
     */
    QgsAbstractProperty* property( int key );

    /** Returns the calculated value of the highest priority property with the specified key from within the stack.
     * @param key integer key for property to calculate. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * @param context expression context to evaluate property against
     * @param defaultValue default value to return if no matching, active property found or if the property value
     * cannot be calculated
     * @returns calculated property value, or default value if property could not be evaluated
     */
    QVariant value( int key, const QgsExpressionContext& context, const QVariant& defaultValue = QVariant() ) const;

    /** Returns the set of any fields referenced by the active properties from the stack.
     * @param context expression context the properties will be evaluated against.
     */
    QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;

};

