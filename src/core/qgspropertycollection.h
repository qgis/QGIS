/***************************************************************************
     qgspropertycollection.h
     -----------------------
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
#ifndef QGSPROPERTYCOLLECTION_H
#define QGSPROPERTYCOLLECTION_H

#include "qgis_core.h"
#include "qgis.h"
#include <QString>
#include <QVariant>
#include <QColor>
#include "qgsexpressioncontext.h"
#include "qgsproperty.h"

class QDomElement;
class QDomDocument;

//! Definition of available properties
typedef QMap< int, QgsPropertyDefinition > QgsPropertiesDefinition;

/**
 * \ingroup core
 * \class QgsAbstractPropertyCollection
 * \brief Abstract base class for QgsPropertyCollection like objects.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsAbstractPropertyCollection
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsPropertyCollection *>( sipCpp ) )
      sipType = sipType_QgsPropertyCollection;
    else if ( dynamic_cast<QgsPropertyCollectionStack *>( sipCpp ) )
      sipType = sipType_QgsPropertyCollectionStack;
    else
      sipType = sipType_QgsAbstractPropertyCollection;
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsAbstractPropertyCollection. The name
     * parameter should be set to a descriptive name for the collection.
     */
    QgsAbstractPropertyCollection( const QString &name = QString() );

    virtual ~QgsAbstractPropertyCollection() = default;

    /**
     * Returns the descriptive name of the property collection.
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the descriptive name for the property collection.
     * \see name()
     */
    void setName( const QString &name ) { mName = name; }

    /**
     * Returns a list of property keys contained within the collection.
     */
    virtual QSet<int> propertyKeys() const = 0;

    /**
     * Removes all properties from the collection.
     */
    virtual void clear() = 0;

    /**
     * Returns true if the collection contains a property with the specified key.
     * \param key integer key for property. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \see property()
     */
    virtual bool hasProperty( int key ) const = 0;

    /**
     * Returns a matching property from the collection, if one exists.
     * \param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \returns matching property, or null if no matching, active property found.
     * \see hasProperty()
     */
    virtual QgsProperty property( int key ) const = 0;

    /**
     * Returns the calculated value of the property with the specified key from within the collection.
     * \param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \param context expression context to evaluate property against
     * \param defaultValue default value to return if no matching, active property found or if the property value
     * cannot be calculated
     * \returns calculated property value, or default value if property could not be evaluated
     * \see valueAsString()
     * \see valueAsColor()
     * \see valueAsDouble()
     * \see valueAsInt()
     * \see valueAsBool()
     */
    virtual QVariant value( int key, const QgsExpressionContext &context, const QVariant &defaultValue = QVariant() ) const = 0;

    /**
     * Calculates the current value of the property with the specified key and interprets it as a string.
     * \param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \param context QgsExpressionContext to evaluate the property for.
     * \param defaultString default string to return if the property cannot be calculated as a string
     * \param ok if specified, will be set to true if conversion was successful
     * \returns value parsed to string
     * \see value()
     * \see valueAsColor()
     * \see valueAsDouble()
     * \see valueAsInt()
     * \see valueAsBool()
     */
    QString valueAsString( int key, const QgsExpressionContext &context, const QString &defaultString = QString(), bool *ok SIP_OUT = nullptr ) const;

    /**
     * Calculates the current value of the property with the specified key and interprets it as a color.
     * \param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \param context QgsExpressionContext to evaluate the property for.
     * \param defaultColor default color to return if the property cannot be calculated as a color
     * \param ok if specified, will be set to true if conversion was successful
     * \returns value parsed to color
     * \see value()
     * \see valueAsString()
     * \see valueAsDouble()
     * \see valueAsInt()
     * \see valueAsBool()
     */
    QColor valueAsColor( int key, const QgsExpressionContext &context, const QColor &defaultColor = QColor(), bool *ok SIP_OUT = nullptr ) const;

    /**
     * Calculates the current value of the property with the specified key and interprets it as a double.
     * \param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \param context QgsExpressionContext to evaluate the property for.
     * \param defaultValue default double to return if the property cannot be calculated as a double
     * \param ok if specified, will be set to true if conversion was successful
     * \returns value parsed to double
     * \see value()
     * \see valueAsString()
     * \see valueAsColor()
     * \see valueAsInt()
     * \see valueAsBool()
     */
    double valueAsDouble( int key, const QgsExpressionContext &context, double defaultValue = 0.0, bool *ok SIP_OUT = nullptr ) const;

    /**
     * Calculates the current value of the property with the specified key and interprets it as an integer.
     * \param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \param context QgsExpressionContext to evaluate the property for.
     * \param defaultValue default integer to return if the property cannot be calculated as a integer
     * \param ok if specified, will be set to true if conversion was successful
     * \returns value parsed to integer
     * \see value()
     * \see valueAsString()
     * \see valueAsColor()
     * \see valueAsDouble()
     * \see valueAsBool()
     */
    int valueAsInt( int key, const QgsExpressionContext &context, int defaultValue = 0, bool *ok SIP_OUT = nullptr ) const;

    /**
     * Calculates the current value of the property with the specified key and interprets it as an boolean.
     * \param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \param context QgsExpressionContext to evaluate the property for.
     * \param defaultValue default boolean to return if the property cannot be calculated as a boolean
     * \param ok if specified, will be set to true if conversion was successful
     * \returns value parsed to bool
     * \see value()
     * \see valueAsString()
     * \see valueAsColor()
     * \see valueAsDouble()
     * \see valueAsInt()
     */
    bool valueAsBool( int key, const QgsExpressionContext &context, bool defaultValue = false, bool *ok SIP_OUT = nullptr ) const;

    /**
     * Prepares the collection against a specified expression context. Calling prepare before evaluating the
     * collection's properties multiple times allows precalculation of expensive setup tasks such as parsing expressions.
     * Returns true if preparation was successful.
     */
    virtual bool prepare( const QgsExpressionContext &context = QgsExpressionContext() ) const = 0;

    /**
     * Returns the set of any fields referenced by the active properties from the collection.
     * \param context expression context the properties will be evaluated against.
     */
    virtual QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const = 0;

    /**
     * Returns true if the collection contains an active property with the specified key.
     * \param key integer key for property to test. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     */
    virtual bool isActive( int key ) const = 0;

    /**
     * Returns true if the collection has any active properties, or false if all properties
     * within the collection are deactivated.
     * \see hasDynamicProperties()
     */
    virtual bool hasActiveProperties() const = 0;

    /**
     * Returns true if the collection has any active, non-static properties, or false if either all non-static properties
     * within the collection are deactivated or if the collection only contains static properties.
     * \see hasActiveProperties()
     */
    virtual bool hasDynamicProperties() const = 0;

    /**
     * Writes the current state of the property collection into an XML element
     * \param collectionElem destination element for the property collection's state
     * \param definitions property definitions
     * \see readXml()
    */
    virtual bool writeXml( QDomElement &collectionElem, const QgsPropertiesDefinition &definitions ) const;

    /**
     * Reads property collection state from an XML element.
     * \param collectionElem source DOM element for property collection's state
     * \param definitions property definitions
     * \see writeXml()
    */
    virtual bool readXml( const QDomElement &collectionElem, const QgsPropertiesDefinition &definitions );

    /**
     * Saves this property collection to a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::writeVariant to save it to an XML document.
     *
     * \see loadVariant()
     */
    virtual QVariant toVariant( const QgsPropertiesDefinition &definitions ) const = 0;

    /**
     * Loads this property collection from a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::readVariant to save it to an XML document.
     *
     * \see toVariant()
     */
    virtual bool loadVariant( const QVariant &configuration, const QgsPropertiesDefinition &definitions ) = 0;

  private:

    QString mName;
};

