/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsprojectproperty.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsProjectProperty
{
%Docstring
An Abstract Base Class for QGIS project property hierarchys.

Each sub-class is either a QgsProjectPropertyKey or QgsProjectPropertyValue.  QgsProjectPropertyKey can
contain either QgsProjectPropertyKey or QgsProjectPropertyValues, thus describing an
hierarchy.  QgsProjectPropertyValues are always graph leaves.

.. note::

   This class is used internally by QgsProject. It's generally recommended that the methods in
QgsProject are used to modify project properties rather than using these low-level classes.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprojectproperty.h"
%End
  public:
    QgsProjectProperty();
    virtual ~QgsProjectProperty();

    virtual void dump( int tabs = 0 ) const = 0;
%Docstring
Dumps out the keys and values

:param tabs: is number of tabs to print; used for pretty-printing hierarchy
%End

    virtual bool isKey() const = 0;
%Docstring
Returns true if the property is a QgsProjectPropertyKey.

.. seealso:: :py:func:`isValue`

.. seealso:: :py:func:`isLeaf`
%End

    virtual bool isValue() const = 0;
%Docstring
Returns true if the property is a QgsProjectPropertyValue.

.. seealso:: :py:func:`isKey`

.. seealso:: :py:func:`isLeaf`
%End

    virtual bool isLeaf() const = 0;
%Docstring
Returns true if property is a leaf node.

A leaf node is a key node that has either no value or only a single value.
A non-leaf node would be a key node with key sub-nodes.

This is used for entryList() and subkeyList() implementation.
%End

    virtual bool readXml( const QDomNode &keyNode ) = 0;
%Docstring
Restores the property hierarchy from a specified DOM node.

Used for restoring properties from project file
%End

    virtual bool writeXml( const QString &nodeName,
                           QDomElement &element,
                           QDomDocument &document ) = 0;
%Docstring
Writes the property hierarchy to a specified DOM element.

Used for saving properties to project file.

:param nodeName: the tag name associated with this element
:param element: the parent (or encompassing) property element
:param document: the overall project file Dom document
%End

    virtual QVariant value() const = 0;
%Docstring
Returns the node's value.

For QgsProjectPropertyValue nodes, this is straightforward -- just return the
embedded QVariant, _value.  For QgsProjectPropertyKey, this means returning
the QgsProjectPropertyValue _value that is keyed by its name, if it exists;
i.e., QgsProjectPropertyKey "foo" will return the property value mapped to its
name, "foo", in its QHash of QProperties.
%End

};


class QgsProjectPropertyValue : QgsProjectProperty
{
%Docstring
Project property value node, contains a QgsProjectPropertyKey's value.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprojectproperty.h"
%End
  public:

    QgsProjectPropertyValue();
%Docstring
Constructor for QgsProjectPropertyValue.
%End

    QgsProjectPropertyValue( const QVariant &value );
%Docstring
Constructor for QgsProjectPropertyValue, initialized to a specified value.
%End

    virtual bool isKey() const;
    virtual bool isValue() const;
    virtual QVariant value() const;

    virtual bool isLeaf() const;

    virtual void dump( int tabs = 0 ) const;

    virtual bool readXml( const QDomNode &keyNode );

     virtual bool writeXml( const QString &nodeName,
                   QDomElement &element,
                   QDomDocument &document );

};


class QgsProjectPropertyKey : QgsProjectProperty
{
%Docstring

Project property key node.

Can, itself, contain QgsProjectPropertyKey and QgsProjectPropertyValues.

The internal QHash, mProperties, maps key names to their respective
QgsProjectPropertyValue or next QgsProjectPropertyKey in the key name sequence.  The key with
the current name should contain its QgsProjectPropertyValue.

E.g., given the key sequence "/foo/bar", "foo" will have a corresponding
QgsProjectPropertyKey with a name "foo".  It will contain an element in its
mProperties that maps to "bar", which is another :py:class:`QgsProjectPropertyKey`.  The "bar"
QgsProjectPropertyKey will, in turn, have an element that maps to itself, i.e. "bar",
that will contain a QgsProjectPropertyValue.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprojectproperty.h"
%End
  public:

    QgsProjectPropertyKey( const QString &name = QString() );
%Docstring
Create a new QgsProjectPropertyKey with the specified identifier.
%End
    ~QgsProjectPropertyKey();

    QString name() const;
%Docstring
The name of the property is used as identifier.

.. seealso:: :py:func:`setName`
%End

    void setName( const QString &name );
%Docstring
The name of the property is used as identifier.

.. versionadded:: 3.0

.. seealso:: :py:func:`name`
%End

    virtual QVariant value() const;

%Docstring
If this key has a value, it will be stored by its name in its
properties
%End

    QgsProjectPropertyKey *addKey( const QString &keyName );
%Docstring
Adds the specified property key as a sub-key.
%End

    void removeKey( const QString &keyName );
%Docstring
Removes the specified key.
%End

    QgsProjectPropertyValue *setValue( const QString &name, const QVariant &value );
%Docstring
Sets the value associated with this key.

:param name: is the key name
:param value: is the value to set

:return: pointer to property value
%End

    QgsProjectPropertyValue *setValue( const QVariant &value );
%Docstring
Set the value associated with this key

.. note::

   that the single value node associated with each key is always
stored keyed by the current key name
%End

    virtual void dump( int tabs = 0 ) const;

    virtual bool readXml( const QDomNode &keyNode );

    virtual bool writeXml( const QString &nodeName, QDomElement &element, QDomDocument &document );


    int count() const;
%Docstring
Returns the number of sub-keys contained by this property.
%End

    bool isEmpty() const;
%Docstring
Returns true if this property contains no sub-keys.
%End

    virtual bool isKey() const;
    virtual bool isValue() const;
    virtual bool isLeaf() const;


    void entryList( QStringList &entries ) const;
%Docstring
Returns any sub-keys contained by this property that do not contain other keys.

.. seealso:: :py:func:`subkeyList`
%End

    void subkeyList( QStringList &entries ) const;
%Docstring
Return any sub-keys contained by this property which themselves contain other keys.

.. seealso:: :py:func:`entryList`
%End

    virtual void clear();
%Docstring
Resets the property to a default, empty state.
%End

    virtual void clearKeys();
%Docstring
Deletes any sub-nodes from the property.
%End

    QgsProjectProperty *find( const QString &propertyName ) const;
%Docstring
Attempts to find a property with a matching sub-key name.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsprojectproperty.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
