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

   Each sub-class is either a QgsProjectPropertyKey or QgsProjectPropertyValue.  QgsProjectPropertyKey can
   contain either QgsProjectPropertyKey or QgsProjectPropertyValues, thus describing an
   hierarchy.  QgsProjectPropertyValues are always graph leaves.

   @note

   This is really a hidden QgsProject implementation class.  It was getting
   too large and unwieldy, so it's broken out here into separate files.

*/
class CORE_EXPORT QgsProjectProperty
{
  public:
    QgsProjectProperty();
    virtual ~QgsProjectProperty() = default;

    /** Dumps out the keys and values
     *
     * @param tabs is number of tabs to print; used for pretty-printing hierarchy
     */
    virtual void dump( int tabs = 0 ) const = 0;

    //! Returns true if is a QgsProjectPropertyKey
    virtual bool isKey() const = 0;

    //! Returns true if is a QgsProjectPropertyValue
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
    virtual bool readXml( QDomNode & keyNode ) = 0;

    /**
     * adds property hierarchy to given Dom element
     *
     * Used for saving properties to project file.
     *
     * @param nodeName the tag name associated with this element
     * @param element the parent (or encompassing) property element
     * @param document the overall project file Dom document
     */
    virtual bool writeXml( const QString & nodeName,
                           QDomElement   & element,
                           QDomDocument  & document ) = 0;

    /** Return the node's value
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


/** \ingroup core
 * QgsProjectPropertyValue node

Contains a QgsProjectPropertyKey's value
*/
class CORE_EXPORT QgsProjectPropertyValue : public QgsProjectProperty
{
  public:
    QgsProjectPropertyValue() = default;

    QgsProjectPropertyValue( const QVariant &value )
        : mValue( value )
    {}

    virtual bool isKey() const override { return false; }
    virtual bool isValue() const override { return true; }

    QVariant value() const override { return mValue; }

    /** Returns true if is a leaf node
     *
     * @note I suppose, in a way, value nodes can also be qualified as leaf
     * nodes even though we're only counting key nodes.
     */
    bool isLeaf() const override { return true; }

    void dump( int tabs = 0 ) const override;

    bool readXml( QDomNode & keyNode ) override;

    bool writeXml( const QString & nodeName,
                   QDomElement   & element,
                   QDomDocument  & document ) override;

    int count() const { return 0; }

    /** Return keys that do not contain other keys
     * Since QgsProjectPropertyValue isn't a key, don't do anything.
     */
    void entryList( QStringList & keyName, QStringList & entries ) const
    { Q_UNUSED( keyName ); Q_UNUSED( entries ); /* NOP */ }

  private:

    /** We use QVariant as it's very handy to keep multiple types and provides
     * type conversions
     */
    QVariant mValue;

};


/** \ingroup core
   QgsProjectPropertyKey node

   Can, itself, contain QgsProjectPropertyKey and QgsProjectPropertyValues.

   The internal QHash, mProperties, maps key names to their respective
   QgsProjectPropertyValue or next QgsProjectPropertyKey in the key name sequence.  The key with
   the current name should contain its QgsProjectPropertyValue.

   E.g., given the key sequence "/foo/bar", "foo" will have a corresponding
   QgsProjectPropertyKey with a name "foo".  It will contain an element in its
   mProperties that maps to "bar", which is another QgsProjectPropertyKey.  The "bar"
   QgsProjectPropertyKey will, in turn, have an element that maps to itself, i.e. "bar",
   that will contain a QgsProjectPropertyValue.

*/
class CORE_EXPORT QgsProjectPropertyKey : public QgsProjectProperty
{
  public:

    /**
     * Create a new QgsProjectPropertyKey with the specified identifier.
     */
    QgsProjectPropertyKey( const QString& name = QString() );
    virtual ~QgsProjectPropertyKey();

    /**
     * The name of the property is used as identifier.
     */
    QString name() const { return mName; }

    /**
     * The name of the property is used as identifier.
     *
     * @note Added in QGIS 3.0
     */
    void setName( const QString& name );

    /**
     * If this key has a value, it will be stored by its name in its
     * properties
     */
    QVariant value() const override;


    /// add the given property key
    QgsProjectPropertyKey *addKey( const QString & keyName )
    {
      delete mProperties.take( keyName );
      mProperties.insert( keyName, new QgsProjectPropertyKey( keyName ) );

      return dynamic_cast<QgsProjectPropertyKey*>( mProperties.value( keyName ) );
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
    QgsProjectPropertyValue * setValue( const QString & name, const QVariant & value )
    {
      delete mProperties.take( name );
      mProperties.insert( name, new QgsProjectPropertyValue( value ) );

      return dynamic_cast<QgsProjectPropertyValue*>( mProperties.value( name ) );
    }

    /** Set the value associated with this key
     *
     * @note that the single value node associated with each key is always
     * stored keyed by the current key name
     */
    QgsProjectPropertyValue * setValue( const QVariant & value )
    {
      return setValue( name(), value );
    }

    void dump( int tabs = 0 ) const override;

    bool readXml( QDomNode & keyNode ) override;

    bool writeXml( const QString &nodeName, QDomElement & element, QDomDocument & document ) override;

    /// how many elements are contained within this one?
    int count() const { return mProperties.count(); }

    /// Does this property not have any subkeys or values?
    /* virtual */ bool isEmpty() const { return mProperties.isEmpty(); }

    virtual bool isKey() const override { return true; }
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
      mName = QLatin1String( "" );
      clearKeys();
    }

    /// delete any sub-nodes
    virtual void clearKeys()
    {
      qDeleteAll( mProperties );
      mProperties.clear();
    }

    QgsProjectProperty * find( QString & propertyName )
    {
      return mProperties.value( propertyName );
    }

  private:

    /// every key has a name
    QString mName;

    /// sub-keys
    QHash < QString, QgsProjectProperty* > mProperties;

};

#endif
