/***************************************************************************
    qgsstaccatalog.h
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

#ifndef QGSSTACCATALOG_H
#define QGSSTACCATALOG_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsstacobject.h"

#include <QStringList>
#include <QSet>

/**
 * \ingroup core
 * \brief Class for storing a STAC Catalog's data
 *
 * \note Not available in python bindings
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsStacCatalog : public QgsStacObject
{
  public:
    //! Default constructor deleted, use the variant with required parameters
    QgsStacCatalog() = delete;

    /**
     * Constructs a valid QgsStacCatalog
     * \param id A unique identifier for the catalog
     * \param version The STAC version the Catalog implements.
     * \param description Detailed multi-line description to fully explain the Catalog. CommonMark 0.29 syntax may be used for rich text representation.
     * \param links A list of references to other documents.
     */
    QgsStacCatalog( const QString &id,
                    const QString &version,
                    const QString &description,
                    const QVector< QgsStacLink > &links );

    QgsStacObject::Type type() const override;
    QString toHtml() const override;

    //! Returns a short descriptive one-line title for the Catalog.
    QString title() const;

    //! Sets a short descriptive one-line \a title for the Catalog.
    void setTitle( const QString &title );

    /**
     * Returns a Detailed multi-line description to fully explain the Catalog.
     * CommonMark 0.29 syntax may be used for rich text representation.
     */
    QString description() const;

    /**
     * Sets a detailed multi-line \a description to fully explain the Catalog.
     * CommonMark 0.29 syntax may be used for rich text representation.
     */
    void setDescription( const QString &description );

    //! Sets the list of \a conformanceClasses this catalog conforms to.
    void setConformanceClasses( const QStringList &conformanceClasses );

    //! Adds \a conformanceClass to the list of catalog's conformance classes
    void addConformanceClass( const QString &conformanceClass );

    //! Checks if the catalog is a STAC API conforming to the specified \a conformanceClass
    bool conformsTo( const QString &conformanceClass ) const;

    //! Checks if the catalog is a STAC API endpoint
    bool supportsStacApi() const;

  protected:
    QString mTitle;
    QString mDescription;
    QSet< QString > mConformanceClasses;
};

#endif // QGSSTACCATALOG_H