/**
 * \ingroup core
 * \class QgsPropertyCollection
 * \brief A grouped map of multiple QgsProperty objects, each referenced by a integer key value.
 *
 * Properties within a collection are referenced by an integer key. This is done to avoid the cost of
 * string creation and comparisons which would be required by a string key. The intended use case is that
 * a context specific enum is cast to int and used for the key value.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsPropertyCollection : public QgsAbstractPropertyCollection
{
  public:

    /**
     * Constructor for QgsPropertyCollection
     * \param name collection name
     */
    QgsPropertyCollection( const QString &name = QString() );

    /**
     * Copy constructor.
     */
    QgsPropertyCollection( const QgsPropertyCollection &other );

    QgsPropertyCollection &operator=( const QgsPropertyCollection &other );

    /**
     * Returns the number of properties contained within the collection.
     */
    int count() const;

    QSet<int> propertyKeys() const override;
    void clear() override;
    bool hasProperty( int key ) const override;
    QgsProperty property( int key ) const override SIP_SKIP;

    /**
     * Returns a reference to a matching property from the collection, if one exists.
     * \param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \returns matching property, or null if no matching, active property found.
     * \see hasProperty()
     */
    virtual QgsProperty &property( int key );

    QVariant value( int key, const QgsExpressionContext &context, const QVariant &defaultValue = QVariant() ) const override;
    bool prepare( const QgsExpressionContext &context = QgsExpressionContext() ) const override;
    QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const override;
    bool isActive( int key ) const override;
    bool hasActiveProperties() const override;
    bool hasDynamicProperties() const override;

    QVariant toVariant( const QgsPropertiesDefinition &definitions ) const override;
    bool loadVariant( const QVariant &configuration, const QgsPropertiesDefinition &definitions ) override;

    /**
     * Adds a property to the collection and takes ownership of it.
     * \param key integer key for property. Any existing property with the same key will be removed
     * and replaced by this property. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \param property property to add. Ownership is transferred to the collection. Setting an invalid property
     * will remove the property from the collection.
     */
    void setProperty( int key, const QgsProperty &property );

    /**
     * Convenience method, creates a QgsStaticProperty and stores it within the collection.
     * \param key integer key for property. Any existing property with the same key will be deleted
     * and replaced by this property. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \param value static value for property
     */
    void setProperty( int key, const QVariant &value );

  private:

    QHash<int, QgsProperty> mProperties;

    mutable bool mDirty = false;
    mutable bool mHasActiveProperties = false;
    mutable bool mHasDynamicProperties = false;
    mutable int mCount = 0;

    //! Scans through properties and updates cached values
    void rescan() const;
};


