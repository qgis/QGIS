/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspropertycollection.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



typedef QMap< int, QgsPropertyDefinition > QgsPropertiesDefinition;


class QgsAbstractPropertyCollection
{
%Docstring
 Abstract base class for QgsPropertyCollection like objects.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspropertycollection.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsPropertyCollection *>( sipCpp ) )
      sipType = sipType_QgsPropertyCollection;
    else if ( dynamic_cast<QgsPropertyCollectionStack *>( sipCpp ) )
      sipType = sipType_QgsPropertyCollectionStack;
    else
      sipType = sipType_QgsAbstractPropertyCollection;
%End
  public:

    QgsAbstractPropertyCollection( const QString &name = QString() );
%Docstring
Constructor for QgsAbstractPropertyCollection. The name
parameter should be set to a descriptive name for the collection.
%End

    virtual ~QgsAbstractPropertyCollection();

    QString name() const;
%Docstring
Returns the descriptive name of the property collection.

.. seealso:: :py:func:`setName`
%End

    void setName( const QString &name );
%Docstring
Sets the descriptive name for the property collection.

.. seealso:: :py:func:`name`
%End

    virtual QSet<int> propertyKeys() const = 0;
%Docstring
Returns a list of property keys contained within the collection.
%End

    virtual void clear() = 0;
%Docstring
Removes all properties from the collection.
%End

    virtual bool hasProperty( int key ) const = 0;
%Docstring
Returns true if the collection contains a property with the specified key.

:param key: integer key for property. The intended use case is that a context specific enum is cast to
int and used for the key value.

.. seealso:: :py:func:`property`
%End

    virtual QgsProperty property( int key ) const = 0;
%Docstring
Returns a matching property from the collection, if one exists.

:param key: integer key for property to return. The intended use case is that a context specific enum is cast to
int and used for the key value.

:return: matching property, or null if no matching, active property found.

.. seealso:: :py:func:`hasProperty`
%End

    virtual QVariant value( int key, const QgsExpressionContext &context, const QVariant &defaultValue = QVariant() ) const = 0;
%Docstring
Returns the calculated value of the property with the specified key from within the collection.

:param key: integer key for property to return. The intended use case is that a context specific enum is cast to
int and used for the key value.
:param context: expression context to evaluate property against
:param defaultValue: default value to return if no matching, active property found or if the property value
cannot be calculated

:return: calculated property value, or default value if property could not be evaluated

.. seealso:: :py:func:`valueAsString`

.. seealso:: :py:func:`valueAsColor`

.. seealso:: :py:func:`valueAsDouble`

.. seealso:: :py:func:`valueAsInt`

