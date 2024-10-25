/***************************************************************************
    qgsstaccollections.h
    ---------------------
    begin                : October 2024
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

#ifndef QGSSTACCOLLECTIONS_H
#define QGSSTACCOLLECTIONS_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsstaclink.h"

#include <QMap>
#include <QUrl>

class QgsStacCollection;

/**
 * \ingroup core
 * \brief Class for storing a list of STAC Collections.
 * This is typically used to store the data returned by STAC API /collections endpoint
 * \note Not available in python bindings
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsStacCollections
{
  public:
    //! Default constructor deleted, use the variant with required parameters
    QgsStacCollections() = delete;

    /**
     *  Constructs a valid list of collections,
     *  \param collections The STAC Collections to be stored, ownership is transferred
     *  \param links A list of references to other documents.
     *  \param numberMatched The total number of collections in the parent catalog, collection or total matching results from a STAC API endpoint
     *  \note ownership of \a collections is transferred. Collections will be deleted when object is destroyed.
     */
    QgsStacCollections( const QVector< QgsStacCollection * > collections, const QVector< QgsStacLink > links, int numberMatched = -1 );

    //! Destructor
    ~QgsStacCollections();

    /**
     * Returns the collections
     * Ownership is not transferred
     */
    QVector< QgsStacCollection * > collections() const;

    /**
     * Returns the collections
     * Caller takes ownership of the returned collections
     */
    QVector< QgsStacCollection * > takeCollections();

    //! Returns the number of returned collections
    int numberReturned() const;

    /**
     * Returns the total number of available collections
     * If this information was not available by the STAC server, -1 is returned
     */
    int numberMatched() const;

    /**
     * Returns the url of the collections' "self" link
     */
    QUrl url() const;

    /**
     * Returns the url of the collections' "root" link
     */
    QUrl rootUrl() const;

    /**
     * Returns the url of the collections' "next" link
     */
    QUrl nextUrl() const;

    /**
     * Returns the url of the collections' "prev" link
     */
    QUrl prevUrl() const;

  private:
    QVector< QgsStacCollection * > mCollections;
    QMap< QString, QString > mUrls;
    int mNumberMatched = -1;
};

#endif // QGSSTACCOLLECTIONS_H
