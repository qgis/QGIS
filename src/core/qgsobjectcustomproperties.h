/***************************************************************************
                          qgsobjectcustomproperties.h
                             -------------------
    begin                : April 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder.sk at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOBJECTCUSTOMPROPERTIES_H
#define QGSOBJECTCUSTOMPROPERTIES_H

#include <QMap>
#include <QVariant>

class QDomDocument;
class QDomNode;

/**
 * Simple key-value store (keys = strings, values = variants) that supports loading/saving to/from XML
 * in \verbatim <customproperties> \endverbatim element.
 *
 * \note added in 2.4
 */
class CORE_EXPORT QgsObjectCustomProperties
{
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


  protected:
    QMap<QString, QVariant> mMap;

};

#endif // QGSOBJECTCUSTOMPROPERTIES_H
