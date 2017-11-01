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
#include "qgsbox3d.h"
#include "qgsrange.h"

class QgsMapLayer;

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
 * Metadata can be validated through the use of QgsLayerMetadataValidator
 * subclasses. E.g. validating against the native QGIS metadata schema can be performed
 * using QgsNativeMetadataValidator.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayerMetadata
{
  public:

    /**
     * Map of vocabulary string to keyword list.
     */
    typedef QMap< QString, QStringList > KeywordMap;

    /**
     * Metadata spatial extent structure.
     */
    struct CORE_EXPORT SpatialExtent
    {

      /**
       * Coordinate reference system for spatial extent.
       * \see spatial
       */
      QgsCoordinateReferenceSystem extentCrs;

      /**
       * Geospatial extent of the resource. X and Y coordinates are in the
       * CRS defined by the metadata (see extentCrs).
       *
       * While the spatial extent can include a Z dimension, this is not
       * compulsory.
       * \see extentCrs
       */
      QgsBox3d bounds;
    };

    /**
     * Metadata extent structure.
     */
    struct CORE_EXPORT Extent
    {
      public:

        /**
         * Spatial extents of the resource.
         * \see setSpatialExtents()
         */
        QList< QgsLayerMetadata::SpatialExtent > spatialExtents() const;

        /**
         * Sets the spatial \a extents of the resource.
         * \see spatialExtents()
         */
        void setSpatialExtents( const QList< QgsLayerMetadata::SpatialExtent > &extents );

        /**
         * Temporal extents of the resource. Use QgsDateTimeRange::isInstant() to determine
         * whether the temporal extent is a range or a single point in time.
         * If QgsDateTimeRange::isInfinite() returns true then the temporal extent
         * is considered to be indeterminate and continuous.
         * \see setTemporalExtents()
         */
        QList< QgsDateTimeRange > temporalExtents() const;

        /**
         * Sets the temporal \a extents of the resource.
         * \see temporalExtents()
         */
        void setTemporalExtents( const QList< QgsDateTimeRange > &extents );

#ifndef SIP_RUN
      private:

        QList< QgsLayerMetadata::SpatialExtent > mSpatialExtents;
        QList< QgsDateTimeRange > mTemporalExtents;

#endif

    };

    /**
     * Metadata constraint structure.
     */
    struct CORE_EXPORT Constraint
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
     * A list of constraints.
     */
    typedef QList< QgsLayerMetadata::Constraint > ConstraintList;


    /**
     * Metadata address structure.
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
     * A list of contacts.
     */
    typedef QList< QgsLayerMetadata::Contact > ContactList;


    /**
     * Metadata link structure.
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
    };

    /**
     * A list of links.
     */
    typedef QList< QgsLayerMetadata::Link > LinkList;

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
    QgsLayerMetadata::ConstraintList constraints() const;

    /**
     * Adds an individual constraint to the existing constraints.
     * \see constraints()
     * \see setConstraints()
     */
    void addConstraint( const QgsLayerMetadata::Constraint &constraint );

    /**
     * Sets the list of \a constraints associated with using the resource.
     * \see constraints()
     */
    void setConstraints( const QgsLayerMetadata::ConstraintList &constraints );

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
     * Returns a list of licenses associated with the resource (examples: http://opendefinition.org/licenses/).
     * \see setLicenses()
     */
    QStringList licenses() const;

    /**
     * Sets a list of \a licenses associated with the resource.
     * (examples: http://opendefinition.org/licenses/).
     * \see licenses()
     */
    void setLicenses( const QStringList &licenses );

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
     * Returns the spatial and temporal extents associated with the resource.
     * \see setExtent()
     */
    SIP_SKIP const QgsLayerMetadata::Extent &extent() const;

    /**
     * Returns the spatial and temporal extents associated with the resource.
     * \see setExtent()
     */
    QgsLayerMetadata::Extent &extent();

    /**
     * Sets the spatial and temporal extents associated with the resource.
     * \see setExtent()
     */
    void setExtent( const QgsLayerMetadata::Extent &extent );

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
     * \see crs()
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
    KeywordMap keywords() const;

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
    void setKeywords( const KeywordMap &keywords );

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
    QgsLayerMetadata::ContactList contacts() const;

    /**
     * Sets the list of \a contacts or entities associated with the resource. Any existing contacts
     * will be replaced.
     * \see contacts()
     * \see addContact()
     */
    void setContacts( const QgsLayerMetadata::ContactList &contacts );

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
    QgsLayerMetadata::LinkList links() const;

    /**
     * Sets the list of online resources associated with the resource. Any existing links
     * will be replaced.
     * \see links()
     * \see addLink()
     */
    void setLinks( const QgsLayerMetadata::LinkList &links );

    /**
     * Adds an individual \a link to the existing links.
     * \see links()
     * \see setLinks()
     */
    void addLink( const QgsLayerMetadata::Link &link );

    /**
     * Saves the metadata to a \a layer's custom properties (see QgsMapLayer::setCustomProperty() ).
     * \see readFromLayer()
     */
    void saveToLayer( QgsMapLayer *layer ) const;

    /**
     * Reads the metadata state from a \a layer's custom properties (see QgsMapLayer::customProperty() ).
     * \see saveToLayer()
     */
    void readFromLayer( const QgsMapLayer *layer );

    /**
     * Sets state from Dom document
     * \param metadataElement The Dom element corresponding to ``resourceMetadata'' tag
     *
     * \returns true if successful
     */
    bool readMetadataXml( const QDomElement &metadataElement );

    /**
     * Stores state in Dom node
     * \param metadataElement is a Dom element corresponding to ``resourceMetadata'' tag
     * \param document is a the dom document being written
     *
     * \returns true if successful
     */
    bool writeMetadataXml( QDomElement &metadataElement, QDomDocument &document ) const;

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
    ConstraintList mConstraints;
    QStringList mRights;
    QStringList mLicenses;
    QStringList mHistory;

    // IMPORTANT - look up before adding anything here!!

    QString mEncoding;
    QgsCoordinateReferenceSystem mCrs;

    Extent mExtent;

    /**
     * Keywords map. Key is the vocabulary, value is a list of keywords for that vocabulary.
     */
    KeywordMap mKeywords;

    ContactList mContacts;

    LinkList mLinks;

    /*
     * IMPORTANT!!!!!!
     *
     * Do NOT add anything to this class without also updating the schema
     * definition located at resources/qgis-resource-metadata.xsd
     *
     */

};

Q_DECLARE_METATYPE( QgsLayerMetadata::KeywordMap )
Q_DECLARE_METATYPE( QgsLayerMetadata::ConstraintList )
Q_DECLARE_METATYPE( QgsLayerMetadata::ContactList )
Q_DECLARE_METATYPE( QgsLayerMetadata::LinkList )
Q_DECLARE_METATYPE( QgsLayerMetadata::Extent )

#endif // QGSLAYERMETADATA_H
