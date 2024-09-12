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
#include "qgis_core.h"

class QDomDocument;
class QDomNode;

/**
 * \ingroup core
 * \brief Simple key-value store (keys = strings, values = variants) that supports loading/saving to/from XML
 * in \verbatim <customproperties> \endverbatim element.
 *
 */
class CORE_EXPORT QgsObjectCustomProperties
{
  public:

    QgsObjectCustomProperties() = default;

    /**
     * Returns a list of all stored keys.
     */
    QStringList keys() const;

    /**
     * Add an entry to the store with the specified \a key.
     *
     * If an entry with the same \a key exists already, it will be overwritten.
     */
    void setValue( const QString &key, const QVariant &value );

    /**
     * Returns the value for the given \a key.
     *
     * If the \a key is not present in the properties, the \a defaultValue will be returned.
     */
    QVariant value( const QString &key, const QVariant &defaultValue = QVariant() ) const;

    /**
     * Removes a \a key (entry) from the store.
     */
    void remove( const QString &key );

    /**
     * Returns TRUE if the properties contains a \a key with the specified name.
     *
     * \since QGIS 3.14
     */
    bool contains( const QString &key ) const;

    /**
     * Read store contents from an XML node.
     * \param parentNode node to read from
     * \param keyStartsWith reads only properties starting with the specified string (or all if the string is empty)
     *
     * \see writeXml()
     */
    void readXml( const QDomNode &parentNode, const QString &keyStartsWith = QString() );

    /**
     * Writes the store contents to an XML node.
     *
     * \see readXml()
     */
    void writeXml( QDomNode &parentNode, QDomDocument &doc ) const;

  protected:
    QMap<QString, QVariant> mMap;

};

#endif // QGSOBJECTCUSTOMPROPERTIES_H
