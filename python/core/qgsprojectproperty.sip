class QgsProjectProperty
{
%TypeHeaderCode
#include <qgsprojectproperty.h>
%End

  public:
    QgsProjectProperty();
    virtual ~QgsProjectProperty();

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
    virtual bool readXml( const QDomNode& keyNode ) = 0;

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
     * For QgsPropertyValue nodes, this is straightforward -- just return the
     * embedded QVariant, _value.  For QgsPropertyKey, this means returning
     * the QgsPropertyValue _value that is keyed by its name, if it exists;
     * i.e., QgsPropertyKey "foo" will return the property value mapped to its
     * name, "foo", in its QHash of QProperties.
     *
     */
    virtual QVariant value() const = 0;

}; // class QgsProperty


class QgsProjectPropertyValue : QgsProjectProperty
{
%TypeHeaderCode
#include <qgsprojectproperty.h>
%End

  public:
    QgsProjectPropertyValue();
    QgsProjectPropertyValue( const QVariant &value );
    virtual ~QgsProjectPropertyValue();

    /** Returns true if is a QgsPropertyKey */
    virtual bool isKey() const;

    /** Returns true if is a QgsPropertyValue */
    virtual bool isValue() const;

    QVariant value() const;

    /** Returns true if is a leaf node
     *
     * @note I suppose, in a way, value nodes can also be qualified as leaf
     * nodes even though we're only counting key nodes.
     */
    bool isLeaf() const;

    void dump( int tabs = 0 ) const;

    bool readXml( const QDomNode& keyNode );

    bool writeXml( const QString & nodeName,
                   QDomElement   & element,
                   QDomDocument  & document );

};

class QgsProjectPropertyKey : QgsProjectProperty
{
%TypeHeaderCode
#include <qgsprojectproperty.h>
%End

  public:
    QgsProjectPropertyKey( const QString& name = QString() );
    virtual ~QgsProjectPropertyKey();

    /**
     * The name of the property is used as identifier.
     */
    QString name() const;

    /**
     * The name of the property is used as identifier.
     */
    void setName( const QString& name );

    /** If this key has a value, it will be stored by its name in its
     * properties
     */
    QVariant value() const;


    /// add the given property key
    QgsProjectPropertyKey * addKey( const QString & keyName );

    /// remove the given key
    void removeKey( const QString & keyName );

    /** Set the value associated with this key
     * @param name is the key name
     * @param value is the value to set
     * @return pointer to property value
     */
    QgsProjectPropertyValue * setValue( const QString & name, const QVariant & value );

    /** Set the value associated with this key
     *
     * @note that the single value node associated with each key is always
     * stored keyed by the current key name
     */
    QgsProjectPropertyValue * setValue( const QVariant & value );


    void dump( int tabs = 0 ) const;

    bool readXml( const QDomNode& keyNode );

    bool writeXml( const QString &nodeName, QDomElement & element, QDomDocument & document );

    /// how many elements are contained within this one?
    int count() const;

    /// Does this property not have any subkeys or values?
    /* virtual */ bool isEmpty() const;

    /** Returns true if is a QgsPropertyKey */
    virtual bool isKey() const;

    /** Returns true if is a QgsPropertyValue */
    virtual bool isValue() const;

    /// return keys that do not contain other keys
    void entryList( QStringList & entries ) const;

    /// return keys that contain other keys
    void subkeyList( QStringList & entries ) const;

    /** Returns true if a leaf node
     * A leaf node is a key node that has either no value or a single value.
     * A non-leaf node would be a key node with key sub-nodes.
     */
    bool isLeaf() const;

    /// reset the QgsProperty key to prestine state
    virtual void clear();

    /// delete any sub-nodes
    virtual void clearKeys();

    QgsProjectProperty * find( QString & propertyName );
}; // class QgsPropertyKey
