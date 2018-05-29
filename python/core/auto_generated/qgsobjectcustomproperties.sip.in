/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsobjectcustomproperties.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsObjectCustomProperties
{
%Docstring
Simple key-value store (keys = strings, values = variants) that supports loading/saving to/from XML
in \verbatim <customproperties> \endverbatim element.

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsobjectcustomproperties.h"
%End
  public:

    QgsObjectCustomProperties();
%Docstring
Constructor for QgsObjectCustomProperties.
%End

    QStringList keys() const;
%Docstring
Returns list of stored keys
%End

    void setValue( const QString &key, const QVariant &value );
%Docstring
Add an entry to the store. If the entry with the keys exists already, it will be overwritten
%End

    QVariant value( const QString &key, const QVariant &defaultValue = QVariant() ) const;
%Docstring
Returns value for the given key. If the key is not stored, default value will be used
%End

    void remove( const QString &key );
%Docstring
Remove a key (entry) from the store
%End


    void readXml( const QDomNode &parentNode, const QString &keyStartsWith = QString() );
%Docstring
Read store contents from XML

:param parentNode: node to read from
:param keyStartsWith: reads only properties starting with the specified string (or all if the string is empty)
%End

    void writeXml( QDomNode &parentNode, QDomDocument &doc ) const;
%Docstring
Write store contents to XML
%End


  protected:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsobjectcustomproperties.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