/**
 * \ingroup core
 * \class QgsPropertyCollectionStack
 * \brief An ordered stack of QgsPropertyCollection containers, where collections added later to the stack will take
 * priority over earlier collections.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsPropertyCollectionStack : public QgsAbstractPropertyCollection
{
  public:

    /**
     * Constructor for QgsPropertyCollectionStack.
     */
    QgsPropertyCollectionStack() = default;

    ~QgsPropertyCollectionStack() override;

    //! Copy constructor
    QgsPropertyCollectionStack( const QgsPropertyCollectionStack &other );

    QgsPropertyCollectionStack &operator=( const QgsPropertyCollectionStack &other );

    /**
     * Returns the number of collections contained within the stack.
     */
    int count() const;

    /**
     * Removes all collections from the stack.
     */
    void clear() override;

    /**
     * Appends a collection to the end of the stack, and transfers ownership of the collection to the stack. Properties
     * from the newly added collection will take priority over any existing properties with the same name.
     * \param collection collection to append. Ownership is transferred to the stack.
     */
    void appendCollection( QgsPropertyCollection *collection SIP_TRANSFER );

    /**
     * Returns the collection at the corresponding index from the stack.
     * \param index position of collection, 0 based
     * \returns collection if one exists at the specified index
     */
    QgsPropertyCollection *at( int index );

    /**
     * Returns the collection at the corresponding index from the stack.
     * \param index position of collection, 0 based
     * \returns collection if one exists at the specified index
     * \note not available in Python bindings
     */
    const QgsPropertyCollection *at( int index ) const SIP_SKIP;

    /**
     * Returns the first collection with a matching name from the stack.
     * \param name name of collection to find
     * \returns collection if one exists with the specified name
     */
    QgsPropertyCollection *collection( const QString &name );

    /**
     * Returns true if the collection has any active properties, or false if all properties
     * within the collection are deactivated.
     * \see isActive()
     * \see hasDynamicProperties()
     */
    bool hasActiveProperties() const override;

    /**
     * Returns true if the collection has any active, non-static properties, or false if either all non-static properties
     * within the collection are deactivated or if the collection only contains static properties.
     * \see hasActiveProperties()
     */
    bool hasDynamicProperties() const override;

    /**
     * Returns true if the stack contains an active property with the specified key.
     * \param key integer key for property to test. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \see hasActiveProperties()
     */
    bool isActive( int key ) const override;

    /**
     * Returns the highest priority property with a matching key from within the stack.
     * \param key integer key for property to return. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \returns matching property, or null if no matching, active property found.
     * \see hasActiveProperty()
     */
    QgsProperty property( int key ) const override;

    /**
     * Returns the calculated value of the highest priority property with the specified key from within the stack.
     * \param key integer key for property to calculate. The intended use case is that a context specific enum is cast to
     * int and used for the key value.
     * \param context expression context to evaluate property against
     * \param defaultValue default value to return if no matching, active property found or if the property value
     * cannot be calculated
     * \returns calculated property value, or default value if property could not be evaluated
     */
    QVariant value( int key, const QgsExpressionContext &context, const QVariant &defaultValue = QVariant() ) const override;

    /**
     * Returns the set of any fields referenced by the active properties from the stack.
     * \param context expression context the properties will be evaluated against.
     */
    QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const override;
    bool prepare( const QgsExpressionContext &context = QgsExpressionContext() ) const override;

    QSet<int> propertyKeys() const override;
    bool hasProperty( int key ) const override;

    QVariant toVariant( const QgsPropertiesDefinition &definitions ) const override;

    bool loadVariant( const QVariant &collection, const QgsPropertiesDefinition &definitions ) override;

  private:

    QList< QgsPropertyCollection * > mStack;

};

#endif // QGSPROPERTYCOLLECTION_H
