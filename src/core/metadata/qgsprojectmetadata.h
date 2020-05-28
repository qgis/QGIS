/***************************************************************************
                             qgsprojectmetadata.h
                             -------------------
    begin                : March 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTMETADATA_H
#define QGSPROJECTMETADATA_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsabstractmetadatabase.h"

#include <QDateTime>

/**
 * \ingroup core
 * \class QgsProjectMetadata
 * \brief A structured metadata store for a map layer.
 *
 * QgsProjectMetadata handles storage and management of the metadata
 * for a QgsProject. This class is an internal QGIS format with a common
 * metadata structure, which allows for code to access the metadata properties for
 * projects in a uniform way.
 *
 * The metadata store is designed to be compatible with the Dublin Core metadata
 * specifications, and will be expanded to allow compatibility with ISO specifications
 * in future releases. However, the QGIS internal schema does not represent a superset
 * of all existing metadata schemas and accordingly conversion from specific
 * metadata formats to QgsProjectMetadata may result in a loss of information.
 *
 * This class is designed to follow the specifications detailed in
 * the schema definition available at resources/qgis-project-metadata.xsd
 * within the QGIS source code.
 *
 * Metadata can be validated through the use of QgsLayerMetadataValidator
 * subclasses. E.g. validating against the native QGIS metadata schema can be performed
 * using QgsNativeProjectMetadataValidator.
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProjectMetadata : public QgsAbstractMetadataBase
{
  public:

    /**
     * Constructor for QgsProjectMetadata.
     */
    QgsProjectMetadata() = default;

    QgsProjectMetadata *clone() const override SIP_FACTORY;

    /**
     * Returns the project author string.
     * \see setAuthor()
     */
    QString author() const;

    /**
     * Sets the project \a author string
     * \see author()
     */
    void setAuthor( const QString &author );

    /**
     * Returns the project's creation date/timestamp.
     * \see setCreationDateTime()
     */
    QDateTime creationDateTime() const;

    /**
     * Sets the project's creation date/timestamp.
     * \see creationDateTime()
     */
    void setCreationDateTime( const QDateTime &creationDateTime );

    bool readMetadataXml( const QDomElement &metadataElement ) override;
    bool writeMetadataXml( QDomElement &metadataElement, QDomDocument &document ) const override;

    bool operator==( const QgsProjectMetadata &metadataOther ) const;

  private:

    /*
     * IMPORTANT!!!!!!
     *
     * Do NOT add anything to this class without also updating the schema
     * definition located at resources/qgis-resource-metadata.xsd
     *
     */

    QString mAuthor;

    QDateTime mCreationDateTime;

    /*
     * IMPORTANT!!!!!!
     *
     * Do NOT add anything to this class without also updating the schema
     * definition located at resources/qgis-resource-metadata.xsd
     *
     */

};


#endif // QGSPROJECTMETADATA_H
