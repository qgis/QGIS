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

class QDomNode;
class QDomElement;
class QDomDocument;


/** \ingroup core
 * An Abstract Base Class for QGIS project property hierarchies.

   Each sub-class is either a QgsPropertyKey or QgsPropertyValue.  QgsPropertyKeys can
   contain either QgsPropertyKeys or QgsPropertyValues, thus describing an
   hierarchy.  QgsPropertyValues are always graph leaves.

   @note

   This is really a hidden QgsProject implementation class.  It was getting
   too large and unwieldy, so it's broken out here into separate files.

*/
class CORE_EXPORT QgsProperty
{
  public:

    QgsProperty()
    {}

    virtual ~ QgsProperty()
    {}

    /** Dumps out the keys and values
     *
     * @param tabs is number of tabs to print; used for pretty-printing hierarchy
     */
    virtual void dump( int tabs = 0 ) const = 0;

    /** Returns true if is a QgsPropertyKey */
    virtual bool isKey() const = 0;

    /** Returns true if is a QgsPropertyValue */
    virtual bool isValue() const = 0;

    /** Returns true if a leaf node
     *
     * A leaf node is a key node that has either no value or a single value.
     * A non-leaf node would be a key node with key sub-nodes.
     *
     * This is used for entryList() and subkeyList() implementation.
     */
    virtual bool isLeaf() const = 0;

    /**
     * restores property hierarchy to given Dom node
     *
     *  Used for restoring properties from project file
     */
    virtual bool readXML( QDomNode & keyNode ) = 0;

    /**
     * adds property hierarchy to given Dom element
     *
     * Used for saving properties to project file.
     *
     * @param nodeName the tag name associated with this element
     * @param element the parent (or encompassing) property element
     * @param document the overall project file Dom document
     */
    virtual bool writeXML( const QString & nodeName,
                           QDomElement   & element,
                           QDomDocument  & document ) = 0;

    /** Return the node's value
     *
     * For QgsPropertyValue nodes, this is straightforward -- just return the
     * embedded QVariant, _value.  For QgsPropertyKey, this means returning
     * the QgsPropertyValue _value that is keyed by its name, if it exists;
     * i.e., QgsPropertyKey "foo" will return the property value mapped to its
     * name, "foo", in its QHash of QProperties.
     *
     */
    virtual QVariant value() const = 0;

}; // class QgsProperty




/** \ingroup core
 * QgsPropertyValue node

Contains a QgsPropertyKey's value
*/
class CORE_EXPORT QgsPropertyValue : public QgsProperty
{
  public:
    QgsPropertyValue() {}

    QgsPropertyValue( const QVariant &value )
        : value_( value )
    {}

    virtual ~QgsPropertyValue() {}

    /** Returns true if is a QgsPropertyKey */
    virtual bool isKey() const override { return false; }

    /** Returns true if is a QgsPropertyValue */
    virtual bool isValue() const override { return true; }

    QVariant value() const override { return value_; }

    /** Returns true if is a leaf node
     *
     * @note I suppose, in a way, value nodes can also be qualified as leaf
     * nodes even though we're only counting key nodes.
     */
    bool isLeaf() const override { return true; }

    void dump( int tabs = 0 ) const override;

    bool readXML( QDomNode & keyNode ) override;

    bool writeXML( const QString & nodeName,
                   QDomElement   & element,
                   QDomDocument  & document ) override;

    int count() const { return 0; }

    /** Return keys that do not contain other keys
     * Since QgsPropertyValue isn't a key, don't do anything.
     */
    void entryList( QStringList & keyName, QStringList & entries ) const
    { Q_UNUSED( keyName ); Q_UNUSED( entries ); /* NOP */ }

  private:

    /** We use QVariant as it's very handy to keep multiple types and provides
     * type conversions
     */
    QVariant value_;

}; // class QgsPropertyValue




/** \ingroup core
   QgsPropertyKey node

   Can, itself, contain QgsPropertyKeys and QgsPropertyValues.

   The internal QHash, mProperties, maps key names to their respective
   QgsPropertyValue or next QgsPropertyKey in the key name sequence.  The key with
   the current name should contain its QgsPropertyValue.

   E.g., given the key sequence "/foo/bar", "foo" will have a corresponding
   QgsPropertyKey with a name "foo".  It will contain an element in its
   mProperties that maps to "bar", which is another QgsPropertyKey.  The "bar"
   QgsPropertyKey will, in turn, have an element that maps to itself, i.e. "bar",
   that will contain a QgsPropertyValue.

*/
class CORE_EXPORT QgsPropertyKey : public QgsProperty
{
  public:
    QgsPropertyKey( const QString &name = "" );
    virtual ~ QgsPropertyKey();

    /// every key has a name
    // @{
    // @note not available in python bindings
    QString name() const { return mName; }

    QString &name() { return mName; }
    // @}


    /** If this key has a value, it will be stored by its name in its
     * properties
     */
    QVariant value() const override;


    /// add the given property key
    QgsPropertyKey * addKey( const QString & keyName )
    {
      delete mProperties.take( keyName );
      mProperties.insert( keyName, new QgsPropertyKey( keyName ) );

      return dynamic_cast<QgsPropertyKey*>( mProperties.value( keyName ) );
    }


    /// remove the given key
    void removeKey( const QString & keyName )
    {
      delete mProperties.take( keyName );
    }

    /** Set the value associated with this key
     * @param name is the key name
     * @param value is the value to set
     * @return pointer to property value
     */
    QgsPropertyValue * setValue( const QString & name, const QVariant & value )
    {
      delete mProperties.take( name );
      mProperties.insert( name, new QgsPropertyValue( value ) );

      return dynamic_cast<QgsPropertyValue*>( mProperties.value( name ) );
    }

    /** Set the value associated with this key
     *
     * @note that the single value node associated with each key is always
     * stored keyed by the current key name
     */
    QgsPropertyValue * setValue( const QVariant & value )
    {
      return setValue( name(), value );
    }

    void dump( int tabs = 0 ) const override;

    bool readXML( QDomNode & keyNode ) override;

    bool writeXML( const QString &nodeName, QDomElement & element, QDomDocument & document ) override;

    /// how many elements are contained within this one?
    int count() const { return mProperties.count(); }

    /// Does this property not have any subkeys or values?
    /* virtual */ bool isEmpty() const { return mProperties.isEmpty(); }

    /** Returns true if is a QgsPropertyKey */
    virtual bool isKey() const override { return true; }

    /** Returns true if is a QgsPropertyValue */
    virtual bool isValue() const override { return false; }

    /// return keys that do not contain other keys
    void entryList( QStringList & entries ) const;

    /// return keys that contain other keys
    void subkeyList( QStringList & entries ) const;

    /** Returns true if a leaf node
     * A leaf node is a key node that has either no value or a single value.
     * A non-leaf node would be a key node with key sub-nodes.
     */
    bool isLeaf() const override;

    /// reset the QgsProperty key to prestine state
    virtual void clear()
    {
      mName = "";
      clearKeys();
    }

    /// delete any sub-nodes
    virtual void clearKeys()
    {
      qDeleteAll( mProperties );
      mProperties.clear();
    }

    QgsProperty * find( QString & propertyName )
    {
      return mProperties.value( propertyName );
    }

  private:

    /// every key has a name
    QString mName;

    /// sub-keys
    QHash < QString, QgsProperty* > mProperties;

}; // class QgsPropertyKey

#endif
