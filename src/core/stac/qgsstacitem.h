/***************************************************************************
    qgsstacitem.h
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

#ifndef QGSSTACITEM_H
#define QGSSTACITEM_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsrange.h"
#include "qgsstacobject.h"
#include "qgsstacasset.h"
#include "qgsgeometry.h"
#include "qgsbox3d.h"
#include "qgsmimedatautils.h"

/**
 * \ingroup core
 * \brief Class for storing a STAC Item's data
 *
 * \note Not available in python bindings
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsStacItem : public QgsStacObject
{
  public:
    //! Default constructor deleted, use the variant with required parameters
    QgsStacItem() = delete;

    /**
     * Constructs a valid QgsStacItem
     * \param id Provider identifier. The ID should be unique within the Collection that contains the Item.
     * \param version The STAC version the Item implements.
     * \param geometry The full footprint of the asset represented by this item, in WGS84
     * \param properties A dictionary of additional metadata for the Item.
     * \param links List of link objects to resources and related URLs.
     * \param assets Dictionary of asset objects that can be downloaded, each with a unique key.
     * \param bbox Bounding Box of the asset represented by this Item. Required if geometry is not null
     */
    QgsStacItem( const QString &id,
                 const QString &version,
                 const QgsGeometry &geometry,
                 const QVariantMap &properties,
                 const QVector< QgsStacLink > &links,
                 const QMap< QString, QgsStacAsset > &assets,
                 const QgsBox3D &bbox );

    QgsStacObject::Type type() const override;
    QString toHtml() const override;

    //! Returns the full footprint of the asset represented by this item, in WGS84
    QgsGeometry geometry() const;

    //! Sets the full footprint of the asset represented by this item, in WGS84
    void setGeometry( const QgsGeometry &geometry );

    //! Returns the STAC item's spatial extent in WGS84 coordinates
    QgsBox3D boundingBox() const;

    //! Sets the STAC item's spatial extent in WGS84 coordinates to \a bbox
    void setBoundingBox( const QgsBox3D &bbox );

    //! Returns a dictionary of additional metadata for the Item.
    QVariantMap properties() const;

    //! Sets the item's additional metadata to \a properties
    void setProperties( const QVariantMap &properties );

    //! Returns a dictionary of asset objects that can be downloaded, each with a unique key.
    QMap< QString, QgsStacAsset > assets() const;

    //! Sets the \a asset objects that can be downloaded, each with a unique key.
    void setAssets( const QMap< QString, QgsStacAsset > &assets );

    //! Returns the id of the STAC Collection this Item references to
    QString collection() const;

    //! Sets the id of the STAC Collection this Item references to
    void setCollection( const QString &collection );

    /**
     *  Returns the single nominal date/time for the item, stored in the item's \a properties().
     *  If a temporal interval is more appropriate for this item then a null QDateTime is returned
     *  and the interval may be retrieved with dateTimeRange()
     *  \see hasDateTimeRange()
     *  \see dateTimeRange()
     */
    QDateTime dateTime() const;

    /**
     *  Returns TRUE if a temporal interval is available for this item, FALSE if a single QDateTime is available.
     *  \see hasDateTimeRange()
     *  \see dateTime()
     */
    bool hasDateTimeRange() const;

    /**
     *  Returns the temporal interval stored in the item's \a properties()
     *  \see hasDateTimeRange()
     *  \see dateTime()
     */
    QgsDateTimeRange dateTimeRange() const;

    /**
     * Returns an optional human readable title describing the Item.
     * \since QGIS 3.42
     */
    QString title() const;

    /**
     * Returns a Detailed multi-line description to fully explain the Item.
     * CommonMark 0.29 syntax may be used for rich text representation.
     * \since QGIS 3.42
     */
    QString description() const;

    /**
     * Returns a list of uris of all assets that have a cloud optimized format like COG or COPC
     * \since QGIS 3.42
     */
    QgsMimeDataUtils::UriList uris() const;

  private:
    QgsGeometry mGeometry;
    QgsBox3D mBbox;
    QVariantMap mProperties;
    QMap< QString, QgsStacAsset > mAssets;
    QString mCollection;
};

#endif // QGSSTACITEM_H
