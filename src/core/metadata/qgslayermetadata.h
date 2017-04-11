/***************************************************************************
                             qgslayermetadata.h
                             -------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLAYERMETADATA_H
#define QGSLAYERMETADATA_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgscoordinatereferencesystem.h"

/**
 * \ingroup core
 * \class QgsLayerMetadata
 * \brief A structured metadata store for a map layer.
 *
 * QgsLayerMetadata handles storage and management of the metadata
 * for a QgsMapLayer. This class is an internal QGIS format with a common
 * metadata structure, which allows for code to access the metadata properties for
 * layers in a uniform way.
 *
 * The metadata store is designed to be compatible with the Dublin Core metadata
 * specifications, and will be expanded to allow compatibility with ISO specifications
 * in future releases. However, the QGIS internal schema does not represent a superset
 * of all existing metadata schemas and accordingly conversion from specific
 * metadata formats to QgsLayerMetadata may result in a loss of information.
 *
 * This class is designed to follow the specifications detailed in
 * the schema definition available at resources/qgis-resource-metadata.xsd
 * within the QGIS source code.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayerMetadata
{
  public:

    /**
     * Metadata constraint structure.
     */
    struct Constraint
    {

      /**
       * Constructor for Constraint.
       */
      Constraint( const QString &constraint = QString(), const QString &type = QString() )
        : type( type )
        , constraint( constraint )
      {}

      /**
       * Constraint type. Standard values include 'access' and 'other', however any
       * string can be used for the type.
       */
      QString type;

      /**
       * Free-form constraint string.
       */
      QString constraint;
    };

    /**
     * Metadata address structure.
     */
    struct Address
    {

      /**
       * Constructor for Address.
       */
      Address( const QString &type = QString(), const QString &address = QString(), const QString &city = QString(), const QString &administrativeArea = QString(), const QString &postalCode = QString(), const QString &country = QString() )
        : type( type )
        , address( address )
        , city( city )
        , administrativeArea( administrativeArea )
        , postalCode( postalCode )
        , country( country )
      {}

      /**
       * Type of address, e.g. 'postal'.
       */
      QString type;

      /**
       * Free-form physical address component, e.g. '221B Baker St' or 'P.O. Box 196'.
       */
      QString address;

      /**
       * City or locality name.
       */
      QString city;

      /**
       * Administrative area (state, provice/territory, etc.).
       */
      QString administrativeArea;

      /**
       * Postal (or ZIP) code.
       */
      QString postalCode;

      /**
       * Free-form country string.
       */
      QString country;
    };

    /**
     * Metadata contact structure.
     */
    struct Contact
    {

      /**
       * Constructor for Contact.
       */
      Contact( const QString &name = QString() )
        : name( name )
      {}

      /**
       * Name of contact.
       */
      QString name;

      /**
       * Organization contact belongs to/represents.
       */
      QString organization;

      /**
       * Position/title of contact.
       */
      QString position;

      /**
       * List of addresses associated with this contact.
       */
      QList< QgsLayerMetadata::Address > addresses;

      /**
       * Voice telephone.
       */
      QString voice;

      /**
       * Facsimile telephone.
       */
      QString fax;

      /**
       * Electronic mail address.
       * \note Do not include mailto: protocol as part of the email address.
       */
      QString email;

      /**
       * Role of contact. Acceptable values are those from the ISO 19115 CI_RoleCode specifications
       * (see http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml).
       * E.g. 'custodian', 'owner', 'distributor', etc.
       */
      QString role;
    };

    /**
     * Metadata link structure.
     */
    struct Link
    {

      /**
       * Constructor for Link.
       */
      Link( const QString &name = QString(), const QString &type = QString(), const QString &url = QString() )
        : name( name )
        , type( type )
        , url( url )
      {}

      /**
       * Short link name. E.g. WMS layer name.
       */
      QString name;

      /**
       * Link type. It is strongly suggested to use values from the 'identifier'
       * column in https://github.com/OSGeo/Cat-Interop/blob/master/LinkPropertyLookupTable.csv
       */
      QString type;

      /**
       * Abstract text about link.
       */
      QString description;

      /**
       * Link url.  If the URL is an OWS server, specify the *base* URL only without parameters like service=xxx....
       */
      QString url;

      /**
       * Format specification of online resource. It is strongly suggested to use GDAL/OGR format values.
       */
      QString format;

      /**
       * MIME type representative of the online resource response (image/png, application/json, etc.)
       */
      QString mimeType;

      /**
       * Estimated size (in bytes) of the online resource response.
       */
      QString size;
    };

    /**
     * Constructor for QgsLayerMetadata.
     */
    QgsLayerMetadata() = default;

    virtual ~QgsLayerMetadata() = default;

    /**
     * A reference, URI, URL or some other mechanism to identify the resource.
     * \see setIdentifier()
     */
    QString identifier() const;

    /**
     * Sets the reference, URI, URL or some other mechanism to identify the resource.
     * \see identifier()
     */
    void setIdentifier( const QString &identifier );

    /**
     * A reference, URI, URL or some other mechanism to identify the parent resource that this resource is a part (child) of.
     * Returns an empty string if no parent identifier is set.
     * \see setParentIdentifier()
     */
    QString parentIdentifier() const;

    /**
     * Sets a reference, URI, URL or some other mechanism to identify the parent resource that this resource is a part (child) of.
     * Set an empty string if no parent identifier is required.
     * \see parentIdentifier()
     */
    void setParentIdentifier( const QString &parentIdentifier );

    /**
     * Returns the human language associated with the resource. Usually the returned string
     * will follow either the ISO 639.2 or ISO 3166 specifications, e.g. 'ENG' or 'SPA', however
     * this is not a hard requirement and the caller must account for non compliant
     * values.
     * \see setLanguage()
     */
    QString language() const;

    /**
     * Sets the human \a language associated with the resource. While a formal vocabulary is not imposed,
     * ideally values should be taken from the ISO 639.2 or ISO 3166 specifications,
     * e.g. 'ENG' or 'SPA' (ISO 639.2) or 'EN-AU' (ISO 3166).
     * \see language()
     */
    void setLanguage( const QString &language );

    /**
     * Returns the nature of the resource.  While a formal vocabulary is not imposed, it is advised
     * to use the ISO 19115 MD_ScopeCode values. E.g. 'dataset' or 'series'.
     * \see setType()
     */
    QString type() const;

    /**
     * Sets the \a type (nature) of the resource.  While a formal vocabulary is not imposed, it is advised
     * to use the ISO 19115 MD_ScopeCode values. E.g. 'dataset' or 'series'.
     * \see type()
     */
    void setType( const QString &type );

    /**
     * Returns the human readable name of the resource, typically displayed in search results.
     * \see setTitle()
     */
    QString title() const;

    /**
     * Sets the human readable \a title (name) of the resource, typically displayed in search results.
     * \see title()
     */
    void setTitle( const QString &title );

    /**
     * Returns a free-form description of the resource.
     * \see setAbstract()
     */
    QString abstract() const;

    /**
     * Sets a free-form \a abstract (description) of the resource.
     * \see abstract()
     */
    void setAbstract( const QString &abstract );

    /**
     * Returns any fees associated with using the resource.
     * An empty string will be returned if no fees are set.
     * \see setFees()
     */
    QString fees() const;

    /**
     * Sets the \a fees associated with using the resource.
     * Use an empty string if no fees are set.
     * \see fees()
     */
    void setFees( const QString &fees );

    /**
     * Returns a list of constraints associated with using the resource.
     * \see setConstraints()
     */
    QList< QgsLayerMetadata::Constraint > constraints() const;

    /**
     * Sets the list of \a constraints associated with using the resource.
     * \see constraints()
     */
    void setConstraints( const QList<QgsLayerMetadata::Constraint> &constraints );

    /**
     * Returns a list of attribution or copyright strings associated with the resource.
     * \see setRights()
     */
    QStringList rights() const;

    /**
     * Sets a list of \a rights (attribution or copyright strings) associated with the resource.
     * \see rights()
     */
    void setRights( const QStringList &rights );

    /**
     * Returns the character encoding of the data in the resource. An empty string will be returned if no encoding is set.
     * \see setEncoding()
     */
    QString encoding() const;

    /**
     * Sets the character \a encoding of the data in the resource. Use an empty string if no encoding is set.
     * \see encoding()
     */
    void setEncoding( const QString &encoding );

    /**
     * Returns the coordinate reference system described by the layer's metadata.
     *
     * Note that this has no link to QgsMapLayer::crs(). While in most cases these
     * two systems are likely to be identical, it is possible to have a layer
     * with a different CRS described by it's accompanying metadata versus the
     * CRS which is actually used to display and manipulate the layer within QGIS.
     * This may be the case when a layer has an incorrect CRS within its metadata
     * and a user has manually overridden the layer's CRS within QGIS.
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets the coordinate reference system for the layer's metadata.
     *
     * Note that this has no link to QgsMapLayer::setCrs(). Setting the layer's
     * CRS via QgsMapLayer::setCrs() does not affect the layer's metadata CRS,
     * and changing the CRS from the metadata will not change the layer's
     * CRS or how it is projected within QGIS.
     *
     * While ideally these two systems are likely to be identical, it is possible to have a layer
     * with a different CRS described by it's accompanying metadata versus the
     * CRS which is actually used to display and manipulate the layer within QGIS.
     * This may be the case when a layer has an incorrect CRS within its metadata
     * and a user has manually overridden the layer's CRS within QGIS.
     * \see setCrs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the keywords map, which is a set of descriptive keywords associated with the resource.
     *
     * The map key is the vocabulary string and map value is a list of keywords for that vocabulary.
     *
     * The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
     * associated with keyword list.
     *
     * \see setKeywords()
     * \see keywordVocabularies()
     */
    QMap<QString, QStringList> keywords() const;

    /**
     * Sets the \a keywords map, which is a set of descriptive keywords associated with the resource.
     *
     * The map key is the vocabulary string and map value is a list of keywords for that vocabulary.
     * Calling this replaces any existing keyword vocabularies.
     *
     * The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
     * associated with keyword list.
     *
     * \see keywords()
     * \see addKeywords()
     */
    void setKeywords( const QMap<QString, QStringList> &keywords );

    /**
     * Adds a list of descriptive \a keywords for a specified \a vocabulary. Any existing
     * keywords for the same vocabulary will be replaced. Other vocabularies
     * will not be affected.
     *
     * The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
     * associated with keyword list.
     *
     * \see setKeywords()
     */
    void addKeywords( const QString &vocabulary, const QStringList &keywords );

    /**
     * Returns a list of keyword vocabularies contained in the metadata.
     *
     * The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
     * associated with keyword list.
     *
     * \see keywords()
     */
    QStringList keywordVocabularies() const;

    /**
     * Returns a list of keywords for the specified \a vocabulary.
     * If the vocabulary is not contained in the metadata, an empty
     * list will be returned.
     *
     * The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
     * associated with keyword list.
     *
     * \see keywordVocabularies()
     */
    QStringList keywords( const QString &vocabulary ) const;

    /**
     * Returns a list of contact persons or entities associated with the resource.
     * \see setContacts()
     */
    QList<QgsLayerMetadata::Contact> contacts() const;

    /**
     * Sets the list of \a contacts or entities associated with the resource. Any existing contacts
     * will be replaced.
     * \see contacts()
     * \see addContact()
     */
    void setContacts( const QList<QgsLayerMetadata::Contact> &contacts );

    /**
     * Adds an individual \a contact to the existing contacts.
     * \see contacts()
     * \see setContacts()
     */
    void addContact( const QgsLayerMetadata::Contact &contact );

    /**
     * Returns a list of online resources associated with the resource.
     * \see setLinks()
     */
    QList<QgsLayerMetadata::Link> links() const;

    /**
     * Sets the list of online resources associated with the resource. Any existing links
     * will be replaced.
     * \see links()
     * \see addLink()
     */
    void setLinks( const QList<QgsLayerMetadata::Link> &links );

    /**
     * Adds an individual \a link to the existing links.
     * \see links()
     * \see setLinks()
     */
    void addLink( const QgsLayerMetadata::Link &link );

  private:

    /*
     * IMPORTANT!!!!!!
     *
     * Do NOT add anything to this class without also updating the schema
     * definition located at resources/qgis-resource-metadata.xsd
     *
     */

    QString mIdentifier;
    QString mParentIdentifier;
    QString mLanguage;
    QString mType;
    QString mTitle;
    QString mAbstract;
    QString mFees;
    QList<QgsLayerMetadata::Constraint> mConstraints;
    QStringList mRights;

    // IMPORTANT - look up before adding anything here!!

    QString mEncoding;
    QgsCoordinateReferenceSystem mCrs;

    /**
     * Keywords map. Key is the vocabulary, value is a list of keywords for that vocabulary.
     */
    QMap< QString, QStringList > mKeywords;

    QList< Contact > mContacts;

    QList< Link > mLinks;

    /*
     * IMPORTANT!!!!!!
     *
     * Do NOT add anything to this class without also updating the schema
     * definition located at resources/qgis-resource-metadata.xsd
     *
     */

};

#endif // QGSLAYERMETADATA_H
