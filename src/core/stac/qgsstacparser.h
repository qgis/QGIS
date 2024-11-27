/***************************************************************************
    qgsstacparser.h
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACPARSER_H
#define QGSSTACPARSER_H

#define SIP_NO_FILE

#include <nlohmann/json.hpp>
#include <QUrl>

#include "qgsstacobject.h"
#include "qgsstacasset.h"

class QgsStacCatalog;
class QgsStacCollection;
class QgsStacCollections;
class QgsStacItem;
class QgsStacItemCollection;


/**
 * \brief SpatioTemporal Asset Catalog JSON parser
 *
 * This class parses json data and creates the appropriate
 * STAC Catalog, Collection, Item or ItemCollection
 *
 * \note not available in Python bindings
*/
class QgsStacParser
{
  public:
    //! Default constructor
    QgsStacParser() = default;

    //! Sets the JSON \data to be parsed
    void setData( const QByteArray &data );

    /**
     *  Sets the base \a url that will be used to resolve relative links.
     *  If not called, relative links will not be resolved to absolute links.
     */
    void setBaseUrl( const QUrl &url );

    /**
     * Returns the parsed STAC Catalog
     * If parsing failed, NULLPTR is returned
     * The caller takes ownership of the returned catalog
     */
    QgsStacCatalog *catalog();

    /**
     * Returns the parsed STAC Collection
     * If parsing failed, NULLPTR is returned
     * The caller takes ownership of the returned collection
     */
    QgsStacCollection *collection();

    /**
     * Returns the parsed STAC Item
     * If parsing failed, NULLPTR is returned
     * The caller takes ownership of the returned item
     */
    QgsStacItem *item();

    /**
     * Returns the parsed STAC API Item Collection
     * If parsing failed, NULLPTR is returned
     * The caller takes ownership of the returned item collection
     */
    QgsStacItemCollection *itemCollection();

    /**
     * Returns the parsed STAC API Collections
     * If parsing failed, NULLPTR is returned
     * The caller takes ownership of the returned collections
     */
    QgsStacCollections *collections();

    //! Returns the type of the parsed object
    QgsStacObject::Type type() const;

    //! Returns the last parsing error
    QString error() const;

  private:
    QgsStacItem *parseItem( const nlohmann::json &data );
    QgsStacCatalog *parseCatalog( const nlohmann::json &data );
    QgsStacCollection *parseCollection( const nlohmann::json &data );

    QVector< QgsStacLink > parseLinks( const nlohmann::json &data );
    QMap< QString, QgsStacAsset > parseAssets( const nlohmann::json &data );
    static bool isSupportedStacVersion( const QString &version );
    //! Returns a QString, treating null elements as empty strings
    static QString getString( const nlohmann::json &data );

    nlohmann::json mData;
    QgsStacObject::Type mType = QgsStacObject::Type::Unknown;
    std::unique_ptr<QgsStacObject> mObject;
    QString mError;
    QUrl mBaseUrl;
};


#endif // QGSSTACPARSER_H
