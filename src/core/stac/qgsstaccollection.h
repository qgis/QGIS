/***************************************************************************
    qgsstaccollection.h
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

#ifndef QGSSTACCOLLECTION_H
#define QGSSTACCOLLECTION_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsstaccatalog.h"
#include "qgsstacextent.h"
#include "qgsstacasset.h"
#include "qgsstacprovider.h"

/**
 * \ingroup core
 * \brief Class for storing a STAC Collection's data
 *
 * \note Not available in python bindings
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsStacCollection : public QgsStacCatalog
{
  public:
    //! Default constructor deleted, use the variant with required parameters
    QgsStacCollection() = delete;

    /**
     * Constructs a valid QgsStacCollection
     * \param id Identifier for the Collection that is unique across the provider.
     * \param version The STAC version the Collection implements.
     * \param description Detailed multi-line description to fully explain the Collection. CommonMark 0.29 syntax MAY be used for rich text representation.
     * \param links A list of references to other documents.
     * \param license Collection's license(s), either a SPDX License identifier, various if multiple licenses apply or proprietary for all other cases.
     * \param extent Spatial and temporal extents of the collection.
     */
    QgsStacCollection( const QString &id,
                       const QString &version,
                       const QString &description,
                       const QVector< QgsStacLink > &links,
                       const QString &license,
                       const QgsStacExtent &extent );

    QgsStacObject::Type type() const override;
    QString toHtml() const override;

    //! Returns the list of keywords describing the Collection
    QStringList keywords() const;

    //! Sets the list of \a keywords describing the Collection
    void setKeywords( const QStringList &keywords );

    //! Returns the Collection's license(s), either a SPDX License identifier, various if multiple licenses apply or proprietary for all other cases.
    QString license() const;

    //! Sets the Collection's license(s), either a SPDX License identifier, various if multiple licenses apply or proprietary for all other cases.
    void setLicense( const QString &license );

    /**
     * Returns a list of providers, which may include all organizations capturing or processing the data or the hosting provider.
     * Providers should be listed in chronological order with the most recent provider being the last element of the list.
     */
    QVector<QgsStacProvider> providers() const;

    /**
     * Sets a list of \a providers, which may include all organizations capturing or processing the data or the hosting provider.
     * Providers should be listed in chronological order with the most recent provider being the last element of the list.
     */
    void setProviders( const QVector<QgsStacProvider> &providers );

    //! Returns the collection's spatial and temporal extent
    QgsStacExtent extent() const;

    //! Sets the collection's spatial and temporal extent
    void setExtent( const QgsStacExtent &extent );

    //! Returns a map of property summaries from the collection
    QVariantMap summaries() const;

    //! Sets the map of property summaries to the collection
    void setSummaries( const QVariantMap &summaries );

    //! Returns a dictionary of asset objects in the catalog, each with a unique id key.
    QMap<QString, QgsStacAsset> assets() const;

    //! Sets the asset objects in the catalog, each with a unique id key.
    void setAssets( const QMap<QString, QgsStacAsset> &assets );

  private:
    QStringList mKeywords;
    QString mLicense;
    QVector< QgsStacProvider > mProviders;
    QgsStacExtent mExtent;
    QVariantMap mSummaries;
    QMap< QString, QgsStacAsset > mAssets;
};

#endif // QGSSTACCOLLECTION_H
