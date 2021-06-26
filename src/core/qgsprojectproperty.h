/***************************************************************************
                                  qgsproject.h

                      Implements persistent project state.

                              -------------------
  begin                : February 24, 2005
  copyright            : (C) 2005 by Mark Coletti
  email                : mcoletti at gmail.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPROJECTPROPERTY_H
#define QGSPROJECTPROPERTY_H

#include <QHash>
#include <QVariant>
#include <QStringList>
#include <QCoreApplication>

#include "qgis_core.h"

class QDomNode;
class QDomElement;
class QDomDocument;


/**
 * \class QgsProjectProperty
 * \ingroup core
 * An Abstract Base Class for QGIS project property hierarchys.
 *
 * Each sub-class is either a QgsProjectPropertyKey or QgsProjectPropertyValue.  QgsProjectPropertyKey can
 * contain either QgsProjectPropertyKey or QgsProjectPropertyValues, thus describing an
 * hierarchy.  QgsProjectPropertyValues are always graph leaves.
 *
 * \note This class is used internally by QgsProject. It's generally recommended that the methods in
 * QgsProject are used to modify project properties rather than using these low-level classes.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsProjectProperty
{
  public:
    QgsProjectProperty();
    virtual ~QgsProjectProperty() = default;

    /**
     * Dumps out the keys and values
     *
     * \param tabs is number of tabs to print; used for pretty-printing hierarchy
     */
    virtual void dump( int tabs = 0 ) const = 0;

    /**
     * Returns TRUE if the property is a QgsProjectPropertyKey.
     * \see isValue()
     * \see isLeaf()
     */
    virtual bool isKey() const = 0;

    /**
     * Returns TRUE if the property is a QgsProjectPropertyValue.
     * \see isKey()
     * \see isLeaf()
     */
    virtual bool isValue() const = 0;

    /**
     * Returns TRUE if property is a leaf node.
     *
     * A leaf node is a key node that has either no value or only a single value.
     * A non-leaf node would be a key node with key sub-nodes.
     *
     * This is used for entryList() and subkeyList() implementation.
     */
    virtual bool isLeaf() const = 0;

    /**
     * Restores the property hierarchy from a specified DOM node.
     *
     * Used for restoring properties from project file
     */
    virtual bool readXml( const QDomNode &keyNode ) = 0;

    /**
     * Writes the property hierarchy to a specified DOM element.
     *
     * Used for saving properties to project file.
     *
     * \param nodeName the tag name associated with this element
     * \param element the parent (or encompassing) property element
     * \param document the overall project file Dom document
     */
    virtual bool writeXml( const QString &nodeName,
                           QDomElement &element,
                           QDomDocument &document ) = 0;

    /**
     * Returns the node's value.
     *
     * For QgsProjectPropertyValue nodes, this is straightforward -- just return the
     * embedded QVariant, _value.  For QgsProjectPropertyKey, this means returning
     * the QgsProjectPropertyValue _value that is keyed by its name, if it exists;
     * i.e., QgsProjectPropertyKey "foo" will return the property value mapped to its
     * name, "foo", in its QHash of QProperties.
     *
     */
    virtual QVariant value() const = 0;

};


/**
 * \class QgsProjectPropertyValue
 * \ingroup core
 * Project property value node, contains a QgsProjectPropertyKey's value.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsProjectPropertyValue : public QgsProjectProperty
{
  public:

    //! Constructor for QgsProjectPropertyValue.
    QgsProjectPropertyValue() = default;

    /**
     * Constructor for QgsProjectPropertyValue, initialized to a specified value.
     */
    QgsProjectPropertyValue( const QVariant &value )
      : mValue( value )
    {}

    bool isKey() const override { return false; }
    bool isValue() const override { return true; }
    QVariant value() const override { return mValue; }

    //value nodes can also be qualified as leaf nodes even though we only count key nodes.
    bool isLeaf() const override { return true; }

    void dump( int tabs = 0 ) const override;
    bool readXml( const QDomNode &keyNode ) override;
    bool writeXml( const QString &nodeName,
                   QDomElement &element,
                   QDomDocument &document ) override;

  private:

    // We use QVariant as it's very handy to keep multiple types and provides type conversions
    QVariant mValue;

};


