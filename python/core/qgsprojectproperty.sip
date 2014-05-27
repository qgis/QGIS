class QgsProperty
{
%TypeHeaderCode
#include <qgsprojectproperty.h>
%End

  public:
    QgsProperty();
    virtual ~QgsProperty();

    /** dumps out the keys and values

    @param tabs is number of tabs to print; used for pretty-printing
    hierarchy
    */
    virtual void dump( int tabs = 0 ) const = 0;

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
    virtual bool writeXML( const QString & nodeName,
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


class QgsPropertyValue : QgsProperty
{
%TypeHeaderCode
#include <qgsprojectproperty.h>
%End

  public:
    QgsPropertyValue();
    QgsPropertyValue( const QVariant &value );
    virtual ~QgsPropertyValue();

    /** returns true if is a QgsPropertyKey */
    virtual bool isKey() const;

    /** returns true if is a QgsPropertyValue */
    virtual bool isValue() const;

    QVariant value() const;

    /** returns true if is a leaf node

    @note I suppose, in a way, value nodes can also be qualified as leaf
    nodes even though we're only counting key nodes.
    */
    bool isLeaf() const;

    void dump( int tabs = 0 ) const;

    bool readXML( QDomNode & keyNode );

    bool writeXML( const QString & nodeName,
                   QDomElement   & element,
                   QDomDocument  & document );

    int count() const;

    /** return keys that do not contain other keys

    Since QgsPropertyValue isn't a key, don't do anything.
    */
    void entryList( QStringList & keyName, QStringList & entries ) const;

};

class QgsPropertyKey : QgsProperty
{
%TypeHeaderCode
#include <qgsprojectproperty.h>
%End

  public:
    QgsPropertyKey( const QString name = "" );
    virtual ~ QgsPropertyKey();

    /// every key has a name
    // @{
    QString &name();
    // @}


    /** if this key has a value, it will be stored by its name in its
     * properties
     */
    QVariant value() const;


    /// add the given property key
    QgsPropertyKey * addKey( const QString & keyName );

    /// remove the given key
    void removeKey( const QString & keyName );

    /** set the value associated with this key
        @param name is the key name
        @param value is the value to set
    @return pointer to property value
    */
    QgsPropertyValue * setValue( const QString & name, const QVariant & value );

    /** set the value associated with this key

    @note that the single value node associated with each key is always
    stored keyed by the current key name
    */
    QgsPropertyValue * setValue( const QVariant & value );


    void dump( int tabs = 0 ) const;

    bool readXML( QDomNode & keyNode );

    bool writeXML( const QString &nodeName, QDomElement & element, QDomDocument & document );

    /// how many elements are contained within this one?
    int count() const;

    /// Does this property not have any subkeys or values?
    /* virtual */ bool isEmpty() const;

    /** returns true if is a QgsPropertyKey */
    virtual bool isKey() const;

    /** returns true if is a QgsPropertyValue */
    virtual bool isValue() const;

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
    virtual void clear();

    /// delete any sub-nodes
    virtual void clearKeys();

    QgsProperty * find( QString & propertyName );
}; // class QgsPropertyKey
