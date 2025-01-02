/***************************************************************************
    qgsstacasset.h
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

#ifndef QGSSTACASSET_H
#define QGSSTACASSET_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsmimedatautils.h"

#include <QString>
#include <QStringList>

/**
 * \ingroup core
 * \brief Class for storing a STAC asset's data
 *
 * \note Not available in python bindings
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsStacAsset
{
  public:
    //! Constructor
    QgsStacAsset( const QString &href,
                  const QString &title,
                  const QString &description,
                  const QString &mediaType,
                  const QStringList &roles );

    //! Returns the URI to the asset object
    QString href() const;

    //! Returns the the displayed title for clients and users.
    QString title() const;

    /**
     * Returns a description of the Asset providing additional details, such as how it was processed or created.
     * CommonMark 0.29 syntax MAY be used for rich text representation.
     */
    QString description() const;

    //! Returns the media type of the asset
    QString mediaType() const;

    /**
     * Returns the roles assigned to the asset.
     * Roles are used to describe the purpose of the asset (eg. thumbnail, data etc).
     */
    QStringList roles() const;

    /**
     * Returns whether the asset is in a cloud optimized format like COG or COPC
     * \since QGIS 3.42
     */
    bool isCloudOptimized() const;

    /**
     * Returns the format name for cloud optimized formats
     * \since QGIS 3.42
     */
    QString formatName() const;

    /**
     * Returns a uri for the asset if it is a cloud optimized file like COG or COPC
     * \since QGIS 3.42
     */
    QgsMimeDataUtils::Uri uri() const;

  private:
    QString mHref;
    QString mTitle;
    QString mDescription;
    QString mMediaType;
    QStringList mRoles;
};

#endif // QGSSTACASSET_H