/**
 * \class QgsProjectPropertyKey
 * \ingroup core
 *
 * Project property key node.
 *
 * Can, itself, contain QgsProjectPropertyKey and QgsProjectPropertyValues.
 *
 * The internal QHash, mProperties, maps key names to their respective
 * QgsProjectPropertyValue or next QgsProjectPropertyKey in the key name sequence.  The key with
 * the current name should contain its QgsProjectPropertyValue.
 *
 * E.g., given the key sequence "/foo/bar", "foo" will have a corresponding
 * QgsProjectPropertyKey with a name "foo".  It will contain an element in its
 * mProperties that maps to "bar", which is another QgsProjectPropertyKey.  The "bar"
 * QgsProjectPropertyKey will, in turn, have an element that maps to itself, i.e. "bar",
 * that will contain a QgsProjectPropertyValue.
 *
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsProjectPropertyKey : public QgsProjectProperty
{
    Q_DECLARE_TR_FUNCTIONS( QgsProjectPropertyKey )

  public:

    /**
     * Create a new QgsProjectPropertyKey with the specified identifier.
     */
    QgsProjectPropertyKey( const QString &name = QString() );
    ~QgsProjectPropertyKey() override;

    /**
     * The name of the property is used as identifier.
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * The name of the property is used as identifier.
     *
     * \see name()
     * \since QGIS 3.0
     */
    void setName( const QString &name );

    /**
     * If this key has a value, it will be stored by its name in its
     * properties
     */
    QVariant value() const override;

    /**
     * Adds the specified property key as a sub-key.
     */
    QgsProjectPropertyKey *addKey( const QString &keyName )
    {
      if ( mProperties.contains( keyName ) )
        delete mProperties.take( keyName );

      QgsProjectPropertyKey *p = new QgsProjectPropertyKey( keyName );
      mProperties.insert( keyName, p );

      return p;
    }

    /**
     * Removes the specified key.
     */
    void removeKey( const QString &keyName )
    {
      delete mProperties.take( keyName );
    }

    /**
     * Sets the value associated with this key.
     * \param name is the key name
     * \param value is the value to set
     * \returns pointer to property value
     */
    QgsProjectPropertyValue *setValue( const QString &name, const QVariant &value )
    {
      if ( mProperties.contains( name ) )
        delete mProperties.take( name );

      QgsProjectPropertyValue *p = new QgsProjectPropertyValue( value );
      mProperties.insert( name, p );

      return p;
    }

    /**
     * Set the value associated with this key
     *
     * \note that the single value node associated with each key is always
     * stored keyed by the current key name
     */
    QgsProjectPropertyValue *setValue( const QVariant &value )
    {
      return setValue( name(), value );
    }

    void dump( int tabs = 0 ) const override;
    bool readXml( const QDomNode &keyNode ) override;
    bool writeXml( const QString &nodeName, QDomElement &element, QDomDocument &document ) override;

    /**
     * Returns the number of sub-keys contained by this property.
     */
    int count() const { return mProperties.count(); }

    /**
     * Returns TRUE if this property contains no sub-keys.
     */
    bool isEmpty() const { return mProperties.isEmpty(); }

    bool isKey() const override { return true; }
    bool isValue() const override { return false; }
    bool isLeaf() const override;

    /**
     * Returns any sub-keys contained by this property that do not contain other keys.
     * \see subkeyList()
     */
    void entryList( QStringList &entries ) const;

    /**
     * Returns any sub-keys contained by this property which themselves contain other keys.
     * \see entryList()
     */
    void subkeyList( QStringList &entries ) const;

    /**
     * Resets the property to a default, empty state.
     */
    virtual void clear()
    {
      mName.clear();
      clearKeys();
    }

    /**
     * Deletes any sub-nodes from the property.
     */
    virtual void clearKeys()
    {
      qDeleteAll( mProperties );
      mProperties.clear();
    }

    /**
     * Attempts to find a property with a matching sub-key name.
     */
    QgsProjectProperty *find( const QString &propertyName ) const
    {
      return mProperties.value( propertyName );
    }

  private:

    //! Name of the key
    QString mName;

    //! Sub-keys
    QHash < QString, QgsProjectProperty * > mProperties;

};

#endif
