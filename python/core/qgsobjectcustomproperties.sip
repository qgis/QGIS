/**
 * Simple key-value store (keys = strings, values = variants) that supports loading/saving to/from XML
 * in \verbatim <customproperties> \endverbatim element.
 *
 * \note added in 2.4
 */
class QgsObjectCustomProperties
{
%TypeHeaderCode
#include <qgsobjectcustomproperties.h>
%End
  public:
    QgsObjectCustomProperties();

    //! Return list of stored keys
    QStringList keys() const;

    //! Add an entry to the store. If the entry with the keys exists already, it will be overwritten
    void setValue( const QString& key, const QVariant& value );

    //! Return value for the given key. If the key is not stored, default value will be used
    QVariant value( const QString& key, const QVariant& defaultValue = QVariant() ) const;

    //! Remove a key (entry) from the store
    void remove( const QString& key );


    /** Read store contents from XML
      @param parentNode node to read from
      @param keyStartsWith reads only properties starting with the specified string (or all if the string is empty)
     */
    void readXml( const QDomNode& parentNode, const QString& keyStartsWith = QString() );

    /** Write store contents to XML */
    void writeXml( QDomNode& parentNode, QDomDocument& doc ) const;

};
