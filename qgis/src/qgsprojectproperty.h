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

/* $Id$ */

#ifndef QGSPROJECTPROPERTY_H
#define QGSPROJECTPROPERTY_H

#include <qvariant.h>
#include <qstring.h>
#include <qdict.h>

class QDomNode;
class QDomElement;
class QDomDocument;
class QStringList;


/**
   ABC for property hierarchies

   Each sub-class is either a QgsPropertyKey or QgsPropertyValue.  QgsPropertyKeys can
   contain either QgsPropertyKeys or QgsPropertyValues, thus describing an
   hierarchy.  QgsPropertyValues are always graph leaves.

   @note 

   This is really a hidden QgsProject implementation class.  It was getting
   too large and unwieldy, so it's broken out here into separate files.

*/
class QgsProperty
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
       restores property hierarchy to given DOM node

       Used for restoring properties from project file
    */
    virtual bool readXML(QDomNode & keyNode) = 0;

    /**
       adds property hierarchy to given DOM node

       Used for saving properties to project file.

       @param nodeName the tag name associated with this element
       @param node     the parent (or encompassing) property node
       @param documetn the overall project file DOM document
    */
    virtual bool writeXML(QString const & nodeName, 
                          QDomElement   & element,
                          QDomDocument  & document) = 0;

    /** return the node's value

       For QgsPropertyValue nodes, this is straightforward -- just return the
       embedded QVariant, _value.  For QgsPropertyKey, this means returning
       the QgsPropertyValue _value that is keyed by its name, if it exists;
       i.e., QgsPropertyKey "foo" will return the property value mapped to its
       name, "foo", in its QDict of QProperties.

     */
    virtual QVariant value() const = 0;

}; // class QgsProperty




/** QgsPropertyValue node

Contains a QgsPropertyKey's value
*/
class QgsPropertyValue : public QgsProperty
{
public:
    QgsPropertyValue()
    {} 

    QgsPropertyValue(QVariant const &value)
        : value_(value)
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

    void dump( size_t tabs = 0) const;

    bool readXML(QDomNode & keyNode);

    bool writeXML( QString const & nodeName, 
                   QDomElement   & element,
                   QDomDocument  & document );

    size_t count() const
    { return 0; }


    /** return keys that do not contain other keys

    Since QgsPropertyValue isn't a key, don't do anything.
    */
    void entryList( QStringList & keyName, QStringList & entries ) const
    { /* NOP */ }

private:

    /** We use QVariant as it's very handy to keep multiple types and provides
        type conversions
    */
    QVariant value_;
    
}; // class QgsPropertyValue




/**
   QgsPropertyKey node

   Can, itself, contain QgsPropertyKeys and QgsPropertyValues.

   The internal QDict, properties_, maps key names to their respective
   QgsPropertyValue or next QgsPropertyKey in the key name sequence.  The key with
   the current name should contain its QgsPropertyValue.

   E.g., given the key sequence "/foo/bar", "foo" will have a corresponding
   QgsPropertyKey with a name "foo".  It will contain an element in its
   properties_ that maps to "bar", which is another QgsPropertyKey.  The "bar"
   QgsPropertyKey will, in turn, have an element that maps to itself, i.e. "bar",
   that will contain a QgsPropertyValue.

*/
class QgsPropertyKey : public QgsProperty
{
public:

    QgsPropertyKey( QString const name = "" );

    virtual ~ QgsPropertyKey()
    { }

    /// every key has a name
    // @{
    QString const & name() const
    { return name_; }

    QString & name()
    { return name_; }
    // @}


    /** if this key has a value, it will be stored by its name in its
     * properties
     */
    QVariant value() const;


    /// add the given property key
    QgsPropertyKey * addKey( QString const & keyName )
    {
        properties_.replace( keyName, new QgsPropertyKey(keyName) );

        return dynamic_cast<QgsPropertyKey*>(properties_.find( keyName ));
    }


    /// remove the given key
    bool removeKey( QString const & keyName )
    {
        return properties_.remove( keyName );
    }

    /** set the value associated with this key
        @param name is the key name
    */
    QgsPropertyValue * setValue( QString const & name, QVariant const & value )
    {
        properties_.replace( name, new QgsPropertyValue( value ) );

        return dynamic_cast<QgsPropertyValue*>(properties_.find( name ));
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

    bool readXML(QDomNode & keyNode);

    bool writeXML(QString const &nodeName, QDomElement & element, QDomDocument & document);

    /// how many elements are contained within this one?
    size_t count() const
    { return properties_.count(); }

    /// Does this property not have any subkeys or values?
    /* virtual */ bool isEmpty() const 
    { return properties_.isEmpty(); }

    /** returns true if is a QgsPropertyKey */ 
    virtual bool isKey() const
    { return true; }

    /** returns true if is a QgsPropertyValue */ 
    virtual bool isValue() const
    { return false; }

    /// return keys that do not contain other keys
    void entryList( QStringList & entries ) const;

    /// return keys that contain other keys
    void subkeyList(QStringList & entries) const;

    /** returns true if a leaf node 

    A leaf node is a key node that has either no value or a single value.  A
    non-leaf node would be a key node with key sub-nodes.
    */
    bool isLeaf() const;

    /// reset the QgsProperty key to prestine state
    virtual void clear()
    {
        name_ = "";
        properties_.clear();
    }

    /// delete any sub-nodes
    virtual void clearKeys()
    {
        properties_.clear();
    }

    QgsProperty * find( QString & propertyName ) 
    {
        return properties_.find( propertyName );
    }

private:

    /// every key has a name
    QString name_;

    /// sub-keys
    QDict < QgsProperty > properties_;

}; // class QgsPropertyKey





#endif
