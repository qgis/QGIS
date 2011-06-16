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

class QDomNode;
class QDomElement;
class QDomDocument;
class QStringList;


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

    /** dumps out the keys and values

    @param tabs is number of tabs to print; used for pretty-printing
    hierarchy
    */
    virtual void dump( size_t tabs = 0 ) const = 0;

    /** returns true if is a QgsPropertyKey */
    virtual bool isKey() const = 0;

    /** returns true if is a QgsPropertyValue */
    virtual bool isValue() const = 0;

    /** returns true if a leaf node

    A leaf node is a key node that has either no value or a single value.
    A non-leaf node would be a key node with key sub-nodes.

    This is used for entryList() and subkeyList() implementation.
    */
    virtual bool isLeaf() const = 0;

    /**
       restores property hierarchy to given Dom node

       Used for restoring properties from project file
    */
    virtual bool readXML( QDomNode & keyNode ) = 0;

    /**
       adds property hierarchy to given Dom element

       Used for saving properties to project file.

       @param nodeName the tag name associated with this element
       @param element the parent (or encompassing) property element
       @param document the overall project file Dom document
    */
    virtual bool writeXML( QString const & nodeName,
                           QDomElement   & element,
                           QDomDocument  & document ) = 0;

    /** return the node's value

       For QgsPropertyValue nodes, this is straightforward -- just return the
       embedded QVariant, _value.  For QgsPropertyKey, this means returning
       the QgsPropertyValue _value that is keyed by its name, if it exists;
       i.e., QgsPropertyKey "foo" will return the property value mapped to its
       name, "foo", in its QHash of QProperties.

     */
    virtual QVariant value() const = 0;

}; // class QgsProperty




/** QgsPropertyValue node

Contains a QgsPropertyKey's value
*/
class CORE_EXPORT QgsPropertyValue : public QgsProperty
{
  public:
    QgsPropertyValue()
    {}

    QgsPropertyValue( QVariant const &value )
        : value_( value )
    {}

    virtual ~ QgsPropertyValue()
    {}

    /** returns true if is a QgsPropertyKey */
    virtual bool isKey() const
    { return false; }

    /** returns true if is a QgsPropertyValue */
    virtual bool isValue() const
    { return true; }

    QVariant value() const
    { return value_; }

    /** returns true if is a leaf node

    @note I suppose, in a way, value nodes can also be qualified as leaf
    nodes even though we're only counting key nodes.
    */
    bool isLeaf() const
    { return true; }

    void dump( size_t tabs = 0 ) const;

    bool readXML( QDomNode & keyNode );

    bool writeXML( QString const & nodeName,
                   QDomElement   & element,
                   QDomDocument  & document );

    size_t count() const
    { return 0; }


    /** return keys that do not contain other keys

    Since QgsPropertyValue isn't a key, don't do anything.
    */
    void entryList( QStringList & keyName, QStringList & entries ) const
    { Q_UNUSED( keyName ); Q_UNUSED( entries ); /* NOP */ }

  private:

    /** We use QVariant as it's very handy to keep multiple types and provides
        type conversions
    */
    QVariant value_;

}; // class QgsPropertyValue




/**
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

    QgsPropertyKey( QString const name = "" );

    virtual ~ QgsPropertyKey();

    /// every key has a name
    // @{
    QString const & name() const
    { return mName; }

    QString & name()
    { return mName; }
    // @}


    /** if this key has a value, it will be stored by its name in its
     * properties
     */
    QVariant value() const;


    /// add the given property key
    QgsPropertyKey * addKey( QString const & keyName )
    {
      delete mProperties.take( keyName );
      mProperties.insert( keyName, new QgsPropertyKey( keyName ) );

      return dynamic_cast<QgsPropertyKey*>( mProperties.value( keyName ) );
    }


    /// remove the given key
    void removeKey( QString const & keyName )
    {
      delete mProperties.take( keyName );
    }

    /** set the value associated with this key
        @param name is the key name
        @param value is the value to set
    @return pointer to property value
    */
    QgsPropertyValue * setValue( QString const & name, QVariant const & value )
    {
      delete mProperties.take( name );
      mProperties.insert( name, new QgsPropertyValue( value ) );

      return dynamic_cast<QgsPropertyValue*>( mProperties.value( name ) );
    }

    /** set the value associated with this key

    @note that the single value node associated with each key is always
    stored keyed by the current key name
    */
    QgsPropertyValue * setValue( QVariant const & value )
    {
      return setValue( name(), value );
    }



    void dump( size_t tabs = 0 ) const;

    bool readXML( QDomNode & keyNode );

    bool writeXML( QString const &nodeName, QDomElement & element, QDomDocument & document );

    /// how many elements are contained within this one?
    size_t count() const
    { return mProperties.count(); }

    /// Does this property not have any subkeys or values?
    /* virtual */ bool isEmpty() const
    { return mProperties.isEmpty(); }

    /** returns true if is a QgsPropertyKey */
    virtual bool isKey() const
    { return true; }

    /** returns true if is a QgsPropertyValue */
    virtual bool isValue() const
    { return false; }

    /// return keys that do not contain other keys
    void entryList( QStringList & entries ) const;

    /// return keys that contain other keys
    void subkeyList( QStringList & entries ) const;

    /** returns true if a leaf node

    A leaf node is a key node that has either no value or a single value.  A
    non-leaf node would be a key node with key sub-nodes.
    */
    bool isLeaf() const;

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
