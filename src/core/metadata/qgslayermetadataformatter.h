/***************************************************************************
                             qgslayermetadataformatter.h
                             ---------------------------
    begin                : September 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERMETADATAFORMATTER_H
#define QGSLAYERMETADATAFORMATTER_H

#include <QCoreApplication>

#include "qgis.h"
#include "qgis_core.h"
#include "qgslayermetadata.h"

/**
 * \ingroup core
 * \class QgsLayerMetadataFormatter
 * \brief Class for metadata formatter.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayerMetadataFormatter
{
    Q_DECLARE_TR_FUNCTIONS( "QgsLayerMetadataFormatter" )

  public:

    /**
     * Constructor for QgsLayerMetadataFormatter
     */
    QgsLayerMetadataFormatter( const QgsLayerMetadata &metadata );

    /**
     * Formats the "Access" section according to a \a metadata object.
     * This will return a HTML table.
     */
    QString accessSectionHtml() const;

    /**
     * Formats the "Contacts" section according to a \a metadata object.
     * This will return a HTML table.
     */
    QString contactsSectionHtml() const;

    /**
     * Formats the "Extents" section according to a \a metadata object (extent and temporal).
     * This will return a HTML table.
     * \param showSpatialExtent flag if the spatial extent needs to be displayed. Default to true.
     */
    QString extentSectionHtml( const bool showSpatialExtent = true ) const;

    /**
     * Formats the "Identification" section according to a \a metadata object.
     * This will return a HTML table.
     */
    QString identificationSectionHtml() const;

    /**
     * Formats the "History" section according to a \a metadata object.
     * This will return a HTML table.
     */
    QString historySectionHtml() const;

    /**
     * Formats the "Links" section according to a \a metadata object.
     * This will return a HTML table.
     */
    QString linksSectionHtml() const;

  private:
    QgsLayerMetadata mMetadata;
};

#endif // QGSLAYERMETADATAFORMATTER_H
