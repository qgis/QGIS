/***************************************************************************
    qgsstacitemcollection.h
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

#ifndef QGSSTACITEMCOLLECTION_H
#define QGSSTACITEMCOLLECTION_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsstaclink.h"

#include <QMap>
#include <QUrl>

class QgsStacItem;

/**
 * \ingroup core
 * \brief Class for storing a STAC Item Collections.
 * An Item Collection is typically returned by STAP API endpoints and contains a subset
 * of the the STAC Items available in a STAC Catalog or Collection
 * \note Not available in python bindings
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsStacItemCollection
{
  public:
    //! Default constructor deleted, use the variant with required parameters
    QgsStacItemCollection() = delete;

    /**
     *  Constructs a valid item collection
     *  \param items The STAC Items in the item collection, ownership is transferred
     *  \param links A list of references to other documents.
     *  \param numberMatched The total number of items in the parent catalog, collection or total matching results from a STAC API endpoint
     *  \note ownership of \a items is transferred. Items will be deleted when object is destroyed.
     */
    QgsStacItemCollection( const QVector< QgsStacItem * > &items, const QVector< QgsStacLink > &links, int numberMatched = -1 );

    //! Destructor
    ~QgsStacItemCollection();

    /**
     * Returns the items in the collection
     * Ownership is not transferred
     */
    QVector< QgsStacItem * > items() const;

    /**
     * Returns the items in the collection
     * Caller takes ownership of the returned items
     */
    QVector< QgsStacItem * > takeItems();

    /**
     * Returns the url of the item collection's "self" link
     */
    QUrl url() const;

    /**
     * Returns the url of the item collection's "root" link
     */
    QUrl rootUrl() const;

    /**
     * Returns the url of the item collection's "parent" link
     */
    QUrl parentUrl() const;

    /**
     * Returns the url of the item collection's "collection" link
     */
    QUrl collectionUrl() const;

    /**
     * Returns the url of the item collection's "next" link
     */
    QUrl nextUrl() const;

    //! Returns the number of items in the collection
    int numberReturned() const;

    /**
     * Returns the total number of items in the parent collection
     * If this information was not available by the STAC server, -1 is returned
     */
    int numberMatched() const;

  private:
    QVector< QgsStacItem * > mItems;
    const QVector< QgsStacLink > mLinks;
    QMap< QString, QString > mUrls;
    int mNumberMatched = -1;
};

#endif // QGSSTACITEMCOLLECTION_H
