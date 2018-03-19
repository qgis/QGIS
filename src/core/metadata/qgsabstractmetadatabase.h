/***************************************************************************
                             QgsAbstractMetadataBase.h
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

#ifndef QGSABSTRACTMETADATABASE_H
#define QGSABSTRACTMETADATABASE_H

#include "qgis.h"
#include "qgis_core.h"

class QDomElement;
class QDomDocument;

/**
 * \ingroup core
 * \class QgsAbstractMetadataBase
 * \brief An abstract base class for metadata stores.
 *
 * QgsAbstractMetadataBase is the base class for handling storage and management of the metadata
 * for various map related assets. This class is an internal QGIS format with a common
 * metadata structure. It is subclassed by layer and project specific metadata classes,
 * such as QgsLayerMetadata and QgsProjectMetadata.
 *
 * The metadata store is designed to be compatible with the Dublin Core metadata
 * specifications, and will be expanded to allow compatibility with ISO specifications
 * in future releases. However, the QGIS internal schema does not represent a superset
 * of all existing metadata schemas and accordingly conversion from specific
 * metadata formats to QgsAbstractMetadataBase may result in a loss of information.
 *
 * This class is designed to follow the specifications detailed in
 * the schema definition available at resources/qgis-base-metadata.xsd
 * within the QGIS source code.
 *
 * Metadata can be validated through the use of QgsAbstractMetadataBaseValidator
 * subclasses. E.g. validating against the native QGIS metadata schema can be performed
 * using QgsNativeMetadataValidator.
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsAbstractMetadataBase
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsLayerMetadata * >( sipCpp ) != NULL )
      sipType = sipType_QgsLayerMetadata;
    else if ( dynamic_cast< QgsProjectMetadata * >( sipCpp ) != NULL )
      sipType = sipType_QgsProjectMetadata;
    else
      sipType = NULL;
    SIP_END
#endif

  public:

    // NOTE - these really belong in a separate namespace, but SIP says no, I want to make you waste more time
    // TODO: dump sip

    /**
     * Map of vocabulary string to keyword list.
     */
    typedef QMap< QString, QStringList > KeywordMap;

    /**
     * Metadata address structure.
     * \ingroup core
     * \since QGIS 3.2
     */
    struct CORE_EXPORT Address
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
       * Administrative area (state, province/territory, etc.).
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

      bool operator==( const QgsAbstractMetadataBase::Address &other ) const;
    };

    /**
     * Metadata contact structure.
     * \ingroup core
     * \since QGIS 3.2
     */
    struct CORE_EXPORT Contact
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
      QList< QgsAbstractMetadataBase::Address > addresses;

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

      bool operator==( const QgsAbstractMetadataBase::Contact &other ) const;
    };

    /**
     * A list of contacts.
     * \ingroup core
     * \since QGIS 3.2
     */
    typedef QList< QgsAbstractMetadataBase::Contact > ContactList;


    /**
     * Metadata link structure.
     * \ingroup core
     * \since QGIS 3.2
     */
    struct CORE_EXPORT Link
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

      bool operator==( const QgsAbstractMetadataBase::Link &other ) const;
    };

    /**
     * A list of links.
     * \ingroup core
     * \since QGIS 3.2
     */
    typedef QList< QgsAbstractMetadataBase::Link > LinkList;

    virtual ~QgsAbstractMetadataBase() = default;

    /**
     * Clones the metadata object.
     * \since QGIS 3.2
     */
    virtual QgsAbstractMetadataBase *clone() const = 0 SIP_FACTORY;

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
     * Returns a freeform description of the history or lineage of the resource.
     * \see setHistory()
     */
    QStringList history() const;

    /**
     * Sets the freeform description of the \a history or lineage of the resource.
     * Any existing history items will be overwritten.
     * \see addHistoryItem()
     * \see history()
     */
    void setHistory( const QStringList &history );

    /**
     * Adds a single history \a text to the end of the existing history list.
     * \see history()
     * \see setHistory()
     */
    void addHistoryItem( const QString &text );

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
    QgsAbstractMetadataBase::KeywordMap keywords() const;

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
    void setKeywords( const QgsAbstractMetadataBase::KeywordMap &keywords );

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
     * Remove a vocabulary from the list.
     *
     * \see setKeywords()
     * \see addKeywords()
     */
    bool removeKeywords( const QString &vocabulary );

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
     * Returns categories of the resource.
     * Categories are stored using a special vocabulary 'gmd:topicCategory' in keywords.
     *
     * \see keywords()
     */
    QStringList categories() const;

    /**
     * Sets categories of the resource.
     * Categories are stored using a special vocabulary 'gmd:topicCategory' in keywords.
     *
     * \see keywords()
     */
    void setCategories( const QStringList &categories );

    /**
     * Returns a list of contact persons or entities associated with the resource.
     * \see setContacts()
     */
    QgsAbstractMetadataBase::ContactList contacts() const;

    /**
     * Sets the list of \a contacts or entities associated with the resource. Any existing contacts
     * will be replaced.
     * \see contacts()
     * \see addContact()
     */
    void setContacts( const QgsAbstractMetadataBase::ContactList &contacts );

    /**
     * Adds an individual \a contact to the existing contacts.
     * \see contacts()
     * \see setContacts()
     */
    void addContact( const QgsAbstractMetadataBase::Contact &contact );

    /**
     * Returns a list of online resources associated with the resource.
     * \see setLinks()
     */
    QgsAbstractMetadataBase::LinkList links() const;

    /**
     * Sets the list of online resources associated with the resource. Any existing links
     * will be replaced.
     * \see links()
     * \see addLink()
     */
    void setLinks( const QgsAbstractMetadataBase::LinkList &links );

    /**
     * Adds an individual \a link to the existing links.
     * \see links()
     * \see setLinks()
     */
    void addLink( const QgsAbstractMetadataBase::Link &link );

    /**
     * Sets state from DOM document.
     *
     * \param metadataElement The DOM element corresponding to ``resourceMetadata'' tag
     *
     * \returns true if successful
     *
     * Subclasses which override this method should take care to also call the base
     * class method in order to read common metadata properties.
     */
    virtual bool readMetadataXml( const QDomElement &metadataElement );

    /**
     * Stores state in a DOM node.
     *
     * \param metadataElement is a DOM element corresponding to ``resourceMetadata'' tag
     * \param document is a the DOM document being written
     *
     * \returns true if successful
     *
     * Subclasses which override this method should take care to also call the base
     * class method in order to write common metadata properties.
     */
    virtual bool writeMetadataXml( QDomElement &metadataElement, QDomDocument &document ) const;

  protected:

    /**
     * Constructor for QgsAbstractMetadataBase.
     *
     * QgsAbstractMetadataBase cannot be instantiated directly, it must be subclassed.
     */
    QgsAbstractMetadataBase() = default;

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
    QStringList mHistory;

    // IMPORTANT - look up before adding anything here!!

    /**
     * Keywords map. Key is the vocabulary, value is a list of keywords for that vocabulary.
     */
    QgsAbstractMetadataBase::KeywordMap mKeywords;

    QgsAbstractMetadataBase::ContactList mContacts;

    QgsAbstractMetadataBase::LinkList mLinks;

    /*
     * IMPORTANT!!!!!!
     *
     * Do NOT add anything to this class without also updating the schema
     * definition located at resources/qgis-resource-metadata.xsd
     *
     */


    /**
     * Tests whether the common metadata fields in this object are equal to \a other.
     *
     * Subclasses should utilize this method from their equality operators to test
     * equality of base class members.
     *
     * \since QGIS 3.2
     */
    bool equals( const QgsAbstractMetadataBase &other ) const;

};

Q_DECLARE_METATYPE( QgsAbstractMetadataBase::KeywordMap )
Q_DECLARE_METATYPE( QgsAbstractMetadataBase::ContactList )
Q_DECLARE_METATYPE( QgsAbstractMetadataBase::LinkList )

#endif // QGSABSTRACTMETADATABASE_H