.. seealso:: :py:func:`valueAsBool`
%End

    QString valueAsString( int key, const QgsExpressionContext &context, const QString &defaultString = QString(), bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property with the specified key and interprets it as a string.

:param key: integer key for property to return. The intended use case is that a context specific enum is cast to
int and used for the key value.
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

    QColor valueAsColor( int key, const QgsExpressionContext &context, const QColor &defaultColor = QColor(), bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property with the specified key and interprets it as a color.

:param key: integer key for property to return. The intended use case is that a context specific enum is cast to
int and used for the key value.
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

    double valueAsDouble( int key, const QgsExpressionContext &context, double defaultValue = 0.0, bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property with the specified key and interprets it as a double.

:param key: integer key for property to return. The intended use case is that a context specific enum is cast to
int and used for the key value.
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

    int valueAsInt( int key, const QgsExpressionContext &context, int defaultValue = 0, bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property with the specified key and interprets it as an integer.

:param key: integer key for property to return. The intended use case is that a context specific enum is cast to
int and used for the key value.
:param context: QgsExpressionContext to evaluate the property for.
:param defaultValue: default integer to return if the property cannot be calculated as a integer
:param ok: if specified, will be set to true if conversion was successful

:return: value parsed to integer

.. seealso:: :py:func:`value`

.. seealso:: :py:func:`valueAsString`

.. seealso:: :py:func:`valueAsColor`

.. seealso:: :py:func:`valueAsDouble`

.. seealso:: :py:func:`valueAsBool`
%End

    bool valueAsBool( int key, const QgsExpressionContext &context, bool defaultValue = false, bool *ok /Out/ = 0 ) const;
%Docstring
Calculates the current value of the property with the specified key and interprets it as an boolean.

:param key: integer key for property to return. The intended use case is that a context specific enum is cast to
int and used for the key value.
:param context: QgsExpressionContext to evaluate the property for.
:param defaultValue: default boolean to return if the property cannot be calculated as a boolean
:param ok: if specified, will be set to true if conversion was successful

:return: value parsed to bool

.. seealso:: :py:func:`value`

.. seealso:: :py:func:`valueAsString`

.. seealso:: :py:func:`valueAsColor`

.. seealso:: :py:func:`valueAsDouble`

.. seealso:: :py:func:`valueAsInt`
%End

    virtual bool prepare( const QgsExpressionContext &context = QgsExpressionContext() ) const = 0;
%Docstring
Prepares the collection against a specified expression context. Calling prepare before evaluating the
collection's properties multiple times allows precalculation of expensive setup tasks such as parsing expressions.
Returns true if preparation was successful.
%End

    virtual QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const = 0;
%Docstring
Returns the set of any fields referenced by the active properties from the collection.

:param context: expression context the properties will be evaluated against.
%End

    virtual bool isActive( int key ) const = 0;
%Docstring
Returns true if the collection contains an active property with the specified key.

:param key: integer key for property to test. The intended use case is that a context specific enum is cast to
int and used for the key value.
%End

    virtual bool hasActiveProperties() const = 0;
%Docstring
Returns true if the collection has any active properties, or false if all properties
within the collection are deactivated.

.. seealso:: :py:func:`hasDynamicProperties`
%End

    virtual bool hasDynamicProperties() const = 0;
%Docstring
Returns true if the collection has any active, non-static properties, or false if either all non-static properties
within the collection are deactivated or if the collection only contains static properties.

.. seealso:: :py:func:`hasActiveProperties`
%End

    virtual bool writeXml( QDomElement &collectionElem, const QgsPropertiesDefinition &definitions ) const;
%Docstring
Writes the current state of the property collection into an XML element

:param collectionElem: destination element for the property collection's state
:param definitions: property definitions

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &collectionElem, const QgsPropertiesDefinition &definitions );
%Docstring
Reads property collection state from an XML element.

:param collectionElem: source DOM element for property collection's state
:param definitions: property definitions

.. seealso:: :py:func:`writeXml`
%End

    virtual QVariant toVariant( const QgsPropertiesDefinition &definitions ) const = 0;
%Docstring
Saves this property collection to a QVariantMap, wrapped in a QVariant.
You can use QgsXmlUtils.writeVariant to save it to an XML document.

.. seealso:: :py:func:`loadVariant`
%End

    virtual bool loadVariant( const QVariant &configuration, const QgsPropertiesDefinition &definitions ) = 0;
%Docstring
Loads this property collection from a QVariantMap, wrapped in a QVariant.
You can use QgsXmlUtils.readVariant to save it to an XML document.

.. seealso:: :py:func:`toVariant`
%End

};


class QgsPropertyCollection : QgsAbstractPropertyCollection
{
%Docstring
 A grouped map of multiple QgsProperty objects, each referenced by a integer key value.

Properties within a collection are referenced by an integer key. This is done to avoid the cost of
string creation and comparisons which would be required by a string key. The intended use case is that
a context specific enum is cast to int and used for the key value.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspropertycollection.h"
%End
  public:

    QgsPropertyCollection( const QString &name = QString() );
%Docstring
Constructor for QgsPropertyCollection

:param name: collection name
%End

    QgsPropertyCollection( const QgsPropertyCollection &other );
%Docstring
Copy constructor.
%End


    int count() const;
%Docstring
Returns the number of properties contained within the collection.
%End

    virtual QSet<int> propertyKeys() const;

    virtual void clear();

    virtual bool hasProperty( int key ) const;


    virtual QgsProperty &property( int key );
%Docstring
Returns a reference to a matching property from the collection, if one exists.

:param key: integer key for property to return. The intended use case is that a context specific enum is cast to
int and used for the key value.

:return: matching property, or null if no matching, active property found.

.. seealso:: :py:func:`hasProperty`
%End

    virtual QVariant value( int key, const QgsExpressionContext &context, const QVariant &defaultValue = QVariant() ) const;

    virtual bool prepare( const QgsExpressionContext &context = QgsExpressionContext() ) const;

    virtual QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const;

    virtual bool isActive( int key ) const;

    virtual bool hasActiveProperties() const;

    virtual bool hasDynamicProperties() const;


    virtual QVariant toVariant( const QgsPropertiesDefinition &definitions ) const;

    virtual bool loadVariant( const QVariant &configuration, const QgsPropertiesDefinition &definitions );


    void setProperty( int key, const QgsProperty &property );
%Docstring
Adds a property to the collection and takes ownership of it.

:param key: integer key for property. Any existing property with the same key will be removed
and replaced by this property. The intended use case is that a context specific enum is cast to
int and used for the key value.
:param property: property to add. Ownership is transferred to the collection. Setting an invalid property
will remove the property from the collection.
%End

    void setProperty( int key, const QVariant &value );
%Docstring
Convience method, creates a QgsStaticProperty and stores it within the collection.

:param key: integer key for property. Any existing property with the same key will be deleted
and replaced by this property. The intended use case is that a context specific enum is cast to
int and used for the key value.
:param value: static value for property
%End

};



class QgsPropertyCollectionStack : QgsAbstractPropertyCollection
{
%Docstring
 An ordered stack of QgsPropertyCollection containers, where collections added later to the stack will take
priority over earlier collections.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspropertycollection.h"
%End
  public:

    QgsPropertyCollectionStack();
%Docstring
Constructor for QgsPropertyCollectionStack.
%End

    ~QgsPropertyCollectionStack();

    QgsPropertyCollectionStack( const QgsPropertyCollectionStack &other );
%Docstring
Copy constructor
%End


    int count() const;
%Docstring
Returns the number of collections contained within the stack.
%End

    virtual void clear();

%Docstring
Removes all collections from the stack.
%End

    void appendCollection( QgsPropertyCollection *collection /Transfer/ );
%Docstring
Appends a collection to the end of the stack, and transfers ownership of the collection to the stack. Properties
from the newly added collection will take priority over any existing properties with the same name.

:param collection: collection to append. Ownership is transferred to the stack.
%End

    QgsPropertyCollection *at( int index );
%Docstring
Returns the collection at the corresponding index from the stack.

:param index: position of collection, 0 based

:return: collection if one exists at the specified index
%End


    QgsPropertyCollection *collection( const QString &name );
%Docstring
Returns the first collection with a matching name from the stack.

:param name: name of collection to find

:return: collection if one exists with the specified name
%End

    virtual bool hasActiveProperties() const;

%Docstring
Returns true if the collection has any active properties, or false if all properties
within the collection are deactivated.

.. seealso:: :py:func:`isActive`

.. seealso:: :py:func:`hasDynamicProperties`
%End

    virtual bool hasDynamicProperties() const;

%Docstring
Returns true if the collection has any active, non-static properties, or false if either all non-static properties
within the collection are deactivated or if the collection only contains static properties.

.. seealso:: :py:func:`hasActiveProperties`
%End

    virtual bool isActive( int key ) const;

%Docstring
Returns true if the stack contains an active property with the specified key.

:param key: integer key for property to test. The intended use case is that a context specific enum is cast to
int and used for the key value.

.. seealso:: :py:func:`hasActiveProperties`
%End

    virtual QgsProperty property( int key ) const;

%Docstring
Returns the highest priority property with a matching key from within the stack.

:param key: integer key for property to return. The intended use case is that a context specific enum is cast to
int and used for the key value.

:return: matching property, or null if no matching, active property found.

.. seealso:: :py:func:`hasActiveProperty`
%End

    virtual QVariant value( int key, const QgsExpressionContext &context, const QVariant &defaultValue = QVariant() ) const;

%Docstring
Returns the calculated value of the highest priority property with the specified key from within the stack.

:param key: integer key for property to calculate. The intended use case is that a context specific enum is cast to
int and used for the key value.
:param context: expression context to evaluate property against
:param defaultValue: default value to return if no matching, active property found or if the property value
cannot be calculated

:return: calculated property value, or default value if property could not be evaluated
%End

    virtual QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const;

%Docstring
Returns the set of any fields referenced by the active properties from the stack.

:param context: expression context the properties will be evaluated against.
%End
    virtual bool prepare( const QgsExpressionContext &context = QgsExpressionContext() ) const;


    virtual QSet<int> propertyKeys() const;

    virtual bool hasProperty( int key ) const;


    virtual QVariant toVariant( const QgsPropertiesDefinition &definitions ) const;


    virtual bool loadVariant( const QVariant &collection, const QgsPropertiesDefinition &definitions );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspropertycollection.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
